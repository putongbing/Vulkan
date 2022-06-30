#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.h>
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>


class Renderer
{
public:
    void Run();
private:
    void InitVulkan();
    void CreateWindow();
    void Cleanup();
    void MainLoop();
    void CreateVKInstance();
    bool CheckValidationLayerSupport();
    std::vector<const char*> GetRequiredExtensions();
    //get debug infomation
    void SetDebugCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& info);
    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagBitsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
        void* userData);
    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT* info,
        const VkAllocationCallbacks* allocator,
        VkDebugUtilsMessengerEXT debugMessenger);
    void DestoryDebugUtilsMessengerEXT(VkInstance instance,
        VkDebugUtilsMessengerEXT debugMessenger,
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
};
