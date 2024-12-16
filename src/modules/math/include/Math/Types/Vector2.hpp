#pragma once

namespace Storm::Math::Types {
    /**
     * @brief   Struct to store data via XY
     *
     * @author  WorHyako
     */
    template<typename T>
    struct Vector2 final {
        /**
         * @brief Ctor.
         */
        Vector2() noexcept;

        /**
         * @brief   Ctor.
         *
         * @param   x_  x value
         *
         * @param   y_  y value
         */
        Vector2(T x_, T y_) noexcept;

        /**
         * @brief   Ctor.
         *
         * @param   rhs Right side value
         */
        Vector2(const Vector2 &rhs) noexcept;

        /**
         * x value
         */
        T x;

        /**
         * y value
         */
        T y;

        template<typename ToType>
        [[nodiscard]]
        Vector2<ToType> to() const;

#pragma region Operators

        friend bool operator<(const Vector2<T> &lhs_, const Vector2<T> &rhs_) noexcept;

        friend bool operator>(const Vector2<T> &lhs_, const Vector2<T> &rhs_) noexcept;

        friend bool operator<=(const Vector2<T> &lhs_, const Vector2<T> &rhs_) noexcept;

        friend bool operator>=(const Vector2<T> &lhs_, const Vector2<T> &rhs_) noexcept;

        Vector2<T> &operator*=(T rhs_) noexcept;

        Vector2<T> &operator*=(const Vector2<T> &rhs_) noexcept;

        Vector2<T> &operator/=(T rhs_) noexcept;

        Vector2<T> &operator/=(const Vector2<T> &rhs_) noexcept;

        Vector2<T> &operator+=(T rhs_) noexcept;

        Vector2<T> &operator+=(const Vector2<T> &rhs_) noexcept;

        Vector2<T> &operator-=(T rhs_) noexcept;

        Vector2<T> &operator-=(const Vector2<T> &rhs_) noexcept;

        Vector2<T> &operator=(T rhs_) noexcept;

        friend Vector2<T> operator+(Vector2<T> lhs_, Vector2<T> rhs_) noexcept;

        friend Vector2<T> operator+(Vector2<T> lhs_, T rhs_) noexcept;

        friend Vector2<T> operator-(Vector2<T> lhs_, T rhs_) noexcept;

        friend Vector2<T> operator-(Vector2<T> lhs_, Vector2<T> rhs_) noexcept;

        bool operator==(const Vector2<T> &rhs_) const noexcept;

        bool operator!=(const Vector2<T> &rhs_) const noexcept;

#pragma endregion Operators
    };

    template<typename T>
    Vector2<T>::Vector2() noexcept {
        x = static_cast<T>(0);
        y = static_cast<T>(0);
    }

    template<typename T>
    Vector2<T>::Vector2(T x_, T y_) noexcept
        : x(x_),
          y(y_) {
    }

    template<typename T>
    Vector2<T>::Vector2(const Vector2 &rhs) noexcept
        : x(rhs.x),
          y(rhs.y) {
    }

    template<typename T>
    template<typename ToType>
    Vector2<ToType> Vector2<T>::to() const {
        return {
            static_cast<ToType>(x),
            static_cast<ToType>(y)
        };
    }

#pragma region Operators

    template<typename T>
    bool operator<(const Vector2<T> &lhs_, const Vector2<T> &rhs_) noexcept {
        return lhs_.x < rhs_.x
               && lhs_.y < rhs_.y;
    }

    template<typename T>
    bool operator>(const Vector2<T> &lhs_, const Vector2<T> &rhs_) noexcept {
        return rhs_ < lhs_;
    }

    template<typename T>
    bool operator>=(const Vector2<T> &lhs_, const Vector2<T> &rhs_) noexcept {
        return !(lhs_ < rhs_);
    }

    template<typename T>
    bool operator<=(const Vector2<T> &lhs_, const Vector2<T> &rhs_) noexcept {
        return !(lhs_ > rhs_);
    }

    template<typename T>
    bool Vector2<T>::operator==(const Vector2<T> &rhs_) const noexcept {
        return x == rhs_.x
               && y == rhs_.y;
    }

    template<typename T>
    bool Vector2<T>::operator!=(const Vector2<T> &rhs_) const noexcept {
        return x != rhs_.x
               && y != rhs_.y;
    }

    template<typename T>
    Vector2<T> operator+(Vector2<T> lhs_, T rhs_) noexcept {
        lhs_ += rhs_;
        return lhs_;
    }

    template<typename T>
    Vector2<T> operator+(Vector2<T> lhs_, Vector2<T> rhs_) noexcept {
        lhs_ += rhs_;
        return lhs_;
    }

    template<typename T>
    Vector2<T> operator-(Vector2<T> lhs_, T rhs_) noexcept {
        return lhs_ -= rhs_;
    }

    template<typename T>
    Vector2<T> operator-(Vector2<T> lhs_, Vector2<T> rhs_) noexcept {
        return lhs_ -= rhs_;
    }

    template<typename T>
    Vector2<T> &Vector2<T>::operator*=(const Vector2<T> &rhs_) noexcept {
        x *= rhs_.x;
        y *= rhs_.y;
        return *this;
    }

    template<typename T>
    Vector2<T> &Vector2<T>::operator*=(T rhs_) noexcept {
        x *= rhs_;
        y *= rhs_;
        return *this;
    }

    template<typename T>
    Vector2<T> &Vector2<T>::operator/=(const Vector2<T> &rhs_) noexcept {
        x /= rhs_.x;
        y /= rhs_.y;
        return *this;
    }

    template<typename T>
    Vector2<T> &Vector2<T>::operator/=(T rhs_) noexcept {
        x /= rhs_;
        y /= rhs_;
        return *this;
    }

    template<typename T>
    Vector2<T> &Vector2<T>::operator+=(const Vector2<T> &rhs_) noexcept {
        x += rhs_.x;
        y += rhs_.y;
        return *this;
    }

    template<typename T>
    Vector2<T> &Vector2<T>::operator+=(T rhs_) noexcept {
        x += rhs_;
        y += rhs_;
        return *this;
    }

    template<typename T>
    Vector2<T> &Vector2<T>::operator-=(T rhs_) noexcept {
        x -= rhs_;
        y -= rhs_;
        return *this;
    }

    template<typename T>
    Vector2<T> &Vector2<T>::operator-=(const Vector2<T> &rhs_) noexcept {
        x -= rhs_.x;
        y -= rhs_.y;
        return *this;
    }

    template<typename T>
    Vector2<T> &Vector2<T>::operator=(T rhs_) noexcept {
        x = rhs_;
        y = rhs_;
        return *this;
    }

#pragma endregion Operators
}
