#include "file_helper.hpp"

#include <spdlog/common.h>
#include <spdlog/details/os.h>

#include <cerrno>
#include <chrono>
#include <cstdio>
#include <string>
#include <thread>
#include <tuple>

namespace storm::logging::details
{
using namespace spdlog;
using namespace spdlog::details;

file_helper::~file_helper()
{
    close();
}

void file_helper::open(const filename_t &fname, bool truncate)
{
    close();
    filename_ = fname;

    auto *mode = SPDLOG_FILENAME_T("ab");
    auto *trunc_mode = SPDLOG_FILENAME_T("wb");

    for (int tries = 0; tries < open_tries_; ++tries)
    {
        // create containing folder if not exists already.
        os::create_dir(os::dir_name(fname));
        if (truncate)
        {
            // Truncate by opening-and-closing a tmp file in "wb" mode, always
            // opening the actual log-we-write-to in "ab" mode, since that
            // interacts more politely with eternal processes that might
            // rotate/truncate the file underneath us.
            std::FILE *tmp;
            if (os::fopen_s(&tmp, fname, trunc_mode))
            {
                continue;
            }
            std::fclose(tmp);
        }
        if (!os::fopen_s(&fd_, fname, mode))
        {
            return;
        }

        details::os::sleep_for_millis(open_interval_);
    }

    throw_spdlog_ex("Failed opening file " + os::filename_to_str(filename_) + " for writing", errno);
}

void file_helper::reopen(bool truncate)
{
    if (filename_.empty())
    {
        throw_spdlog_ex("Failed re opening file - was not opened before");
    }
    this->open(filename_, truncate);
}

void file_helper::flush()
{
    std::fflush(fd_);
}

void file_helper::close()
{
    if (fd_ != nullptr)
    {
        std::fclose(fd_);
        fd_ = nullptr;
    }
}

void file_helper::write(const memory_buf_t &buf)
{
    size_t msg_size = buf.size();
    auto data = buf.data();
    if (std::fwrite(data, 1, msg_size, fd_) != msg_size)
    {
        throw_spdlog_ex("Failed writing to file " + os::filename_to_str(filename_), errno);
    }
}

size_t file_helper::size() const
{
    if (fd_ == nullptr)
    {
        throw_spdlog_ex("Cannot use size() on closed file " + os::filename_to_str(filename_));
    }
    return os::filesize(fd_);
}

const filename_t &file_helper::filename() const
{
    return filename_;
}

std::FILE *file_helper::getfd() const
{
    return fd_;
}

//
// return file path and its extension:
//
// "mylog.txt" => ("mylog", ".txt")
// "mylog" => ("mylog", "")
// "mylog." => ("mylog.", "")
// "/dir1/dir2/mylog.txt" => ("/dir1/dir2/mylog", ".txt")
//
// the starting dot in filenames is ignored (hidden files):
//
// ".mylog" => (".mylog". "")
// "my_folder/.mylog" => ("my_folder/.mylog", "")
// "my_folder/.mylog.txt" => ("my_folder/.mylog", ".txt")
std::tuple<filename_t, filename_t> file_helper::split_by_extension(const filename_t &fname)
{
    auto ext_index = fname.rfind('.');

    // no valid extension found - return whole path and empty string as
    // extension
    if (ext_index == filename_t::npos || ext_index == 0 || ext_index == fname.size() - 1)
    {
        return std::make_tuple(fname, filename_t());
    }

    // treat cases like "/etc/rc.d/somelogfile or "/abc/.hiddenfile"
    auto folder_index = fname.find_last_of(details::os::folder_seps_filename);
    if (folder_index != filename_t::npos && folder_index >= ext_index - 1)
    {
        return std::make_tuple(fname, filename_t());
    }

    // finally - return a valid base and extension tuple
    return std::make_tuple(fname.substr(0, ext_index), fname.substr(ext_index));
}

} // namespace storm::logging::details
