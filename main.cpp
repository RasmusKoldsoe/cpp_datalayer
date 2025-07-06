#include <iostream>
#include <iomanip>
#include "DataModel.h"
#include "ModbusModel.h"

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