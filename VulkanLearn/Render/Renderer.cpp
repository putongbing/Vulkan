#include "Renderer.h"

#include <set>
#include <limits>
#include <algorithm>

#define CHECK_SUCCESS(res, errorInfo) \
if(res != VK_SUCCESS) \
{\
    throw std::runtime_error(errorInfo);\
}

void Renderer::InitVulkan()
{
    CreateVKInstance();
    CreateValidationLayer();
    //the window surface needs to be created right after the instance creation,
    //because it can actually influence the physical device selection
    CreateSurface();
    PickPhysicalDevice();
    CreateLogicalDevice();
    CreateSwapChain();
    CreateImageViews();
    CreateGeaphicsPipline();
}


void Renderer::InitWindow()
{
    glfwInit();
    
    //tell glfw that no require create opengl context
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);


    _window = glfwCreateWindow(_width, _height, _title.data(), nullptr, nullptr);
    if(!_window)
    {
        throw std::runtime_error("window create fail!");
    }
}

void Renderer::MainLoop()
{
    while(!glfwWindowShouldClose(_window))
    {
        glfwPollEvents();
    }
}

void Renderer::CreateVKInstance()
{
    //check validation layer support
    if (_enableValidationLayers)
    {
        if(!CheckValidationLayerSupport())
        {
            throw std::runtime_error("not support validation layer!!!");
        }
    }

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
    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    SetDebugCreateInfo(createInfo);
    VkInstanceCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    info.pApplicationInfo = &appInfo;
    info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    info.ppEnabledExtensionNames = extensions.data();
    if(_enableValidationLayers)
    {
        info.enabledLayerCount = static_cast<uint32_t>(_validationLayers.size());
        info.ppEnabledLayerNames = _validationLayers.data();
        info.pNext = &createInfo;
    }
    else
    {
        info.enabledLayerCount = 0;
        info.pNext = nullptr;
    }
    
    VkResult res = vkCreateInstance(&info, nullptr, &_instance);
    CHECK_SUCCESS(res, "failed to create vk instance!!!")
}

bool Renderer::CheckValidationLayerSupport()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> layerProperties(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, layerProperties.data());
    for(auto& validation : _validationLayers)
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
    if(_enableValidationLayers)
        resExt.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    return resExt;
}

void Renderer::CreateSwapChain()
{
    SwapChain sc = QueryPhysicalDeviceSwapChainSupport(_physicalDevice);
    VkExtent2D extent = ChooseSwapChainCapbilities(sc.capabilities);
    VkSurfaceFormatKHR format = ChooseSwapChainSurfaceFormat(sc.formats);
    VkPresentModeKHR presentModel = ChooseSwapChainPresentModel(sc.presentModels);

    uint32_t imageCount = sc.capabilities.minImageCount + 1;
    imageCount = std::clamp(imageCount, sc.capabilities.minImageCount, sc.capabilities.maxImageCount);

    VkSwapchainCreateInfoKHR info{};
    info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    info.surface = _surface;
    info.minImageCount = imageCount;
    info.imageFormat = format.format;
    info.imageColorSpace = format.colorSpace;
    info.imageExtent = extent;
    info.imageArrayLayers = 1;
    info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = QueryPhysicalDeviceQueueFamilies(_physicalDevice);
    if (indices.graphicsFamily != indices.presentFamily)
    {
        info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        info.queueFamilyIndexCount = 2;
        std::vector<uint32_t> tmpIndices = { indices.graphicsFamily.value(), indices.presentFamily.value() };
        info.pQueueFamilyIndices = tmpIndices.data();
    }
    else
    {
        info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    info.preTransform = sc.capabilities.currentTransform;
    info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    info.presentMode = presentModel;
    info.clipped = VK_TRUE;
    info.oldSwapchain = nullptr;

    VkResult res = vkCreateSwapchainKHR(_logicalDevice, &info, nullptr, &_swapchain);
    CHECK_SUCCESS(res, "failed to create swap chain!!!")

    vkGetSwapchainImagesKHR(_logicalDevice, _swapchain, &imageCount, nullptr);
    _swapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(_logicalDevice, _swapchain, &imageCount, _swapchainImages.data());
    _swapchainImageFormat = format.format;
    _swapchainExtent = extent;
}

Renderer::SwapChain Renderer::QueryPhysicalDeviceSwapChainSupport(VkPhysicalDevice device)
{
    SwapChain sc;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, _surface, &sc.capabilities);

    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &formatCount, nullptr);
    if(formatCount != 0)
    {
        sc.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface,
            &formatCount,
            sc.formats.data());
    }

    uint32_t presentModelCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &presentModelCount, nullptr);
    if(presentModelCount != 0)
    {
        sc.presentModels.resize(presentModelCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface,
            &presentModelCount,
            sc.presentModels.data());
    }
    return sc;
}

