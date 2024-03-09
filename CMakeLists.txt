cmake_minimum_required(VERSION 3.27)
project(learn-vulkan)

set(CMAKE_CXX_STANDARD 17)

find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(Vulkan REQUIRED)

add_executable(learn-vulkan)

target_sources(learn-vulkan
PRIVATE
	main.cpp
)

target_link_libraries(learn-vulkan
PRIVATE
	glfw
	glm::glm
	Vulkan::Vulkan
)
