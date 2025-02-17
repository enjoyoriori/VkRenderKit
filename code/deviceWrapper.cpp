#include "vulkanContext.hpp"

// staticメンバ変数の定義
std::vector<uint32_t> VulkanContext::DeviceWrapper::QueueWrapper::usedQueueFamilyIndices;
std::map<uint32_t, vk::DeviceQueueCreateInfo> VulkanContext::DeviceWrapper::QueueWrapper::queueCreateInfos;

void VulkanContext::DeviceWrapper::initDevice() {
    // キュー情報の取得
    graphicsQueueWrapper.findQueues(QueueWrapper::QueueType::Graphics);
    computeQueueWrapper.findQueues(QueueWrapper::QueueType::Compute);

    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    for(auto& [key, value] : graphicsQueueWrapper.getQueueCreateInfos()) {
        queueCreateInfos.push_back(value);
    }

    // 論理デバイスの初期化
    vk::DeviceCreateInfo deviceCreateInfo(
        {},
        static_cast<uint32_t>(queueCreateInfos.size()),
        queueCreateInfos.data(),
        0,
        nullptr,
        context.deviceExtensions.size(),
        context.deviceExtensions.data(),
        &context.deviceFeatures
    );

    vk::StructureChain createInfoChain{
        deviceCreateInfo,
        vk::PhysicalDeviceDynamicRenderingFeatures{true}
    };

    device = context.physicalDevice.createDeviceUnique(createInfoChain.get<vk::DeviceCreateInfo>());
    // キューの初期化
    graphicsQueueWrapper.initQueues();
    computeQueueWrapper.initQueues();

    // コマンドバッファの初期化
    graphicsCommandBufWrapper.initCommandBuf(graphicsQueueWrapper);
    computeCommandBufWrapper.initCommandBuf(computeQueueWrapper);

    std::cout << "デバイスの初期化が完了しました" << std::endl;
}

//なるべく数の多いキューファミリーを選択
void VulkanContext::DeviceWrapper::QueueWrapper::findQueues(QueueType queueType) {
    std::vector<vk::QueueFamilyProperties> queueProps = deviceWrapper.context.physicalDevice.getQueueFamilyProperties();
    
    uint32_t graphicsQueueIndex = -1;
    uint32_t graphicsQueueCount = 0;
    uint32_t computeQueueIndex = -1;
    uint32_t computeQueueCount = 0;

    for(uint32_t i = 0; i < queueProps.size(); i++) {//キューを持つ数が最大のものを選択
        if(queueProps[i].queueFlags & vk::QueueFlagBits::eGraphics && deviceWrapper.context.physicalDevice.getSurfaceSupportKHR(i, deviceWrapper.context.surface.get()) && !queueCreateInfos.contains(i)) {
            graphicsQueueCount = std::max(graphicsQueueCount, queueProps[i].queueCount);
            if(graphicsQueueCount == queueProps[i].queueCount) {
                graphicsQueueIndex = i;
            }
        }
        if(queueProps[i].queueFlags & vk::QueueFlagBits::eCompute && !queueCreateInfos.contains(i)) {
            computeQueueCount = std::max(computeQueueCount, queueProps[i].queueCount);
            if(computeQueueCount == queueProps[i].queueCount) {
                computeQueueIndex = i;
            }
        }
    }


    vk::DeviceQueueCreateInfo queueCreateInfo;  // switch文の前で変数を宣言
    switch (queueType) {
        case QueueType::Graphics:
            if(graphicsQueueIndex == -1) {
                throw std::runtime_error("グラフィックスキューが見つかりませんでした");
            }
            queueFamilyIndex = graphicsQueueIndex;
            for(uint32_t i = 0; i < graphicsQueueCount; i++) {
                queuePriorities.push_back(i / graphicsQueueCount);
            }
            queueCreateInfo = vk::DeviceQueueCreateInfo(  // 既存の変数に代入
                {},
                graphicsQueueIndex,
                graphicsQueueCount,
                queuePriorities.data()
            );
            queueCreateInfos[graphicsQueueIndex] = queueCreateInfo;
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
            queueCreateInfo = vk::DeviceQueueCreateInfo(  // 既存の変数に代入
                {},
                computeQueueIndex,
                computeQueueCount,
                queuePriorities.data()
            );
            queueCreateInfos[computeQueueIndex] = queueCreateInfo;
            usedQueueFamilyIndices.push_back(computeQueueIndex);
            break;
        
        default:
            throw std::runtime_error("不正なキュータイプです");
            break;
    }
}    

//キューの初期化
void VulkanContext::DeviceWrapper::QueueWrapper::initQueues() {

    for(uint32_t i = 0; i < queueCreateInfos.at(queueFamilyIndex).queueCount; i++) {
        queues.push_back(deviceWrapper.device->getQueue(queueFamilyIndex, i));
    }

}

//コマンドバッファの作成
void VulkanContext::DeviceWrapper::CommandBufWrapper::initCommandBuf(QueueWrapper& queueWrapper){
    vk::CommandPoolCreateInfo poolCreateInfo(
        {vk::CommandPoolCreateFlagBits::eResetCommandBuffer},
        queueWrapper.queueFamilyIndex
    );
    commandPool = queueWrapper.deviceWrapper.device->createCommandPoolUnique(poolCreateInfo);

    vk::CommandBufferAllocateInfo allocInfo(
        *commandPool,
        vk::CommandBufferLevel::ePrimary,
        1
    );
    commandBuffers = queueWrapper.deviceWrapper.device->allocateCommandBuffersUnique(allocInfo);
}
