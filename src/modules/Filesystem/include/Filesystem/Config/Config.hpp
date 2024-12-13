#pragma once

#include <filesystem>

#include <toml++/toml.hpp>

namespace Storm::Filesystem {
    class Config final {
    public:
        static Config load(std::filesystem::path file_path) noexcept;

        template<class T>
        [[nodiscard]]
        std::optional<T> get(const std::string &section_name, const std::string &key) noexcept;

        template<class T>
        [[nodiscard]]
        T get(const std::string &section_name, const std::string &key, T default_value) noexcept;

        template<class T>
        [[nodiscard]]
        bool set(const std::string &section_name, const std::string &key, T value) noexcept;

        template<class T>
        [[nodiscard]]
        std::optional<std::vector<T> > getArray(const std::string &section_name, const std::string &key) noexcept;

    private:
        /**
         * @brief   Ctor.
         */
        Config() = default;

        void write() const noexcept;

        [[nodiscard]]
        toml::table *section(const std::string &section_name);

        toml::parse_result _config;

        std::filesystem::path _config_path;
    };

    template<class T>
    std::optional<std::vector<T> > Config::getArray(const std::string &section_name, const std::string &key) noexcept {
        const auto section_node = section(section_name);
        if (section_node == nullptr) {
            return {};
        }
        toml::array* array_node = section_node->get(key)->as_array();
        if (array_node == nullptr) {
            return {};
        }
        std::vector<T> result;
        result.reserve(std::size(*array_node));
        for (auto &&element: *array_node) {
            toml::impl::wrap_node<T> *value = element.as<T>();
            if (value == nullptr) {
                return {};
            }
            result.push_back(static_cast<T>(value->get()));
        }

        return result;
    }

    template<class T>
    std::optional<T> Config::get(const std::string &section_name, const std::string &key) noexcept {
        const auto section_node = section(section_name);
        if (section_node == nullptr) {
            return {};
        }
        const auto value_node = section_node->get(key);
        return value_node == nullptr ? std::nullopt : value_node->value<T>();
    }

    template<class T>
    T Config::get(const std::string &section_name, const std::string &key, T default_value) noexcept {
        const auto section_node = section(section_name);
        if (section_node == nullptr) {
            return default_value;
        }
        const auto value_node = section_node->get(key);
        if (value_node == nullptr) {
            return default_value;
        }
        auto value = value_node->value<T>();
        return value == std::nullopt ? default_value : value_node->value<T>().value();
    }

    template<class T>
    bool Config::set(const std::string &section_name, const std::string &key, T value) noexcept {
        auto section_node = section(section_name);
        if (section_node == nullptr) {
            return false;
        }
        auto it = section_node->insert_or_assign(key, value);
        const auto result = it.first != section_node->end();
        if (result) {
            write();
        }
        return result;
    }
}
