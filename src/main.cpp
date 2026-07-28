#include "app/app.hpp"
#include <stdx/profiler.hh>

int main(int argc, char* argv[]) {
    stdx::profiler profiler{argv[0]};
    mbr::app_t app{argc, argv};
    app.run();
}
