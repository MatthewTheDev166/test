#include <Geode/Geode.hpp>
#include "server/WebSocketServer.hpp"
#include "server/AdbManager.hpp"
#include "stats/StatsManager.hpp"

using namespace geode::prelude;

$on_mod(Loaded) {
    uint16_t port = static_cast<uint16_t>(Mod::get()->getSettingValue<int64_t>("server-port"));
    if (port == 0) port = 34567;

    log::info("GD Mobile Stats initializing on port {}...", port);

    // Setup incoming message handler
    WebSocketServer::get().setOnMessage([](const std::string& msg) {
        StatsManager::get().handleClientMessage(msg);
    });

    // Start WebSocket server
    WebSocketServer::get().start(port);

    // Start ADB daemon for USB reverse port forwarding
    AdbManager::get().start(port);

    // Listen to setting changes
    listenForSettingChanges<int64_t>("server-port", [](int64_t newPort) {
        if (newPort > 0 && newPort <= 65535) {
            uint16_t p = static_cast<uint16_t>(newPort);
            WebSocketServer::get().start(p);
            AdbManager::get().start(p);
        }
    });
}
