set(SRC_ROOT ${PROJECT_SOURCE_DIR}/core)

add_library(core)

target_sources(core
PUBLIC
	${SRC_ROOT}/core.h
	${SRC_ROOT}/logging.h
	${SRC_ROOT}/typedefs.h
	${SRC_ROOT}/uassert.h
PRIVATE
	${SRC_ROOT}/core.cpp
)

target_include_directories(core
PRIVATE # Do not populate the root dir to main or other modules.
	${PROJECT_SOURCE_DIR}
)
