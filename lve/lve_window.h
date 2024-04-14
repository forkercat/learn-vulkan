//
// Created by Junhao Wang (@forkercat) on 4/1/24.
//

#pragma once

#include "core/core.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

namespace lve {

	class LveWindow
	{
	public:
		LveWindow(U32 width, U32 height, std::string name);
		~LveWindow();

		LveWindow(const LveWindow&) = delete;
		LveWindow& operator=(const LveWindow&) = delete;

		bool ShouldClose() { return glfwWindowShouldClose(mGlfwWindow); }
		bool WasWindowResized() { return mFramebufferResized; }
		void ResetWindowResizedFlag() { mFramebufferResized = false; }

		void CreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

		VkExtent2D GetExtent() { return { static_cast<U32>(mWidth), static_cast<U32>(mHeight) }; }
		U32 GetWidth() { return mWidth; }
		U32 GetHeight() { return mHeight; }

	private:
		void InitWindow();

		static void FrameBufferResizeCallback(GLFWwindow* window, int width, int height);

	private:
		U32 mWidth;
		U32 mHeight;
		bool mFramebufferResized = false;

		std::string mWindowName;
		GLFWwindow* mGlfwWindow;
	};

}  // namespace lve
