#include "app/app.hpp"

int main(int arc, char* argv[]) {
    auto app = std::make_unique<App>();
    app->Run();
}
