set(SRC_ROOT ${PROJECT_SOURCE_DIR}/tutorial)

add_executable(tutorial)

target_sources(tutorial
PUBLIC
	${SRC_ROOT}/hello_triangle.h
	${SRC_ROOT}/shader.h
PRIVATE
	${SRC_ROOT}/hello_triangle.cpp
	${SRC_ROOT}/main.cpp
)

target_link_libraries(tutorial
PRIVATE
	glfw
	Vulkan::Vulkan
	core
)

target_include_directories(tutorial
PRIVATE
	${PROJECT_SOURCE_DIR}
)

# TestApp
add_executable(TestApp)

target_sources(TestApp
PRIVATE
	test_app.cpp
)

target_link_libraries(TestApp
PRIVATE
	glfw
	Vulkan::Vulkan
	glm::glm
)
