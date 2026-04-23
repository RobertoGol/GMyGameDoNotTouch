cmake_minimum_required(VERSION 3.16)
project(BunkerProtocolSuite LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
enable_testing()

find_package(OpenGL REQUIRED)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
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
    ${CMAKE_CURRENT_SOURCE_DIR}/external/glfw/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/backends
)

function(configure_bunker_target target_name)
    target_include_directories(${target_name} PRIVATE ${COMMON_INCLUDE_DIRS})
    target_link_libraries(${target_name} PRIVATE glfw OpenGL::GL)
    if(MSVC)
        target_compile_options(${target_name} PRIVATE /W4 /permissive- /Zm300)
    else()
        target_compile_options(${target_name} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endfunction()

add_executable(BunkerGame
    src/GameplayDescriptorRegistry.cpp
    src/GameRuntime.cpp
    src/GameRuntimePipPad.cpp
    src/LanlineServices.cpp
    src/LanlineSession.cpp
    src/LaunchSession.cpp
    src/main.cpp
    src/Progression.cpp
    src/SessionProfiles.cpp
    src/Renderer.cpp
    src/SkillSystem.cpp
    src/StoryRoute.cpp
    src/WorldEvents.cpp
    src/World.cpp
    src/WorldSemanticAuthoring.cpp
    src/WorldValidation.cpp
    ${IMGUI_SRC}
)
configure_bunker_target(BunkerGame)

add_executable(BunkerLauncher
    Launcher/src/Launcher_Main.cpp
    Launcher/src/LauncherSupport.cpp
    src/GameplayDescriptorRegistry.cpp
    src/LanlineServices.cpp
    src/LanlineSession.cpp
    src/LaunchSession.cpp
    src/SessionProfiles.cpp
    src/StoryRoute.cpp
    ${IMGUI_SRC}
)
configure_bunker_target(BunkerLauncher)

add_executable(BunkerEditor
    Editor/src/Editor_Main.cpp
    Editor/src/EditorSupport.cpp
    src/PrefabLibrary.cpp
    src/GameplayDescriptorRegistry.cpp
    src/SessionProfiles.cpp
    src/World.cpp
    src/WorldEditorUndo.cpp
    src/WorldExport.cpp
    src/WorldSemanticAuthoring.cpp
    src/WorldValidation.cpp
    ${IMGUI_SRC}
)
configure_bunker_target(BunkerEditor)

add_executable(BunkerSmokeChecks
    tests/SmokeChecks.cpp
    src/GameRuntime.cpp
    src/GameRuntimePipPad.cpp
    src/PrefabLibrary.cpp
    src/GameplayDescriptorRegistry.cpp
    src/LanlineServices.cpp
    src/LanlineSession.cpp
    src/LaunchSession.cpp
    src/Progression.cpp
    src/Renderer.cpp
    src/SessionProfiles.cpp
    src/SkillSystem.cpp
    src/StoryRoute.cpp
    src/World.cpp
    src/WorldEditorUndo.cpp
    src/WorldEvents.cpp
    src/WorldExport.cpp
    src/WorldSemanticAuthoring.cpp
    src/WorldValidation.cpp
    ${IMGUI_SRC}
)
configure_bunker_target(BunkerSmokeChecks)
add_test(NAME BunkerSmokeChecks COMMAND BunkerSmokeChecks)

message(STATUS "Configured targets: BunkerGame, BunkerLauncher, BunkerEditor")
