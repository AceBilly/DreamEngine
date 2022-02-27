module;
#define VK_PROTOTYPES
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include <iostream>
#include <format>
#include <cassert>
#include <filesystem>

export module vulkanRenderEngine;
import RenderEngineInterface;
import VulkanUtility;
import common;

export class VulkanRenderEngine
	: public RenderEngineInterface {
public:
	VulkanRenderEngine(UINT width, UINT height, HWND m_hwnd, HINSTANCE hInstance);
	virtual ~VulkanRenderEngine();
public:
	// 初始化vulkan obj
	void init() override;

public:
	void logPhysicalDevice();
private:
	// 创建vkInstance 实例
	void createVKInstance();
	void createDevice();
	// 枚举物理设备
	void enumPhysicalDevices();
	// 绑定窗口
	void bindWindow();
	// 创建命令队列
	void createVkQueue();
	void getColorFormat();
	void createSwapChain();
	// 为VkImage 创建 ImageView
	void createImageViews();
	// 创建渲染缓冲对象
	void createRenderPass();
	// 创建图形管线
	void createGraphicsPipeline();
	// 创建vulkan shader module
	VkShaderModule createShaderModule(const std::vector<char>& shaderCode);
	void createFrameBuffer();
	// 创建命令池 并 分配命令缓冲区
	void createCommandPool();
	void allocateCommandBuffer();
	// 回收资源
	void cleanUp();
private:
	// pipeline state obj
	VkInstance m_vkInstance;
	VkDevice m_vkDevice;
	UINT m_physicalDeviceCount;
	std::vector<VkPhysicalDevice> m_physicalDevice;
	float m_queuePriorities[1]{1.0f};  // 队列优先级
	VkSurfaceKHR m_surface;

	VkExtent2D m_swapChainBufferSize;
	VkSwapchainKHR m_swapChain;
	UINT m_swapChainImageCount;  // swap chain image num
	std::vector<VkImage> m_swapChainImage;
	std::vector<VkImageView> m_swapChainImageView;

	VkRenderPass m_renderPass;
	VkPipelineLayout m_pipelineLayout;  // 控制着色器变量
	VkPipeline m_pipeline;

	UINT m_queueCount;
	std::vector<VkQueueFamilyProperties> m_queueFamilyProperties;
	// 支持颜色格式
	UINT m_surfaceFormatCount;
	std::vector<VkSurfaceFormatKHR> m_surfaceFormat;
	
	VkFormat m_colorFormat;
	VkColorSpaceKHR m_colorSpace;

	// 帧缓冲
	std::vector<VkFramebuffer> m_frameBuffer;

	VkCommandPool m_commandPool;
	std::vector<VkCommandBuffer> m_commandBuffer;
	// custom obj;
	std::string m_appName;
	UINT m_currentDevice = 0; // 当前正在使用的设备
	UINT32 m_currentQueueIndex = UINT32_MAX;  // 当前队列index
	std::vector<VkBool32> m_supportPresenting;

};

module : private;

