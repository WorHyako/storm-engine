#pragma once

#include "ConfigConcept.hpp"

#include <filesystem>

#include <toml++/toml.hpp>

namespace Storm::Filesystem {
    /**
     * \brief
     *
     * \author  WorHyako
     */
    class Config final {
    public:
        /**
         * \brief
         *
         * \param   file_path   Config file's name.
         *
         * \return  Config or empty config.
         */
        [[nodiscard]]
        static Config Load(const std::filesystem::path &file_path) noexcept;

        /**
         * \brief   Select section in toml config for future accessors and mutators.
         *
         * \param   section_name    Section name.
         *
         * \return  <code>true</code> - Section was found and selected.
         *          \n
         *          <code>false</code> - Section was not found.
         */
        [[nodiscard]]
        bool SelectSection(const std::string_view &section_name) noexcept;

        template<typename ValueType>
        [[nodiscard]]
        static ValueType GetOrGet(std::pair<const Config &, const Config &> configs,
                                  const std::string_view &key, ValueType default_value) noexcept;

        template<typename ValueType>
        [[nodiscard]]
        static std::optional<ValueType> GetOrGet(std::pair<const Config &, const Config &> configs,
                                                 const std::string_view &key) noexcept;

    private:
        /**
         * \brief   Makes lowercase representation of in-string with no modification origin one.
         *
         * \param   str String to modify.
         *              \n
         *              Case independent.
         *
         * \return  Lowercase string representation.
         */
        [[nodiscard]]
        std::string ToLowercase(const std::string_view &str) const noexcept;

        [[nodiscard]]
        std::string PrintInfo(const std::string_view &key, const std::string_view &message) const noexcept;

        /**
         * \brief   Ctor.
         */
        Config() noexcept;

        void Write() const noexcept;

        toml::parse_result _config;

        /**
         * \brief   Name of current section.
         */
        std::string _section_name;

        /**
         * \brief   Pointer to selected section.
         */
        toml::table *_section;

#pragma region Accessors/Mutators

    public:
        /**
         * \brief   Checks config for empty data.
         *
         * \return  <code>true</code> - Config contains parsed data.
         *          \n
         *          <code>false</code> - Config is empty. Parsing error.
         */
        [[nodiscard]]
        bool Empty() const noexcept;

        /**
         * \brief   Accessor for config file's name.
         *
         * \return  Config file's name.
         *          \n
         *          Empty string if config is empty.
         */
        [[nodiscard]]
        std::string Name() const noexcept;

        /**
         * \brief   Accessor for all config's sections.
         *
         * \return  Section's name list.
         */
        [[nodiscard]]
        std::vector<std::string> Sections() noexcept;

        template<class ValueType>
        [[nodiscard]]
        ValueType Get(const std::string_view &key, ValueType default_value) const noexcept;

        template<class ValueType>
        [[nodiscard]]
        std::optional<ValueType> Get(const std::string_view &key) const noexcept;

        template<class ValueType>
        void Set(const std::string_view &key, ValueType value) noexcept;

    private:
        template<class ElementType>
        [[nodiscard]]
        std::vector<std::vector<ElementType> > Matrix(toml::node *node) const noexcept;

        template<typename ValueType>
        [[nodiscard]]
        std::optional<ValueType> SingleValue(toml::node *node) const;

        template<typename ValueType>
        [[nodiscard]]
        bool SingleValue(const std::string_view &key, ValueType value) const;

        template<typename ElementType>
        [[nodiscard]]
        std::vector<ElementType> ArrayValue(toml::node *node) const;

        template<typename ElementType>
        [[nodiscard]]
        std::vector<Math::Types::Vector2<ElementType> > ArrayVector2Value(toml::node *node) const;

        template<typename ElementType>
        [[nodiscard]]
        std::vector<Math::Types::Vector3<ElementType> > ArrayVector3Value(toml::node *node) const;

        template<typename ElementType>
        [[nodiscard]]
        std::optional<Math::Types::Vector2<ElementType> > Vector2Value(toml::node *node) const;

        template<typename ElementType>
        [[nodiscard]]
        bool Vector2Value(const std::string_view &key, Math::Types::Vector2<ElementType> value) const;

        template<typename ElementType>
        [[nodiscard]]
        bool Vector3Value(const std::string_view &key, Math::Types::Vector3<ElementType> value) const;

        template<typename ElementType>
        [[nodiscard]]
        std::optional<Math::Types::Vector3<ElementType> > Vector3Value(toml::node *node) const;

        template<typename ElementType>
        [[nodiscard]]
        bool Vector4Value(const std::string_view &key, Math::Types::Vector4<ElementType> value) const;

        template<typename ElementType>
        [[nodiscard]]
        std::optional<Math::Types::Vector4<ElementType> > Vector4Value(toml::node *node) const;

