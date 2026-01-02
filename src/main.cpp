#include "app/app.hpp"
#include <ixwebsocket/IXNetSystem.h>

int main(int arc, char* argv[]) {
    ix::initNetSystem();
    App app;
    app->Run();
}
