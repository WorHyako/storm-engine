#pragma once

namespace Storm::Math::Types {
    /**
     * @brief   Struct to store data via XYZ
     *
     * @author  WorHyako
     */
    template<typename T>
    struct Vector3 final {
        /**
         * @brief Ctor.
         */
        Vector3() noexcept;

        /**
         * @brief   Ctor.
         *
         * @param   x_  x value
         *
         * @param   y_  y value
         *
         * @param   z_  z value
         */
        Vector3(T x_, T y_, T z_) noexcept;

        /**
         * @brief   Ctor.
         *
         * @param   rhs  Right size value
         */
        Vector3(const Vector3 &rhs) noexcept;

        /**
         * x value
         */
        T x;

        /**
         * y value
         */
        T y;

        /**
         * y value
         */
        T z;

        template<typename ToType>
        [[nodiscard]]
        Vector3<ToType> to() const;

#pragma region Operators

        friend bool operator<(const Vector3<T> &lhs_, const Vector3<T> &rhs_) noexcept;

        friend bool operator>(const Vector3<T> &lhs_, const Vector3<T> &rhs_) noexcept;

        friend bool operator<=(const Vector3<T> &lhs_, const Vector3<T> &rhs_) noexcept;

        friend bool operator>=(const Vector3<T> &lhs_, const Vector3<T> &rhs_) noexcept;

        Vector3<T> &operator*=(T rhs_) noexcept;

        Vector3<T> &operator*=(const Vector3<T> &rhs_) noexcept;

        Vector3<T> &operator/=(T rhs_) noexcept;

        Vector3<T> &operator/=(const Vector3<T> &rhs_) noexcept;

        Vector3<T> &operator+=(T rhs_) noexcept;

        Vector3<T> &operator+=(const Vector3<T> &rhs_) noexcept;

        Vector3<T> &operator-=(T rhs_) noexcept;

        Vector3<T> &operator-=(const Vector3<T> &rhs_) noexcept;

        Vector3<T> &operator=(T rhs_) noexcept;

        friend Vector3<T> operator+(Vector3<T> lhs_, Vector3<T> rhs_) noexcept;

        friend Vector3<T> operator+(Vector3<T> lhs_, T rhs_) noexcept;

        friend Vector3<T> operator-(Vector3<T> lhs_, T rhs_) noexcept;

        friend Vector3<T> operator-(Vector3<T> lhs_, Vector3<T> rhs_) noexcept;

        bool operator==(const Vector3<T> &rhs_) const noexcept;

        bool operator!=(const Vector3<T> &rhs_) const noexcept;

#pragma endregion Operators
    };

    template<typename T>
    Vector3<T>::Vector3() noexcept
        : x{},
          y{},
          z{} {
    }

    template<typename T>
    Vector3<T>::Vector3(T x_, T y_, T z_) noexcept
        : x(x_),
          y(y_),
          z(z_) {
    }

    template<typename T>
    Vector3<T>::Vector3(const Vector3 &rhs) noexcept
        : x(rhs.x),
          y(rhs.y),
          z(rhs.z) {
    }

    template<typename T>
    template<typename ToType>
    Vector3<ToType> Vector3<T>::to() const {
        return {
            static_cast<ToType>(x),
            static_cast<ToType>(y),
            static_cast<ToType>(z)
        };
    }

#pragma region Operators

    template<typename T>
    bool operator<(const Vector3<T> &lhs_, const Vector3<T> &rhs_) noexcept {
        return lhs_.x < rhs_.x
               && lhs_.y < rhs_.y
               && lhs_.z < rhs_.z;
    }

    template<typename T>
    bool operator>(const Vector3<T> &lhs_, const Vector3<T> &rhs_) noexcept {
        return rhs_ < lhs_;
    }

    template<typename T>
    bool operator>=(const Vector3<T> &lhs_, const Vector3<T> &rhs_) noexcept {
        return !(lhs_ < rhs_);
    }

    template<typename T>
    bool operator<=(const Vector3<T> &lhs_, const Vector3<T> &rhs_) noexcept {
        return !(lhs_ > rhs_);
    }

    template<typename T>
    bool Vector3<T>::operator==(const Vector3<T> &rhs_) const noexcept {
        return x == rhs_.x
               && y == rhs_.y
               && z == rhs_.z;
    }

    template<typename T>
    bool Vector3<T>::operator!=(const Vector3<T> &rhs_) const noexcept {
        return x != rhs_.x
               && y != rhs_.y
               && z != rhs_.z;
    }

    template<typename T>
    Vector3<T> operator+(Vector3<T> lhs_, T rhs_) noexcept {
        lhs_ += rhs_;
        return lhs_;
    }

    template<typename T>
    Vector3<T> operator+(Vector3<T> lhs_, Vector3<T> rhs_) noexcept {
        lhs_ += rhs_;
        return lhs_;
    }

    template<typename T>
    Vector3<T> operator-(Vector3<T> lhs_, T rhs_) noexcept {
        return lhs_ -= rhs_;
    }

    template<typename T>
    Vector3<T> operator-(Vector3<T> lhs_, Vector3<T> rhs_) noexcept {
        return lhs_ -= rhs_;
    }

    template<typename T>
    Vector3<T> &Vector3<T>::operator*=(const Vector3<T> &rhs_) noexcept {
        x *= rhs_.x;
        y *= rhs_.y;
        z *= rhs_.z;
        return *this;
    }

    template<typename T>
    Vector3<T> &Vector3<T>::operator*=(T rhs_) noexcept {
        x *= rhs_;
        y *= rhs_;
        z *= rhs_;
        return *this;
    }

    template<typename T>
    Vector3<T> &Vector3<T>::operator/=(const Vector3<T> &rhs_) noexcept {
        x /= rhs_.x;
        y /= rhs_.y;
        z /= rhs_.z;
        return *this;
    }

    template<typename T>
    Vector3<T> &Vector3<T>::operator/=(T rhs_) noexcept {
        x /= rhs_;
        y /= rhs_;
        z /= rhs_;
        return *this;
    }

    template<typename T>
    Vector3<T> &Vector3<T>::operator+=(const Vector3<T> &rhs_) noexcept {
        x += rhs_.x;
        y += rhs_.y;
        z += rhs_.z;
        return *this;
    }

    template<typename T>
    Vector3<T> &Vector3<T>::operator+=(T rhs_) noexcept {
        x += rhs_;
        y += rhs_;
        z += rhs_;
        return *this;
    }

    template<typename T>
    Vector3<T> &Vector3<T>::operator-=(T rhs_) noexcept {
        x -= rhs_;
        y -= rhs_;
        z -= rhs_;
        return *this;
    }

    template<typename T>
    Vector3<T> &Vector3<T>::operator-=(const Vector3<T> &rhs_) noexcept {
        x -= rhs_.x;
        y -= rhs_.y;
        z -= rhs_.z;
        return *this;
    }

    template<typename T>
    Vector3<T> &Vector3<T>::operator=(T rhs_) noexcept {
        x = rhs_;
        y = rhs_;
        z = rhs_;
        return *this;
    }

#pragma endregion Operators
}