        /**
         * \brief   Searches value by <code>key</code> in current section.
         *
         * \param   key     Option key.
         *
         * \return  Pointer to config's node by <code>key</code>.
         *          \n
         *          <code>nullptr</code> - key wasn't found.
         */
        [[nodiscard]]
        toml::node *Node(const std::string_view &key) const noexcept;

#pragma endregion Accessors/Mutators
    };

    template<typename ValueType>
    ValueType Config::GetOrGet(std::pair<const Config &, const Config &> configs,
                               const std::string_view &key, ValueType default_value) noexcept {
        if (configs.first.Empty() && configs.second.Empty()) {
            return default_value;
        }

        auto first_value{configs.first.Get<ValueType>(key)};
        if (first_value.has_value()) {
            return first_value.value();
        }
        return configs.second.Get<ValueType>(key, default_value);
    }

    template<typename ValueType>
    std::optional<ValueType> Config::GetOrGet(std::pair<const Config &, const Config &> configs,
                                              const std::string_view &key) noexcept {
        if (configs.first.Empty() && configs.second.Empty()) {
            return {};
        }

        auto first_value{configs.first.Get<ValueType>(key)};
        return first_value.has_value()
                   ? first_value
                   : configs.second.Get<ValueType>(key);
    }

#pragma region Accessors/Mutators

    template<typename ElementType>
    std::vector<std::vector<ElementType> > Config::Matrix(toml::node *node) const noexcept {
        if (node == nullptr || !node->is_array()) {
            return {};
        }
        std::vector<std::vector<ElementType> > result{};
        for (auto &&row: *node->as_array()) {
            if (row.is_array()) {
                std::vector<ElementType> column_array{};
                for (auto &&column: *row.as_array()) {
                    auto column_value{column.as<ElementType>()};
                    if (column_value == nullptr) {
                        continue;
                    }
                    auto val_to_emplace{static_cast<ElementType>(*column_value)};
                    column_array.emplace_back(val_to_emplace);
                }
                result.emplace_back(column_array);
            } else {
                auto row_value{row.as<ElementType>()};
                if (row_value == nullptr) {
                    continue;
                }
                auto val_to_emplace{static_cast<ElementType>(*row_value)};
                result.template emplace_back<std::vector<ElementType> >({val_to_emplace});
            }
        }

        return result;
    }

    template<class ValueType>
    std::optional<ValueType> Config::Get(const std::string_view &key) const noexcept {
        const auto config_node{Node(key)};
        if (config_node == nullptr) {
            return {};
        }

        std::optional<ValueType> result;
        if constexpr (is_single<ValueType>) {
            result = SingleValue<ValueType>(config_node);
        } else if constexpr (is_array<ValueType>) {
            result = ArrayValue<typename ValueType::value_type>(config_node);
        } else if constexpr (is_vector2_array<ValueType>) {
            result = ArrayVector2Value<typename ValueType::value_type::value_type>(config_node);
        } else if constexpr (is_vector3_array<ValueType>) {
            result = ArrayVector3Value<typename ValueType::value_type::value_type>(config_node);
        } else if constexpr (std::is_same_v<ValueType, std::vector<std::vector<std::string> > >) {
            result = Matrix<std::string>(config_node);
        } else if constexpr (is_vector2<ValueType>) {
            result = Vector2Value<typename ValueType::value_type>(config_node);
        } else if constexpr (is_vector3<ValueType>) {
            result = Vector3Value<typename ValueType::value_type>(config_node);
        } else if constexpr (is_vector4<ValueType>) {
            result = Vector4Value<typename ValueType::value_type>(config_node);
        }
        if (!result.has_value()) {
            std::printf("\nError: \n%s", PrintInfo(key, "Error parsing value").c_str());
        }
        return result;
    }

    template<class ValueType>
    ValueType Config::Get(const std::string_view &key, ValueType default_value) const noexcept {
        std::optional res{Get<ValueType>(key)};
        return res.has_value()
                   ? res.value()
                   : default_value;
    }

    template<class ValueType>
    void Config::Set(const std::string_view &key, ValueType value) noexcept {
        if (_section == nullptr) {
            return;
        }
        bool result{false};
        if constexpr (is_single<ValueType>) {
            result = SingleValue<ValueType>(ToLowercase(key), value);
        } else if constexpr (is_vector2<ValueType>) {
            result = Vector2Value<typename ValueType::value_type>(ToLowercase(key), value);
        } else if constexpr (is_vector3<ValueType>) {
            result = Vector3Value<typename ValueType::value_type>(ToLowercase(key), value);
        } else if constexpr (is_vector4<ValueType>) {
            result = Vector4Value<typename ValueType::value_type>(ToLowercase(key), value);
        }
        if (result) {
            Write();
        }
    }

#pragma endregion Accessors/Mutators
}
