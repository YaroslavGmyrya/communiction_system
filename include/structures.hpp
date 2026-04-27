#pragma once

#include <complex>
#include <vector>
#include <cstdint>

/*
  enum for creating and work with OFDM grid (template for create OFDM symbols)

  grid[i] == 0 -> position for guard zero
  grid[i] == 1 -> position of data
  grid[i] == pilot -> position for pilot
*/
enum cell_type
{
    guard,
    data,
    pilot
};

/*Tx config structure. This parameters using in TX thread and GUI thread for drawing plots*/
struct tx_cfg
{
    bool run;        // for stop work
    bool DEBUG_MODE; // START/STOP debug mode (in CLI)

    int mod_order;    // 2-BPSK, 4-QPSK, 16-QAM16
    int FFT_size;     // subcarriers count
    int CP_size;      // Cyclic prefix size
    int pilots_count; // cout of pilots in one ofdm symbol
    int guard_size;   // count of guard symbols in one ofdm symbols (from each side)
    int padding;      // count of zeros in end of last OFDM symbol

    int max_padding_bits; // how many bits in start of first OFDM symbol contain padding in end of last OFDM symbol

    std::vector<std::complex<double>> zc; // Zadov-Chu sequence
    std::vector<cell_type> grid;          // OFDM grid (position guard zeros, pilots and data)

    std::string message; // tx message

    std::vector<uint8_t> bits;             // message in bits
    std::vector<uint8_t> post_conv_coding; // bits after convolution coding
    std::vector<uint8_t> post_shuffuling;  // bits post shuffuling

    std::vector<std::complex<double>> symbols; // M-PSK/M-QAM symbols

    std::vector<std::complex<double>> ofdm_symbols;    // OFDM symbols (with pilots, guard interval)
    std::vector<std::complex<double>> ofdm_signal;     // OFDM symbols in frequency domain
    std::vector<std::complex<double>> ofdm_symbols_cp; // final OFDM signal with CP

    std::complex<double> pilot = {1, 1}; // pilot value
};

/*Rx config structure. This parameters using in RX thread and GUI thread for drawing plots*/
struct rx_cfg
{
    bool run;                // for stop programm
    bool DEBUG_MODE = false; // START/STOP debug mode

    int mod_order; // 2-BPSK, 4-QPSK, 16-QAM16

    int FFT_size;     // subcarriers count
    int CP_size;      // Cyclic prefix size
    int pilots_count; // count of pilots in one OFDM symbol
    int guard_size;   // count of guard zeros in one OFDM symbol
    float SNR;        // Signal Noise Ration on RX side

    int channel_type;                                 // 0 is AWGN, 1 is COST207
    std::string profile = "TU50";                     // profile for COST207
    int sins = 33;                                    // count of sinusoids for generate rays in Jakes model
    std::vector<std::vector<std::complex<double>>> H; // channel matrix
    std::vector<double> t;                            // channel timeline
    std::vector<double> avg_E;                        // avg energy on rays

    int max_padding_bits; // how many bits in start of first OFDM symbol contain padding in end of last OFDM symbol

    double BER; // Bit Error Rate

    std::complex<double> pilot_value; // pilot value

    std::vector<cell_type> grid;          // OFDM grid (positions of guard zeros, data, pilots)
    std::vector<std::complex<double>> zc; // Zadov-Chu sequence

    std::vector<std::complex<double>> rx_samples;

    std::vector<double> zc_corr; // PSS correlation for frame sync
    std::vector<double> CP_corr; // CP correlation for symbol sync
    std::vector<int> CP_peaks;   // CP peaks (start index of ofdm symbols)

    std::vector<std::complex<double>> cut_samples; // samples post FRAME sync (without PSS)

    std::vector<std::complex<double>> ofdm_symbols; // samples without CP
    std::vector<std::complex<double>> estimation;   // channel estimation (with help pilots)

    std::vector<std::complex<double>> freq_domain; // symbols in frequency domain

    std::vector<std::complex<double>> raw_symbols; // symbols in frequency domain

    std::vector<uint8_t> bits;

    std::string message;
};