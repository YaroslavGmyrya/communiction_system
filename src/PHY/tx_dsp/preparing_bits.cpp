#include <complex>
#include <cstdint>
#include <random>
#include <vector>
#include <iostream>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

std::vector<uint8_t> coder(const std::string &str)
{
  if (str.size() == 0)
  {
    spdlog::error("[preparing_bits.cpp] String size is zero!");
    return {};
  }

  int bits_size = str.size() * 8;

  std::vector<uint8_t> out;
  out.resize(bits_size);

  for (int i = 0; i < str.size(); i++)
  {
    for (int j = 0; j < 8; j++)
    {
      out[i * 8 + j] = (str[i] >> (7 - j)) & 1;
    }
  }

  return out;
}

/**
 * @file conv_coding.cpp
 * @brief Transalte polynome to registr position (find "1" positions)
 *
 * @param[in] poly polynoms
 * @return Sequence of index
 **/
std::vector<std::vector<int>> poly2pos(const std::vector<int> &poly)
{
  std::vector<std::vector<int>> result;
  auto max = *std::max_element(poly.begin(), poly.end());
  int reg_size = 0;
  for (int x = max; x > 0; x >>= 1)
  {
    ++reg_size;
  }

  for (int i = 0; i < poly.size(); ++i)
  {
    std::vector<int> positions;
    for (int j = reg_size - 1; j >= 0; --j)
    {
      int bit = (poly[i] >> j) & 1;

      if (bit)
      {
        positions.push_back(reg_size - j - 1);
      }
    }
    result.push_back(positions);
  }

  return result;
}

std::vector<uint8_t> conv_coder(const std::vector<uint8_t> &bits,
                                const std::vector<int> &poly)
{
  if (bits.size() == 0)
  {
    spdlog::error("[preparing_bits.cpp]: Invalid bits size!");
    return {};
  }

  if (poly.size() == 0)
  {
    spdlog::error("[preparing_bits.cpp]: Invalid poly size!");
    return {};
  }

  auto max = *std::max_element(poly.begin(), poly.end());
  int reg_size = 0;
  for (int x = max; x > 0; x >>= 1)
  {
    ++reg_size;
  }

  std::vector<uint8_t> reg(reg_size, 0);
  std::vector<std::vector<int>> positions = poly2pos(poly);
  std::vector<uint8_t> out;

  for (int i = 0; i < bits.size(); ++i)
  {
    std::rotate(reg.begin(), reg.end() - 1, reg.end());
    reg[0] = bits[i];

    for (int j = 0; j < positions.size(); ++j)
    {
      int res = 0;
      for (int k = 0; k < positions[j].size(); ++k)
      {
        res ^= reg[positions[j][k]];
      }
      out.push_back(res);
    }
  }

  return out;
}

/**
 * @file intervaling.cpp
 * @brief Calculate new order for shuffuling
 *
 * @param[in] N count of elements
 * @param[in] seed seed for determanating
 * @return New order for elements
 **/
std::vector<int> order_gen(const int N, const int seed)
{
  std::vector<int> order(N);

  for (int i = 0; i < N; ++i)
  {
    order[i] = i;
  }

  std::mt19937 gen(seed);

  std::shuffle(order.begin(), order.end(), gen);

  return order;
}

std::vector<uint8_t> shuffuling(const std::vector<uint8_t> &bits,
                                const int &seed)
{
  if (bits.size() <= 1)
  {
    spdlog::error("[preparing_bits.cpp]: Bits size must be more then 1!");
    return {};
  }

  std::vector<int> order = order_gen(bits.size(), seed);

  std::vector<uint8_t> out;
  out.resize(bits.size());

  for (int i = 0; i < bits.size(); ++i)
  {
    out[order[i]] = bits[i];
  }

  return out;
}