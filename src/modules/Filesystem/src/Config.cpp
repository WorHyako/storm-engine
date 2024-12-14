#include "Filesystem/Config/Config.hpp"

using namespace Storm::Filesystem;

Config::Config() noexcept
    : _section(nullptr) {
}

Config Config::load(std::filesystem::path file_path) noexcept {
    Config config;
    config._config_path = std::move(file_path);
    if (!std::filesystem::exists(file_path)) {
        std::printf("Can't open config file - %s\n", config._config_path.string().c_str());
        return config;
    }
    try {
        config._config = toml::parse_file(config._config_path.string());
    } catch (const toml::parse_error &err) {
        std::printf("Can't parse config file - %s\n", config._config_path.string().c_str());
        std::printf("\t\tErorr - %s\n\t\t%s\n", err.what(), std::string(err.description()).c_str());
    }
    return config;
}

bool Config::selectSection(const std::string &section_name) noexcept {
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
    std::ofstream config_file(_config_path);
    if (!config_file.is_open()) {
        return;
    }
    config_file << _config;
    config_file.close();
}
