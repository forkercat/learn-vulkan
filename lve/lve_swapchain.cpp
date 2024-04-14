//
// Created by Junhao Wang (@forkercat) on 4/3/24.
//

#include "lve_swapchain.h"

namespace lve {

	LveSwapchain::LveSwapchain(LveDevice& device, VkExtent2D windowExtent)
		: mDevice(device), mWindowExtent(windowExtent)
	{
		Init();
	}

	LveSwapchain::LveSwapchain(LveDevice& device, VkExtent2D windowExtent, std::shared_ptr<LveSwapchain> previous)
		: mDevice(device), mWindowExtent(windowExtent), mOldSwapchain(previous)
	{
		Init();

		// Clean up old swapchain since it's no longer needed.
		mOldSwapchain = nullptr;
	}

	void LveSwapchain::Init()
	{
		CreateSwapchain();
		CreateImageViews();
		CreateRenderPass();
		CreateDepthResources();
		CreateFramebuffers();
		CreateSyncObjects();
	}

	LveSwapchain::~LveSwapchain()
	{
		for (VkImageView& imageView : mSwapchainImageViews)
		{
			vkDestroyImageView(mDevice.GetDevice(), imageView, nullptr);
		}
		mSwapchainImageViews.clear();

		if (mSwapchain != nullptr)
		{
			vkDestroySwapchainKHR(mDevice.GetDevice(), mSwapchain, nullptr);
			mSwapchain = nullptr;
		}

		for (USize i = 0; i < mDepthImages.size(); i++)
		{
			vkDestroyImageView(mDevice.GetDevice(), mDepthImageViews[i], nullptr);
			vkDestroyImage(mDevice.GetDevice(), mDepthImages[i], nullptr);
			vkFreeMemory(mDevice.GetDevice(), mDepthImageMemorys[i], nullptr);
		}

		for (VkFramebuffer& framebuffer : mSwapchainFramebuffers)
		{
			vkDestroyFramebuffer(mDevice.GetDevice(), framebuffer, nullptr);
		}

		vkDestroyRenderPass(mDevice.GetDevice(), mRenderPass, nullptr);

		for (USize i = 0; i < MaxFramesInFlight; i++)
		{
			vkDestroySemaphore(mDevice.GetDevice(), mRenderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(mDevice.GetDevice(), mImageAvailableSemaphores[i], nullptr);
			vkDestroyFence(mDevice.GetDevice(), mInFlightFences[i], nullptr);
		}
	}

	/////////////////////////////////////////////////////////////////////////////////
	// Public functions
	/////////////////////////////////////////////////////////////////////////////////

	VkFormat LveSwapchain::FindDepthFormat()
	{
		return mDevice.FindSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
										   VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
	}

	VkResult LveSwapchain::AcquireNextImage(U32* imageIndex)
	{
		// Wait on host for the frame to finish.
		vkWaitForFences(mDevice.GetDevice(), 1, &mInFlightFences[mCurrentFrame], VK_TRUE,
						std::numeric_limits<U64>::max());  // disable timeout

		// Asynchronously on GPU get the next available swapchain image and signal the semaphore.
		return vkAcquireNextImageKHR(mDevice.GetDevice(), mSwapchain, std::numeric_limits<U64>::max(),
									 mImageAvailableSemaphores[mCurrentFrame], VK_NULL_HANDLE, imageIndex);
	}

	VkResult LveSwapchain::SubmitCommandBuffers(const VkCommandBuffer* buffers, U32* imageIndex)
	{
		ASSERT(imageIndex, "Image index pointer is nullptr.");

		if (mImagesInFlight[*imageIndex] != VK_NULL_HANDLE)
		{
			vkWaitForFences(mDevice.GetDevice(), 1, &mImagesInFlight[*imageIndex], VK_TRUE, std::numeric_limits<U64>::max());
		}

		mImagesInFlight[*imageIndex] = mInFlightFences[mCurrentFrame];

		// Submit command buffer to graphics queue.
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = buffers;

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

		// Only reset the fence if we are actually submitting work to avoid deadlock.
		vkResetFences(mDevice.GetDevice(), 1, &mInFlightFences[mCurrentFrame]);

		// When the command buffer execution is done, it signals that the command buffer can be reused.
		VkResult submitResult = vkQueueSubmit(mDevice.GetGraphicsQueue(), 1, &submitInfo, mInFlightFences[mCurrentFrame]);
		ASSERT_EQ(submitResult, VK_SUCCESS, "Failed to submit command buffer to graphics queue!");

		// Presentation (submitting the result back to swapchain)
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;	 // RenderedFinishedSemaphore

		VkSwapchainKHR swapchains[] = { mSwapchain };
		presentInfo.swapchainCount = 1;	 // Always be a single one.
		presentInfo.pSwapchains = swapchains;
		presentInfo.pImageIndices = imageIndex;

		VkResult presentResult = vkQueuePresentKHR(mDevice.GetPresentQueue(), &presentInfo);

		// Advance the current frame index.
		mCurrentFrame = (mCurrentFrame + 1) % MaxFramesInFlight;

		return presentResult;
	}

	/////////////////////////////////////////////////////////////////////////////////
	// Functions to create Vulkan resources
	/////////////////////////////////////////////////////////////////////////////////

	void LveSwapchain::CreateSwapchain()
	{
		PRINT("Creating swapchain...");
		SwapchainSupportDetails swapchainSupport = mDevice.GetSwapchainSupport();

		VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapchainSupport.formats);
		VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapchainSupport.presentModes);
		VkExtent2D extent2D = ChooseSwapExtent(swapchainSupport.capabilities);

