#pragma once
#include "vulkanContext.hpp"
#include "geometry.hpp"
//#include "pipelineBuilder.hpp"

class Application {

    public:
        void run() {
            loadGLTF("./Resource/Fox.glb");
            loadGLTF("./Resource/DamagedHelmet.glb");
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