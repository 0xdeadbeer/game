#ifndef UI_IMGUI_HPP 
#define UI_IMGUI_HPP 
#include <engine/renderer.hpp>

class UI : public UIApi {
    public: 
        int Start(void *e, void *r) override; 
        void NewUpdate(float dt) override; 
        void Update(float dt) override; 
        void UpdateObjectsWindow(float dt) override; 
        void UpdatePropertiesWindow(float dt) override; 
        void Reset(int width, int height) override; 
    private: 
        EngineApi *e; 
        RendererApi *r;
};

#endif 
