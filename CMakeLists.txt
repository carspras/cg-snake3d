cmake_minimum_required (VERSION 2.8) # CMake script created and tested for 2.8.12, use another version at own responsibility

project (aufgabe2)

find_package(OpenGL REQUIRED)

FILE(GLOB currentSources RELATIVE "${PROJECT_SOURCE_DIR}" "*.cpp" "*.h" "*.glsl" "shader/*.glsl")

include_directories(${PROJECT_SOURCE_DIR})

IF(APPLE)
  ADD_DEFINITIONS("-ObjC++")
  SET(EXTRA_LINKER_FLAGS "-framework Cocoa -framework OpenGL")
ENDIF()

add_executable(${PROJECT_NAME}
	${currentSources}
)

target_link_libraries(${PROJECT_NAME}
	${OPENGL_LIBRARY}
	${COCOA_LIBRARY}
	${EXTRA_LINKER_FLAGS}
)

IF (WIN32)
	# Windows - unicode
	ADD_DEFINITIONS( "-D UNICODE" )
	ADD_DEFINITIONS( "-D _UNICODE" )
ENDIF()
