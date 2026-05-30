#include <iostream>
#include <string>

#include <GLFW/glfw3.h>

// ImGui
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

std::string loginInput;
std::string passwordInput;
bool authFailed = false;

void TryLogin() {
    if (loginInput == "test" && passwordInput == "123") {
        std::cout << "[Launcher] Login success\n";
        system("BunkerGame.exe"); // запуск игры
    } else {
        authFailed = true;
    }
}

int main() {
    glfwInit();
    GLFWwindow* window = glfwCreateWindow(800, 500, "Bunker Protocol Launcher", NULL, NULL);
    glfwMakeContextCurrent(window);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    char loginBuf[64] = "";
    char passBuf[64] = "";

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // --- UI ---
        ImGui::SetNextWindowPos(ImVec2(250, 150), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(300, 200));

        ImGui::Begin("Login", nullptr,
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoCollapse);

        ImGui::InputText("Login", loginBuf, 64);
        ImGui::InputText("Password", passBuf, 64, ImGuiInputTextFlags_Password);

        if (ImGui::Button("Login")) {
            loginInput = loginBuf;
            passwordInput = passBuf;
            TryLogin();
        }

        if (authFailed) {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "Invalid login or password");
        }

        ImGui::End();

        // --- render ---
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}