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

    // スワップチェインの初期化
    swapchainWrapper.initSwapchain();

    // パイプラインの初期化
    pipelineWrapper.initPipeline();

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

void VulkanContext::DeviceWrapper::QueueWrapper::submit(vk::SubmitInfo submitInfo) {
    queues.at(0).submit(submitInfo, VK_NULL_HANDLE);
}

void VulkanContext::DeviceWrapper::QueueWrapper::present(vk::PresentInfoKHR presentInfo) {
    queues.at(0).presentKHR(presentInfo);
    queues.at(0).waitIdle();
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

void VulkanContext::DeviceWrapper::CommandBufWrapper::startRendering(vk::RenderingInfo renderingInfo) {
    vk::CommandBufferBeginInfo beginInfo;
    commandBuffers.at(0)->begin(beginInfo);
    commandBuffers.at(0)->beginRendering(renderingInfo);
} 

void VulkanContext::DeviceWrapper::CommandBufWrapper::endRendering(vk::ImageMemoryBarrier imageMemoryBarrier) {
    commandBuffers.at(0)->endRendering();
    commandBuffers.at(0)->pipelineBarrier(
        vk::PipelineStageFlagBits::eColorAttachmentOutput,
        vk::PipelineStageFlagBits::eBottomOfPipe,
        {},
        {},
        {},
        imageMemoryBarrier
    );
    commandBuffers.at(0)->end();
}

vk::SubmitInfo VulkanContext::DeviceWrapper::CommandBufWrapper::getSubmitInfo() {
    return vk::SubmitInfo(
        0,
        nullptr,
        nullptr,
        1,
        &commandBuffers.at(0).get()
    );
}

//スワップチェインの初期化
void VulkanContext::DeviceWrapper::SwapchainWrapper::initSwapchain() {
    vk::SurfaceCapabilitiesKHR surfaceCapabilities = deviceWrapper.context.physicalDevice.getSurfaceCapabilitiesKHR(deviceWrapper.context.surface.get());
    std::vector<vk::SurfaceFormatKHR> surfaceFormats = deviceWrapper.context.physicalDevice.getSurfaceFormatsKHR(deviceWrapper.context.surface.get());
    std::vector<vk::PresentModeKHR> surfacePresentModes = deviceWrapper.context.physicalDevice.getSurfacePresentModesKHR(deviceWrapper.context.surface.get());

    vk::SurfaceFormatKHR swapchainFormat = surfaceFormats[0];
    vk::PresentModeKHR presentMode = surfacePresentModes[0];

    vk::SwapchainCreateInfoKHR swapchainCreateInfo(
        {},
        deviceWrapper.context.surface.get(),
        surfaceCapabilities.minImageCount + 1,
        swapchainFormat.format,
        swapchainFormat.colorSpace,
        surfaceCapabilities.currentExtent,
        1,
        vk::ImageUsageFlagBits::eColorAttachment,
        vk::SharingMode::eExclusive,
        0,
        nullptr,
        surfaceCapabilities.currentTransform,
        vk::CompositeAlphaFlagBitsKHR::eOpaque,
        presentMode,
        VK_TRUE,
        nullptr
    );
    swapchain = deviceWrapper.device->createSwapchainKHRUnique(swapchainCreateInfo);
    swapchainImages = deviceWrapper.device->getSwapchainImagesKHR(swapchain.get());

    for(uint32_t i = 0; i < swapchainImages.size(); i++) {
        vk::ImageViewCreateInfo imageViewCreateInfo(
            {},
            swapchainImages[i],
            vk::ImageViewType::e2D,
            swapchainFormat.format,
            vk::ComponentMapping(),
            vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)
        );
        swapchainImageViews.push_back(deviceWrapper.device->createImageViewUnique(imageViewCreateInfo));
    }

    vk::FenceCreateInfo fenceCreateInfo{};
    swapchainImgFence = deviceWrapper.device->createFenceUnique(fenceCreateInfo);

}

vk::ImageView VulkanContext::DeviceWrapper::SwapchainWrapper::getNextImage() {
    deviceWrapper.device->resetFences({swapchainImgFence.get()});
    vk::ResultValue acquireResult = deviceWrapper.device->acquireNextImageKHR(swapchain.get(), UINT64_MAX,{} ,swapchainImgFence.get());

    if (acquireResult.result != vk::Result::eSuccess) {
        throw std::runtime_error("スワップチェーンイメージの取得に失敗しました");
    }
    imageIndex = acquireResult.value;

    if (deviceWrapper.device->waitForFences({swapchainImgFence.get()}, VK_TRUE, UINT64_MAX) != vk::Result::eSuccess) {
        throw std::runtime_error("フェンスの待機に失敗しました");
    }

    return swapchainImageViews[imageIndex].get();
}

vk::PresentInfoKHR VulkanContext::DeviceWrapper::SwapchainWrapper::getPresentInfo() {
    auto imageIndices = {imageIndex};

    return vk::PresentInfoKHR(
        0,                          // waitSemaphoreCount
        nullptr,                    // pWaitSemaphores
        1,                          // swapchainCount
        &swapchain.get(),          // pSwapchains
        &imageIndex,               // pImageIndices
        nullptr                     // pResults
    );
}

vk::ImageMemoryBarrier VulkanContext::DeviceWrapper::SwapchainWrapper::getImageMemoryBarrier(vk::ImageLayout oldLayout, vk::ImageLayout newLayout) {
    return vk::ImageMemoryBarrier(
        {},//srcAccessMask
        {},//dstAccessMask
        oldLayout,//oldLayout
        newLayout,//newLayout
        VK_QUEUE_FAMILY_IGNORED,//srcQueueFamilyIndex
        VK_QUEUE_FAMILY_IGNORED,//dstQueueFamilyIndex
        swapchainImages.at(imageIndex),//image
        vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)//subresourceRange
    );
}

void VulkanContext::DeviceWrapper::draw(){
    vk::ImageView swapChainImageView = swapchainWrapper.getNextImage();

    std::vector<vk::RenderingAttachmentInfo> colorAttachments = {
        vk::RenderingAttachmentInfo(
            swapChainImageView,// imageView
            vk::ImageLayout::eColorAttachmentOptimal, // imageLayout
            vk::ResolveModeFlagBits::eNone, // resolveMode
            {},                          // resolveImageView
            vk::ImageLayout::eUndefined, // resolveImageLayout
            vk::AttachmentLoadOp::eClear, // loadOp
            vk::AttachmentStoreOp::eStore, // storeOp
            vk::ClearValue{}             // clearValue
        )
    };

    vk::RenderingInfo renderingInfo(
        {},//flags
        vk::Rect2D({0, 0},{context.width, context.height}),//renderArea
        1,//layerCount
        0,//viewMask
        colorAttachments.size(),//colorAttachmentCount
        colorAttachments.data(),//pColorAttachments
        nullptr,//pDepthAttachment
        nullptr//pStencilAttachment
    );

    graphicsCommandBufWrapper.startRendering(renderingInfo);
    vk::ImageMemoryBarrier imageMemoryBarrier = swapchainWrapper.getImageMemoryBarrier(vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR);
    graphicsCommandBufWrapper.endRendering(imageMemoryBarrier);
    vk::SubmitInfo submitInfo = graphicsCommandBufWrapper.getSubmitInfo();

    graphicsQueueWrapper.submit(submitInfo);

    vk::PresentInfoKHR presentInfo = swapchainWrapper.getPresentInfo();
    graphicsQueueWrapper.present(presentInfo);

}

