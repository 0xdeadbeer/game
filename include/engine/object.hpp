#ifndef OBJECT_HPP
#define OBJECT_HPP

#include <vector>
#include <engine/quad.hpp>
#include <glm/glm.hpp>
#include <bgfx/bgfx.h>
#include <assimp/scene.h>
#include <engine/graph.hpp> 
#include <engine/model.hpp>

class EngineObject {
    public:
        EngineObject() : 
            name("Object"),
            position(0.0f), 
            rotation(0.0f), 
            scale(1.0f), 
            graph(NULL), 
            model(NULL) {}

        std::string name; 
        glm::vec4 position;
        glm::vec4 rotation;
        glm::vec4 scale;

        GraphComponent *graph; 
        ModelComponent *model;

};

#endif
