#include <random>

#include "../../../include/channel.hpp"

void AWGN(std::vector<sample> &samples, double SNR, const int FFT_size, const int CP_size)
{
    double mean_Es = 0;

    for (int i = CP_size; i < CP_size + FFT_size; ++i)
    {
        mean_Es += samples[i].real() * samples[i].real() + samples[i].imag() * samples[i].imag();
    }

    mean_Es /= static_cast<double>(FFT_size);

    double lin_SNR = std::pow(10, SNR / 10);
    double E_n = mean_Es / lin_SNR;
    double sigma = std::sqrt(E_n / 2);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<double> dist(0, sigma);

    for (size_t i = 0; i < samples.size(); ++i)
    {
        samples[i] += std::complex<double>(dist(gen), dist(gen));
    }
}