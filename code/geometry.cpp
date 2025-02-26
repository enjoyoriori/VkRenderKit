#include "geometry.hpp"
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tiny_gltf.h>

void readGLTF(std::string filename){
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err, warn;

    std::string extension = filename.substr(filename.find_last_of(".") + 1);
    if (extension != "gltf" && extension != "glb") {
        throw std::runtime_error("GLTFファイルではありません");
    }

    bool ret = extension == "gltf" ? loader.LoadASCIIFromFile(&model, &err, &warn, filename) : loader.LoadBinaryFromFile(&model, &err, &warn, filename);

    if (!warn.empty()) {
        std::cout << "Warn: " << warn << std::endl;
    }
    if (!err.empty()) {
        std::cout << "Err: " << err << std::endl;
    }
    if (!ret) {
        throw std::runtime_error("GLTFファイルの読み込みに失敗しました");
    }

    for(size_t i = 0; i < model.scenes.size(); i++) {
        for(size_t j = 0; j < model.scenes[i].nodes.size(); j++) {
            readNode(model, model.nodes[model.scenes[i].nodes[j]]);
        }        
    }
    dumpGLTF(model);
}

void readNode(tinygltf::Model &model, tinygltf::Node &node){
    if (node.mesh != -1) {
        readMesh(model, model.meshes[node.mesh]);
    }
    for(size_t i = 0; i < node.children.size(); i++) {
        readNode(model, model.nodes[node.children[i]]);
    }
}

void readMesh(tinygltf::Model &model, tinygltf::Mesh &mesh){
    for(size_t i = 0; i < mesh.primitives.size(); i++) {
        readPrimitive(model, mesh.primitives[i]);
    }
}

void readPrimitive(tinygltf::Model &model, tinygltf::Primitive &primitive){
    if (primitive.indices != -1) {
        const tinygltf::Accessor &indexAccessor = model.accessors[primitive.indices];
        const tinygltf::BufferView &indexBufferView = model.bufferViews[indexAccessor.bufferView];
        const tinygltf::Buffer &indexBuffer = model.buffers[indexBufferView.buffer];
        const uint8_t *indexBufferPtr = &indexBuffer.data[indexBufferView.byteOffset + indexAccessor.byteOffset];
        const uint32_t indexCount = static_cast<uint32_t>(indexAccessor.count);
        const uint32_t indexStride = indexAccessor.ByteStride(indexBufferView);
        const uint32_t indexByteSize = indexCount * indexStride;
        std::vector<uint32_t> indices(indexCount);
        std::memcpy(indices.data(), indexBufferPtr, indexByteSize);
    }

    for (const auto &attrib : primitive.attributes) {
        const tinygltf::Accessor &accessor = model.accessors[attrib.second];
        const tinygltf::BufferView &bufferView = model.bufferViews[accessor.bufferView];
        const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];
        const uint8_t *bufferPtr = &buffer.data[bufferView.byteOffset + accessor.byteOffset];
        const uint32_t count = static_cast<uint32_t>(accessor.count);
        const uint32_t stride = accessor.ByteStride(bufferView);
        const uint32_t byteSize = count * stride;
        std::vector<uint8_t> data(byteSize);
        std::memcpy(data.data(), bufferPtr, byteSize);
    }

    if (primitive.material != -1) {
        //loadMaterial(model, model.materials[primitive.material]);
    }
}

void dumpGLTF(tinygltf::Model &model) {
    for(size_t i = 0; i < model.scenes.size(); i++) {
        std::cout << "Scene " << i << std::endl;
        for(size_t j = 0; j < model.scenes[i].nodes.size(); j++) {
            dumpNode(model, model.nodes[model.scenes[i].nodes[j]], j, 1);
        }        
    }    
}

void dumpSpace(uint32_t depth) {
    for(uint32_t i = 0; i < depth; i++) {
        std::cout << "|    ";
    }
}

void dumpNode(tinygltf::Model &model, tinygltf::Node &node, uint32_t nodeIndex, uint32_t depth) {
    dumpSpace(depth);
    std::cout << "Node " << nodeIndex << " : " << node.name << std::endl;
    
    if (node.camera != -1) {
        dumpSpace(depth);
        std::cout << "  Camera: " << node.camera << std::endl;
    }
    if (!node.translation.empty()) {
        dumpSpace(depth);
        std::cout << "  Translation: ";
        for (const auto &t : node.translation) {
            std::cout << t << " ";
        }
        std::cout << std::endl;
    }
    if (!node.rotation.empty()) {
        dumpSpace(depth);
        std::cout << "  Rotation: ";
        for (const auto &r : node.rotation) {
            std::cout << r << " ";
        }
        std::cout << std::endl;
    }
    if (!node.scale.empty()) {
        dumpSpace(depth);
        std::cout << "  Scale: ";
        for (const auto &s : node.scale) {
            std::cout << s << " ";
        }
        std::cout << std::endl;
    }
    if (!node.matrix.empty()) {
        dumpSpace(depth);
        std::cout << "  Matrix: ";
        for (const auto &m : node.matrix) {
            std::cout << m << " ";
        }
        std::cout << std::endl;
    }
    if (node.mesh != -1) {
        dumpSpace(depth);
        std::cout << "  Mesh: " << node.mesh << std::endl;
        dumpMesh(model, model.meshes[node.mesh], depth + 1);
    }
    for(size_t i = 0; i < node.children.size(); i++) {
        uint32_t nodeIndex = node.children[i];
        dumpNode(model, model.nodes[node.children[i]], nodeIndex, depth + 1);
    }
}

void dumpMesh(tinygltf::Model &model, tinygltf::Mesh &mesh, uint32_t depth) {
    dumpSpace(depth);
    std::cout << "Mesh: " << mesh.name << std::endl;

    for(size_t i = 0; i < mesh.primitives.size(); i++) {
        dumpPrimitive(model, mesh.primitives[i], depth + 1);
    }
}

void dumpPrimitive(tinygltf::Model &model, tinygltf::Primitive &primitive, uint32_t depth) {
    dumpSpace(depth);
    std::cout << "Primitive: " << std::endl;

    for (const auto &attrib : primitive.attributes) {
        dumpSpace(depth + 1);
        std::cout << "Attribute: " << attrib.first << " (Accessor ID: " << attrib.second << ")" << std::endl;
    }

    if (primitive.indices != -1) {
        dumpSpace(depth + 1);
        std::cout << "Indices (Accessor ID): " << primitive.indices << std::endl;
    }

    dumpSpace(depth + 1);
    std::cout << "Material: " << primitive.material << std::endl;

    dumpSpace(depth + 1);
    std::cout << "Mode: " << primitive.mode << std::endl;
}

