#include <sokol_app.h>
#include <sokol_gfx.h>
#include <sokol_glue.h>
#include <sokol_imgui.h>

#include "core/log.hpp"

#include "app/gui.hpp"

#include "esp32/backend.hpp"
#include "esp32/data.hpp"

sapp_desc sokol_main(int argc, char* argv[]) {
    Log::Init();
    static std::shared_ptr<AppContext> ctx = std::make_shared<AppContext>();
    static GUI                         app(ctx);
    return app.GetSokolDesc();
}
