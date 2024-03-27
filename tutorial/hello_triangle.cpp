//
// Created by Junhao Wang (@forkercat) on 3/9/24.
//

#include "hello_triangle.h"

#include "shader.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <algorithm>
#include <chrono>

static const U32 kWidth = 800;
static const U32 kHeight = 600;

struct Vertex
{
	glm::vec2 position;
	glm::vec3 color;

	static VkVertexInputBindingDescription GetBindingDescription()
	{
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 2> GetAttributeDescriptions()
	{
		std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, position);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);
		return attributeDescriptions;
	}
};

struct UniformBufferObject
{
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

static const std::vector<Vertex> kVertexData = { { { -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f } },
												 { { 0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f } },
												 { { 0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f } },
												 { { -0.5f, 0.5f }, { 1.0f, 1.0f, 1.0f } } };

static const std::vector<U16> kIndexData = { 0, 1, 2, 2, 3, 0 };

static const std::vector<const char*> kValidationLayers = { "VK_LAYER_KHRONOS_validation" };
static const std::vector<const char*> kDeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME,
															"VK_KHR_portability_subset" };

struct QueueFamilyIndices
{
	std::optional<U32> graphicsFamily;
	std::optional<U32> presentFamily;

	bool IsComplete() { return graphicsFamily.has_value() && presentFamily.has_value(); }
};

struct SwapchainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

#ifdef NDEBUG
static const bool kEnableValidationLayers = false;
#else
static const bool kEnableValidationLayers = true;
#endif

VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
													  VkDebugUtilsMessageTypeFlagsEXT messageType,
													  const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
													  void* pUserData)
{
	DEBUG("Validation Output: %s", pCallbackData->pMessage);
	return VK_FALSE;  // Original Vulkan call is not aborted
}

// Proxy functions to create and destroy debug messenger.
VkResult CreateDebugUtilsMessengerEXT(VkInstance, const VkDebugUtilsMessengerCreateInfoEXT*,
									  const VkAllocationCallbacks*, VkDebugUtilsMessengerEXT*);
void DestroyDebugUtilsMessengerEXT(VkInstance, VkDebugUtilsMessengerEXT, const VkAllocationCallbacks*);
void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT&);

bool CheckDeviceExtensionSupport(VkPhysicalDevice);
SwapchainSupportDetails QuerySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);

/////////////////////////////////////////////////////////////////////////////////

void HelloTriangleApplication::Run()
{
	InitWindow();
	InitVulkan();
	MainLoop();
	CleanUp();
}

void HelloTriangleApplication::InitWindow()
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	mWindow = glfwCreateWindow(kWidth, kHeight, "Vulkan HelloTriangle", nullptr, nullptr);
	glfwSetWindowUserPointer(mWindow, this);
	glfwSetFramebufferSizeCallback(mWindow, FramebufferResizeCallback);

	U32 extensionCount{};
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> supportedExtensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, supportedExtensions.data());

	PRINT("Vulkan supported extension count: %u", extensionCount);
	std::string supportedExtensionsOutput = "Vulkan supported extensions: ";
	for (const auto& extensionProperties : supportedExtensions)
	{
		supportedExtensionsOutput += std::string(extensionProperties.extensionName) + " ";
	}
	PRINT("%s", supportedExtensionsOutput.c_str());
	NEWLINE("---------------------------------");
}

void HelloTriangleApplication::InitVulkan()
{
	CreateInstance();
	SetupDebugMessenger();

	CreateWindowSurface();
	PickPhysicalDevice();
	CreateLogicalDeviceAndQueues();

	CreateSwapchain();
	CreateImageViews();
	CreateRenderPass();
	CreateDescriptorSetLayout();
	CreateGraphicsPipeline();
	CreateFramebuffers();

	CreateCommandPool();
	CreateTextureImage();
	CreateTextureImageView();
	CreateTextureSampler();

	CreateVertexBuffer();
	CreateIndexBuffer();

	CreateUniformBuffers();
	CreateDescriptorPool();
	CreateDescriptorSets();

	CreateCommandBuffers();

	CreateSyncObjects();
}

void HelloTriangleApplication::CreateInstance()
{
	ERROR_IF(kEnableValidationLayers && !CheckValidationLayerSupport(),
			 "Validation layers requested, but not available!");

	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Hello Triangle";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	// Example: VK_KHR_surface, VK_EXT_metal_surface, VK_KHR_portability_enumeration
	std::vector<const char*> requiredExtensions = GetRequiredInstanceExtensions();
	createInfo.enabledExtensionCount = static_cast<U32>(requiredExtensions.size());
	createInfo.ppEnabledExtensionNames = requiredExtensions.data();

	// Additional settings for macOS, otherwise you would get VK_ERROR_INCOMPATIBLE_DRIVER.
	createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

	// Placed outside if for longer lifecycle before instance will be created.
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};

	if (kEnableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<U32>(kValidationLayers.size());
		createInfo.ppEnabledLayerNames = kValidationLayers.data();

		// Needed for debugging issues in the vkCreateInstance and vkDestroyInstance calls.
		PopulateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = static_cast<VkDebugUtilsMessengerCreateInfoEXT*>(&debugCreateInfo);
	}
	else
	{
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
	}

	VkResult result = vkCreateInstance(&createInfo, nullptr, &mInstance);
	ASSERT_EQ(result, VK_SUCCESS, "Failed to create Vulkan instance!");
}

std::vector<const char*> HelloTriangleApplication::GetRequiredInstanceExtensions()
{
	U32 glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (kEnableValidationLayers)
	{
		extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	// Additional settings for macOS, otherwise you would get VK_ERROR_INCOMPATIBLE_DRIVER.
	extensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);

	// Fixing the device error on macOS.
	extensions.emplace_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

	return extensions;
}

void HelloTriangleApplication::SetupDebugMessenger()
{
	if (kEnableValidationLayers)
	{
		VkDebugUtilsMessengerCreateInfoEXT createInfo{};
		PopulateDebugMessengerCreateInfo(createInfo);

		VkResult result = CreateDebugUtilsMessengerEXT(mInstance, &createInfo, nullptr, &mDebugMessenger);
		ASSERT_EQ(result, VK_SUCCESS, "Failed to set up debug messenger!");
	}
}

