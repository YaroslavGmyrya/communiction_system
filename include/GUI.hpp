#pragma once

#include <complex>
#include <vector>

#include <GL/glew.h>
#include <SDL2/SDL.h>

#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl2.h"
#include "imgui.h"
#include "implot.h"

#include "structures.hpp"

/*GUI thread. This thread drawing GUI with plots and settings for model*/
void run_gui(tx_cfg &tx_config, rx_cfg &rx_config);
