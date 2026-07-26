#include "app/app.hpp"

int main(int argc, char* argv[]) {
    mbr::app_t app{argc, argv};
    app.run();
}
