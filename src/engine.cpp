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

Engine::Engine(void) {
    this->width = DEFAULT_WIDTH;
    this->height = DEFAULT_HEIGHT;
    this->title = "Polynomial Graphical Viewer";

    this->main_view = 0;

    memset(this->keyboard_slots, 0, sizeof(this->keyboard_slots));
    memset(this->cursor_slots, 0, sizeof(this->cursor_slots));

    this->last_time = 0; 
    this->dt = 0; 
    this->time = 0; 
    this->input_map = 0; 
    this->debug_flag = 0; 
}

void Engine::keyboard_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    Engine *engine = (Engine *) glfwGetWindowUserPointer(window);
    engine->keyboard_slots[key] = action; 

    unsigned char new_input_map; 
    new_input_map |= (key == GLFW_KEY_A) && ((action & GLFW_REPEAT) || (action & GLFW_PRESS)) ? INPUT_A : 0;
    new_input_map |= (key == GLFW_KEY_D) && ((action & GLFW_REPEAT) || (action & GLFW_PRESS)) ? INPUT_D : 0; 
    new_input_map |= (key == GLFW_KEY_W) && ((action & GLFW_REPEAT) || (action & GLFW_PRESS)) ? INPUT_W : 0; 
    new_input_map |= (key == GLFW_KEY_S) && ((action & GLFW_REPEAT) || (action & GLFW_PRESS)) ? INPUT_S : 0; 
    new_input_map |= (key == GLFW_KEY_UP) && ((action & GLFW_REPEAT) || (action & GLFW_PRESS)) ? INPUT_ZOOM_IN : 0; 
    new_input_map |= (key == GLFW_KEY_DOWN) && ((action & GLFW_REPEAT) || (action & GLFW_PRESS)) ? INPUT_ZOOM_OUT : 0; 

    engine->input_map = new_input_map;
}

bx::Vec3 at = {0.0f, 0.0f, 0.0f};
bx::Vec3 eye = {0.0f, 0.0f, -35.0f};

double prev_x = 0.0f; 
void Engine::cursor_callback(GLFWwindow *window, double x, double y) {
    Engine *engine = (Engine *) glfwGetWindowUserPointer(window);
    engine->cursor_xpos = x;
    engine->cursor_ypos = y;

    float dx = x-prev_x; 
    prev_x = x;
    eye.x -= dx / 10;
}

void Engine::cursor_button_callback(GLFWwindow *window, int button, int action, int mods) {
    Engine *engine = (Engine *) glfwGetWindowUserPointer(window);
    engine->cursor_slots[button] = action;

    ImGuiIO &io = ImGui::GetIO();
    if (button >= 0 && button < IM_ARRAYSIZE(io.MouseDown)) {
        if (action == GLFW_PRESS) {
            io.MouseDown[button] = true;
        }
        else {
            io.MouseDown[button] = false;
        }
    }
}

void Engine::window_size_callback(GLFWwindow *window, int width, int height) {
    Engine *engine = (Engine *) glfwGetWindowUserPointer(window);
    engine->width = width; 
    engine->height = height;
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
    bgfx::reset(this->width, this->height, 0);
    imgui_reset(this->width, this->height);
    bgfx::setViewRect(this->main_view, 0, 0, width, height);
    bgfx::setViewClear(this->main_view, 
            BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 
            0x00000000,
            1.0f,
            0);
}

