#include "../../include/tx_lib.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

/**
 * @file conv_coding.cpp
 * @brief Transalte polynome to registr position (find "1" positions)
 *
 * @param[in] poly polynoms
 * @return Sequence of index
 **/

std::vector<std::vector<int>> poly2pos(const std::vector<int> &poly) {
  std::vector<std::vector<int>> result;
  auto max = *std::max_element(poly.begin(), poly.end());
  int reg_size = 0;
  for (int x = max; x > 0; x >>= 1) {
    ++reg_size;
  }

  for (int i = 0; i < poly.size(); ++i) {
    std::vector<int> positions;
    for (int j = reg_size - 1; j >= 0; --j) {
      int bit = (poly[i] >> j) & 1;

      if (bit) {
        positions.push_back(reg_size - j - 1);
      }
    }
    result.push_back(positions);
  }

  return result;
}

std::vector<uint8_t> conv_coder(const std::vector<uint8_t> &bits,
                                const std::vector<int> poly) {
  // check params
  if (bits.size() == 0) {
    spdlog::error("[conv_coding.cpp]: Invalid bits size!");
    return {};
  }

  if (poly.size() == 0) {
    spdlog::error("[conv_coding.cpp]: Invalid poly size!");
    return {};
  }

  // find register size. std::ceil(max(poly))
  auto max = *std::max_element(poly.begin(), poly.end());
  int reg_size = 0;
  for (int x = max; x > 0; x >>= 1) {
    ++reg_size;
  }

  std::vector<uint8_t> reg(reg_size, 0);

  // get positions from polynome
  std::vector<std::vector<int>> positions = poly2pos(poly);

  std::vector<uint8_t> result;

  for (int i = 0; i < bits.size(); ++i) {
    // shfit reg
    std::rotate(reg.begin(), reg.end() - 1, reg.end());

    // add bit in first position
    reg[0] = bits[i];

    for (int j = 0; j < positions.size(); ++j) {
      int res = 0;
      for (int k = 0; k < positions[j].size(); ++k) {
        res ^= reg[positions[j][k]];
      }
      result.push_back(res);
    }
  }

  return result;
}