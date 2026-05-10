#include "../include/Renderer.hpp"

#include <GLFW/glfw3.h>

#include <algorithm>
#include <cmath>
#include <string>

namespace bunker {

namespace {

struct Vec3 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

Vec3 operator-(const Vec3& lhs, const Vec3& rhs) {
    return {lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z};
}

float Dot(const Vec3& lhs, const Vec3& rhs) {
    return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}

Vec3 Cross(const Vec3& lhs, const Vec3& rhs) {
    return {
        lhs.y * rhs.z - lhs.z * rhs.y,
        lhs.z * rhs.x - lhs.x * rhs.z,
        lhs.x * rhs.y - lhs.y * rhs.x,
    };
}

Vec3 Normalize(Vec3 value) {
    const float length = std::sqrt(Dot(value, value));
    if (length <= 0.0001f) {
        return {};
    }
    return {value.x / length, value.y / length, value.z / length};
}

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

void DrawGroundRect(float x, float z, float halfWidth, float halfDepth) {
    glBegin(GL_QUADS);
    glVertex3f(x - halfWidth, 0.0f, z - halfDepth);
    glVertex3f(x + halfWidth, 0.0f, z - halfDepth);
    glVertex3f(x + halfWidth, 0.0f, z + halfDepth);
    glVertex3f(x - halfWidth, 0.0f, z + halfDepth);
    glEnd();
}

void DrawBox(float x, float z, float width, float depth, float height) {
    const float minX = x - width * 0.5f;
    const float maxX = x + width * 0.5f;
    const float minZ = z - depth * 0.5f;
    const float maxZ = z + depth * 0.5f;
    const float maxY = std::max(0.08f, height);

    glBegin(GL_QUADS);
    glVertex3f(minX, 0.0f, minZ);
    glVertex3f(maxX, 0.0f, minZ);
    glVertex3f(maxX, 0.0f, maxZ);
    glVertex3f(minX, 0.0f, maxZ);

    glVertex3f(minX, maxY, minZ);
    glVertex3f(minX, maxY, maxZ);
    glVertex3f(maxX, maxY, maxZ);
    glVertex3f(maxX, maxY, minZ);

    glVertex3f(minX, 0.0f, minZ);
    glVertex3f(minX, maxY, minZ);
    glVertex3f(maxX, maxY, minZ);
    glVertex3f(maxX, 0.0f, minZ);

    glVertex3f(maxX, 0.0f, minZ);
    glVertex3f(maxX, maxY, minZ);
    glVertex3f(maxX, maxY, maxZ);
    glVertex3f(maxX, 0.0f, maxZ);

    glVertex3f(maxX, 0.0f, maxZ);
    glVertex3f(maxX, maxY, maxZ);
    glVertex3f(minX, maxY, maxZ);
    glVertex3f(minX, 0.0f, maxZ);

    glVertex3f(minX, 0.0f, maxZ);
    glVertex3f(minX, maxY, maxZ);
    glVertex3f(minX, maxY, minZ);
    glVertex3f(minX, 0.0f, minZ);
    glEnd();
}

void DrawRing(float x, float z, float radius, float thickness, int segments) {
    if (radius <= 0.0f || thickness <= 0.0f) {
        return;
    }

    const float innerRadius = std::max(0.05f, radius - thickness);
    glBegin(GL_TRIANGLE_STRIP);
    for (int index = 0; index <= segments; ++index) {
        const float angle = (static_cast<float>(index) / static_cast<float>(segments)) * 6.2831853f;
        const float cosAngle = std::cos(angle);
        const float sinAngle = std::sin(angle);
        glVertex3f(x + cosAngle * radius, 0.04f, z + sinAngle * radius);
        glVertex3f(x + cosAngle * innerRadius, 0.04f, z + sinAngle * innerRadius);
    }
    glEnd();
}

void ApplyPerspective(float fovDegrees, float aspect, float nearPlane, float farPlane) {
    const double fovRadians = static_cast<double>(fovDegrees) * 3.14159265358979323846 / 180.0;
    const double top = std::tan(fovRadians * 0.5) * static_cast<double>(nearPlane);
    const double right = top * static_cast<double>(aspect);
    glFrustum(-right, right, -top, top, nearPlane, farPlane);
}

void ApplyLookAt(const RuntimeCamera& camera) {
    const Vec3 eye{camera.positionX, camera.positionY, camera.positionZ};
    const Vec3 target{camera.targetX, camera.targetY, camera.targetZ};
    Vec3 forward = Normalize(target - eye);
    if (Dot(forward, forward) <= 0.0001f) {
        forward = {0.0f, 0.0f, -1.0f};
    }
    Vec3 side = Normalize(Cross(forward, {0.0f, 1.0f, 0.0f}));
    if (Dot(side, side) <= 0.0001f) {
        side = {1.0f, 0.0f, 0.0f};
    }
    const Vec3 up = Cross(side, forward);
    const float matrix[16] = {
        side.x, up.x, -forward.x, 0.0f,
        side.y, up.y, -forward.y, 0.0f,
        side.z, up.z, -forward.z, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
    };
    glMultMatrixf(matrix);
    glTranslatef(-eye.x, -eye.y, -eye.z);
}

std::string ToLowerCopy(std::string_view value) {
    std::string lower(value);
    std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return lower;
}

bool LooksLikeGlassObject(const MapObject& object) {
    const std::string label = ToLowerCopy(object.registryId + " " + object.displayName + " " + object.scriptTag);
    return label.find("glass") != std::string::npos || label.find("window") != std::string::npos;
}

bool LooksLikeFoliageObject(const MapObject& object) {
    const std::string label = ToLowerCopy(object.registryId + " " + object.displayName + " " + object.scriptTag);
    return label.find("brush") != std::string::npos ||
        label.find("shrub") != std::string::npos ||
        label.find("foliage") != std::string::npos ||
        label.find("vine") != std::string::npos;
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

RuntimeCamera BuildRuntimeCamera(const PlayerState& player) {
    const float forwardX = std::cos(player.facingRadians);
    const float forwardZ = std::sin(player.facingRadians);
    const float recoilCameraOffset = player.insideTank ? player.recoilOffset : 0.0f;

    RuntimeCamera camera;
    switch (player.viewMode) {
        case ViewMode::FirstPerson:
            camera.positionX = player.x - forwardX * recoilCameraOffset;
            camera.positionY = player.insideTank ? 2.1f : 1.65f;
            camera.positionZ = player.y - forwardZ * recoilCameraOffset;
            camera.targetX = player.x + forwardX * 8.0f;
            camera.targetY = player.insideTank ? 1.9f : 1.55f;
            camera.targetZ = player.y + forwardZ * 8.0f;
            camera.fovDegrees = player.insideTank ? 58.0f : 64.0f;
            break;
        case ViewMode::ThirdPerson:
            camera.positionX = player.x - forwardX * 8.5f;
            camera.positionY = player.insideTank ? 5.6f : 4.2f;
            camera.positionZ = player.y - forwardZ * 8.5f;
            camera.targetX = player.x + forwardX * 2.0f;
            camera.targetY = player.insideTank ? 1.4f : 1.1f;
            camera.targetZ = player.y + forwardZ * 2.0f;
            camera.fovDegrees = 62.0f;
            break;
        case ViewMode::Cockpit:
            camera.positionX = player.x - forwardX * (0.8f + recoilCameraOffset);
            camera.positionY = 2.3f;
            camera.positionZ = player.y - forwardZ * (0.8f + recoilCameraOffset);
            camera.targetX = player.x + forwardX * 12.0f;
            camera.targetY = 1.95f;
            camera.targetZ = player.y + forwardZ * 12.0f;
            camera.fovDegrees = 54.0f;
            break;
    }
    return camera;
}

void RenderWorld(const World& world, const PlayerState& player, WeatherAnomaly weather, float weatherIntensity, int width, int height) {
    glViewport(0, 0, width, height);
    glClearColor(0.08f, 0.09f, 0.12f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    const float aspect = static_cast<float>(std::max(1, width)) / static_cast<float>(std::max(1, height));
    const RuntimeCamera camera = BuildRuntimeCamera(player);
    ApplyPerspective(camera.fovDegrees, aspect, 0.1f, 260.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    ApplyLookAt(camera);

    glColor3f(0.16f, 0.18f, 0.21f);
    glBegin(GL_LINES);
    for (int line = -40; line <= 40; ++line) {
        glVertex3f(static_cast<float>(line) * 2.0f, 0.0f, -80.0f);
        glVertex3f(static_cast<float>(line) * 2.0f, 0.0f, 80.0f);
        glVertex3f(-80.0f, 0.0f, static_cast<float>(line) * 2.0f);
        glVertex3f(80.0f, 0.0f, static_cast<float>(line) * 2.0f);
    }
    glEnd();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    for (const auto& object : world.objects) {
        const bool glassObject = LooksLikeGlassObject(object);
        const bool foliageObject = LooksLikeFoliageObject(object);
        const float swayOffset = foliageObject
            ? std::sin(static_cast<float>(glfwGetTime()) * 2.1f + object.x * 0.45f) * 0.12f
            : 0.0f;
        const float renderX = object.x + swayOffset;

        if (glassObject) {
            glColor4f(0.56f, 0.82f, 0.96f, 0.34f);
            DrawBox(renderX, object.y, object.width, object.depth, object.height);

            glColor4f(0.88f, 0.96f, 1.0f, 0.5f);
            glBegin(GL_LINE_LOOP);
            glVertex3f(renderX - object.width * 0.5f, object.height + 0.02f, object.y - object.depth * 0.5f);
            glVertex3f(renderX + object.width * 0.5f, object.height + 0.02f, object.y - object.depth * 0.5f);
            glVertex3f(renderX + object.width * 0.5f, object.height + 0.02f, object.y + object.depth * 0.5f);
            glVertex3f(renderX - object.width * 0.5f, object.height + 0.02f, object.y + object.depth * 0.5f);
            glEnd();
            continue;
        }

        if (foliageObject) {
            const float rainBias = weather == WeatherAnomaly::AcidRain ? std::min(0.22f, weatherIntensity * 0.14f) : 0.0f;
            glColor4f(0.24f, 0.64f + rainBias, 0.30f, 0.82f);
            DrawBox(renderX, object.y, object.width, object.depth, std::max(0.45f, object.height));
            continue;
        }

        SetColorForCategory(object.category);
        DrawBox(renderX, object.y, object.width, object.depth, object.height);
    }
    glDisable(GL_BLEND);

    const float forwardX = std::cos(player.facingRadians);
    const float forwardY = std::sin(player.facingRadians);
    const float rightX = std::cos(player.facingRadians + 1.5707963f);
    const float rightY = std::sin(player.facingRadians + 1.5707963f);

    if (player.bucketRaised) {
        glColor3f(0.85f, 0.74f, 0.22f);
        glBegin(GL_QUADS);
        glVertex3f(player.x + forwardX * 1.3f - rightX * 1.4f, 0.35f, player.y + forwardY * 1.3f - rightY * 1.4f);
        glVertex3f(player.x + forwardX * 1.3f + rightX * 1.4f, 0.35f, player.y + forwardY * 1.3f + rightY * 1.4f);
        glVertex3f(player.x + forwardX * 2.4f + rightX * 1.6f, 0.55f, player.y + forwardY * 2.4f + rightY * 1.6f);
        glVertex3f(player.x + forwardX * 2.4f - rightX * 1.6f, 0.55f, player.y + forwardY * 2.4f - rightY * 1.6f);
        glEnd();
    }

    glColor3f(player.insideTank ? 0.88f : 0.42f, player.insideTank ? 0.83f : 0.84f, player.insideTank ? 0.32f : 0.52f);
    DrawBox(player.x, player.y, player.insideTank ? 2.4f : 0.8f, player.insideTank ? 3.2f : 0.8f, player.insideTank ? 1.45f : 1.75f);

    if (player.muzzleFlashTimer > 0.0f || player.shockWaveTimer > 0.0f) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    if (player.muzzleFlashTimer > 0.0f) {
        const float flashAlpha = std::min(0.92f, player.muzzleFlashTimer * (player.insideTank ? 2.4f : 2.8f) * std::max(0.25f, player.muzzleFlashStrength));
        const float flashReach = player.insideTank ? (2.6f + player.muzzleFlashStrength * 2.8f) : (1.4f + player.muzzleFlashStrength * 1.4f);
        const float flashWidth = player.insideTank ? (0.8f + player.muzzleFlashStrength * 1.8f) : (0.35f + player.muzzleFlashStrength * 0.9f);
        const float muzzleX = player.x + forwardX * (player.insideTank ? 1.8f : 0.85f);
        const float muzzleY = player.y + forwardY * (player.insideTank ? 1.8f : 0.85f);

        glColor4f(1.0f, 0.78f, 0.24f, flashAlpha);
        glBegin(GL_TRIANGLES);
        glVertex3f(muzzleX + forwardX * flashReach, 1.1f, muzzleY + forwardY * flashReach);
        glVertex3f(muzzleX - rightX * flashWidth, 0.75f, muzzleY - rightY * flashWidth);
        glVertex3f(muzzleX + rightX * flashWidth, 0.75f, muzzleY + rightY * flashWidth);
        glEnd();

        glColor4f(1.0f, 0.94f, 0.72f, flashAlpha * 0.7f);
        glBegin(GL_TRIANGLES);
        glVertex3f(muzzleX + forwardX * (flashReach * 0.7f), 1.15f, muzzleY + forwardY * (flashReach * 0.7f));
        glVertex3f(muzzleX - rightX * (flashWidth * 0.45f), 0.85f, muzzleY - rightY * (flashWidth * 0.45f));
        glVertex3f(muzzleX + rightX * (flashWidth * 0.45f), 0.85f, muzzleY + rightY * (flashWidth * 0.45f));
        glEnd();
    }

    if (player.shockWaveTimer > 0.0f && player.shockWaveDuration > 0.0f) {
        const float progress = 1.0f - std::clamp(player.shockWaveTimer / player.shockWaveDuration, 0.0f, 1.0f);
        const float radius = 0.8f + progress * (player.insideTank ? 6.8f : 4.0f) * std::max(0.35f, player.shockWaveStrength);
        const float thickness = 0.18f + (1.0f - progress) * 0.32f * std::max(0.35f, player.shockWaveStrength);
        const float alpha = (1.0f - progress) * 0.35f * std::max(0.35f, player.shockWaveStrength);
        glColor4f(1.0f, 0.70f, 0.26f, alpha);
        DrawRing(player.x, player.y, radius, thickness, 48);
    }

    if (player.muzzleFlashTimer > 0.0f || player.shockWaveTimer > 0.0f) {
        glDisable(GL_BLEND);
    }

    glDisable(GL_DEPTH_TEST);

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
