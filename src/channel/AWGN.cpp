#include <random>
#include <iostream>

#include "../../../include/channel.hpp"

using namespace std::complex_literals;

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

std::vector<sample> frequency_offset(const std::vector<sample> &samples, double f_offset, double Fs)
{
    const std::complex<double> j(0, 1);
    const double PI = 3.141592653589793;

    std::vector<sample> result(samples.size());

    std::cout << samples.size() << "\t" << result.size() << "\n\n";

    for (int n = 0; n < samples.size(); ++n)
    {
        std::cout << samples[n] << " ";
        // std::cout << n << "\n";
        // double phase = 2.0 * PI * f_offset * n / Fs;
        // result[n] = samples[n] * std::exp(j * phase);
    }

    std::cout << "\n\n";

    return result;
}