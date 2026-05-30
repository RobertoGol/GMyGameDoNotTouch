#include "../include/Renderer_Vulkan.hpp"
#include <iostream>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace BunkerProtocol {

    struct VulkanContext {
        GLFWwindow* window;
        VkInstance instance;
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        VkDevice device;
    };

    VulkanContext ctx;

    bool RendererVulkan::InitWindow(int width, int height, const std::string& title) {
        if (!glfwInit()) return false;

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // Отключаем OpenGL
        ctx.window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);

        if (!ctx.window) return false;

        std::cout << "[Graphics] Window Created. Initializing Vulkan Instance..." << std::endl;
        
        // Здесь идет создание VkInstance и выбор видеокарты
        // На слабых ПК мы будем отдавать приоритет интегрированным GPU
        isInitialized = true;
        return true;
    }

    void RendererVulkan::RenderFrame(const LocationData& currentWorld) {
        if (!isInitialized) return;

        // ЛОГИКА ОТРИСОВКИ "ЗАГЛУШЕК" (Whiteboxing)
        // Для каждого объекта из .wld файла:
        // 1. Берем координаты (x, y, z)
        // 2. Берем форму на основе Registry_ID (куб для стен, сундук для лута)
        // 3. Отрисовываем цветной примитив
    }

    bool RendererVulkan::ShouldClose() {
        return glfwWindowShouldClose(ctx.window);
    }

    void RendererVulkan::Cleanup() {
        glfwDestroyWindow(ctx.window);
        glfwTerminate();
    }
}
