# CMakeList.txt : CMake project for Voxel simulation, include source and define
# project specific logic here.
#

file(GLOB SRC_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

# Add source to this project's executable.
add_executable (CMakeTarget 
	${SRC_FILES}
    "Voxel simulation.cpp"
	glad.c
    "mesh.cpp" "Shader.cpp" "Texture.cpp" "Camera.cpp" "Perlin.cpp" "Chunk.cpp"
    ${CMAKE_SOURCE_DIR}/include/imgui-master/imgui.cpp
    ${CMAKE_SOURCE_DIR}/include/imgui-master/imgui_draw.cpp
    ${CMAKE_SOURCE_DIR}/include/imgui-master/imgui_tables.cpp
    ${CMAKE_SOURCE_DIR}/include/imgui-master/imgui_widgets.cpp
    ${CMAKE_SOURCE_DIR}/include/imgui-master/backends/imgui_impl_sdl3.cpp
    ${CMAKE_SOURCE_DIR}/include/imgui-master/backends/imgui_impl_opengl3.cpp
  "ThreadPool.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET CMakeTarget PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
target_link_libraries(CMakeTarget PRIVATE SDL3::SDL3)

find_package(OpenGL REQUIRED)
target_link_libraries(CMakeTarget PRIVATE OpenGL::GL)

target_include_directories(CMakeTarget PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/imgui-master
    ${CMAKE_SOURCE_DIR}/include/imgui-master/backends
)
