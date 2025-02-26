#pragma once
#include "vulkanContext.hpp"
#include "geometry.hpp"
//#include "pipelineBuilder.hpp"

class Application {

    public:
        void run() {
            readGLTF("./Resource/Fox.glb");
            readGLTF("./Resource/DamagedHelmet.glb");
            vulkanContext.initWindow(800, 600);
            vulkanContext.initVulkan();
            while(!vulkanContext.windowShouldClose()) {
                glfwPollEvents();
                vulkanContext.draw();
            }
            vulkanContext.cleanup();
        }

    private:
        VulkanContext vulkanContext;
};