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

    /*===================================================== FRAME SYNC ==========================================================================*/

    /*get correlation function on PSS*/
    config.zc_corr = ZC_corr(config.rx_samples, config.zc);

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

    /*===================================================== SYM SYNC ==========================================================================*/

    /*Get correlation function on CP*/
    config.CP_corr = OFDM_corr_receiving(config.cut_samples, config.FFT_size, config.CP_size, 0);

    config.CP_corr.insert(config.CP_corr.begin(), 1, 0);
    config.CP_corr.insert(config.CP_corr.end(), 1, 0);

    /*find peaks*/
    conditions.set_height(0.9);                                // min correlation value
    conditions.set_distance(config.FFT_size + config.CP_size); // min distance bw peaks (ofdm symbol size)
    config.CP_peaks = findPeaks::find_peaks(config.CP_corr, conditions);

    for(auto& el : config.CP_peaks){
      el -= 1;
    }
  
    /*estimate coarse frequency offset with help correlation*/
    // CFO_correction(config.cut_samples, config.CP_peaks, config.CP_corr, config.CP_size, config.FFT_size);

    /*delete CP*/
    config.ofdm_symbols = delete_CP(config.cut_samples, config.CP_peaks, config.CP_size, config.FFT_size);

    /*time domain -> frequency domain*/
    batch_fft(config.ofdm_symbols, config.freq_domain, config.FFT_size);


    /*=============================================================== CHANNEL ESTIMATION ===================================================================================*/

    /*get channel estimation with help pilots*/
    config.estimation = channel_estimation(config.freq_domain, config.grid, config.pilot_value);

    /*recovery signal*/
    channel_equalization(config.ofdm_symbols, config.estimation);

    /*delete guard zeros and pilots. Extract data symbols*/
    config.raw_symbols = extract_inner_symbols(config.freq_domain, config.grid, 0);

    /*symbols -> bits*/
    config.bits = BPSK_demodulator(config.raw_symbols);

    config.BER = BER(config.bits, tx_config.bits);

    /*deshuffuling bits*/
    // config.bits = deintervale(config.bits, config.seed);

    /*bits -> message*/
    // config.message = decoder(config.bits);

    /*=========================================== DEBUG INFO ===========================================================*/

    if(config.DEBUG_MODE){
      std::cout << "============================= RX SIGNAL =========================================";

      std::cout << "\n\nValue: ";
      for(auto el : config.rx_samples)
        std::cout << el << " ";

      std::cout << "\n";

      std::cout << "SIZE: " << config.rx_samples.size() << " \n\n";

      std::cout << "============================= FRAME SYNC =========================================";

      std::cout << "\n\nPSS CORR FUNC VALUE: ";
      for(auto el : config.zc_corr)
        std::cout << el << " ";
      
      std::cout << "\n";

      std::cout << "PSS CORR FUNC SIZE: "  << config.zc_corr.size() << " \n\n";

      std::cout << "\n\n";

      std::cout << "PSS PEAKS VALUE: ";

      for(auto el : zc_peaks)
        std::cout << el << " ";

      std::cout << "\n";

      std::cout << "PSS PEAKS SIZE: " << zc_peaks.size() << "\n\n";

      std::cout << "CUT SAMPLES VALUE: ";

      for(auto el : config.cut_samples)
        std::cout << el << " ";

      std::cout << "\n";

      std::cout << "CUT SAMPLES SIZE: " << config.cut_samples.size() << "\n\n";

      std::cout << "============================= SYM SYNC =========================================";

      std::cout << "\n\nCP CORR FUNC VALUE: ";
      for(auto el : config.CP_corr)
        std::cout << el << " ";
      
      std::cout << "\n";

      std::cout << "CP CORR FUNC SIZE: "  << config.CP_corr.size() << " \n\n";

      std::cout << "\n\n";

      std::cout << "CP PEAKS VALUE: ";

      for(auto el :  config.CP_peaks)
        std::cout << el << " ";

      std::cout << "\n";

      std::cout << "CP PEAKS SIZE: " << config.CP_peaks.size() << "\n\n";

      std::cout << "SAMPLES WITHOUT CP VALUE: ";
      for(auto el :  config.ofdm_symbols)
        std::cout << el << " ";  

      std::cout << "\n";

      std::cout << "SAMPLES WITHOUT CP SIZE: " << config.ofdm_symbols.size() << "\n\n";

      
      std::cout << "============================= FREQUENCY DOMAIN =========================================";

      std::cout << "\n\nPOST FFT SIGNAL VALUE: ";
      for(auto el :  config.freq_domain)
        std::cout << el << " ";  

      std::cout << "\n";

      std::cout << "POST FFT SIGNAL SIZE: " << config.freq_domain.size() << "\n\n";

      std::cout << "\n\nINNER SYMBOLS VALUE: ";
      for(auto el :  config.raw_symbols)
        std::cout << el << " ";  

      std::cout << "\n";

      std::cout << "INNER SYMBOLS SIZE: " << config.freq_domain.size() << "\n\n";


      std::cout << "============================= DEMODULATION =========================================";

      std::cout << "\n\nBITS VALUE: ";
      for(auto el :  config.bits)
        std::cout << el << " "; 

      std::cout << "\n";

      std::cout << "BITS SIZE: " << config.bits.size() << "\n\n";

      std::cout << "============================= BER =========================================";

      std::cout << "BER VALUE: " << config.BER << "\n\n";

    }

  std::this_thread::sleep_for(std::chrono::seconds(5));

  }
}
