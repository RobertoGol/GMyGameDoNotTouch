cmake_minimum_required(VERSION 3.16)
project(BunkerProtocolSuite LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Поиск необходимых системных графических пакетов и потоков
find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)

# Регистрация путей ко всем внутренним заголовочным хедерам
set(COMMON_INCLUDE_DIRS 
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/external/imgui"
    "${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/backends"
    "${CMAKE_CURRENT_SOURCE_DIR}/external/glfw/include"
)

# Подключаем GLFW как встроенный подпроект репозитория
add_subdirectory(external/glfw)

# Функция тонкой настройки и безопасного разделения флагов компилятора
function(configure_bunker_target target_name)
    target_include_directories(${target_name} PRIVATE ${COMMON_INCLUDE_DIRS})
    target_link_libraries(${target_name} PRIVATE glfw OpenGL::GL Threads::Threads)
    
    if(MSVC)
        target_compile_options(${target_name} PRIVATE /W4 /permissive- /Zm300)
        # Динамическое разделение флагов сборки (устраняет ошибку D8016 командной строки cl)
        target_compile_options(${target_name} PRIVATE 
            $<$<CONFIG:Debug>:/Od /RTC1 /MDd>
            $<$<CONFIG:Release>:/O2 /Ob2 /MT>
            $<$<CONFIG:MinSizeRel>:/O1 /Ob1 /MT>
            $<$<CONFIG:RelWithDebInfo>:/O2 /Zi /MT>
        )
    else()
        target_compile_options(${target_name} PRIVATE -Wall -Wextra -Wpedantic)
        target_compile_options(${target_name} PRIVATE
            $<$<CONFIG:Debug>:-O0 -g>
            $<$<CONFIG:Release>:-O3>
        )
    endif()
endfunction()

# ==============================================================================
# ОПРЕДЕЛЕНИЕ ТАРГЕТОВ СБОРКИ СЮИТЫ ПОДСИСТЕМ БУНКЕРА
# ==============================================================================

# 1. Основной исполняемый файл игры
add_executable(BunkerGame
    src/main.cpp
    src/GameExecution.cpp
    src/GameplayDescriptorRegistry.cpp
    src/GameRuntime.cpp
    src/GameRuntimePipPad.cpp
    src/LanlineServices.cpp
    src/LanlineSession.cpp
    src/LaunchSession.cpp
    src/PrefabLibrary.cpp
    src/ProceduralAnimation.cpp
    src/Progression.cpp
    src/SessionProfiles.cpp
    src/SkillSystem.cpp
    src/StoryRoute.cpp
    src/WorldEvents.cpp
    src/World.cpp
    src/WorldExport.cpp
    src/WorldSemanticAuthoring.cpp
    src/WorldValidation.cpp
    src/EscMenuSystem.cpp
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/backends/imgui_impl_glfw.cpp
    external/imgui/backends/imgui_impl_dx11.cpp
    src/Dx11Renderer.cpp # Или просто Dx11Renderer.cpp, если файл лежит в корне проекта
)
configure_bunker_target(BunkerGame)

# 2. Исполняемый файл Лаунчера
add_executable(BunkerLauncher
    Launcher/src/Launcher_Main.cpp
    Launcher/src/LauncherSupport.cpp
    src/GameplayDescriptorRegistry.cpp
    src/LanlineServices.cpp
    src/LanlineSession.cpp
    src/LaunchSession.cpp
    src/SessionProfiles.cpp
    src/StoryRoute.cpp
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/backends/imgui_impl_glfw.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp
)
configure_bunker_target(BunkerLauncher)
