#include <algorithm>
#include <iostream>

#include <fftw3.h>
#include <spdlog/spdlog.h>

#include "../../include/ImGUI_interface.hpp"

using sample = std::complex<double>;

// std::vector<cell_type> create_ofdm_grid(const int FFT_size,
//                                         const int pilots_count,
//                                         const int gi_size)
// {
//     // create grid and fill her by data cell
//     std::vector<cell_type> grid(FFT_size, data);

//     // fill left/right guard
//     for (int i = 0; i < gi_size; ++i)
//     {
//         grid[i] = guard;
//         grid[grid.size() - i - 1] = guard;
//     }

//     /*compute space b/w pilots. Each symbol has pilots on the sides, and in
//     the
//      * center the pilots are distributed evenly*/
//     double pilot_step = double(FFT_size - 2 * gi_size - 1) / (pilots_count -
//     1);

//     int pilot_pos;

//     // fill pilots
//     for (int i = 0; i < pilots_count; ++i)
//     {
//         pilot_pos = gi_size + std::lround(i * pilot_step);
//         grid[pilot_pos] = pilot;
//     }

//     return grid;
// }

std::vector<double>
OFDM_corr_receiving(const std::vector<std::complex<double>> &samples,
                    const int FFT_size, const int CP_size, const int padding)
{
  std::vector<double> norm_corr;

  /*offset for prevention out of range*/
  const int offset = FFT_size + CP_size - 1;

  /*length of vectors (for normalization)*/
  double A = 0;
  double B = 0;

  /*cur correlation*/
  std::complex<double> corr = 0;

  /*
    Compute full correlation for first symbol.
    if CP element have index k, then original element in end of OFDM symbol have
    index [k+FFT_size]
  */
  for (int k = 0; k < CP_size; ++k)
  {
    corr += samples[k] * std::conj(samples[k + FFT_size]);

    /*|vector| = sqrt(vector[0] ^ 2 + vector[1] ^ 2 ...)*/
    A += std::abs(samples[k]) * std::abs(samples[k]);
    B += std::abs(samples[k + FFT_size]) * std::abs(samples[k + FFT_size]);
  }

  double denom = std::sqrt(A * B);
  norm_corr.push_back(denom > 0 ? std::abs(corr) / denom : 0);

  /*
  update correlation
  We compute correlation b/w two sliding windows. In next iteration we capture
  one element from right side, but loss one element from left side. This means,
  that we can compute correlation recursively:

  corr[n+1] = corr[n-1] + right - left

  For our case:

  corr[k+1] = corr[k] -  samples[k - 1] * conj(samples[k + FFT_size - 1]) +
  samples[k + CP_size - 1] * conj(samples[k + FFT_size + CP_size - 1])

  corr[k] - prev correlation,
  samples[k - 1] * conj(samples[k + FFT_size - 1]) - correlation for loss
  element (left side) samples[k + FFT_size - 1] * conj(samples[k + FFT_size]) -
  correlation for new element (right side)
  */

  for (int k = 1; k < samples.size() - offset; ++k)
  {
    corr = corr - samples[k - 1] * std::conj(samples[k + FFT_size - 1]) +
           samples[k + CP_size - 1] *
               std::conj(samples[k + FFT_size + CP_size - 1]);

    A = A - std::norm(samples[k - 1]) + std::norm(samples[k + CP_size - 1]);

    B = B - std::norm(samples[k + FFT_size - 1]) +
        std::norm(samples[k + FFT_size + CP_size - 1]);

    denom = std::sqrt(A * B);
    norm_corr.push_back(denom > 0 ? std::abs(corr) / denom : 0);
  }

  return norm_corr;
}

// #include <vector>
// #include <complex>
// #include <cmath>

// std::vector<double>
// OFDM_corr_receiving(const std::vector<std::complex<double>> &samples,
//                     const int FFT_size, const int CP_size)
// {
//     std::vector<double> norm_corr;

//     if (FFT_size <= 0 || CP_size <= 0)
//         return norm_corr;

//     const size_t offset = static_cast<size_t>(FFT_size + CP_size);

//     if (samples.size() < offset)
//         return norm_corr;

//     std::complex<double> corr = 0.0;
//     double A = 0.0;
//     double B = 0.0;

//     for (int k = 0; k < CP_size; ++k)
//     {
//         corr += samples[k] * std::conj(samples[k + FFT_size]);
//         A += std::norm(samples[k]);
//         B += std::norm(samples[k + FFT_size]);
//     }

