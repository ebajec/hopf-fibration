cmake_minimum_required(VERSION 3.12)
set(PROJECT_NAME "hopf")
list(APPEND CMAKE_MODULE_PATH $ENV{CMAKE_MODULE_PATH})
project(${PROJECT_NAME})

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)

# Set cache variables for GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

# External dependencies
set(EXTERNAL_DEPS 
    external/linear-algebra/lib 
    external/imgui)

set(EXTERNAL_SOURCES
    "external/imgui/*.cpp"
    "external/imgui/backends/imgui_impl_glfw.cpp"
    "external/imgui/backends/imgui_impl_opengl3.cpp"
    )

include_directories(
    ${EXTERNAL_DEPS}
    ${EXTERNAL_DEPS}/*.h 
    graphics/include
    simulation/include
    util
    )

# Find source files
file(GLOB SOURCES 
    "graphics/src/*.cpp" 
    "simulation/src/*.cpp"
    ${EXTERNAL_SOURCES}
)

# Add source files
add_executable(${PROJECT_NAME} ${SOURCES})

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(GLEW_LIB ${CMAKE_SOURCE_DIR}/lib/glew32.lib)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(GLEW_LIB GLEW)
else ()
endif ()


find_package(OpenGL REQUIRED)

include(FetchContent)

FetchContent_Declare(
	glm
	GIT_REPOSITORY	https://github.com/g-truc/glm.git
	GIT_TAG 	bf71a834948186f4097caa076cd2663c69a10e1e #refs/tags/1.0.1
)
FetchContent_MakeAvailable(glm)

target_link_libraries(${PROJECT_NAME} glm::glm)
target_link_libraries(${PROJECT_NAME} glfw)
target_link_libraries(${PROJECT_NAME} ${GLEW_LIB})
target_link_libraries(${PROJECT_NAME} OpenGL::GL)


