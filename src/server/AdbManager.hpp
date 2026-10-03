#pragma once
#include <string>
#include <filesystem>
#include <thread>
#include <atomic>
#include <cstdint>

class AdbManager {
public:
    static AdbManager& get();

    void start(uint16_t port);
    void stop();
    bool isDeviceConnected() const { return m_deviceConnected.load(); }
    bool isReverseActive() const { return m_reverseActive.load(); }
    std::string getAdbPath() const { return m_cachedAdbPath; }

private:
    AdbManager();
    ~AdbManager();

    std::filesystem::path locateAdb();
    void adbLoop();
    bool executeSilent(const std::string& command, std::string* output = nullptr);

    std::atomic<bool> m_running{false};
    std::atomic<bool> m_deviceConnected{false};
    std::atomic<bool> m_reverseActive{false};
    uint16_t m_port{34567};
    std::string m_cachedAdbPath;

    std::thread m_workerThread;
};
