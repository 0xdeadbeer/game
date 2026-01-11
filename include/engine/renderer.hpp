#ifndef RENDERER_HPP 
#define RENDERER_HPP 
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_X11
#include <GLFW/glfw3native.h>
#include <apis.hpp> 
#include <bx/math.h> 

class Renderer : public RendererApi {
    public: 
        Renderer() : fov(120.0f),eye(0.0f, 0.0f, -35.0f), at(0.0f, 0.0f, 1.0f), up(0.0f, 1.0f, 0.0f), RendererApi() { }
        int Start(void *window, void *e, void *ui) override; 
        void NewUpdate(float dt) override; 
        void Update(float dt) override; 

        void Camera(void) override; 
        void CameraUpdate(float dx, float dy) override; 
        void CameraResolution(int width, int height) override; 
        void CameraReset() override;
    private: 
        EngineApi *e; 
        UIApi *ui; 

        float fov;
        bx::Vec3 eye;
        bx::Vec3 at;
        bx::Vec3 up;

        bgfx::ProgramHandle normal_program;
        bgfx::ProgramHandle selected_program; 
        bgfx::UniformHandle u_flags; 
        bgfx::UniformHandle u_position; 
        bgfx::UniformHandle u_rotation; 
        bgfx::UniformHandle u_scale; 

        float pitch; 
        float yaw; 

        float camera_sensitivity; 
        int camera_width; 
        int camera_height; 
}; 

#endif 