void HelloTriangleApplication::CreateWindowSurface()
{
	VkResult result = glfwCreateWindowSurface(mInstance, mWindow, nullptr, &mSurface);
	ASSERT_EQ(result, VK_SUCCESS, "Failed to create window surface!");
}

void HelloTriangleApplication::PickPhysicalDevice()
{
	U32 deviceCount{};
	vkEnumeratePhysicalDevices(mInstance, &deviceCount, nullptr);

	ASSERT_NEQ(deviceCount, 0, "Failed to find GPUs with Vulkan support!");

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(mInstance, &deviceCount, devices.data());

	for (const auto& device : devices)
	{
		if (IsPhysicalDeviceSuitable(device))
		{
			mPhysicalDevice = device;
			break;
		}
	}

	ASSERT_NEQ(mPhysicalDevice, VK_NULL_HANDLE, "Failed to find a suitable GPU device!");
}

void HelloTriangleApplication::CreateLogicalDeviceAndQueues()
{
	PRINT("Creating logical device and queues...");

	// Queue families
	QueueFamilyIndices queueFamilyData = FindQueueFamilies(mPhysicalDevice);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<U32> uniqueQueueFamilies = { queueFamilyData.graphicsFamily.value(),
										  queueFamilyData.presentFamily.value() };
	// If the queue families are the same, then we only need to pass its index once.

	F32 queuePriority = 1.0f;

	for (U32 queueFamilyIndex : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	// Features
	VkPhysicalDeviceFeatures deviceFeatures{};
	deviceFeatures.samplerAnisotropy = VK_TRUE;

	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = static_cast<U32>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.pEnabledFeatures = &deviceFeatures;

	createInfo.enabledExtensionCount = static_cast<U32>(kDeviceExtensions.size());
	createInfo.ppEnabledExtensionNames = kDeviceExtensions.data();	// e.g. swap chain

	if (kEnableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<U32>(kValidationLayers.size());
		createInfo.ppEnabledLayerNames = kValidationLayers.data();
	}
	else
	{
		createInfo.enabledLayerCount = 0;
	}

	VkResult result = vkCreateDevice(mPhysicalDevice, &createInfo, nullptr, &mDevice);
	ASSERT_EQ(result, VK_SUCCESS, "Failed to create logical device!");

	// Fetch queue handle.
	vkGetDeviceQueue(mDevice, queueFamilyData.graphicsFamily.value(), 0, &mGraphicsQueue);
	vkGetDeviceQueue(mDevice, queueFamilyData.presentFamily.value(), 0, &mPresentQueue);
}

void HelloTriangleApplication::RecreateSwapchain()
{
	// Note that we don't recreate the render pass here for simplicity. In theory, it can be possible for the swapchain
	// image format to change, e.g. when moving a window from a standard range to a high dynamic range monitor.

	// Handle window minimization.
	int width = 0;
	int height = 0;
	glfwGetFramebufferSize(mWindow, &width, &height);

	while (width == 0 || height == 0)
	{
		glfwGetFramebufferSize(mWindow, &width, &height);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(mDevice);

	// It is possible to create a new swapchain while drawing commands on an image from the old swapchain are still
	// in-flight. You need to pass the previous swapchain to the oldSwapChain field in create info.
	CleanUpSwapchain();

	CreateSwapchain();
	CreateImageViews();
	CreateFramebuffers();  // Functions that depend on the swapchain
}

void HelloTriangleApplication::CleanUpSwapchain()
{
	for (USize i = 0; i < mSwapchainFramebuffers.size(); i++)
	{
		vkDestroyFramebuffer(mDevice, mSwapchainFramebuffers[i], nullptr);
	}

	for (USize i = 0; i < mSwapchainImageViews.size(); i++)
	{
		vkDestroyImageView(mDevice, mSwapchainImageViews[i], nullptr);
	}

	vkDestroySwapchainKHR(mDevice, mSwapchain, nullptr);
}

void HelloTriangleApplication::CreateSwapchain()
{
	PRINT("Creating swapchain...");
	SwapchainSupportDetails swapchainSupport = QuerySwapchainSupport(mPhysicalDevice, mSurface);

	VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapchainSupport.formats);
	VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapchainSupport.presentModes);
	VkExtent2D extent2D = ChooseSwapExtent(mWindow, swapchainSupport.capabilities);

	// It is recommended to request at least one more image than the minimum.
	U32 imageCount = swapchainSupport.capabilities.minImageCount + 1;

	if (swapchainSupport.capabilities.maxImageCount > 0)
	{
		imageCount = std::min(imageCount, swapchainSupport.capabilities.maxImageCount);
	}

	PRINT("Image count: %u", imageCount);

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = mSurface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent2D;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	// Specify how to handle swapchain images that will be used across multiple queue families.
	// E.g. Drawing on the images in the swap chain from the graphics queue and then submitting
	// them on the presentation queue.
	QueueFamilyIndices familyIndices = FindQueueFamilies(mPhysicalDevice);
	U32 queueFamilyIndices[] = { familyIndices.graphicsFamily.value(), familyIndices.presentFamily.value() };

	if (familyIndices.graphicsFamily != familyIndices.presentFamily)
	{
		// Graphics and presentation queue families differ.
		// Images should be used across multiple queue families without explicit ownership transfers.
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;	   // Optional
		createInfo.pQueueFamilyIndices = nullptr;  // Optional
	}

	createInfo.preTransform = swapchainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	VkResult result = vkCreateSwapchainKHR(mDevice, &createInfo, nullptr, &mSwapchain);
	ASSERT_EQ(result, VK_SUCCESS, "Failed to create swapchain!");

	// Retrieve images.
	vkGetSwapchainImagesKHR(mDevice, mSwapchain, &imageCount, nullptr);
	mSwapchainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(mDevice, mSwapchain, &imageCount, mSwapchainImages.data());

	mSwapchainImageFormat = surfaceFormat.format;
	mSwapchainExtent = extent2D;
}

void HelloTriangleApplication::CreateImageViews()
{
	mSwapchainImageViews.resize(mSwapchainImages.size());

	for (USize i = 0; i < mSwapchainImages.size(); i++)
	{
		mSwapchainImageViews[i] = CreateImageView(mSwapchainImages[i], mSwapchainImageFormat);
	}
}