void VulkanRenderEngine::createSwapChain() {
	VkSurfaceCapabilitiesKHR surfaceCapabilities{};
	handleVulkanError(
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice[m_currentDevice], m_surface,
			&surfaceCapabilities)
	);
	
	if (surfaceCapabilities.currentExtent.width == -1 || surfaceCapabilities.currentExtent.height == -1) {
		m_swapChainBufferSize.height = m_height;
		m_swapChainBufferSize.width = m_width;
	}
	else {
		m_swapChainBufferSize = surfaceCapabilities.currentExtent;
	}
	
	UINT presentModeCount = 0;
	handleVulkanError(
	vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice[m_currentDevice],
		m_surface, &presentModeCount, nullptr)
	);
	assert(presentModeCount > 0);
	std::vector<VkPresentModeKHR> presentMode(presentModeCount);
	handleVulkanError(
		vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice[m_currentDevice],
			m_surface, &presentModeCount, presentMode.data())
	);
	VkPresentModeKHR presentModeTmp = VK_PRESENT_MODE_FIFO_KHR;
	for (UINT i = 0; i < presentModeCount; ++i) {
		if (presentMode[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
			presentModeTmp = VK_PRESENT_MODE_MAILBOX_KHR;
			break;
		}
		if (presentMode[i] == VK_PRESENT_MODE_IMMEDIATE_KHR) {
			presentModeTmp = VK_PRESENT_MODE_IMMEDIATE_KHR;
		}
	}
	
	// create swap chain
	{
		assert(surfaceCapabilities.maxImageCount > 0);
		VkSwapchainCreateInfoKHR swapChainInfo{
			.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
			.pNext = nullptr,
			.flags = 0,
			.surface = m_surface,
			.minImageCount =
			(surfaceCapabilities.minImageCount + 1 > surfaceCapabilities.maxImageCount)
			? surfaceCapabilities.minImageCount + 1 : surfaceCapabilities.maxImageCount,
			.imageFormat = m_colorFormat,
			.imageColorSpace = m_colorSpace,
			.imageExtent = m_swapChainBufferSize,
			.imageArrayLayers = 1,
			.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.queueFamilyIndexCount = 1,
			.pQueueFamilyIndices = { 0 },
			.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
			.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			.presentMode = presentModeTmp
		};
		handleVulkanError(
			vkCreateSwapchainKHR(m_vkDevice, &swapChainInfo, nullptr, &m_swapChain)
		);
	}
	// 获取交换链中的图片
	handleVulkanError(
		vkGetSwapchainImagesKHR(m_vkDevice, m_swapChain, &m_swapChainImageCount, nullptr)
	);
	assert(m_swapChainImageCount > 0);
	m_swapChainImage.resize(m_swapChainImageCount);
	handleVulkanError(
		vkGetSwapchainImagesKHR(m_vkDevice, m_swapChain, &m_swapChainImageCount, m_swapChainImage.data())
	);
}

void VulkanRenderEngine::createImageViews() {
	m_swapChainImageView.resize(m_swapChainImage.size());  // 分配足够的空间

	VkImageViewCreateInfo imageViewCreateInfo {
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.pNext = nullptr,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = m_colorFormat,
		.components = {
			.r = VK_COMPONENT_SWIZZLE_IDENTITY,
			.g = VK_COMPONENT_SWIZZLE_IDENTITY,
			.b = VK_COMPONENT_SWIZZLE_IDENTITY,
			.a = VK_COMPONENT_SWIZZLE_IDENTITY},
		.subresourceRange = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1}
	};

	for (UINT i = 0; i < m_swapChainImage.size(); ++i) {
		imageViewCreateInfo.image = m_swapChainImage[i];
		handleVulkanError(
			vkCreateImageView(m_vkDevice, &imageViewCreateInfo, nullptr, &m_swapChainImageView[i])
		);
	}

}

void VulkanRenderEngine::createRenderPass() {
	VkAttachmentDescription colorAttachment{
		.format = m_colorFormat,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
	};
	// muti pass
	VkAttachmentReference colorAttachmentRef{
		.attachment = 0,
		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	};
	VkSubpassDescription subPass{
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.colorAttachmentCount = 1,
		.pColorAttachments = &colorAttachmentRef,
	};
	VkRenderPassCreateInfo renderPassCreateInfo{
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.attachmentCount = 1,
		.pAttachments = &colorAttachment,
		.subpassCount = 1,
		.pSubpasses = &subPass
	};
	handleVulkanError(
		vkCreateRenderPass(m_vkDevice, &renderPassCreateInfo, nullptr, &m_renderPass)
	);

}

