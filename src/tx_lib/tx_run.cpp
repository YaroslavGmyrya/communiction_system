#include <iostream>

#include <chrono>
#include <thread>

#include "../../include/ImGUI_interface.hpp"
#include "../../include/tx_lib.hpp"

void tx_run(tx_cfg &config) {
  // std::string message = R"(My family lives in a small house. It's simple but
  // pretty. It has a large garden. I like to work in the garden but my sister
  // hates to work in the garden. She prefers to read. She reads in the morning,
  // in the afternoon and at night. I give all of the vegetables to mom and dad.
  // They like to cook in our small kitchen. I eat any vegetable but my sister
  // eats only a few. My family always eats breakfast and dinner together. We
  // talk. We laugh. Then my sister washes the dishes. At night dad likes to
  // listen to music. Mom works on the computer. I watch television. And my
  // sister reads. Soon we go to bed. My parents go to bed late but my sister
  // and I go to bed early. I'm ready to go to sleep but my sister wants to keep
  // read)";

  std::string message(
      "My family lives in a small house. It's simple but pretty. It has a "
      "large garden. I like to work in the garden but my sister");

  std::vector<int> polynomes = {0171, 0133};

  /*message (word) -> bits*/
  config.bits = coder(message);

  while (config.run) {
    // std::cout << "\n BITS SIZE: " << config.bits.size() << "\n";

    // std::cout << "bits: ";
    // for (int i = 0; i < config.bits.size(); ++i)
    // {
    //   printf("%d ", config.bits[i]);
    // }

    // printf("\n\n\n");

    /*convolve coding*/
    // config.post_conv_coding = conv_coder(config.bits, polynomes);

    // std::cout << "post_conv_coding: ";
    // for (int i = 0; i < config.post_conv_coding.size(); ++i)
    // {
    //   printf("%d ", config.post_conv_coding[i]);
    // }

    // printf("\n\n\n");

    /*shuffle*/
    // config.post_shuffuling = shuffuling(config.bits, config.seed);

    // std::cout << "POST SHUFFULING: ";
    // for (int i = 0; i < config.post_shuffuling.size(); ++i)
    // {
    //   printf("%d ", config.post_shuffuling[i]);
    // }

    // printf("\n\n\n");

    /*bits -> M-PSK/QAM symbols*/
    config.symbols = modulation(config.bits, config.mod_order);

    // std::cout << "\n QPSK SIZE: " << config.symbols.size() << "\n";

    // std::cout << "\n\nSYMBOLS: ";
    // for (int i = 0; i < config.symbols.size(); ++i)
    // {
    //   std::cout << config.symbols[i] << " ";
    // }

    // std::cout << "SYMBOLS: ";
    // for (int i = 0; i < config.symbols.size(); ++i)
    // {
    //   std::cout << config.symbols[i] << " ";
    // }

    // printf("\n\n\n");

    /*generate ofdm grid*/
    config.grid = create_ofdm_grid(config.FFT_size, config.pilots_count,
                                   config.guard_size);

    /*generate ofdm symbols*/
    config.ofdm_symbols = create_ofdm_signal(config.symbols, config.grid,
                                             config.pilot, config.buff_size);

    // std::cout << "\n\n"
    //           << config.ofdm_symbols.size() << "\n\n";

    // std::cout << "\n OFDM SIZE: " << config.ofdm_symbols.size() << "\n";

    // std::cout << "\n\n"
    //           << config.ofdm_symbols.size();

    // std::cout << "ofdm_symbols: ";
    // for (int i = 0; i < config.ofdm_symbols.size(); ++i)
    // {
    //   if (i % (config.FFT_size) == 0)
    //     std::cout << "\n\n\n";

    //   std::cout << config.ofdm_symbols[i] << " ";
    // }

    // printf("\n\n\n");

    // std::cout << "\n ofdm_symbols_cp SIZE: " << config.ofdm_symbols_cp.size()
    // << "\n";

    // std::cout << "\n\n"
    //           << config.ofdm_symbols_cp.size();

    std::cout << "ofdm_symbols_cp: ";
    for (int i = 0; i < config.ofdm_symbols.size(); ++i) {
      if (i % config.FFT_size == 0)
        std::cout << "\n\n\n";
      std::cout << config.ofdm_symbols[i] << " ";
    }

    // printf("\n\n\n");

    /*frequency domain -> time domain*/
    batch_ifft(config.ofdm_symbols, config.ofdm_signal, config.FFT_size,
               config.CP_size);

    /*Add CP*/
    config.ofdm_symbols_cp =
        add_CP(config.ofdm_signal, config.FFT_size, config.CP_size);

    // std::cout << "\n\n"
    //           << config.ofdm_symbols_cp.size() << "\n\n";

    config.ofdm_symbols_cp.insert(config.ofdm_symbols_cp.begin(), 10, 0);
    config.ofdm_symbols_cp.insert(config.ofdm_symbols_cp.end(), 10, 0);

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }
}