#include "Filesystem/Config/Config.hpp"

using namespace Storm::Filesystem;
using namespace Storm::Math;

Config::Config() noexcept
    : _section{nullptr} {
}

Config Config::Load(const std::filesystem::path &file_path) noexcept {
    Config config;
    if (!std::filesystem::exists(file_path)) {
        std::printf("Can't open config file - %s\n", file_path.string().c_str());
        return config;
    }
    try {
        config._config = toml::parse_file(file_path.string());
    } catch (const toml::parse_error &err) {
        std::printf("Can't parse config file - %s\n", file_path.string().c_str());
        std::printf("\tError - %s\n\t\t%s\n", err.what(), std::string(err.description()).c_str());
    }
    return std::move(config);
}

bool Config::SelectSection(const std::string_view &section_name) noexcept {
    if (_config.empty()) {
        return false;
    }
    _section_name = section_name;
    const auto section_node = _config.get(_section_name);
    const auto section_repr = section_node == nullptr
                                  ? nullptr
                                  : section_node->as_table();
    _section = section_repr;
    return _section != nullptr;
}

bool Config::Empty() const noexcept {
    return _config.empty();
}

std::string Config::Name() const noexcept {
    return Empty()
               ? std::string()
               : *_config.source().path;
}

std::vector<std::string> Config::Sections() noexcept {
    std::vector<std::string> sections{};
    sections.reserve(_config.size());
    for (const auto &[section_name, section]: _config) {
        sections.emplace_back(section_name);
    }
    return sections;
}

void Config::Write() const noexcept {
    std::ofstream config_file(_config.source().path.get()->c_str());
    if (!config_file.is_open()) {
        return;
    }
    config_file << toml::toml_formatter{_config, toml::format_flags::none};
    config_file.close();
}

std::string Config::ToLowercase(const std::string_view &str) const noexcept {
    std::string out{str};
    std::ranges::transform(out, std::begin(out),
                           [](auto symbol) {
                               return std::tolower(symbol);
                           });
    return out;
}

std::string Config::PrintInfo(const std::string_view &key, const std::string_view &message) const noexcept {
    std::stringstream ss;
    ss << "Info:";
    ss << "\n\tFile: " << _config.source().path.get()->c_str();
    ss << "\n\tSection: [" << _section_name << "]";
    if (_section == nullptr) {
        ss << " - not found";
    }
    ss << "\n\tKey: [" << key << "]";
    if (_section == nullptr || _section->get(key) == nullptr) {
        ss << " - not found";
    }
    ss << "\n\tError message: " << message;
    return ss.str();
}

toml::node *Config::Node(const std::string_view &key) const noexcept {
    toml::node *section = _section == nullptr
                              ? nullptr
                              : _section->get(ToLowercase(key));
    if (section == nullptr) {
        std::printf("\nError: \n%s", PrintInfo(key, "Section/key error").c_str());
    }
    return section;
}

#pragma region Specialization

template<>
std::optional<double> Config::SingleValue<double>(toml::node *node) const {
    return node != nullptr && node->value<double>().has_value()
               ? node->value<double>()
               : std::nullopt;
}

template<>
std::optional<std::string> Config::SingleValue<std::string>(toml::node *node) const {
    return node != nullptr && node->value<std::string>().has_value()
               ? node->value<std::string>()
               : std::nullopt;
}

template<>
std::optional<std::int64_t> Config::SingleValue<std::int64_t>(toml::node *node) const {
    return node != nullptr && node->value<std::int64_t>().has_value()
               ? node->value<std::int64_t>()
               : std::nullopt;
}

template<>
bool Config::SingleValue<double>(const std::string_view &key, double value) const {
    const auto &&node{Node(key)};
    toml::impl::wrap_node<double> *wrap_node{nullptr};
    if (node == nullptr) {
        const auto [it, _]{_section->insert(key, 0.0)};
        wrap_node = it->second.as<double>();
    } else {
        wrap_node = node->as<double>();
    }

    if (wrap_node == nullptr) {
        return false;
    }
    *wrap_node = value;
    return true;
}

template<>
bool Config::SingleValue<std::int64_t>(const std::string_view &key, std::int64_t value) const {
    const auto &&node{Node(key)};
    toml::impl::wrap_node<std::int64_t> *wrap_node{nullptr};
    if (node == nullptr) {
        const auto [it, _]{_section->insert(key, 0.0)};
        wrap_node = it->second.as<std::int64_t>();
    } else {
        wrap_node = node->as<std::int64_t>();
    }

    if (wrap_node == nullptr) {
        return false;
    }
    *wrap_node = value;
    return true;
}

