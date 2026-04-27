#include <iostream>

#include <chrono>
#include <thread>

#include "../../../include/GUI.hpp"
#include "../../../include/PHY/tx_dsp.hpp"

void tx_run(tx_cfg &config)
{

  const int SEED = 10;

  config.message = "One thing";
  // config.message = "One thing";

  const int ZC_ROOT = 25;

  std::vector<int> polynomes = {0171, 0133};

  /*generate ZC*/
  config.zc = ZC_gen(ZC_ROOT, config.FFT_size);

  // std::vector<uint8_t> shuffuling_bits;

  while (config.run)
  {

    auto begin = std::chrono::steady_clock::now();

    config.bits.clear();

    /*message (word) -> bits*/
    std::vector<uint8_t> bits;
    bits = coder(config.message);

    /*bits in one inner symbol*/
    int bps = static_cast<int>(std::log2(config.mod_order));

    /*bits in one OFDM symbol*/
    int payload = (config.FFT_size - 2 * config.guard_size - config.pilots_count) * bps;

    /*compute padding (zeros in end of last OFDM symbol)*/
    config.padding = (payload - (bits.size() % payload)) % payload;

    config.padding -= config.max_padding_bits;

    /*extract bits*/
    for (int i = config.max_padding_bits - 1; i >= 0; --i)
    {
      config.bits.push_back((config.padding >> i) & 1);
    }

    /*final bits*/
    config.bits.insert(config.bits.end(), bits.begin(), bits.end());

    // shuffuling_bits = shuffuling(bits, SEED);

    // std::cout << shuffuling_bits.size() << "\n\n";

    /*generate ofdm grid*/
    config.grid = create_ofdm_grid(config.FFT_size, config.pilots_count, config.guard_size);

    /*bits -> M-PSK/QAM symbols*/
    config.symbols = modulation(config.bits, config.mod_order);

    /*generate ofdm symbols*/
    config.ofdm_symbols = create_ofdm_signal(config.symbols, config.grid,
                                             config.pilot);

    /*add ZC*/
    config.ofdm_symbols = add_ZC(config.ofdm_symbols, config.zc);

    /*frequency domain -> time domain*/
    batch_ifft(config.ofdm_symbols, config.ofdm_signal, config.FFT_size,
               config.CP_size);

    /*Add CP*/
    config.ofdm_symbols_cp = add_CP(config.ofdm_signal, config.FFT_size, config.CP_size);

    /*Add trash*/
    config.ofdm_symbols_cp.insert(config.ofdm_symbols_cp.begin(), 5, 0);
    config.ofdm_symbols_cp.insert(config.ofdm_symbols_cp.end(), 5, 0);

    auto end = std::chrono::steady_clock::now();

    auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);

    // std::cout << "The TX time: " << elapsed_ms.count() << " ms\n";

    std ::this_thread::sleep_for(std::chrono::milliseconds(5000));

    /*================================================= DEBUG INFO =========================================================*/

    if (config.DEBUG_MODE)
    {
      std::cout << "============================= MESSAGE =========================================";

      std::cout << "\nVALUE: " << config.message << "\n";
      std::cout << "SIZE: " << config.message.size() << "\n";

      std::cout << "\n\n";

      std::cout << "============================= BITS =========================================";

      std::cout << "\nVALUE: ";
      for (int i = 0; i < config.bits.size(); ++i)
      {
        if (i % 8 == 0)
          std::cout << "\n\n";
        printf("%d ", config.bits[i]);
      }

      std::cout << "\nSIZE: " << config.bits.size() << "\n";

      std::cout << "\n\n";

      std::cout << "PADDING: " << config.padding << "\n";

      std::cout << "\n\n";

      std::cout << "============================= SYMBOLS =========================================";

      std::cout << "\nSIZE: " << config.symbols.size() << "\n";
      std::cout << "MODULATION ORDER: " << config.mod_order << "\n";

      std::cout << "VALUE: ";
      for (int i = 0; i < config.symbols.size(); ++i)
      {
        std::cout << config.symbols[i] << " ";
      }

      std::cout << "\n\n";

      std::cout << "============================= OFDM PARAMETERS =========================================";

      std::cout << "\nFFT_SIZE: " << config.FFT_size << "\n";
      std::cout << "CP_SIZE: " << config.CP_size << "\n";
      std::cout << "PILOTS COUNT: " << config.pilots_count << "\n";
      std::cout << "PILOT VALUE: " << config.pilot << "\n";

      std::cout << "GUARD INTERVAL (ON ONE SIDE): " << config.guard_size << "\n";

      std::cout << "OFDM GRID: " << "\n";

      for (int i = 0; i < config.grid.size(); ++i)
      {
        if (config.grid[i] == 0)
          std::cout << "G ";

        if (config.grid[i] == 1)
          std::cout << "D ";

        if (config.grid[i] == 2)
          std::cout << "P ";
      }

      std::cout << "\n";

      const int PAYLOAD_SIZE = config.FFT_size - 2 * config.guard_size - config.pilots_count;
      std::cout << "OFDM SYMBOLS COUNT: " << std::ceil(config.symbols.size() / PAYLOAD_SIZE);

      std::cout << "\n\n";

      std::cout << "============================= PSS =========================================";

      std::cout << "\nTYPE: " << "Zadoff-Chu\n";
      std::cout << "\nROOT: " << ZC_ROOT << "\n";

      std::cout << "VALUE: ";
      for (int i = 0; i < config.zc.size(); ++i)
      {
        std::cout << config.zc[i] << " ";
      }

      std::cout << "SIZE: " << config.zc.size() << "\n";

      std::cout << "\n\n";

      std::cout << "============================= OFDM SYMBOLS (FREQUENCY DOMAIN) WITH PSS =========================================";

      std::cout << "\nSIZE: " << config.ofdm_symbols.size() << "\n";

      std::cout << "VALUE: ";

      for (int i = 0; i < config.ofdm_symbols.size(); ++i)
      {
        if (i % config.FFT_size == 0)
          std::cout << "\n\n";
        std::cout << config.ofdm_symbols[i] << " ";
      }

      std::cout << "\n\n";

      std::cout << "============================= OFDM SYMBOLS (TIME DOMAIN) =========================================";

      std::cout << "\nSIZE: " << config.ofdm_signal.size() << "\n";

      std::cout << "VALUE: ";

      for (int i = 0; i < config.ofdm_signal.size(); ++i)
      {
        if (i % config.FFT_size == 0)
          std::cout << "\n\n";
        std::cout << config.ofdm_signal[i] << " ";
      }

      std::cout << "\n\n";

      std::cout << "============================= OFDM SYMBOLS SIGNAL (WITH CP) =========================================";

      std::cout << "\nSIZE: " << config.ofdm_symbols_cp.size() << "\n";

      std::cout << "VALUE: ";

      for (int i = 0; i < config.ofdm_symbols_cp.size(); ++i)
      {
        if (i % (config.FFT_size + config.CP_size) == 0)
          std::cout << "\n\n";
        std::cout << config.ofdm_symbols_cp[i] << " ";
      }
    }
  }
}