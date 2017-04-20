#include "GameApp.h"

GameApp app;

#undef main
int main(int argc, char *argv[]) {
    return app.Run(std::vector<std::string>(argv + 1, argv + argc));
}
