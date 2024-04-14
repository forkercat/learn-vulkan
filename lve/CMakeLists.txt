add_executable(lve-app)

target_sources(lve-app
PRIVATE
	lve_window.cpp
	lve_device.cpp
	lve_pipeline.cpp
	lve_swapchain.cpp
	lve_model.cpp
	lve_renderer.cpp
	lve_game_object.cpp
	first_app.cpp
	main.cpp
PUBLIC
	lve_window.h
	lve_device.h
	lve_pipeline.h
	lve_swapchain.h
	lve_model.h
	lve_renderer.h
	lve_game_object.h
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

add_custom_target(
	compile-shaders ALL
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/shaders
	COMMAND ${CMAKE_COMMAND} -E echo "Running pre-built script to compile shaders..."
	COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/shaders/compile.sh
	COMMENT "Executing pre-built script..."
)

add_dependencies(lve-app compile-shaders)
