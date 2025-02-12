#include "vulkanContext.hpp"

void VulkanContext::initWindow(uint32_t width, uint32_t height) {
    if (!glfwInit()) {
        throw std::runtime_error("GLFWの初期化に失敗しました");
    }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr);
    if (!window) {
        throw std::runtime_error("ウィンドウの作成に失敗しました");
    }
}

void VulkanContext::initVulkan() {
    
}