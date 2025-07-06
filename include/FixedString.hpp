#pragma once
#include <cstring>
#include <string_view>

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