		// It is recommended to request at least one more image than the minimum.
		U32 imageCount = swapchainSupport.capabilities.minImageCount + 1;

		if (swapchainSupport.capabilities.maxImageCount > 0)
		{
			imageCount = std::min(imageCount, swapchainSupport.capabilities.maxImageCount);
		}

		PRINT("Image count: %u", imageCount);

		VkSwapchainCreateInfoKHR swapchainInfo{};
		swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;

		swapchainInfo.surface = mDevice.GetSurface();
		swapchainInfo.minImageCount = imageCount;
		swapchainInfo.imageFormat = surfaceFormat.format;
		swapchainInfo.imageColorSpace = surfaceFormat.colorSpace;
		swapchainInfo.imageExtent = extent2D;
		swapchainInfo.imageArrayLayers = 1;
		swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		// Specify how to handle swapchain images that will be used across multiple queue families.
		// E.g. Drawing on the images in the swap chain from the graphics queue and then submitting
		// them on the presentation queue.
		QueueFamilyIndices familyIndices = mDevice.FindPhysicalQueueFamilies();
		U32 queueFamilyIndices[] = { familyIndices.graphicsFamily.value(), familyIndices.presentFamily.value() };

		if (familyIndices.graphicsFamily != familyIndices.presentFamily)
		{
			// Graphics and presentation queue families differ.
			// Images should be used across multiple queue families without explicit ownership transfers.
			swapchainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			swapchainInfo.queueFamilyIndexCount = 2;
			swapchainInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else
		{
			swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			swapchainInfo.queueFamilyIndexCount = 0;	  // Optional
			swapchainInfo.pQueueFamilyIndices = nullptr;  // Optional
		}

		swapchainInfo.preTransform = swapchainSupport.capabilities.currentTransform;
		swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

		swapchainInfo.presentMode = presentMode;
		swapchainInfo.clipped = VK_TRUE;
		swapchainInfo.oldSwapchain = mOldSwapchain == nullptr ? VK_NULL_HANDLE : mOldSwapchain->mSwapchain;

		VkResult result = vkCreateSwapchainKHR(mDevice.GetDevice(), &swapchainInfo, nullptr, &mSwapchain);
		ASSERT_EQ(result, VK_SUCCESS, "Failed to create swapchain!");

		// Retrieve images.
		vkGetSwapchainImagesKHR(mDevice.GetDevice(), mSwapchain, &imageCount, nullptr);
		mSwapchainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(mDevice.GetDevice(), mSwapchain, &imageCount, mSwapchainImages.data());

		mSwapchainImageFormat = surfaceFormat.format;
		mSwapchainExtent = extent2D;
	}

