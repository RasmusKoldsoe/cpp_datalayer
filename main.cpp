#include <iostream>
#include <iomanip>
#include <algorithm>
#include <array>
#include <ranges>
#include <cstdint>
#include <cstring>
#include <functional>

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

template<std::size_t N>
struct FixedString {
    char data[N]{0, } ;

    constexpr FixedString() = default;

    constexpr FixedString(const char* s)
    {
        size_t i = 0;
        while (s[i] && i < N - 1)
        {
            data[i] = s[i];
            ++i;
        }
        data[i] = '\0';
    }

    void set(const char* s) {
        std::strncpy(data, s, N - 1);
        data[N - 1] = '\0';
    }

    constexpr const char* c_str() const { return data; }
    constexpr bool empty() const { return data[0] == '\0'; }
    constexpr std::string_view view() const {return std::string_view(data);}

    bool operator==(const char* rhs) const {
        return std::strncmp(data, rhs, N) == 0;
    }
};

struct NonEmptyValidator {
    constexpr bool operator()(const FixedString<32>& s) const {
        return !s.empty();
    }
};

enum class myEnum {ONE = 1, TWO, THREE, FOUR, LAST};
struct enumValidator {
    constexpr bool operator()(const myEnum e) const {
        return e < myEnum::LAST;
    }
};

struct DataModel
{
    DataField<uint16_t> integerValue;
    DataField<int32_t, void, true> longintValue;
    DataField<myEnum, enumValidator> enumValue;
    DataField<FixedString<32>, NonEmptyValidator> stringValue;
};

constinit static DataModel dm = {
    .integerValue = DataField<uint16_t>(10, 0, 100),
    .longintValue = DataField<int32_t, void, true>(100, -10, 10000),
    .enumValue = DataField<myEnum, enumValidator>(myEnum::TWO, enumValidator{}),
    .stringValue = DataField<FixedString<32>, NonEmptyValidator>(FixedString<32>("Hello world"), NonEmptyValidator{})
};

enum class ModbusAccess : uint8_t {
    Open = 0,
    Factory,
    Admin
};

template<typename T, typename ValidatorT = void, bool isOverride = false>
struct ModbusField;

template<typename T, typename ValidatorT, bool isOverride>
struct ModbusField
{
    DataField<T, ValidatorT, isOverride>& dataField;
    const uint16_t address;
    const ModbusAccess factoryWriteProtect : 4;
    const ModbusAccess userWriteProtect : 4;

    constexpr ModbusField(DataField<T, ValidatorT, isOverride>& data, const uint16_t address,
            const ModbusAccess factory, const ModbusAccess user)
        : dataField(data), address(address),
            factoryWriteProtect(factory), userWriteProtect(user) {}
    
    constexpr bool Serialize(uint8_t* const b, size_t* const len)
    {
        if (b == nullptr || len == nullptr) return false;

        size_t data_len;

        if constexpr (std::is_same_v<int32_t, T> || std::is_same_v<uint32_t, T> || std::is_same_v<float, T>) {
            data_len = sizeof(int32_t);
        } else if constexpr (std::is_same_v<int16_t, T> || std::is_same_v<uint16_t, T>) {
            data_len = sizeof(uint16_t);
        } else if constexpr (std::is_same_v<int8_t, T> || std::is_same_v<uint8_t, T> || std::is_same_v<char, T>) {
            data_len = sizeof(uint8_t);
        }

        if(*len >= data_len)
        {
            const T value = dataField.get();
            *len = data_len;
            memcpy(b, static_cast<const void*>(&value), *len);
            return true;
        }
        return false;
    }
};

template<typename T, typename V = void, bool isOverride = false>
struct ModbusHoldingField;

template<typename T, typename V>
struct ModbusHoldingField<T, V, false> : public ModbusField<T, V, false>
{
    using Storage = ModbusField<T, V, false>;

    constexpr ModbusHoldingField(DataField<T, V, false>& data, const uint16_t address,
            const ModbusAccess factory, const ModbusAccess user)
        : Storage(data, address, factory, user)
    {}

