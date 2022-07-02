#include "Renderer.h"

#include <set>

#define CHECK_SUCCESS(res, errorInfo) \
if(res != VK_SUCCESS) \
{\
    throw std::runtime_error(errorInfo);\
}

void Renderer::InitVulkan()
{
    CreateVKInstance();
    //the window surface needs to be created right after the instance creation,
    //because it can actually influence the physical device selection
    CreateSurface();
    PickPhysicalDevice();
    FindQueueFamilies();
    if (!queueFamilyIndices.IsComplete())
    {
        throw std::runtime_error("no find queue family!!!");
    }
    CreateLogicalDevice();
}


void Renderer::CreateWin32Window()
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
    info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
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

    res = CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr);
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

void Renderer::CreateSurface()
{
    VkWin32SurfaceCreateInfoKHR surfaceInfo{};
    surfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surfaceInfo.hwnd = glfwGetWin32Window(window);
    surfaceInfo.hinstance = GetModuleHandle(nullptr);

    VkResult res = vkCreateWin32SurfaceKHR(instance, &surfaceInfo, nullptr, &surface);
    CHECK_SUCCESS(res, "failed to create win32 surface!!!")
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
    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    {
        std::cout << callbackData->pMessage << std::endl;
    }
    return VK_FALSE;
}

VkResult Renderer::CreateDebugUtilsMessengerEXT(VkInstance instance, 
    const VkDebugUtilsMessengerCreateInfoEXT* info,
    const VkAllocationCallbacks* allocator)
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
    const VkAllocationCallbacks* allocator)
{
    auto  func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if(func)
    {
        func(instance, debugMessenger, allocator);
    }
}

void Renderer::PickPhysicalDevice()
{
    uint32_t physicalDeviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);
    if (physicalDeviceCount == 0)
    {
        throw std::runtime_error("no find physical device!!!");
    }
    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
    vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data());
    for (auto& physical : physicalDevices)
    {
        if (IsPhysicalDeviceSuitable(physical))
        {
            physicalDevice = physical;
            break;
        }
    }

    if (physicalDevice == nullptr)
    {
        throw std::runtime_error("no suitable physical device!!!");
    }
}

bool Renderer::IsPhysicalDeviceSuitable(VkPhysicalDevice device)
{
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    VkPhysicalDeviceFeatures deviceFeature;
    vkGetPhysicalDeviceFeatures(device, &deviceFeature);

    return (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) &&
        (deviceFeature.geometryShader);
}

void Renderer::FindQueueFamilies()
{
    //find graphics queue family
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, 
        &queueFamilyCount, 
        queueFamilyProperties.data());
    for (int i = 0; i < queueFamilyProperties.size(); i++)
    {
        VkQueueFamilyProperties properties = queueFamilyProperties.at(i);
        if (properties.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            queueFamilyIndices.graphicsFamily = i;
            if(queueFamilyIndices.presentFamily.has_value())
                break;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
        if(presentSupport)
        {
            queueFamilyIndices.presentFamily = i;
            if(queueFamilyIndices.graphicsFamily.has_value())
                break;
        }
    }


    
}

void Renderer::CreateLogicalDevice()
{
    std::set<uint32_t> queueIndices = { queueFamilyIndices.graphicsFamily.value(),
                                        queueFamilyIndices.presentFamily.value()};
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfoList;
    for(const unsigned int& indice : queueIndices)
    {
        VkDeviceQueueCreateInfo queueInfo{};
        queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueInfo.queueCount = 1;
        queueInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
        float priority = 1.0f;
        queueInfo.pQueuePriorities = &priority;
        queueCreateInfoList.push_back(queueInfo);
    }


    VkDeviceCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO; 
    info.pQueueCreateInfos = queueCreateInfoList.data();
    info.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfoList.size());
    VkPhysicalDeviceFeatures deviceFeature{};
    info.pEnabledFeatures = &deviceFeature;
    info.enabledExtensionCount = 0;

    VkResult res = vkCreateDevice(physicalDevice, &info, nullptr, &logicalDevice);
    CHECK_SUCCESS(res, "can't to create logical device!!!");


    vkGetDeviceQueue(logicalDevice,
        queueFamilyIndices.graphicsFamily.value(),
        0,
        &queueGraphics);

    vkGetDeviceQueue(logicalDevice,
    queueFamilyIndices.presentFamily.value(),
    0,
    &queuePresent);
}


void Renderer::Run()
{
    CreateWin32Window();
    CheckValidationLayerSupport();
    InitVulkan();
    MainLoop();
    Cleanup();
}

void Renderer::Cleanup()
{
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyDevice(logicalDevice, nullptr);
    if(enableValidationLayers)
        DestoryDebugUtilsMessengerEXT(instance, nullptr);
    vkDestroyInstance(instance, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
}