VkSurfaceFormatKHR Renderer::ChooseSwapChainSurfaceFormat(std::vector<VkSurfaceFormatKHR>& formats)
{
    for(const auto & format : formats)
    {
        if(format.format == VK_FORMAT_B8G8R8A8_SRGB &&
            format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return format;
        }
    }
}

VkPresentModeKHR Renderer::ChooseSwapChainPresentModel(std::vector<VkPresentModeKHR>& presentModels)
{
    for(const auto& model : presentModels)
    {
        if(model == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return model;
        }
    }
}

VkExtent2D Renderer::ChooseSwapChainCapbilities(VkSurfaceCapabilitiesKHR capabilities)
{
    //(std::numeric_limits<uint32_t>::max)() avoid window.h max conflict 
    if(capabilities.currentExtent.width != (std::numeric_limits<uint32_t>::max)())
    {
        return capabilities.currentExtent;
    }
    else
    {
        int width = 0;
        int height = 0;
        glfwGetFramebufferSize(_window, &width, &height);

        VkExtent2D extent2D =
        {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };
        extent2D.width = std::clamp(extent2D.width,
            capabilities.minImageExtent.width,
            capabilities.maxImageExtent.width);
        extent2D.height = std::clamp(extent2D.height,
            capabilities.minImageExtent.height,
            capabilities.maxImageExtent.height);
        return extent2D;
    }
}

void Renderer::CreateSurface()
{
    VkWin32SurfaceCreateInfoKHR surfaceInfo{};
    surfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surfaceInfo.hwnd = glfwGetWin32Window(_window);
    surfaceInfo.hinstance = GetModuleHandle(nullptr);

    VkResult res = vkCreateWin32SurfaceKHR(_instance, &surfaceInfo, nullptr, &_surface);
    CHECK_SUCCESS(res, "failed to create win32 surface!!!")
}

void Renderer::SetDebugCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& info)
{
    if(!_enableValidationLayers)
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
        return func(instance, info, allocator, &_debugMessenger);
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
        func(instance, _debugMessenger, allocator);
    }
}

void Renderer::PickPhysicalDevice()
{
    uint32_t physicalDeviceCount = 0;
    vkEnumeratePhysicalDevices(_instance, &physicalDeviceCount, nullptr);
    if (physicalDeviceCount == 0)
    {
        throw std::runtime_error("no find physical device!!!");
    }
    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
    vkEnumeratePhysicalDevices(_instance, &physicalDeviceCount, physicalDevices.data());
    for (auto& physical : physicalDevices)
    {
        if (IsPhysicalDeviceSuitable(physical))
        {
            _physicalDevice = physical;
            break;
        }
    }

    if (_physicalDevice == nullptr)
    {
        throw std::runtime_error("no suitable physical device!!!");
    }
}

bool Renderer::IsPhysicalDeviceSuitable(VkPhysicalDevice device)
{
    QueueFamilyIndices indices = QueryPhysicalDeviceQueueFamilies(device);
    bool support = CheckPhysicalExtensionsSupport(device);
    if (support)
    {
        SwapChain sc = QueryPhysicalDeviceSwapChainSupport(device);
        if (indices.IsComplete() && !sc.formats.empty() && !sc.presentModels.empty())
            return true;
    }
    return false;
}

