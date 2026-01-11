#ifndef MODEL_HPP
#define MODEL_HPP

#include <vector>
#include <engine/quad.hpp>
#include <glm/glm.hpp>
#include <bgfx/bgfx.h>
#include <assimp/scene.h>

class ModelComponent {
    public:
        ModelComponent();

        int LoadNode(aiScene *scene, aiNode *node);
        int LoadModel(std::string filename);

        std::string filename; 
        std::vector<float> vertices;
        std::vector<unsigned int> indices;

        uint64_t render_state; 

        bgfx::VertexLayout layout;
        bgfx::TextureHandle texture;
        bgfx::VertexBufferHandle vbh;
        bgfx::IndexBufferHandle ibh;
};

#endif
