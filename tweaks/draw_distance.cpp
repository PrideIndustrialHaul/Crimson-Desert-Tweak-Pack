/*
 * Draw Distance Manager — Crimson Desert Mod Pack
 * Fine-tune render range for terrain, foliage, NPCs, and structures
 */

#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <cmath>

namespace CrimsonTweaks {

struct DrawDistanceProfile {
    float terrainDistance = 5000.0f;
    float foliageDistance = 800.0f;
    float npcDistance = 300.0f;
    float buildingDistance = 3000.0f;
    float particleDistance = 200.0f;
    float shadowDistance = 150.0f;
    float grassDensityMultiplier = 1.0f;
};

static const DrawDistanceProfile PRESET_POTATO    = { 2000, 300, 150, 1500, 80,  60,  0.3f };
static const DrawDistanceProfile PRESET_BALANCED  = { 4000, 600, 250, 2500, 150, 120, 0.7f };
static const DrawDistanceProfile PRESET_QUALITY   = { 6000, 900, 350, 4000, 250, 180, 1.0f };
static const DrawDistanceProfile PRESET_ULTRA     = { 10000, 1500, 500, 6000, 400, 300, 1.5f };

class DrawDistanceManager {
private:
    DrawDistanceProfile currentProfile;
    bool initialized = false;

    struct MemoryPatch {
        uintptr_t address;
        float originalValue;
        float patchedValue;
    };

    std::vector<MemoryPatch> patches;

    bool WriteFloat(uintptr_t address, float value) {
        DWORD oldProtect;
        if (!VirtualProtect(reinterpret_cast<void*>(address), sizeof(float), PAGE_EXECUTE_READWRITE, &oldProtect)) {
            return false;
        }
        *reinterpret_cast<float*>(address) = value;
        VirtualProtect(reinterpret_cast<void*>(address), sizeof(float), oldProtect, &oldProtect);
        return true;
    }

public:
    bool Initialize(const std::string& preset = "balanced") {
        if (preset == "potato")        currentProfile = PRESET_POTATO;
        else if (preset == "balanced") currentProfile = PRESET_BALANCED;
        else if (preset == "quality")  currentProfile = PRESET_QUALITY;
        else if (preset == "ultra")    currentProfile = PRESET_ULTRA;
        else {
            std::cerr << "[Draw Distance] Unknown preset: " << preset << std::endl;
            return false;
        }

        initialized = true;
        std::cout << "[Draw Distance] Initialized with preset: " << preset << std::endl;
        PrintProfile();
        return true;
    }

    void SetTerrainDistance(float dist)  { currentProfile.terrainDistance = dist; }
    void SetFoliageDistance(float dist)  { currentProfile.foliageDistance = dist; }
    void SetNPCDistance(float dist)      { currentProfile.npcDistance = dist; }
    void SetBuildingDistance(float dist)  { currentProfile.buildingDistance = dist; }
    void SetShadowDistance(float dist)   { currentProfile.shadowDistance = dist; }
    void SetGrassDensity(float mult)     { currentProfile.grassDensityMultiplier = std::clamp(mult, 0.0f, 2.0f); }

    void PrintProfile() const {
        std::cout << "  Terrain:   " << currentProfile.terrainDistance << " m" << std::endl;
        std::cout << "  Foliage:   " << currentProfile.foliageDistance << " m" << std::endl;
        std::cout << "  NPCs:      " << currentProfile.npcDistance << " m" << std::endl;
        std::cout << "  Buildings: " << currentProfile.buildingDistance << " m" << std::endl;
        std::cout << "  Shadows:   " << currentProfile.shadowDistance << " m" << std::endl;
        std::cout << "  Grass:     " << currentProfile.grassDensityMultiplier << "x" << std::endl;
    }

    bool SaveConfig(const std::string& filepath) {
        std::ofstream file(filepath);
        if (!file.is_open()) return false;

        file << "terrain_distance=" << currentProfile.terrainDistance << "\n";
        file << "foliage_distance=" << currentProfile.foliageDistance << "\n";
        file << "npc_distance=" << currentProfile.npcDistance << "\n";
        file << "building_distance=" << currentProfile.buildingDistance << "\n";
        file << "particle_distance=" << currentProfile.particleDistance << "\n";
        file << "shadow_distance=" << currentProfile.shadowDistance << "\n";
        file << "grass_density=" << currentProfile.grassDensityMultiplier << "\n";

        return true;
    }

    bool LoadConfig(const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) return false;

        std::string line;
        while (std::getline(file, line)) {
            auto pos = line.find('=');
            if (pos == std::string::npos) continue;

            std::string key = line.substr(0, pos);
            float val = std::stof(line.substr(pos + 1));

            if (key == "terrain_distance")  currentProfile.terrainDistance = val;
            else if (key == "foliage_distance")  currentProfile.foliageDistance = val;
            else if (key == "npc_distance")      currentProfile.npcDistance = val;
            else if (key == "building_distance") currentProfile.buildingDistance = val;
            else if (key == "shadow_distance")   currentProfile.shadowDistance = val;
            else if (key == "grass_density")     currentProfile.grassDensityMultiplier = val;
        }

        std::cout << "[Draw Distance] Config loaded from: " << filepath << std::endl;
        return true;
    }

    void RestoreDefaults() {
        for (auto& patch : patches) {
            WriteFloat(patch.address, patch.originalValue);
        }
        patches.clear();
    }
};

} // namespace CrimsonTweaks
