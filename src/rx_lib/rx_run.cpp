#include <iostream>
#include <mutex>

#include "../../include/ImGUI_interface.hpp"
#include "../../include/find_peaks.hpp"
#include "../../include/rx_lib.hpp"

void rx_run(rx_cfg &config,
            const std::vector<std::complex<double>> &rx_samples) {
  std::vector<std::complex<double>> samples;

  while (config.run) {

    samples = rx_samples;

    if (samples.size() == 0)
      continue;

    /*correlation receiving. Get correlation function*/
    config.corr_func =
        OFDM_corr_receiving(samples, config.FFT_size, config.CP_size);

    /*find peaks*/
    findPeaks::PeakConditions conditions;

    conditions.set_height(0.9);
    // conditions.set_prominence(0.5);
    conditions.set_distance(config.FFT_size + config.CP_size);

    config.peaks = findPeaks::find_peaks(config.corr_func, conditions);

    // std::cout << "\n\n\n";
    // for (int i = 0; i < config.peaks.size(); ++i)
    // {
    //     std::cout << config.peaks[i] << " ";
    // }

    /*estimate coarse frequency offset with help correlation*/
    CFO_correction(samples, config.peaks, config.corr_func, config.CP_size,
                   config.FFT_size);

    /*delete CP*/
    config.ofdm_symbols =
        delete_CP(samples, config.peaks, config.CP_size, config.FFT_size);

    /*time domain -> frequency domain*/
    batch_fft(config.ofdm_symbols, config.freq_domain, config.FFT_size);

    /*create ofdm grid*/
    config.grid = create_ofdm_grid(config.FFT_size, config.pilots_count,
                                   config.guard_size);

    /*get channel estimation with help pilots*/
    config.estimation = channel_estimation(config.ofdm_symbols, config.grid,
                                           config.pilot_value);

    /*recovery signal*/
    channel_equalization(config.ofdm_symbols, config.estimation);

    /*delete guard zeros and pilots. Extract data symbols*/
    config.raw_symbols =
        extract_inner_symbols(config.ofdm_symbols, config.grid);

    std::cout << "\n\n\n";
    for (int i = 0; i < config.raw_symbols.size(); ++i) {
      std::cout << config.raw_symbols[i] << " ";
    }
  }
}