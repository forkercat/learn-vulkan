set(SRC_ROOT ${PROJECT_SOURCE_DIR}/playground)

add_executable(playground)

target_sources(playground
PRIVATE
	${SRC_ROOT}/print.cpp
	${SRC_ROOT}/app.cpp
	${SRC_ROOT}/main.cpp
PUBLIC
	${SRC_ROOT}/print.h
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
