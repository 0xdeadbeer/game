#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <iostream>
#include <bx/file.h>
#include <bx/bx.h>
#include <bgfx/bgfx.h>
#include <GLFW/glfw3.h>
#include <engine/object.hpp>
#include <memory>

#define CAMERA_WIDTH 50.0f
#define CAMERA_NEAR 0.01f
#define CAMERA_FAR 100.0f

#define ENGINE_INPUT_A              (uint32_t) 1<<0
#define ENGINE_INPUT_D              (uint32_t) 1<<1
#define ENGINE_INPUT_W              (uint32_t) 1<<2
#define ENGINE_INPUT_S              (uint32_t) 1<<3
#define ENGINE_INPUT_ESC            (uint32_t) 1<<15
#define INPUT_ZOOM_OUT              (uint32_t) 1<<4
#define INPUT_ZOOM_IN               (uint32_t) 1<<5

enum focused_window {
    ENGINE_VIEWPORT = 0,  
    ENGINE_INTERMEDIATE_GPU  
};

#define ENGINE_DEBUG_STATS          (uint32_t) 1<<0
#define ENGINE_DEBUG_WIREFRAME      (uint32_t) 1<<1 

#define toggle_bit(a,b) (a & b) ? (a & ~(b)) : (a | b)
#define check_input(a,b,c) (a == c) && ((b & GLFW_REPEAT) || (b & GLFW_PRESS))

class Engine {
    public:
        Engine(void);
        int Init(int mode);
        void Update(void);
        void Shutdown(void); 
        int UserLoad(void);
        void UserUpdate(void);
        void ImguiUpdate();
        void Reset(void);
    
        GLFWwindow* main_window;
        int main_view;

        int keyboard_slots[GLFW_KEY_LAST];
        int cursor_slots[GLFW_MOUSE_BUTTON_LAST+1];
        float pitch;
        float yaw;

        static void error_callback(int error, const char *s);
        static void keyboard_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
        static void cursor_callback(GLFWwindow *window, double x, double y);
        static void cursor_button_callback(GLFWwindow *window, int button, int action, int mods);
        static void window_size_callback(GLFWwindow *window, int width, int height);
        static void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
        static void char_callback(GLFWwindow *window, unsigned int codepoint);

        uint32_t input_map;

    private: 
        int width; 
        int height; 
        std::string title; 

        bgfx::ProgramHandle program;
        bgfx::UniformHandle u_position; 
        bgfx::UniformHandle u_rotation; 
        bgfx::UniformHandle u_scale; 

        std::vector<EngineObject> objs; 
        EngineObject *obj_selected; 

        float last_time; 
        float dt; 
        float time; 

        uint32_t focused; 

        bool debug_wireframe; 
        bool debug_stats; 
};

#endif 
