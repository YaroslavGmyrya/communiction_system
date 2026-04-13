#pragma once

#include <complex>
#include <cstdint>
#include <string>
#include <vector>

#include "ImGUI_interface.hpp"

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

std::vector<uint8_t> BPSK_demodulator(const std::vector<sample> &symbols);

std::vector<double>
OFDM_corr_receiving(const std::vector<std::complex<double>> &samples,
                    int FFT_size, int CP_size, const int padding);

void batch_fft(std::vector<std::complex<double>> &data,
               std::vector<std::complex<double>> &fft_out, int FFT_size);

std::vector<std::complex<double>>
delete_CP(const std::vector<std::complex<double>> &samples,
          const std::vector<int> &peaks, const int CP_size, const int FFT_size);

void CFO_correction(std::vector<std::complex<double>> &samples,
                    const std::vector<int> &peaks,
                    const std::vector<double> &correlation, int CP_size,
                    int FFT_size);

std::vector<cell_type> create_ofdm_grid(int FFT_size, int pilots_count,
                                        int gi_size);

std::vector<int> get_pilots_pos(const std::vector<cell_type> &grid);

void linear_interpolation(std::vector<std::complex<double>> &H,
                          const std::vector<int> &pos, int FFT_size);

void linear_interpolation2(std::vector<double> &H, const std::vector<int> &pos,
                           int FFT_size);

void unwrap_phase(std::vector<double> &phase, int FFT_size);

std::vector<std::complex<double>>
channel_estimation(std::vector<std::complex<double>> &signal,
                   const std::vector<cell_type> &grid,
                   std::complex<double> pilot_value);

void channel_equalization(std::vector<std::complex<double>> &symbols,
                          const std::vector<std::complex<double>> &estimation);

std::vector<std::complex<double>>
extract_inner_symbols(const std::vector<std::complex<double>> &ofdm_symbols,
                      const std::vector<cell_type> &grid, const int padding);

void rx_run(rx_cfg &config,
            const tx_cfg &tx_config);

std::vector<double> ZC_corr(const std::vector<std::complex<double>> &samples, const std::vector<std::complex<double>> &ZC);

double BER(const std::vector<uint8_t>& rx_bits,  const std::vector<uint8_t>& tx_bits);