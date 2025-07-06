#pragma once
#include <algorithm>
#include <concepts>
#include <optional>
#include <type_traits>

/*
By introducing the `IsArithmetic` trait, we avoid directly using `std::is_arithmetic_v<T>`
in the specialization condition, which is not allowed. Instead, the trait computes the value,
and the result (`true` or `false`) is used as a non-type template parameter, which is valid for
partial specialization.
*/
template<typename T>
struct IsArithmetic {
    static constexpr bool value = std::is_arithmetic_v<T>;
};

template<typename T, typename ValidatorT = void, bool = std::is_arithmetic_v<T>>
struct DataFieldStorage;

// Specialization for arithmetic types
template<typename T>
requires std::is_arithmetic_v<T>
struct DataFieldStorage<T, void, true> {
    T value{};
    const T min{};
    const T max{};

    constexpr DataFieldStorage(T init, T min_, T max_)
        : value(std::clamp(init, min_, max_)), min(min_), max(max_) {}
    
    constexpr bool Set(const T& v) {
        value = std::clamp(v, min, max);
        return true;
    }

    constexpr bool isValid(const T& v) const {return v >= min && v <= max;}
};

template<typename T, typename ValidatorT>
concept ValidatesT = requires(const T& value, ValidatorT v) {
    { v(value) } -> std::convertible_to<bool>;
};

// Specialization for non-arithmetic types with validator
template<typename T, typename ValidatorT>
requires ValidatesT<T, ValidatorT>
struct DataFieldStorage<T, ValidatorT, false> {
    T value{};
    ValidatorT validator_;

    constexpr DataFieldStorage(T init, ValidatorT validator)
        : value(init), validator_(validator) {}

    constexpr bool Set(const T& v) {
        if (validator_(v)) {
            value = v;
            return true;
        }
        return false;
    }

    constexpr bool isValid(const T& v) const {return validator_(v);}
};


template<typename T, typename ValidatorT = void, bool isOverride = false, bool IsArithmetic = IsArithmetic<T>::value>
struct DataField;

template<typename T>
struct DataField<T, void, false, true> : private DataFieldStorage<T>
{
    static const bool isOverride = false;
    using Storage = DataFieldStorage<T, void, std::is_arithmetic_v<T>>;
    using Storage::Set;
    using Storage::isValid;

    constexpr DataField(T init, T min_, T max_)
        requires std::is_arithmetic_v<T>
        : Storage(init, min_, max_) {}

    constexpr T operator=(const T& rhs) {
        Storage::Set(rhs);
        return Storage::value;
    }

    constexpr T operator+=(const T& rhs) {
        Storage::Set(Storage::value + rhs);
        return Storage::value;
    }

    constexpr T operator-=(const T& rhs) {
        Storage::Set(Storage::value - rhs);
        return Storage::value;
    }

    constexpr T get() const { return Storage::value; }
};

template<typename T>
struct DataField<T, void, true, true> : private DataField<T, void, false>
{
    static const bool isOverride = true;
    std::optional<T> overrideValue;

    using Storage = DataField<T, void, false>;
    using Storage::Set;
    using Storage::isValid;
    using Storage::operator=;
    using Storage::operator+=;
    using Storage::operator-=;

    constexpr DataField(T init, T min_, T max_)
        requires std::is_arithmetic_v<T>
        : Storage(init, min_, max_) {}
    
    bool SetOverride(const T& v)
    {
        if(Storage::isValid(v))
        {
            overrideValue = v;
            return true;
        }
        return false;
    }
    void ResetOverride() {overrideValue.reset();}

    constexpr T get() const
    {
        return overrideValue.value_or(Storage::get());
    }
};

template<typename T, typename ValidatorT>
requires ValidatesT<T, ValidatorT>
struct DataField<T, ValidatorT, false, false> : private DataFieldStorage<T, ValidatorT>
{
    static const bool isOverride = false;
    using Storage = DataFieldStorage<T, ValidatorT, std::is_arithmetic_v<T>>;
    using Storage::Set;
    using Storage::isValid;

    constexpr DataField(T init, ValidatorT validator)
        requires (!std::is_pointer_v<T> && !std::is_void_v<ValidatorT>)
        : Storage(init, validator) {}

    T get() const { return Storage::value; }
};