	void LveSwapchain::CreateImageViews()
	{
		mSwapchainImageViews.resize(mSwapchainImages.size());

		for (USize i = 0; i < mSwapchainImages.size(); i++)
		{
			mSwapchainImageViews[i] = CreateImageView(mSwapchainImages[i], mSwapchainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
		}
	}

	void LveSwapchain::CreateRenderPass()
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

		VkAttachmentDescription depthAttachment{};
		depthAttachment.format = FindDepthFormat();
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;	 // we just read from it for testing
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		// Subpass
		VkSubpassDescription subpassDescription{};
		subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDescription.colorAttachmentCount = 1;
		subpassDescription.pColorAttachments = &colorAttachmentRef;
		subpassDescription.pDepthStencilAttachment = &depthAttachmentRef;

		VkRenderPassCreateInfo renderPassCreateInfo{};
		renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassCreateInfo.attachmentCount = static_cast<U32>(attachments.size());
		renderPassCreateInfo.pAttachments = attachments.data();
		renderPassCreateInfo.subpassCount = 1;
		renderPassCreateInfo.pSubpasses = &subpassDescription;

		// Prevent image layout transition from happening until it's actually allowed,
		// i.e. when we want to start writing colors to it.
		// This ensures that the subpass don't begin until the image is available.
		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		// Operations that the subpass wait on. Wait for the swapchain to finish reading from the image.
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.srcAccessMask = 0;

		dependency.dstSubpass = 0;
		// Operations on the subpass that are waiting (i.e. writing to color attachment).
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		renderPassCreateInfo.dependencyCount = 1;
		renderPassCreateInfo.pDependencies = &dependency;

		VkResult result = vkCreateRenderPass(mDevice.GetDevice(), &renderPassCreateInfo, nullptr, &mRenderPass);
		ASSERT_EQ(result, VK_SUCCESS, "Failed to create render pass!");
	}

	void LveSwapchain::CreateDepthResources()
	{
		PRINT("Creating depth resources...");

		VkFormat depthFormat = FindDepthFormat();

		mDepthImages.resize(GetImageCount());
		mDepthImageMemorys.resize(GetImageCount());
		mDepthImageViews.resize(GetImageCount());

		for (USize i = 0; i < mDepthImages.size(); i++)
		{
			VkImageCreateInfo imageInfo{};
			imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageInfo.imageType = VK_IMAGE_TYPE_2D;
			imageInfo.extent.width = mSwapchainExtent.width;
			imageInfo.extent.height = mSwapchainExtent.height;
			imageInfo.extent.depth = 1;
			imageInfo.mipLevels = 1;
			imageInfo.arrayLayers = 1;
			imageInfo.format = depthFormat;
			imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			imageInfo.flags = 0;

			mDevice.CreateImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mDepthImages[i], mDepthImageMemorys[i]);
			mDepthImageViews[i] = CreateImageView(mDepthImages[i], depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
		}

		// We don't need to explicitly transition the layout of the image to a depth attachment because this will be done in
		// the render pass.
		// TransitionImageLayout(mDepthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED,
		// VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
	}

	void LveSwapchain::CreateFramebuffers()
	{
		PRINT("Creating %zu framebuffers...", mSwapchainFramebuffers.size());

		mSwapchainFramebuffers.resize(mSwapchainImageViews.size());

		for (USize i = 0; i < mSwapchainImageViews.size(); i++)
		{
			std::array<VkImageView, 2> imageViews = { mSwapchainImageViews[i], mDepthImageViews[i] };

			VkFramebufferCreateInfo framebufferCreateInfo{};
			framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferCreateInfo.renderPass = mRenderPass;
			framebufferCreateInfo.attachmentCount = static_cast<U32>(imageViews.size());
			framebufferCreateInfo.pAttachments = imageViews.data();
			framebufferCreateInfo.width = mSwapchainExtent.width;
			framebufferCreateInfo.height = mSwapchainExtent.height;
			framebufferCreateInfo.layers = 1;

			VkResult result = vkCreateFramebuffer(mDevice.GetDevice(), &framebufferCreateInfo, nullptr, &mSwapchainFramebuffers[i]);
			ASSERT_EQ(result, VK_SUCCESS, "Failed to create framebuffers!");
		}
	}

