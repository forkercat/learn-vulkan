add_executable(lve-app)

target_sources(lve-app
PRIVATE
	lve_window.cpp
	lve_device.cpp
	lve_pipeline.cpp
	lve_swapchain.cpp
	lve_buffer.cpp
	lve_model.cpp
	lve_renderer.cpp
	lve_descriptors.cpp
	lve_game_object.cpp
	lve_camera.cpp
	keyboard_movement_controller.cpp
	system/simple_render_system.cpp
	first_app.cpp
	main.cpp
PUBLIC
	lve_window.h
	lve_device.h
	lve_pipeline.h
	lve_swapchain.h
	lve_buffer.h
	lve_model.h
	lve_renderer.h
	lve_descriptors.h
	lve_frame_info.h
	lve_game_object.h
	lve_camera.h
	lve_utils.h
	keyboard_movement_controller.h
	system/simple_render_system.h
	system/rainbow_system.h
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
