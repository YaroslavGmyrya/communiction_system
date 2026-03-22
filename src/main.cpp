#include <GL/glew.h>
#include <SDL2/SDL.h>

#include <chrono>
#include <cmath>
#include <iostream>
#include <thread>

#include "../include/ImGUI_interface.hpp"
#include "../include/tx_lib.hpp"

int main(int argc, char *argv[]) {

  /*init TX config*/
  tx_cfg tx_config;
  tx_config.run = true;
  tx_config.sps = 10;
  tx_config.mod_order = 2;
  // tx_config.bitrate = static_cast<int>(sdr_config.tx_sample_rate /
  //                                      tx_config.sps * tx_config.mod_order);
  tx_config.IR_type = 0;
  // tx_config.tx_samples.resize(sdr_config.buff_size);
  tx_config.OFDM = 1;
  tx_config.FFT_size = 16;
  tx_config.CP_size = 4;
  tx_config.count_OFDM_symb = 10;
  tx_config.count_bits = 1000;
  tx_config.pilot = {1, 1};
  tx_config.pilots_count = 4;
  tx_config.guard_size = 2;

  /*init RX config*/
  rx_cfg rx_config;
  rx_config.run = true;
  rx_config.costas_BnTs = 0.5;
  rx_config.costas_Kp = 1;
  rx_config.gardner_BnTs = 0.5;
  rx_config.gardner_Kp = 1;
  rx_config.IR_type = 1;
  rx_config.mod_order = 2;
  rx_config.sps = 10;
  // rx_config.rx_samples.resize(sdr_config.buff_size);
  rx_config.OFDM = 0;
  rx_config.Nc = 16;
  rx_config.CP_size = 4;

  std::thread gui_thread(run_gui, std::ref(tx_config), std::ref(rx_config));
  std::thread tx_thread(tx_run, std::ref(tx_config));

  gui_thread.join();
  tx_thread.join();

  return 0;
}