VkImageView HelloTriangleApplication::CreateImageView(VkImage image, VkFormat format)
{
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	VkImageView imageView;
	VkResult result = vkCreateImageView(mDevice, &viewInfo, nullptr, &imageView);
	ASSERT_EQ(result, VK_SUCCESS, "Failed to create image view!");

	return imageView;
}

VkSurfaceFormatKHR HelloTriangleApplication::ChooseSwapSurfaceFormat(
	const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	auto it = std::find_if(availableFormats.begin(), availableFormats.end(), [](VkSurfaceFormatKHR surfaceFormat) {
		return surfaceFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
			   surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	});

	if (it != availableFormats.end())
	{
		return *it;
	}

	ASSERT(!availableFormats.empty(), "Failed to pick available format!");
	return availableFormats[0];
}

VkPresentModeKHR HelloTriangleApplication::ChooseSwapPresentMode(
	const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	auto it = std::find_if(availablePresentModes.begin(), availablePresentModes.end(),
						   [](VkPresentModeKHR presentMode) { return presentMode == VK_PRESENT_MODE_MAILBOX_KHR; });

	return (it != availablePresentModes.end()) ? *it : VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D HelloTriangleApplication::ChooseSwapExtent(GLFWwindow* window, const VkSurfaceCapabilitiesKHR& capabilities)
{
	if (capabilities.currentExtent.width != std::numeric_limits<U32>::max())
	{
		PRINT("Current extent: (%dx%d)", capabilities.currentExtent.width, capabilities.currentExtent.height);
		return capabilities.currentExtent;
	}

	// Window manager allows us to change the bounds here.

	int widthInPixel{}, heightInPixel{};
	glfwGetFramebufferSize(window, &widthInPixel, &heightInPixel);
	PRINT("GLFW framebuffer size: (%dx%d)", widthInPixel, heightInPixel);

	VkExtent2D actualExtent = { static_cast<U32>(widthInPixel), static_cast<U32>(heightInPixel) };

	actualExtent.width =
		std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
	actualExtent.height =
		std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

	PRINT("Image extent width:  [%d, %d]", capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
	PRINT("Image extent height: [%d, %d]", capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

	return actualExtent;
}

void HelloTriangleApplication::CreateRenderPass()
{
	PRINT("Creating render pass...");

	// Attachment: Load or store operations.
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = mSwapchainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	// Subpass
	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpassDescription{};
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.colorAttachmentCount = 1;
	subpassDescription.pColorAttachments = &colorAttachmentRef;

	VkRenderPassCreateInfo renderPassCreateInfo{};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.attachmentCount = 1;
	renderPassCreateInfo.pAttachments = &colorAttachment;
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpassDescription;

	// Prevent image layout transition from happening until it's actually allowed,
	// i.e. when we want to start writing colors to it.
	// This ensures that the subpass don't begin until the image is available.
	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	// Operations that the subpass wait on. Wait for the swapchain to finish reading from the image.
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	// Operations on the subpass that are waiting (i.e. writing to color attachment).
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	renderPassCreateInfo.dependencyCount = 1;
	renderPassCreateInfo.pDependencies = &dependency;

	VkResult result = vkCreateRenderPass(mDevice, &renderPassCreateInfo, nullptr, &mRenderPass);
	ASSERT_EQ(result, VK_SUCCESS, "Failed to create render pass!");
}

void HelloTriangleApplication::CreateDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding uboLayoutBinding{};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	uboLayoutBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings = &uboLayoutBinding;

	VkResult result = vkCreateDescriptorSetLayout(mDevice, &layoutInfo, nullptr, &mDescriptorSetLayout);
	ASSERT_EQ(result, VK_SUCCESS, "Failed to create descriptor set layout!");
}

void HelloTriangleApplication::CreateGraphicsPipeline()
{
	PRINT("Creating graphics pipeline...");

	// Shader modules
	std::vector<char> vertexShaderCode = Shader::ReadFile("shaders/vert.spv");
	std::vector<char> fragmentShaderCode = Shader::ReadFile("shaders/frag.spv");

	// We can clean up the shader modules after the bytecode is complied to machine code or linked,
	// which happens when the graphics pipeline is created.
	VkShaderModule vertexShaderModule = CreateShaderModule(mDevice, vertexShaderCode);
	VkShaderModule fragmentShaderModule = CreateShaderModule(mDevice, fragmentShaderCode);

	VkPipelineShaderStageCreateInfo vertexShaderStageInfo{};
	vertexShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertexShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertexShaderStageInfo.module = vertexShaderModule;
	vertexShaderStageInfo.pName = "main";  // entry point

	VkPipelineShaderStageCreateInfo fragmentShaderStageInfo{};
	fragmentShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragmentShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragmentShaderStageInfo.module = fragmentShaderModule;
	fragmentShaderStageInfo.pName = "main";	 // entry point

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertexShaderStageInfo, fragmentShaderStageInfo };

	// Pipeline states
	std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

	VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
	dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStateInfo.dynamicStateCount = static_cast<U32>(dynamicStates.size());
	dynamicStateInfo.pDynamicStates = dynamicStates.data();

	// Viewport and scissors
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (F32)mSwapchainExtent.width;
	viewport.height = (F32)mSwapchainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = mSwapchainExtent;

	VkPipelineViewportStateCreateInfo viewportStateInfo{};
	viewportStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportStateInfo.viewportCount = 1;
	viewportStateInfo.pViewports = &viewport;
	viewportStateInfo.scissorCount = 1;
	viewportStateInfo.pScissors = &scissor;

	// Vertex input format
	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	auto bindingDescription = Vertex::GetBindingDescription();
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;

	auto attributeDescription = Vertex::GetAttributeDescriptions();
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<U32>(attributeDescription.size());
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescription.data();

	// Input assembly
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
	inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

	// Rasterizer
	VkPipelineRasterizationStateCreateInfo rasterizerInfo{};
	rasterizerInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizerInfo.depthClampEnable = VK_FALSE;
	rasterizerInfo.rasterizerDiscardEnable = VK_FALSE;
	rasterizerInfo.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizerInfo.lineWidth = 1.0f;
	rasterizerInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizerInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizerInfo.depthBiasEnable = VK_FALSE;
	rasterizerInfo.depthBiasConstantFactor = 0.0f;	// Optional
	rasterizerInfo.depthBiasClamp = 0.0f;			// Optional
	rasterizerInfo.depthBiasSlopeFactor = 0.0f;		// Optional

	// Multisampling, depth and stencil testing
	VkPipelineMultisampleStateCreateInfo multisampleInfo{};
	multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleInfo.sampleShadingEnable = VK_FALSE;
	multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampleInfo.minSampleShading = 1.0f;
	multisampleInfo.pSampleMask = nullptr;
	multisampleInfo.alphaToCoverageEnable = VK_FALSE;
	multisampleInfo.alphaToOneEnable = VK_FALSE;

	// VkPipelineDepthStencilStateCreateInfo;

	// Color blending
	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlendInfo{};
	colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendInfo.logicOpEnable = VK_FALSE;
	colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;
	colorBlendInfo.attachmentCount = 1;
	colorBlendInfo.pAttachments = &colorBlendAttachment;

	// Pipeline layout (uniform)
	// This will be referenced throughout the program's lifetime.
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &mDescriptorSetLayout;

	VkResult createPipelineLayoutResult =
		vkCreatePipelineLayout(mDevice, &pipelineLayoutInfo, nullptr, &mPipelineLayout);
	ASSERT_EQ(createPipelineLayoutResult, VK_SUCCESS, "Failed to create pipeline layout!");

	// Graphics pipeline
	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
	pipelineInfo.pViewportState = &viewportStateInfo;
	pipelineInfo.pRasterizationState = &rasterizerInfo;
	pipelineInfo.pMultisampleState = &multisampleInfo;
	pipelineInfo.pDepthStencilState = nullptr;
	pipelineInfo.pColorBlendState = &colorBlendInfo;
	pipelineInfo.pDynamicState = &dynamicStateInfo;

	pipelineInfo.layout = mPipelineLayout;
	pipelineInfo.renderPass = mRenderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;  // Optional

	VkResult result = vkCreateGraphicsPipelines(mDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &mGraphicsPipeline);
	ASSERT_EQ(result, VK_SUCCESS, "Failed to create graphics pipeline!");

	// Cleanup
	vkDestroyShaderModule(mDevice, fragmentShaderModule, nullptr);
	vkDestroyShaderModule(mDevice, vertexShaderModule, nullptr);
}

VkShaderModule HelloTriangleApplication::CreateShaderModule(VkDevice device, const std::vector<char>& code)
{
	ASSERT(!code.empty(), "Failed to create a shader module for empty code.");

	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();

	// Alignment: The allocator of std::vector already ensures that the data satisfies the worst case alignment
	// requirements.
	createInfo.pCode = reinterpret_cast<const U32*>(code.data());

	VkShaderModule shaderModule;
	VkResult result = vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule);
	ASSERT_EQ(result, VK_SUCCESS, "Failed to create shader module!");

	return shaderModule;
}

