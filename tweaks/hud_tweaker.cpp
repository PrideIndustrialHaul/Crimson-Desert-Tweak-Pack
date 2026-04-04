/*
 * HUD Tweaker — Crimson Desert Mod Pack
 * Show / hide / rescale any UI element
 */

#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <algorithm>

namespace CrimsonTweaks {

struct HUDElement {
    std::string name;
    bool visible = true;
    float posX = 0.0f;
    float posY = 0.0f;
    float scaleX = 1.0f;
    float scaleY = 1.0f;
    float opacity = 1.0f;
};

class HUDLayout {
private:
    std::unordered_map<std::string, HUDElement> elements;

public:
    void RegisterElement(const std::string& name, float x, float y) {
        elements[name] = {name, true, x, y, 1.0f, 1.0f, 1.0f};
    }

    void SetVisible(const std::string& name, bool visible) {
        if (elements.count(name)) elements[name].visible = visible;
    }

    void SetPosition(const std::string& name, float x, float y) {
        if (elements.count(name)) {
            elements[name].posX = x;
            elements[name].posY = y;
        }
    }

    void SetScale(const std::string& name, float sx, float sy) {
        if (elements.count(name)) {
            elements[name].scaleX = std::clamp(sx, 0.1f, 5.0f);
            elements[name].scaleY = std::clamp(sy, 0.1f, 5.0f);
        }
    }

    void SetOpacity(const std::string& name, float alpha) {
        if (elements.count(name)) {
            elements[name].opacity = std::clamp(alpha, 0.0f, 1.0f);
        }
    }

    void HideAll() {
        for (auto& [name, el] : elements) el.visible = false;
    }

    void ShowAll() {
        for (auto& [name, el] : elements) el.visible = true;
    }

    void ResetAll() {
        for (auto& [name, el] : elements) {
            el.scaleX = 1.0f;
            el.scaleY = 1.0f;
            el.opacity = 1.0f;
            el.visible = true;
        }
    }

    const HUDElement* GetElement(const std::string& name) const {
        auto it = elements.find(name);
        return it != elements.end() ? &it->second : nullptr;
    }

    std::vector<std::string> ListElements() const {
        std::vector<std::string> names;
        for (const auto& [name, _] : elements) names.push_back(name);
        std::sort(names.begin(), names.end());
        return names;
    }
};

class HUDTweaker {
private:
    HUDLayout layout;
    bool active = false;

    void RegisterDefaultElements() {
        layout.RegisterElement("health_bar", 0.05f, 0.90f);
        layout.RegisterElement("stamina_bar", 0.05f, 0.87f);
        layout.RegisterElement("mana_bar", 0.05f, 0.84f);
        layout.RegisterElement("minimap", 0.85f, 0.05f);
        layout.RegisterElement("quest_tracker", 0.80f, 0.20f);
        layout.RegisterElement("hotbar", 0.35f, 0.92f);
        layout.RegisterElement("damage_numbers", 0.50f, 0.50f);
        layout.RegisterElement("boss_health", 0.30f, 0.05f);
        layout.RegisterElement("compass", 0.35f, 0.02f);
        layout.RegisterElement("fps_counter", 0.01f, 0.01f);
        layout.RegisterElement("crosshair", 0.50f, 0.50f);
        layout.RegisterElement("chat_box", 0.01f, 0.60f);
        layout.RegisterElement("notification_area", 0.50f, 0.15f);
        layout.RegisterElement("buff_icons", 0.75f, 0.90f);
        layout.RegisterElement("enemy_markers", 0.50f, 0.50f);
    }

public:
    void Initialize() {
        RegisterDefaultElements();
        active = true;
        std::cout << "[HUD Tweaker] Initialized with " << layout.ListElements().size() << " elements" << std::endl;
    }

    bool SaveLayout(const std::string& filepath) {
        std::ofstream file(filepath);
        if (!file.is_open()) return false;

        for (const auto& name : layout.ListElements()) {
            const auto* el = layout.GetElement(name);
            if (!el) continue;
            file << name
                 << "," << el->visible
                 << "," << el->posX << "," << el->posY
                 << "," << el->scaleX << "," << el->scaleY
                 << "," << el->opacity << "\n";
        }

        std::cout << "[HUD Tweaker] Layout saved to: " << filepath << std::endl;
        return true;
    }

    HUDLayout& GetLayout() { return layout; }

    void Shutdown() {
        active = false;
        std::cout << "[HUD Tweaker] Shutdown" << std::endl;
    }
};

} // namespace CrimsonTweaks
