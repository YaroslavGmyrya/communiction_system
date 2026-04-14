#include <iostream>
#include <mutex>
#include <thread>
#include <chrono>


#include "../../include/ImGUI_interface.hpp"
#include "../../include/find_peaks.hpp"
#include "../../include/rx_lib.hpp"
#include "../../include/tx_lib.hpp"

void rx_run(rx_cfg &config, const tx_cfg &tx_config)
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
      config.rx_samples = tx_config.ofdm_symbols_cp;


    if (config.rx_samples.size() == 0)
      continue;

    std::cout << config.rx_samples.size() << "\n\n";

    /*===================================================== FRAME SYNC ==========================================================================*/
    auto start = std::chrono::high_resolution_clock::now();
    /*get correlation function on PSS*/
    config.zc_corr = ZC_corr(config.rx_samples, config.zc);
    auto end = std::chrono::high_resolution_clock::now();

    auto duration_us =
        std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    std::cout << "Time: " << duration_us.count() << " us" << std::endl;
    std::cout << "Time: " << duration_us.count() / 1000.0 << " ms" << std::endl;

    /*find correlation peaks*/
    findPeaks::PeakConditions conditions;
    conditions.set_height(0.9); // min peak value (filter)
    std::vector<int> zc_peaks = findPeaks::find_peaks(config.zc_corr, conditions);

    if (zc_peaks.size() != 2)
      continue;

    /*cut signal (extract useful signal)*/
    const int start_idx = zc_peaks[0] + config.FFT_size;
    const int end_idx = zc_peaks[1] - config.CP_size;
    const int range = end_idx - start_idx;

    config.cut_samples.resize(range);

    for (int i = start_idx; i < end_idx; ++i)
    {
      config.cut_samples[i-start_idx] = config.rx_samples[i];
    }

    std::cout << config.cut_samples.size() << "\n\n";

    /*===================================================== SYM SYNC ==========================================================================*/

    /*Get correlation function on CP*/
    config.CP_corr = OFDM_corr_receiving(config.cut_samples, config.FFT_size, config.CP_size, 0);

      config.CP_corr.insert(config.CP_corr.begin(), 1, 0);
    config.CP_corr.insert(config.CP_corr.end(), 1, 0);


    /*find peaks*/
    conditions.set_height(0.9);                                // min correlation value
    conditions.set_distance(config.FFT_size + config.CP_size); // min distance bw peaks (ofdm symbol size)
    config.CP_peaks = findPeaks::find_peaks(config.CP_corr, conditions);
    

    for(int &el : config.CP_peaks){
      el -= 1;
      // std::cout << el << " ";
    }

    // config.CP_peaks[config.CP_peaks.size() - 1] += 1;

    std::cout << "peaks: " << config.CP_peaks.size() << "\n\n";

    // /*estimate coarse frequency offset with help correlation*/
    // // CFO_correction(config.cut_samples, config.CP_peaks, config.CP_corr, config.CP_size, config.FFT_size);

    // /*delete CP*/
    config.ofdm_symbols = delete_CP(config.cut_samples, config.CP_peaks, config.CP_size, config.FFT_size);

    std::cout << "POST CP: " << config.ofdm_symbols.size() << "\n\n";

    // /*time domain -> frequency domain*/
    batch_fft(config.ofdm_symbols, config.freq_domain, config.FFT_size);


  //  for(int i = 0; i < config.freq_domain.size(); ++i){
  //     if(i % (config.FFT_size) == 0)
  //     printf("\n\n");
  //     std::cout << config.freq_domain[i] << " ";
  //   }

    std::cout << config.freq_domain.size() << "\n\n";

    // /*=============================================================== CHANNEL ESTIMATION ===================================================================================*/

    // /*get channel estimation with help pilots*/
    // config.estimation = channel_estimation(config.ofdm_symbols, config.grid, config.pilot_value);

    // /*recovery signal*/
    // channel_equalization(config.ofdm_symbols, config.estimation);

    // /*delete guard zeros and pilots. Extract data symbols*/
    config.raw_symbols = extract_inner_symbols(config.freq_domain, config.grid, 0);

    // for(int i = 0; i < config.raw_symbols.size(); ++i){
    //   if(i % (config.FFT_size - config.guard_size*2 - config.pilots_count) == 0)
    //   printf("\n\n");
    //   std::cout << config.raw_symbols[i] << " ";
    // }

    std::cout << config.raw_symbols.size() << "\n\n";

    /*symbols -> bits*/
    config.bits = BPSK_demodulator(config.raw_symbols);

    //    for(auto &el : config.bits){
    //   printf("%d ", el);
    // }

    // std::cout << "\n\n";

    // for(auto &el : tx_config.bits){
    //   printf("%d ", el);
    // }

    // std::cout << config.bits.size() << "\n\n";
    // std::cout << tx_config.bits.size() << "\n\n";


    config.BER = BER(config.bits, tx_config.bits);

    std::cout << "\n\nBER: " << config.BER << "\n\n";

    /*deshuffuling bits*/
    // config.bits = deintervale(config.bits, config.seed);

    /*bits -> message*/
    // config.message = decoder(config.bits);

  std::this_thread::sleep_for(std::chrono::seconds(5));

  }
}
