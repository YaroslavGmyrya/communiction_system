#pragma once

#include <complex>
#include <cstdint>
#include <string>
#include <vector>

#include "../include/ImGUI_interface.hpp"

using sample = std::complex<double>;

/**
 * @brief Convert ASCII string to bit sequence.
 *
 * @param[in] str Input ASCII string.
 * @return Vector of bits. Returns empty vector on error.
 */
std::vector<uint8_t> coder(const std::string &str);

/**
 * @brief Convert convolutional polynomials to tap positions.
 *
 * @param[in] poly Generator polynomials.
 * @return Tap positions for each polynomial.
 */
std::vector<std::vector<int>> poly2pos(const std::vector<int> &poly);

/**
 * @brief Perform convolutional encoding.
 *
 * @param[in] bits Input bit sequence.
 * @param[in] poly Generator polynomials.
 * @return Encoded bit sequence. Returns empty vector on error.
 */
std::vector<uint8_t> conv_coder(const std::vector<uint8_t> &bits,
                                const std::vector<int> &poly);

/**
 * @brief Generate shuffled index order.
 *
 * @param[in] N Number of elements.
 * @param[in] seed Random seed.
 * @return Permutation of indices.
 */
std::vector<int> order_gen(int N, int seed);

/**
 * @brief Shuffle bits using generated permutation.
 *
 * @param[in] bits Input bit sequence.
 * @param[in] seed Random seed.
 * @return Shuffled bit sequence. Returns empty vector on error.
 */
std::vector<uint8_t> shuffuling(const std::vector<uint8_t> &bits,
                                const int &seed);

/**
 * @brief Modulate bits using BPSK.
 *
 * @param[in] bits Input bit sequence.
 * @return Vector of complex BPSK symbols. Returns empty vector on error.
 */
std::vector<sample> BPSK(const std::vector<uint8_t> &bits);

/**
 * @brief Modulate bits using QPSK.
 *
 * @param[in] bits Input bit sequence.
 * @return Vector of complex QPSK symbols. Returns empty vector on error.
 */
std::vector<sample> QPSK(const std::vector<uint8_t> &bits);

/**
 * @brief Modulate bits using 16-QAM.
 *
 * @param[in] bits Input bit sequence.
 * @return Vector of complex 16-QAM symbols. Returns empty vector on error.
 */
std::vector<sample> QAM16(const std::vector<uint8_t> &bits);

/**
 * @brief Modulate bits using 64-QAM.
 *
 * @param[in] bits Input bit sequence.
 * @return Vector of complex 64-QAM symbols. Returns empty vector on error.
 */
std::vector<sample> QAM64(const std::vector<uint8_t> &bits);

/**
 * @brief Modulate bits using 256-QAM.
 *
 * @param[in] bits Input bit sequence.
 * @return Vector of complex 256-QAM symbols. Returns empty vector on error.
 */
std::vector<sample> QAM256(const std::vector<uint8_t> &bits);

/**
 * @brief Modulate bits using 1024-QAM.
 *
 * @param[in] bits Input bit sequence.
 * @return Vector of complex 1024-QAM symbols. Returns empty vector on error.
 */
std::vector<sample> QAM1024(const std::vector<uint8_t> &bits);

/**
 * @brief Modulate bits with selected modulation order.
 *
 * Supported orders:
 * 2 -> BPSK
 * 4 -> QPSK
 * 16 -> 16-QAM
 * 64 -> 64-QAM
 * 256 -> 256-QAM
 * 1024 -> 1024-QAM
 *
 * @param[in] bits Input bit sequence.
 * @param[in] order Modulation order.
 * @return Vector of complex symbols. Returns empty vector on error.
 */
std::vector<sample> modulation(const std::vector<uint8_t> &bits,
                               const int &order);

std::vector<cell_type> create_ofdm_grid(int FFT_size, int pilots_count,
                                        int gi_size);

std::vector<std::complex<double>>
create_ofdm_signal(const std::vector<std::complex<double>> &symbols,
                   const std::vector<cell_type> &grid,
                   std::complex<double> pilot_value, int buff_size);

std::vector<std::complex<double>>
add_CP(const std::vector<std::complex<double>> &samples, int FFT_size,
       int CP_size);

void batch_ifft(std::vector<std::complex<double>> &data,
                std::vector<std::complex<double>> &ifft_out, const int FFT_size,
                const int CP_size);

/**
 * @brief Run TX processing chain and update shared TX configuration.
 *
 * @param[in,out] config Shared TX configuration.
 */
void tx_run(tx_cfg &config);
