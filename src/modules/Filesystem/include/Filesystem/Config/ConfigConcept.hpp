#pragma once

#include <string>
#include <vector>

#include "Math/Types/Vector2.hpp"
#include "Math/Types/Vector3.hpp"
#include "Math/Types/Vector4.hpp"

namespace Storm::Filesystem {

    template<typename ValueType>
    concept is_vector2 = std::is_same_v<ValueType, std::vector<Math::Types::Vector2<double> > >
                         || std::is_same_v<ValueType, std::vector<Math::Types::Vector2<std::int64_t> > >
                         || std::is_same_v<ValueType, std::vector<Math::Types::Vector2<std::string> > >;

    template<typename ValueType>
    concept is_vector3 = std::is_same_v<ValueType, std::vector<Math::Types::Vector3<double> > >
                         || std::is_same_v<ValueType, std::vector<Math::Types::Vector3<std::int64_t> > >
                         || std::is_same_v<ValueType, std::vector<Math::Types::Vector3<std::string> > >;

    template<typename ValueType>
    concept is_vector4 = std::is_same_v<ValueType, std::vector<Math::Types::Vector4<double> > >
                         || std::is_same_v<ValueType, std::vector<Math::Types::Vector4<std::int64_t> > >
                         || std::is_same_v<ValueType, std::vector<Math::Types::Vector4<std::string> > >;

    template<typename ValueType>
    concept is_single = std::is_same_v<ValueType, double>
                        || std::is_same_v<ValueType, std::int64_t>
                        || std::is_same_v<ValueType, std::string>;

    template<typename ValueType>
    concept is_array = std::is_same_v<ValueType, std::vector<double> >
                              || std::is_same_v<ValueType, std::vector<std::int64_t> >
                              || std::is_same_v<ValueType, std::vector<std::string> >;

    template<typename ValueType>
    concept is_vector2_array = std::is_same_v<ValueType, std::vector<Math::Types::Vector2<double> > >
                             || std::is_same_v<ValueType, std::vector<Math::Types::Vector2<std::int64_t> > >
                             || std::is_same_v<ValueType, std::vector<Math::Types::Vector2<std::string> > >;

    template<typename ValueType>
    concept is_vector3_array = std::is_same_v<ValueType, std::vector<Math::Types::Vector3<double> > >
                             || std::is_same_v<ValueType, std::vector<Math::Types::Vector3<std::int64_t> > >
                             || std::is_same_v<ValueType, std::vector<Math::Types::Vector3<std::string> > >;
}