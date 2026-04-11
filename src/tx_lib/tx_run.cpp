#include <iostream>

#include <chrono>
#include <thread>

#include "../../include/ImGUI_interface.hpp"
#include "../../include/tx_lib.hpp"

void tx_run(tx_cfg &config)
{

  config.message = "message";

  const int ZC_ROOT = 25;

  std::vector<int> polynomes = {0171, 0133};

  /*generate ZC*/
  config.zc = ZC_gen(ZC_ROOT, config.FFT_size);

  while (config.run)
  {

    /*message (word) -> bits*/
    config.bits = coder(config.message);

    /*generate ofdm grid*/
    config.grid = create_ofdm_grid(config.FFT_size, config.pilots_count, config.guard_size);

    /*bits -> M-PSK/QAM symbols*/
    config.symbols = modulation(config.bits, config.mod_order);

    /*generate ofdm symbols*/
    config.ofdm_symbols = create_ofdm_signal(config.symbols, config.grid,
                                             config.pilot, config.buff_size);
    /*add ZC*/
    config.ofdm_symbols = add_ZC(config.ofdm_symbols, config.zc);

    /*frequency domain -> time domain*/
    batch_ifft(config.ofdm_symbols, config.ofdm_signal, config.FFT_size,
               config.CP_size);

    /*Add CP*/
    config.ofdm_symbols_cp = add_CP(config.ofdm_signal, config.FFT_size, config.CP_size, 0);

    /*Add trash*/
    config.ofdm_symbols_cp.insert(config.ofdm_symbols_cp.begin(), 500, 0);
    config.ofdm_symbols_cp.insert(config.ofdm_symbols_cp.end(), 500, 0);

    std ::this_thread::sleep_for(std::chrono::milliseconds(5000));

    /*DEBUG INFO*/

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
      std::cout << "\ROOT: " << ZC_ROOT << "\n";

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