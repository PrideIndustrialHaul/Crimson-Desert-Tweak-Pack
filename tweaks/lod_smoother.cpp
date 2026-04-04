/*
 * LOD Smoother — Crimson Desert Mod Pack
 * Fixes level-of-detail pop-in during exploration and fast travel
 */

#include <windows.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

namespace CrimsonTweaks {

struct LODConfig {
    float transitionSpeed = 2.0f;
    float biasMultiplier = 1.5f;
    float hysteresisRange = 0.15f;
    int maxLODLevel = 4;
    bool smoothTransitions = true;
    bool ditheredFade = true;
};

struct LODObject {
    uintptr_t objectPtr;
    int currentLOD;
    int targetLOD;
    float blendFactor;
    float distanceToCamera;
    float lodDistances[5];
};

class LODSmoother {
private:
    LODConfig config;
    std::vector<LODObject> trackedObjects;
    bool active = false;

    int CalculateTargetLOD(const LODObject& obj) {
        for (int i = 0; i < config.maxLODLevel; ++i) {
            float threshold = obj.lodDistances[i] * config.biasMultiplier;

            if (obj.currentLOD <= i) {
                threshold *= (1.0f + config.hysteresisRange);
            } else {
                threshold *= (1.0f - config.hysteresisRange);
            }

            if (obj.distanceToCamera < threshold) {
                return i;
            }
        }
        return config.maxLODLevel;
    }

    float SmoothStep(float edge0, float edge1, float x) {
        float t = std::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
        return t * t * (3.0f - 2.0f * t);
    }

public:
    void Initialize(const LODConfig& cfg = {}) {
        config = cfg;
        active = true;
        std::cout << "[LOD Smoother] Initialized" << std::endl;
        std::cout << "  Transition speed: " << config.transitionSpeed << std::endl;
        std::cout << "  Bias multiplier:  " << config.biasMultiplier << std::endl;
        std::cout << "  Hysteresis:       " << config.hysteresisRange << std::endl;
        std::cout << "  Dithered fade:    " << (config.ditheredFade ? "ON" : "OFF") << std::endl;
    }

    void Update(float deltaTime) {
        if (!active) return;

        for (auto& obj : trackedObjects) {
            int target = CalculateTargetLOD(obj);
            obj.targetLOD = target;

            if (config.smoothTransitions && obj.currentLOD != obj.targetLOD) {
                float direction = (obj.targetLOD > obj.currentLOD) ? 1.0f : -1.0f;
                obj.blendFactor += direction * config.transitionSpeed * deltaTime;

                if (obj.blendFactor >= 1.0f || obj.blendFactor <= 0.0f) {
                    obj.currentLOD = obj.targetLOD;
                    obj.blendFactor = 0.0f;
                }
            } else {
                obj.currentLOD = obj.targetLOD;
                obj.blendFactor = 0.0f;
            }
        }
    }

    void TrackObject(uintptr_t ptr, const float distances[5]) {
        LODObject obj{};
        obj.objectPtr = ptr;
        obj.currentLOD = 0;
        obj.targetLOD = 0;
        obj.blendFactor = 0.0f;
        obj.distanceToCamera = 0.0f;
        std::copy(distances, distances + 5, obj.lodDistances);
        trackedObjects.push_back(obj);
    }

    void SetBias(float bias) {
        config.biasMultiplier = std::clamp(bias, 0.5f, 3.0f);
    }

    void Shutdown() {
        trackedObjects.clear();
        active = false;
        std::cout << "[LOD Smoother] Shutdown" << std::endl;
    }
};

} // namespace CrimsonTweaks
