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

                        void findQueues(QueueType queueType);//キューを探す
                        std::map<uint32_t, vk::DeviceQueueCreateInfo> getQueueCreateInfos(){return queueCreateInfos;};//queueCreateInfosを取得
                        //論理デバイスの初期化を挟む
                        void initQueues();//queueを初期化
        
                    private:
                        QueueType queueType;
                        uint32_t queueFamilyIndex;
                        static std::vector<uint32_t> usedQueueFamilyIndices;
                        static std::map<uint32_t, vk::DeviceQueueCreateInfo> queueCreateInfos;
                        
                        std::vector<float> queuePriorities{};
                        std::vector<vk::Queue> queues;
                };
                QueueWrapper graphicsQueueWrapper;
                QueueWrapper computeQueueWrapper;

                class CommandPoolWrapper{
                    
                };

        };
        DeviceWrapper deviceWrapper;

        
};