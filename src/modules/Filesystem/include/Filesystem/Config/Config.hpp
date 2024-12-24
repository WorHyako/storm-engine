#pragma once

#include <filesystem>

#include "Math/Types/Vector2.hpp"
#include "Math/Types/Vector3.hpp"
#include "Math/Types/Vector4.hpp"

#include <toml++/toml.hpp>

namespace Storm::Filesystem {
    class Config final {
    public:
        [[nodiscard]]
        static Config Load(const std::filesystem::path &file_path) noexcept;

        [[nodiscard]]
        bool SelectSection(std::string section_name) noexcept;

    private:
        using SetResultIterator = std::pair<toml::table_iterator, bool>;

        [[nodiscard]]
        std::string ToLowercase(std::string str) const noexcept;

        [[nodiscard]]
        std::string PrintInfo(const std::string_view &key, const std::string_view &message) const noexcept;

        /**
         * @brief   Ctor.
         */
        Config() noexcept;

        void Write() const noexcept;

        toml::parse_result _config;

        std::string _section_name;

        toml::table *_section;

#pragma region Accessors/Mutators

    public:
        [[nodiscard]]
        std::vector<std::string> Sections() noexcept;

        template<class ValueType>
        [[nodiscard]]
        ValueType Get(const std::string &key, ValueType default_value) const noexcept;

        template<class ValueType>
        [[nodiscard]]
        std::optional<ValueType> Get(const std::string &key) const noexcept;

        template<class ValueType>
        void Set(const std::string &key, ValueType value) noexcept;

        template<class ValueType>
        void write_vector4(const std::string &key, Math::Types::Vector4<ValueType> value) noexcept;

        template<class ValueType>
        void write_vector3(const std::string &key, Math::Types::Vector3<ValueType> value) noexcept;

    private:
        template<class ElementType>
        [[nodiscard]]
        std::vector<std::vector<ElementType> > Matrix(toml::node *node) const noexcept;

        template<typename ValueType>
        [[nodiscard]]
        std::optional<ValueType> SingleValue(toml::node *node) const;

        template<typename ValueType>
        [[nodiscard]]
        bool SingleValue(const std::string &key, ValueType value) const;

        template<typename ElementType>
        [[nodiscard]]
        std::vector<ElementType> ArrayValue(toml::node *node) const;

        template<typename ElementType>
        [[nodiscard]]
        std::vector<Math::Types::Vector2<ElementType>> ArrayVector2Value(toml::node *node) const;

        template<typename ElementType>
        [[nodiscard]]
        std::vector<Math::Types::Vector3<ElementType>> ArrayVector3Value(toml::node *node) const;

        template<typename ElementType>
        [[nodiscard]]
        std::optional<Math::Types::Vector2<ElementType> > Vector2Value(toml::node *node) const;

        template<typename ElementType>
        [[nodiscard]]
        bool Vector2Value(const std::string &key, Math::Types::Vector2<ElementType> value) const;

        template<typename ElementType>
        [[nodiscard]]
        std::optional<Math::Types::Vector3<ElementType> > Vector3Value(toml::node *node) const;

        template<typename ElementType>
        [[nodiscard]]
        std::optional<Math::Types::Vector4<ElementType> > Vector4Value(toml::node *node) const;

        [[nodiscard]]
        toml::node *Node(const std::string &key) const noexcept;

