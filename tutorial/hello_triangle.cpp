//
// Created by Junhao Wang (@forkercat) on 3/9/24.
//

#include "hello_triangle.h"

static const U32 kWidth = 800;
static const U32 kHeight = 600;

static const std::vector<const char*> kValidationLayers = { "VK_LAYER_KHRONOS_validation" };
static const std::vector<const char*> kDeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME,
															"VK_KHR_portability_subset" };

struct QueueFamilyIndices
{
	std::optional<U32> graphicsFamily;
	std::optional<U32> presentFamily;

	bool IsComplete() { return graphicsFamily.has_value() && presentFamily.has_value(); }
};

struct SwapChainSupportDetails
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

VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
											 VkDebugUtilsMessageTypeFlagsEXT messageType,
											 const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;
	return VK_FALSE;  // Original Vulkan call is not aborted
}

// Proxy functions to create and destroy debug messenger.
VkResult CreateDebugUtilsMessengerEXT(VkInstance, const VkDebugUtilsMessengerCreateInfoEXT*,
									  const VkAllocationCallbacks*, VkDebugUtilsMessengerEXT*);
void DestroyDebugUtilsMessengerEXT(VkInstance, VkDebugUtilsMessengerEXT, const VkAllocationCallbacks*);
void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT&);

bool CheckDeviceExtensionSupport(VkPhysicalDevice);
SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice, VkSurfaceKHR);

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
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	mWindow = glfwCreateWindow(kWidth, kHeight, "Vulkan HelloTriangle", nullptr, nullptr);

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
	CreateSwapChain();
}

void HelloTriangleApplication::CreateInstance()
{
	if (kEnableValidationLayers && !CheckValidationLayerSupport())
	{
		ERROR("Validation layers requested, but not available!");
	}

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

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};  // Placed outside if for longer lifecycle.

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
	};

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
	PRINT("Creating logical device...");

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

void HelloTriangleApplication::CreateSwapChain()
{
	PRINT("Creating swap chain...");
	SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(mPhysicalDevice, mSurface);

	VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent2D = ChooseSwapExtent(mWindow, swapChainSupport.capabilities);

	// It is recommended to request at least one more image than the minimum.
	U32 imageCount = swapChainSupport.capabilities.minImageCount + 1;

	if (swapChainSupport.capabilities.maxImageCount > 0)
	{
		imageCount = std::min(imageCount, swapChainSupport.capabilities.maxImageCount);
	}

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = mSurface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent2D;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	// Specify how to handle swap chain images that will be used across multiple queue families.
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

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	VkResult result = vkCreateSwapchainKHR(mDevice, &createInfo, nullptr, &mSwapChain);
	ASSERT_EQ(result, VK_SUCCESS, "Failed to create swap chain!");

	// Retrieve images.
	vkGetSwapchainImagesKHR(mDevice, mSwapChain, &imageCount, nullptr);
	mSwapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(mDevice, mSwapChain, &imageCount, mSwapChainImages.data());

	mSwapChainImageFormat = surfaceFormat.format;
	mSwapChainExtent = extent2D;
}

VkSurfaceFormatKHR HelloTriangleApplication::ChooseSwapSurfaceFormat(
	const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	for (const auto& availableFormat : availableFormats)
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
			availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return availableFormat;
		}
	}

	ASSERT(!availableFormats.empty(), "Failed to pick available format!");
	return availableFormats[0];
}

VkPresentModeKHR HelloTriangleApplication::ChooseSwapPresentMode(
	const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	for (const auto& availablePresentMode : availablePresentModes)
	{
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return availablePresentMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
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

void HelloTriangleApplication::MainLoop()
{
	PRINT("Running main loop...");
	while (!glfwWindowShouldClose(mWindow))
	{
		glfwPollEvents();
	}
}

void HelloTriangleApplication::CleanUp()
{
	PRINT("Cleaning up...");
	vkDestroySwapchainKHR(mDevice, mSwapChain, nullptr);
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
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers)
		{
			if (strcmp(layerName, layerProperties.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}

		if (!layerFound)
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

	createInfo.pfnUserCallback = DebugCallback;
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
		SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device, mSurface);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	return queueFamilyIndices.IsComplete() && extensionsSupported && swapChainAdequate;
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

// Device and surface are needed as they are core components of swap chain.
SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	SwapChainSupportDetails details{};
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
