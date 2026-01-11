#include <common.hpp>
#include <engine.hpp>
#include <engine/renderer.hpp>
#include <bx/file.h>
#include <bx/math.h>
#include <imgui.h>
#include <engine/imgui.hpp>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_X11
#include <GLFW/glfw3native.h>

int Renderer::Start(void *window, void *e, void *ui) {
    this->e = (EngineApi *) e; 
    this->ui = (UIApi *) ui; 
    bgfx::Init init;
    init.platformData.ndt = glfwGetX11Display();
    init.platformData.nwh = (void*) (uintptr_t) glfwGetX11Window((GLFWwindow *) window);

    glfwGetWindowSize((GLFWwindow *) window, &camera_width, &camera_height);
    init.resolution.width = camera_width; 
    init.resolution.height = camera_height; 
    init.resolution.reset = BGFX_RESET_VSYNC;
    if (!bgfx::init(init)) {
        return -1; 
    }

    bgfx::setViewRect(main_view, 0, 0, camera_width, camera_height);
    bgfx::setViewClear(main_view, 
            BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 
            0x00000000,
            1.0f,
            0);

    u_flags = bgfx::createUniform("u_flags", bgfx::UniformType::Vec4);
    u_position = bgfx::createUniform("u_position", bgfx::UniformType::Vec4);
    u_rotation = bgfx::createUniform("u_rotation", bgfx::UniformType::Vec4);
    u_scale = bgfx::createUniform("u_scale", bgfx::UniformType::Vec4);
    normal_program = load_program(NORMAL_VERTEX, NORMAL_FRAGMENT);
    selected_program = load_program(NORMAL_VERTEX, SELECTED_FRAGMENT);

    return 0; 
}

void Renderer::NewUpdate(float dt) {
    bgfx::touch(this->main_view);
}

void Renderer::Update(float dt) {
    bgfx::setDebug(
        (this->debug_wireframe ? BGFX_DEBUG_WIREFRAME : 0) | 
        (this->debug_stats ? BGFX_DEBUG_STATS : 0)
    );

    float pos_mtx[16]; 
    float rot_mtx[16]; 
    float scl_mtx[16]; 
    float test[16];
    float mtx[16]; 
    for (int i = 0; i < e->objs.size(); i++) { 
        EngineObject *obj = &e->objs[i]; 
        if (!obj->model->indices.size() || !obj->model->vertices.size()) {
            continue; 
        }

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

        glm::vec4 flags;

        flags.x = glfwGetTime();
        flags.y = 0.0f; 
        flags.z = 0.0f; 
        flags.w = 0.0f; 

        bgfx::setUniform(this->u_flags, &flags);
        bgfx::setUniform(this->u_position, &obj->position);
        bgfx::setUniform(this->u_rotation, &obj->rotation);
        bgfx::setUniform(this->u_scale, &obj->scale);
        bgfx::submit(this->main_view, obj == e->obj_selected ? this->selected_program : this->normal_program);
    } 

    bgfx::frame();
    ImGui::Render();
    imgui_render(ImGui::GetDrawData());
}

void Renderer::Camera(void) {
    float view[16];
    bx::mtxLookAt(view, eye, at);

    float proj[16];
    bx::mtxProj(proj, fov, camera_width/camera_height, 0.1f, 500.0f, bgfx::getCaps()->homogeneousDepth);

    bgfx::setViewTransform(this->main_view, view, proj);
}

void Renderer::CameraUpdate(float dx, float dy) {
    pitch -= dy * camera_sensitivity;
    yaw -= dx * camera_sensitivity; 

    pitch = bx::clamp(pitch, -90.0f, 90.0f);

    bx::Vec3 dir = {
        bx::cos(bx::toRad(pitch)) * bx::cos(bx::toRad(yaw)),
        bx::sin(bx::toRad(pitch)),
        bx::cos(bx::toRad(pitch)) * bx::sin(bx::toRad(yaw))
    }; 

    float view[16];
    bx::mtxLookAt(view, eye, bx::add(eye, dir));

    float proj[16];
    bx::mtxProj(proj, fov, camera_width/camera_height, 0.1f, 500.0f, bgfx::getCaps()->homogeneousDepth);

    bgfx::setViewTransform(main_view, view, proj);
}

void Renderer::CameraResolution(int width, int height) {
    camera_width = width; 
    camera_height = height; 
}

void Renderer::CameraReset(void) {
    bgfx::reset(camera_width, camera_height, 0);
    ui->Reset(camera_width, camera_height); 
    bgfx::setViewRect(main_view, 0, 0, camera_width, camera_height);
    bgfx::setViewClear(main_view, 
            BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 
            0x0f0f0f0f,
            1.0f,
            0);
}
