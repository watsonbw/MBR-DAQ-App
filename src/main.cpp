#include "app/app.hpp"
#include <stdx/profiler.hh>

i32 main(i32 argc, char* argv[]) {
    stdx::profiler profiler{argv[0]};
    mbr::app_t     app{argc, argv};
    app.run();
    return 0;
}
