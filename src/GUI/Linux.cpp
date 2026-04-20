#include <complex.h>
#include <iostream>
#include <stdint.h>
#include <stdlib.h>
#include <thread>
#include <unistd.h>
#include <vector>

#include "../include/GUI.hpp"

template <typename T>
ImPlotPoint get_value(int idx, void *user_data)
{
  auto *vec = static_cast<std::vector<T> *>(user_data);
  return ImPlotPoint(idx, (*vec)[idx]);
}

template <typename T>
ImPlotPoint get_I(int idx, void *user_data)
{
  auto *vec = static_cast<std::vector<std::complex<T>> *>(user_data);
  return ImPlotPoint(idx, (*vec)[idx].real());
}

template <typename T>
ImPlotPoint get_Q(int idx, void *user_data)
{
  auto *vec = static_cast<std::vector<std::complex<T>> *>(user_data);
  return ImPlotPoint(idx, (*vec)[idx].imag());
}

template <typename T>
ImPlotPoint get_points(int idx, void *data)
{
  auto *vec = static_cast<std::vector<std::complex<T>> *>(data);

  const auto &s = (*vec)[idx];

  return ImPlotPoint(s.real(), s.imag());
}

template <typename T>
ImPlotPoint get_abs(int idx, void *data)
{
  auto *vec = static_cast<std::vector<std::complex<T>> *>(data);

  const auto &s = (*vec)[idx];

  return ImPlotPoint(idx, std::abs((*vec)[idx]));
}

ImPlotPoint get_phase_spec(int idx, void *data)
{
  auto *fft = static_cast<
      std::pair<std::vector<std::complex<double>>, std::vector<double>> *>(
      data);

  const auto &samples = fft->first;
  const auto &freqs = fft->second;

  double x = freqs[idx];
  double y = std::arg(samples[idx]);

  return ImPlotPoint(x, y);
}

ImPlotPoint get_amp_spec(int idx, void *data)
{
  auto *fft = static_cast<
      std::pair<std::vector<std::complex<double>>, std::vector<double>> *>(
      data);

  const auto &samples = fft->first;
  const auto &freqs = fft->second;

  double x = freqs[idx];
  double y = std::abs(samples[idx]);

  return ImPlotPoint(x, y);
}

