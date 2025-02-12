#pragma once
#include "vulkanContext.hpp"
//#include "geometry.hpp"
//#include "pipelineBuilder.hpp"

class Application {

    public:
        void run() {
            VulkanContext vulkanContext;
            vulkanContext.initWindow(800, 600);
            vulkanContext.initVulkan();
            //mainLoop();
            //cleanup();
        }

    private:
        VulkanContext vulkanContext;
    
};