void HelloTriangleApplication::CreateFramebuffers()
{
	mSwapchainFramebuffers.resize(mSwapchainImageViews.size());
	PRINT("Creating %zu framebuffers...", mSwapchainFramebuffers.size());

	for (USize i = 0; i < mSwapchainImageViews.size(); i++)
	{
		VkImageView attachmentImageViews[] = { mSwapchainImageViews[i] };
		VkFramebufferCreateInfo framebufferCreateInfo{};
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.renderPass = mRenderPass;
		framebufferCreateInfo.attachmentCount = 1;
		framebufferCreateInfo.pAttachments = attachmentImageViews;
		framebufferCreateInfo.width = mSwapchainExtent.width;
		framebufferCreateInfo.height = mSwapchainExtent.height;
		framebufferCreateInfo.layers = 1;

		VkResult result = vkCreateFramebuffer(mDevice, &framebufferCreateInfo, nullptr, &mSwapchainFramebuffers[i]);
		ASSERT_EQ(result, VK_SUCCESS, "Failed to create framebuffers!");
	}
}

void HelloTriangleApplication::CreateCommandPool()
{
	QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(mPhysicalDevice);

	// We will be recording a command buffer every frame, so we want to be able to reset and
	// record over it again.
	VkCommandPoolCreateInfo poolCreateInfo{};
	poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolCreateInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

	// Command buffers are executed by submitting them on one of the device queues, e.g. graphics queue.
	VkResult result = vkCreateCommandPool(mDevice, &poolCreateInfo, nullptr, &mCommandPool);
	ASSERT_EQ(result, VK_SUCCESS, "Failed to create command pool!");
}

void HelloTriangleApplication::CreateCommandBuffers()
{
	mCommandBuffers.resize(kMaxFramesInFlight);

	VkCommandBufferAllocateInfo bufferAllocateInfo{};
	bufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	bufferAllocateInfo.commandPool = mCommandPool;
	bufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	bufferAllocateInfo.commandBufferCount = (U32)mCommandBuffers.size();

	VkResult result = vkAllocateCommandBuffers(mDevice, &bufferAllocateInfo, mCommandBuffers.data());
	ASSERT_EQ(result, VK_SUCCESS, "Failed to create command buffers!");
}

