#include "../include/rx_lib.hpp"
#include "../include/tx_lib.hpp"
#include <cstdint>
#include <gtest/gtest.h>
#include <string>

std::vector<uint8_t> bin_switch = {
    0, 1, 0, 1, 0, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 0, 1, 0, 0, 1,
    0, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 1, 0, 0, 0};

std::vector<uint8_t> bin_router = {
    0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1,
    0, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 0, 0, 1, 0, 1, 0, 1, 1, 1, 0, 0, 1, 0};

std::vector<uint8_t> bin_MPLS = {0, 1, 0, 0, 1, 1, 0, 1, 0, 1, 0,
                                 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1,
                                 0, 0, 0, 1, 0, 1, 0, 0, 1, 1};

std::vector<uint8_t> bin_ethernet = {
    0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 0, 1, 0,
    0, 0, 0, 1, 1, 0, 0, 1, 0, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0,
    1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 0};

std::string str_switch("Switch");
std::string str_router("Router");
std::string str_MPSL("MPLS");
std::string str_ethernet("Ethernet");

std::vector<uint8_t> empty;

/*###################################### CODER
 * ##############################################*/

TEST(coderTest, Switch) { EXPECT_EQ(coder(str_switch), bin_switch); }

TEST(coderTest, Router) { EXPECT_EQ(coder(str_router), bin_router); }

TEST(coderTest, MPLS) { EXPECT_EQ(coder(str_MPSL), bin_MPLS); }

TEST(coderTest, Ethernet) { EXPECT_EQ(coder(str_ethernet), bin_ethernet); }

TEST(coderTest, EmptyString) { EXPECT_EQ(coder(std::string{}), empty); }

/*############################## DECODER ###################################*/

TEST(decoderTest, Switch) { EXPECT_EQ(decoder(bin_switch), str_switch); }

TEST(decoderTest, Router) { EXPECT_EQ(decoder(bin_router), str_router); }

TEST(decoderTest, MPLS) { EXPECT_EQ(decoder(bin_MPLS), str_MPSL); }

TEST(decoderTest, Ethernet) { EXPECT_EQ(decoder(bin_ethernet), str_ethernet); }

TEST(decoderTest, EmptyVector) { EXPECT_EQ(decoder(empty), std::string{}); }

/*###################################### CONVOLVE CODER
 * ##############################################*/

TEST(convCoderTest, convCoderTest) {
  EXPECT_EQ(conv_coder({1, 0, 1, 1, 0, 0, 0}, {7, 5}),
            std::vector<uint8_t>({1, 1, 1, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0}));
}

TEST(convCoderTest, emptyPoly) {
  EXPECT_EQ(conv_coder({1, 0, 1, 1, 0, 0, 0}, {}), std::vector<uint8_t>({}));
}

TEST(convCoderTest, emptyBits) {
  EXPECT_EQ(conv_coder({}, {7, 5}), std::vector<uint8_t>({}));
}

/*###################################### Intervaler and Deintervaler
 * ##############################################*/

TEST(intervalTest, Basic) {
  std::vector<uint8_t> bits{0, 1, 1, 0, 1, 1, 1, 0, 0, 0, 1, 0, 1};
  std::vector<uint8_t> intervale_bits = intervale(bits, 10);
  std::vector<uint8_t> deintervale_bits = deintervale(intervale_bits, 10);

  EXPECT_EQ(intervale_bits, deintervale_bits);
}

/*###################################### Modulation
 * ##############################################*/
TEST(QPSKmodulatorTest, Basic) {
  std::vector<uint8_t> bits{0, 0, 0, 1, 1, 0, 1, 1};
  std::vector<sample> symbols = QPSK_modulation(bits);

  double val = 1 / std::sqrt(2);

  EXPECT_EQ(symbols, std::vector<sample>(
                         {{val, val}, {val, -val}, {-val, val}, {-val, -val}}));
}

/*###################################### Demodulation
 * ##############################################*/
TEST(QPSKdemodulatorTest, Basic) {
  double val = 1 / std::sqrt(2);

  std::vector<uint8_t> bits{0, 0, 0, 1, 1, 0, 1, 1};
  std::vector<sample> symbols = {
      {val, val}, {val, -val}, {-val, val}, {-val, -val}};

  EXPECT_EQ(QPSK_demodulation(symbols), bits);
}