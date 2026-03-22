#include <complex.h>
#include <iostream>
#include <stdint.h>
#include <stdlib.h>
#include <thread>
#include <unistd.h>
#include <vector>

#include "../include/ImGUI_interface.hpp"

template <typename T> ImPlotPoint get_value(int idx, void *user_data) {
  auto *vec = static_cast<std::vector<T> *>(user_data);
  return ImPlotPoint(idx, (*vec)[idx]);
}

template <typename T> ImPlotPoint get_I(int idx, void *user_data) {
  auto *vec = static_cast<std::vector<std::complex<T>> *>(user_data);
  return ImPlotPoint(idx, (*vec)[idx].real());
}

template <typename T> ImPlotPoint get_Q(int idx, void *user_data) {
  auto *vec = static_cast<std::vector<std::complex<T>> *>(user_data);
  return ImPlotPoint(idx, (*vec)[idx].imag());
}

template <typename T> ImPlotPoint get_points(int idx, void *data) {
  auto *vec = static_cast<std::vector<std::complex<T>> *>(data);

  const auto &s = (*vec)[idx];

  return ImPlotPoint(s.real(), s.imag());
}

ImPlotPoint get_phase_spec(int idx, void *data) {
  auto *fft = static_cast<
      std::pair<std::vector<std::complex<double>>, std::vector<double>> *>(
      data);

  const auto &samples = fft->first;
  const auto &freqs = fft->second;

  double x = freqs[idx];
  double y = std::arg(samples[idx]);

  return ImPlotPoint(x, y);
}

ImPlotPoint get_amp_spec(int idx, void *data) {
  auto *fft = static_cast<
      std::pair<std::vector<std::complex<double>>, std::vector<double>> *>(
      data);

  const auto &samples = fft->first;
  const auto &freqs = fft->second;

  double x = freqs[idx];
  double y = std::abs(samples[idx]);

  return ImPlotPoint(x, y);
}