bool Renderer::CheckPhysicalExtensionsSupport(VkPhysicalDevice device)
{
    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensionProperties(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, extensionProperties.data());
    for(auto extension : _deviceExtensions)
    {
        bool res = false;
        for(auto property : extensionProperties)
        {
            if(std::strcmp(property.extensionName, extension) == 0)
            {
                res = true;
            }
        }

        if(!res)
            return false;
    }

    return true;
}

Renderer::QueueFamilyIndices Renderer::QueryPhysicalDeviceQueueFamilies(VkPhysicalDevice device)
{
    //find graphics queue family
    QueueFamilyIndices indices;
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device,
        &queueFamilyCount, 
        queueFamilyProperties.data());
    for (int i = 0; i < queueFamilyProperties.size(); i++)
    {
        VkQueueFamilyProperties properties = queueFamilyProperties.at(i);
        if (properties.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphicsFamily = i;
            if(indices.presentFamily.has_value())
                break;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, _surface, &presentSupport);
        if(presentSupport)
        {
            indices.presentFamily = i;
            if(indices.graphicsFamily.has_value())
                break;
        }
    }
    return indices;
}

void Renderer::CreateLogicalDevice()
{
    QueueFamilyIndices indices = QueryPhysicalDeviceQueueFamilies(_physicalDevice);
    std::set<uint32_t> queueIndices = { indices.graphicsFamily.value(),
                                        indices.presentFamily.value()};
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfoList;
    for(const unsigned int& indice : queueIndices)
    {
        VkDeviceQueueCreateInfo queueInfo{};
        queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueInfo.queueCount = 1;
        queueInfo.queueFamilyIndex = indices.graphicsFamily.value();
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
    info.enabledExtensionCount = static_cast<uint32_t>(_deviceExtensions.size());
    info.ppEnabledExtensionNames = _deviceExtensions.data();

    VkResult res = vkCreateDevice(_physicalDevice, &info, nullptr, &_logicalDevice);
    CHECK_SUCCESS(res, "can't to create logical device!!!");


    vkGetDeviceQueue(_logicalDevice,
        indices.graphicsFamily.value(),
        0,
        &_queueGraphics);

    vkGetDeviceQueue(_logicalDevice,
        indices.presentFamily.value(),
        0,
        &_queuePresent);
}



void Renderer::Run()
{
    InitWindow();
    CheckValidationLayerSupport();
    InitVulkan();
    MainLoop();
    Cleanup();
}

void Renderer::Cleanup()
{
    for (auto& imageView : _imageViews)
    {
        vkDestroyImageView(_logicalDevice, imageView, nullptr);
    }
    vkDestroySwapchainKHR(_logicalDevice, _swapchain, nullptr);
    vkDestroySurfaceKHR(_instance, _surface, nullptr);
    vkDestroyDevice(_logicalDevice, nullptr);
    if(_enableValidationLayers)
        DestoryDebugUtilsMessengerEXT(_instance, nullptr);
    vkDestroyInstance(_instance, nullptr);
    glfwDestroyWindow(_window);
    glfwTerminate();
}

void Renderer::CreateValidationLayer()
{
    VkDebugUtilsMessengerCreateInfoEXT info{};
    SetDebugCreateInfo(info);
    VkResult res = CreateDebugUtilsMessengerEXT(_instance, &info, nullptr);
    CHECK_SUCCESS(res, "failed to create debug messenger!!!")
}

void Renderer::CreateImageViews()
{
    _imageViews.resize(_swapchainImages.size());
    for (int i = 0; i < _swapchainImages.size(); i++)
    {
        VkImageViewCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        info.image = _swapchainImages[i];
        info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        info.format = _swapchainImageFormat;
        info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        info.subresourceRange.baseMipLevel = 0;
        info.subresourceRange.levelCount = 1;
        info.subresourceRange.baseArrayLayer = 0;
        info.subresourceRange.layerCount = 1;
        VkResult res = vkCreateImageView(_logicalDevice, &info, nullptr, &_imageViews[i]);
        CHECK_SUCCESS(res, "failed to create image view!!!")
    }
}

void Renderer::CreateGeaphicsPipline()
{

}