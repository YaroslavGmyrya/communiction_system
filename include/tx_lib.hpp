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


/**
 * @brief Create OFDM grid. She used as template for creating OFDM symbols: M-PSK/M-QAM symbols insert in position, 
 * which have type "data". Template have N zeros from each side and N pilots, which are distributed evely (guranteed, that
 * pilots located on the sides)
 *
 * @param[in] FFT_size OFDM symbols size.
 * @param[in] pilots_count count of pilots.
 * @param[in] gi_size Modulation order.
 * @return OFDM grid.
 */
std::vector<cell_type> create_ofdm_grid(int FFT_size, int pilots_count,
                                        int gi_size);


/**
 * @brief Fill OFDM grid with zeros, pilots, data
 *
 * @param[in] symbols M-PSK/M-QAM symbols.
 * @param[in] grid template for OFDM symbol.
 * @param[in] pilot_value value for pilot position.
 * @return OFDM symbols
 */
std::vector<std::complex<double>>
create_ofdm_signal(const std::vector<std::complex<double>> &symbols,
                   const std::vector<cell_type> &grid,
                   std::complex<double> pilot_value);


/**
 * @brief Add Cyclic Prefix. This function copy in start OFDM symbol N samples from end. In RX side CP using for 
 * symbol sync (search start ofdm symbols with help correlation), frequency sync (search Coarse Frequency Offset with help
 * correlation). Also CP using for suppression ISI
 *
 * @param[in] samples ofdm_symbols in time domain.
 * @param[in] FFT_size ofdm symbol size.
 * @param[in] CP_size Cyclic Prefix size.

 * @return final OFDM symbols
 */
std::vector<std::complex<double>>
add_CP(const std::vector<std::complex<double>> &samples, const int FFT_size,
       const int CP_size);

/**
 * @brief Transition from frequency domain to time domain. This function receive vector with N OFDM symbols without separators, but
 * function independently separate vector on N batch with size FFT_size
 *
 * @param[in] data OFDM symbols in frequency domain.
 * @param[in] ifft_out vector, where writing result.
 * @param[in] FFT_size OFDM symbol size (batch size).
 * @param[in] CP_size Cyclic Prefix size.

 * @return final OFDM symbols
 */
void batch_ifft(std::vector<std::complex<double>> &data,
                std::vector<std::complex<double>> &ifft_out, const int FFT_size,
                const int CP_size);

/**
 * @brief Generate Zadov-Chu sequence for frame sync. This is special sequence, which have good correlation properties and 
 * have noise resistance. Sequence have size 63 samples, but padding with zeros if FFT_size > 63. ZC have size equal OFDM symbol size.
 *
 * @param[in] root Zadov-Chu parameters.
 * @param[in] FFT_size vector, where writing result.
 * @param[in] FFT_size OFDM symbol size (batch size).
 * @param[in] CP_size Cyclic Prefix size.

 * @return Zadov-Chu sequence
 */
std::vector<std::complex<double>> ZC_gen(const int root, const int FFT_size);

/**
 * @brief Add two ZC sequences (to start and end)to OFDM symbols
 *
 * @param[in] ofdm_samples ofdm symbols.
 * @param[in] ZC Zadov-Chu sequence.

 * @return Zadov-Chu sequence
 */
std::vector<std::complex<double>> add_ZC(const std::vector<std::complex<double>> &ofdm_samples, const std::vector<std::complex<double>> &ZC);

/**
 * @brief Run TX processing chain and update shared TX configuration.
 *
 * @param[in,out] config Shared TX configuration.
 */
void tx_run(tx_cfg &config);
