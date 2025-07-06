CMAKE_MINIMUM_REQUIRED(VERSION 3.22)

project(CPP_Datalayer LANGUAGES CXX)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(CPP_Datalayer
    main.cpp
    src/DataModel.cpp
    src/ModbusModel.cpp
)

target_include_directories(CPP_Datalayer
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_options(CPP_Datalayer
  PUBLIC
    LINKER:-Map=CPP_Datalayer.map
)

target_compile_options(CPP_Datalayer PUBLIC
    -Wall -fdata-sections -ffunction-sections -fno-common
    $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti -fno-exceptions -fno-threadsafe-statics -Wno-volatile>
    $<$<COMPILE_LANGUAGE:ASM>:-x assembler-with-cpp -MMD -MP>
    $<$<CONFIG:Debug>:-Og -g3>
    $<$<CONFIG:Release>:-Os -g0>
)