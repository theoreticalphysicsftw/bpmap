# Copyright 2018-2023 Mihail Mladenov
#
# This file is part of bpmap.
#
# bpmap is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# bpmap is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with bpmap.  If not, see <http://www.gnu.org/licenses/>.


cmake_minimum_required(VERSION 3.5)

project(bpmap)
set(CXX_STANDARD 20)

if((DEFINED CMAKE_BUILD_TYPE))
    if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
        add_definitions(-DBPMAP_DEBUG)
    endif()
endif()

set(RELEASE_OPTIONS "-O3" "-fno-exceptions" "-std=c++17")
set(DEBUG_OPTIONS "-g" "-O0" "-std=c++17")

file(
      GLOB
      SOURCE_FILES
      "src/*"
      "src/window/*"
      "src/scene/*"
      "src/core/*"
      "src/vulkan/*"
      "src/gui/*"
    )

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

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif(NOT CMAKE_BUILD_TYPE)

if(NOT (DEFINED DONT_COPY_TEST_SCENE))
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_scene/b5.obj ${CMAKE_CURRENT_BINARY_DIR}/b5.obj COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_scene/b5.mtl ${CMAKE_CURRENT_BINARY_DIR}/b5.mtl COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_scene/scene.bpmap ${CMAKE_CURRENT_BINARY_DIR}/scene.bpmap COPYONLY)
endif()

add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:Debug>:${DEBUG_OPTIONS}>")
target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:Release>:${RELEASE_OPTIONS}>")
target_link_libraries(${PROJECT_NAME} "glfw")
target_link_libraries(${PROJECT_NAME} "vulkan")