void VulkanRenderEngine::createGraphicsPipeline() {
	auto vertexShaderCode = readShaderFile(std::filesystem::path("shaders/vulkanTestVertexShader.hlsl"));
	auto fragmentShaderCode = readShaderFile(std::filesystem::path("shaders/vulkanTestFragmentShader.hlsl"));
	VkShaderModule vertexShaderModule = createShaderModule(vertexShaderCode);
	VkShaderModule fragmentShaderModule = createShaderModule(fragmentShaderCode);
	// 可编程着色器配置
	// TODO: 动态
	VkPipelineShaderStageCreateInfo pipelineVertexStageCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.pNext = nullptr,
		.stage = VK_SHADER_STAGE_VERTEX_BIT,
		.module = vertexShaderModule,
		.pName = "main"
	};
	VkPipelineShaderStageCreateInfo pipelineFragmentStageCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.pNext = nullptr,
		.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
		.module = fragmentShaderModule,
		.pName = "main"
	};
	VkPipelineShaderStageCreateInfo shaderStageConfig[] = { pipelineVertexStageCreateInfo, pipelineFragmentStageCreateInfo };

	// 固定着色器
	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.pNext = nullptr,
		.vertexBindingDescriptionCount = 0,
		.pVertexBindingDescriptions = nullptr,
		.vertexAttributeDescriptionCount = 0,
		.pVertexAttributeDescriptions = nullptr
	};
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.pNext = nullptr,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.primitiveRestartEnable = VK_FALSE
	};
	VkViewport viewport{
		.x = 0,.y = 0,
		.width = m_swapChainBufferSize.width, .height = m_swapChainBufferSize.height,
		.minDepth = 0.0f, .maxDepth = 1.0f
	};
	// 裁剪空间大小
	VkRect2D scissor{
		.offset = {0, 0},
		.extent = m_swapChainBufferSize
	};
	VkPipelineViewportStateCreateInfo viewPortCreateInfo{
		.viewportCount = 1,.pViewports = &viewport,
		.scissorCount = 1, .pScissors = &scissor
	};
	VkPipelineRasterizationStateCreateInfo rasterizer{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.pNext = nullptr,
		.depthClampEnable = VK_FALSE,
		.rasterizerDiscardEnable = VK_FALSE,
		.polygonMode = VK_POLYGON_MODE_FILL,
		.cullMode = VK_CULL_MODE_BACK_BIT,
		.frontFace = VK_FRONT_FACE_CLOCKWISE,
		.depthBiasEnable = VK_FALSE,
		.lineWidth = 1.0f,
	};
	// 反走样
	VkPipelineMultisampleStateCreateInfo mutiSampleCreateInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.pNext = nullptr,
		.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
		.minSampleShading = 1.f
	};
	// 颜色混合
	VkPipelineColorBlendAttachmentState colorBlendAttachment{
		.blendEnable = VK_FALSE,
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT		
	};
	VkPipelineColorBlendStateCreateInfo colorBlendingCreateInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.logicOpEnable = VK_FALSE,
		.attachmentCount = 1,
		.pAttachments = &colorBlendAttachment
	};
	// 动态管线状态
	VkDynamicState dynamicStates[] { 
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_LINE_WIDTH
	};
	VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.dynamicStateCount = 2,.pDynamicStates = dynamicStates
	};
	// 与着色器变量通信 空布局
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO
		// ignore 
	};
	handleVulkanError(
		vkCreatePipelineLayout(m_vkDevice, &pipelineLayoutCreateInfo, nullptr, &m_pipelineLayout)
	);
	VkGraphicsPipelineCreateInfo graphicsPipeCreateInfo{
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.stageCount = 2,
		.pStages = shaderStageConfig,
		.pVertexInputState = &vertexInputInfo,
		.pInputAssemblyState = &inputAssemblyInfo,
		.pViewportState = &viewPortCreateInfo,
		.pRasterizationState = &rasterizer,
		.pMultisampleState = &mutiSampleCreateInfo,
		.pDepthStencilState = nullptr,
		.pColorBlendState = &colorBlendingCreateInfo,
		.pDynamicState = nullptr,
		.layout = m_pipelineLayout,
		.renderPass = m_renderPass,
		.subpass = 0,
		.basePipelineHandle = VK_NULL_HANDLE,
		.basePipelineIndex = -1
	};
	handleVulkanError(
		vkCreateGraphicsPipelines(m_vkDevice, VK_NULL_HANDLE, 1, &graphicsPipeCreateInfo, nullptr, &m_pipeline)
	);
	vkDestroyShaderModule(m_vkDevice, vertexShaderModule, nullptr);
	vkDestroyShaderModule(m_vkDevice, fragmentShaderModule, nullptr);
}