        [[nodiscard]]
        toml::table *section(const std::string &section_name);

#pragma endregion Accessors/Mutators
    };

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
                    auto column_value = column.as<ElementType>();
                    if (column_value == nullptr) {
                        continue;
                    }
                    auto val_to_emplace = static_cast<ElementType>(*column_value);
                    column_array.emplace_back(val_to_emplace);
                }
                result.emplace_back(column_array);
            } else {
                auto row_value = row.as<ElementType>();
                if (row_value == nullptr) {
                    continue;
                }
                auto val_to_emplace = static_cast<ElementType>(*row_value);
                result.template emplace_back<std::vector<ElementType> >({val_to_emplace});
            }
        }

        return result;
    }

    template<typename ValueType>
    std::optional<ValueType> Config::SingleValue(toml::node *node) const {
        return node != nullptr && node->value<ValueType>().has_value()
                   ? node->value<ValueType>()
                   : std::nullopt;
    }

    template<typename ElementType>
    std::optional<Math::Types::Vector2<ElementType> > Config::Vector2Value(toml::node *node) const {
        const auto array_value = ArrayValue<ElementType>(node);
        auto result = array_value.empty() || std::size(array_value) != 2
                          ? std::nullopt
                          : std::optional{Math::Types::Vector2<ElementType>(array_value[0], array_value[1])};
        return result;
    }

    template<typename ElementType>
    std::optional<Math::Types::Vector3<ElementType> > Config::Vector3Value(toml::node *node) const {
        auto array_value = ArrayValue<ElementType>(node);
        auto result = array_value.empty() || std::size(array_value) != 3
                          ? std::nullopt
                          : std::optional{
                              Math::Types::Vector3<ElementType>(array_value[0], array_value[1], array_value[2])
                          };
        return result;
    }

    template<typename ElementType>
    std::optional<Math::Types::Vector4<ElementType> > Config::Vector4Value(toml::node *node) const {
        const auto array_value = ArrayValue<ElementType>(node);
        auto result = array_value.empty() || std::size(array_value) != 4
                          ? std::nullopt
                          : std::optional{
                              Math::Types::Vector4<ElementType>(array_value[0], array_value[1],
                                                                array_value[2], array_value[3])
                          };
        return result;
    }

    template<typename ElementType>
    std::vector<ElementType> Config::ArrayValue(toml::node *node) const {
        if (node->as_array() == nullptr) {
            return {};
        }
        std::vector<ElementType> result;
        result.reserve(std::size(*node->as_array()));
        for (auto &&element: *node->as_array()) {
            toml::impl::wrap_node<ElementType> *value = element.as<ElementType>();
            if (value == nullptr) {
                return {};
            }
            result.push_back(static_cast<ElementType>(*value));
        }

        return result;
    }

    template<typename ElementType>
    std::vector<Math::Types::Vector2<ElementType>> Config::ArrayVector2Value(toml::node *node) const {
        std::vector<Math::Types::Vector2<ElementType> > result{};
        const auto matrix = Matrix<ElementType>(node);
        if (matrix.empty()) {
            return {};
        }

        for (const std::vector<ElementType> &element: matrix) {
            if (std::size(element) != 2) {
                return {};
            }
            result.template emplace_back<Math::Types::Vector2<ElementType>>({element[0], element[1]});
        }
        return result;
    }

    template<typename ElementType>
    std::vector<Math::Types::Vector3<ElementType>> Config::ArrayVector3Value(toml::node *node) const {
        std::vector<Math::Types::Vector3<ElementType> > result{};
        const auto matrix = Matrix<ElementType>(node);
        if (matrix.empty()) {
            return {};
        }

        for (const std::vector<ElementType> &element: matrix) {
            if (std::size(element) != 3) {
                return {};
            }
            result.template emplace_back<Math::Types::Vector3<ElementType>>({element[0], element[1], element[2]});
        }
        return result;
    }

    template<class ValueType>
    std::optional<ValueType> Config::Get(const std::string &key) const noexcept {
        const auto config_node = Node(key);
        if (config_node == nullptr) {
            std::printf("\nError: \n%s", PrintInfo(key, "Can't find key").c_str());
            return {};
        }

        std::optional<ValueType> result;

        if constexpr (std::is_same_v<ValueType, std::int64_t>
                      || std::is_same_v<ValueType, double>
                      || std::is_same_v<ValueType, std::string>) {
            result = SingleValue<ValueType>(config_node);
        } else if constexpr (std::is_same_v<ValueType, std::vector<std::int64_t> >
                             || std::is_same_v<ValueType, std::vector<double> >
                             || std::is_same_v<ValueType, std::vector<std::string> >) {
            result = ArrayValue<typename ValueType::value_type>(config_node);

        } else if constexpr (std::is_same_v<ValueType, std::vector<Math::Types::Vector2<std::int64_t> > >) {
            result = ArrayVector2Value<std::int64_t>(config_node);
        } else if constexpr (std::is_same_v<ValueType, std::vector<Math::Types::Vector2<double> > >) {
            result = ArrayVector2Value<double>(config_node);
        } else if constexpr (std::is_same_v<ValueType, std::vector<Math::Types::Vector2<std::string> > >) {
            result = ArrayVector2Value<std::string>(config_node);

        } else if constexpr (std::is_same_v<ValueType, std::vector<Math::Types::Vector3<std::int64_t> > >) {
            result = ArrayVector3Value<std::int64_t>(config_node);
        } else if constexpr (std::is_same_v<ValueType, std::vector<Math::Types::Vector3<double> > >) {
            result = ArrayVector3Value<double>(config_node);
        } else if constexpr (std::is_same_v<ValueType, std::vector<Math::Types::Vector3<std::string> > >) {
            result = ArrayVector3Value<std::string>(config_node);

        } else if constexpr (std::is_same_v<ValueType, std::vector<std::vector<std::string> > >) {
            result = Matrix<std::string>(config_node);

        } else if constexpr (std::is_same_v<ValueType, Math::Types::Vector2<std::int64_t> >) {
            result = Vector2Value<std::int64_t>(config_node);
        } else if constexpr (std::is_same_v<ValueType, Math::Types::Vector2<double> >) {
            result = Vector2Value<double>(config_node);
        } else if constexpr (std::is_same_v<ValueType, Math::Types::Vector2<std::string> >) {
            result = Vector2Value<std::string>(config_node);

        } else if constexpr (std::is_same_v<ValueType, Math::Types::Vector3<std::int64_t> >) {
            result = Vector3Value<std::int64_t>(config_node);
        } else if constexpr (std::is_same_v<ValueType, Math::Types::Vector3<double> >) {
            result = Vector3Value<double>(config_node);
        } else if constexpr (std::is_same_v<ValueType, Math::Types::Vector3<std::string> >) {
            result = Vector3Value<std::string>(config_node);

        } else if constexpr (std::is_same_v<ValueType, Math::Types::Vector4<std::int64_t> >) {
            result = Vector4Value<std::int64_t>(config_node);
        } else if constexpr (std::is_same_v<ValueType, Math::Types::Vector4<double> >) {
            result = Vector4Value<double>(config_node);
        } else if constexpr (std::is_same_v<ValueType, Math::Types::Vector4<std::string> >) {
            result = Vector4Value<std::string>(config_node);
        }
        if (!result.has_value()) {
            std::printf("\nError: \n%s", PrintInfo(key, "Error parsing value").c_str());
        }
        return result;
    }

    template<class ValueType>
    ValueType Config::Get(const std::string &key, ValueType default_value) const noexcept {
        std::optional res{Get<ValueType>(key)};
        return res.has_value()
                   ? res.value()
                   : default_value;
    }

    template<class ValueType>
    void Config::Set(const std::string &key, ValueType value) noexcept {
        if (_section == nullptr) {
            return;
        }
        bool result = false;
        if constexpr (std::is_same_v<ValueType, std::int64_t>
                      || std::is_same_v<ValueType, double>
                      || std::is_same_v<ValueType, std::string>) {
            result = SingleValue(ToLowercase(key), value);
        } else if constexpr (std::is_same_v<ValueType, Math::Types::Vector2<std::int64_t> >) {
            result = Vector2Value<std::int64_t>(ToLowercase(key), value);
        } else if constexpr (std::is_same_v<ValueType, Math::Types::Vector2<double> >) {
            result = Vector2Value<double>(ToLowercase(key), value);
        } else if constexpr (std::is_same_v<ValueType, Math::Types::Vector2<std::string> >) {
            result = Vector2Value<std::string>(ToLowercase(key), value);
        }
        if (result) {
            Write();
        }
    }

    template<typename ValueType>
    [[nodiscard]]
    bool Config::SingleValue(const std::string &key, ValueType value) const {
        const auto&& node = Node(key);
        toml::impl::wrap_node<ValueType> *wrap_node = nullptr;
        if (node == nullptr) {
            const SetResultIterator it = _section->insert(key, ValueType());
            wrap_node = it.first->second.as<ValueType>();
        } else {
            wrap_node = node->as<ValueType>();
        }

        if (wrap_node == nullptr) {
            return false;
        }
        *wrap_node = std::move(value);
        return true;
    }

    template<typename ElementType>
    [[nodiscard]]
    bool Config::Vector2Value(const std::string &key, Math::Types::Vector2<ElementType> value) const {
        const auto&& node = Node(key);
        toml::impl::wrap_node<toml::array> *wrap_node = nullptr;
        if (node == nullptr) {
            const SetResultIterator it = _section->insert(key, toml::array());
            wrap_node = it.first->second.as<toml::array>();
        } else {
            wrap_node = node->as<toml::array>();
        }

        if (wrap_node == nullptr) {
            return false;
        }

        wrap_node->clear();
        wrap_node->emplace_back<ElementType>(std::move(value.x));
        wrap_node->emplace_back<ElementType>(std::move(value.y));
        return true;
    }

    template<class ValueType>
    void Config::write_vector4(const std::string &key, Math::Types::Vector4<ValueType> value) noexcept {
        auto config_node = Node(key);
        if (config_node == nullptr || config_node->as_array() == nullptr) {
            return;
        }
        auto &&array = _section->get_as<toml::array>(key);
        array->replace(array->cbegin(), value.x);
        array->replace(array->cbegin() + 1, value.y);
        array->replace(array->cbegin() + 2, value.z);
        array->replace(array->cbegin() + 3, value.w);
        Write();
    }

    template<class ValueType>
    void Config::write_vector3(const std::string &key, Math::Types::Vector3<ValueType> value) noexcept {
        auto config_node = Node(key);
        if (config_node == nullptr || config_node->as_array() == nullptr) {
            return;
        }
        auto &&array = _section->get_as<toml::array>(key);
        array->replace(array->cbegin(), value.x);
        array->replace(array->cbegin() + 1, value.y);
        array->replace(array->cbegin() + 2, value.z);
        Write();
    }

#pragma endregion Accessors/Mutators
}
