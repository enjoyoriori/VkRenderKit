#include "vulkanContext.hpp"
#include "geometry.hpp"

vk::UniqueShaderModule VulkanContext::DeviceWrapper::PipelineWrapper::initShaderModule(std::string filename) {
    size_t spvFileSz = std::filesystem::file_size(filename);

    std::ifstream spvFile(filename, std::ios::binary);
    if (!spvFile.is_open()) {
        throw std::runtime_error("シェーダーファイルを開けませんでした: " + filename);
    }

    std::vector<char> spvFileData(spvFileSz);
    spvFile.read(spvFileData.data(), spvFileSz);
    if (!spvFile) {
        throw std::runtime_error("シェーダーファイルの読み込みに失敗しました: " + filename);
    }

    vk::ShaderModuleCreateInfo shaderCreateInfo{};
    shaderCreateInfo.codeSize = spvFileSz;
    shaderCreateInfo.pCode = reinterpret_cast<const uint32_t*>(spvFileData.data());

    return deviceWrapper.device->createShaderModuleUnique(shaderCreateInfo);
}

void VulkanContext::DeviceWrapper::PipelineWrapper::initPipeline(){
    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
    vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
    vk::PipelineInputAssemblyStateCreateInfo inputAssembly;
    vk::PipelineViewportStateCreateInfo viewportState;
    vk::PipelineRasterizationStateCreateInfo rasterizer;
    vk::PipelineDepthStencilStateCreateInfo depthStencil;
    vk::PipelineMultisampleStateCreateInfo multisampling;
    vk::PipelineColorBlendStateCreateInfo colorBlending;
    vk::PipelineDynamicStateCreateInfo dynamicState;

    uint32_t WIDTH = deviceWrapper.context.width;
    uint32_t HEIGHT = deviceWrapper.context.height;
    
    //シェーダーモジュールの初期化
    vk::UniqueShaderModule vertShaderModule = initShaderModule("./shader/shader.vert.spv");
    vk::UniqueShaderModule fragShaderModule = initShaderModule("./shader/shader.frag.spv");

    shaderStages = {
        vk::PipelineShaderStageCreateInfo{
            {},
            vk::ShaderStageFlagBits::eVertex,
            vertShaderModule.get(),
            "main"
        },
        vk::PipelineShaderStageCreateInfo{
            {},
            vk::ShaderStageFlagBits::eFragment,
            fragShaderModule.get(),
            "main"
        }
    };

    //VertexInputStateの設定
    vk::VertexInputBindingDescription bindingDescription = Vertex::getBindingDescription();
    //vk::VertexInputBindingDescription instanceBindingDescription = Object::getBindingDescription();

    std::vector<vk::VertexInputAttributeDescription> attributeDescriptions = Vertex::getAttributeDescriptions();
    //std::vector<vk::VertexInputAttributeDescription> instanceAttributeDescriptions = Object::getAttributeDescriptions();

    //std::vector<vk::VertexInputBindingDescription> bindingDescriptions = {bindingDescription, instanceBindingDescription};
    //attributeDescriptions.insert(attributeDescriptions.end(), instanceAttributeDescriptions.begin(), instanceAttributeDescriptions.end());
    
    std::vector<vk::VertexInputBindingDescription> bindingDescriptions = {bindingDescription};

    vertexInputInfo = vk::PipelineVertexInputStateCreateInfo(
        {},//flags
        bindingDescriptions.size(),//vertexBindingDescriptionCount
        bindingDescriptions.data(),//pVertexBindingDescriptions
        attributeDescriptions.size(),//vertexAttributeDescriptionCount
        attributeDescriptions.data()//pVertexAttributeDescriptions
    );
    /*
    vertexInputInfo = vk::PipelineVertexInputStateCreateInfo(
        {}, // flags
        0,  // vertexBindingDescriptionCount
        nullptr, // pVertexBindingDescriptions
        0,  // vertexAttributeDescriptionCount
        nullptr // pVertexAttributeDescriptions
    );
    */
    //InputAssemblyStateの設定
    inputAssembly = vk::PipelineInputAssemblyStateCreateInfo(
        {},//flags
        vk::PrimitiveTopology::eTriangleList,//topology
        VK_FALSE//primitiveRestartEnable
    );

    vk::Viewport viewport(
        0.0f, 0.0f,
        static_cast<float>(WIDTH), static_cast<float>(HEIGHT),
        0.0f, 1.0f
    );

    vk::Rect2D scissor(
        {0, 0},
        {WIDTH, HEIGHT}
    );

    viewportState = vk::PipelineViewportStateCreateInfo(
        {},//flags
        1,//viewportCount
        &viewport,//pViewports
        1,//scissorCount
        &scissor//pScissors
    );

    rasterizer = vk::PipelineRasterizationStateCreateInfo(
        {},//flags
        VK_FALSE,//depthClampEnable
        VK_FALSE,//rasterizerDiscardEnable
        vk::PolygonMode::eFill,//polygonMode
        vk::CullModeFlagBits::eBack,//cullMode
        vk::FrontFace::eCounterClockwise,//frontFace
        VK_FALSE,//depthBiasEnable
        0.0f,//depthBiasConstantFactor
        0.0f,//depthBiasClamp
        0.0f,//depthBiasSlopeFactor
        1.0f//lineWidth
    );

    multisampling = vk::PipelineMultisampleStateCreateInfo(
        {},//flags
        vk::SampleCountFlagBits::e1,//rasterizationSamples
        VK_FALSE,//sampleShadingEnable
        1.0f,//minSampleShading
        nullptr,//pSampleMask
        VK_FALSE,//alphaToCoverageEnable
        VK_FALSE//alphaToOneEnable
    );

    vk::PipelineColorBlendAttachmentState colorBlendAttachment(
        VK_FALSE,//blendEnable
        vk::BlendFactor::eOne,//srcColorBlendFactor
        vk::BlendFactor::eZero,//dstColorBlendFactor
        vk::BlendOp::eAdd,//colorBlendOp
        vk::BlendFactor::eOne,//srcAlphaBlendFactor
        vk::BlendFactor::eZero,//dstAlphaBlendFactor
        vk::BlendOp::eAdd,//alphaBlendOp
        vk::ColorComponentFlags(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA)//colorWriteMask
    );

    colorBlending = vk::PipelineColorBlendStateCreateInfo(
        {},//flags
        VK_FALSE,//logicOpEnable
        vk::LogicOp::eCopy,//logicOp
        1,//attachmentCount
        &colorBlendAttachment,//pAttachments
        {0.0f, 0.0f, 0.0f, 0.0f}//blendConstants
    );

    //DynamicStateの設定
    std::vector<vk::DynamicState> dynamicStates = {
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor
    };

    dynamicState = vk::PipelineDynamicStateCreateInfo(
        {},//flags
        dynamicStates.size(),//dynamicStateCount
        dynamicStates.data()//pDynamicStates
    );

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo(
        {},//flags
        0,//setLayoutCount
        nullptr,//pSetLayouts
        0,//pushConstantRangeCount
        nullptr//pPushConstantRanges
    );

    vk::UniquePipelineLayout pipelineLayout = deviceWrapper.device->createPipelineLayoutUnique(pipelineLayoutInfo);

    //RenderingCreateInfoの設定
    std::vector<vk::Format> colorAttachmentFormats = {vk::Format::eB8G8R8A8Unorm};

    vk::PipelineRenderingCreateInfo renderingCreateInfo(
        0,//viewMask
        colorAttachmentFormats.size(),//colorAttachmentCount
        colorAttachmentFormats.data(),//pColorAttachmentFormats
        vk::Format::eUndefined,//depthAttachmentFormat
        vk::Format::eUndefined//stencilAttachmentFormat
    );

    vk::GraphicsPipelineCreateInfo pipelineCreateInfo(
        {},//flags
        shaderStages.size(),//stageCount
        shaderStages.data(),//pStages
        &vertexInputInfo, // pVertexInputState
        &inputAssembly, // pInputAssemblyState
        VK_NULL_HANDLE, // pTessellationState
        &viewportState, // pViewportState
        &rasterizer, // pRasterizationState
        &multisampling, // pMultisampleState
        nullptr, // pDepthStencilState
        &colorBlending, // pColorBlendState
        VK_NULL_HANDLE, // pDynamicState
        pipelineLayout.get(), // layout
        VK_NULL_HANDLE, // renderPass
        0, // subpass
        VK_NULL_HANDLE, // basePipelineHandle
        -1 // basePipelineIndex
    );
    pipelineCreateInfo.setPNext(&renderingCreateInfo);

    vk::UniquePipeline pipeline = deviceWrapper.device->createGraphicsPipelineUnique(VK_NULL_HANDLE, pipelineCreateInfo).value;

}