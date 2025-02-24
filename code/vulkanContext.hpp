#include "header.hpp"

class VulkanContext {
    public:
        VulkanContext() : deviceWrapper(*this) {}

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
                DeviceWrapper(VulkanContext& ctx) 
                    : context(ctx)
                    , graphicsQueueWrapper(*this)
                    , computeQueueWrapper(*this)
                    , graphicsCommandBufWrapper(*this)
                    , computeCommandBufWrapper(*this)
                    , swapchainWrapper(*this) {}

                //ムーブ代入演算子
                DeviceWrapper& operator=(DeviceWrapper&& other) noexcept {
                    if(this != &other) {
                        device = std::move(other.device);
                        graphicsQueueWrapper = std::move(other.graphicsQueueWrapper);
                        computeQueueWrapper = std::move(other.computeQueueWrapper);
                        graphicsCommandBufWrapper = std::move(other.graphicsCommandBufWrapper);
                        computeCommandBufWrapper = std::move(other.computeCommandBufWrapper);
                        swapchainWrapper = std::move(other.swapchainWrapper);
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

                        QueueWrapper(DeviceWrapper& dev) : deviceWrapper(dev) {};
                        
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

                class CommandBufWrapper{
                    friend class DeviceWrapper;
                    public:
                       
                        CommandBufWrapper(DeviceWrapper& dev) : deviceWrapper(dev) {};
                        //ムーブコンストラクタ
                        CommandBufWrapper(CommandBufWrapper&& other) noexcept
                            : deviceWrapper(other.deviceWrapper)                    // デバイスラッパーへの参照を移動
                            , commandPool(std::move(other.commandPool))             // コマンドプールを移動
                            , commandBuffers(std::move(other.commandBuffers)) {}    // コマンドバッファを移動
                        //ムーブ代入演算子
                        CommandBufWrapper& operator=(CommandBufWrapper&& other) noexcept {
                            if(this != &other) {
                                commandPool = std::move(other.commandPool);
                                commandBuffers = std::move(other.commandBuffers);
                            }
                            return *this;
                        }

                        void initCommandBuf(QueueWrapper& queues);//コマンドバッファを初期化

                    private:
                        DeviceWrapper& deviceWrapper;
                        vk::UniqueCommandPool commandPool;
                        std::vector<vk::UniqueCommandBuffer> commandBuffers;
                };
                CommandBufWrapper graphicsCommandBufWrapper;
                CommandBufWrapper computeCommandBufWrapper;

                class SwapchainWrapper{
                    friend class DeviceWrapper;
                    
                    public:
                        SwapchainWrapper(DeviceWrapper& dev) : deviceWrapper(dev) {};

                        //ムーブコンストラクタ
                        SwapchainWrapper(SwapchainWrapper&& other) noexcept
                            : deviceWrapper(other.deviceWrapper)                    // デバイスラッパーへの参照を移動
                            , swapchain(std::move(other.swapchain))                 // スワップチェインを移動
                            , swapchainImages(std::move(other.swapchainImages))     // スワップチェインイメージを移動
                            , swapchainImageViews(std::move(other.swapchainImageViews)) {} // スワップチェインイメージビューを移動
                        
                        //ムーブ代入演算子
                        SwapchainWrapper& operator=(SwapchainWrapper&& other) noexcept {
                            if(this != &other) {
                                swapchain = std::move(other.swapchain);
                                swapchainImages = std::move(other.swapchainImages);
                                swapchainImageViews = std::move(other.swapchainImageViews);
                            }
                            return *this;
                        }

                        void initSwapchain();
                        
                    private:
                        DeviceWrapper& deviceWrapper;
                        vk::UniqueSwapchainKHR swapchain;
                        std::vector<vk::Image> swapchainImages;
                        std::vector<vk::UniqueImageView> swapchainImageViews;
                };
                SwapchainWrapper swapchainWrapper;

                class PipelineWrapper{
                    friend class DeviceWrapper;
                    public:

                        class ShaderModuleWrapper{
                            public:
                                ShaderModuleWrapper() = default;

                                // ムーブは許可
                                ShaderModuleWrapper(ShaderModuleWrapper&&) noexcept = default;
                                ShaderModuleWrapper& operator=(ShaderModuleWrapper&&) noexcept = default;

                                void initShaderModule(std::string filename);

                            private:
                                vk::UniqueShaderModule shaderModule;
                        };
                        std::vector<ShaderModuleWrapper> shaderModules;
                        
                    private:
                        DeviceWrapper& deviceWrapper;
                };

        };
        DeviceWrapper deviceWrapper;

};