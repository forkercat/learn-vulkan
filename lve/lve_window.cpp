//
// Created by Junhao Wang (@forkercat) on 4/1/24.
//

#include "lve_window.h"

namespace lve {

	LveWindow::LveWindow(U32 width, U32 height, std::string name)
		: mWidth(width), mHeight(height), mWindowName(name)
	{
		InitWindow();
	}

	LveWindow::~LveWindow()
	{
		glfwDestroyWindow(mGlfwWindow);
		glfwTerminate();
	}

	void LveWindow::InitWindow()
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		mGlfwWindow = glfwCreateWindow((int)mWidth, (int)mHeight, mWindowName.c_str(), nullptr, nullptr);

		ASSERT(mGlfwWindow, "Failed to create glfw window!");
	}

	void LveWindow::CreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface)
	{
		VkResult result = glfwCreateWindowSurface(instance, mGlfwWindow, nullptr, surface);
		ASSERT_EQ(result, VK_SUCCESS, "Failed to create a window surface for Vulkan!");
	}

}  // namespace lve
