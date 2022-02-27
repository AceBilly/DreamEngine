module;
#include <vulkan/vulkan.h>
#include <Windows.h>
#include <string>
export module VulkanUtility;
// 处理vk相关的返回值
export
void handleVulkanError(VkResult res);
// 将VkPhysicalDeviceType 枚举类型转换为str
export
std::string getVkPhysicalDeviceTypeStr(VkPhysicalDeviceType type);
export
struct VkSwapChainBuffer {
    VkImage image;
    VkImageView view;
    VkFramebuffer frameBuffer;
};
module : private;
void exitOnError(std::wstring errMsg) {
    MessageBox(nullptr, errMsg.c_str(), L"vulkan engine", MB_ICONERROR);
    exit(EXIT_FAILURE);
}
void handleVulkanError(VkResult res) {
    if (res == VK_ERROR_INCOMPATIBLE_DRIVER) {
        exitOnError(
            L"Cannot find a compatible Vulkan installable client "
            L"driver (ICD). Please make sure your driver supports "
            L"Vulkan before continuing. The call to vkCreateInstance failed.");
    }
    else if (res != VK_SUCCESS) {
        exitOnError(
            L"The call to vkCreateInstance failed. Please make sure "
            L"you have a Vulkan installable client driver (ICD) before "
            L"continuing.");
    }
}

std::string getVkPhysicalDeviceTypeStr(VkPhysicalDeviceType type)
{
    switch (type) {
    case VK_PHYSICAL_DEVICE_TYPE_OTHER:
        return "VK_PHYSICAL_DEVICE_TYPE_CPU";
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
        return "VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU";
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
        return "VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU";
    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
        return "VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU";
    case VK_PHYSICAL_DEVICE_TYPE_CPU:
        return "VK_PHYSICAL_DEVICE_TYPE_CPU";
    }
    return std::string("UNKONWN");
}

