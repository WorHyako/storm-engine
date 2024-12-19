#include "Filesystem/Config/Config.hpp"

using namespace Storm::Filesystem;

Config::Config() noexcept
    : _section(nullptr) {
}

Config Config::load(const std::filesystem::path& file_path) noexcept {
    Config config;
    if (!std::filesystem::exists(file_path)) {
        std::printf("Can't open config file - %s\n", file_path.string().c_str());
        return config;
    }
    try {
        config._config = toml::parse_file(file_path.string());
    } catch (const toml::parse_error &err) {
        std::printf("Can't parse config file - %s\n", file_path.string().c_str());
        std::printf("\t\tErorr - %s\n\t\t%s\n", err.what(), std::string(err.description()).c_str());
    }
    return config;
}

bool Config::select_section(const std::string &section_name) noexcept {
    if (_config.empty()) {
        return false;
    }
    const auto section_node = _config.get(section_name);
    const auto section_repr = section_node == nullptr
                                  ? nullptr
                                  : section_node->as_table();
    _section = section_repr;
    return _section != nullptr;
}

void Config::write() const noexcept {
    std::ofstream config_file(_config.source().path.get()->c_str());
    if (!config_file.is_open()) {
        return;
    }
    config_file << toml::toml_formatter{_config, toml::format_flags::none};
    config_file.close();
}

std::string Config::to_lowercase(std::string str) const noexcept {
    std::ranges::transform(str, std::begin(str),
                           [](auto &sym) {
                               return std::tolower(sym);
                           });
    return str;
}

toml::node *Config::node(const std::string &key) const noexcept {
    return _section == nullptr
               ? nullptr
               : _section->get(to_lowercase(key));
}
