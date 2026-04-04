/*
 * TweakFPS Booster — Crimson Desert Mod Pack
 * Core frame rate optimization module
 * Handles shader pre-compilation, frame pacing, and memory pool tuning
 */

#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <unordered_map>

namespace CrimsonTweaks {

struct FrameConfig {
    int targetFPS = 60;
    bool enableFramePacing = true;
    bool precompileShaders = true;
    float memoryPoolSizeMB = 512.0f;
    bool dynamicResolution = false;
    float minResScale = 0.75f;
    float maxResScale = 1.0f;
};

class ShaderCache {
private:
    std::unordered_map<std::string, std::vector<uint8_t>> compiledShaders;
    std::string cachePath;

public:
    ShaderCache(const std::string& path) : cachePath(path) {}

    bool LoadCache() {
        std::ifstream file(cachePath, std::ios::binary);
        if (!file.is_open()) return false;

        uint32_t entryCount = 0;
        file.read(reinterpret_cast<char*>(&entryCount), sizeof(entryCount));

        for (uint32_t i = 0; i < entryCount; ++i) {
            uint32_t nameLen = 0;
            file.read(reinterpret_cast<char*>(&nameLen), sizeof(nameLen));

            std::string name(nameLen, '\0');
            file.read(&name[0], nameLen);

            uint32_t dataLen = 0;
            file.read(reinterpret_cast<char*>(&dataLen), sizeof(dataLen));

            std::vector<uint8_t> data(dataLen);
            file.read(reinterpret_cast<char*>(data.data()), dataLen);

            compiledShaders[name] = std::move(data);
        }

        std::cout << "[FPS Booster] Loaded " << compiledShaders.size() << " cached shaders" << std::endl;
        return true;
    }

    bool HasShader(const std::string& name) const {
        return compiledShaders.find(name) != compiledShaders.end();
    }

    void SaveCache() {
        std::ofstream file(cachePath, std::ios::binary);
        uint32_t count = static_cast<uint32_t>(compiledShaders.size());
        file.write(reinterpret_cast<const char*>(&count), sizeof(count));

        for (const auto& [name, data] : compiledShaders) {
            uint32_t nameLen = static_cast<uint32_t>(name.size());
            file.write(reinterpret_cast<const char*>(&nameLen), sizeof(nameLen));
            file.write(name.c_str(), nameLen);

            uint32_t dataLen = static_cast<uint32_t>(data.size());
            file.write(reinterpret_cast<const char*>(&dataLen), sizeof(dataLen));
            file.write(reinterpret_cast<const char*>(data.data()), dataLen);
        }
    }
};

class FramePacer {
private:
    std::chrono::high_resolution_clock::time_point lastFrameTime;
    double targetFrameTimeMs;
    double accumulator = 0.0;

public:
    FramePacer(int targetFPS) {
        targetFrameTimeMs = 1000.0 / static_cast<double>(targetFPS);
        lastFrameTime = std::chrono::high_resolution_clock::now();
    }

    void WaitForNextFrame() {
        auto now = std::chrono::high_resolution_clock::now();
        double elapsed = std::chrono::duration<double, std::milli>(now - lastFrameTime).count();
        double waitTime = targetFrameTimeMs - elapsed;

        if (waitTime > 1.0) {
            std::this_thread::sleep_for(std::chrono::microseconds(static_cast<int>((waitTime - 1.0) * 1000)));
        }

        while (std::chrono::duration<double, std::milli>(
                   std::chrono::high_resolution_clock::now() - lastFrameTime).count() < targetFrameTimeMs) {
            _mm_pause();
        }

        lastFrameTime = std::chrono::high_resolution_clock::now();
    }

    void SetTargetFPS(int fps) {
        targetFrameTimeMs = 1000.0 / static_cast<double>(fps);
    }
};

class MemoryPoolManager {
private:
    void* poolBase = nullptr;
    size_t poolSize = 0;
    size_t allocated = 0;

public:
    bool Initialize(float sizeMB) {
        poolSize = static_cast<size_t>(sizeMB * 1024 * 1024);
        poolBase = VirtualAlloc(nullptr, poolSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

        if (!poolBase) {
            std::cerr << "[FPS Booster] Failed to allocate memory pool: " << sizeMB << " MB" << std::endl;
            return false;
        }

        std::cout << "[FPS Booster] Memory pool allocated: " << sizeMB << " MB" << std::endl;
        return true;
    }

    void* Allocate(size_t size) {
        size_t aligned = (size + 15) & ~15;
        if (allocated + aligned > poolSize) return nullptr;

        void* ptr = static_cast<uint8_t*>(poolBase) + allocated;
        allocated += aligned;
        return ptr;
    }

    void Reset() {
        allocated = 0;
    }

    ~MemoryPoolManager() {
        if (poolBase) {
            VirtualFree(poolBase, 0, MEM_RELEASE);
        }
    }
};

class FPSBooster {
private:
    FrameConfig config;
    ShaderCache shaderCache;
    FramePacer framePacer;
    MemoryPoolManager memPool;
    bool initialized = false;

public:
    FPSBooster()
        : shaderCache("crimson_shader_cache.bin"),
          framePacer(60) {}

    bool Initialize(const FrameConfig& cfg) {
        config = cfg;

        if (config.precompileShaders) {
            shaderCache.LoadCache();
        }

        framePacer.SetTargetFPS(config.targetFPS);

        if (!memPool.Initialize(config.memoryPoolSizeMB)) {
            return false;
        }

        initialized = true;
        std::cout << "[FPS Booster] Initialized — target " << config.targetFPS << " FPS" << std::endl;
        return true;
    }

    void OnFrameBegin() {
        if (!initialized) return;
        memPool.Reset();
    }

    void OnFrameEnd() {
        if (!initialized) return;
        if (config.enableFramePacing) {
            framePacer.WaitForNextFrame();
        }
    }

    void Shutdown() {
        if (config.precompileShaders) {
            shaderCache.SaveCache();
        }
        initialized = false;
        std::cout << "[FPS Booster] Shutdown complete" << std::endl;
    }
};

} // namespace CrimsonTweaks
