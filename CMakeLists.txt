cmake_minimum_required(VERSION 2.8)

project(bpmap)

if((DEFINED CMAKE_BUILD_TYPE))
    if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
        add_definitions(-DBPMAP_DEBUG)
    endif()
endif()

set(RELEASE_OPTIONS "-O3" "-fno-exceptions" "-std=c++17")
set(DEBUG_OPTIONS "-g" "-O0" "-std=c++17")

list(APPEND SOURCE_FILES "src/main.cpp")
list(APPEND SOURCE_FILES "src/application.cpp")
list(APPEND SOURCE_FILES "src/window/window.cpp")
list(APPEND SOURCE_FILES "src/scene/scene.cpp")
list(APPEND SOURCE_FILES "src/scene/scene_loader.cpp")
list(APPEND SOURCE_FILES "src/core/io.cpp")
list(APPEND SOURCE_FILES "src/vulkan/vulkan.cpp")
list(APPEND SOURCE_FILES "src/vulkan/gui_renderer.cpp")
list(APPEND SOURCE_FILES "src/vulkan/renderer.cpp")
list(APPEND SOURCE_FILES "src/gui/gui.cpp")
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


execute_process(COMMAND "glslangValidator" "-V" "-o" "${PROJECT_BINARY_DIR}/gui.vert.spv" "gui.vert" WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/src/gpu" OUTPUT_VARIABLE ov0)
execute_process(COMMAND "glslangValidator" "-V" "-o" "${PROJECT_BINARY_DIR}/gui.frag.spv" "gui.frag" WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/src/gpu" OUTPUT_VARIABLE ov1)
execute_process(COMMAND "glslangValidator" "-V" "-o" "${PROJECT_BINARY_DIR}/raytrace.comp.spv" "raytrace.comp" WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/src/gpu" OUTPUT_VARIABLE ov2)
execute_process(COMMAND "glslangValidator" "-V" "-o" "${PROJECT_BINARY_DIR}/render_output.vert.spv" "render_output.vert" WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/src/gpu" OUTPUT_VARIABLE ov3)
execute_process(COMMAND "glslangValidator" "-V" "-o" "${PROJECT_BINARY_DIR}/render_output.frag.spv" "render_output.frag" WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/src/gpu" OUTPUT_VARIABLE ov4)
message("Compiled ${ov0}.")
message("Compiled ${ov1}.")
message("Compiled ${ov2}.")
message("Compiled ${ov3}.")
message("Compiled ${ov4}.")

add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:DEFAULT>:${RELEASE_OPTIONS}>")
target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:DEBUG>:${DEBUG_OPTIONS}>")
target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:RELEASE>:${RELEASE_OPTIONS}>")
target_link_libraries(${PROJECT_NAME} "glfw")
target_link_libraries(${PROJECT_NAME} "vulkan")


