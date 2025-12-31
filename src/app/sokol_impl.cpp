#include <GLFW/glfw3.h>

#if defined(_WIN32)
#define SOKOL_D3D11
#elif defined(__APPLE__)
#define SOKOL_METAL
#endif

#define SOKOL_IMPL
#include "sokol_gfx.h"
#include "sokol_glue.h"

#define SOKOL_IMGUI_IMPL
#include "sokol_imgui.h"
