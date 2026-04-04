/*
 * Visual Enhancer — Crimson Desert Mod Pack
 * Sharpening, color grading, contrast, film grain removal
 */

#include <windows.h>
#include <iostream>
#include <string>
#include <cmath>
#include <algorithm>

namespace CrimsonTweaks {

struct ColorGrading {
    float brightness = 1.0f;
    float contrast = 1.05f;
    float saturation = 1.1f;
    float gamma = 1.0f;
    float redBalance = 1.0f;
    float greenBalance = 1.0f;
    float blueBalance = 1.0f;
};

struct PostProcessConfig {
    float sharpenStrength = 0.5f;
    float sharpenRadius = 1.0f;
    bool enableCAS = true;            // Contrast Adaptive Sharpening
    bool removeFilmGrain = true;
    bool removeVignette = false;
    bool removeChromaticAberration = true;
    bool removeMotionBlur = false;
    float ambientOcclusionScale = 1.0f;
    bool enableBloom = true;
    float bloomIntensity = 0.8f;
    ColorGrading colorGrading;
};

struct Pixel {
    float r, g, b, a;

    Pixel operator*(float s) const { return {r*s, g*s, b*s, a}; }
    Pixel operator+(const Pixel& o) const { return {r+o.r, g+o.g, b+o.b, a}; }
    Pixel operator-(const Pixel& o) const { return {r-o.r, g-o.g, b-o.b, a}; }
};

class SharpeningFilter {
private:
    float strength;
    float radius;

public:
    SharpeningFilter(float str = 0.5f, float rad = 1.0f)
        : strength(str), radius(rad) {}

    Pixel Apply(const Pixel& center, const Pixel& avgNeighbors) const {
        Pixel diff = center - avgNeighbors;
        return {
            std::clamp(center.r + diff.r * strength, 0.0f, 1.0f),
            std::clamp(center.g + diff.g * strength, 0.0f, 1.0f),
            std::clamp(center.b + diff.b * strength, 0.0f, 1.0f),
            center.a
        };
    }

    void SetStrength(float s) { strength = std::clamp(s, 0.0f, 2.0f); }
    void SetRadius(float r) { radius = std::clamp(r, 0.5f, 3.0f); }
};

class ColorCorrector {
private:
    ColorGrading grading;

    float ApplyGamma(float val, float gamma) const {
        return std::pow(val, 1.0f / gamma);
    }

public:
    void SetGrading(const ColorGrading& g) { grading = g; }

    Pixel Process(const Pixel& input) const {
        float r = input.r * grading.redBalance;
        float g = input.g * grading.greenBalance;
        float b = input.b * grading.blueBalance;

        r = (r - 0.5f) * grading.contrast + 0.5f;
        g = (g - 0.5f) * grading.contrast + 0.5f;
        b = (b - 0.5f) * grading.contrast + 0.5f;

        r *= grading.brightness;
        g *= grading.brightness;
        b *= grading.brightness;

        float luminance = 0.2126f * r + 0.7152f * g + 0.0722f * b;
        r = luminance + (r - luminance) * grading.saturation;
        g = luminance + (g - luminance) * grading.saturation;
        b = luminance + (b - luminance) * grading.saturation;

        r = ApplyGamma(std::clamp(r, 0.0f, 1.0f), grading.gamma);
        g = ApplyGamma(std::clamp(g, 0.0f, 1.0f), grading.gamma);
        b = ApplyGamma(std::clamp(b, 0.0f, 1.0f), grading.gamma);

        return {r, g, b, input.a};
    }
};

class VisualEnhancer {
private:
    PostProcessConfig config;
    SharpeningFilter sharpener;
    ColorCorrector colorCorrector;
    bool active = false;

public:
    void Initialize(const PostProcessConfig& cfg = {}) {
        config = cfg;
        sharpener.SetStrength(config.sharpenStrength);
        sharpener.SetRadius(config.sharpenRadius);
        colorCorrector.SetGrading(config.colorGrading);

        active = true;
        std::cout << "[Visual Enhancer] Initialized" << std::endl;
        std::cout << "  Sharpening:  " << config.sharpenStrength << std::endl;
        std::cout << "  Film grain:  " << (config.removeFilmGrain ? "REMOVED" : "kept") << std::endl;
        std::cout << "  Chrom. aber: " << (config.removeChromaticAberration ? "REMOVED" : "kept") << std::endl;
        std::cout << "  Bloom:       " << (config.enableBloom ? "ON" : "OFF") << std::endl;
    }

    void SetSharpenStrength(float s) { sharpener.SetStrength(s); }
    void ToggleFilmGrain(bool remove) { config.removeFilmGrain = remove; }
    void ToggleMotionBlur(bool remove) { config.removeMotionBlur = remove; }
    void SetBloomIntensity(float i) { config.bloomIntensity = std::clamp(i, 0.0f, 2.0f); }

    void Shutdown() {
        active = false;
        std::cout << "[Visual Enhancer] Shutdown" << std::endl;
    }
};

} // namespace CrimsonTweaks