void HelloTriangleApplication::RecordCommandBuffer(VkCommandBuffer commandBuffer, U32 imageIndex)
{
	// Begin command buffer.
	VkCommandBufferBeginInfo commandBufferBeginInfo{};
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.flags = 0;
	commandBufferBeginInfo.pInheritanceInfo = nullptr;

	VkResult beginResult = vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);
	ASSERT_EQ(beginResult, VK_SUCCESS, "Failed to begin recording command buffer!");

	// Begin render pass.
	VkRenderPassBeginInfo renderPassBeginInfo{};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = mRenderPass;
	// (We previously linked all framebuffers to mRenderPass.)
	renderPassBeginInfo.framebuffer = mSwapchainFramebuffers[imageIndex];  // set the current buffer.
	renderPassBeginInfo.renderArea.offset = { 0, 0 };
	renderPassBeginInfo.renderArea.extent = mSwapchainExtent;
	VkClearValue clearColor = { { { 0.0f, 0.0f, 0.0f, 1.0f } } };
	renderPassBeginInfo.clearValueCount = 1;
	renderPassBeginInfo.pClearValues = &clearColor;

	vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	// Bind graphics pipeline.
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mGraphicsPipeline);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<F32>(mSwapchainExtent.width);
	viewport.height = static_cast<F32>(mSwapchainExtent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = mSwapchainExtent;
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	// Bind vertex and index buffers.
	VkBuffer vertexBuffers[] = { mVertexBuffer };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
	vkCmdBindIndexBuffer(commandBuffer, mIndexBuffer, 0, VK_INDEX_TYPE_UINT16);

	// Bind descriptor sets.
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout, 0, 1,
							&mDescriptorSets[mCurrentFrame], 0, nullptr);

	vkCmdDrawIndexed(commandBuffer, static_cast<U32>(kIndexData.size()), 1, 0, 0, 0);

	vkCmdEndRenderPass(commandBuffer);
	VkResult endResult = vkEndCommandBuffer(commandBuffer);
	ASSERT_EQ(endResult, VK_SUCCESS, "Failed to end command buffer!");
}

VkCommandBuffer HelloTriangleApplication::BeginSingleTimeCommands()
{
	VkCommandBufferAllocateInfo allocateInfo{};
	allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocateInfo.commandPool = mCommandPool;
	allocateInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(mDevice, &allocateInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void HelloTriangleApplication::EndSingleTimeCommands(VkCommandBuffer commandBuffer)
{
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(mGraphicsQueue);

	vkFreeCommandBuffers(mDevice, mCommandPool, 1, &commandBuffer);
}

void HelloTriangleApplication::UpdateUniformBuffer(U32 currentFrame)
{
	static std::chrono::time_point startTime = std::chrono::high_resolution_clock::now();

	std::chrono::time_point currentTime = std::chrono::high_resolution_clock::now();
	F32 duration = std::chrono::duration<F32, std::chrono::seconds::period>(currentTime - startTime).count();

	UniformBufferObject ubo{};
	ubo.model = glm::rotate(glm::mat4(1.0f), duration * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	F32 aspectRatio = mSwapchainExtent.width / (F32)mSwapchainExtent.height;
	ubo.proj = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 10.0f);
	// glm was designed for OpenGL, where the Y coordinate of the clip coordinates is inverted.
	// If not doing this, the image will be rendered upside down.
	ubo.proj[1][1] *= -1;

	memcpy(mUniformBufferMappedPointers[currentFrame], &ubo, sizeof(ubo));
}

void HelloTriangleApplication::CreateVertexBuffer()
{
	// We can use VERTEX_BUFFER_BIT for our vertex buffer, but it is not optimal for GPU to read data from.
	// The most optimal one for the GPU has the DEVICE_LOCAL property bit (meaning we cannot use vkMapMemory) and is
	// usually not accessible by the CPU.
	// So, we need to create a staging buffer as a middle man for transferring the data.
	VkDeviceSize bufferSize = sizeof(Vertex) * kVertexData.size();

	// Create the staging buffer.
	// SRC_BIT: Buffer can be used as source in a memory operation.
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
				 stagingBufferMemory);

	// Copying to GPU (memory mapping).
	void* dstData;
	vkMapMemory(mDevice, stagingBufferMemory, 0, bufferSize, 0, &dstData);
	// Unfortunately the driver may not immediately copy the data into the buffer memory, for example because of
	// caching. It is also possible that writes to the buffer are not visible in the mapped memory yet.
	// This can be resolved by setting the COHERENT bit like we did above.
	memcpy(dstData, kVertexData.data(), (USize)bufferSize);
	vkUnmapMemory(mDevice, stagingBufferMemory);

	// Create the vertex buffer.
	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
				 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mVertexBuffer, mVertexBufferMemory);

	CopyBuffer(stagingBuffer, mVertexBuffer, bufferSize);

	vkDestroyBuffer(mDevice, stagingBuffer, nullptr);
	vkFreeMemory(mDevice, stagingBufferMemory, nullptr);
}

void HelloTriangleApplication::CreateIndexBuffer()
{
	VkDeviceSize bufferSize = sizeof(kIndexData[0]) * kIndexData.size();

	// Create the staging buffer.
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
				 stagingBufferMemory);

	// Copying to GPU (memory mapping).
	void* dstData;
	vkMapMemory(mDevice, stagingBufferMemory, 0, bufferSize, 0, &dstData);
	memcpy(dstData, kIndexData.data(), (USize)bufferSize);
	vkUnmapMemory(mDevice, stagingBufferMemory);

	// Create the index buffer.
	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
				 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mIndexBuffer, mIndexBufferMemory);

	CopyBuffer(stagingBuffer, mIndexBuffer, bufferSize);

	vkDestroyBuffer(mDevice, stagingBuffer, nullptr);
	vkFreeMemory(mDevice, stagingBufferMemory, nullptr);
}