template<>
bool Config::SingleValue<std::string>(const std::string_view &key, std::string value) const {
    const auto &&node{Node(key)};
    toml::impl::wrap_node<std::string> *wrap_node{nullptr};
    if (node == nullptr) {
        const auto [it, _]{_section->insert(key, std::string())};
        wrap_node = it->second.as<std::string>();
    } else {
        wrap_node = node->as<std::string>();
    }

    if (wrap_node == nullptr) {
        return false;
    }
    *wrap_node = std::move(value);
    return true;
}

template<>
std::vector<double> Config::ArrayValue(toml::node *node) const {
    if (node->as_array() == nullptr) {
        return {};
    }
    std::vector<double> result{};
    result.reserve(std::size(*node->as_array()));
    for (auto &&element: *node->as_array()) {
        toml::impl::wrap_node<double> *value{element.as<double>()};
        if (value == nullptr) {
            return {};
        }
        result.emplace_back(*value);
    }

    return result;
}

template<>
std::vector<std::int64_t> Config::ArrayValue(toml::node *node) const {
    if (node->as_array() == nullptr) {
        return {};
    }
    std::vector<std::int64_t> result{};
    result.reserve(std::size(*node->as_array()));
    for (auto &&element: *node->as_array()) {
        toml::impl::wrap_node<std::int64_t> *value{element.as<std::int64_t>()};
        if (value == nullptr) {
            return {};
        }
        result.emplace_back(*value);
    }

    return result;
}

template<>
std::vector<std::string> Config::ArrayValue(toml::node *node) const {
    if (node->as_array() == nullptr) {
        return {};
    }
    std::vector<std::string> result{};
    result.reserve(std::size(*node->as_array()));
    for (auto &&element: *node->as_array()) {
        toml::impl::wrap_node<std::string> *value{element.as<std::string>()};
        if (value == nullptr) {
            return {};
        }
        result.emplace_back(*value);
    }

    return result;
}

template<>
std::optional<Types::Vector2<double> > Config::Vector2Value(toml::node *node) const {
    const auto array_value{ArrayValue<double>(node)};
    auto result{
        array_value.empty() || std::size(array_value) != 2
            ? std::nullopt
            : std::optional{Types::Vector2(array_value[0], array_value[1])}
    };
    return result;
}

template<>
std::optional<Types::Vector2<std::int64_t> > Config::Vector2Value(toml::node *node) const {
    const auto array_value{ArrayValue<std::int64_t>(node)};
    auto result{
        array_value.empty() || std::size(array_value) != 2
            ? std::nullopt
            : std::optional{Types::Vector2(array_value[0], array_value[1])}
    };
    return result;
}

template<>
std::optional<Types::Vector2<std::string> > Config::Vector2Value(toml::node *node) const {
    const auto array_value{ArrayValue<std::string>(node)};
    auto result{
        array_value.empty() || std::size(array_value) != 2
            ? std::nullopt
            : std::optional{Types::Vector2(array_value[0], array_value[1])}
    };
    return result;
}

template<>
std::optional<Types::Vector3<std::int64_t> > Config::Vector3Value(toml::node *node) const {
    auto array_value{ArrayValue<std::int64_t>(node)};
    auto result{
        array_value.empty() || std::size(array_value) != 3
            ? std::nullopt
            : std::optional{
                Types::Vector3(array_value[0], array_value[1], array_value[2])
            }
    };
    return result;
}

template<>
std::optional<Types::Vector3<double> > Config::Vector3Value(toml::node *node) const {
    auto array_value{ArrayValue<double>(node)};
    auto result{
        array_value.empty() || std::size(array_value) != 3
            ? std::nullopt
            : std::optional{
                Types::Vector3(array_value[0], array_value[1], array_value[2])
            }
    };
    return result;
}

template<>
std::optional<Types::Vector3<std::string> > Config::Vector3Value(toml::node *node) const {
    auto array_value{ArrayValue<std::string>(node)};
    auto result{
        array_value.empty() || std::size(array_value) != 3
            ? std::nullopt
            : std::optional{
                Types::Vector3(array_value[0], array_value[1], array_value[2])
            }
    };
    return result;
}

template<>
std::optional<Types::Vector4<double> > Config::Vector4Value(toml::node *node) const {
    const auto array_value{ArrayValue<double>(node)};
    auto result{
        array_value.empty() || std::size(array_value) != 4
            ? std::nullopt
            : std::optional{
                Types::Vector4(array_value[0], array_value[1],
                               array_value[2], array_value[3])
            }
    };
    return result;
}

template<>
std::optional<Types::Vector4<std::int64_t> > Config::Vector4Value(toml::node *node) const {
    const auto array_value{ArrayValue<std::int64_t>(node)};
    auto result{
        array_value.empty() || std::size(array_value) != 4
            ? std::nullopt
            : std::optional{
                Types::Vector4(array_value[0], array_value[1],
                               array_value[2], array_value[3])
            }
    };
    return result;
}

