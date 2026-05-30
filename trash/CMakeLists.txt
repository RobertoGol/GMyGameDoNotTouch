cmake_minimum_required(VERSION 3.16)
project(BunkerProtocol)

set(CMAKE_CXX_STANDARD 20)

# Поиск зависимостей
find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

# Пути к заголовкам
include_directories(include external)

# Собираем все исходники (src, world, WorldEditor)
file(GLOB_RECURSE SOURCES 
    "src/*.cpp" 
    "world/*.cpp"
    "WorldEditor/src/*.cpp"
)

add_executable(BunkerProtocol main.cpp ${SOURCES})

# Линковка библиотек
target_link_libraries(BunkerProtocol Vulkan::Vulkan glfw)

message(STATUS "Bunker Protocol: Ready to build for OS: ${CMAKE_SYSTEM_NAME}")
