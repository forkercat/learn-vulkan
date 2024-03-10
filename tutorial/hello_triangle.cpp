//
// Created by Junhao Wang (@forkercat) on 3/9/24.
//

#include "hello_triangle.h"

#include <iostream>
#include <vector>

static const U32 kWidth = 800;
static const U32 kHeight = 600;

static const std::vector<const char*> kValidationLayers = { "VK_LAYER_KHRONOS_validation" };

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
void ClearAndPopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

void HelloTriangleApplication::Run()
{
	InitWindow();
	InitVulkan();
	MainLoop();
	Cleanup();
}

void HelloTriangleApplication::InitWindow()
{
	LOG("InitWindow...");
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	mWindow = glfwCreateWindow(kWidth, kHeight, "Vulkan", nullptr, nullptr);

	U32 extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> supportedExtensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, supportedExtensions.data());

	LOG("Vulkan extension count: %u", extensionCount);
}

void HelloTriangleApplication::InitVulkan()
{
	CreateInstance();
	SetupDebugMessenger();
}

void HelloTriangleApplication::CreateInstance()
{
	LOG("InitVulkan - CreateInstance");
	if (kEnableValidationLayers && !CheckValidationLayerSupport())
	{
		throw std::runtime_error("Validation layers requested, but not available!");
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
	std::vector<const char*> requiredExtensions = GetRequiredExtensions();
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
		ClearAndPopulateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = static_cast<VkDebugUtilsMessengerCreateInfoEXT*>(&debugCreateInfo);
	}
	else
	{
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
	};

	VkResult result = vkCreateInstance(&createInfo, nullptr, &mInstance);

	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Vulkan instance!");
	}
}

std::vector<const char*> HelloTriangleApplication::GetRequiredExtensions()
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

	return extensions;
}

void HelloTriangleApplication::SetupDebugMessenger()
{
	LOG("InitVulkan - SetupDebugMessenger");
	if (kEnableValidationLayers)
	{
		VkDebugUtilsMessengerCreateInfoEXT createInfo{};
		ClearAndPopulateDebugMessengerCreateInfo(createInfo);

		VkResult result = CreateDebugUtilsMessengerEXT(mInstance, &createInfo, nullptr, &mDebugMessenger);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to set up debug messenger!");
		}
	}
}

void HelloTriangleApplication::MainLoop()
{
	while (!glfwWindowShouldClose(mWindow))
	{
		glfwPollEvents();
	}
}

void HelloTriangleApplication::Cleanup()
{
	LOG("Cleanup...");
	if (kEnableValidationLayers)
	{
		DestroyDebugUtilsMessengerEXT(mInstance, mDebugMessenger, nullptr);
	}

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

void ClearAndPopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
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