template<>
std::optional<Types::Vector4<std::string> > Config::Vector4Value(toml::node *node) const {
    const auto array_value{ArrayValue<std::string>(node)};
    auto result{
        array_value.empty() || std::size(array_value) != 4
            ? std::nullopt
            : std::optional{
                Types::Vector4(array_value[0], array_value[1],
                               array_value[2], array_value[3])
            }
    };
    return result;
}

template<>
std::vector<Types::Vector2<double> > Config::ArrayVector2Value(toml::node *node) const {
    std::vector<Types::Vector2<double> > result{};
    const auto matrix{Matrix<double>(node)};
    if (matrix.empty()) {
        return {};
    }

    for (const std::vector<double> &element: matrix) {
        if (std::size(element) != 2) {
            return {};
        }
        result.emplace_back(element[0], element[1]);
    }
    return result;
}

template<>
std::vector<Types::Vector2<std::int64_t> > Config::ArrayVector2Value(toml::node *node) const {
    std::vector<Types::Vector2<std::int64_t> > result{};
    const auto matrix{Matrix<std::int64_t>(node)};
    if (matrix.empty()) {
        return {};
    }

    for (const std::vector<std::int64_t> &element: matrix) {
        if (std::size(element) != 2) {
            return {};
        }
        result.emplace_back(element[0], element[1]);
    }
    return result;
}

template<>
std::vector<Types::Vector2<std::string> > Config::ArrayVector2Value(toml::node *node) const {
    std::vector<Types::Vector2<std::string> > result{};
    const auto matrix{Matrix<std::string>(node)};
    if (matrix.empty()) {
        return {};
    }

    for (const std::vector<std::string> &element: matrix) {
        if (std::size(element) != 2) {
            return {};
        }
        result.emplace_back(element[0], element[1]);
    }
    return result;
}

template<>
std::vector<Types::Vector3<double> > Config::ArrayVector3Value(toml::node *node) const {
    std::vector<Types::Vector3<double> > result{};
    const auto matrix{Matrix<double>(node)};
    if (matrix.empty()) {
        return {};
    }

    for (const std::vector<double> &element: matrix) {
        if (std::size(element) != 3) {
            return {};
        }
        result.emplace_back(element[0], element[1], element[2]);
    }
    return result;
}

template<>
std::vector<Types::Vector3<std::int64_t> > Config::ArrayVector3Value(toml::node *node) const {
    std::vector<Types::Vector3<std::int64_t> > result{};
    const auto matrix{Matrix<std::int64_t>(node)};
    if (matrix.empty()) {
        return {};
    }

    for (const std::vector<std::int64_t> &element: matrix) {
        if (std::size(element) != 3) {
            return {};
        }
        result.emplace_back(element[0], element[1], element[2]);
    }
    return result;
}

template<>
std::vector<Types::Vector3<std::string> > Config::ArrayVector3Value(toml::node *node) const {
    std::vector<Types::Vector3<std::string> > result{};
    const auto matrix{Matrix<std::string>(node)};
    if (matrix.empty()) {
        return {};
    }

    for (const std::vector<std::string> &element: matrix) {
        if (std::size(element) != 3) {
            return {};
        }
        result.emplace_back(element[0], element[1], element[2]);
    }
    return result;
}

template<>
bool Config::Vector2Value(const std::string_view &key, Types::Vector2<double> value) const {
    const auto &&node{Node(key)};
    toml::impl::wrap_node<toml::array> *wrap_node{nullptr};
    if (node == nullptr) {
        const auto [it, _]{_section->insert(key, toml::array())};
        wrap_node = it->second.as<toml::array>();
    } else {
        wrap_node = node->as<toml::array>();
    }

    if (wrap_node == nullptr) {
        return false;
    }

    wrap_node->clear();
    wrap_node->emplace_back(value.x);
    wrap_node->emplace_back(value.y);
    return true;
}

template<>
bool Config::Vector2Value(const std::string_view &key, Types::Vector2<std::int64_t> value) const {
    const auto &&node{Node(key)};
    toml::impl::wrap_node<toml::array> *wrap_node{nullptr};
    if (node == nullptr) {
        const auto [it, _]{_section->insert(key, toml::array())};
        wrap_node = it->second.as<toml::array>();
    } else {
        wrap_node = node->as<toml::array>();
    }

    if (wrap_node == nullptr) {
        return false;
    }

    wrap_node->clear();
    wrap_node->emplace_back(value.x);
    wrap_node->emplace_back(value.y);
    return true;
}

