#include <random>
#include <iostream>
#include <spdlog/spdlog.h>

#include "../../../include/channel.hpp"

std::vector<std::string> allowed_profiles{
    std::string("TU50"),  // Typical Urban, 50 km/h
    std::string("TU3"),   // Typical Urban, 3 km/h
    std::string("RA130"), // Rural Area, 130 km/h
    std::string("HT100"), // Hilly Terrain, 100 km/h
    std::string("EQ50")   // Equalization, 50 km/h
};

std::map<std::string, profile_s> models = {
    {"TU50",
     {{0.0e-6, 0.2e-6, 0.5e-6, 1.6e-6, 2.3e-6, 5.0e-6},
      {-3.0, 0.0, -2.0, -6.0, -8.0, -10.0},
      50}},
    {"TU3",
     {{0.0e-6, 0.2e-6, 0.5e-6, 1.6e-6, 2.3e-6, 5.0e-6},
      {-3.0, 0.0, -2.0, -6.0, -8.0, -10.0},
      3}},
    {"RA130",
     {{0.0e-6, 0.1e-6, 0.2e-6, 0.3e-6, 0.4e-6, 0.5e-6},
      {30.0, -4.0, -8.0, -12.0, -16.0, -20.0},
      130}},
    {"HT100",
     {{0.0e-6, 0.1e-6, 0.3e-6, 0.5e-6, 15.0e-6, 17.2e-6},
      {0.0, -1.5, -4.5, -7.5, -8.0, -17.7},
      100}},
    {"EQ50",
     {{0.0e-6, 3.2e-6, 6.4e-6, 9.6e-6, 12.8e-6, 16.0e-6},
      {0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
      50}}};

std::vector<std::complex<double>> jakes(const double f_d, const double N, const std::vector<double> &t, const int seed)
{
    std::vector<std::complex<double>> ray(t.size());

    std::mt19937 gen(std::random_device{}());
    std::uniform_real_distribution<double> dist(0.0, 2.0 * M_PI);

    double phi = dist(gen);
    double theta = dist(gen);

    std::vector<double> Xr(t.size());
    std::vector<double> Xi(t.size());

    double alpha, phase;
    for (int i = 0; i < N; ++i)
    {
        alpha = ((2.0 * M_PI * (i + 1)) - M_PI + theta) / (4 * N);
        phase = dist(gen);

        for (int j = 0; j < t.size(); ++j)
        {
            Xr[j] += std::sqrt(2.0 / N) * std::cos(phase) * std::cos(2.0 * M_PI * f_d * t[j] * std::cos(alpha) + phi);
            Xi[j] += std::sqrt(2.0 / N) * std::sin(phase) * std::cos(2.0 * M_PI * f_d * t[j] * std::cos(alpha) + phi);
        }
    }

    for (int i = 0; i < t.size(); ++i)
    {
        ray[i] = std::complex<double>(Xr[i], Xi[i]);
    }

    return ray;
}

std::vector<std::vector<std::complex<double>>> COST_207(const std::string &profile, const double f_c, const int N, const std::vector<double> &t, const double Ts)
{
    bool found = false;
    for (const std::string &el : allowed_profiles)
    {
        if (el == profile)
        {
            found = true;
        }
    }

    if (!found)
    {
        spdlog::error("[COST207.cpp:COST_207]: Invalid profile type! Check channel.hpp to find allowed profiles!");
        return {};
    }

    profile_s model = models[profile];

    std::vector<double> powers_lin(model.powers_db.size());

    for (int i = 0; i < powers_lin.size(); ++i)
    {
        powers_lin[i] = pow(10, model.powers_db[i] / 10);
    }

    std::vector<int> samples_delay(powers_lin.size());

    for (int i = 0; i < powers_lin.size(); ++i)
    {
        samples_delay[i] = std::round(model.delays[i] / Ts);
    }

    int max_shift = INT32_MIN;

    for (int i = 0; i < samples_delay.size(); ++i)
    {
        if (samples_delay[i] > max_shift)
            max_shift = samples_delay[i];
    }

    max_shift += 1;

    std::vector<std::vector<std::complex<double>>> H(t.size(), std::vector<std::complex<double>>(max_shift));

    double f_d = (f_c * model.velocity_kmh / 3.6) / 3e8;

    std::vector<std::complex<double>> ray;
    double coeff;
    for (int i = 0; i < powers_lin.size(); ++i)
    {
        ray = jakes(f_d, N, t, Ts);

        coeff = std::sqrt(powers_lin[i]);

        for (int j = 0; j < ray.size(); ++j)
        {
            H[j][samples_delay[i]] = coeff * ray[j];
        }
    }

    return H;
}

std::vector<sample> channel_pass(std::vector<sample> x, std::vector<std::vector<sample>> H)
{
    const int N = x.size();
    const int L = H[0].size();

    std::vector<sample> y(x.size());

    for (int n = 0; n < N; ++n)
    {
        for (int l = 0; l < L; ++l)
        {
            if (n - l >= 0)
            {
                y[n] += H[n][l] * x[n - l];
            }
        }
    }

    return y;
}

std::vector<double> get_rays_energy(const std::vector<std::vector<sample>> &H)
{
    std::vector<double> avg_energy(H[0].size());

    double energy;

    for (int j = 0; j < H[0].size(); ++j)
    {
        energy = 0;

        for (int i = 0; i < H.size(); ++i)
        {
            double abs_val = std::abs(H[i][j]);
            energy += abs_val * abs_val;
        }

        energy /= H.size();

        if (std::abs(energy) == 0)
        {
            avg_energy[j] = 0;
        }
        else
        {
            avg_energy[j] = 10 * std::log10(energy);
        }
    }

    return avg_energy;
}