void run_gui(tx_cfg &tx_config, rx_cfg &rx_config)
{
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

  while (running)
  {
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
      ImGui_ImplSDL2_ProcessEvent(&event);
      if (event.type == SDL_QUIT)
      {
        tx_config.run = false;
        rx_config.run = false;
        running = false;
      }
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    ImGui::DockSpaceOverViewport(0, nullptr, ImGuiDockNodeFlags_None);

    if (ImGui::Begin("Simulator"))
    {
      if (ImGui::BeginTabBar("MyTabBar"))
      {
        if (ImGui::BeginTabItem("Transmitter"))
        {
          /*========================================= TX PARAMETERS ====================================================*/
          if (ImGui::BeginChild("TX", ImVec2(left_width, 0), true))
          {
            ImGui::Text("Settings");

            ImGui::SeparatorText("Modulator");
            if (ImGui::RadioButton("BPSK", &tx_config.mod_order, 2))
            {
            }
            if (ImGui::RadioButton("QPSK", &tx_config.mod_order, 4))
            {
            }
            if (ImGui::RadioButton("QAM16", &tx_config.mod_order, 16))
            {
            }
            if (ImGui::RadioButton("QAM64", &tx_config.mod_order, 64))
            {
            }
            if (ImGui::RadioButton("QAM256", &tx_config.mod_order, 256))
            {
            }
            if (ImGui::RadioButton("QAM1024", &tx_config.mod_order, 1024))
            {
            }

            if (ImGui::Checkbox("DEBUG MODE", &tx_config.DEBUG_MODE))
            {
            }

            ImGui::SeparatorText("OFDM");
            ImGui::InputInt("FFT size", &tx_config.FFT_size, 1, 128);
            ImGui::InputInt("Cyclic prefix size", &tx_config.CP_size, 1, 128);
            ImGui::InputInt("Count of pilots", &tx_config.pilots_count, 1,
                            128);
            ImGui::SliderInt("Guard interval size (double side)",
                             &tx_config.guard_size, 2,
                             tx_config.FFT_size / 3);

            ImGui::EndChild();
          }

          ImGui::SameLine();
          /*========================================= TX PLOTS ====================================================*/

          if (ImGui::BeginChild("TX_Plots", ImVec2(0, 0), true))
          {

            if (ImPlot::BeginPlot("I/Q constellation", ImVec2(-1, plot_height)))
            {
              ImPlot::SetupAxes("I", "Q");
              ImPlot::PlotScatterG("symbols", get_points<double>,
                                   &tx_config.symbols,
                                   tx_config.symbols.size());

              ImPlot::EndPlot();
            }

            if (ImPlot::BeginPlot("OFDM signal", ImVec2(-1, plot_height)))
            {
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

        if (ImGui::BeginTabItem("Receiver"))
        {
          /*========================================= RX PARAMETERS ====================================================*/

          if (ImGui::BeginChild("RX", ImVec2(left_width, 0), true))
          {
            ImGui::Text("Settings");

            ImGui::SeparatorText("Modulator");
            ImGui::RadioButton("BPSK", &rx_config.mod_order, 2);
            ImGui::RadioButton("QPSK", &rx_config.mod_order, 4);
            ImGui::RadioButton("QAM16", &rx_config.mod_order, 16);
            ImGui::RadioButton("QAM64", &rx_config.mod_order, 64);
            ImGui::RadioButton("QAM256", &rx_config.mod_order, 256);
            ImGui::RadioButton("QAM1024", &rx_config.mod_order, 1024);

            if (ImGui::Checkbox("DEBUG MODE", &rx_config.DEBUG_MODE))
            {
            }

            ImGui::SeparatorText("OFDM");
            ImGui::InputInt("FFT size", &rx_config.FFT_size, 1, 128);
            ImGui::InputInt("Cyclic prefix size", &rx_config.CP_size, 1, 128);
            ImGui::InputInt("Count of pilots", &rx_config.pilots_count, 1, 128);
            ImGui::SliderInt("Guard interval size (double side)", &rx_config.guard_size, 2, rx_config.FFT_size / 3);

            ImGui::SeparatorText("Channel");

            ImGui::SliderFloat("SNR", &rx_config.SNR, -100, 100);

            ImGui::EndChild();
          }

          ImGui::SameLine();

          /*========================================= RX PLOTS ====================================================*/

          if (ImGui::BeginChild("RX_Plots", ImVec2(0, 0), true))
          {

            if (ImPlot::BeginPlot("RX SIGNAL", ImVec2(-1, plot_height)))
            {
              ImPlot::SetupAxes("time, s", "Amplitude");
              ImPlot::PlotLineG("I component", get_I<double>,
                                &rx_config.rx_samples,
                                rx_config.rx_samples.size());

              ImPlot::PlotLineG("Q component", get_Q<double>,
                                &rx_config.rx_samples,
                                rx_config.rx_samples.size());
              ImPlot::EndPlot();
            }

            if (ImPlot::BeginPlot("PSS CORRELATION", ImVec2(-1, plot_height)))
            {
              ImPlot::SetupAxes("Time", "Corr");
              ImPlot::PlotLineG("Corr lvl", get_value<double>,
                                &rx_config.zc_corr,
                                rx_config.zc_corr.size());

              ImPlot::EndPlot();
            }

            if (ImPlot::BeginPlot("CP correlation",
                                  ImVec2(-1, plot_height)))
            {
              ImPlot::SetupAxes("Time", "CP corr");
              ImPlot::PlotLineG("CP corr", get_value<double>,
                                &rx_config.CP_corr,
                                rx_config.CP_corr.size());
              ImPlot::EndPlot();
            }

            if (ImPlot::BeginPlot("CHANNEL ESTIMATION",
                                  ImVec2(-1, plot_height)))
            {
              ImPlot::SetupAxes("Time", "ESTIMATION");
              ImPlot::PlotLineG("ESTIMATION", get_abs<double>,
                                &rx_config.estimation,
                                rx_config.estimation.size());
              ImPlot::EndPlot();
            }

            if (ImPlot::BeginPlot("RX CONSTELLATION",
                                  ImVec2(-1, plot_height)))
            {
              ImPlot::SetupAxes("I", "Q");
              ImPlot::PlotScatterG("Points", get_points<double>,
                                   &rx_config.raw_symbols,
                                   rx_config.raw_symbols.size());
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
