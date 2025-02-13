#include "header.hpp"

class VulkanContext {
    public:
        // コピー禁止を明示的に宣言
        VulkanContext(const VulkanContext&) = delete;
        VulkanContext& operator=(const VulkanContext&) = delete;
        // ムーブは許可
        VulkanContext(VulkanContext&&) noexcept = default;
        VulkanContext& operator=(VulkanContext&&) noexcept = default;

        void initWindow(uint32_t width, uint32_t height);
        void initVulkan();

    private:
        GLFWwindow* window;

        vk::UniqueInstance instance;

        std::vector<const char*> deviceExtensions;
        vk::PhysicalDeviceFeatures deviceFeatures;
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
            friend class VulkanContext;
            public:
                // コピー禁止を明示的に宣言
                DeviceWrapper(const DeviceWrapper&) = delete;
                DeviceWrapper& operator=(const DeviceWrapper&) = delete;

                DeviceWrapper(VulkanContext& ctx) 
                    : context(ctx)
                    , graphicsQueueWrapper(*this)
                    , computeQueueWrapper(*this) {}

                //ムーブコンストラクタ
                DeviceWrapper(DeviceWrapper&& other) noexcept
                    : context(other.context)
                    , device(std::move(other.device))
                    , graphicsQueueWrapper(*this)
                    , computeQueueWrapper(*this) {}
                //ムーブ代入演算子
                DeviceWrapper& operator=(DeviceWrapper&& other) noexcept {
                    if(this != &other) {
                        device = std::move(other.device);
                        graphicsQueueWrapper = QueueWrapper(*this);
                        computeQueueWrapper = QueueWrapper(*this);
                    }
                    return *this;
                }
                
                void initDevice();

            private:
                VulkanContext& context;//VulkanContextの参照を持つ

                vk::UniqueDevice device;
                
                class QueueWrapper{
                    friend class DeviceWrapper;
                    public:
                        enum class QueueType{
                            Graphics,
                            Compute,
                            Count
                        };

                        // コピー禁止を明示的に宣言
                        QueueWrapper(const QueueWrapper&) = delete;
                        QueueWrapper& operator=(const QueueWrapper&) = delete;

                        QueueWrapper(DeviceWrapper& dev) : deviceWrapper(dev) {};
                        //ムーブコンストラクタ
                        QueueWrapper(QueueWrapper&& other) noexcept
                            : deviceWrapper(other.deviceWrapper)                    // デバイスラッパーへの参照を移動
                            , queueType(std::move(other.queueType))               // キュータイプを移動
                            , queueFamilyIndex(std::move(other.queueFamilyIndex)) // キューファミリーインデックスを移動
                            , queuePriorities(std::move(other.queuePriorities))   // 優先度を移動
                            , queues(std::move(other.queues)) {}                  // キューを移動
                        //ムーブ代入演算子
                        QueueWrapper& operator=(QueueWrapper&& other) noexcept {
                            if(this != &other) {
                                queueType = std::move(other.queueType);
                                queueFamilyIndex = std::move(other.queueFamilyIndex);
                                queuePriorities = std::move(other.queuePriorities);
                                queues = std::move(other.queues);
                            }
                            return *this;
                        }


                        void findQueues(QueueType queueType);//キューを探す
                        std::map<uint32_t, vk::DeviceQueueCreateInfo> getQueueCreateInfos(){return queueCreateInfos;};//queueCreateInfosを取得
                        //論理デバイスの初期化を挟む
                        void initQueues();//queueを初期化
        
                    private:
                        DeviceWrapper& deviceWrapper;
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