	void LveSwapchain::CreateSyncObjects()
	{
		mImageAvailableSemaphores.resize(MaxFramesInFlight);
		mRenderFinishedSemaphores.resize(MaxFramesInFlight);
		mInFlightFences.resize(MaxFramesInFlight);
		mImagesInFlight.resize(GetImageCount(), VK_NULL_HANDLE);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (USize i = 0; i < MaxFramesInFlight; i++)
		{
			VkResult result1 = vkCreateSemaphore(mDevice.GetDevice(), &semaphoreInfo, nullptr, &mImageAvailableSemaphores[i]);
			VkResult result2 = vkCreateSemaphore(mDevice.GetDevice(), &semaphoreInfo, nullptr, &mRenderFinishedSemaphores[i]);
			VkResult result3 = vkCreateFence(mDevice.GetDevice(), &fenceInfo, nullptr, &mInFlightFences[i]);

			ASSERT(result1 == VK_SUCCESS && result2 == VK_SUCCESS && result3 == VK_SUCCESS,
				   "Failed to create synchronization objects for a frame!");
		}
	}

	/////////////////////////////////////////////////////////////////////////////////
	// Helper functions
	/////////////////////////////////////////////////////////////////////////////////

	VkSurfaceFormatKHR LveSwapchain::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		auto it = std::find_if(availableFormats.begin(), availableFormats.end(), [](VkSurfaceFormatKHR surfaceFormat) {
			return surfaceFormat.format == VK_FORMAT_B8G8R8A8_SRGB && surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
		});

		if (it != availableFormats.end())
		{
			return *it;
		}

		ASSERT(!availableFormats.empty(), "Failed to pick available format!");
		return availableFormats[0];
	}

	VkPresentModeKHR LveSwapchain::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
	{
		// 1. FIFO is the default mode. If GPU finishes very fast, it would become idle if no more back-buffer is available.
		// This will also increase the latency between the frame is being rendered and the frame is actually being shown.
		// 2. Mailbox will make the GPU busy instead by discarding the back-buffer content and re-draw. Swapchain will pick
		// the one that is most recently being drawn, which will decrease the latency but increase power consumption.
		// 3. Immediate mode does not do any synchronization, which will cause high CPU and GPU usage and tearing.
		auto mailboxIt = std::find_if(availablePresentModes.begin(), availablePresentModes.end(),
									  [](VkPresentModeKHR presentMode) { return presentMode == VK_PRESENT_MODE_MAILBOX_KHR; });

		// auto immediateIt = std::find_if(availablePresentModes.begin(), availablePresentModes.end(),
		// 								[](VkPresentModeKHR presentMode) { return presentMode == VK_PRESENT_MODE_IMMEDIATE_KHR; });

		if (mailboxIt != availablePresentModes.end())
		{
			PRINT("Present mode: Mailbox");
			return *mailboxIt;
		}
		// else if (immediateIt != availablePresentModes.end())
		// {
		// 	PRINT("Present mode: Immediate");
		// 	return *immediateIt;
		// }
		else
		{
			PRINT("Present mode: V-Sync");
			return VK_PRESENT_MODE_FIFO_KHR;
		}
	}

	VkExtent2D LveSwapchain::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		if (capabilities.currentExtent.width != std::numeric_limits<U32>::max())
		{
			PRINT("Current extent: %d x %d", capabilities.currentExtent.width, capabilities.currentExtent.height);
			return capabilities.currentExtent;
		}

		// Window manager allows us to change the bounds here.

		// Maybe change to get pixel size instead?
		//
		// int widthInPixel{}, heightInPixel{};
		// glfwGetFramebufferSize(mGlfwWindow, &widthInPixel, &heightInPixel);
		// PRINT("GLFW framebuffer size: (%dx%d)", widthInPixel, heightInPixel);

		VkExtent2D actualExtent = mWindowExtent;
		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		PRINT("Image extent width range:  [%d, %d]", capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		PRINT("Image extent height range: [%d, %d]", capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}

	VkImageView LveSwapchain::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
	{
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		VkImageView imageView;
		VkResult result = vkCreateImageView(mDevice.GetDevice(), &viewInfo, nullptr, &imageView);
		ASSERT_EQ(result, VK_SUCCESS, "Failed to create image view!");

		return imageView;
	}

}  // namespace lve
