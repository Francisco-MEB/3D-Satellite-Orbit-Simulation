cmake_minimum_required(VERSION 3.10)
project(CubeSatSim)

set(CMAKE_CXX_STANDARD 17)

# Include directories
include_directories(include)
include_directories(/opt/homebrew/include)  # macOS Homebrew
include_directories(${CMAKE_SOURCE_DIR}/assimp/include)
include_directories(${CMAKE_SOURCE_DIR}/include/freetype2/include)

# Link directories
link_directories(/opt/homebrew/lib)
link_directories(${CMAKE_SOURCE_DIR}/assimp/build/lib)

# Find and link FreeType
find_package(Freetype REQUIRED)

# Source files
add_executable(CubeSatSim
    src/main.cpp
    src/functions_main.cpp
    src/stb_image.cpp
    src/glad.c
    src/sphere.cpp
    src/cube.cpp 
    src/reaction_wheel.cpp
    src/reaction_wheel_system.cpp
    src/attitude.cpp
    src/nadir_controller.cpp
    src/text_renderer.cpp
    src/camera_controller.cpp
    src/telemetry_display.cpp
)

# Link libraries
target_link_libraries(CubeSatSim
    assimp
    glfw
    ${FREETYPE_LIBRARIES} 
    "-framework OpenGL"
    z
)

target_compile_definitions(CubeSatSim PRIVATE PROJECT_SOURCE_DIR="${CMAKE_SOURCE_DIR}")

add_custom_command(TARGET CubeSatSim POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/include/shaders
            $<TARGET_FILE_DIR:CubeSatSim>/shaders
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/resources/textures
            $<TARGET_FILE_DIR:CubeSatSim>/textures
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/resources/skyboxes
            $<TARGET_FILE_DIR:CubeSatSim>/skyboxes
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/include/fonts
            $<TARGET_FILE_DIR:CubeSatSim>/fonts
)

