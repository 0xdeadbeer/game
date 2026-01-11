#ifndef APIS_HPP
#define APIS_HPP

#include <vector> 
#include <engine/object.hpp> 

struct EngineApi {
    EngineApi() : obj_selected(NULL) { } 
    std::vector<EngineObject> objs; 
    EngineObject *obj_selected; 
}; 

struct RendererApi {
    RendererApi() : debug_wireframe(false), debug_stats(false), main_view(0), camera_sensitivity(0.1f), camera_width(0), camera_height(0) {}
    virtual int Start(void *window, void *e, void *ui) { return 0; } 
    virtual void NewUpdate(float dt) { } 
    virtual void Update(float dt) { }

    virtual void Camera(void) { }
    virtual void CameraUpdate(float dx, float dy) { }
    virtual void CameraResolution(int width, int height) { }
    virtual void CameraReset() { }

    bool debug_wireframe; 
    bool debug_stats; 

    int main_view; 

    float camera_sensitivity; 
    int camera_width; 
    int camera_height; 
};

struct UIApi {
    UIApi() {}
    virtual int Start(void *e, void *r) { return 0; }
    virtual void NewUpdate(float dt) { }
    virtual void Update(float dt) { }
    virtual void UpdateObjectsWindow(float dt) { }
    virtual void UpdatePropertiesWindow(float dt) { }
    virtual void Reset(int width, int height) { }
};

#endif 