//     {
//         double denom = std::sqrt(A * B);
//         norm_corr.push_back(denom > 0.0 ? std::abs(corr) / denom : 0.0);
//     }

//     for (size_t k = 1; k < samples.size() - offset; ++k)
//     {
//         corr = corr - samples[k - 1] * std::conj(samples[k + FFT_size - 1]) +
//         samples[k + CP_size - 1] * std::conj(samples[k + FFT_size + CP_size -
//         1]);

//         A = A - std::norm(samples[k - 1]) + std::norm(samples[k + CP_size -
//         1]); B = B - std::norm(samples[k + FFT_size - 1]) +
//         std::norm(samples[k + FFT_size + CP_size - 1]);

//         double denom = std::sqrt(A * B);
//         norm_corr.push_back(denom > 0.0 ? std::abs(corr) / denom : 0.0);
//     }

//     return norm_corr;
// }

void CFO_correction(std::vector<std::complex<double>> &samples,
                    const std::vector<int> &peaks,
                    const std::vector<double> &correlation, const int CP_size,
                    const int FFT_size)
{
  /*We can estimate coarse frequency offset b/w SDR.
    1. Take correlation peak (eps)
    2. CFO = -2pi*eps*k/CP_size
    3. Recovery signal: samples *= e^{i*CFO}
  */

  int peak_idx;
  double eps;

  for (int i = 0; i < peaks.size(); ++i)
  {
    /*step 1*/
    peak_idx = peaks[i];

    /*step 2*/
    eps = correlation[peak_idx];

    /*step 3*/
    for (int k = 0; k < FFT_size + CP_size; ++k)
    {
      /*
        peaks_idx - offset in buffer
        k - offset in symbol
      */
      samples[peak_idx + k] *=
          std::exp(std::complex<double>(0, -2 * M_PI * eps * k / CP_size));
    }
  }
}

std::vector<std::complex<double>>
delete_CP(const std::vector<std::complex<double>> &samples,
          const std::vector<int> &peaks, const int CP_size,
          const int FFT_size)
{
  /*check errors*/
  if (samples.size() < FFT_size)
  {
    return {};
  }

  std::vector<std::complex<double>> result;

  /*size without CP*/
  result.reserve(peaks.size() * FFT_size);

  for (int i = 0; i < peaks.size(); ++i)
  {
    int peak = peaks[i];

    /*start iterator of OFDM symbol*/
    auto start = samples.begin() + peak + CP_size;

    /*end iterator of OFDM symbol*/
    auto end = samples.begin() + peak + CP_size + FFT_size;

    result.insert(result.end(), start, end);
  }

  return result;
}

std::vector<int> get_pilots_pos(const std::vector<cell_type> &grid)
{

  std::vector<int> pos;

  for (int i = 0; i < grid.size(); ++i)
  {
    if (grid[i] == pilot)
    {
      pos.push_back(i);
    }
  }

  return pos;
}

void unwrap_phase(std::vector<double> &phase, int FFT_size)
{
  int total_size = phase.size();
  int symbs_count = total_size / FFT_size;

  for (int s = 0; s < symbs_count; ++s)
  {
    double offset = 0.0;
    int prev_idx = -1;

    int start = s * FFT_size;
    int end = start + FFT_size;

    for (int i = start; i < end; ++i)
    {
      if (phase[i] == 0.0)
        continue;

      if (prev_idx == -1)
      {
        prev_idx = i;
        continue;
      }

      double d = phase[i] - phase[prev_idx];

      if (d > M_PI)
        offset -= 2 * M_PI;
      else if (d < -M_PI)
        offset += 2 * M_PI;

      phase[i] += offset;

      prev_idx = i;
    }
  }
}

void linear_interpolation(std::vector<std::complex<double>> &H,
                          const std::vector<int> &pos, int FFT_size)
{
  if (pos.size() < 2)
    return;

  int sym = 0;
  bool flag = true;

  while (flag)
  {
    for (int i = 0; i < pos.size() - 1; ++i)
    {
      int left_index = pos[i] + sym * FFT_size;
      int right_index = pos[i + 1] + sym * FFT_size;

      if (left_index >= H.size() || right_index >= H.size())
      {
        flag = false;
        break;
      }

      std::complex<double> left_point = H[left_index];
      std::complex<double> right_point = H[right_index];

      for (int idx = left_index + 1; idx < right_index; ++idx)
      {
        double coeff = static_cast<double>(idx - left_index) /
                       static_cast<double>(right_index - left_index);

        H[idx] = left_point + coeff * (right_point - left_point);
      }
    }

    ++sym;
  }
}

