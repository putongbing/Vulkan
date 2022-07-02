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
    void Run();
private:
    void InitVulkan();
    void CreateWin32Window();
    void Cleanup();
    void MainLoop();
    void CreateVKInstance();
    bool CheckValidationLayerSupport();
    std::vector<const char*> GetRequiredExtensions();

    //surface
    void CreateSurface();
    
    //logical device
    void CreateLogicalDevice();
    
    //physical deveic
    void PickPhysicalDevice();
    bool IsPhysicalDeviceSuitable(VkPhysicalDevice device);
    
    //queue families
    void FindQueueFamilies();
    
    //get debug infomation
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
private:
    //window infomation
    GLFWwindow* window = nullptr;
    const uint32_t width = 800;
    const uint32_t height = 600;
    std::string title = "Vulkan Learn";

    // vulkan infomation
    VkInstance instance = nullptr;

    //validation infomation
    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation",
    };

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif
    VkDebugUtilsMessengerEXT debugMessenger;

    //physical device
    VkPhysicalDevice physicalDevice = nullptr;

    //queue families
    QueueFamilyIndices queueFamilyIndices;

    //logical device
    VkDevice logicalDevice = nullptr;

    //surface
    VkSurfaceKHR surface = nullptr;

    //queue
    VkQueue queueGraphics = nullptr;
    VkQueue queuePresent = nullptr;
};
