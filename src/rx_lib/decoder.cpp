#include "../../include/rx_lib.hpp"
#include <spdlog/spdlog.h>
#include <string>
#include <vector>

/**
 * @file decoder.cpp
 * @brief Translate bits to string (only ascii)
 *
 * @param[in] bits bit sequence
 * @return String
 **/

std::string decoder(const std::vector<uint8_t> &bits) {
  if (bits.size() % 8 != 0) {
    spdlog::error("[decoder]: Bits size must be % 8!");
    return {};
  }

  std::string word;

  for (int i = 0; i < bits.size(); i += 8) {
    uint8_t byte = 0;

    for (int j = 0; j < 8; ++j) {
      byte = (byte << 1) | bits[i + j];
    }

    word += static_cast<char>(byte);
  }

  return word;
}