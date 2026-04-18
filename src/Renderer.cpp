#include "../include/Renderer.hpp"

#include <GLFW/glfw3.h>

#include <cmath>

namespace bunker {

namespace {

void SetColorForCategory(ObjectCategory category) {
    switch (category) {
        case ObjectCategory::Structure:
            glColor3f(0.35f, 0.38f, 0.45f);
            break;
        case ObjectCategory::ResourceNode:
            glColor3f(0.72f, 0.56f, 0.22f);
            break;
        case ObjectCategory::Terminal:
            glColor3f(0.10f, 0.70f, 0.90f);
            break;
        case ObjectCategory::Vehicle:
            glColor3f(0.62f, 0.62f, 0.22f);
            break;
        case ObjectCategory::Landmark:
            glColor3f(0.28f, 0.75f, 0.42f);
            break;
        case ObjectCategory::Container:
            glColor3f(0.68f, 0.28f, 0.20f);
            break;
        case ObjectCategory::Hangar:
            glColor3f(0.55f, 0.48f, 0.30f);
            break;
        case ObjectCategory::Hostile:
            glColor3f(0.78f, 0.18f, 0.18f);
            break;
    }
}

void DrawRect(float x, float y, float halfWidth, float halfHeight) {
    glBegin(GL_QUADS);
    glVertex2f(x - halfWidth, y - halfHeight);
    glVertex2f(x + halfWidth, y - halfHeight);
    glVertex2f(x + halfWidth, y + halfHeight);
    glVertex2f(x - halfWidth, y + halfHeight);
    glEnd();
}

}  // namespace

const char* ToString(ViewMode mode) {
    switch (mode) {
        case ViewMode::FirstPerson:
            return "First Person";
        case ViewMode::ThirdPerson:
            return "Third Person";
        case ViewMode::Cockpit:
            return "Cockpit";
    }
    return "Unknown";
}

const char* ToString(WeatherAnomaly weather) {
    switch (weather) {
        case WeatherAnomaly::Clear:
            return "Clear";
        case WeatherAnomaly::AcidRain:
            return "Acid Rain";
        case WeatherAnomaly::EtherFog:
            return "Ether Fog";
    }
    return "Unknown";
}

void RenderWorld(const World& world, const PlayerState& player, WeatherAnomaly weather, float weatherIntensity, int width, int height) {
    glViewport(0, 0, width, height);
    glClearColor(0.08f, 0.09f, 0.12f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    float visibleWidth = 34.0f;
    float visibleHeight = visibleWidth * (static_cast<float>(height) / static_cast<float>(width));
    float focusAhead = 0.0f;

    switch (player.viewMode) {
        case ViewMode::FirstPerson:
            visibleWidth = 18.0f;
            focusAhead = 3.5f;
            break;
        case ViewMode::ThirdPerson:
            visibleWidth = 34.0f;
            focusAhead = 0.0f;
            break;
        case ViewMode::Cockpit:
            visibleWidth = 14.0f;
            focusAhead = 5.0f;
            break;
    }

    visibleHeight = visibleWidth * (static_cast<float>(height) / static_cast<float>(width));

    const float recoilCameraOffset = player.insideTank ? player.recoilOffset : 0.0f;
    const float cameraX = player.x + std::cos(player.facingRadians) * (focusAhead - recoilCameraOffset);
    const float cameraY = player.y + std::sin(player.facingRadians) * (focusAhead - recoilCameraOffset);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(cameraX - visibleWidth, cameraX + visibleWidth, cameraY - visibleHeight, cameraY + visibleHeight, -1.0f, 1.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glColor3f(0.16f, 0.18f, 0.21f);
    glBegin(GL_LINES);
    for (int line = -40; line <= 40; ++line) {
        glVertex2f(static_cast<float>(line) * 2.0f, -80.0f);
        glVertex2f(static_cast<float>(line) * 2.0f, 80.0f);
        glVertex2f(-80.0f, static_cast<float>(line) * 2.0f);
        glVertex2f(80.0f, static_cast<float>(line) * 2.0f);
    }
    glEnd();

    for (const auto& object : world.objects) {
        SetColorForCategory(object.category);
        DrawRect(object.x, object.y, object.width * 0.5f, object.depth * 0.5f);
    }

    const float forwardX = std::cos(player.facingRadians);
    const float forwardY = std::sin(player.facingRadians);
    const float rightX = std::cos(player.facingRadians + 1.5707963f);
    const float rightY = std::sin(player.facingRadians + 1.5707963f);

    if (player.bucketRaised) {
        glColor3f(0.85f, 0.74f, 0.22f);
        glBegin(GL_QUADS);
        glVertex2f(player.x + forwardX * 1.3f - rightX * 1.4f, player.y + forwardY * 1.3f - rightY * 1.4f);
        glVertex2f(player.x + forwardX * 1.3f + rightX * 1.4f, player.y + forwardY * 1.3f + rightY * 1.4f);
        glVertex2f(player.x + forwardX * 2.4f + rightX * 1.6f, player.y + forwardY * 2.4f + rightY * 1.6f);
        glVertex2f(player.x + forwardX * 2.4f - rightX * 1.6f, player.y + forwardY * 2.4f - rightY * 1.6f);
        glEnd();
    }

    glColor3f(player.insideTank ? 0.88f : 0.42f, player.insideTank ? 0.83f : 0.84f, player.insideTank ? 0.32f : 0.52f);
    glBegin(GL_TRIANGLES);
    glVertex2f(player.x + forwardX * 1.2f, player.y + forwardY * 1.2f);
    glVertex2f(player.x - forwardX * 0.8f + rightX * 0.6f, player.y - forwardY * 0.8f + rightY * 0.6f);
    glVertex2f(player.x - forwardX * 0.8f - rightX * 0.6f, player.y - forwardY * 0.8f - rightY * 0.6f);
    glEnd();

    if (player.viewMode == ViewMode::Cockpit) {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0.0f, static_cast<double>(width), static_cast<double>(height), 0.0, -1.0, 1.0);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glColor4f(0.05f, 0.12f, 0.10f, 0.55f);
        glBegin(GL_QUADS);
        glVertex2f(0.0f, 0.0f);
        glVertex2f(static_cast<float>(width), 0.0f);
        glVertex2f(static_cast<float>(width), 90.0f);
        glVertex2f(0.0f, 90.0f);
        glEnd();
    }
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    float fogR = 0.0f;
    float fogG = 0.0f;
    float fogB = 0.0f;
    float fogAlpha = 0.0f;
    float timePulse = static_cast<float>(std::sin(glfwGetTime() * 0.2f) * 0.5f + 0.5f);

    if (weather == WeatherAnomaly::EtherFog) {
        fogR = 0.48f;
        fogG = 0.22f;
        fogB = 0.74f;
        fogAlpha = (player.insideTank ? 0.12f : 0.32f) * std::max(0.25f, weatherIntensity) * (0.6f + timePulse * 0.5f);
    } else if (weather == WeatherAnomaly::AcidRain) {
        fogR = 0.36f;
        fogG = 0.78f;
        fogB = 0.22f;
        fogAlpha = (player.insideTank ? 0.08f : 0.18f) * std::max(0.25f, weatherIntensity) * (0.65f + timePulse * 0.45f);
    }

    // Рисуем оверлей тумана поверх всего экрана
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, width, height, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    if (fogAlpha > 0.001f) {
        glColor4f(fogR, fogG, fogB, fogAlpha);
        glBegin(GL_QUADS);
            glVertex2f(0, 0);
            glVertex2f((float)width, 0);
            glVertex2f((float)width, (float)height);
            glVertex2f(0, (float)height);
        glEnd();
    }

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glDisable(GL_BLEND);

}

}  // namespace bunker
