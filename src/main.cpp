#include <stdx/profiler.hh>
#include <stdx/types.hh>

#include "app/app.hpp"

i32 main(i32 argc, char** argv) {
    stdx::profiler profiler{argv[0]};
    mbr::app_t     app{argc, argv};
    app.run();
}
