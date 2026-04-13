#include <iostream>
#include <mutex>
#include <thread>
#include <chrono>


#include "../../include/ImGUI_interface.hpp"
#include "../../include/find_peaks.hpp"
#include "../../include/rx_lib.hpp"
#include "../../include/tx_lib.hpp"

int main()
{
    std::string str("12345678");
    std::vector<uint8_t> bits = coder(str);

    std::vector<std::complex<double>> bpsk = modulation(bits, 2);

    std::cout << "BPSK:\n\n";

    for(int i = 0; i < bpsk.size(); ++i){
        std::cout << bpsk[i] << " ";
    }

    std::vector<std::complex<double>> time;
    batch_ifft(bpsk, time, 64, 0);

    std::cout << "TIME:\n\n";

    for(int i = 0; i < time.size(); ++i){
        std::cout << time[i] << " ";
    }

    std::cout << "\n\n";

    std::vector<std::complex<double>> freq;
    batch_fft(time, time, 64);

    std::cout << "FREQ:\n\n";

    for(int i = 0; i < freq.size(); ++i){
        std::cout << freq[i] << " ";
    }

    std::cout << "\n\n";


}
