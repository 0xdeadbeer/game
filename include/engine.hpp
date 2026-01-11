#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <iostream>
#include <memory>
#include <bx/file.h>
#include <bx/bx.h>
#include <bgfx/bgfx.h>
#include <GLFW/glfw3.h>
#include <engine/ui-imgui.hpp>
#include <engine/renderer.hpp>
#include <engine/object.hpp>
#include <apis.hpp> 

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

#define set_bit(a,b,c) (c) ? (a) | (c << b) : (a) & ~(c << b)
#define toggle_bit(a,b) (a & b) ? (a & ~(b)) : (a | b)
#define check_input(a,b,c) (a == c) && ((b & GLFW_REPEAT) || (b & GLFW_PRESS))

class Engine : public EngineApi {
    public:
        Engine(void);
        int Start(int mode);
        int IsOk(void); 
        void Update(void);
        void Shutdown(void); 
        int UserLoad(void);
        void UserUpdate(void);
        void Reset(void);
    
        int keyboard_slots[GLFW_KEY_LAST];
        int cursor_slots[GLFW_MOUSE_BUTTON_LAST+1];

        static void error_callback(int error, const char *s);
        static void keyboard_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
        static void cursor_callback(GLFWwindow *window, double x, double y);
        static void cursor_button_callback(GLFWwindow *window, int button, int action, int mods);
        static void window_size_callback(GLFWwindow *window, int width, int height);
        static void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
        static void char_callback(GLFWwindow *window, unsigned int codepoint);


    private: 
        Renderer renderer; 
        UI ui; 

        std::string title; 
        GLFWwindow* main_window;

        float last_time; 
        float dt; 
        float time; 

        uint32_t focused; 
        uint32_t input_map;
};

#endif 
