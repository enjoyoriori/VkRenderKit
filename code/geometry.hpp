#pragma once
#include "header.hpp"

namespace geometry{

struct StaticVertexAttributes {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec4 tangent;
    
    //可変要素
    glm::vec2 texCoord;
    glm::vec4 color;
    glm::uvec4 joint;
    glm::vec4 weight;

    static vk::VertexInputBindingDescription getBindingDescription() {
        return vk::VertexInputBindingDescription(0, sizeof(StaticVertexAttributes), vk::VertexInputRate::eVertex);
    }

    static std::vector<vk::VertexInputAttributeDescription> getAttributeDescriptions() {
        return {
            vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(StaticVertexAttributes, position)),
            vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32Sfloat, offsetof(StaticVertexAttributes, normal)),
            vk::VertexInputAttributeDescription(2, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(StaticVertexAttributes, tangent)),
            vk::VertexInputAttributeDescription(3, 0, vk::Format::eR32G32Sfloat, offsetof(StaticVertexAttributes, texCoord)),
            vk::VertexInputAttributeDescription(4, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(StaticVertexAttributes, color)),
            vk::VertexInputAttributeDescription(5, 0, vk::Format::eR32G32B32A32Uint, offsetof(StaticVertexAttributes, joint)),
            vk::VertexInputAttributeDescription(6, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(StaticVertexAttributes, weight))
        };
    }
};

struct DynamicVertexAttributes {
    glm::vec3 position;

    static vk::VertexInputBindingDescription getBindingDescription() {
        return vk::VertexInputBindingDescription(1, sizeof(DynamicVertexAttributes), vk::VertexInputRate::eVertex);
    }

    static std::vector<vk::VertexInputAttributeDescription> getAttributeDescriptions() {
        return {
            vk::VertexInputAttributeDescription(7, 0, vk::Format::eR32G32B32Sfloat, offsetof(DynamicVertexAttributes, position))
        };
    }
};

struct Material {
    glm::vec4 baseColorFactor;
    float metallicFactor;
    float roughnessFactor;

    vk::UniqueImage baseColorTexture;
    vk::UniqueImageView baseColorTextureView;
    vk::UniqueSampler baseColorTextureSampler;

    vk::UniqueImage metallicRoughnessTexture;
    vk::UniqueImageView metallicRoughnessTextureView;
    vk::UniqueSampler metallicRoughnessTextureSampler;

    vk::UniqueImage normalTexture;
    vk::UniqueImageView normalTextureView;
    vk::UniqueSampler normalTextureSampler;

    vk::UniqueImage occlusionTexture;
    vk::UniqueImageView occlusionTextureView;
    vk::UniqueSampler occlusionTextureSampler;

    vk::UniqueImage emissiveTexture;
    vk::UniqueImageView emissiveTextureView;
    vk::UniqueSampler emissiveTextureSampler;
};

struct Transform {
    glm::vec3 translation;
    glm::quat rotation;
    glm::vec3 scale;
};

struct Scene {
    std::string name;
    std::vector<uint32_t> rootNodeIndices;
};

struct Node {
    uint32_t nodeIndex;
    std::vector<int32_t> parents;
    std::vector<uint32_t> children;
    uint32_t meshIndex;
    
    Transform transform = {
        glm::vec3(0.0f),
        glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
        glm::vec3(1.0f)
    };

    glm::mat4 localMatrix = glm::mat4(1.0f);
    glm::mat4 globalMatrix = glm::mat4(1.0f);
};

struct Primitive {
    uint32_t firstIndex;
    uint32_t indexCount;
    uint32_t vertexOffset;//初期化はバッファを読み込むときに行う
    uint32_t materialIndex;
    vk::PrimitiveTopology topology;

    // アトリビュート存在フラグ
    struct {
        bool hasNormals : 1;
        bool hasTangents : 1;
        bool hasTexCoords : 1;
        bool hasColors : 1;
        bool hasJoints : 1;
        bool hasWeights : 1;
    } attributes;

    // 透過フラグ（レンダリング順序決定用）
    bool isTransparent;
};

struct Mesh {
    std::vector<int32_t> nodeIndex;
    uint32_t meshIndex;
    std::vector<Primitive> primitives;
};

struct Model {
    std::vector<Scene> scenes;
    std::vector<Node> nodes;
    std::vector<Mesh> meshes;
    std::vector<Material> materials;

    std::unordered_map<uint32_t, uint32_t> gltfToNode; //key: gltf node index, value: node index
    std::unordered_map<uint32_t, uint32_t> gltfToMesh; //key: gltf mesh index, value: mesh index
    std::unordered_map<uint32_t, uint32_t> gltfToMaterial; //key: gltf material index, value: material index
        
    void readGLTF(std::string filename);
    uint32_t readNode(tinygltf::Model &model, uint32_t gltfNodeIndex, int32_t parentIndex);
    void readMesh(tinygltf::Model& model, uint32_t gltfMeshIndex);
    Primitive readPrimitive(tinygltf::Model& model, tinygltf::Primitive& primitive);
    void readMaterial(tinygltf::Model& model, tinygltf::Material& material);

    void checkGLTF();
    void checkNode(uint32_t nodeIndex);
    void checkMesh(uint32_t meshIndex);

};


}


void dumpGLTF(tinygltf::Model& model);
void dumpNode(tinygltf::Model& model, tinygltf::Node& node, uint32_t nodeIndex, uint32_t depth);
void dumpMesh(tinygltf::Model &model, tinygltf::Mesh &mesh, uint32_t depth);
void dumpPrimitive(tinygltf::Model &model, tinygltf::Primitive &primitive, uint32_t depth);