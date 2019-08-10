cmake_minimum_required(VERSION 3.5)
project(Honeycomb)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 17)

# gl3w - use newer OpenGL extensions
include_directories("externals/gl3w/include")
set(GL3W_SOURCES "externals/gl3w/src/gl3w.c")

# glfw - platform-independant window creation & input processing
#set(GLFW_USE_WAYLAND ON CACHE BOOL "Use Wayland instead of X11")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "Disable GLFW doc")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "Disable GLFW tests")
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Disable GLFW examples")
add_subdirectory("externals/glfw" glfw-build)

# glm - math library
set(GLM_TEST_ENABLE OFF CACHE BOOL "Disable GLM tests")
add_subdirectory("externals/glm" glm-build)

# stb - image library
include_directories("externals/stb")

# core files
include_directories("source/core")
file(GLOB CORE_SOURCES "source/core/*.cpp")

# graphics files
include_directories("source/graphics")
file(GLOB GRAPHICS_SOURCES "source/graphics/*.cpp")

# memory files
include_directories("source/memory")
file(GLOB MEMORY_SOURCE "source/memory/*.cpp")

# storage files
include_directories("source/storage")
file(GLOB STORAGE_SOURCES "source/storage/*.cpp")

# Application : Heat Diffusion
file(GLOB DIFFUSION_SOURCES "source/app/diffusion/*.cpp")
add_executable(Diffusion.exe
        ${DIFFUSION_SOURCES}
        ${GL3W_SOURCES}
        ${CORE_SOURCES}
        ${GRAPHICS_SOURCES}
        ${MEMORY_SOURCES}
        ${STORAGE_SOURCES})
target_include_directories(Diffusion.exe PUBLIC "source/app/diffusion")
target_link_libraries(Diffusion.exe glfw glm)

# Application : Two chemical Reaction-Diffusion
file(GLOB REACTION_DIFFUSION_SOURCES "source/app/reaction_diffusion/*.cpp")
add_executable(ReactionDiffusion.exe
        ${REACTION_DIFFUSION_SOURCES}
        ${GL3W_SOURCES}
        ${CORE_SOURCES}
        ${GRAPHICS_SOURCES}
        ${MEMORY_SOURCES}
        ${STORAGE_SOURCES})
target_include_directories(ReactionDiffusion.exe PUBLIC "source/app/reaction_diffusion")
target_link_libraries(ReactionDiffusion.exe glfw glm)

# Application : Texture generation with compute shader
file(GLOB COMPUTE_TEXTURE_SOURCES "source/app/compute_texture/*.cpp")
add_executable(ComputeTexture.exe
        ${COMPUTE_TEXTURE_SOURCES}
        ${GL3W_SOURCES}
        ${CORE_SOURCES}
        ${GRAPHICS_SOURCES}
        ${MEMORY_SOURCES}
        ${STORAGE_SOURCES})
target_include_directories(ComputeTexture.exe PUBLIC "source/app/compute_texture")
target_link_libraries(ComputeTexture.exe glfw glm)

# Application : GPU Boid Particles
file(GLOB PARTICLES_SOURCES "source/app/particles/*.cpp")
add_executable(Particles.exe
        ${PARTICLES_SOURCES}
        ${GL3W_SOURCES}
        ${CORE_SOURCES}
        ${GRAPHICS_SOURCES}
        ${MEMORY_SOURCES}
        ${STORAGE_SOURCES})
target_include_directories(Particles.exe PUBLIC "source/app/particles")
target_link_libraries(Particles.exe glfw glm)
