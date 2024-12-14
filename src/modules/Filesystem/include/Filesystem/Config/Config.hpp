#pragma once

#include <filesystem>

#include <toml++/toml.hpp>

namespace Storm::Filesystem {
    class Config final {
    public:
        static Config load(std::filesystem::path file_path) noexcept;

        template<class T>
        [[nodiscard]]
        std::optional<T> get(std::string key) noexcept;

        template<class T>
        [[nodiscard]]
        T get(std::string key, T default_value) noexcept;

        template<class T>
        void set(std::string key, T value) noexcept;

        template<class T>
        [[nodiscard]]
        std::optional<std::vector<T> > getArray(std::string key) const noexcept;

        [[nodiscard]]
        bool selectSection(const std::string &section_name) noexcept;

    private:
        /**
         * @brief   Ctor.
         */
        Config() noexcept;

        void write() const noexcept;

        [[nodiscard]]
        toml::table *section(const std::string &section_name);

        toml::parse_result _config;

        toml::table* _section;

        std::filesystem::path _config_path;
    };

    template<class T>
    std::optional<std::vector<T> > Config::getArray(std::string key) const noexcept {
        if (_section == nullptr) {
            return {};
        }
        std::transform(std::begin(key), std::end(key), std::begin(key),
                       [](auto &sym) {
                           return std::tolower(sym);
                       });
        auto array_node = _section->get(key);
        if (array_node == nullptr) {
            // std::printf("Can't load [file][section][option] - [%s][%s][%s]\n",
                        // _config_path.string().c_str(), section_name.c_str(), key.c_str());
            return {};
        }
        std::vector<T> result;
        result.reserve(std::size(*array_node->as_array()));
        for (auto &&element: *array_node->as_array()) {
            toml::impl::wrap_node<T> *value = element.as<T>();
            if (value == nullptr) {
                return {};
            }
            result.push_back(static_cast<T>(*value));
        }

        return result;
    }

    template<class T>
    std::optional<T> Config::get(std::string key) noexcept {
        if (_section == nullptr) {
            return {};
        }
        std::transform(std::begin(key), std::end(key), std::begin(key),
                       [](auto &sym) {
                           return std::tolower(sym);
                       });
        const auto value_node = _section->get(key);
        if (value_node == nullptr) {
            // std::printf("Can't load [file][section][option] - [%s][%s][%s]\n",
                        // _config_path.string().c_str(), section_name.c_str(), key.c_str());
        }
        return value_node == nullptr
                   ? std::nullopt
                   : value_node->value<T>();
    }

    template<class T>
    T Config::get(std::string key, T default_value) noexcept {
        if (_section == nullptr) {
            return default_value;
        }
        std::transform(std::begin(key), std::end(key), std::begin(key),
                       [](auto &sym) {
                           return std::tolower(sym);
                       });
        const auto value_node = _section->get(key);
        if (value_node == nullptr) {
            // std::printf("Can't load [file][section][option] - [%s][%s][%s]\n",
                        // _config_path.string().c_str(), section_name.c_str(), key.c_str());
            return default_value;
        }
        auto value = value_node->value<T>();
        return value == std::nullopt
                   ? default_value
                   : value.value();
    }

    template<class T>
    void Config::set(std::string key, T value) noexcept {
        if (_section == nullptr) {
            return;
        }
        std::transform(std::begin(key), std::end(key), std::begin(key),
                       [](auto &sym) {
                           return std::tolower(sym);
                       });
        auto it = _section->insert_or_assign(key, value);
        if (const auto result = it.first != _section->end()) {
            write();
        }
    }
}
