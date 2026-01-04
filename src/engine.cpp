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

Engine::Engine(void) : 
    main_view(0),
    last_time(0),
    dt(0),
    time(0),
    input_map(0),
    focused(ENGINE_INTERMEDIATE_GPU),
    debug_stats(0),
    debug_wireframe(0),
    pitch(0),
    yaw(0), 
    obj_selected(0) {
    this->width = DEFAULT_WIDTH;
    this->height = DEFAULT_HEIGHT;
    this->title = "Dev game";

    memset(this->keyboard_slots, 0, sizeof(this->keyboard_slots));
    memset(this->cursor_slots, 0, sizeof(this->cursor_slots));
}

void Engine::keyboard_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    Engine *engine = (Engine *) glfwGetWindowUserPointer(window);
    engine->keyboard_slots[key] = action; 

    uint32_t new_input_map; 
    new_input_map |= check_input(key, action, GLFW_KEY_A) ? ENGINE_INPUT_A : 0;
    new_input_map |= check_input(key, action, GLFW_KEY_D) ? ENGINE_INPUT_D : 0; 
    new_input_map |= check_input(key, action, GLFW_KEY_W) ? ENGINE_INPUT_W : 0; 
    new_input_map |= check_input(key, action, GLFW_KEY_S) ? ENGINE_INPUT_S : 0; 
    new_input_map |= check_input(key, action, GLFW_KEY_ESCAPE) ? ENGINE_INPUT_ESC : 0; 

    engine->input_map = new_input_map;
}

bx::Vec3 eye = {0.0f, 0.0f, -35.0f};
bx::Vec3 at = {0.0f, 0.0f, 1.0f};
bx::Vec3 up = {0.0f, 1.0f, 0.0f}; 

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

    float sensitivity = 0.1; 
    engine->pitch -= dy * sensitivity;
    engine->yaw -= dx * sensitivity; 

    engine->pitch = bx::clamp(engine->pitch, -90.0f, 90.0f);

    bx::Vec3 dir = {
        bx::cos(bx::toRad(engine->pitch)) * bx::cos(bx::toRad(engine->yaw)),
        bx::sin(bx::toRad(engine->pitch)),
        bx::cos(bx::toRad(engine->pitch)) * bx::sin(bx::toRad(engine->yaw))
    }; 

    float view[16];
    bx::mtxLookAt(view, eye, bx::add(eye, dir));

    float proj[16];
    bx::mtxProj(proj, 120.0f, float(engine->width)/float(engine->height), 0.1f, 500.0f, bgfx::getCaps()->homogeneousDepth);

    bgfx::setViewTransform(engine->main_view, view, proj);
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

    this->main_window = glfwCreateWindow(this->width, this->height, this->title.c_str(), NULL, NULL);
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
    glfwSetInputMode(this->main_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

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
        bx::mtxProj(proj, 120.0f, (float) this->width/this->height, 0.1f, 500.0f, bgfx::getCaps()->homogeneousDepth);

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

int Engine::UserLoad(void) {
    EngineObject obj; 
    obj.model = new ModelComponent(); 
    obj.model->LoadModel("models/dragon.obj");


    obj.position.z = -20.0f; 
    obj.position.x = -10.0f; 
    this->objs.push_back(obj); 

    obj.position.x = 10.0f; 
    this->objs.push_back(obj); 

    obj.position.z = 0.0f;
    obj.position.x = -5.0f;

    for (int i = 0; i < 10; i++) {
        this->objs.push_back(obj);
        obj.position.x += 1.0f; 
    }

    return 0;
}

void Engine::ImguiUpdate() {
    ImGui::ShowDemoWindow();

    { 
        ImGui::Begin("Objects window", NULL, ImGuiWindowFlags_MenuBar);
        
        // general properties
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("Debug")) { 
                ImGui::MenuItem("Toggle debug wireframe", NULL, &this->debug_wireframe);
                ImGui::MenuItem("Toggle debug stats", NULL, &this->debug_stats);
                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }

        ImGui::Text("Object tree structure:");

        // object tree structure 
        if (ImGui::BeginTable("3ways", 2, ImGuiTableFlags_Resizable)) {
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_NoHide);
            ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_NoHide);
            ImGui::TableHeadersRow(); 

            size_t n = this->objs.size(); 
            for (size_t i = 0; i < n; i++) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();

                EngineObject *obj = &this->objs[i];
                ImGui::PushID(obj);

                size_t flags = ImGuiTreeNodeFlags_SpanAllColumns | 
                    ImGuiTreeNodeFlags_Leaf | 
                    ImGuiTreeNodeFlags_NoTreePushOnOpen;
                flags |= ImGuiTreeNodeFlags_Selected ? this->obj_selected == obj : 0; 

                if (ImGui::TreeNodeEx(obj->name.c_str(), flags)) {
                    this->obj_selected  = ImGui::IsItemClicked() ? obj : this->obj_selected;  
                }
                ImGui::PopID();

                ImGui::TableNextColumn();
                ImGui::Text("Engine::EngineObject");
            } 

            ImGui::EndTable();
        }


        ImGui::End();
    }

    ImGui::Begin("Object properties window", NULL, ImGuiWindowFlags_MenuBar); 

    char buf1[256] = { 0 }; 
    if (this->obj_selected) { 
        EngineObject *obj = this->obj_selected; 

        memset(buf1, 0x00, sizeof(buf1));
        memcpy(buf1, obj->name.c_str(), obj->name.length()); // assuming (length < sizeof(buff))
        if (ImGui::InputText("Object Name", buf1, sizeof(buf1))) {
            obj->name = std::string(buf1);
        }

        ImGui::Text("Position: "); 
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
            ImGui::DragFloat("Rotation x", &obj->rotation.x, 0.01f, -30.0f, 30.0f);

            ImGui::PushItemWidth(-100);
            ImGui::DragFloat("Rotation y", &obj->rotation.y, 0.01f, -30.0f, 30.0f);

            ImGui::PushItemWidth(-100);
            ImGui::DragFloat("Rotation z", &obj->rotation.z, 0.01f, -30.0f, 30.0f);
        }

        ImGui::SeparatorText("Scale");
        { 
            ImGui::PushItemWidth(-100);
            ImGui::DragFloat("Scale x", &obj->scale.x, 0.01f, -30.0f, 30.0f);

            ImGui::PushItemWidth(-100);
            ImGui::DragFloat("Scale y", &obj->scale.y, 0.01f, -30.0f, 30.0f);

            ImGui::PushItemWidth(-100);
            ImGui::DragFloat("Scale z", &obj->scale.z, 0.01f, -30.0f, 30.0f);
        }
    } 
    else {
        ImGui::Text("Object properties...");
    } 

    ImGui::End(); 
}

