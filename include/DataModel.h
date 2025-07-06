#pragma once
#include <cstdint>
#include "FixedString.hpp"
#include "DataField.hpp"

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

extern constinit DataModel dm;