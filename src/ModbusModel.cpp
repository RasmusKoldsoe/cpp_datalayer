#include "ModbusModel.h"

constinit ModbusHoldingModel mbm_holding = {
    .integerValue = ModbusHoldingField<uint16_t>(dm.integerValue, 10u, ModbusAccess::Open, ModbusAccess::Open),
    .longintValue = ModbusHoldingField<int32_t, void, true>(dm.longintValue, 11u, ModbusAccess::Open, ModbusAccess::Admin),
    .stringValue = ModbusHoldingField<FixedString<32>, NonEmptyValidator>(dm.stringValue, 12u, ModbusAccess::Open, ModbusAccess::Open)
};

constinit ModbusInputModel mbm_input = {
    .enumValue = ModbusInputField<myEnum, enumValidator>(dm.enumValue, 10u, ModbusAccess::Open, ModbusAccess::Open)
};