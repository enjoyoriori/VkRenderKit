#include "header.hpp"

class VulkanContext {
    public:
        void initWindow(uint32_t width, uint32_t height);
        void initVulkan();

    private:
        GLFWwindow* window;

        vk::UniqueInstance instance;
        vk::PhysicalDevice physicalDevice;

        VkSurfaceKHR c_surface;
        vk::UniqueSurfaceKHR surface;

        // 物理デバイスの選択
        vk::PhysicalDevice pickPhysicalDevice(const std::vector<const char*>& deviceExtensions, vk::PhysicalDeviceFeatures deviceFeatures);
        bool checkDeviceExtensionSupport(vk::PhysicalDevice device,  const std::vector<const char*>& deviceExtensions);
        bool checkDeviceFeatures(vk::PhysicalDevice device, vk::PhysicalDeviceFeatures requiredFeatures);
        
        // サーフェスの作成
        void createSurface();

        class DeviceWrapper{
            public:
                void initDevice();

            private:
                vk::UniqueDevice device;
                
                class QueueWrapper{
                    public:
                        enum class QueueType{
                            Graphics,
                            Compute,
                            Count
                        };

                        std::vector<vk::DeviceQueueCreateInfo> findQueues();//queue情報を取得
                        void initQueues();//queueを初期化
        
                    private:
                        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos{static_cast<size_t>(QueueType::Count), vk::DeviceQueueCreateInfo{}};
                        std::vector<vk::Queue> graphicsQueue;
                        std::vector<vk::Queue> computeQueue;
                        std::vector<float> graphicQueuePriorities;
                        std::vector<float> computeQueuePriorities;
                };
                QueueWrapper queueWrapper;

        };
        DeviceWrapper deviceWrapper;

        
};