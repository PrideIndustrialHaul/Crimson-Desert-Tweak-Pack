/*
 * Input Lag Fix — Crimson Desert Mod Pack
 * Reduces input-to-screen latency for tighter combat responsiveness
 */

#include <windows.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <atomic>

namespace CrimsonTweaks {

struct InputConfig {
    bool reducePreRenderedFrames = true;
    int maxPreRenderedFrames = 1;
    bool disableVSyncQueue = true;
    bool highPriorityInput = true;
    float pollingRateMultiplier = 2.0f;
    bool rawInputMode = true;
};

class RawInputHandler {
private:
    RAWINPUTDEVICE rid[2];
    bool registered = false;

public:
    bool Register(HWND hwnd) {
        rid[0].usUsagePage = 0x01;
        rid[0].usUsage = 0x02; // Mouse
        rid[0].dwFlags = RIDEV_NOLEGACY;
        rid[0].hwndTarget = hwnd;

        rid[1].usUsagePage = 0x01;
        rid[1].usUsage = 0x06; // Keyboard
        rid[1].dwFlags = RIDEV_NOLEGACY;
        rid[1].hwndTarget = hwnd;

        if (RegisterRawInputDevices(rid, 2, sizeof(RAWINPUTDEVICE))) {
            registered = true;
            std::cout << "[Input Lag Fix] Raw input registered" << std::endl;
            return true;
        }

        std::cerr << "[Input Lag Fix] Failed to register raw input" << std::endl;
        return false;
    }

    void Unregister() {
        if (!registered) return;

        rid[0].dwFlags = RIDEV_REMOVE;
        rid[0].hwndTarget = nullptr;
        rid[1].dwFlags = RIDEV_REMOVE;
        rid[1].hwndTarget = nullptr;

        RegisterRawInputDevices(rid, 2, sizeof(RAWINPUTDEVICE));
        registered = false;
    }
};

class FrameQueueController {
private:
    int originalMaxFrames = 3;

    bool SetNVIDIAPreRender(int frames) {
        HKEY key;
        if (RegOpenKeyExA(HKEY_CURRENT_USER,
                "Software\\NVIDIA Corporation\\Global\\FLControl",
                0, KEY_SET_VALUE, &key) == ERROR_SUCCESS) {
            DWORD val = frames;
            RegSetValueExA(key, "MaxPreRenderedFrames", 0, REG_DWORD,
                          reinterpret_cast<const BYTE*>(&val), sizeof(val));
            RegCloseKey(key);
            return true;
        }
        return false;
    }

public:
    void ReduceFrameQueue(int maxFrames = 1) {
        if (SetNVIDIAPreRender(maxFrames)) {
            std::cout << "[Input Lag Fix] Pre-rendered frames set to " << maxFrames << std::endl;
        }
    }

    void Restore() {
        SetNVIDIAPreRender(originalMaxFrames);
    }
};

class InputTimingOptimizer {
private:
    std::atomic<bool> running{false};
    std::thread pollingThread;
    float multiplier;

    void HighFrequencyPoll() {
        DWORD sleepMs = static_cast<DWORD>(1000.0f / (1000.0f * multiplier));
        if (sleepMs < 1) sleepMs = 1;

        while (running.load()) {
            MSG msg;
            while (PeekMessage(&msg, nullptr, WM_INPUT, WM_INPUT, PM_REMOVE)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            Sleep(sleepMs);
        }
    }

public:
    InputTimingOptimizer(float mult = 2.0f) : multiplier(mult) {}

    void Start() {
        running.store(true);
        pollingThread = std::thread(&InputTimingOptimizer::HighFrequencyPoll, this);
        SetThreadPriority(pollingThread.native_handle(), THREAD_PRIORITY_HIGHEST);
        std::cout << "[Input Lag Fix] High-frequency polling started (x" << multiplier << ")" << std::endl;
    }

    void Stop() {
        running.store(false);
        if (pollingThread.joinable()) pollingThread.join();
    }

    ~InputTimingOptimizer() { Stop(); }
};

class InputLagFix {
private:
    InputConfig config;
    RawInputHandler rawInput;
    FrameQueueController frameQueue;
    InputTimingOptimizer timingOptimizer;
    bool active = false;

public:
    InputLagFix() : timingOptimizer(2.0f) {}

    void Initialize(const InputConfig& cfg = {}) {
        config = cfg;

        if (config.reducePreRenderedFrames) {
            frameQueue.ReduceFrameQueue(config.maxPreRenderedFrames);
        }

        if (config.highPriorityInput) {
            timingOptimizer.Start();
        }

        SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);

        active = true;
        std::cout << "[Input Lag Fix] Initialized — low latency mode active" << std::endl;
    }

    void Shutdown() {
        timingOptimizer.Stop();
        frameQueue.Restore();
        rawInput.Unregister();
        SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
        active = false;
        std::cout << "[Input Lag Fix] Shutdown — restored defaults" << std::endl;
    }
};

} // namespace CrimsonTweaks
