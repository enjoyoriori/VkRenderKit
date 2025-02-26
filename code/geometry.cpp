#include "geometry.hpp"
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tiny_gltf.h>

void loadGLTF(std::string filename){
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

    
}