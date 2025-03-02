#include "geometry.hpp"
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tiny_gltf.h>

namespace geometry
{
    
void Model::readGLTF(std::string filename){
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err, warn;
    std::vector<Scene> scenes;

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
        Scene scene;
        scene.name = model.scenes[i].name;
        for(size_t j = 0; j < model.scenes[i].nodes.size(); j++) {
            uint32_t rootIndex = readNode(model, model.scenes[i].nodes[j], -1);
            scene.rootNodeIndices.push_back(rootIndex);
        }
        scenes.push_back(scene);
    }
    dumpGLTF(model);
}

uint32_t Model::readNode(tinygltf::Model &model, uint32_t gltfNodeIndex, int32_t parentIndex) {
    //すでに読み込まれているノードの場合はインデックスを返す
    if (gltfToNode.find(gltfNodeIndex) != gltfToNode.end()) {
        if(parentIndex != -1) {
            nodes[gltfToNode[gltfNodeIndex]].parents.push_back(parentIndex);
        }

        return gltfToNode[gltfNodeIndex];
    }
    
    //ノード作成とプロパティ設定
    tinygltf::Node node = model.nodes[gltfNodeIndex];
    Node newNode;
    newNode.parents.push_back(parentIndex);
    newNode.meshIndex = node.mesh;
    
    //トランスフォーム設定 - GLTFの仕様に従う
    if (!node.matrix.empty()) {
        // 行列が明示的に指定されている場合
        newNode.localMatrix = glm::make_mat4(node.matrix.data());
        
        // オプション: 行列からTRSを抽出
        glm::vec3 skew;
        glm::vec4 perspective;
        glm::decompose(newNode.localMatrix, 
                    newNode.transform.scale,
                    newNode.transform.rotation, 
                    newNode.transform.translation, 
                    skew, perspective);
    } else {
        // TRSが指定されている場合
        if (!node.translation.empty()) {
            newNode.transform.translation = glm::make_vec3(node.translation.data());
        }
        if (!node.rotation.empty()) {
            newNode.transform.rotation = glm::make_quat(node.rotation.data());
        }
        if (!node.scale.empty()) {
            newNode.transform.scale = glm::make_vec3(node.scale.data());
        }
        
        // TRSから行列を計算
        glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), newNode.transform.translation);
        glm::mat4 rotationMatrix = glm::mat4_cast(newNode.transform.rotation);
        glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), newNode.transform.scale);
        
        // T * R * S の順に適用
        newNode.localMatrix = translationMatrix * rotationMatrix * scaleMatrix;
    }
    
    //メッシュの読み込み
    if (node.mesh != -1) {
        readMesh(model, node.mesh);
    }

    //ノードを追加し、インデックスを保存
    uint32_t currentIndex = static_cast<uint32_t>(nodes.size());
    nodes.push_back(newNode);
    nodes[currentIndex].nodeIndex = currentIndex;
    gltfToNode[gltfNodeIndex] = currentIndex;
    
    //子ノードを処理
    for(size_t i = 0; i < node.children.size(); i++) {
        uint32_t childIndex = readNode(model, node.children[i], currentIndex);
        nodes[currentIndex].children.push_back(childIndex);
    }
    
    return currentIndex;
}

void Model::readMesh(tinygltf::Model &model, uint32_t gltfMeshIndex){

    if(gltfToMesh.find(gltfMeshIndex) != gltfToMesh.end()) {
        return;
    }

    tinygltf::Mesh mesh = model.meshes[gltfMeshIndex];
    Mesh newMesh;
    meshes.push_back(newMesh);
    meshes[meshes.size() - 1].meshIndex = meshes.size() - 1;
    gltfToMesh[gltfMeshIndex] = meshes.size() - 1;
    for(size_t i = 0; i < mesh.primitives.size(); i++) {
        meshes[meshes.size() - 1].primitives.push_back(readPrimitive(model, mesh.primitives[i]));
    }
}

