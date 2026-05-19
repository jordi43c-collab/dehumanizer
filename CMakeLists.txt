cmake_minimum_required(VERSION 3.20)
project(Dehumanizer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fetch Raylib so it compiles automatically without needing pre-installation
include(FetchContent)
FetchContent_Declare(
  raylib
  URL https://github.com/raysan5/raylib/archive/refs/tags/5.0.tar.gz
)
FetchContent_MakeAvailable(raylib)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)

# Mute some raylib warnings if compiling with MSVC
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX-)
endif()
