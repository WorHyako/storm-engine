#pragma once

namespace Storm::Math::Types {
    /**
     * @brief   Struct to store data via XYZW
     *
     * @author  WorHyako
     */
    template<typename T>
    struct Vector4 final {
        using value_type = T;
        /**
         * x value
         */
        T x;

        /**
         * y value
         */
        T y;

        /**
         * z value
         */
        T z;

        /**
         * x value
         */
        T w;

        /**
         * @brief Ctor.
         */
        Vector4() noexcept;

        Vector4(T rhs) noexcept;

        /**
         * @brief   Ctor
         *
         * @param   x_  x value
         *
         * @param   y_  y value
         *
         * @param   z_  z value

         * @param   w_  w value
         */
        Vector4(T x_, T y_, T z_, T w_) noexcept;

        /**
         * @brief   Ctor
         *
         * @param   rhs  Right side value
         */
        Vector4(const Vector4 &rhs) noexcept;

        template<typename ToType>
        [[nodiscard]]
        Vector4<ToType> to() const;

        [[nodiscard]]
        std::initializer_list<T> list() const;

#pragma region Operators

        friend bool operator<(const Vector4<T> &lhs_, const Vector4<T> &rhs_) noexcept;

        friend bool operator>(const Vector4<T> &lhs_, const Vector4<T> &rhs_) noexcept;

        friend bool operator<=(const Vector4<T> &lhs_, const Vector4<T> &rhs_) noexcept;

        friend bool operator>=(const Vector4<T> &lhs_, const Vector4<T> &rhs_) noexcept;

        Vector4<T> &operator*=(T rhs_) noexcept;

        Vector4<T> &operator*=(const Vector4<T> &rhs_) noexcept;

        Vector4<T> &operator/=(T rhs_) noexcept;

        Vector4<T> &operator/=(const Vector4<T> &rhs_) noexcept;

        Vector4<T> &operator+=(T rhs_) noexcept;

        Vector4<T> &operator+=(const Vector4<T> &rhs_) noexcept;

        Vector4<T> &operator-=(T rhs_) noexcept;

        Vector4<T> &operator-=(const Vector4<T> &rhs_) noexcept;

        Vector4<T> &operator=(T rhs_) noexcept;

        friend Vector4<T> operator+(Vector4<T> lhs_, Vector4<T> rhs_) noexcept;

        friend Vector4<T> operator+(Vector4<T> lhs_, T rhs_) noexcept;

        friend Vector4<T> operator-(Vector4<T> lhs_, T rhs_) noexcept;

        friend Vector4<T> operator-(Vector4<T> lhs_, Vector4<T> rhs_) noexcept;

        bool operator==(const Vector4<T> &rhs_) const noexcept;

        bool operator!=(const Vector4<T> &rhs_) const noexcept;

#pragma endregion Operators
    };

    template<typename T>
    Vector4<T>::Vector4() noexcept
        : x(static_cast<T>(0)),
          y(static_cast<T>(0)),
          z(static_cast<T>(0)),
          w(static_cast<T>(0)) {
    }

    template<typename T>
    Vector4<T>::Vector4(T rhs) noexcept
        : x(rhs),
          y(rhs),
          z(rhs),
          w(rhs) {
    }

    template<typename T>
    Vector4<T>::Vector4(T x_, T y_, T z_, T w_) noexcept
        : x(x_),
          y(y_),
          z(z_),
          w(w_) {
    }

    template<typename T>
    Vector4<T>::Vector4(const Vector4 &rhs) noexcept
        : x(rhs.x),
          y(rhs.y),
          z(rhs.z),
          w(rhs.w) {
    }

    template<typename T>
    template<typename ToType>
    Vector4<ToType> Vector4<T>::to() const {
        return {
            static_cast<ToType>(x),
            static_cast<ToType>(y),
            static_cast<ToType>(z),
            static_cast<ToType>(w)
        };
    }

#pragma region Operators

