#pragma once

#include <complex>
#include <cstdint>
#include <string>
#include <vector>

using sample = std::complex<double>;

/**
 * @file decoder.cpp
 * @brief Translate bits to string (only ascii)
 *
 * @param[in] bits bit sequence
 * @return String
 **/
std::string decoder(const std::vector<uint8_t> &bits);

/**
 * @file deintervaling.cpp
 * @brief Recovery intervale bits
 *
 * @param[in] bits bits after demodulation
 * @param[in] seed seed for determanating
 * @return Recovery bits sequence
 **/
std::vector<uint8_t> deintervale(const std::vector<uint8_t> &bits,
                                 const int seed);

/**
 * @file demodulation.cpp
 * @brief Translate QPSK symbols to bits
 *
 * @param[in] symbols symbols after deshuffuling
 * @return Bits
 **/
std::vector<uint8_t> QPSK_demodulation(const std::vector<sample> &samples);