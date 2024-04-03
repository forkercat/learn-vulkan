//
// Created by Junhao Wang (@forkercat) on 3/22/24.
//

#include "hello_triangle.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>

int main()
{
	HelloTriangleApplication app;

	app.Run();

	return EXIT_SUCCESS;
}
