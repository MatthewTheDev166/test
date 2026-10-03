#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#include "AdbManager.hpp"
#include <Geode/Geode.hpp>

#include <chrono>
#include <sstream>

AdbManager& AdbManager::get() {
    static AdbManager instance;
    return instance;
}

AdbManager::AdbManager() = default;

AdbManager::~AdbManager() {
    stop();
}

std::filesystem::path AdbManager::locateAdb() {
    using namespace geode::prelude;

    // Isolate platform-tools outside Steam's directory (inside Mod save directory).
    // Steam monitors the game directory (steamapps/common/Geometry Dash/) and considers
    // the game running as long as any executable inside that directory (like adb.exe) is alive!
    std::filesystem::path isolatedDir;
    try {
        isolatedDir = Mod::get()->getSaveDir() / "platform-tools";
    } catch (...) {
        isolatedDir = std::filesystem::temp_directory_path() / "gd-mobile-stats" / "platform-tools";
    }

    auto isolatedAdb = isolatedDir / "adb.exe";

    auto copyDependencies = [&](const std::filesystem::path& srcDir) {
        try {
            std::filesystem::create_directories(isolatedDir);
            for (const auto& fname : {"adb.exe", "AdbWinApi.dll", "AdbWinUsbApi.dll"}) {
                auto src = srcDir / fname;
                auto dst = isolatedDir / fname;
                if (std::filesystem::exists(src)) {
                    std::error_code ec;
                    std::filesystem::copy_file(src, dst, std::filesystem::copy_options::overwrite_existing, ec);
                }
            }
        } catch (...) {}
    };

    // 1. If already isolated and present, use it
    if (std::filesystem::exists(isolatedAdb)) {
        return isolatedAdb;
    }

    // 2. Mod resources folder
    auto resDir = Mod::get()->getResourcesDir();
    if (std::filesystem::exists(resDir / "platform-tools" / "adb.exe")) {
        copyDependencies(resDir / "platform-tools");
        if (std::filesystem::exists(isolatedAdb)) return isolatedAdb;
    }
    if (std::filesystem::exists(resDir / "adb.exe")) {
        copyDependencies(resDir);
        if (std::filesystem::exists(isolatedAdb)) return isolatedAdb;
    }

    // 3. Game folder (migrate it out of Steam)
    auto gameDir = dirs::getGameDir();
    if (std::filesystem::exists(gameDir / "platform-tools" / "adb.exe")) {
        copyDependencies(gameDir / "platform-tools");
        if (std::filesystem::exists(isolatedAdb)) return isolatedAdb;
    }

    // 4. Workspace / local directory (development mode)
    std::filesystem::path localDevPath = "platform-tools/adb.exe";
    if (std::filesystem::exists(localDevPath)) {
        copyDependencies("platform-tools");
        if (std::filesystem::exists(isolatedAdb)) return isolatedAdb;
    }

    // 5. System PATH fallback
    return "adb.exe";
}

bool AdbManager::executeSilent(const std::string& cmdLine, std::string* output) {
#ifdef GEODE_IS_WINDOWS
    HANDLE hReadPipe = NULL;
    HANDLE hWritePipe = NULL;
    SECURITY_ATTRIBUTES sa{};
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    if (output) {
        if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0)) {
            return false;
        }
        SetHandleInformation(hReadPipe, HANDLE_FLAG_INHERIT, 0);
    }

    STARTUPINFOA si{};
    si.cb = sizeof(STARTUPINFOA);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    if (output) {
        si.dwFlags |= STARTF_USESTDHANDLES;
        si.hStdOutput = hWritePipe;
        si.hStdError = hWritePipe;
    }

    PROCESS_INFORMATION pi{};
    std::string commandCopy = cmdLine;

    DWORD creationFlags = CREATE_NO_WINDOW;
#ifdef CREATE_BREAKAWAY_FROM_JOB
    creationFlags |= CREATE_BREAKAWAY_FROM_JOB;
