#pragma once

#include <complex>
#include <vector>

using sample = std::complex<double>;


/**
 * @file AWGN.cpp
 * @brief Add Additive White Gaussian Noise
 
 * @param[in] samples signal samples
 * @param[in] SNR Signal Noise Ratio
 * @param[in] FFT_size OFDM symbol size (without CP)
 * @param[in] CP_size Cyclic Prefix size

 * @return samples with AWGN
 **/
void AWGN(std::vector<sample>& samples, double SNR, const int FFT_size, const int CP_size);
