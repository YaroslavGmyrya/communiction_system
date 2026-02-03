#include "../../include/tx_lib.hpp"
#include <cstdint>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

std::vector<sample> QPSK_modulation(std::vector<uint8_t> &bits) {
  if (bits.size() == 0) {
    spdlog::error("Empty bit sequence!");
    return {};
  }

  if (bits.size() % 2 != 0) {
    spdlog::warn("Bit sequence size must be % 2");
    bits.push_back(0);
  }

  std::vector<sample> samples;
  for (int i = 0; i < bits.size(); i += 2) {
    double I = (1 - 2 * bits[i]) / std::sqrt(2);
    double Q = (1 - 2 * bits[i + 1]) / std::sqrt(2);

    samples.push_back(sample(I, Q));
  }

  return samples;
}