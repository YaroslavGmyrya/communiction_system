#pragma once

#include <complex>
#include <vector>

#include <GL/glew.h>
#include <SDL2/SDL.h>

#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_sdl2.h"
#include "imgui.h"
#include "implot.h"

enum cell_type
{
  guard,
  data,
  pilot
};

struct tx_cfg
{
  bool run; // for stop work
  int bitrate;
  int mod_order; // 2-BPSK, 4-QPSK, 16-QAM16
  int sps;       // samples per symbol
  int IR_type;   // 0-Rectangle, 1-Raised-Cosine
  int OFDM;      // 0 - OFDM 0FF, 1 - OFDM ON
  int FFT_size;  // subcarriers count
  int CP_size;   // Cyclic prefix size
  int count_OFDM_symb;
  int count_bits;
  int buff_size;
  int pilots_count;
  int guard_size;
  bool DEBUG_MODE;
  int seed = 10;

  std::vector<uint8_t> bits;
  std::vector<uint8_t> post_conv_coding;
  std::vector<uint8_t> post_shuffuling;
  std::vector<std::complex<double>> symbols;
  std::vector<std::complex<double>> ofdm_symbols;
  std::vector<std::complex<double>> ofdm_symbols_cp;
  std::vector<std::complex<double>> zc;

  std::vector<std::complex<double>> ofdm_signal;

  std::complex<double> pilot = {1, 1};

  std::vector<std::complex<int16_t>> tx_samples;
  std::vector<cell_type> grid;

  std::string message;
};

struct rx_cfg
{
  bool run;

  int mod_order; // 2-BPSK, 4-QPSK, 16-QAM16
  int sps;       // samples per symbol
  int IR_type;   // 0-Rectangle, 1-Raised-Cosine
  int OFDM;      // 0 - OFDM 0FF, 1 - OFDM ON
  int FFT_size;  // subcarriers count
  int CP_size;   // Cyclic prefix size
  int count_OFDM_symb;

  // gardner params
  double gardner_BnTs;
  double gardner_Kp;

  // costas params
  double costas_Kp;
  double costas_BnTs;

  int pilots_count;
  int guard_size;

  int seed = 10;

  std::complex<double> pilot_value;

  // buffers
  std::vector<std::complex<int16_t>> rx_samples;
  std::vector<std::complex<double>> mf_samples_out;
  std::vector<std::complex<double>> raw_symbols;
  std::vector<double> CP_corr;
  std::pair<std::vector<std::complex<double>>, std::vector<double>> spectrum;
  std::pair<std::vector<std::complex<double>>, std::vector<double>>
      CFO_spectrum;
  std::pair<std::vector<std::complex<double>>, std::vector<double>>
      post_CFO_spectrum;
  std::vector<std::complex<double>> post_cfo_signal;
  std::pair<std::vector<std::complex<double>>, std::vector<double>>
      post_fine_CFO_spectrum;
  std::vector<std::complex<double>> post_costas;

  std::vector<int> CP_peaks;
  std::vector<std::complex<double>> ofdm_symbols;
  std::vector<std::complex<double>> freq_domain;
  std::vector<std::complex<double>> estimation;
  std::string message;
  std::vector<uint8_t> bits;
  std::vector<cell_type> grid;
  std::vector<std::complex<double>> zc;
  std::vector<std::complex<double>> cut_samples;
  std::vector<double> zc_corr;
};

void run_gui(tx_cfg &tx_config, rx_cfg &rx_config);
