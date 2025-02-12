#include "header.hpp"

class VulkanContext {
    public:
        void initWindow(uint32_t width, uint32_t height);
        void initVulkan();

        class MyDevice {
            public:

            private:
                vk::UniqueDevice device;
        };
        
        MyDevice myDevice;

    private:

        GLFWwindow* window;

        vk::UniqueInstance instance;
        vk::PhysicalDevice physicalDevice;

        
};