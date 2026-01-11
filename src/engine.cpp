#include "bgfx/embedded_shader.h"
#include "glm/ext/scalar_constants.hpp"
#include <bgfx/defines.h>
#include <bx/file.h>
#include <bx/math.h>
#include <cmath>
#include <exception>
#include <glm/fwd.hpp>
#include <cstring>
#include <engine.hpp>
#include <bgfx/bgfx.h>
#include <glm/glm.hpp>
#include <common.hpp>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_X11
#include <GLFW/glfw3native.h>
#include <engine/quad.hpp>
#include <imgui.h>
#include <engine/imgui.hpp>
#include <engine/graph.hpp>
#include <engine/model.hpp>

Engine::Engine(void) : last_time(0), dt(0), time(0), input_map(0), focused(ENGINE_INTERMEDIATE_GPU), renderer(), ui(), EngineApi() {
    this->title = "Mapping engine";

    memset(this->keyboard_slots, 0, sizeof(this->keyboard_slots));
    memset(this->cursor_slots, 0, sizeof(this->cursor_slots));
}

void Engine::keyboard_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    Engine *engine = (Engine *) glfwGetWindowUserPointer(window);
    ImGuiIO &io = ImGui::GetIO(); 

    bool escape = check_input(key, action, GLFW_KEY_ESCAPE) ? ENGINE_INPUT_ESC : 0;
    if (escape) {
        engine->focused = ENGINE_INTERMEDIATE_GPU;  
        glfwSetInputMode(engine->main_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    if (engine->focused == ENGINE_INTERMEDIATE_GPU) {
        ImGuiKey imgui_key = ImGuiKey_None; 
        switch (key) {
            case GLFW_KEY_LEFT: imgui_key = ImGuiKey_LeftArrow; break; 
            case GLFW_KEY_RIGHT: imgui_key = ImGuiKey_RightArrow; break; 
            case GLFW_KEY_BACKSPACE: imgui_key = ImGuiKey_Backspace; break; 
        }

        if (imgui_key != ImGuiKey_None) {
            io.AddKeyEvent(imgui_key, action == GLFW_PRESS);
        }
        return; 
    }

    engine->keyboard_slots[key] = action; 

    engine->input_map |= check_input(key, action, GLFW_KEY_A) ? ENGINE_INPUT_A : 0;
    engine->input_map |= check_input(key, action, GLFW_KEY_D) ? ENGINE_INPUT_D : 0; 
    engine->input_map |= check_input(key, action, GLFW_KEY_W) ? ENGINE_INPUT_W : 0; 
    engine->input_map |= check_input(key, action, GLFW_KEY_S) ? ENGINE_INPUT_S : 0; 
}


double prev_x = 0.0f; 
double prev_y = 0.0f; 
bool start = true; 
void Engine::cursor_callback(GLFWwindow *window, double x, double y) {
    Engine *engine = (Engine *) glfwGetWindowUserPointer(window);
    if (engine->focused == ENGINE_INTERMEDIATE_GPU) {
        return; 
    }

    if (start == true) {
        prev_x = x; 
        prev_y = y; 
        start = false; 
    }

    float dx = x-prev_x; 
    float dy = y-prev_y; 

    prev_x = x;
    prev_y = y; 

    engine->renderer.CameraUpdate(dx, dy); 
}

void Engine::cursor_button_callback(GLFWwindow *window, int button, int action, int mods) {
    Engine *engine = (Engine *) glfwGetWindowUserPointer(window);
    engine->cursor_slots[button] = action;

    if (engine->focused == ENGINE_VIEWPORT) {
        return; 
    }

    ImGuiIO &io = ImGui::GetIO();
    if (button < 0) {
        return; 
    }

    if (button >= IM_ARRAYSIZE(io.MouseDown)) {
        return; 
    }

    if (action == GLFW_PRESS) {
        io.MouseDown[button] = true;
        return; 
    }

    io.MouseDown[button] = false;

    if (io.WantCaptureMouse) {
        return; 
    }

    glfwSetInputMode(engine->main_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    engine->focused = ENGINE_VIEWPORT; 
    start = true; 
}

void Engine::window_size_callback(GLFWwindow *window, int width, int height) {
    Engine *engine = (Engine *) glfwGetWindowUserPointer(window);
    engine->renderer.CameraResolution(width, height); 
    engine->Reset();
}

void Engine::scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    ImGuiIO &io = ImGui::GetIO();
    io.MouseWheelH += xoffset;
    io.MouseWheel += yoffset;
}

void Engine::char_callback(GLFWwindow *window, unsigned int codepoint) {
    ImGuiIO &io = ImGui::GetIO();
    io.AddInputCharacter(codepoint);
}

void Engine::Reset(void) {
    renderer.CameraReset(); 
}

int Engine::Start(int mode) {
    int ret = glfwInit();
    if (ret < 0) {
        ERROR("failed initializing glfw");
        return -1;
    }

    glfwSetErrorCallback(this->error_callback);

    this->main_window = glfwCreateWindow(DEFAULT_WIDTH, DEFAULT_HEIGHT, this->title.c_str(), NULL, NULL);
    if (!this->main_window) {
        ERROR("failed creating window");
        return -1;
    }

    glfwSetWindowUserPointer(main_window, this);
    glfwSetKeyCallback(main_window,keyboard_callback);
    glfwSetCursorPosCallback(main_window,cursor_callback);
    glfwSetMouseButtonCallback(main_window,cursor_button_callback);
    glfwSetWindowSizeCallback(main_window,window_size_callback);
    glfwSetScrollCallback(main_window,scroll_callback);
    glfwSetCharCallback(main_window,char_callback);
    glfwSetInputMode(main_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    if (renderer.Start(main_window, this, &ui) < 0) { 
        ERROR("failed starting renderer"); 
        return -1; 
    }

    if (ui.Start(this, &renderer) < 0) {
        ERROR("failed starting ui");
        return -1; 
    }

    imgui_init(this->main_window);

    renderer.CameraReset(); 

    return 0;
}

int Engine::UserLoad(void) {
    return 0;
}

void Engine::UserUpdate(void) {
    // do nothing lol 
}

int Engine::IsOk(void) {
    return !glfwWindowShouldClose(main_window);
}

void Engine::Update(void) {
    this->time = glfwGetTime();
    this->dt = this->time - this->last_time;
    this->last_time = this->time;

    glfwPollEvents();

    ui.NewUpdate(dt); 
    renderer.NewUpdate(dt);
    
    this->UserUpdate();

    ui.Update(dt);
    renderer.Update(dt); 
}

void Engine::Shutdown(void) { 
    imgui_shutdown();
    bgfx::shutdown();
    glfwTerminate();
}

void Engine::error_callback(int error, const char *s) {
    ERROR("glfw failed -> " << s);
}
