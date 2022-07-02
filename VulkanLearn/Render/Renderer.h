#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <vulkan/vulkan.h>
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <optional>


class Renderer
{
public:
    struct QueueFamilyIndices
    {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;
        bool IsComplete()
        {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    struct SwapChain
    {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModels;
    };
    void Run();
private:
    void InitVulkan();
    void InitWindow();
    void Cleanup();
    void MainLoop();
    void CreateImageViews();

    std::vector<const char*> GetRequiredExtensions();

    //graphics pipline
    void CreateGeaphicsPipline();

    //vk instance
    void CreateVKInstance();

    //vk validation layer
    bool CheckValidationLayerSupport();
    void CreateValidationLayer();
    void SetDebugCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& info);
    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagBitsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
        void* userData);
    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT* info,
        const VkAllocationCallbacks* allocator);
    void DestoryDebugUtilsMessengerEXT(VkInstance instance,
        const VkAllocationCallbacks* allocator);

    //swap chain
    void CreateSwapChain();
    SwapChain QueryPhysicalDeviceSwapChainSupport(VkPhysicalDevice device);
    VkSurfaceFormatKHR ChooseSwapChainSurfaceFormat(std::vector<VkSurfaceFormatKHR>& formats);
    VkPresentModeKHR ChooseSwapChainPresentModel(std::vector<VkPresentModeKHR>& presentModels);
    VkExtent2D ChooseSwapChainCapbilities(VkSurfaceCapabilitiesKHR capabilities);

    //surface
    void CreateSurface();
    
    //logical device
    void CreateLogicalDevice();
    
    //physical deveic
    void PickPhysicalDevice();
    bool IsPhysicalDeviceSuitable(VkPhysicalDevice device);
    bool CheckPhysicalExtensionsSupport(VkPhysicalDevice device);
    
    //queue families
    QueueFamilyIndices QueryPhysicalDeviceQueueFamilies(VkPhysicalDevice device);
private:
    //window infomation
    GLFWwindow* _window = nullptr;
    const uint32_t _width = 800;
    const uint32_t _height = 600;
    std::string _title = "Vulkan Learn";

    // vulkan infomation
    VkInstance _instance = nullptr;

    //validation infomation
    const std::vector<const char*> _validationLayers =
    {
        "VK_LAYER_KHRONOS_validation",
    };

#ifdef NDEBUG
    const bool _enableValidationLayers = false;
#else
    const bool _enableValidationLayers = true;
#endif
    VkDebugUtilsMessengerEXT _debugMessenger;

    //physical device
    VkPhysicalDevice _physicalDevice = nullptr;

    //logical device
    VkDevice _logicalDevice = nullptr;

    //surface
    VkSurfaceKHR _surface = nullptr;

    //queue
    VkQueue _queueGraphics = nullptr;
    VkQueue _queuePresent = nullptr;

    //swap chain
    const std::vector<const char*> _deviceExtensions =
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
    VkSwapchainKHR _swapchain;
    std::vector<VkImage> _swapchainImages;
    VkFormat _swapchainImageFormat;
    VkExtent2D _swapchainExtent;
    std::vector<VkImageView> _imageViews;
};
