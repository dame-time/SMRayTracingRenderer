cmake_minimum_required(VERSION 3.30)
project(SMRayTracingRenderer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_definitions(-DPROJECT_SOURCE_DIR="${CMAKE_SOURCE_DIR}")

add_subdirectory(external/assimp)
add_subdirectory(external/SphereMeshBlendShape)

include_directories(external/assimp external/glm)

set(GLM_DIR "external/glm"
        src/rendering/BumperGraphRenderer.cpp
        src/rendering/BumperGraphRenderer.hpp)

find_package(Qt6 COMPONENTS
    Core
    Gui
    Widgets
    OpenGLWidgets
  REQUIRED)

add_executable(SMRayTracingRenderer main.cpp
        src/rendering/Window.cpp
        src/rendering/Window.hpp
        src/rendering/Renderer.cpp
        src/rendering/Renderer.hpp
        ${GLM_DIR}
        src/rendering/Camera.cpp
        src/rendering/Camera.hpp
        src/rendering/Shader.cpp
        src/rendering/Shader.hpp
)

target_link_libraries(SMRayTracingRenderer
    Qt::Core
    Qt::Gui
    Qt::Widgets
    Qt6::OpenGLWidgets
    assimp
    SphereMeshBlendShape
)

target_compile_options(SMRayTracingRenderer PRIVATE
        -O0
        -g
        -Wall
)

