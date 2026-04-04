/*
 * VRAM Optimizer — Crimson Desert Mod Pack
 * Smart texture streaming profiles for 4GB / 6GB / 8GB / 12GB+ GPUs
 */

#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

#pragma comment(lib, "dxgi.lib")

namespace CrimsonTweaks {

enum class VRAMTier {
    LOW_4GB,
    MID_6GB,
    HIGH_8GB,
    ULTRA_12GB
};

struct TextureStreamConfig {
    int maxTextureResolution = 2048;
    int streamingPoolMB = 256;
    float mipBias = 0.0f;
    bool compressTextures = false;
    int maxAnisotropy = 8;
    bool asyncStreaming = true;
    int streamingBudgetPerFrame = 4;
};

class VRAMDetector {
public:
    static size_t GetAvailableVRAM_MB() {
        IDXGIFactory1* factory = nullptr;
        if (FAILED(CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&factory))) {
            return 4096;
        }

        IDXGIAdapter1* adapter = nullptr;
        if (FAILED(factory->EnumAdapters1(0, &adapter))) {
            factory->Release();
            return 4096;
        }

        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);

        size_t vramMB = desc.DedicatedVideoMemory / (1024 * 1024);

        adapter->Release();
        factory->Release();

        return vramMB;
    }

    static VRAMTier ClassifyGPU(size_t vramMB) {
        if (vramMB >= 12000) return VRAMTier::ULTRA_12GB;
        if (vramMB >= 8000)  return VRAMTier::HIGH_8GB;
        if (vramMB >= 6000)  return VRAMTier::MID_6GB;
        return VRAMTier::LOW_4GB;
    }
};

class VRAMOptimizer {
private:
    TextureStreamConfig config;
    VRAMTier gpuTier;
    size_t detectedVRAM;
    bool active = false;

    TextureStreamConfig GetConfigForTier(VRAMTier tier) {
        TextureStreamConfig cfg;
        switch (tier) {
            case VRAMTier::LOW_4GB:
                cfg.maxTextureResolution = 1024;
                cfg.streamingPoolMB = 128;
                cfg.mipBias = 1.0f;
                cfg.compressTextures = true;
                cfg.maxAnisotropy = 4;
                cfg.streamingBudgetPerFrame = 2;
                break;
            case VRAMTier::MID_6GB:
                cfg.maxTextureResolution = 2048;
                cfg.streamingPoolMB = 256;
                cfg.mipBias = 0.5f;
                cfg.compressTextures = true;
                cfg.maxAnisotropy = 8;
                cfg.streamingBudgetPerFrame = 3;
                break;
            case VRAMTier::HIGH_8GB:
                cfg.maxTextureResolution = 4096;
                cfg.streamingPoolMB = 512;
                cfg.mipBias = 0.0f;
                cfg.compressTextures = false;
                cfg.maxAnisotropy = 16;
                cfg.streamingBudgetPerFrame = 4;
                break;
            case VRAMTier::ULTRA_12GB:
                cfg.maxTextureResolution = 4096;
                cfg.streamingPoolMB = 1024;
                cfg.mipBias = -0.5f;
                cfg.compressTextures = false;
                cfg.maxAnisotropy = 16;
                cfg.streamingBudgetPerFrame = 8;
                break;
        }
        return cfg;
    }

public:
    bool Initialize() {
        detectedVRAM = VRAMDetector::GetAvailableVRAM_MB();
        gpuTier = VRAMDetector::ClassifyGPU(detectedVRAM);
        config = GetConfigForTier(gpuTier);

        std::cout << "[VRAM Optimizer] Detected VRAM: " << detectedVRAM << " MB" << std::endl;
        std::cout << "[VRAM Optimizer] GPU tier: ";
        switch (gpuTier) {
            case VRAMTier::LOW_4GB:    std::cout << "4 GB (Low)"; break;
            case VRAMTier::MID_6GB:    std::cout << "6 GB (Mid)"; break;
            case VRAMTier::HIGH_8GB:   std::cout << "8 GB (High)"; break;
            case VRAMTier::ULTRA_12GB: std::cout << "12 GB+ (Ultra)"; break;
        }
        std::cout << std::endl;
        std::cout << "[VRAM Optimizer] Max texture: " << config.maxTextureResolution << "px" << std::endl;
        std::cout << "[VRAM Optimizer] Streaming pool: " << config.streamingPoolMB << " MB" << std::endl;

        active = true;
        return true;
    }

    void OverrideTextureRes(int maxRes) { config.maxTextureResolution = maxRes; }
    void OverrideMipBias(float bias)    { config.mipBias = bias; }
    void OverridePoolSize(int mb)       { config.streamingPoolMB = mb; }

    void Shutdown() {
        active = false;
        std::cout << "[VRAM Optimizer] Shutdown" << std::endl;
    }
};

} // namespace CrimsonTweaks
