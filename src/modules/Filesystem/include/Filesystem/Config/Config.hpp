#pragma once

#include <filesystem>

#include "Math/Types/Vector2.hpp"
#include "Math/Types/Vector3.hpp"
#include "Math/Types/Vector4.hpp"

#include <toml++/toml.hpp>

namespace Storm::Filesystem {
    class Config final {
    public:
        static Config load(const std::filesystem::path &file_path) noexcept;

        [[nodiscard]]
        bool selectSection(const std::string &section_name) noexcept;

    private:
        [[nodiscard]]
        std::string to_lowercase(std::string str) const noexcept;

        /**
         * @brief   Ctor.
         */
        Config() noexcept;

        void write() const noexcept;

        toml::parse_result _config;

        toml::table *_section;

#pragma region Accessors/Mutators

    public:
        template<class ValueType>
        [[nodiscard]]
        std::optional<ValueType> get(const std::string &key) const noexcept;

        template<class ValueType>
        [[nodiscard]]
        ValueType get(const std::string &key, ValueType default_value) const noexcept;

        template<class Type>
        void set(const std::string &key, Type value) noexcept;

        template<class ValueType>
        [[nodiscard]]
        std::vector<ValueType> get_array(const std::string &key) const noexcept;

        template<class ValueType>
        [[nodiscard]]
        std::vector<std::vector<ValueType> > get_matrix(const std::string &key) const noexcept;

        template<typename ValueType>
        [[nodiscard]]
        std::optional<Math::Types::Vector4<ValueType> > get_vector4(const std::string &key) const noexcept;

        template<typename ValueType>
        [[nodiscard]]
        std::optional<Math::Types::Vector3<ValueType> > get_vector3(const std::string &key) const noexcept;

        template<typename ValueType>
        [[nodiscard]]
        std::optional<Math::Types::Vector2<ValueType> > get_vector2(const std::string &key) const noexcept;

        template<typename ValueType>
        [[nodiscard]]
        Math::Types::Vector4<ValueType> get_vector4(const std::string &key,
                                                    Math::Types::Vector4<ValueType> default_value) const noexcept;

        template<typename ValueType>
        [[nodiscard]]
        Math::Types::Vector3<ValueType> get_vector3(const std::string &key,
                                                    Math::Types::Vector3<ValueType> default_value) const noexcept;

        template<typename ValueType>
        [[nodiscard]]
        Math::Types::Vector2<ValueType> get_vector2(const std::string &key,
                                                    Math::Types::Vector2<ValueType> default_value) const noexcept;

        template<typename ValueType>
        [[nodiscard]]
        std::optional<Math::Types::Vector2<ValueType> >
        get_vector2_from(const std::string &key, std::int16_t idx) const noexcept;

    private:
        [[nodiscard]]
        toml::node *node(const std::string &key) const noexcept;

        [[nodiscard]]
        toml::table *section(const std::string &section_name);

#pragma endregion Accessors/Mutators
    };

#pragma region Accessors/Mutators

    template<typename ValueType>
    std::vector<ValueType> Config::get_array(const std::string &key) const noexcept {
        auto config_node = node(key);
        if (config_node == nullptr) {
            std::printf("Can't find key [%s]\n", key.c_str());
            return {};
        }
        std::vector<ValueType> result;
        result.reserve(std::size(*config_node->as_array()));
        for (auto &&element: *config_node->as_array()) {
            toml::impl::wrap_node<ValueType> *value = element.as<ValueType>();
            if (value == nullptr) {
                std::printf(
                    "Warning:\n\tConfig - [%s]\n\tKey [%s] has different type - [%s].\n\tRequested type - [%s]\n",
                    _config.source().path.get()->c_str(),
                    key.c_str(),
                    toml::impl::node_type_friendly_names[static_cast<int>(value->type())].data(),
                    typeid(ValueType).name());
                return {};
            }
            result.push_back(static_cast<ValueType>(*value));
        }

        return result;
    }

