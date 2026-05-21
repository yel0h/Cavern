cmake_minimum_required(VERSION 3.30)
project(Cavern)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

include(FetchContent)

FetchContent_Declare(glfw URL https://github.com/glfw/glfw/releases/download/3.4/glfw-3.4.zip)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

FetchContent_Declare(glm URL https://github.com/g-truc/glm/releases/download/1.0.1/glm-1.0.1-light.zip)
FetchContent_MakeAvailable(glm)

add_library(glad STATIC glad/src/glad.c)
target_include_directories(glad PUBLIC glad/include)

add_executable(Cavern src/main.cpp
        src/world/Block.hpp
        src/world/Chunk.hpp
        src/world/World.cpp
        src/world/World.hpp
        src/world/WorldGen.cpp
        src/world/WorldGen.hpp
        src/world/Lighting.cpp
        src/world/Lighting.hpp
        src/render/Shader.cpp
        src/render/Shader.hpp
        src/render/TextureAtlas.cpp
        src/render/TextureAtlas.hpp
        src/render/Camera.cpp
        src/render/Camera.hpp
        src/render/ChunkMesh.cpp
        src/render/ChunkMesh.hpp
        src/render/Renderer.cpp
        src/render/Renderer.hpp
        src/player/Physics.hpp
        src/player/Player.cpp
        src/player/Player.hpp
        src/core/Input.cpp
        src/core/Input.hpp
        src/core/Timer.hpp
        src/core/Game.cpp
        src/core/Game.hpp
)
target_include_directories(Cavern PRIVATE ${CMAKE_SOURCE_DIR})
target_compile_definitions(Cavern PRIVATE GLFW_INCLUDE_NONE)
target_link_libraries(Cavern PRIVATE glfw glad glm::glm)

if(WIN32)
    target_link_libraries(Cavern PRIVATE opengl32)
endif()
