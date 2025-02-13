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
    // インスタンスの初期化

    vk::ApplicationInfo appInfo{};
    appInfo.pApplicationName = "Vulkan Application";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    auto requiredLayers = { "VK_LAYER_KHRONOS_validation" };
    uint32_t instanceExtensionCount = 0;
    const char** requiredExtensions = glfwGetRequiredInstanceExtensions(&instanceExtensionCount);
    vk::InstanceCreateInfo instCreateInfo(
        {},
        &appInfo,
        requiredLayers.size(),
        requiredLayers.begin() ,
        instanceExtensionCount,
        requiredExtensions
    );

    instance = vk::createInstanceUnique(instCreateInfo);
    // 物理デバイスの初期化
    deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
    deviceFeatures.geometryShader = VK_TRUE;

    // 物理デバイスの選択
    physicalDevice = pickPhysicalDevice(deviceExtensions, deviceFeatures);

    // サーフェスの作成
    createSurface();
    
    // デバイスの初期化
    deviceWrapper = DeviceWrapper{*this};
    deviceWrapper.initDevice();
}

//物理デバイスの選択
vk::PhysicalDevice VulkanContext::pickPhysicalDevice(const std::vector<const char*>& deviceExtensions, vk::PhysicalDeviceFeatures deviceFeatures) {
    for (const auto& device : instance->enumeratePhysicalDevices()) {
        if(checkDeviceExtensionSupport(device, deviceExtensions) && checkDeviceFeatures(device, deviceFeatures)) {
            return device;
        }
    }
    throw std::runtime_error("適切な物理デバイスが見つかりませんでした");
}

//物理デバイスのextensionをチェック
bool VulkanContext::checkDeviceExtensionSupport(vk::PhysicalDevice device,  const std::vector<const char*>& deviceExtensions) {
    std::set<std::string> requiredExtensions{deviceExtensions.begin(),
                                             deviceExtensions.end()};
    for (const auto& extension : device.enumerateDeviceExtensionProperties()) {
        requiredExtensions.erase(extension.extensionName);
    }
    return requiredExtensions.empty();
}

//物理デバイスのfeaturesをチェック
bool VulkanContext::checkDeviceFeatures(vk::PhysicalDevice device, vk::PhysicalDeviceFeatures requiredFeatures) {
    vk::PhysicalDeviceFeatures deviceFeatures = device.getFeatures();
    if(!deviceFeatures.robustBufferAccess && requiredFeatures.robustBufferAccess) {
        return false;
    }
    else if(!deviceFeatures.geometryShader && requiredFeatures.geometryShader) {
        return false;
    }
    return true;
}

//サーフェスの作成
void VulkanContext::createSurface() {
    auto result = glfwCreateWindowSurface(instance.get(), window, nullptr, &c_surface);
    if (result != VK_SUCCESS) {
        const char* err;
        glfwGetError(&err);
        std::cout << err << std::endl;
        glfwTerminate();
        throw std::runtime_error("サーフェスの作成に失敗しました");
    }
    surface = vk::UniqueSurfaceKHR{c_surface, instance.get()};
}