#pragma once
#include <string>
#include <vector>
#include <functional>
#include <thread>
#include <atomic>
#include <mutex>
#include <cstdint>

class WebSocketServer {
public:
    static WebSocketServer& get();

    bool start(uint16_t port = 34567);
    void stop();
    bool isRunning() const;

    void broadcast(const std::string& text);
    size_t getClientCount();
    uint16_t getPort() const { return m_port; }

    void setOnMessage(std::function<void(const std::string&)> callback);

private:
    WebSocketServer();
    ~WebSocketServer();

    void serverLoop();
    void handleClient(uintptr_t clientSocket);
    bool doHandshake(uintptr_t clientSocket, const std::string& request);
    void sendFrame(uintptr_t clientSocket, const std::string& text);

    std::atomic<bool> m_running{false};
    uint16_t m_port{34567};
    uintptr_t m_listenSocket{static_cast<uintptr_t>(~0)}; // INVALID_SOCKET

    std::thread m_serverThread;
    std::mutex m_clientsMutex;
    std::vector<uintptr_t> m_clients;

    std::function<void(const std::string&)> m_onMessage;
};
