#pragma once

#include <complex>
#include <cstdint>
#include <string>
#include <vector>

using sample = std::complex<double>;

/**
 * @file coder.cpp
 * @brief Translate string to bits (only ascii)
 *
 * @param[in] str ascii string
 * @return Bit sequence
 **/
std::vector<uint8_t> coder(const std::string &str);

/**
 * @file conv_coding.cpp
 * @brief Add extra bits to improve noise immunity
 *
 * @param[in] bits bits after coder
 * @param[in] poly polynoms
 * @return New bit sequence with extra bits
 **/
std::vector<uint8_t> conv_coder(const std::vector<uint8_t> &bits,
                                const std::vector<int> poly);

/**
 * @file intervaling.cpp
 * @brief Intervale bits
 *
 * @param[in] bits bits after convolve coder
 * @param[in] seed seed for determanating
 * @return New bits sequence with new order
 **/
std::vector<uint8_t> intervale(const std::vector<uint8_t> &bits,
                               const int seed);

/**
 * @file modulation.cpp
 * @brief Translate bits to QPSK symbols
 *
 * @param[in] bits bits after shuffuling
 * @return QPSK symbols
 **/
std::vector<sample> QPSK_modulation(std::vector<uint8_t> &bits);