VkShaderModule VulkanRenderEngine::createShaderModule(const std::vector<char>& shaderCode) {
	VkShaderModuleCreateInfo shaderModuleCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = shaderCode.size(),
		.pCode = reinterpret_cast<const UINT32*>(shaderCode.data())
	};
	VkShaderModule shaderModule;
	handleVulkanError(
		vkCreateShaderModule(m_vkDevice, &shaderModuleCreateInfo, nullptr, &shaderModule)
	);
	return shaderModule;
}

void VulkanRenderEngine::createFrameBuffer() {
	m_frameBuffer.resize(m_swapChainImageView.size());
	VkFramebufferCreateInfo frameBufferCreateInfo{
		.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
		.renderPass = m_renderPass,
		.attachmentCount = 1,
		.width = m_swapChainBufferSize.width, .height = m_swapChainBufferSize.height,
		.layers = 1
	};
	for (UINT i = 0; i < m_swapChainImageView.size(); ++i) {
		VkImageView attachments[] = { m_swapChainImageView[i] };
		frameBufferCreateInfo.pAttachments = attachments;
		handleVulkanError(
			vkCreateFramebuffer(m_vkDevice, &frameBufferCreateInfo, nullptr, &m_frameBuffer[i])
		);
	}
}

void VulkanRenderEngine::createCommandPool()
{
	VkCommandPoolCreateInfo poolInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.queueFamilyIndex = m_currentQueueIndex,
	};
	handleVulkanError(
		vkCreateCommandPool(m_vkDevice, &poolInfo, nullptr, &m_commandPool)
	);
}

void VulkanRenderEngine::allocateCommandBuffer()
{
	m_commandBuffer.resize(m_frameBuffer.size());
	VkCommandBufferAllocateInfo commandBufferInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = m_commandPool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = (UINT32)m_commandBuffer.size(),
	};
	handleVulkanError(
		vkAllocateCommandBuffers(m_vkDevice, &commandBufferInfo, m_commandBuffer.data())
	);
}

void VulkanRenderEngine::cleanUp() {
	// 清除ImageView
	for (auto imageView : m_swapChainImageView) {
		vkDestroyImageView(m_vkDevice, imageView, nullptr);
	}
	// 释放layout
	vkDestroyPipelineLayout(m_vkDevice, m_pipelineLayout, nullptr);
	// 清除 renderpass
	vkDestroyRenderPass(m_vkDevice, m_renderPass, nullptr);
	// 释放管线
	vkDestroyPipeline(m_vkDevice, m_pipeline, nullptr);
	// 清除帧缓冲
	for (auto frameBuffer : m_frameBuffer) {
		vkDestroyFramebuffer(m_vkDevice, frameBuffer, nullptr);
	}
	// 清除command pool
	vkDestroyCommandPool(m_vkDevice, m_commandPool, nullptr);
}

void VulkanRenderEngine::getColorFormat() {
	handleVulkanError(
		vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice[m_currentDevice], m_surface, &m_surfaceFormatCount, nullptr)
	);
	assert(m_surfaceFormatCount > 0);
	m_surfaceFormat.resize(m_surfaceFormatCount);
	handleVulkanError(
		vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice[m_currentDevice], m_surface, &m_surfaceFormatCount, m_surfaceFormat.data())
	);
	if (m_surfaceFormatCount == 1 && m_surfaceFormat[0].format == VK_FORMAT_UNDEFINED) {
		m_colorFormat = VK_FORMAT_B8G8R8A8_UNORM;
	}
	else
		m_colorFormat = m_surfaceFormat[0].format;

	m_colorSpace = m_surfaceFormat[0].colorSpace;
}

