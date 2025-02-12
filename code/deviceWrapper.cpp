#include "vulkanContext.hpp"

void VulkanContext::DeviceWrapper::initDevice() {
    // キューの作成
    auto queueCreateInfos = queueWrapper.findQueues();
    // 論理デバイスの初期化
    vk::DeviceCreateInfo deviceCreateInfo(
        {},
        static_cast<uint32_t>(queueCreateInfos.size()),
        queueCreateInfos.data(),
        0,
        nullptr,
        deviceExtensions.size(),
        deviceExtensions.begin(),
        &deviceFeatures
    );

    vk::StructureChain createInfoChain{
        deviceCreateInfo,
        vk::PhysicalDeviceDynamicRenderingFeatures{true}
    };

    device = physicalDevice.createDeviceUnique(createInfoChain.get<vk::DeviceCreateInfo>());

    queueWrapper.initQueues();
}

std::vector<vk::DeviceQueueCreateInfo> VulkanContext::DeviceWrapper::QueueWrapper::findQueues() {
    std::vector<vk::QueueFamilyProperties> queueProps = physicalDevice.getQueueFamilyProperties();
    
    uint32_t graphicsQueueIndex = -1;
    uint32_t graphicsQueueCount = 0;
    uint32_t computeQueueIndex = -1;
    uint32_t computeQueueCount = 0;

    for(uint32_t i = 0; i < queueProps.size(); i++) {//キューを持つ数が最大のものを選択
        if(queueProps[i].queueFlags & vk::QueueFlagBits::eGraphics && physicalDevice.getSurfaceSupportKHR(i, surface.get())) {
            graphicsQueueCount = std::max(graphicsQueueCount, queueProps[i].queueCount);
            if(graphicsQueueCount == queueProps[i].queueCount) {
                graphicsQueueIndex = i;
            }
        }
        if(queueProps[i].queueFlags & vk::QueueFlagBits::eCompute) {
            computeQueueCount = std::max(computeQueueCount, queueProps[i].queueCount);
            if(computeQueueCount == queueProps[i].queueCount) {
                computeQueueIndex = i;
            }
        }
    }

    std::cout << "Graphics Queue Index: " << graphicsQueueCount << std::endl;

    if(graphicsQueueIndex >= 0) {
        for(uint32_t i = 0; i < graphicsQueueCount; i++) {
            float priority = static_cast<float>(i) / static_cast<float>(graphicsQueueCount);
            graphicQueuePriorities.push_back(priority);
            std::cout << "Graphics Queue Priority: " << priority << std::endl;
        }
        queueCreateInfos.at(0) = vk::DeviceQueueCreateInfo({}, 
                                                           graphicsQueueIndex, 
                                                           graphicsQueueCount, 
                                                           graphicQueuePriorities.data());
    }
    else{
        throw std::runtime_error("適切なキューが見つかりませんでした");
    }
    if(computeQueueIndex >= 0) {
        for(uint32_t i=0; i < computeQueueCount; i++) {
            float priority = static_cast<float>(i) / static_cast<float>(computeQueueCount);
            computeQueuePriorities.push_back(priority);
            std::cout << "Compute Queue Priority: " << priority << std::endl;
        }
        queueCreateInfos.at(1) = vk::DeviceQueueCreateInfo({}, 
                                                           computeQueueIndex, 
                                                           computeQueueCount, 
                                                           computeQueuePriorities.data());
    }
    else{
        throw std::runtime_error("適切なキューが見つかりませんでした");
    }
    if(queueCreateInfos.at(1).queueFamilyIndex == queueCreateInfos.at(0).queueFamilyIndex) {
        queueCreateInfos.pop_back();
    }
    return queueCreateInfos;
}    
    
void VulkanContext::DeviceWrapper::QueueWrapper::initQueues() {

    for(uint32_t i = 0; i < queueCreateInfos.[QueueType.Graphics].queueCount; i++) {
        graphicsQueue.push_back(device.getQueue(queueCreateInfos[QueueType.Graphics].queueFamilyIndex, i));
    }
    for(uint32_t i = 0; i < queueCreateInfos[QueueType.Compute].queueCount; i++) {
        computeQueue.push_back(device.getQueue(queueCreateInfos[QueueType.Compute].queueFamilyIndex, i));
    }

}