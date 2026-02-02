#include "../../include/TX.hpp"
#include <string>
#include <cstdint>
#include <vector>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

/**
 * @file coder.cpp
 * @brief Translate string to bits (only ascii)
 *
 * @param[in] str ascii string
 * @return Bit sequence
 **/

std::vector<uint8_t> coder(const std::string &str)
{
    if (str.size() == 0)
    {
        spdlog::error("[coder.cpp] String size is zero!")
    }

    int bits_size = str.size() * 8;
    std::vector<uint8_t> bits(bits_size);

    // iterate on string
    for (int i = 0; i < str.size(); i++)
    {
        // iterate on bits massive
        for (int j = 0; j < 8; j++)
        {
            // convert char to bits
            bits[i * 8 + j] = (str[i] >> (7 - j)) & 1;
        }
    }

    return bits;
}