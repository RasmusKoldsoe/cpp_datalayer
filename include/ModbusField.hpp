#pragma once
#include <cstdint>

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
