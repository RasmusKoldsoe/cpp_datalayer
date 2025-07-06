#include "DataModel.h"

constinit DataModel dm = {
    .integerValue = DataField<uint16_t>(10, 0, 100),
    .longintValue = DataField<int32_t, void, true>(100, -10, 10000),
    .enumValue = DataField<myEnum, enumValidator>(myEnum::TWO, enumValidator{}),
    .stringValue = DataField<FixedString<32>, NonEmptyValidator>(FixedString<32>("Hello world"), NonEmptyValidator{})
};