void HelloTriangleApplication::CreateTextureImage()
{
	int texWidth{}, texHeight{}, texChannels{};
	stbi_uc* pixels = stbi_load("textures/Jialuo.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	ASSERT(pixels, "Failed to load texture image!");

	// Create staging buffer for transferring.
	VkDeviceSize imageSize = texWidth * texHeight * 4;
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
				 stagingBufferMemory);

	void* dstData = nullptr;
	vkMapMemory(mDevice, stagingBufferMemory, 0, imageSize, 0, &dstData);
	memcpy(dstData, pixels, static_cast<USize>(imageSize));
	vkUnmapMemory(mDevice, stagingBufferMemory);

	stbi_image_free(pixels);

	// Create image.
	CreateImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
				VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				mTextureImage, mTextureImageMemory);

	// Transition the texture image to VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL.
	TransitionImageLayout(mTextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED,
						  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	// Copy the staging buffer to the texture image.
	CopyBufferToImage(stagingBuffer, mTextureImage, static_cast<U32>(texWidth), static_cast<U32>(texHeight));

	// Transition the texture image again for shader access.
	TransitionImageLayout(mTextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
						  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vkDestroyBuffer(mDevice, stagingBuffer, nullptr);
	vkFreeMemory(mDevice, stagingBufferMemory, nullptr);
}

void HelloTriangleApplication::CreateImage(U32 width, U32 height, VkFormat format, VkImageTiling tiling,
										   VkImageUsageFlags usageFlags, VkMemoryPropertyFlags propertyFlags,
										   VkImage& image, VkDeviceMemory& imageMemory)
{
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usageFlags;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.flags = 0;

	VkResult result = vkCreateImage(mDevice, &imageInfo, nullptr, &image);
	ASSERT_EQ(result, VK_SUCCESS, "Failed to create image!");

	// Allocate memory.
	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(mDevice, image, &memoryRequirements);

	VkMemoryAllocateInfo allocateInfo{};
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.allocationSize = memoryRequirements.size;
	allocateInfo.memoryTypeIndex = FindMemoryType(memoryRequirements.memoryTypeBits, propertyFlags);

	VkResult allocateResult = vkAllocateMemory(mDevice, &allocateInfo, nullptr, &imageMemory);
	ASSERT_EQ(allocateResult, VK_SUCCESS, "Failed to allocate image memory!");

	vkBindImageMemory(mDevice, image, imageMemory, 0);
}

void HelloTriangleApplication::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout,
													 VkImageLayout newLayout)
{
	VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	// Barriers are primarily used for synchronization purposes. We need to do that despite already using
	// vkQueueWaitIdle to manually synchronize.
	VkPipelineStageFlags sourceStage = 0;
	VkPipelineStageFlags destinationStage = 0;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		// Transfer writes must occur in the pipeline transfer stage. Since the writes don't have to wait on anything,
		// you may specify an empty access mask and the earliest possible stage. Note that TRANSFER_BIT stage is not a
		// real stage within the graphics and compute pipelines.
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else
	{
		ASSERT(false, "Unsupported layout transition!");
	}

	vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

	EndSingleTimeCommands(commandBuffer);
}

void HelloTriangleApplication::CopyBufferToImage(VkBuffer buffer, VkImage image, U32 width, U32 height)
{
	VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

	VkBufferImageCopy copyRegion{};
	copyRegion.bufferOffset = 0;
	copyRegion.bufferRowLength = 0;
	copyRegion.bufferImageHeight = 0;

	copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	copyRegion.imageSubresource.mipLevel = 0;
	copyRegion.imageSubresource.baseArrayLayer = 0;
	copyRegion.imageSubresource.layerCount = 1;

	copyRegion.imageOffset = { 0, 0, 0 };
	copyRegion.imageExtent = { width, height, 1 };

	vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

	EndSingleTimeCommands(commandBuffer);
}

void HelloTriangleApplication::CreateTextureImageView()
{
	mTextureImageView = CreateImageView(mTextureImage, VK_FORMAT_R8G8B8A8_SRGB);
}

void HelloTriangleApplication::CreateTextureSampler()
{
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;

	VkPhysicalDeviceProperties deviceProperties{};
	vkGetPhysicalDeviceProperties(mPhysicalDevice, &deviceProperties);
	samplerInfo.maxAnisotropy = deviceProperties.limits.maxSamplerAnisotropy;

	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;

	VkResult result = vkCreateSampler(mDevice, &samplerInfo, nullptr, &mTextureSampler);
	ASSERT_EQ(result, VK_SUCCESS, "Failed to create texture sampler!");
}

void HelloTriangleApplication::CreateUniformBuffers()
{
	VkDeviceSize bufferSize = sizeof(UniformBufferObject);
	mUniformBuffers.resize(kMaxFramesInFlight);
	mUniformBufferMemoryList.resize(kMaxFramesInFlight);
	mUniformBufferMappedPointers.resize(kMaxFramesInFlight);

	for (USize i = 0; i < kMaxFramesInFlight; i++)
	{
		CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
					 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, mUniformBuffers[i],
					 mUniformBufferMemoryList[i]);

		vkMapMemory(mDevice, mUniformBufferMemoryList[i], 0, bufferSize, 0, &mUniformBufferMappedPointers[i]);
	}
}

void HelloTriangleApplication::CreateDescriptorPool()
{
	VkDescriptorPoolSize poolSize{};
	poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize.descriptorCount = static_cast<U32>(kMaxFramesInFlight);

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &poolSize;
	poolInfo.maxSets = static_cast<U32>(kMaxFramesInFlight);

	VkResult result = vkCreateDescriptorPool(mDevice, &poolInfo, nullptr, &mDescriptorPool);
	ASSERT_EQ(result, VK_SUCCESS, "Failed to create descriptor pool!");
}

void HelloTriangleApplication::CreateDescriptorSets()
{
	std::vector<VkDescriptorSetLayout> descriptorSetLayout(kMaxFramesInFlight, mDescriptorSetLayout);

	VkDescriptorSetAllocateInfo allocateInfo{};
	allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocateInfo.descriptorPool = mDescriptorPool;
	allocateInfo.descriptorSetCount = static_cast<U32>(kMaxFramesInFlight);
	allocateInfo.pSetLayouts = descriptorSetLayout.data();

	mDescriptorSets.resize(kMaxFramesInFlight);
	VkResult result = vkAllocateDescriptorSets(mDevice, &allocateInfo, mDescriptorSets.data());
	ASSERT_EQ(result, VK_SUCCESS, "Failed to allocate descriptor sets!");

	// Configure descriptor sets.
	for (USize i = 0; i < kMaxFramesInFlight; i++)
	{
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = mUniformBuffers[i];
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformBufferObject);

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = mDescriptorSets[i];
		descriptorWrite.dstBinding = 0;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo = &bufferInfo;
		descriptorWrite.pImageInfo = nullptr;
		descriptorWrite.pTexelBufferView = nullptr;

		vkUpdateDescriptorSets(mDevice, 1, &descriptorWrite, 0, nullptr);
	}
}

