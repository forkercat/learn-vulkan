set(SRC_ROOT ${PROJECT_SOURCE_DIR}/playground)

add_executable(playground)

target_sources(playground
PRIVATE
	${SRC_ROOT}/main.cpp
)

target_link_libraries(playground
PRIVATE
	glfw
	glm::glm
	Vulkan::Vulkan
	core
)

target_include_directories(playground
PRIVATE
	${PROJECT_SOURCE_DIR}
)
