cmake_minimum_required(VERSION 3.16)
project(hello)

# set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

# This assumes the SDL source is available in vendored/SDL
add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)

# @TODO: see if there's a more "proper" way to link 8080.cc
file(GLOB SOURCES src/sdl_8080.cc src/8080.cc)
# Create your game executable target as usual
add_executable(sdl_8080 WIN32 ${SOURCES})

# Link to the actual SDL3 library.
target_link_libraries(sdl_8080 PRIVATE SDL3::SDL3)
