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

# Базовые файлы ImGui без привязки к графическому API
set(IMGUI_CORE_SRC
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/backends/imgui_impl_glfw.cpp
)

# Бэкенд для утилит, работающих на OpenGL (Лаунчеры)
set(OPENGL_IMGUI_SRC
    ${IMGUI_CORE_SRC}
    external/imgui/backends/imgui_impl_opengl3.cpp
)

# Бэкенд для ядра игры и редактора, работающих на DirectX 11
set(DX11_IMGUI_SRC
    ${IMGUI_CORE_SRC}
    external/imgui/backends/imgui_impl_dx11.cpp
    Dx11Renderer.cpp
)

set(COMMON_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external
    ${CMAKE_CURRENT_SOURCE_DIR}/external/glfw/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/backends
)

# Общая конфигурация флагов компилятора для всех подсистем бункера
function(configure_bunker_target target_name)
    target_include_directories(${target_name} PRIVATE ${COMMON_INCLUDE_DIRS})
    target_link_libraries(${target_name} PRIVATE glfw OpenGL::GL)
    if(MSVC)
        target_compile_options(${target_name} PRIVATE /W4 /permissive- /Zm300 /O2 /MT)
    else()
        target_compile_options(${target_name} PRIVATE -Wall -Wextra -Wpedantic -O3)
    endif()
endfunction()

# ==============================================================================
# 🎮 1. ОСНОВНАЯ ИГРА: BUNKER GAME (СТРОГО DX11)
# ==============================================================================
add_executable(BunkerGame
    ${DX11_IMGUI_SRC}
    src/GameExecution.cpp
    src/GameplayDescriptorRegistry.cpp
    src/GameRuntime.cpp
    src/GameRuntimePipPad.cpp
    src/LanlineServices.cpp
    src/LanlineSession.cpp
    src/LaunchSession.cpp
    src/PrefabLibrary.cpp
    src/ProceduralAnimation.cpp
    src/main.cpp
    src/Progression.cpp
    src/SessionProfiles.cpp
    src/Renderer.cpp
    src/SkillSystem.cpp
    src/StoryRoute.cpp
    src/WorldEvents.cpp
    src/World.cpp
    src/WorldExport.cpp
    src/WorldSemanticAuthoring.cpp
    src/WorldValidation.cpp
)
configure_bunker_target(BunkerGame)
target_link_libraries(BunkerGame PRIVATE d3d11 dxgi d3dcompiler)

# ==============================================================================
# 🚀 2. СИСТЕМНЫЙ ЛАУНЧЕР: BUNKER LAUNCHER (OPENGL)
# ==============================================================================
add_executable(BunkerLauncher
    Launcher/src/Launcher_Main.cpp
    Launcher/src/LauncherSupport.cpp
    src/GameplayDescriptorRegistry.cpp
    src/LanlineServices.cpp
    src/LanlineSession.cpp
    src/LaunchSession.cpp
    src/SessionProfiles.cpp
    src/StoryRoute.cpp
    ${OPENGL_IMGUI_SRC}
)
configure_bunker_target(BunkerLauncher)

# ==============================================================================
# 👑 3. АДМИН-ПАНЕЛЬ: BUNKER ADMIN LAUNCHER (OPENGL)
# ==============================================================================
add_executable(BunkerAdminLauncher
    Launcher/src/AdminLauncher_Main.cpp
    Launcher/src/LauncherSupport.cpp
    src/GameplayDescriptorRegistry.cpp
    src/LanlineServices.cpp
    src/LanlineSession.cpp
    src/LaunchSession.cpp
    src/SessionProfiles.cpp
    src/StoryRoute.cpp
    src/World.cpp
    src/WorldSemanticAuthoring.cpp
    src/WorldValidation.cpp
    ${OPENGL_IMGUI_SRC}
)
configure_bunker_target(BunkerAdminLauncher)

# ==============================================================================
# 🛠️ 4. РЕДАКТОР КАРТ: BUNKER EDITOR (СТРОГО DX11)
# ==============================================================================
add_executable(BunkerEditor
    ${DX11_IMGUI_SRC}
    Editor/src/Editor_Main.cpp
    Editor/src/EditorSupport.cpp
    src/PrefabLibrary.cpp
    src/ProceduralAnimation.cpp
    src/GameplayDescriptorRegistry.cpp
    src/SessionProfiles.cpp
    src/World.cpp
    src/WorldEditorUndo.cpp
    src/WorldExport.cpp
    src/WorldSemanticAuthoring.cpp
    src/WorldValidation.cpp
)
configure_bunker_target(BunkerEditor)
target_link_libraries(BunkerEditor PRIVATE d3d11 dxgi d3dcompiler)

# ==============================================================================
# 🧪 5. ТЕСТОВЫЙ СТЕНД: SMOKE CHECKS (DX11/OPENGL ГИБРИДНЫЙ ТЕСТ)
# ==============================================================================
add_executable(BunkerSmokeChecks
    tests/SmokeChecks.cpp
    Editor/src/EditorSupport.cpp
    src/GameExecution.cpp
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
    ${DX11_IMGUI_SRC} # Тесты проверяют DX11 рендерер ядра
)
configure_bunker_target(BunkerSmokeChecks)
target_link_libraries(BunkerSmokeChecks PRIVATE d3d11 dxgi d3dcompiler)
add_test(NAME BunkerSmokeChecks COMMAND BunkerSmokeChecks)

message(STATUS "BunkerProtocolSuite: Configured targets successfully.")
