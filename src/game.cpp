#include <bgfx/bgfx.h>
#include <GLFW/glfw3.h>
#include <common.hpp>
#include <engine.hpp>

int main(int argc, char *argv[]) {
    Engine engine;
    
    int mode = 0; 
    if (argc == 2) {
        if (strcmp("map", argv[1]) == 0) {
            mode = 1; 
        }
    }

    int ret = engine.Start(mode);
    if (ret < 0) {
        ERROR("failed initializing engine");
        return EXIT_FAILURE;
    }

    while (engine.IsOk()) { 
        engine.Update();
    }
    engine.Shutdown();

    return EXIT_SUCCESS;
}
