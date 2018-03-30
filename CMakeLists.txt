cmake_minimum_required(VERSION 2.8)

set(RELEASE_OPTIONS "-O3" "-fno-exceptions" "-std=c++17")
set(DEBUG_OPTIONS "-std=c++17")

list(APPEND SOURCE_FILES "src/main.cpp")
list(APPEND SOURCE_FILES "src/application.cpp")
list(APPEND SOURCE_FILES "src/window/window.cpp")
list(APPEND SOURCE_FILES "src/scene/scene.cpp")
list(APPEND SOURCE_FILES "src/scene/scene_loader.cpp")
list(APPEND SOURCE_FILES "src/core/io.cpp")
list(APPEND SOURCE_FILES "src/vulkan/vulkan.cpp")
list(APPEND SOURCE_FILES "src/error.cpp")

include_directories("src")
include_directories("third_party")
include_directories("third_party/glfw3/include/GLFW")
include_directories("src/core")

set(BUILD_SHARED_LIBS "NO")
set(GLFW_BUILD_EXAMPLES "NO")
set(GLFW_BUILD_TESTS "NO")
set(GLFW_BUILD_DOCS "NO")
set(GLFW_VULKAN_STATIC "NO")
add_subdirectory("third_party/glfw3")
link_directories("third_party/glfw3")

project(bpmap)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:DEFAULT>:${RELEASE_OPTIONS}>")
target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:DEBUG>:${DEBUG_OPTIONS}>")
target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:RELEASE>:${RELEASE_OPTIONS}>")
target_link_libraries(${PROJECT_NAME} "glfw")
target_link_libraries(${PROJECT_NAME} "vulkan")