void HelloTriangleApplication::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usageFlags,
											VkMemoryPropertyFlags propertyFlags, VkBuffer& buffer,
											VkDeviceMemory& bufferMemory)
{
	// Buffer creation
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usageFlags;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;	 // only used by the graphics queue

	VkResult bufferResult = vkCreateBuffer(mDevice, &bufferInfo, nullptr, &buffer);
	ASSERT_EQ(bufferResult, VK_SUCCESS, "Failed to create vertex buffer!");

	// Memory allocation
	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(mDevice, buffer, &memoryRequirements);

	VkMemoryAllocateInfo allocateInfo{};
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.allocationSize = memoryRequirements.size;
	// To be able to write to it from CPU.
	allocateInfo.memoryTypeIndex = FindMemoryType(memoryRequirements.memoryTypeBits, propertyFlags);

	VkResult allocateResult = vkAllocateMemory(mDevice, &allocateInfo, nullptr, &bufferMemory);
	ASSERT_EQ(allocateResult, VK_SUCCESS, "Failed to allocate vertex buffer memory!");

	// Bind buffer and allocation.
	vkBindBufferMemory(mDevice, buffer, bufferMemory, 0);
}

void HelloTriangleApplication::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	EndSingleTimeCommands(commandBuffer);
}

void HelloTriangleApplication::CreateSyncObjects()
{
	mImageAvailableSemaphores.resize(kMaxFramesInFlight);
	mRenderFinishedSemaphores.resize(kMaxFramesInFlight);
	mInFlightFences.resize(kMaxFramesInFlight);

	VkSemaphoreCreateInfo semaphoreCreateInfo{};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceCreateInfo{};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (USize i = 0; i < kMaxFramesInFlight; i++)
	{
		VkResult result1 = vkCreateSemaphore(mDevice, &semaphoreCreateInfo, nullptr, &mImageAvailableSemaphores[i]);
		VkResult result2 = vkCreateSemaphore(mDevice, &semaphoreCreateInfo, nullptr, &mRenderFinishedSemaphores[i]);
		VkResult result3 = vkCreateFence(mDevice, &fenceCreateInfo, nullptr, &mInFlightFences[i]);

		ASSERT(result1 == VK_SUCCESS && result2 == VK_SUCCESS && result3 == VK_SUCCESS,
			   "Failed to create synchronization objects for a frame!");
	}
}

void HelloTriangleApplication::DrawFrame()
{
	// 1. Wait for the previous frame to finish.
	// 2. Acquire an image from the swapchain.
	// 3. Record a command buffer which draws the scene onto that image.
	// 4. Submit the command buffer to the graphics queue.
	// 5. Present the swapchain image.

	// Vulkan APIs for the above operations are asynchronous. All functions in DrawFrame are async.
	// Semaphore: Swapchain operations on GPU wait for the previous frame to finish.
	// Fence: Host (CPU) waits for the previous frame to finish to draw the next frame.

	// 1. Wait on host for the previous frame to finish.
	vkWaitForFences(mDevice, 1, &mInFlightFences[mCurrentFrame], VK_TRUE, UINT64_MAX);	// Disable timeout

	// 2. Get the next available swapchain image and signal the semaphore.
	U32 imageIndex;
	VkResult acquireResult = vkAcquireNextImageKHR(
		mDevice, mSwapchain, UINT64_MAX, mImageAvailableSemaphores[mCurrentFrame], VK_NULL_HANDLE, &imageIndex);

	if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR)
	{
		// The swapchain has become incompatible with the surface and can no longer be used for rendering.
		// E.g. window resize.
		RecreateSwapchain();
		return;
	}
	else if (acquireResult != VK_SUCCESS && acquireResult != VK_SUBOPTIMAL_KHR)
	{
		ASSERT(false, "Failed to acquire a swapchain image!");
	}

	// Uniform data
	UpdateUniformBuffer(mCurrentFrame);

	// Only reset the fence if we are actually submitting work to avoid deadlock.
	// Now re-enable the fence between host and GPU.
	vkResetFences(mDevice, 1, &mInFlightFences[mCurrentFrame]);

	// 3. Record commands (begin buffer, begin render pass, bind pipeline, draw)
	vkResetCommandBuffer(mCommandBuffers[mCurrentFrame], 0);
	RecordCommandBuffer(mCommandBuffers[mCurrentFrame], imageIndex);

	// 4. Submit command buffer to graphics queue.
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &mCommandBuffers[mCurrentFrame];

	// Semaphores to wait (on GPU) before command execution.
	VkSemaphore waitSemaphores[] = { mImageAvailableSemaphores[mCurrentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	// Semaphores to signal (on GPU) after command execution.
	VkSemaphore signalSemaphores[] = { mRenderFinishedSemaphores[mCurrentFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	// When the command buffer execution is done, it signals that the command buffer can be reused.
	VkResult submitResult = vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, mInFlightFences[mCurrentFrame]);
	ASSERT_EQ(submitResult, VK_SUCCESS, "Failed to submit command buffer to graphics queue!");

	// 5. Presentation (submitting the result back to swapchain)
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;	 // RenderedFinishedSemaphore

	VkSwapchainKHR swapchains[] = { mSwapchain };
	presentInfo.swapchainCount = 1;	 // Always be a single one.
	presentInfo.pSwapchains = swapchains;
	presentInfo.pImageIndices = &imageIndex;

	VkResult presentResult = vkQueuePresentKHR(mPresentQueue, &presentInfo);

	if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR || mFramebufferResized)
	{
		mFramebufferResized = false;
		RecreateSwapchain();
	}
	else if (presentResult != VK_SUCCESS)
	{
		ASSERT(false, "Failed to present the swapchain image!");
	}

	// Advance the current frame index.
	mCurrentFrame = (mCurrentFrame + 1) % kMaxFramesInFlight;
}

void HelloTriangleApplication::MainLoop()
{
	PRINT("Running main loop...");

	while (!glfwWindowShouldClose(mWindow))
	{
		glfwPollEvents();
		DrawFrame();
	}

	// Wait for asynchronous drawing and presentation operations that may still be going on.
	// You can also use vkQueueWaitIdle.
	vkDeviceWaitIdle(mDevice);
}

void HelloTriangleApplication::CleanUp()
{
	PRINT("Cleaning up...");

	CleanUpSwapchain();	 // framebuffers, image views, swapchain

	vkDestroySampler(mDevice, mTextureSampler, nullptr);
	vkDestroyImageView(mDevice, mTextureImageView, nullptr);
	vkDestroyImage(mDevice, mTextureImage, nullptr);
	vkFreeMemory(mDevice, mTextureImageMemory, nullptr);

	for (USize i = 0; i < kMaxFramesInFlight; i++)
	{
		vkDestroyBuffer(mDevice, mUniformBuffers[i], nullptr);
		vkFreeMemory(mDevice, mUniformBufferMemoryList[i], nullptr);
	}

	vkDestroyDescriptorPool(mDevice, mDescriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(mDevice, mDescriptorSetLayout, nullptr);

	vkDestroyBuffer(mDevice, mIndexBuffer, nullptr);
	vkFreeMemory(mDevice, mIndexBufferMemory, nullptr);

	vkDestroyBuffer(mDevice, mVertexBuffer, nullptr);
	vkFreeMemory(mDevice, mVertexBufferMemory, nullptr);

	vkDestroyPipeline(mDevice, mGraphicsPipeline, nullptr);
	vkDestroyPipelineLayout(mDevice, mPipelineLayout, nullptr);
	vkDestroyRenderPass(mDevice, mRenderPass, nullptr);

	for (USize i = 0; i < kMaxFramesInFlight; i++)
	{
		vkDestroySemaphore(mDevice, mImageAvailableSemaphores[i], nullptr);
		vkDestroySemaphore(mDevice, mRenderFinishedSemaphores[i], nullptr);
		vkDestroyFence(mDevice, mInFlightFences[i], nullptr);
	}

	vkDestroyCommandPool(mDevice, mCommandPool, nullptr);  // Also frees command buffers

	vkDestroyDevice(mDevice, nullptr);

	if (kEnableValidationLayers)
	{
		DestroyDebugUtilsMessengerEXT(mInstance, mDebugMessenger, nullptr);
	}

	vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
	vkDestroyInstance(mInstance, nullptr);

	glfwDestroyWindow(mWindow);
	glfwTerminate();
}

bool HelloTriangleApplication::CheckValidationLayerSupport()
{
	U32 layerCount{};
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	// Make sure all the layer names are present.
	for (const char* layerName : kValidationLayers)
	{
		auto it = std::find_if(
			availableLayers.begin(), availableLayers.end(),
			[layerName](VkLayerProperties layerProperties) { return strcmp(layerProperties.layerName, layerName); });

		if (it == availableLayers.end())
		{
			return false;
		}
	}

	return true;
}

// Proxy functions to create and destroy debug messenger.
// Since this function is an extension function, it is not automatically loaded.
// We have to look up its address ourselves.
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
									  const VkAllocationCallbacks* pAllocator,
									  VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

	if (func)
	{
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
								   const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

	ASSERT(func, "Function to destroy debug messenger could not be found!");

	if (func)
	{
		func(instance, debugMessenger, pAllocator);
	}
}

void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
								 VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
								 VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
							 VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
							 VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

	createInfo.pfnUserCallback = DebugMessengerCallback;
	createInfo.pUserData = nullptr;
}

bool HelloTriangleApplication::IsPhysicalDeviceSuitable(VkPhysicalDevice device)
{
	QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(device);
	bool extensionsSupported = CheckDeviceExtensionSupport(device);

	// It is important that we only query for swap chain support after verifying that the extensions are available.
	bool swapChainAdequate = false;
	if (extensionsSupported)
	{
		SwapchainSupportDetails swapChainSupport = QuerySwapchainSupport(device, mSurface);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	VkPhysicalDeviceFeatures supportedDeviceFeatures{};
	vkGetPhysicalDeviceFeatures(device, &supportedDeviceFeatures);

	return queueFamilyIndices.IsComplete() && extensionsSupported && swapChainAdequate &&
		   supportedDeviceFeatures.samplerAnisotropy;
}

QueueFamilyIndices HelloTriangleApplication::FindQueueFamilies(VkPhysicalDevice device)
{
	U32 queueFamilyCount{};
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	// Find at least one queue family that supports VK_QUEUE_GRAPHICS_BIT.
	QueueFamilyIndices queueFamilyData;
	U32 queueFamilyIndex = 0;

	for (const auto& queueFamily : queueFamilies)
	{
		// Graphics
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			queueFamilyData.graphicsFamily = queueFamilyIndex;
		}

		// Present
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, queueFamilyIndex, mSurface, &presentSupport);

		if (presentSupport)
		{
			queueFamilyData.presentFamily = queueFamilyIndex;
		}

		if (queueFamilyData.IsComplete())
		{
			break;
		}

		queueFamilyIndex++;
	}

	return queueFamilyData;
}

