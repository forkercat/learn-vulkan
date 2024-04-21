//
// Created by Junhao Wang (@forkercat) on 3/9/24.
//

#pragma once

#include "core/core.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <vector>
#include <iostream>
#include <set>
#include <vector>
#include <limits>
#include <algorithm>

struct QueueFamilyIndices;

struct Vertex
{
	glm::vec3 position;
	glm::vec3 color;
	glm::vec2 texCoord;

	static VkVertexInputBindingDescription GetBindingDescription()
	{
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 3> GetAttributeDescriptions()
	{
		std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, position);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

		return attributeDescriptions;
	}

	// Equality test
	bool operator==(const Vertex& other) const
	{
		return position == other.position && color == other.color && texCoord == other.texCoord;
	};
};

namespace std
{
	template <>
	struct hash<Vertex>
	{
		size_t operator()(Vertex const& vertex) const
		{
			return ((hash<glm::vec3>()(vertex.position) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
} // namespace std

class HelloTriangleApplication
{
public:
	void Run();

private:
	// Initialize GLFW window.
	void InitWindow();
	// Initialize Vulkan resources.
	void InitVulkan();

	/// Instance, device & validation layers

	// Create a Vulkan instance. Usually you would create the Vulkan application and check and specify required
	// extensions by GLFW, your platform, and validation layer.
	void CreateInstance();
	// Set up the debug messenger callback for validation layer. You would also configure the validation debug options.
	void SetupDebugMessenger();
	// Create window surface. Since GLFW supports Vulkan API, we will use GLFW API to create the Vulkan surface object.
	void CreateWindowSurface();
	// Pick an available physical device and check if required device extensions are supported (e.g. swapchain).
	// We can also query the physical device about queue families and device features.
	void PickPhysicalDevice();
	// Create a logical device and queues.
	void CreateLogicalDeviceAndQueues();

	/// Swapchain & pipeline
	void RecreateSwapchain();
	void CleanUpSwapchain();

	// Create a swapchain with swap images and queues.
	void CreateSwapchain();
	void CreateSwapchainImageViews();
	VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
	// Create a render pass with subpasses that define attachment formats.
	void CreateRenderPass();
	// Create a descriptor set layout.
	void CreateDescriptorSetLayout();
	// Create a graphics pipeline state object (PSO) with shaders. Set the pipeline's render pass.
	void CreateGraphicsPipeline();
	// Create framebuffers for swap image views. Note that we create framebuffers after and later when we begin the
	// render pass it would require setting framebuffers for the render pass.
	void CreateFramebuffers();

	/// Command buffer

	// Create command pool for a particular family of queues (i.e. graphics).
	void CreateCommandPool();
	void CreateCommandBuffers();
	void RecordCommandBuffer(VkCommandBuffer commandBuffer, U32 imageIndex);
	VkCommandBuffer BeginSingleTimeCommands();
	void EndSingleTimeCommands(VkCommandBuffer commandBuffer);

	/// Texture
	void CreateTextureImage();
	void CreateImage(U32 width, U32 height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usageFlags,
		VkMemoryPropertyFlags propertyFlags, VkImage& image, VkDeviceMemory& imageMemory);
	void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
	void CopyBufferToImage(VkBuffer buffer, VkImage image, U32 width, U32 height);
	void CreateTextureImageView();
	void CreateTextureSampler();

	/// Depth
	void CreateDepthResources();
	VkFormat FindDepthFormat();

	/// Uniform buffer
	void CreateUniformBuffers();
	void UpdateUniformBuffer(U32 currentFrame);

	/// Model
	void LoadModel();

	/// Vertex buffer
	void CreateVertexBuffer();
	void CreateIndexBuffer();

	void CreateDescriptorPool();
	void CreateDescriptorSets();

	void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags propertyFlags,
		VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

	/// Synchronization
	void CreateSyncObjects();

	/// Draw
	void DrawFrame();

	void MainLoop();
	void CleanUp();

	static std::vector<const char*> GetRequiredInstanceExtensions();
	static bool CheckValidationLayerSupport();

	bool IsPhysicalDeviceSuitable(VkPhysicalDevice device);
	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);

	U32 FindMemoryType(U32 typeFilter, VkMemoryPropertyFlags propertyFlags);

	static VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	static VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	static VkExtent2D ChooseSwapExtent(GLFWwindow* window, const VkSurfaceCapabilitiesKHR& capabilities);

	static VkShaderModule CreateShaderModule(VkDevice device, const std::vector<char>& code);

	static VkFormat FindSupportedFormat(VkPhysicalDevice physicalDevice, const std::vector<VkFormat>& formatCandidates,
		VkImageTiling tiling, VkFormatFeatureFlags features);
	static bool HasStencilComponent(VkFormat format);

	static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);

private:
	GLFWwindow* m_window;
	VkInstance m_instance;
	VkDebugUtilsMessengerEXT m_debugMessenger;
	VkSurfaceKHR m_surface;
	VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE; // Auto destroyed when VkInstance is destroyed.
	VkDevice mDevice;
	VkQueue m_graphicsQueue;
	VkQueue m_presentQueue;

	// Swapchain
	VkSwapchainKHR m_swapchain;
	VkFormat m_swapchainImageFormat;
	VkExtent2D m_swapchainExtent;
	std::vector<VkImage> m_swapchainImages; // Auto destroyed when the swap chain is cleaned up.
	std::vector<VkImageView> m_swapchainImageViews;
	std::vector<VkFramebuffer> m_swapchainFramebuffers;

	// Depth buffer
	VkImage m_depthImage;
	VkDeviceMemory m_depthImageMemory;
	VkImageView m_depthImageView;

	// Graphics pipeline & descriptors
	VkRenderPass m_renderPass;
	VkDescriptorSetLayout m_descriptorSetLayout;
	VkDescriptorPool m_descriptorPool;
	std::vector<VkDescriptorSet> m_descriptorSets;
	VkPipelineLayout m_pipelineLayout;
	VkPipeline m_graphicsPipeline;

	// Commands
	VkCommandPool m_commandPool;
	std::vector<VkCommandBuffer> m_commandBuffers;

	// Buffers
	VkBuffer m_vertexBuffer;
	VkDeviceMemory m_vertexBufferMemory;
	VkBuffer m_indexBuffer;
	VkDeviceMemory m_indexBufferMemory;

	// Texture images
	VkImage mTextureImage;
	VkDeviceMemory mTextureImageMemory;
	VkImageView mTextureImageView;
	VkSampler mTextureSampler;

	std::vector<VkBuffer> m_uniformBuffers;
	std::vector<VkDeviceMemory> m_uniformBufferMemoryList;
	std::vector<void*> m_uniformBufferMappedPointers;

	// Synchronization
	std::vector<VkSemaphore> m_imageAvailableSemaphores;
	std::vector<VkSemaphore> m_renderFinishedSemaphores;
	std::vector<VkFence> m_inFlightFences;

	// Keep track of the current frame to use the right sync objects. Having multiple frames in flight enables us start
	// rendering the next, with rendering of one frame to not interfere with the recording of the next.
	// Why 2 frames? Don't want the CPU to get too far ahead of the GPU.
	// The CPU and the GPU can be working on their own tasks at the same time.
	// -----------------------------------------------------------------------
	// Records 1st frame on CPU -> Renders 1st frame on GPU -> Renders 2nd frame on GPU
	//                          -> Records 2nd frame on CPU -> Records 1st frame on CPU
	U32 m_currentFrame = 0;
	static const U32 MAX_FRAMES_IN_FLIGHT = 2;

	// Handling resizes explicitly for drivers/platforms that cannot trigger VK_ERROR_OUT_OF_DATE_KHR.
	bool m_framebufferResized = false;

	// Model data
	std::vector<Vertex> m_modelVertices;
	std::vector<U32> m_modelIndices;
	VkBuffer m_modelVertexBuffer;
	VkDeviceMemory m_modelVertexBufferMemory;
};
