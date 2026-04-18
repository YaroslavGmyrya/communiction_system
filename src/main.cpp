#include <chrono>
#include <cmath>
#include <iostream>
#include <thread>

#include "../include/ImGUI_interface.hpp"
#include "../include/rx_lib.hpp"
#include "../include/tx_lib.hpp"

int main(int argc, char *argv[])
{

  /*init TX config*/
  tx_cfg tx_config;
  tx_config.run = true;
  tx_config.mod_order = 2;
  tx_config.FFT_size = 128;
  tx_config.CP_size = 16;
  tx_config.pilot = {1, 1};
  tx_config.pilots_count = 8;
  tx_config.guard_size = 3;
  tx_config.message = "message";
  tx_config.DEBUG_MODE = false;
  tx_config.max_padding_bits = 12;

  /*init RX config*/
  rx_cfg rx_config;
  rx_config.run = true;
  rx_config.mod_order = 2;
  rx_config.FFT_size = 128;
  rx_config.CP_size = 16;
  rx_config.pilot_value = {1, 1};
  rx_config.pilots_count = 8;
  rx_config.guard_size = 3;
  rx_config.max_padding_bits = 12;

  rx_config.SNR = 50;

  std::thread tx_thread(tx_run, std::ref(tx_config));
  std::this_thread::sleep_for(std::chrono::seconds(1));

  std::thread rx_thread(rx_run, std::ref(rx_config), std::ref(tx_config));

  run_gui(std::ref(tx_config), std::ref(rx_config));

  tx_thread.join();
  rx_thread.join();

  return 0;
}