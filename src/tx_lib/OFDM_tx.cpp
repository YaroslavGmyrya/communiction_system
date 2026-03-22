#include <algorithm>
#include <iostream>

#include <fftw3.h>
#include <spdlog/spdlog.h>

#include "../../include/ImGUI_interface.hpp"

std::vector<cell_type> create_ofdm_grid(const int FFT_size,
                                        const int pilots_count,
                                        const int gi_size) {
  // create grid and fill her by data cell
  std::vector<cell_type> grid(FFT_size, data);

  // fill left/right guard
  for (int i = 0; i < gi_size; ++i) {
    grid[i] = guard;
    grid[grid.size() - i - 1] = guard;
  }

  /*
    compute space b/w pilots. Each symbol has pilots on the sides, and in the
    center the pilots are distributed evenly
   */
  double pilot_step = double(FFT_size - 2 * gi_size - 1) / (pilots_count - 1);

  int pilot_pos;

  // fill pilots
  for (int i = 0; i < pilots_count; ++i) {
    pilot_pos = gi_size + std::lround(i * pilot_step);
    grid[pilot_pos] = pilot;
  }

  return grid;
}

std::vector<std::complex<double>>
create_ofdm_signal(const std::vector<std::complex<double>> &symbols,
                   const std::vector<cell_type> &grid,
                   const std::complex<double> pilot_value,
                   const int buff_size) {
  /*buff size - size of buffer in SDR*/
  std::vector<std::complex<double>> signal;
  signal.reserve(buff_size);

  int k = 0;

  while (k < symbols.size()) {
    for (int j = 0; j < grid.size(); ++j) {

      if (k < symbols.size()) {

        /*if cell is guard, then signal is (0,0)*/
        if (grid[j] == guard) {
          signal.push_back({0, 0});
        }

        /*if cell is pilot, then insert pilot*/
        else if (grid[j] == pilot) {
          signal.push_back(pilot_value);
        }

        /*if cell is data, then insert symbol*/
        else if (grid[j] == data) {
          signal.push_back(symbols[k++]);
        }
      }

      /*if PSK/QAM symbols are over, then fill symbol with zeros*/
      else {
        signal.push_back({-2, 2});
      }
    }
  }

  return signal;
}

std::vector<std::complex<double>>
add_CP(const std::vector<std::complex<double>> &samples, const int FFT_size,
       const int CP_size) {
  const int symbols_count = samples.size() / FFT_size;

  /*symbols_count * CP_size - additional size for CP*/
  std::vector<std::complex<double>> result(samples.size() +
                                           symbols_count * CP_size);

  for (int i = 0; i < symbols_count; ++i) {
    /*insert position*/
    auto dst = result.data() + i * (CP_size + FFT_size);

    /*start elements for CP*/
    auto src = samples.data() + i * FFT_size;

    std::memcpy(dst, src + (FFT_size - CP_size),
                CP_size * sizeof(std::complex<double>));

    std::memcpy(dst + CP_size, src, FFT_size * sizeof(std::complex<double>));
  }

  return result;
}

void batch_ifft(std::vector<std::complex<double>> &data,
                std::vector<std::complex<double>> &ifft_out, const int FFT_size,
                const int CP_size) {

  /*check errors*/
  if (data.size() == 0) {
    spdlog::error("[OFDM.cpp:batch_ifft]: The data size is zero!");
    return;
  }

  if (FFT_size <= 0) {
    spdlog::error("[OFDM.cpp:batch_ifft]: The FFT_size size is invalid!");
    return;
  }

  if (data.size() % (FFT_size + CP_size) != 0) {
    spdlog::error("[OFDM.cpp:batch_ifft]: Fractional number of OFDM symbols!");
    return;
  }

  /*count of OFDM symbols*/
  const int howmany = data.size() / FFT_size;

  /*OFDM symbol size*/
  const int n[] = {FFT_size};

  ifft_out.clear();
  ifft_out.resize(FFT_size * howmany);

  /*create FFTW3 complex number (сompatibility with std::complex)*/
  fftw_complex *in = reinterpret_cast<fftw_complex *>(data.data());
  fftw_complex *out = reinterpret_cast<fftw_complex *>(ifft_out.data());

  /*plan is set of settings for FFT
    1 - rank (1D, 2D ...),
    n - size of batch (FFT_size),
    in - fftw_complex array with OFDM in frequency domain,
    FFT_size - space b/w batches,
    out - output array with IFFT result,
    FFTW_BACKWARD - flag for IFFT (FFT have flag FFTW_FORWARD)
  */
  fftw_plan plan =
      fftw_plan_many_dft(1, n, howmany, in, nullptr, 1, FFT_size, out, nullptr,
                         1, FFT_size, FFTW_BACKWARD, FFTW_ESTIMATE);

  /*calculate IFFT*/
  fftw_execute(plan);

  /*delete FFTW3 plan*/
  fftw_destroy_plan(plan);
}
