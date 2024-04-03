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

		bool ShouldClose() { return glfwWindowShouldClose(mWindow); }

		void CreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

		U32 GetWidth() { return mWidth; }
		U32 GetHeight() { return mHeight; }

	private:
		void InitWindow();

	private:
		const U32 mWidth;
		const U32 mHeight;

		std::string mWindowName;
		GLFWwindow* mWindow;
	};

}  // namespace lve
