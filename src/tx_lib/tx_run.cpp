#include <iostream>

#include <chrono>
#include <thread>

#include "../../include/ImGUI_interface.hpp"
#include "../../include/tx_lib.hpp"

void tx_run(tx_cfg &config) {
  // std::string message("My family lives in a small house. It’s simple but
  // pretty. It has a large garden. I like to work in the garden but my sister
  // hates to work in the garden. She prefers to read. She reads in the morning,
  // in the afternoon and at night. I give all of the vegetables to mom and dad.
  // They like to cook in our small kitchen. I eat any vegetable but my sister
  // eats only a few. My family always eats breakfast and dinner together. We
  // talk. We laugh. Then my sister washes the dishes. At night dad likes to
  // listen to music. Mom works on the computer. I watch television. And my
  // sister reads. Soon we go to bed. My parents go to bed late but my sister
  // and I go to bed early. I’m ready to go to sleep but my sister wants to keep
  // reading.");
  std::string message("My family lives in a small house.");
  std::vector<int> polynomes = {0171, 0133};
  int seed;

  /*message (word) -> bits*/
  config.bits = coder(message);

  seed = rand();

  while (config.run) {

    /*convolve coding*/
    config.post_conv_coding = conv_coder(config.bits, polynomes);

    /*shuffle*/
    config.post_shuffuling = shuffuling(config.bits, seed);

    /*bits -> M-PSK/QAM symbols*/
    config.symbols = modulation(config.post_shuffuling, config.mod_order);

    /*generate ofdm grid*/
    config.grid = create_ofdm_grid(config.FFT_size, config.pilots_count,
                                   config.guard_size);

    /*generate ofdm symbols*/
    config.ofdm_symbols = create_ofdm_signal(config.symbols, config.grid,
                                             config.pilot, config.buff_size);

    /*Add CP*/
    config.ofdm_symbols_cp =
        add_CP(config.ofdm_symbols, config.FFT_size, config.CP_size);

    /*frequency domain -> time domain*/
    batch_ifft(config.ofdm_symbols_cp, config.ofdm_signal, config.FFT_size,
               config.CP_size);

    config.ofdm_symbols_cp.insert(config.ofdm_symbols_cp.begin(), 10, 0);
    config.ofdm_symbols_cp.insert(config.ofdm_symbols_cp.end(), 10, 0);

    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
  }
}