    bool UpdateFromModbus(T& newValue)
    {
        return Storage::dataField.Set(newValue);
    }
};

template<typename T, typename V>
struct ModbusHoldingField<T, V, true> : public ModbusField<T, V, true>
{
    using Storage = ModbusField<T, V, true>;

    constexpr ModbusHoldingField(DataField<T, V, true>& data, const uint16_t address,
            const ModbusAccess factory, const ModbusAccess user)
        : Storage(data, address, factory, user)
    {}

    bool UpdateFromModbus(T& newValue)
    {
        return Storage::dataField.Set(newValue);
    }

    bool UpdateOverrideFromModbus(T& newOverride)
    {
        return Storage::dataField.SetOverride(newOverride);
    }

    void ResetOverrideFromModbus()
    {
        Storage::dataField.ResetOverride();
    }
};

template<typename T, typename V = void>
struct ModbusInputField;

template<typename T, typename V>
struct ModbusInputField : public ModbusField<T, V>
{
    using Storage = ModbusField<T, V>;

    constexpr ModbusInputField(DataField<T, V>& data, const uint16_t address,
            const ModbusAccess factory, const ModbusAccess user)
        : Storage(data, address, factory, user)
    {}
};

struct ModbusHoldingModel
{
    ModbusHoldingField<uint16_t> integerValue;
    ModbusHoldingField<int32_t, void, true> longintValue;
    ModbusHoldingField<FixedString<32>, NonEmptyValidator> stringValue;
};

struct ModbusInputModel
{
    ModbusInputField<myEnum, enumValidator> enumValue;
};

constinit static ModbusHoldingModel mbm_holding = {
    .integerValue = ModbusHoldingField<uint16_t>(dm.integerValue, 10u, ModbusAccess::Open, ModbusAccess::Open),
    .longintValue = ModbusHoldingField<int32_t, void, true>(dm.longintValue, 11u, ModbusAccess::Open, ModbusAccess::Admin),
    .stringValue = ModbusHoldingField<FixedString<32>, NonEmptyValidator>(dm.stringValue, 12u, ModbusAccess::Open, ModbusAccess::Open)
};

constinit static ModbusInputModel mbm_input = {
    .enumValue = ModbusInputField(dm.enumValue, 10u, ModbusAccess::Open, ModbusAccess::Open)
};

static void functionThatDoesStuff(auto &reg)
{
    reg += 10;
}

void PrintBytes(const uint8_t* pBytes, const size_t nBytes) {
    for ( size_t i = 0; i < nBytes; i++ ) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (unsigned int)( pBytes[ i ] );
    }
}

int main (void)
{
    dm.integerValue = 12;

    std::cout << dm.integerValue.get() << std::endl;
    std::cout << dm.stringValue.get().c_str() << std::endl;
    std::cout << static_cast<std::size_t>(dm.enumValue.get()) << std::endl;
    std::cout << dm.longintValue.get() << std::endl;

    dm.enumValue.Set(myEnum::TWO);
    std::cout << static_cast<std::size_t>(dm.enumValue.get()) << std::endl;

    functionThatDoesStuff(dm.longintValue);
    std::cout << dm.longintValue.get() << std::endl;

    int32_t newLongIntValue = 42;
    mbm_holding.longintValue.UpdateOverrideFromModbus(newLongIntValue);
    std::cout << dm.longintValue.get() << std::endl;

    dm.longintValue += 1;
    std::cout << dm.longintValue.get() << std::endl;

    mbm_holding.longintValue.ResetOverrideFromModbus();
    std::cout << dm.longintValue.get() << std::endl;

    size_t buffer_len = 8;
    uint8_t buffer[8] = {0, };
    mbm_holding.longintValue.Serialize(&buffer[0], &buffer_len);
    PrintBytes(buffer, buffer_len);
    std::cout << " Length: " << std::dec << buffer_len << std::endl;

    uint16_t newIntegerValue = 24;
    mbm_holding.integerValue.UpdateFromModbus(newIntegerValue);
    std::cout << dm.integerValue.get() << std::endl;

}