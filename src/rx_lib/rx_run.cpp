#include <iostream>
#include <mutex>

#include "../../include/ImGUI_interface.hpp"
#include "../../include/find_peaks.hpp"
#include "../../include/rx_lib.hpp"
#include "../../include/tx_lib.hpp"

void rx_run(rx_cfg &config, const std::vector<std::complex<double>> &rx_samples)
{

  /*generate ZC-sequence*/
  std::vector<std::complex<double>> tmp_zc = ZC_gen(25, config.FFT_size);

  /*ZC to time domain*/
  batch_ifft(tmp_zc, config.zc, config.FFT_size, config.CP_size);

  /*create ofdm grid*/
  config.grid = create_ofdm_grid(config.FFT_size, config.pilots_count,
                                 config.guard_size);

  while (config.run)
  {

    if (rx_samples.size() == 0)
      continue;

    /*===================================================== FRAME SYNC ==========================================================================*/
    /*get correlation function on PSS*/
    config.zc_corr = ZC_corr(rx_samples, config.zc);

    /*find correlation peaks*/
    findPeaks::PeakConditions conditions;
    conditions.set_height(0.9); // min peak value (filter)
    std::vector<int> zc_peaks = findPeaks::find_peaks(config.zc_corr, conditions);

    if (zc_peaks.size() != 2)
      continue;

    /*cut signal (extract useful signal)*/
    const int start_idx = zc_peaks[1] + config.FFT_size;
    const int end_idx = zc_peaks[2] - config.CP_size;

    config.cut_samples.reserve(end_idx - start_idx);

    for (int i = start_idx; i < end_idx; ++i)
    {
      config.cut_samples.push_back(rx_samples[i]);
    }

    /*===================================================== SYM SYNC ==========================================================================*/

    /*Get correlation function on CP*/
    config.CP_corr = OFDM_corr_receiving(config.cut_samples, config.FFT_size, config.CP_size, 0);

    /*find peaks*/
    conditions.set_height(0.9);                                // min correlation value
    conditions.set_distance(config.FFT_size + config.CP_size); // min distance bw peaks (ofdm symbol size)
    config.CP_peaks = findPeaks::find_peaks(config.CP_corr, conditions);

    /*estimate coarse frequency offset with help correlation*/
    // CFO_correction(config.cut_samples, config.CP_peaks, config.CP_corr, config.CP_size, config.FFT_size);

    /*delete CP*/
    config.ofdm_symbols = delete_CP(config.cut_samples, config.CP_peaks, config.CP_size, config.FFT_size);

    /*time domain -> frequency domain*/
    batch_fft(config.ofdm_symbols, config.freq_domain, config.FFT_size);

    /*=============================================================== CHANNEL ESTIMATION ===================================================================================*/

    /*get channel estimation with help pilots*/
    config.estimation = channel_estimation(config.ofdm_symbols, config.grid, config.pilot_value);

    /*recovery signal*/
    channel_equalization(config.ofdm_symbols, config.estimation);

    /*delete guard zeros and pilots. Extract data symbols*/
    config.raw_symbols = extract_inner_symbols(config.ofdm_symbols, config.grid, 0);

    /*symbols -> bits*/
    // config.bits = QPSK_demodulation(config.raw_symbols);

    /*deshuffuling bits*/
    // config.bits = deintervale(config.bits, config.seed);

    /*bits -> message*/
    // config.message = decoder(config.bits);
  }
}