void linear_interpolation2(std::vector<double> &H, const std::vector<int> &pos,
                           int FFT_size)
{
  if (pos.size() < 2)
    return;

  int sym = 0;
  bool flag = true;

  while (flag)
  {
    for (int i = 0; i < pos.size() - 1; ++i)
    {
      int left_index = pos[i] + sym * FFT_size;
      int right_index = pos[i + 1] + sym * FFT_size;

      if (left_index >= H.size() || right_index >= H.size())
      {
        flag = false;
        break;
      }

      double left_point = H[left_index];
      double right_point = H[right_index];

      for (int idx = left_index + 1; idx < right_index; ++idx)
      {
        double coeff = static_cast<double>(idx - left_index) /
                       static_cast<double>(right_index - left_index);

        H[idx] = left_point + coeff * (right_point - left_point);
      }
    }

    ++sym;
  }
}

std::vector<std::complex<double>>
channel_estimation(std::vector<std::complex<double>> &signal,
                   const std::vector<cell_type> &grid,
                   std::complex<double> pilot_value)
{
  /*get pilots position from ofdm grid*/
  std::vector<int> pilots_pos = get_pilots_pos(grid);

  const int FFT_size = grid.size();

  const int symbs_count = signal.size() / FFT_size;

  /*total estimation*/
  std::vector<std::complex<double>> estimation(FFT_size * symbs_count);

  /*amplitude and phase of estimation*/
  std::vector<double> A(FFT_size * symbs_count);
  std::vector<double> phi(FFT_size * symbs_count);

  std::complex<double> cur_estimation;

  /*iterate on symbols*/
  for (int i = 0; i < symbs_count; ++i)
  {
    /*iterate inside symbol*/
    for (int j = 0; j < pilots_pos.size(); ++j)
    {
      /*estimate channel influence on pilot (estimate channel)*/
      cur_estimation = signal[pilots_pos[j] + i * FFT_size] / pilot_value;

      /*separate estimation on amplitude and phase*/
      A[pilots_pos[j] + i * FFT_size] = std::abs(cur_estimation);
      phi[pilots_pos[j] + i * FFT_size] = std::arg(cur_estimation);
    }
  }

  /*unwrap phase for correct interpolation (delete phase jump)*/
  unwrap_phase(phi, FFT_size);

  linear_interpolation2(A, pilots_pos, FFT_size);

  linear_interpolation2(phi, pilots_pos, FFT_size);

  /*build estimation from amplitude and phase*/
  for (int i = 0; i < A.size(); ++i)
  {
    estimation[i] = A[i] * std::exp(std::complex<double>(0.0, phi[i]));
  }

  return estimation;
}

void channel_equalization(std::vector<std::complex<double>> &symbols,
                          const std::vector<std::complex<double>> &estimation)
{
  /*Recovery signal after channel influence*/
  for (int i = 0; i < symbols.size(); ++i)
  {
    /*check on machine zero*/
    if (std::abs(estimation[i]) > 1e-12)
      symbols[i] /= estimation[i];
  }
}

std::vector<std::complex<double>>
extract_inner_symbols(const std::vector<std::complex<double>> &ofdm_symbols,
                      const std::vector<cell_type> &grid, const int padding)
{
  std::vector<std::complex<double>> clear_symbols;
  int k = 0;
  bool flag = true;

  while (flag)
  {
    for (int i = 0; i < grid.size(); ++i)
    {
      /*check out of range*/
      if (i + k * grid.size() >= ofdm_symbols.size())
      {
        flag = false;
        break;
      }

      /*extract symbols with data only*/
      if (grid[i] == data && std::abs(ofdm_symbols[i + k * grid.size()]) > 1e-12)
      {
        clear_symbols.push_back(ofdm_symbols[i + k * grid.size()]);
      }
    }

    k++;
  }
  // clear_symbols.resize(clear_symbols.size() - padding);
  return clear_symbols;
}

