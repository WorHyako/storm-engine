#include "Filesystem/Config/Config.hpp"

using namespace Storm::Filesystem;

Config Config::load(std::filesystem::path file_path) noexcept {
    Config config;
    config._config_path = std::move(file_path);
    try {
        config._config = toml::parse_file(config._config_path.string());
    } catch (const toml::parse_error &err) {
    }
    return config;
}

void Config::write() const noexcept {
    std::ofstream config_file(_config_path);
    if (!config_file.is_open()) {
        return;
    }
    config_file << _config;
    config_file.close();
}

toml::table *Config::section(const std::string &section) {
    const auto section_node = _config.get(section);
    if (section_node == nullptr) {
        return {};
    }
    const auto section_repr = section_node->as_table();
    if (section_repr == nullptr) {
        return {};
    }
    return section_repr;
}
