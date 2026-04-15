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

/**
 * @file demodulation.cpp
 * @brief Translate BPSK symbols to bits
 *
 * @param[in] symbols symbols after deshuffuling
 * @return Bits
 **/
std::vector<uint8_t> BPSK_demodulator(const std::vector<sample> &symbols);

/**
 * @file OFDM_rx.cpp
 * @brief Compute CP correlation function for symbol sync
 *
 * @param[in] samples OFDM symbols
 * @param[in] FFT_size OFDM symbol size
 * @param[in] CP_size Cyclic Prefix size

 * @return Correlation function
 **/
std::vector<double>
OFDM_corr_receiving(const std::vector<std::complex<double>> &samples,
                    int FFT_size, int CP_size);

/**
 * @file OFDM_rx.cpp
 * @brief Compute FFT. Transition from time domain to frequency domain
 *
 * @param[in] data OFDM symbols without CP
 * @param[in] fft_out result
 * @param[in] FFT_size OFDM symbol size

 * @return OFDM symbols in frequency domain
 **/
void batch_fft(std::vector<std::complex<double>> &data,
               std::vector<std::complex<double>> &fft_out, int FFT_size);

/**
 * @file OFDM_rx.cpp
 * @brief Compute FFT. Transition from time domain to frequency domain
 *
 * @param[in] data OFDM symbols without CP
 * @param[in] fft_out result
 * @param[in] FFT_size OFDM symbol size

 * @return OFDM symbols without CP
 **/
std::vector<std::complex<double>>
delete_CP(const std::vector<std::complex<double>> &samples,
          const std::vector<int> &peaks, const int CP_size, const int FFT_size);


void CFO_correction(std::vector<std::complex<double>> &samples,
                    const std::vector<int> &peaks,
                    const std::vector<double> &correlation, int CP_size,
                    int FFT_size);

/**
 * @file OFDM_rx.cpp
 * @brief Create OFDM grid. She used for extracting payload data from OFDM symbols and deleting overheads
 *
 * @param[in] FFT_size OFDM symbol size
 * @param[in] pilots_count count of pilots in one OFDM symbol
 * @param[in] gi_size count of guard zeros in one OFDM symbol

 * @return OFDM grid
 **/                  
std::vector<cell_type> create_ofdm_grid(int FFT_size, int pilots_count,
                                        int gi_size);


/**
 * @file OFDM_rx.cpp
 * @brief Get pilots position from OFDM grid for channel estimation
 *
 * @param[in] grid OFDM grid

 * @return Vector with pilots pos
 **/                                              
std::vector<int> get_pilots_pos(const std::vector<cell_type> &grid);



/**
 * @file OFDM_rx.cpp
 * @brief Simple linear interpolation. Used for interpolation channel estimation between pilots
 *
 * @param[in] H channel estimation
 * @param[in] pos pilots positions
 * @param[in] FFT_size OFDM symbol size

 * @return Vector with pilots pos
 **/     
void linear_interpolation(std::vector<std::complex<double>> &H,
                          const std::vector<int> &pos, int FFT_size);


void unwrap_phase(std::vector<double> &phase, int FFT_size);


/**
 * @file OFDM_rx.cpp
 * @brief Simple linear interpolation. Used for interpolation channel estimation between pilots
 *
 * @param[in] H channel estimation
 * @param[in] pos pilots positions
 * @param[in] FFT_size OFDM symbol size

 * @return Vector with pilots pos
 **/     
std::vector<std::complex<double>>
channel_estimation(std::vector<std::complex<double>> &signal,
                   const std::vector<cell_type> &grid,
                   std::complex<double> pilot_value);


/**
 * @file OFDM_rx.cpp
 * @brief Simple linear interpolation. Used for interpolation channel estimation between pilots
 *
 * @param[in] H channel estimation
 * @param[in] pos pilots positions
 * @param[in] FFT_size OFDM symbol size

 * @return Vector with pilots pos
 **/     
void channel_equalization(std::vector<std::complex<double>> &symbols,
                          const std::vector<std::complex<double>> &estimation);


/**
 * @file OFDM_rx.cpp
 * @brief Extract M-PSK/M-QAM symbols from OFDM symbols
 *
 * @param[in] samples OFDM symbols
 * @param[in] ZC Zadov-Chu sequence

 * @return BER
 **/  
std::vector<std::complex<double>>
extract_inner_symbols(const std::vector<std::complex<double>> &ofdm_symbols,
                      const std::vector<cell_type> &grid, const int padding);



/**
 * @file OFDM_rx.cpp
 * @brief Compute PSS correlation for frame sync
 *
 * @param[in] samples OFDM symbols
 * @param[in] ZC Zadov-Chu sequence

 * @return BER
 **/  
std::vector<double> ZC_corr(const std::vector<std::complex<double>> &samples, const std::vector<std::complex<double>> &ZC);

/**
 * @file OFDM_rx.cpp
 * @brief Compute BER
 *
 * @param[in] rx_bits
 * @param[in] tx_bits

 * @return BER
 **/     
double BER(const std::vector<uint8_t>& rx_bits,  const std::vector<uint8_t>& tx_bits);


/**
 * @file OFDM_rx.cpp
 * @brief Compute BER
 *
 * @param[in] config rx config 
 * @param[in] tx_config tx config

 * @return RX side DSP
 **/  
void rx_run(rx_cfg &config, const tx_cfg &tx_config);
