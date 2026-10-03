#include "AdbManager.hpp"
#include <Geode/Geode.hpp>

#ifdef GEODE_IS_WINDOWS
#include <windows.h>
#endif

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

    // 1. Mod resources folder
    auto resDir = Mod::get()->getResourcesDir();
    if (std::filesystem::exists(resDir / "platform-tools" / "adb.exe")) {
        return resDir / "platform-tools" / "adb.exe";
    }
    if (std::filesystem::exists(resDir / "adb.exe")) {
        return resDir / "adb.exe";
    }

    // 2. Game folder / platform-tools
    auto gameDir = dirs::getGameDir();
    if (std::filesystem::exists(gameDir / "platform-tools" / "adb.exe")) {
        return gameDir / "platform-tools" / "adb.exe";
    }

    // 3. Workspace / local directory (development mode)
    std::filesystem::path localDevPath = "platform-tools/adb.exe";
    if (std::filesystem::exists(localDevPath)) {
        return std::filesystem::absolute(localDevPath);
    }

    // 4. Mod save / config dir
    auto saveDir = Mod::get()->getSaveDir();
    if (std::filesystem::exists(saveDir / "platform-tools" / "adb.exe")) {
        return saveDir / "platform-tools" / "adb.exe";
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

    BOOL success = CreateProcessA(
        NULL,
        commandCopy.data(),
        NULL,
        NULL,
        output ? TRUE : FALSE,
        CREATE_NO_WINDOW,
        NULL,
        NULL,
        &si,
        &pi
    );

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
}