#endif

    BOOL success = CreateProcessA(
        NULL,
        commandCopy.data(),
        NULL,
        NULL,
        output ? TRUE : FALSE,
        creationFlags,
        NULL,
        NULL,
        &si,
        &pi
    );

    if (!success && (creationFlags & CREATE_BREAKAWAY_FROM_JOB)) {
        creationFlags = CREATE_NO_WINDOW;
        success = CreateProcessA(
            NULL,
            commandCopy.data(),
            NULL,
            NULL,
            output ? TRUE : FALSE,
            creationFlags,
            NULL,
            NULL,
            &si,
            &pi
        );
    }

    if (output) {
        CloseHandle(hWritePipe);
    }

    if (!success) {
        if (output) {
            CloseHandle(hReadPipe);
        }
        return false;
    }

    if (output) {
        char buffer[512];
        DWORD bytesRead = 0;
        output->clear();
        while (ReadFile(hReadPipe, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
            buffer[bytesRead] = '\0';
            output->append(buffer, bytesRead);
        }
        CloseHandle(hReadPipe);
    }

    WaitForSingleObject(pi.hProcess, 3000); // 3 sec timeout
    DWORD exitCode = 0;
    GetExitCodeProcess(pi.hProcess, &exitCode);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return (exitCode == 0);
#else
    return false;
#endif
}

void AdbManager::start(uint16_t port) {
    if (m_running.load()) return;
    m_port = port;

    auto adbPath = locateAdb();
    m_cachedAdbPath = adbPath.string();
    geode::log::info("AdbManager using ADB binary at: {}", m_cachedAdbPath);

    m_running.store(true);
    m_workerThread = std::thread(&AdbManager::adbLoop, this);
}

void AdbManager::stop() {
    if (!m_running.load()) return;
    m_running.store(false);

    if (m_workerThread.joinable()) {
        m_workerThread.join();
    }

    // Kill ADB daemon so it never lingers after GD exits
    if (!m_cachedAdbPath.empty()) {
        executeSilent("\"" + m_cachedAdbPath + "\" kill-server", nullptr);
    }

    m_deviceConnected.store(false);
    m_reverseActive.store(false);
}

void AdbManager::adbLoop() {
    using namespace geode::prelude;

    // Start server first
    executeSilent("\"" + m_cachedAdbPath + "\" start-server", nullptr);

    while (m_running.load()) {
        bool adbEnabled = Mod::get()->getSettingValue<bool>("enable-adb-reverse");
        int intervalSec = static_cast<int>(Mod::get()->getSettingValue<int64_t>("adb-interval-seconds"));
        if (intervalSec < 1) intervalSec = 1;

        if (adbEnabled) {
            // Check connected devices
            std::string devicesOutput;
            bool ok = executeSilent("\"" + m_cachedAdbPath + "\" devices", &devicesOutput);

            bool foundDevice = false;
            if (ok && !devicesOutput.empty()) {
                std::istringstream iss(devicesOutput);
                std::string line;
                while (std::getline(iss, line)) {
                    if (line.find("List of devices") != std::string::npos || line.empty()) {
                        continue;
                    }
                    if (line.find("device") != std::string::npos && line.find("offline") == std::string::npos) {
                        foundDevice = true;
                        break;
                    }
                }
            }

            m_deviceConnected.store(foundDevice);

            if (foundDevice) {
                // Run adb reverse tcp:PORT tcp:PORT
                std::string reverseCmd = "\"" + m_cachedAdbPath + "\" reverse tcp:" + 
                                         std::to_string(m_port) + " tcp:" + std::to_string(m_port);
                bool revOk = executeSilent(reverseCmd, nullptr);
                m_reverseActive.store(revOk);
            } else {
                m_reverseActive.store(false);
            }
        } else {
            m_deviceConnected.store(false);
            m_reverseActive.store(false);
        }

        // Sleep in 200ms increments to allow fast stop
        for (int i = 0; i < intervalSec * 5 && m_running.load(); ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    }

    if (!m_cachedAdbPath.empty()) {
        executeSilent("\"" + m_cachedAdbPath + "\" kill-server", nullptr);
    }
}
