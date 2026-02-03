#include "../../include/rx_lib.hpp"
#include "../../include/tx_lib.hpp"
#include <cstdint>
#include <random>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

/**
 * @file intervaling.cpp
 * @brief Calculate new order for shuffuling
 *
 * @param[in] N count of elements
 * @param[in] seed seed for determanating
 * @return New order for elements
 **/
std::vector<int> order_gen_rx(const int N, const int seed) {
  std::vector<int> order(N);

  for (int i = 0; i < N; ++i) {
    order[i] = i;
  }

  std::mt19937 gen(seed);

  std::shuffle(order.begin(), order.end(), gen);

  return order;
}

std::vector<uint8_t> deintervale(const std::vector<uint8_t> &bits,
                                 const int seed) {
  if (bits.size() <= 1) {
    spdlog::error("[deintervaler.cpp]: Bits size must be more then 1!");
    return {};
  }

  // generate new order
  std::vector<int> order = order_gen_rx(bits.size(), seed);

  std::vector<uint8_t> result;
  result.reserve(bits.size());

  // shuffle
  for (int i = 0; i < bits.size(); ++i) {
    result[i] = bits[order[i]];
  }

  return result;
}