void Engine::UserUpdate(void) {
    bgfx::setDebug(
        (this->debug_wireframe ? BGFX_DEBUG_WIREFRAME : 0) | 
        (this->debug_stats ? BGFX_DEBUG_STATS : 0)
    );

    if (this->input_map & ENGINE_INPUT_ESC) {
        this->focused = ENGINE_INTERMEDIATE_GPU;  
        glfwSetInputMode(this->main_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    this->ImguiUpdate();

    float pos_mtx[16]; 
    float rot_mtx[16]; 
    float scl_mtx[16]; 
    float test[16];
    float mtx[16]; 
    for (int i = 0; i < this->objs.size(); i++) { 
        EngineObject *obj = &this->objs[i]; 

        bx::mtxTranslate(pos_mtx, obj->position.x, obj->position.y, obj->position.z);
        bx::mtxRotateXYZ(rot_mtx, obj->rotation.x, obj->rotation.y, obj->rotation.z);
        bx::mtxScale(scl_mtx, obj->scale.x, obj->scale.y, obj->scale.z);
        bx::mtxMul(test, scl_mtx, pos_mtx);
        bx::mtxMul(mtx, rot_mtx, test); 
        bgfx::setTransform(mtx);

        // render
        bgfx::setState(obj->model->render_state);
        bgfx::setVertexBuffer(0, obj->model->vbh);
        bgfx::setIndexBuffer(obj->model->ibh);
        bgfx::setUniform(this->u_position, &obj->position);
        bgfx::setUniform(this->u_rotation, &obj->rotation);
        bgfx::setUniform(this->u_scale, &obj->scale);
        bgfx::submit(this->main_view, this->program);
    } 
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
