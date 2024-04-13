add_executable(lve-app)

target_sources(lve-app
PRIVATE
	lve_window.cpp
	lve_device.cpp
	lve_pipeline.cpp
	lve_swapchain.cpp
	first_app.cpp
	main.cpp
PUBLIC
	lve_window.h
	lve_device.h
	lve_pipeline.h
	lve_swapchain.h
	first_app.h
)

target_link_libraries(lve-app
PRIVATE
	glfw
	Vulkan::Vulkan
	core
)

target_include_directories(lve-app
PRIVATE
	${PROJECT_SOURCE_DIR}
)