// If typeFilter is 0000 1100, the function will return an index of 2.
U32 HelloTriangleApplication::FindMemoryType(U32 typeFilter, VkMemoryPropertyFlags propertyFlags)
{
	// Memory heaps are distinct memory resources like dedicated VRAM and swap space in RAM for when VRAM runs out.
	// The different types of memory exist within these heaps.

	VkPhysicalDeviceMemoryProperties memoryProperties;
	vkGetPhysicalDeviceMemoryProperties(mPhysicalDevice, &memoryProperties);

	// PRINT("Memory type count: %u | heap count: %u", memoryProperties.memoryTypeCount,
	// memoryProperties.memoryHeapCount);

	for (U32 typeIndex = 0; typeIndex < memoryProperties.memoryTypeCount; typeIndex++)
	{
		if (typeFilter & (1 << typeIndex))
		{
			VkMemoryType memoryType = memoryProperties.memoryTypes[typeIndex];

			// Check if the desired property flags are all matched.
			if ((memoryType.propertyFlags & propertyFlags) == propertyFlags)
			{
				return typeIndex;
			}
		}
	}

	ASSERT(false, "Failed to find suitable memory type!");
	return -1;
}

bool CheckDeviceExtensionSupport(VkPhysicalDevice device)
{
	U32 extensionCount = 0;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(kDeviceExtensions.begin(), kDeviceExtensions.end());

	for (const auto& extension : availableExtensions)
	{
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

// Physical device and surface are needed as they are core components of swapchain.
SwapchainSupportDetails QuerySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	SwapchainSupportDetails details{};
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

	U32 formatCount{};
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

	if (formatCount != 0)
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
	}

	U32 presentModeCount{};
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

	if (presentModeCount != 0)
	{
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}

void HelloTriangleApplication::FramebufferResizeCallback(GLFWwindow* window, int width, int height)
{
	// auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
	// 'void*' conversion is well defined, so we can use static_cast.
	auto app = static_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
	app->mFramebufferResized = true;
}