template<>
bool Config::Vector2Value(const std::string_view &key, Types::Vector2<std::string> value) const {
    const auto &&node{Node(key)};
    toml::impl::wrap_node<toml::array> *wrap_node{nullptr};
    if (node == nullptr) {
        const auto [it, _]{_section->insert(key, toml::array())};
        wrap_node = it->second.as<toml::array>();
    } else {
        wrap_node = node->as<toml::array>();
    }

    if (wrap_node == nullptr) {
        return false;
    }

    wrap_node->clear();
    wrap_node->emplace_back(value.x);
    wrap_node->emplace_back(value.y);
    return true;
}

template<>
bool Config::Vector3Value(const std::string_view &key, Types::Vector3<double> value) const {
    const auto &&node{Node(key)};
    toml::impl::wrap_node<toml::array> *wrap_node{nullptr};
    if (node == nullptr) {
        const auto [it, _]{_section->insert(key, toml::array())};
        wrap_node = it->second.as<toml::array>();
    } else {
        wrap_node = node->as<toml::array>();
    }

    if (wrap_node == nullptr) {
        return false;
    }

    wrap_node->clear();
    wrap_node->emplace_back(value.x);
    wrap_node->emplace_back(value.y);
    wrap_node->emplace_back(value.z);
    return true;
}

template<>
bool Config::Vector3Value(const std::string_view &key, Types::Vector3<std::int64_t> value) const {
    const auto &&node{Node(key)};
    toml::impl::wrap_node<toml::array> *wrap_node{nullptr};
    if (node == nullptr) {
        const auto [it, _]{_section->insert(key, toml::array())};
        wrap_node = it->second.as<toml::array>();
    } else {
        wrap_node = node->as<toml::array>();
    }

    if (wrap_node == nullptr) {
        return false;
    }

    wrap_node->clear();
    wrap_node->emplace_back(value.x);
    wrap_node->emplace_back(value.y);
    wrap_node->emplace_back(value.z);
    return true;
}

template<>
bool Config::Vector3Value(const std::string_view &key, Types::Vector3<std::string> value) const {
    const auto &&node{Node(key)};
    toml::impl::wrap_node<toml::array> *wrap_node{nullptr};
    if (node == nullptr) {
        const auto [it, _]{_section->insert(key, toml::array())};
        wrap_node = it->second.as<toml::array>();
    } else {
        wrap_node = node->as<toml::array>();
    }

    if (wrap_node == nullptr) {
        return false;
    }

    wrap_node->clear();
    wrap_node->emplace_back(value.x);
    wrap_node->emplace_back(value.y);
    wrap_node->emplace_back(value.z);
    return true;
}

template<>
bool Config::Vector4Value(const std::string_view &key, Types::Vector4<double> value) const {
    const auto &&node{Node(key)};
    toml::impl::wrap_node<toml::array> *wrap_node{nullptr};
    if (node == nullptr) {
        const auto [it, _]{_section->insert(key, toml::array())};
        wrap_node = it->second.as<toml::array>();
    } else {
        wrap_node = node->as<toml::array>();
    }

    if (wrap_node == nullptr) {
        return false;
    }

    wrap_node->clear();
    wrap_node->emplace_back(value.x);
    wrap_node->emplace_back(value.y);
    wrap_node->emplace_back(value.z);
    wrap_node->emplace_back(value.w);
    return true;
}

template<>
bool Config::Vector4Value(const std::string_view &key, Types::Vector4<std::int64_t> value) const {
    const auto &&node{Node(key)};
    toml::impl::wrap_node<toml::array> *wrap_node{nullptr};
    if (node == nullptr) {
        const auto [it, _]{_section->insert(key, toml::array())};
        wrap_node = it->second.as<toml::array>();
    } else {
        wrap_node = node->as<toml::array>();
    }

    if (wrap_node == nullptr) {
        return false;
    }

    wrap_node->clear();
    wrap_node->emplace_back(value.x);
    wrap_node->emplace_back(value.y);
    wrap_node->emplace_back(value.z);
    wrap_node->emplace_back(value.w);
    return true;
}

template<>
bool Config::Vector4Value(const std::string_view &key, Types::Vector4<std::string> value) const {
    const auto &&node{Node(key)};
    toml::impl::wrap_node<toml::array> *wrap_node{nullptr};
    if (node == nullptr) {
        const auto [it, _]{_section->insert(key, toml::array())};
        wrap_node = it->second.as<toml::array>();
    } else {
        wrap_node = node->as<toml::array>();
    }

    if (wrap_node == nullptr) {
        return false;
    }

    wrap_node->clear();
    wrap_node->emplace_back(value.x);
    wrap_node->emplace_back(value.y);
    wrap_node->emplace_back(value.z);
    wrap_node->emplace_back(value.w);
    return true;
}

#pragma endregion Specialization
