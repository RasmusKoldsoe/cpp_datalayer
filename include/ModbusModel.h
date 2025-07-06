#pragma once
#include "FixedString.hpp"
#include "DataModel.h"
#include "ModbusField.hpp"

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

extern constinit ModbusHoldingModel mbm_holding;
extern constinit ModbusInputModel mbm_input;