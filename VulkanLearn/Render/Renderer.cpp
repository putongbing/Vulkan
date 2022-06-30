#include "Renderer.h"

#define CHECK_SUCCESS(res, errorInfo) \
if(res != VK_SUCCESS) \
{\
    throw std::runtime_error(errorInfo);\
}

void Renderer::InitVulkan()
{
    CreateVKInstance();
}


void Renderer::CreateWindow()
{
    glfwInit();
    
    //tell glfw that no require create opengl context
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);


    window = glfwCreateWindow(width, height, title.data(), nullptr, nullptr);
    if(!window)
    {
        throw std::runtime_error("window create fail!");
    }
}

void Renderer::MainLoop()
{
    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }
}

void Renderer::CreateVKInstance()
{
    //fill application infomation
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan Learn";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Engine Learn";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;
    
    // uint32_t vkExtensionCount;
    // vkEnumerateInstanceExtensionProperties(nullptr, &vkExtensionCount, nullptr);
    // std::vector<VkExtensionProperties> vkExtensions(vkExtensionCount);
    // vkEnumerateInstanceExtensionProperties(nullptr, &vkExtensionCount, vkExtensions.data());
    // for(VkExtensionProperties extension : vkExtensions)
    // {
    //     std::cout << extension.extensionName << std::endl;
    // }

    std::vector<const char*> extensions = GetRequiredExtensions();
    bool flag = CheckValidationLayerSupport();
    if(!flag)
    {
        throw std::runtime_error("not support validation layer!!!");
    }
    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    SetDebugCreateInfo(createInfo);
    VkInstanceCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    info.pApplicationInfo = &appInfo;
    info.enabledExtensionCount = extensions.size();
    info.ppEnabledExtensionNames = extensions.data();
    if(enableValidationLayers)
    {
        info.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        info.ppEnabledLayerNames = validationLayers.data();
        info.pNext = &createInfo;
    }
    else
    {
        info.enabledLayerCount = 0;
        info.pNext = nullptr;
    }
    
    VkResult res = vkCreateInstance(&info, nullptr, &instance);
    CHECK_SUCCESS(res, "failed to create vk instance!!!")

    res = CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, debugMessenger);
    CHECK_SUCCESS(res, "failed to create debug messenger!!!")
}

bool Renderer::CheckValidationLayerSupport()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> layerProperties(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, layerProperties.data());
    for(auto& validation : validationLayers)
    {
        bool exist = false;
        for(auto& property : layerProperties)
        {
            if(std::strcmp(validation, property.layerName) == 0)
            {
                exist = true;
                break;
            }
        }
        
        if(!exist)
        {
            return false;
        }
    }
    return true;
}

std::vector<const char*> Renderer::GetRequiredExtensions()
{
    uint32_t extensionCount = 0;
    const char** extensions;
    extensions = glfwGetRequiredInstanceExtensions(&extensionCount);
    // for(int i = 0; i < extensionCount; i++)
    // {
    //     std::cout << extensions[i] << std::endl;
    // }
    std::vector<const char*> resExt(extensions, extensions + extensionCount);
    if(enableValidationLayers)
        resExt.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    return resExt;
}

void Renderer::SetDebugCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& info)
{
    if(!enableValidationLayers)
        return;
    info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    info.pfnUserCallback = (PFN_vkDebugUtilsMessengerCallbackEXT)DebugCallback;
    info.pUserData = nullptr;
}

VkBool32 Renderer::DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                 VkDebugUtilsMessageTypeFlagBitsEXT messageType,
                                 const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
                                 void* userData)
{
    std::cout << callbackData->pMessage << std::endl;
    return VK_FALSE;
}

VkResult Renderer::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* info,
    const VkAllocationCallbacks* allocator, VkDebugUtilsMessengerEXT debugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        instance, "vkCreateDebugUtilsMessengerEXT");
    if(func != nullptr)
    {
        return func(instance, info, allocator, &debugMessenger);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void Renderer::DestoryDebugUtilsMessengerEXT(VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks* allocator)
{
    auto  func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestoryDebugUtilsMessengerEXT");
    if(func)
    {
        func(instance, debugMessenger, allocator);
    }
}


void Renderer::Run()
{
    CreateWindow();
    CheckValidationLayerSupport();
    InitVulkan();
    MainLoop();
    Cleanup();
}

void Renderer::Cleanup()
{
    if(enableValidationLayers)
        DestoryDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    vkDestroyInstance(instance, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
}