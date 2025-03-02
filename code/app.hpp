#pragma once
#include "vulkanContext.hpp"
#include "geometry.hpp"
//#include "pipelineBuilder.hpp"

class Application {

    public:
        void run() {
            geometry::Model fox;
            fox.readGLTF("./Resource/Fox.glb");
            geometry::Model damagedHelmet;
            damagedHelmet.readGLTF("./Resource/DamagedHelmet.glb");

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