void VulkanRenderEngine::createVkQueue() {
	vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice[m_currentDevice], &m_queueCount, nullptr);
	assert(m_queueCount > 0);
	m_queueFamilyProperties.resize(m_queueCount);
	vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice[m_currentDevice], &m_queueCount, m_queueFamilyProperties.data());
	m_supportPresenting.resize(m_queueCount);
	for (UINT i = 0; i < m_queueCount; ++i) {
		vkGetPhysicalDeviceSurfaceSupportKHR(m_physicalDevice[m_currentDevice], i, m_surface, &m_supportPresenting[i]);
		if ((m_queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
			if (m_supportPresenting[i] == VK_TRUE) {
				m_currentQueueIndex = i;
				break;
			}
		}
	}
	assert(m_currentQueueIndex != UINT32_MAX);
}
void VulkanRenderEngine::bindWindow() {
	VkWin32SurfaceCreateInfoKHR surfaceInfo = {
		.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
		.pNext = nullptr,
		.flags = 0,
		.hinstance = m_appHinstance,
		.hwnd = m_windowHandle
	};
	handleVulkanError(
		vkCreateWin32SurfaceKHR(m_vkInstance, &surfaceInfo, nullptr, &m_surface)
	);
}
VulkanRenderEngine::VulkanRenderEngine(UINT width, UINT height, HWND hwnd, HINSTANCE hInstance)
:RenderEngineInterface(height, width, hwnd, hInstance) {

}
void VulkanRenderEngine::createDevice() {
	VkDeviceQueueCreateInfo deviceQueueInfo = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.queueFamilyIndex = 0,
		.queueCount = 1,
		.pQueuePriorities = &(m_queuePriorities[0])
	};

	std::vector<const char*> enableExtension = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	VkDeviceCreateInfo deviceInfo = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.queueCreateInfoCount = 1,
		.pQueueCreateInfos = &deviceQueueInfo,
		.enabledExtensionCount = (UINT32)enableExtension.size(),
		.ppEnabledExtensionNames = enableExtension.data(),
		.pEnabledFeatures = nullptr
	};
	handleVulkanError(
		vkCreateDevice(m_physicalDevice[m_currentDevice], &deviceInfo, nullptr, &m_vkDevice)
	);
}
void VulkanRenderEngine::enumPhysicalDevices() {
	handleVulkanError(
		vkEnumeratePhysicalDevices(m_vkInstance, &m_physicalDeviceCount, nullptr)
	);
	if (m_physicalDeviceCount > 0) {
		m_physicalDevice.resize(m_physicalDeviceCount);
		handleVulkanError(
			vkEnumeratePhysicalDevices(m_vkInstance, &m_physicalDeviceCount, m_physicalDevice.data())
		);
	}
}
void VulkanRenderEngine::logPhysicalDevice() {
	assert(m_physicalDeviceCount > 0);
	const std::string fmtStr = 
		"Device name: {}\n"
		"Device type: {}\n"
		"Driver version: {}\n";
	
	VkPhysicalDeviceProperties deviceProperties{};
	for (UINT i = 0; i > 0; ++i) {
		vkGetPhysicalDeviceProperties(m_physicalDevice[i], &deviceProperties);
		std::cout << std::format(fmtStr, deviceProperties.deviceName,
			getVkPhysicalDeviceTypeStr(deviceProperties.deviceType), deviceProperties.driverVersion);
	}
}

VulkanRenderEngine::~VulkanRenderEngine()
{
	vkDestroyInstance(m_vkInstance, nullptr);
}

void VulkanRenderEngine::init() {
	createVKInstance();
	// ----
	bindWindow();
	createVkQueue();
	// 获取设备支持的颜色空间以及色欲
	getColorFormat();
	createSwapChain(); // 初始化vk 交换链
	createImageViews();
	createRenderPass();
	createGraphicsPipeline();
	createFrameBuffer();
	createCommandPool();
	allocateCommandBuffer();
}

void VulkanRenderEngine::createVKInstance(){
	VkApplicationInfo vkAppInfo = {
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pNext = nullptr,
		.pApplicationName = m_appName.c_str(),
		.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
		.pEngineName = "No Engine", // 不知道是做什么用的 暂时不讨论
		.engineVersion = VK_MAKE_VERSION(1, 0, 0),
		.apiVersion = VK_API_VERSION_1_2
	};
	std::vector<const char*> enableExtension{ VK_KHR_SURFACE_EXTENSION_NAME };
#if defined(_WIN32)
	enableExtension.push_back( VK_KHR_WIN32_SURFACE_EXTENSION_NAME );
#endif
	VkInstanceCreateInfo createInfo = {
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.pApplicationInfo = &vkAppInfo,
		.enabledExtensionCount = (UINT32)enableExtension.size(),
		.ppEnabledExtensionNames = enableExtension.data(),
	};
	handleVulkanError(vkCreateInstance(&createInfo, nullptr, &m_vkInstance));
}
