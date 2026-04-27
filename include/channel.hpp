#pragma once

#include <complex>
#include <vector>
#include <map>

using sample = std::complex<double>;

/*Profiles types for COST207 channel model*/
extern std::vector<std::string> allowed_profiles;

struct profile_s
{
    std::vector<double> delays;
    std::vector<double> powers_db;
    double velocity_kmh;
};

extern std::map<std::string, profile_s> models;

/**
 * @brief Add Additive White Gaussian Noise

 * @param[in] samples signal samples
 * @param[in] SNR Signal Noise Ratio
 * @param[in] FFT_size OFDM symbol size (without CP)
 * @param[in] CP_size Cyclic Prefix size

 * @return samples with AWGN
 **/
void AWGN(std::vector<sample> &samples, double SNR, const int FFT_size, const int CP_size);

std::vector<std::complex<double>> jakes(const double f_d, const double N, const std::vector<double> &t, const int seed);

std::vector<std::vector<std::complex<double>>> COST_207(const std::string &profile, const double f_c, const int N, const std::vector<double> &t, const double Ts);

std::vector<sample> channel_pass(std::vector<sample> x, std::vector<std::vector<sample>> H);

std::vector<double> get_rays_energy(const std::vector<std::vector<sample>> &H);