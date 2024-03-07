cmake_minimum_required(VERSION 3.27)
project(learn-vulkan)

set(CMAKE_CXX_STANDARD 17)

add_executable(learn-vulkan)

target_sources(learn-vulkan
PRIVATE
	main.cpp
)
