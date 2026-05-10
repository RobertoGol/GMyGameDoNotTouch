#include <GLFW/glfw3.h>

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <string>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "../../include/AtomicPersistence.hpp"
#include "../../include/LaunchSession.hpp"
#include "../../include/SessionProfiles.hpp"
#include "../../include/StoryRoute.hpp"
#include "../../include/World.hpp"
#include "../../include/WorldValidation.hpp"
#include "LauncherSupport.hpp"

namespace {

std::string FileNameLabel(const std::filesystem::path& path) {
    return path.empty() ? std::string("No world") : path.filename().generic_string();
}

}  // namespace

int main() {
    if (!glfwInit()) {
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(1120, 720, "BunkerAdminLauncher", nullptr, nullptr);
    if (window == nullptr) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    bunker::SessionProfile sessionProfile;
    const auto profilePath = bunker::DefaultSessionProfilePath();
    if (!bunker::LoadSessionProfile(profilePath, sessionProfile)) {
        sessionProfile = bunker::MakeDefaultSessionProfile();
    }
    bunker::NormalizeSessionProfile(sessionProfile);

    launcher_support::LauncherState launcherState;
    launcher_support::LauncherDataCache launcherData;
    launcher_support::RefreshLauncherData(launcherData, launcherState, sessionProfile);
    std::snprintf(launcherState.login, sizeof(launcherState.login), "%s", sessionProfile.account.username.c_str());
    launcherState.loggedIn = true;
    launcherState.statusText = "Admin launcher ready. Development tools are isolated from the player launcher.";

    std::string validationSummary = "No validation run yet.";
    int validationErrorCount = 0;
    int validationWarningCount = 0;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        launcherState.selectedWorldIndex =
            launcher_support::ClampIndex(launcherState.selectedWorldIndex, static_cast<int>(launcherData.worlds.size()));
        const auto selectedWorld = launcher_support::SelectedWorldPath(launcherData.worlds, launcherState.selectedWorldIndex);

        ImGui::SetNextWindowSize(ImVec2(920.0f, 620.0f), ImGuiCond_FirstUseEver);
        ImGui::Begin("ADMIN / DEVELOPMENT Launcher", nullptr, ImGuiWindowFlags_NoCollapse);
        ImGui::Text("Bunker Protocol Admin Launcher");
        ImGui::TextDisabled("Separate service executable for validation, diagnostics, and controlled runtime launch.");
        ImGui::Separator();

        ImGui::InputText("Operator", launcherState.login, IM_ARRAYSIZE(launcherState.login));
        if (ImGui::Button("Refresh Data", ImVec2(160.0f, 0.0f))) {
            launcher_support::RefreshLauncherData(launcherData, launcherState, sessionProfile);
            launcherState.statusText = "Admin launcher data refreshed.";
        }

        ImGui::Separator();
        if (launcherData.worldLabels.empty()) {
            ImGui::TextDisabled("No .bwld worlds found.");
        } else {
            ImGui::Combo(
                "World",
                &launcherState.selectedWorldIndex,
                launcherData.worldLabels.data(),
                static_cast<int>(launcherData.worldLabels.size()));
        }
        ImGui::Text("Selected world: %s", FileNameLabel(selectedWorld).c_str());
        ImGui::Text("Profile world: %s", sessionProfile.selectedWorld.c_str());
        ImGui::Text("Profile route: %s", bunker::CurrentStoryCheckpointLabel(sessionProfile).c_str());

        if (ImGui::Button("Validate Selected World", ImVec2(220.0f, 0.0f))) {
            bunker::World world;
            if (selectedWorld.empty() || !world.Load(selectedWorld.string())) {
                validationSummary = "Selected world could not be loaded for validation.";
                validationErrorCount = 1;
                validationWarningCount = 0;
            } else {
                const auto issues = bunker::ValidateWorldForRuntime(world);
                validationSummary = bunker::BuildValidationSummary(issues);
                validationErrorCount = bunker::CountValidationErrors(issues);
                validationWarningCount = bunker::CountValidationWarnings(issues);
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Launch Runtime As Admin", ImVec2(240.0f, 0.0f))) {
            if (selectedWorld.empty()) {
                launcherState.statusText = "Select a world before launching runtime.";
            } else {
                sessionProfile.account.username = launcherState.login;
                sessionProfile.selectedWorld = bunker::NormalizeWorldReference(selectedWorld.string());
                bunker::SaveProfileAtomically(sessionProfile, profilePath);

                bunker::LaunchTicketInfo launchTicket;
                launchTicket.accountId = sessionProfile.account.username;
                launchTicket.sessionMode = "Admin Development";
                launchTicket.characterName = sessionProfile.character.displayName;
                launchTicket.selectedWorld = sessionProfile.selectedWorld;
                launchTicket.bt72SeatRole = "pilot";
                launchTicket.bt72SecondSeatPolicy = sessionProfile.partnerTank.secondSeatPolicy;
                launchTicket.bt72TrustedGunnerHandle = sessionProfile.partnerTank.trustedGunnerHandle;
                launchTicket.launcherRole = "admin";
                if (!bunker::IssueLaunchTicket(launchTicket)) {
                    launcherState.statusText = "Failed to create admin launch ticket.";
                } else {
                    launcher_support::TryLaunchSiblingExecutable("BunkerGame.exe", launcherState.statusText);
                }
            }
        }

        ImGui::Separator();
        ImGui::Text("Validation diagnostics");
        ImGui::BulletText("Errors: %d", validationErrorCount);
        ImGui::BulletText("Warnings: %d", validationWarningCount);
        ImGui::TextWrapped("%s", validationSummary.c_str());
        ImGui::Separator();
        ImGui::TextWrapped("%s", launcherState.statusText.c_str());
        ImGui::TextDisabled("No destructive admin actions are exposed in this first split.");
        ImGui::End();

        ImGui::Render();
        int width = 0;
        int height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        glClearColor(0.06f, 0.07f, 0.09f, 1.0f);
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
