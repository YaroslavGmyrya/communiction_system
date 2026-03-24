#include <cmath>
#include <complex>
#include <cstdint>
#include <vector>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

using sample = std::complex<double>;

std::vector<sample> BPSK(const std::vector<uint8_t> &bits) {
  if (bits.size() == 0) {
    spdlog::error("Empty bit sequence!");
    return {};
  }

  double IQ;
  double norm_coeff = std::sqrt(2);

  std::vector<sample> out;
  out.reserve(bits.size());

  for (int i = 0; i < bits.size(); ++i) {
    IQ = (1 - 2 * bits[i]) / norm_coeff;
    out.push_back(sample(IQ, IQ));
  }

  return out;
}

std::vector<sample> QPSK(const std::vector<uint8_t> &bits) {
  if (bits.size() == 0) {
    spdlog::error("Empty bit sequence!");
    return {};
  }

  std::vector<uint8_t> padded = bits;

  if (padded.size() % 2 != 0) {
    spdlog::warn("Bit sequence size must be % 2");
    padded.push_back(0);
  }

  std::vector<sample> out;
  out.reserve(padded.size() / 2);

  double I, Q;
  double coeff = std::sqrt(2);

  for (int i = 0; i < padded.size(); i += 2) {
    I = (1 - 2 * padded[i]) / coeff;
    Q = (1 - 2 * padded[i + 1]) / coeff;
    out.push_back(sample(I, Q));
  }

  return out;
}

std::vector<sample> QAM16(const std::vector<uint8_t> &bits) {
  if (bits.empty()) {
    spdlog::error("Empty bit sequence!");
    return {};
  }

  std::vector<uint8_t> padded = bits;

  if (padded.size() % 4 != 0) {
    spdlog::warn("Bit sequence size must be % 4");
    while (padded.size() % 4 != 0)
      padded.push_back(0);
  }

  std::vector<sample> out;
  out.reserve(padded.size() / 4);

  const double coeff = std::sqrt(10);
  double I, Q;

  for (int i = 0; i < padded.size(); i += 4) {
    I = (1 - 2 * padded[i]) * (2 - (1 - 2 * padded[i + 2])) / coeff;
    Q = (1 - 2 * padded[i + 1]) * (2 - (1 - 2 * padded[i + 3])) / coeff;
    out.push_back(sample(I, Q));
  }

  return out;
}

std::vector<sample> QAM64(const std::vector<uint8_t> &bits) {
  if (bits.empty()) {
    spdlog::error("Empty bit sequence!");
    return {};
  }

  std::vector<uint8_t> padded = bits;

  if (padded.size() % 6 != 0) {
    spdlog::warn("Bit sequence size must be % 6");
    while (padded.size() % 6 != 0)
      padded.push_back(0);
  }

  std::vector<sample> out;
  out.reserve(padded.size() / 6);

  const double coeff = std::sqrt(42);
  double I, Q;

  for (int i = 0; i < padded.size(); i += 6) {
    I = (1 - 2 * padded[i]) *
        (4 - (1 - 2 * padded[i + 2]) * (2 - (1 - 2 * padded[i + 4]))) / coeff;

    Q = (1 - 2 * padded[i + 1]) *
        (4 - (1 - 2 * padded[i + 3]) * (2 - (1 - 2 * padded[i + 5]))) / coeff;

    out.push_back(sample(I, Q));
  }

  return out;
}

std::vector<sample> QAM256(const std::vector<uint8_t> &bits) {
  if (bits.empty()) {
    spdlog::error("Empty bit sequence!");
    return {};
  }

  std::vector<uint8_t> padded = bits;

  if (padded.size() % 8 != 0) {
    spdlog::warn("Bit sequence size must be % 8");
    while (padded.size() % 8 != 0)
      padded.push_back(0);
  }

  std::vector<sample> out;
  out.reserve(padded.size() / 8);

  const double coeff = std::sqrt(170);
  double I, Q;

  for (int i = 0; i < padded.size(); i += 8) {
    I = (1 - 2 * padded[i]) *
        (8 -
         (1 - 2 * padded[i + 2]) *
             (4 - (1 - 2 * padded[i + 4]) * (2 - (1 - 2 * padded[i + 6])))) /
        coeff;

    Q = (1 - 2 * padded[i + 1]) *
        (8 -
         (1 - 2 * padded[i + 3]) *
             (4 - (1 - 2 * padded[i + 5]) * (2 - (1 - 2 * padded[i + 7])))) /
        coeff;

    out.push_back(sample(I, Q));
  }

  return out;
}

std::vector<sample> QAM1024(const std::vector<uint8_t> &bits) {
  if (bits.empty()) {
    spdlog::error("Empty bit sequence!");
    return {};
  }

  std::vector<uint8_t> padded = bits;

  if (padded.size() % 10 != 0) {
    spdlog::warn("Bit sequence size must be % 10");
    while (padded.size() % 10 != 0)
      padded.push_back(0);
  }

  std::vector<sample> out;
  out.reserve(padded.size() / 10);

  const double coeff = std::sqrt(682);
  double I, Q;

  for (int i = 0; i < padded.size(); i += 10) {
    I = (1 - 2 * padded[i]) *
        (16 - (1 - 2 * padded[i + 2]) *
                  (8 - (1 - 2 * padded[i + 4]) *
                           (4 - (1 - 2 * padded[i + 6]) *
                                    (2 - (1 - 2 * padded[i + 8]))))) /
        coeff;

    Q = (1 - 2 * padded[i + 1]) *
        (16 - (1 - 2 * padded[i + 3]) *
                  (8 - (1 - 2 * padded[i + 5]) *
                           (4 - (1 - 2 * padded[i + 7]) *
                                    (2 - (1 - 2 * padded[i + 9]))))) /
        coeff;

    out.push_back(sample(I, Q));
  }

  return out;
}

std::vector<sample> modulation(const std::vector<uint8_t> &bits,
                               const int &order) {
  if (order == 2) {
    return BPSK(bits);
  } else if (order == 4) {
    return QPSK(bits);
  } else if (order == 16) {
    return QAM16(bits);
  } else if (order == 64) {
    return QAM64(bits);
  } else if (order == 256) {
    return QAM256(bits);
  } else if (order == 1024) {
    return QAM1024(bits);
  } else {
    spdlog::error("[modulation.cpp]: Invalid order! Try 2,4,16,64,128,256!");
    return {};
  }
}