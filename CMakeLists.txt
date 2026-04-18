cmake_minimum_required(VERSION 3.16)
project(BunkerProtocolSuite LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenGL REQUIRED)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(external/glfw)

set(IMGUI_SRC
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/backends/imgui_impl_glfw.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp
)

set(COMMON_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external
    ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/backends
)

function(configure_bunker_target target_name)
    target_include_directories(${target_name} PRIVATE ${COMMON_INCLUDE_DIRS})
    target_link_libraries(${target_name} PRIVATE glfw OpenGL::GL)
    if(MSVC)
        target_compile_options(${target_name} PRIVATE /W4 /permissive-)
    else()
        target_compile_options(${target_name} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endfunction()

add_executable(BunkerGame
    src/GameRuntime.cpp
    src/LanlineServices.cpp
    src/LanlineSession.cpp
    src/LaunchSession.cpp
    src/main.cpp
    src/Progression.cpp
    src/Renderer.cpp
    src/SkillSystem.cpp
    src/StoryRoute.cpp
    src/WorldEvents.cpp
    src/World.cpp
    ${IMGUI_SRC}
)
configure_bunker_target(BunkerGame)

add_executable(BunkerLauncher
    Launcher/src/Launcher_Main.cpp
    src/LanlineServices.cpp
    src/LanlineSession.cpp
    src/LaunchSession.cpp
    ${IMGUI_SRC}
)
configure_bunker_target(BunkerLauncher)

add_executable(BunkerEditor
    Editor/src/Editor_Main.cpp
    src/World.cpp
    ${IMGUI_SRC}
)
configure_bunker_target(BunkerEditor)

message(STATUS "Configured targets: BunkerGame, BunkerLauncher, BunkerEditor")