void batch_fft(std::vector<std::complex<double>> &data,
               std::vector<std::complex<double>> &fft_out, int FFT_size)
{

  /*check errors*/

  if (data.size() == 0)
  {
    spdlog::error("[OFDM.cpp:batch_ifft]: The data size is zero!");
    return;
  }

  if (FFT_size <= 0)
  {
    spdlog::error("[OFDM.cpp:batch_ifft]: The FFT_size size is invalid!");
    return;
  }

  if (data.size() % FFT_size != 0)
  {
    spdlog::error("[OFDM.cpp:batch_ifft]: Fractional number of OFDM symbols!");
    return;
  }

  /*count of OFDM symbols*/
  const int howmany = data.size() / FFT_size;

  /*OFDM symbol size*/
  const int n[] = {FFT_size};

  fft_out.clear();
  fft_out.resize(FFT_size * howmany);

  /*create FFTW3 complex number (сompatibility with std::complex)*/
  fftw_complex *in = reinterpret_cast<fftw_complex *>(data.data());
  fftw_complex *out = reinterpret_cast<fftw_complex *>(fft_out.data());

  /*plan is set of settings for FFT
    1 - rank (1D, 2D ...),
    n - size of batch (FFT_size),
    in - fftw_complex array with OFDM in time domain,
    FFT_size - space b/w batches,
    out - output array with FFT result,
    FFTW_FORWARD - flag for FFT (IFFT have flag BACKWARD)
  */
  fftw_plan plan =
      fftw_plan_many_dft(1, n, howmany, in, nullptr, 1, FFT_size, out, nullptr,
                         1, FFT_size, FFTW_FORWARD, FFTW_ESTIMATE);

  /*calculate FFT*/
  fftw_execute(plan);

  /*delete FFTW3 plan*/
  fftw_destroy_plan(plan);
}

std::vector<double>
ZC_corr(const std::vector<std::complex<double>>& samples,
                  const std::vector<std::complex<double>>& zc)
{
    std::vector<double> norm_corr;

    const size_t N = samples.size();
    const size_t M = zc.size();

    if (M == 0 || N < M)
        return norm_corr;

    const size_t num_positions = N - M + 1;
    norm_corr.reserve(num_positions);

    /* energy of ZC sequence */
    double B = 0.0;
    for (size_t k = 0; k < M; ++k)
        B += std::norm(zc[k]);

    /* energy of first received window */
    double A = 0.0;
    for (size_t k = 0; k < M; ++k)
        A += std::norm(samples[k]);

    /* full correlation for first window */
    std::complex<double> corr = 0.0;
    for (size_t k = 0; k < M; ++k)
        corr += samples[k] * std::conj(zc[k]);

    double denom = std::sqrt(A * B);
    norm_corr.push_back(denom > 0.0 ? std::abs(corr) / denom : 0.0);

    /*
      For next positions:
      - update A recursively as sliding window energy
      - recompute corr for new window
    */
    for (size_t n = 1; n < num_positions; ++n)
    {
        A = A - std::norm(samples[n - 1]) + std::norm(samples[n + M - 1]);

        corr = 0.0;
        for (size_t k = 0; k < M; ++k)
            corr += samples[n + k] * std::conj(zc[k]);

        denom = std::sqrt(A * B);
        norm_corr.push_back(denom > 0.0 ? std::abs(corr) / denom : 0.0);
    }

    return norm_corr;
}

void CFO_correction(std::vector<sample> &samples, const std::vector<sample> &corr_function, const std::vector<int> &peaks, const double Ts)
{
  double cfo = std::arg(corr_function[peaks[1]] * std::conj(corr_function[peaks[0]]));
  int range = peaks[1] - peaks[0];

  cfo = cfo / (2 * M_PI * range * Ts);

  for (int i = 0; i < samples.size(); ++i)
  {
    double phi = 2.0 * M_PI * cfo * i * Ts;
    std::complex<double> rot = std::exp(std::complex<double>(0.0, -phi));
    samples[i] *= rot;
  }
}

double BER(const std::vector<uint8_t>& rx_bits,  const std::vector<uint8_t>& tx_bits){
  if(rx_bits.size() != tx_bits.size()){
    spdlog::error("RX_BITS and TX_BITS must be have same size!");
    return -1;
  }

  double BER = 0;

  for(int i = 0; i < rx_bits.size(); ++i){
    BER += rx_bits[i] == tx_bits[i] ? 0 : 1;
  }

  BER /= rx_bits.size();

  return BER;

}