#include "vulkanContext.hpp"

void VulkanContext::DeviceWrapper::initDevice() {
    // キュー情報の取得
    graphicsQueueWrapper.findQueues(QueueWrapper::QueueType::Graphics);
    computeQueueWrapper.findQueues(QueueWrapper::QueueType::Compute);
    std::map<uint32_t, vk::DeviceQueueCreateInfo> queueCreateInfos = graphicsQueueWrapper.getQueueCreateInfos();

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
    // キューの初期化
    queueWrapper.initQueues();


}

//なるべく数の多いキューファミリーを選択
void VulkanContext::DeviceWrapper::QueueWrapper::findQueues(QueueType queueType) {
    std::vector<vk::QueueFamilyProperties> queueProps = physicalDevice.getQueueFamilyProperties();
    
    uint32_t graphicsQueueIndex = -1;
    uint32_t graphicsQueueCount = 0;
    uint32_t computeQueueIndex = -1;
    uint32_t computeQueueCount = 0;

    for(uint32_t i = 0; i < queueProps.size(); i++) {//キューを持つ数が最大のものを選択
        if(queueProps[i].queueFlags & vk::QueueFlagBits::eGraphics && physicalDevice.getSurfaceSupportKHR(i, surface.get()) && !usedQueueFamilyIndices.contains(i)) {
            graphicsQueueCount = std::max(graphicsQueueCount, queueProps[i].queueCount);
            if(graphicsQueueCount == queueProps[i].queueCount) {
                graphicsQueueIndex = i;
            }
        }
        if(queueProps[i].queueFlags & vk::QueueFlagBits::eCompute && !usedQueueFamilyIndices.contains(i)) {
            computeQueueCount = std::max(computeQueueCount, queueProps[i].queueCount);
            if(computeQueueCount == queueProps[i].queueCount) {
                computeQueueIndex = i;
            }
        }
    }

    switch (queueType) {
        case QueueType::Graphics:
            if(graphicsQueueIndex == -1) {
                throw std::runtime_error("グラフィックスキューが見つかりませんでした");
            }
            queueFamilyIndex = graphicsQueueIndex;
            for(uint32_t i = 0; i < graphicsQueueCount; i++) {
                queuePriorities.push_back(i / graphicsQueueCount);
            }
            vk::DeviceQueueCreateInfo graphicsQueueCreateInfo(
                {},
                graphicsQueueIndex,
                graphicsQueueCount,
                queuePriorities.data()
            );
            queueCreateInfos[graphicsQueueIndex] = graphicsQueueCreateInfo;
            usedQueueFamilyIndices.push_back(graphicsQueueIndex);
            break;
        
        case QueueType::Compute:
            if(computeQueueIndex == -1) {
                throw std::runtime_error("コンピュートキューが見つかりませんでした");
            }
            queueFamilyIndex = computeQueueIndex;
            for(uint32_t i = 0; i < computeQueueCount; i++) {
                queuePriorities.push_back(i / computeQueueCount);
            }
            vk::DeviceQueueCreateInfo computeQueueCreateInfo(
                {},
                computeQueueIndex,
                computeQueueCount,
                queuePriorities.data()
            );
            queueCreateInfos[computeQueueIndex] = computeQueueCreateInfo;
            usedQueueFamilyIndices.push_back(computeQueueIndex);
            break;
        
        default:
            throw std::runtime_error("不正なキュータイプです");
            break;
        }
}    
    
void VulkanContext::DeviceWrapper::QueueWrapper::initQueues() {

    for(uint32_t i = 0; i < queueCreateInfos.at(queueFamilyIndex).queueCount; i++) {
        queues.push_back(device->getQueue(queueFamilyIndex, i));
    }

}