    template<typename ValueType>
    std::vector<std::vector<ValueType> > Config::get_matrix(const std::string &key) const noexcept {
        auto config_node = node(key);
        if (config_node == nullptr || !config_node->is_array()) {
            std::printf("Can't find key [%s]\n", key.c_str());
            return {};
        }
        std::vector<std::vector<ValueType> > result{};
        for (auto &&row: *config_node->as_array()) {
            if (row.is_array()) {
                std::vector<ValueType> column_array{};
                for (auto &&column: *row.as_array()) {
                    auto column_value = column.as<ValueType>();
                    ValueType val_to_emplace = column_value == nullptr
                                                   ? ValueType()
                                                   : static_cast<ValueType>(*column_value);
                    column_array.emplace_back(val_to_emplace);
                }
                result.emplace_back(column_array);
            } else {
                auto row_value = row.as<ValueType>();
                ValueType val_to_emplace = row_value == nullptr
                                               ? ValueType()
                                               : static_cast<ValueType>(*row_value);
                result.template emplace_back<std::vector<ValueType> >({val_to_emplace});
            }
        }

        return result;
    }

    template<typename ValueType>
    std::optional<Math::Types::Vector4<ValueType> > Config::get_vector4(const std::string &key) const noexcept {
        auto array = get_array<ValueType>(key);
        if (array.empty() || std::size(array) != 4) {
            return {};
        }
        return Math::Types::Vector4<ValueType>(array[0], array[1], array[2], array[3]);
    }

    template<typename ValueType>
    std::optional<Math::Types::Vector3<ValueType> > Config::get_vector3(const std::string &key) const noexcept {
        auto array = get_array<ValueType>(key);
        if (array.empty() || std::size(array) != 3) {
            return {};
        }
        return Math::Types::Vector3<ValueType>(array[0], array[1], array[2]);
    }

    template<typename ValueType>
    std::optional<Math::Types::Vector2<ValueType> > Config::get_vector2(const std::string &key) const noexcept {
        auto array = get_array<ValueType>(key);
        if (array.empty() || std::size(array) != 2) {
            return {};
        }
        return Math::Types::Vector2<ValueType>(array[0], array[1]);
    }

    template<typename ValueType>
    Math::Types::Vector4<ValueType> Config::get_vector4(const std::string &key,
                                                        Math::Types::Vector4<ValueType> default_value) const noexcept {
        return get_vector4<ValueType>(key).has_value()
                   ? get_vector4<ValueType>(key).value()
                   : default_value;
    }

    template<typename ValueType>
    Math::Types::Vector3<ValueType> Config::get_vector3(const std::string &key,
                                                        Math::Types::Vector3<ValueType> default_value) const noexcept {
        return get_vector3<ValueType>(key).has_value()
                   ? get_vector3<ValueType>(key).value()
                   : default_value;
    }

    template<typename ValueType>
    Math::Types::Vector2<ValueType> Config::get_vector2(const std::string &key,
                                                        Math::Types::Vector2<ValueType> default_value) const noexcept {
        return get_vector2<ValueType>(key).has_value()
                   ? get_vector2<ValueType>(key).value()
                   : default_value;
    }

    template<typename ValueType>
    std::optional<Math::Types::Vector2<ValueType> > Config::get_vector2_from(const std::string &key,
                                                                             std::int16_t idx) const noexcept {
        auto matrix = get_matrix<ValueType>(key);
        if (std::size(matrix) < idx + 1 || std::size(matrix[idx]) != 2) {
            return {};
        }
        return Math::Types::Vector2<ValueType>(matrix[idx][0], matrix[idx][1]);
    }

    template<class ValueType>
    std::optional<ValueType> Config::get(const std::string &key) const noexcept {
        auto config_node = node(key);
        if (config_node == nullptr) {
            std::printf("Warning:\n\tCan't find key [%s]\n", key.c_str());
            return {};
        }
        auto value = config_node->value<ValueType>();
        if (!value.has_value()) {
            std::printf(
                "Warning:\n\tConfig [%s]\n\tKey [%s] has different type - [%s].\n\tRequested type - [%s]\n",
                _config.source().path.get()->c_str(),
                key.c_str(),
                toml::impl::node_type_friendly_names[static_cast<int>(config_node->type())].data(),
                typeid(ValueType).name());
        }
        return value;
    }

    template<class ValueType>
    ValueType Config::get(const std::string &key, ValueType default_value) const noexcept {
        return get<ValueType>(key).has_value()
                   ? get<ValueType>(key).value()
                   : default_value;
    }

    template<class ValueType>
    void Config::set(const std::string &key, ValueType value) noexcept {
        if (_section == nullptr) {
            return;
        }
        auto it = _section->insert_or_assign(to_lowercase(key), value);
        if (const auto result = it.first != _section->end()) {
            write();
        }
    }

#pragma endregion Accessors/Mutators
}