    template<typename T>
    bool operator<(const Vector4<T> &lhs_, const Vector4<T> &rhs_) noexcept {
        return lhs_.x < rhs_.x
               && lhs_.y < rhs_.y
               && lhs_.z < rhs_.z
               && lhs_.w < rhs_.w;
    }

    template<typename T>
    bool operator>(const Vector4<T> &lhs_, const Vector4<T> &rhs_) noexcept {
        return rhs_ < lhs_;
    }

    template<typename T>
    bool operator>=(const Vector4<T> &lhs_, const Vector4<T> &rhs_) noexcept {
        return !(lhs_ < rhs_);
    }

    template<typename T>
    bool operator<=(const Vector4<T> &lhs_, const Vector4<T> &rhs_) noexcept {
        return !(lhs_ > rhs_);
    }

    template<typename T>
    bool Vector4<T>::operator==(const Vector4<T> &rhs_) const noexcept {
        return x == rhs_.x
               && y == rhs_.y
               && z == rhs_.z
               && w == rhs_.w;
    }

    template<typename T>
    bool Vector4<T>::operator!=(const Vector4<T> &rhs_) const noexcept {
        return x != rhs_.x
               && y != rhs_.y
               && z != rhs_.z
               && w != rhs_.w;
    }

    template<typename T>
    Vector4<T> operator+(Vector4<T> lhs_, T rhs_) noexcept {
        lhs_ += rhs_;
        return lhs_;
    }

    template<typename T>
    Vector4<T> operator+(Vector4<T> lhs_, Vector4<T> rhs_) noexcept {
        lhs_ += rhs_;
        return lhs_;
    }

    template<typename T>
    Vector4<T> operator-(Vector4<T> lhs_, T rhs_) noexcept {
        return lhs_ -= rhs_;
    }

    template<typename T>
    Vector4<T> operator-(Vector4<T> lhs_, Vector4<T> rhs_) noexcept {
        return lhs_ -= rhs_;
    }

    template<typename T>
    Vector4<T> &Vector4<T>::operator*=(const Vector4<T> &rhs_) noexcept {
        x *= rhs_.x;
        y *= rhs_.y;
        z *= rhs_.z;
        w *= rhs_.w;
        return *this;
    }

    template<typename T>
    Vector4<T> &Vector4<T>::operator*=(T rhs_) noexcept {
        x *= rhs_;
        y *= rhs_;
        z *= rhs_;
        w *= rhs_;
        return *this;
    }

    template<typename T>
    Vector4<T> &Vector4<T>::operator/=(const Vector4<T> &rhs_) noexcept {
        x /= rhs_.x;
        y /= rhs_.y;
        z /= rhs_.z;
        w /= rhs_.w;
        return *this;
    }

    template<typename T>
    Vector4<T> &Vector4<T>::operator/=(T rhs_) noexcept {
        x /= rhs_;
        y /= rhs_;
        z /= rhs_;
        w /= rhs_;
        return *this;
    }

    template<typename T>
    Vector4<T> &Vector4<T>::operator+=(const Vector4<T> &rhs_) noexcept {
        x += rhs_.x;
        y += rhs_.y;
        z += rhs_.z;
        w += rhs_.w;
        return *this;
    }

    template<typename T>
    Vector4<T> &Vector4<T>::operator+=(T rhs_) noexcept {
        x += rhs_;
        y += rhs_;
        z += rhs_;
        w += rhs_;
        return *this;
    }

    template<typename T>
    Vector4<T> &Vector4<T>::operator-=(T rhs_) noexcept {
        x -= rhs_;
        y -= rhs_;
        z -= rhs_;
        w -= rhs_;
        return *this;
    }

    template<typename T>
    Vector4<T> &Vector4<T>::operator-=(const Vector4<T> &rhs_) noexcept {
        x -= rhs_.x;
        y -= rhs_.y;
        z -= rhs_.z;
        w -= rhs_.w;
        return *this;
    }

    template<typename T>
    Vector4<T> &Vector4<T>::operator=(T rhs_) noexcept {
        x = rhs_;
        y = rhs_;
        z = rhs_;
        w = rhs_;
        return *this;
    }

#pragma endregion Operators
}
