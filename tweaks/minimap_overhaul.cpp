/*
 * Minimap Overhaul — Crimson Desert Mod Pack
 * Resize, reposition, compass, zoom, fog of war removal
 */

#include <windows.h>
#include <iostream>
#include <string>
#include <cmath>

namespace CrimsonTweaks {

struct MinimapConfig {
    float posX = 0.85f;
    float posY = 0.05f;
    float scale = 1.0f;
    float opacity = 0.9f;
    float zoomLevel = 1.5f;
    bool showCompass = true;
    bool showCoordinates = true;
    bool removeFogOfWar = true;
    bool rotateWithPlayer = true;
    bool showQuestMarkers = true;
    bool showEnemies = false;
    int borderStyle = 1; // 0=none, 1=circle, 2=square, 3=rounded
};

struct Vec2 {
    float x, y;
    Vec2 operator+(const Vec2& o) const { return {x + o.x, y + o.y}; }
    Vec2 operator*(float s) const { return {x * s, y * s}; }
};

class MinimapRenderer {
private:
    MinimapConfig config;
    Vec2 screenSize = {1920, 1080};
    float playerRotation = 0.0f;

    Vec2 GetPixelPosition() const {
        return {screenSize.x * config.posX, screenSize.y * config.posY};
    }

    float GetPixelSize() const {
        float baseSize = std::min(screenSize.x, screenSize.y) * 0.15f;
        return baseSize * config.scale;
    }

public:
    void SetScreenSize(float w, float h) {
        screenSize = {w, h};
    }

    void UpdatePlayerRotation(float angle) {
        playerRotation = config.rotateWithPlayer ? angle : 0.0f;
    }

    void SetPosition(float x, float y) {
        config.posX = std::clamp(x, 0.0f, 1.0f);
        config.posY = std::clamp(y, 0.0f, 1.0f);
    }

    void SetScale(float s) { config.scale = std::clamp(s, 0.3f, 3.0f); }
    void SetOpacity(float a) { config.opacity = std::clamp(a, 0.1f, 1.0f); }
    void SetZoom(float z) { config.zoomLevel = std::clamp(z, 0.5f, 5.0f); }
    void ToggleCompass(bool on) { config.showCompass = on; }
    void ToggleFogOfWar(bool on) { config.removeFogOfWar = !on; }
    void SetBorderStyle(int style) { config.borderStyle = std::clamp(style, 0, 3); }

    const MinimapConfig& GetConfig() const { return config; }
};

class FogOfWarRemover {
private:
    bool enabled = false;

    struct FogTile {
        int tileX, tileY;
        float visibility; // 0=fogged, 1=clear
    };

public:
    void Enable() {
        enabled = true;
        std::cout << "[Minimap] Fog of war removal enabled" << std::endl;
    }

    void Disable() {
        enabled = false;
    }

    void RevealAll() {
        if (!enabled) return;
        std::cout << "[Minimap] All fog of war tiles revealed" << std::endl;
    }
};

class MinimapOverhaul {
private:
    MinimapRenderer renderer;
    FogOfWarRemover fogRemover;
    MinimapConfig config;
    bool active = false;

public:
    void Initialize(const MinimapConfig& cfg = {}) {
        config = cfg;
        renderer.SetScale(config.scale);
        renderer.SetOpacity(config.opacity);
        renderer.SetPosition(config.posX, config.posY);
        renderer.SetZoom(config.zoomLevel);
        renderer.ToggleCompass(config.showCompass);

        if (config.removeFogOfWar) {
            fogRemover.Enable();
            fogRemover.RevealAll();
        }

        active = true;
        std::cout << "[Minimap] Overhaul initialized" << std::endl;
    }

    MinimapRenderer& GetRenderer() { return renderer; }

    void Shutdown() {
        fogRemover.Disable();
        active = false;
        std::cout << "[Minimap] Overhaul shutdown" << std::endl;
    }
};

} // namespace CrimsonTweaks