int Engine::Init(int mode) {
    int ret = glfwInit();
    if (ret < 0) {
        ERROR("failed initializing glfw");
        return -1;
    }

    glfwSetErrorCallback(this->error_callback);

    this->main_window = glfwCreateWindow(this->width, this->height, this->title.c_str(), 
#ifdef GLFW_DEBUG
            NULL
#else 
            glfwGetPrimaryMonitor() // fullscreen
#endif
            , NULL);
    if (!this->main_window) {
        ERROR("failed creating window");
        return -1;
    }

    glfwSetWindowUserPointer(this->main_window, this);
    glfwSetKeyCallback(this->main_window, this->keyboard_callback);
    glfwSetCursorPosCallback(this->main_window, this->cursor_callback);
    glfwSetMouseButtonCallback(this->main_window, this->cursor_button_callback);
    glfwSetWindowSizeCallback(this->main_window, this->window_size_callback);
    glfwSetScrollCallback(this->main_window, this->scroll_callback);
    glfwSetCharCallback(this->main_window, this->char_callback);;

#ifdef GLFW_DEBUG
    glfwSetInputMode(this->main_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
#endif

    bgfx::Init init;
    init.platformData.ndt = glfwGetX11Display();
    init.platformData.nwh = (void*) (uintptr_t) glfwGetX11Window(this->main_window);
    
    glfwGetWindowSize(this->main_window, &this->width, &this->height);
    init.resolution.width = this->width; 
    init.resolution.height = this->height; 
    init.resolution.reset = BGFX_RESET_VSYNC;
    if (!bgfx::init(init)) {
        ERROR("failed initializing bgfx");
        return -1;
    }

    bgfx::setViewClear(this->main_view, 
            BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 
            0x00000000,
            1.0f,
            0);

    {
        float view[16];
        bx::mtxLookAt(view, eye, at);

        float proj[16];
        bx::mtxProj(proj, 60.0f, float(this->width)/float(this->height), 0.1f, 500.0f, bgfx::getCaps()->homogeneousDepth);

        bgfx::setViewTransform(this->main_view, view, proj);
    }

    bgfx::setViewRect(this->main_view, 0, 0, this->width, this->height);

    this->u_position = bgfx::createUniform("u_position", bgfx::UniformType::Vec4);
    this->u_rotation = bgfx::createUniform("u_rotation", bgfx::UniformType::Vec4);
    this->u_scale = bgfx::createUniform("u_scale", bgfx::UniformType::Vec4);
    this->program = load_program(DEFAULT_VERTEX, DEFAULT_FRAGMENT);

    imgui_init(this->main_window);
    this->Reset();

    ret = this->UserLoad();
    if (ret < 0) {
        return -1;
    }

    return 0;
}

float computation_function(float time, float x, float y) {
    return x/y; 
}

int Engine::UserLoad(void) {
    EngineObject obj; 
    obj.model = new ModelComponent(); 
    obj.model->LoadModel("models/dragon.obj");

    obj.rotation.x = glm::pi<float>()/3;
    obj.position.z = 4.5f;

    this->objs.push_back(obj);

    return 0;
}

void Engine::ImguiUpdate(EngineObject *obj) {
    ImGui::ShowDemoWindow();

    ImGui::Begin("Multi-Dimensional Curve Renderer", NULL, ImGuiWindowFlags_NoResize);
    // general properties
    ImGui::SeparatorText("General");
    {
        ImGui::PushItemWidth(-100);
        if (ImGui::Button("Toggle Debug Wireframe")) {
            this->debug_flag = (this->debug_flag+1) % 2;
        }
    }

    ImGui::SeparatorText("Position");
    { 
        ImGui::PushItemWidth(-100);
        ImGui::DragFloat("position x", &obj->position.x, 0.01f, -30.0f, 30.0f);

        ImGui::PushItemWidth(-100);
        ImGui::DragFloat("position y", &obj->position.y, 0.01f, -30.0f, 30.0f);

        ImGui::PushItemWidth(-100);
        ImGui::DragFloat("position z", &obj->position.z, 0.01f, -30.0f, 30.0f);
    }

    ImGui::SeparatorText("Rotation");
    { 
        ImGui::PushItemWidth(-100);
        ImGui::DragFloat("rotation x", &obj->rotation.x, 0.01f, -2*glm::pi<float>(), 2*glm::pi<float>());

        ImGui::PushItemWidth(-100);
        ImGui::DragFloat("rotation y", &obj->rotation.y, 0.01f, -2*glm::pi<float>(), 2*glm::pi<float>());

        ImGui::PushItemWidth(-100);
        ImGui::DragFloat("rotation z", &obj->rotation.z, 0.01f, -2*glm::pi<float>(), 2*glm::pi<float>());
    }

    ImGui::End();
}

void Engine::UserUpdate(void) {
    bgfx::setDebug(this->debug_flag ? BGFX_DEBUG_WIREFRAME : 0);

    EngineObject *obj = &this->objs[0]; 
    this->ImguiUpdate(obj);

    // render
    bgfx::setState(obj->model->render_state);
    bgfx::setVertexBuffer(0, obj->model->vbh);
    bgfx::setIndexBuffer(obj->model->ibh);
    bgfx::setUniform(this->u_position, &obj->position);
    bgfx::setUniform(this->u_rotation, &obj->rotation);
    bgfx::setUniform(this->u_scale, &obj->scale);
    bgfx::submit(this->main_view, this->program);
}

void Engine::Update(void) {
    this->time = glfwGetTime();
    this->dt = this->time - this->last_time;
    this->last_time = this->time;

    glfwPollEvents();

    imgui_events(this->dt);
    ImGui::NewFrame();
    bgfx::touch(this->main_view);

    this->UserUpdate();

    bgfx::frame();
    ImGui::Render();
    imgui_render(ImGui::GetDrawData());
}

void Engine::Shutdown(void) { 
    imgui_shutdown();

    bgfx::destroy(this->u_position);
    bgfx::destroy(this->u_rotation);
    bgfx::destroy(this->u_scale);

    bgfx::destroy(this->program);

    bgfx::shutdown();
    glfwTerminate();
}

void Engine::error_callback(int error, const char *s) {
    ERROR("glfw failed -> " << s);
}