Primitive Model::readPrimitive(tinygltf::Model &model, tinygltf::Primitive &primitive){
    Primitive newPrimitive;
    
    // インデックスバッファ情報の抽出
    if (primitive.indices >= 0) {
        const tinygltf::Accessor &indexAccessor = model.accessors[primitive.indices];
        newPrimitive.indexCount = static_cast<uint32_t>(indexAccessor.count);
        // firstIndexはバッファ読み込み時に設定
    } else {
        // インデックスバッファが無い場合は頂点数をカウント
        // 頂点数は最初のアトリビュートから取得（通常はPOSITION）
        if (!primitive.attributes.empty()) {
            auto it = primitive.attributes.find("POSITION");
            if (it != primitive.attributes.end()) {
                const tinygltf::Accessor &posAccessor = model.accessors[it->second];
                newPrimitive.indexCount = static_cast<uint32_t>(posAccessor.count);
            }
        }
    }

    newPrimitive.attributes = {};
    for (const auto &attrib : primitive.attributes) {
        if (attrib.first == "NORMAL") {
            newPrimitive.attributes.hasNormals = true;
        } else if (attrib.first == "TANGENT") {
            newPrimitive.attributes.hasTangents = true;
        } else if (attrib.first.rfind("TEXCOORD", 0) == 0) {
            newPrimitive.attributes.hasTexCoords = true;
        } else if (attrib.first.rfind("COLOR", 0) == 0) {
            newPrimitive.attributes.hasColors = true;
        } else if (attrib.first.rfind("JOINTS", 0) == 0) {
            newPrimitive.attributes.hasJoints = true;
        } else if (attrib.first.rfind("WEIGHTS", 0) == 0) {
            newPrimitive.attributes.hasWeights = true;
        }
    }

    

    // トポロジーの設定
    switch (primitive.mode) {
        case TINYGLTF_MODE_POINTS:
            newPrimitive.topology = vk::PrimitiveTopology::ePointList;
            break;
        case TINYGLTF_MODE_LINE:
            newPrimitive.topology = vk::PrimitiveTopology::eLineList;
            break;
        case TINYGLTF_MODE_LINE_LOOP:
        case TINYGLTF_MODE_LINE_STRIP:
            newPrimitive.topology = vk::PrimitiveTopology::eLineStrip;
            break;
        case TINYGLTF_MODE_TRIANGLE_STRIP:
            newPrimitive.topology = vk::PrimitiveTopology::eTriangleStrip;
            break;
        case TINYGLTF_MODE_TRIANGLE_FAN:
            newPrimitive.topology = vk::PrimitiveTopology::eTriangleFan;
            break;
        case TINYGLTF_MODE_TRIANGLES:
        default:
            newPrimitive.topology = vk::PrimitiveTopology::eTriangleList;
            break;
    }

    // マテリアルインデックス
    newPrimitive.materialIndex = primitive.material >= 0 ? 
                                static_cast<uint32_t>(primitive.material) : 
                                UINT32_MAX;  // 無効値

    if (primitive.material != -1) {
        //loadMaterial(model, model.materials[primitive.material]);
    }

    // 透明度フラグの設定（マテリアル情報から決定）
    newPrimitive.isTransparent = false;
    if (primitive.material >= 0 && primitive.material < model.materials.size()) {
        const auto &material = model.materials[primitive.material];
        // アルファモードとアルファ値をチェック
        if (material.alphaMode == "BLEND") {
            newPrimitive.isTransparent = true;
        } else if (material.alphaMode == "MASK" && material.alphaCutoff < 1.0f) {
            newPrimitive.isTransparent = true;
        } else if (!material.pbrMetallicRoughness.baseColorFactor.empty() && 
                  material.pbrMetallicRoughness.baseColorFactor.size() >= 4 && 
                  material.pbrMetallicRoughness.baseColorFactor[3] < 1.0f) {
            newPrimitive.isTransparent = true;
        }
    }
    
    return newPrimitive;
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
