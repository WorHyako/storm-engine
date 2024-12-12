#include "logging.hpp"

#include <spdlog/spdlog.h>

#include "spdlog_sinks/syncable_sink.hpp"

#include "Filesystem/Constants/Paths.hpp"

namespace
{

constexpr auto kLogExtension = ".log";

}

namespace storm::logging
{

// TODO: loggers with periodic flush shall be thread safe; we should measure performance diff and decide what to do
logger_ptr getOrCreateLogger(const std::string &name, const spdlog::level::level_enum level,
                                                  const bool truncate)
{
    auto logger = spdlog::get(name);
    if (logger)
    {
        return logger;
    }

    auto path = std::filesystem::path(Storm::Filesystem::Constants::Paths::logs()) / name;
    path += kLogExtension;

    logger = spdlog::create<sinks::syncable_sink>(name, path.string(), truncate);
    logger->set_level(level);

    return logger;
}

} // namespace storm::logging
