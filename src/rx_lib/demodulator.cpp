#include "../../include/tx_lib.hpp"
#include <cstdint>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

std::vector<uint8_t> QPSK_demodulation(const std::vector<sample> &symbols) {
  if (symbols.size() == 0) {
    spdlog::error("Empty bit sequence!");
    return {};
  }

  std::vector<uint8_t> bits;

  for (int i = 0; i < symbols.size(); ++i) {
    double I = symbols[i].real();
    double Q = symbols[i].imag();

    uint8_t b0 = (I < 0) ? 1 : 0;
    uint8_t b1 = (Q < 0) ? 1 : 0;

    bits.push_back(b0);
    bits.push_back(b1);
  }

  return bits;
}