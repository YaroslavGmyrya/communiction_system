#include <complex.h>
#include <iostream>
#include <stdint.h>
#include <stdlib.h>
#include <thread>
#include <unistd.h>
#include <vector>

#include "../include/ImGUI_interface.hpp"

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_metal.h"

#define GLFW_INCLUDE_NONE
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>

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
  auto *fft = static_cast<std::pair<std::vector<std::complex<double>>, std::vector<double>> *>(data);
  const auto &samples = fft->first;
  const auto &freqs = fft->second;
  double x = freqs[idx];
  double y = std::arg(samples[idx]);
  return ImPlotPoint(x, y);
}

ImPlotPoint get_amp_spec(int idx, void *data)
{
  auto *fft = static_cast<std::pair<std::vector<std::complex<double>>, std::vector<double>> *>(data);
  const auto &samples = fft->first;
  const auto &freqs = fft->second;
  double x = freqs[idx];
  double y = std::abs(samples[idx]);
  return ImPlotPoint(x, y);
}

void run_gui(tx_cfg &tx_config, rx_cfg &rx_config)
{
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  float main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor());
  GLFWwindow *window = glfwCreateWindow((int)(1280 * main_scale), (int)(800 * main_scale), "Simulator", nullptr, nullptr);

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImPlot::CreateContext();

  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  ImGui::StyleColorsDark();
  ImGuiStyle &style = ImGui::GetStyle();
  style.ScaleAllSizes(main_scale);
  style.FontScaleDpi = main_scale;

  id<MTLDevice> device = MTLCreateSystemDefaultDevice();
  id<MTLCommandQueue> commandQueue = [device newCommandQueue];

  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplMetal_Init(device);

  NSWindow *nswin = glfwGetCocoaWindow(window);
  CAMetalLayer *layer = [CAMetalLayer layer];
  layer.device = device;
  layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
  nswin.contentView.layer = layer;
  nswin.contentView.wantsLayer = YES;

  MTLRenderPassDescriptor *renderPassDescriptor = [MTLRenderPassDescriptor new];

  float left_width = 450.0f;
  float plot_height = 250.0f;

  while (!glfwWindowShouldClose(window))
  {
    @autoreleasepool
    {
      glfwPollEvents();

      int width, height;
      glfwGetFramebufferSize(window, &width, &height);
      layer.drawableSize = CGSizeMake(width, height);
      id<CAMetalDrawable> drawable = [layer nextDrawable];

      id<MTLCommandBuffer> commandBuffer = [commandQueue commandBuffer];
      renderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(0.1, 0.1, 0.1, 1.0);
      renderPassDescriptor.colorAttachments[0].texture = drawable.texture;
      renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
      renderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
      id<MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];

      ImGui_ImplMetal_NewFrame(renderPassDescriptor);
      ImGui_ImplGlfw_NewFrame();
      ImGui::NewFrame();

      ImGui::DockSpaceOverViewport(0, nullptr, ImGuiDockNodeFlags_None);

      if (ImGui::Begin("Simulator"))
      {
        if (ImGui::BeginTabBar("MyTabBar"))
        {
          if (ImGui::BeginTabItem("Transmitter"))
          {
            if (ImGui::BeginChild("TX", ImVec2(left_width, 0), true))
            {
              ImGui::Text("Settings");

              ImGui::SeparatorText("Modulator");
              ImGui::RadioButton("BPSK", &tx_config.mod_order, 2);
              ImGui::RadioButton("QPSK", &tx_config.mod_order, 4);
              ImGui::RadioButton("QAM16", &tx_config.mod_order, 16);
              ImGui::RadioButton("QAM64", &tx_config.mod_order, 64);
              ImGui::RadioButton("QAM256", &tx_config.mod_order, 256);
              ImGui::RadioButton("QAM1024", &tx_config.mod_order, 1024);

              ImGui::Checkbox("DEBUG MODE", &tx_config.DEBUG_MODE);

              ImGui::SeparatorText("OFDM");
              ImGui::InputInt("FFT size", &tx_config.FFT_size, 1, 128);
              ImGui::InputInt("Cyclic prefix size", &tx_config.CP_size, 1, 128);
              ImGui::InputInt("Count of pilots", &tx_config.pilots_count, 1, 128);
              ImGui::SliderInt("Guard interval size (double side)", &tx_config.guard_size, 2, tx_config.FFT_size / 3);

              ImGui::EndChild();
            }

            ImGui::SameLine();

            if (ImGui::BeginChild("TX_Plots", ImVec2(0, 0), true))
            {
              if (ImPlot::BeginPlot("I/Q constellation", ImVec2(-1, plot_height)))
              {
                ImPlot::SetupAxes("I", "Q");
                ImPlot::PlotScatterG("symbols", get_points<double>, &tx_config.symbols, tx_config.symbols.size());
                ImPlot::EndPlot();
              }

              if (ImPlot::BeginPlot("OFDM signal", ImVec2(-1, plot_height)))
              {
                ImPlot::SetupAxes("Time", "Amplitude");
                ImPlot::PlotLineG("I component", get_I<double>, &tx_config.ofdm_signal, tx_config.ofdm_signal.size());
                ImPlot::PlotLineG("Q component", get_Q<double>, &tx_config.ofdm_signal, tx_config.ofdm_signal.size());
                ImPlot::EndPlot();
              }

              ImGui::EndChild();
            }

            ImGui::EndTabItem();
          }

          if (ImGui::BeginTabItem("Receiver"))
          {
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

              ImGui::Checkbox("DEBUG MODE", &rx_config.DEBUG_MODE);

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

            if (ImGui::BeginChild("RX_Plots", ImVec2(0, 0), true))
            {
              if (ImPlot::BeginPlot("RX SIGNAL", ImVec2(-1, plot_height)))
              {
                ImPlot::SetupAxes("time, s", "Amplitude");
                ImPlot::PlotLineG("I component", get_I<double>, &rx_config.rx_samples, rx_config.rx_samples.size());
                ImPlot::PlotLineG("Q component", get_Q<double>, &rx_config.rx_samples, rx_config.rx_samples.size());
                ImPlot::EndPlot();
              }

              if (ImPlot::BeginPlot("PSS CORRELATION", ImVec2(-1, plot_height)))
              {
                ImPlot::SetupAxes("Time", "Corr");
                ImPlot::PlotLineG("Corr lvl", get_value<double>, &rx_config.zc_corr, rx_config.zc_corr.size());
                ImPlot::EndPlot();
              }

              if (ImPlot::BeginPlot("CP correlation", ImVec2(-1, plot_height)))
              {
                ImPlot::SetupAxes("Time", "CP corr");
                ImPlot::PlotLineG("CP corr", get_value<double>, &rx_config.CP_corr, rx_config.CP_corr.size());
                ImPlot::EndPlot();
              }

              if (ImPlot::BeginPlot("CHANNEL ESTIMATION", ImVec2(-1, plot_height)))
              {
                ImPlot::SetupAxes("Time", "ESTIMATION");
                ImPlot::PlotLineG("ESTIMATION", get_abs<double>, &rx_config.estimation, rx_config.estimation.size());
                ImPlot::EndPlot();
              }

              if (ImPlot::BeginPlot("RX CONSTELLATION", ImVec2(-1, plot_height)))
              {
                ImPlot::SetupAxes("I", "Q");
                ImPlot::PlotScatterG("Points", get_points<double>, &rx_config.raw_symbols, rx_config.raw_symbols.size());
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
      ImGui_ImplMetal_RenderDrawData(ImGui::GetDrawData(), commandBuffer, renderEncoder);

      [renderEncoder endEncoding];
      [commandBuffer presentDrawable:drawable];
      [commandBuffer commit];
    }
  }

  ImGui_ImplMetal_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImPlot::DestroyContext();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();
}