void run_gui(tx_cfg &tx_config, rx_cfg &rx_config) {
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);

  SDL_Window *window = SDL_CreateWindow(
      "Backend start", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1024,
      768, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

  SDL_GLContext gl_context = SDL_GL_CreateContext(window);

  ImGui::CreateContext();
  ImPlot::CreateContext();

  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
  ImGui_ImplOpenGL3_Init("#version 330");

  float left_width = 450.0f;
  float plot_height = 250.0f;
  bool running = true;

  while (running) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      ImGui_ImplSDL2_ProcessEvent(&event);
      if (event.type == SDL_QUIT) {
        tx_config.run = false;
        rx_config.run = false;
        running = false;
      }
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    ImGui::DockSpaceOverViewport(0, nullptr, ImGuiDockNodeFlags_None);

    if (ImGui::Begin("Simulator")) {
      if (ImGui::BeginTabBar("MyTabBar")) {
        if (ImGui::BeginTabItem("Transmitter")) {
          if (ImGui::BeginChild("TX", ImVec2(left_width, 0), true)) {
            ImGui::Text("Settings");

            ImGui::SeparatorText("Modulator");
            if (ImGui::RadioButton("BPSK", &tx_config.mod_order, 2)) {
              tx_config.count_bits =
                  tx_config.count_OFDM_symb * tx_config.FFT_size;
            }
            if (ImGui::RadioButton("QPSK", &tx_config.mod_order, 4)) {
              tx_config.count_bits =
                  tx_config.count_OFDM_symb * tx_config.FFT_size * 2;
            }

            if (ImGui::RadioButton("QAM16", &tx_config.mod_order, 16)) {
              tx_config.count_bits =
                  tx_config.count_OFDM_symb * tx_config.FFT_size * 4;
            }

            ImGui::SeparatorText("Upsampler");
            ImGui::SliderInt("Samples per symbol", &tx_config.sps, 2, 100);

            ImGui::SeparatorText("Pulse Shaping filter");
            ImGui::RadioButton("Rectangle", &tx_config.IR_type, 0);
            ImGui::RadioButton("Rised-Cosine", &tx_config.IR_type, 1);

            ImGui::SeparatorText("Technology");
            ImGui::RadioButton("OFDM", &tx_config.OFDM, 1);
            ImGui::RadioButton("Non-OFDM", &tx_config.OFDM, 0);

            if (tx_config.OFDM) {
              ImGui::SeparatorText("OFDM");
              ImGui::InputInt("Subcarriers count", &tx_config.FFT_size, 1, 128);
              ImGui::InputInt("Cycle prefix size", &tx_config.CP_size, 1, 128);
              ImGui::InputInt("Count of symbols", &tx_config.count_OFDM_symb, 1,
                              128);
            }

            ImGui::EndChild();
          }

          ImGui::SameLine();

          if (ImGui::BeginChild("TX_Plots", ImVec2(0, 0), true)) {
            // if (ImPlot::BeginPlot("Bits", ImVec2(-1, plot_height)))
            // {

            //   ImPlot::PlotLineG("Bits", get_value<uint8_t>, &tx_config.bits,
            //                     tx_config.bits.size());
            //   ImPlot::EndPlot();
            // }

            if (ImPlot::BeginPlot("I/Q constellation",
                                  ImVec2(-1, plot_height))) {
              ImPlot::PlotScatterG("symbols", get_points<double>,
                                   &tx_config.symbols,
                                   tx_config.symbols.size());

              ImPlot::EndPlot();
            }

            if (ImPlot::BeginPlot("I/Q samples", ImVec2(-1, plot_height))) {
              ImPlot::SetupAxes("Time", "Amplitude");
              ImPlot::PlotLineG("I component", get_I<double>,
                                &tx_config.ofdm_signal,
                                tx_config.ofdm_signal.size());
              ImPlot::PlotLineG("Q component", get_Q<double>,
                                &tx_config.ofdm_signal,
                                tx_config.ofdm_signal.size());
              ImPlot::EndPlot();
            }

            ImGui::EndChild();
          }

          ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Receiver")) {
          if (ImGui::BeginChild("RX", ImVec2(left_width, 0), true)) {
            ImGui::Text("Settings");

            ImGui::SeparatorText("Modulator");
            ImGui::RadioButton("BPSK", &rx_config.mod_order, 2);
            ImGui::RadioButton("QPSK", &rx_config.mod_order, 4);
            ImGui::RadioButton("QAM16", &rx_config.mod_order, 16);

            ImGui::SeparatorText("Downsampler");
            ImGui::SliderInt("Samples per symbol", &rx_config.sps, 1, 100);

            ImGui::SeparatorText("Pulse Shaping filter");
            ImGui::RadioButton("Rectangle", &rx_config.IR_type, 0);
            ImGui::RadioButton("Rised-Cosine", &rx_config.IR_type, 1);

            ImGui::SeparatorText("Gardner (symbol sync)");
            ImGui::InputDouble("gBnTs", &rx_config.gardner_BnTs, 0, 3.0f,
                               "%.12f");
            ImGui::InputDouble("gKp", &rx_config.gardner_Kp, 0, 10, "%.8f");

            ImGui::SeparatorText("Costas (frequency sync)");
            ImGui::InputDouble("cBnTs", &rx_config.costas_BnTs, 0, 0.1, "%.7f");
            ImGui::InputDouble("Kp", &rx_config.costas_Kp, 0, 10, "%.7f");

            ImGui::SeparatorText("Technology");
            ImGui::RadioButton("OFDM", &rx_config.OFDM, 1);
            ImGui::RadioButton("Non-OFDM", &rx_config.OFDM, 0);

            if (rx_config.OFDM) {
              ImGui::SeparatorText("OFDM");
              ImGui::InputInt("Subcarriers count", &rx_config.FFT_size, 1, 128);
              ImGui::InputInt("Cycle prefix size", &rx_config.CP_size, 1, 128);
            }

            ImGui::EndChild();
          }

          ImGui::SameLine();

          if (ImGui::BeginChild("RX_Plots", ImVec2(0, 0), true)) {

            // if (ImPlot::BeginPlot("CFO spectrum", ImVec2(-1, plot_height)))
            // {
            //   ImPlot::SetupAxes("frequency", "Amplitude");
            //   ImPlot::PlotLineG("I component", get_amp_spec,
            //                     &rx_config.CFO_spectrum.first,
            //                     rx_config.CFO_spectrum.first.size());
            //   ImPlot::EndPlot();
            // }

            // if (ImPlot::BeginPlot("I/Q samples", ImVec2(-1, plot_height)))
            // {
            //   ImPlot::SetupAxes("Time", "Amplitude");
            //   ImPlot::PlotLineG("I component", get_I<int16_t>,
            //                     &rx_config.rx_samples,
            //                     rx_config.rx_samples.size());
            //   ImPlot::PlotLineG("Q component", get_Q<int16_t>,
            //                     &rx_config.rx_samples,
            //                     rx_config.rx_samples.size());
            //   ImPlot::EndPlot();
            // }

            // if (ImPlot::BeginPlot("POST CFO spectrum",
            //                       ImVec2(-1, plot_height)))
            // {
            //   ImPlot::SetupAxes("frequency", "Amplitude");
            //   ImPlot::PlotLineG("I component", get_amp_spec,
            //                     &rx_config.post_CFO_spectrum.first,
            //                     rx_config.post_CFO_spectrum.first.size());
            //   ImPlot::EndPlot();
            // }

            // if (ImPlot::BeginPlot("POST FINE CFO spectrum",
            //                       ImVec2(-1, plot_height)))
            // {
            //   ImPlot::SetupAxes("frequency", "Amplitude");
            //   ImPlot::PlotLineG("I component", get_amp_spec,
            //                     &rx_config.post_fine_CFO_spectrum,
            //                     rx_config.post_fine_CFO_spectrum.first.size());
            //   ImPlot::EndPlot();
            // }

            // if (ImPlot::BeginPlot("Amplitude spectrum",
            //                       ImVec2(-1, plot_height)))
            // {
            //   ImPlot::SetupAxes("frequency", "Amplitude");
            //   ImPlot::PlotLineG("I component", get_amp_spec,
            //                     &rx_config.spectrum.first,
            //                     rx_config.spectrum.first.size());
            //   ImPlot::EndPlot();
            // }

            // if (ImPlot::BeginPlot("Phase spectrum", ImVec2(-1, plot_height)))
            // {
            //   ImPlot::SetupAxes("frequency", "Amplitude");
            //   ImPlot::PlotLineG("I component", get_phase_spec,
            //                     &rx_config.spectrum,
            //                     rx_config.spectrum.first.size());
            //   ImPlot::EndPlot();
            // }

            // if (ImPlot::BeginPlot("Before Gardner I/Q constellation",
            //                       ImVec2(-1, plot_height)))
            // {

            //   ImPlot::PlotScatterG("Raw symbols", get_points<int16_t>,
            //                        &rx_config.rx_samples,
            //                        rx_config.rx_samples.size());

            //   ImPlot::EndPlot();
            // }

            // if (ImPlot::BeginPlot("Matched filter output",
            //                       ImVec2(-1, plot_height)))
            // {
            //   ImPlot::SetupAxes("Time", "Amplitude");
            //   ImPlot::PlotLineG("I component", get_I<double>,
            //                     &rx_config.mf_samples_out,
            //                     rx_config.mf_samples_out.size() / 2);
            //   ImPlot::PlotLineG("Q component", get_Q<double>,
            //                     &rx_config.mf_samples_out,
            //                     rx_config.mf_samples_out.size() / 2);
            //   ImPlot::EndPlot();
            // }

            // if (ImPlot::BeginPlot("Post Gardner I/Q constellation",
            //                       ImVec2(-1, plot_height)))
            // {
            //   ImPlot::PlotScatterG("Symbols", get_points<double>,
            //                        &rx_config.raw_symbols,
            //                        rx_config.raw_symbols.size());

            //   ImPlot::EndPlot();
            // }

            // if (ImPlot::BeginPlot("Post Costas Loop I/Q constellation",
            //                       ImVec2(-1, plot_height)))
            // {
            //   ImPlot::PlotScatterG("Symbols", get_points<double>,
            //                        &rx_config.post_costas,
            //                        rx_config.post_costas.size());

            //   ImPlot::EndPlot();
            // }

            if (ImPlot::BeginPlot("Correlation function",
                                  ImVec2(-1, plot_height))) {
              ImPlot::SetupAxes("Time", "Amplitude");
              ImPlot::PlotLineG("I component", get_value<double>,
                                &rx_config.corr_func,
                                rx_config.corr_func.size());

              ImPlot::EndPlot();
            }

            ImGui::EndChild();
          }

          ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
      }

      ImGui::End();
    }

    ImGui::Render();
    glViewport(0, 0, 1024, 768);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(window);
  }

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImPlot::DestroyContext();
  ImGui::DestroyContext();

  SDL_GL_DeleteContext(gl_context);
  SDL_DestroyWindow(window);
  SDL_Quit();
}
