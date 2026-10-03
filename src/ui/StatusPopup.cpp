#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#include "StatusPopup.hpp"
#include "../server/WebSocketServer.hpp"
#include "../server/AdbManager.hpp"
#include <Geode/Geode.hpp>

using namespace geode::prelude;

namespace {
    std::string getLocalIP() {
#ifdef GEODE_IS_WINDOWS
        char hostname[256];
        if (gethostname(hostname, sizeof(hostname)) == 0) {
            addrinfo hints{}, *info = nullptr;
            hints.ai_family = AF_INET;
            hints.ai_socktype = SOCK_STREAM;
            if (getaddrinfo(hostname, nullptr, &hints, &info) == 0 && info) {
                for (auto p = info; p != nullptr; p = p->ai_next) {
                    sockaddr_in* addr = reinterpret_cast<sockaddr_in*>(p->ai_addr);
                    char ipStr[INET_ADDRSTRLEN];
                    if (inet_ntop(AF_INET, &(addr->sin_addr), ipStr, sizeof(ipStr))) {
                        std::string s(ipStr);
                        if (s != "127.0.0.1") {
                            freeaddrinfo(info);
                            return s;
                        }
                    }
                }
                freeaddrinfo(info);
            }
        }
#endif
        return "192.168.x.x";
    }
}

StatusPopup* StatusPopup::create() {
    auto ret = new StatusPopup();
    if (ret && ret->init(360.f, 250.f)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool StatusPopup::init(float width, float height) {
    if (!Popup::init(width, height)) {
        return false;
    }

    this->setTitle("Mobile HUD Status");

    auto center = m_mainLayer->getContentSize() / 2;

    uint16_t port = WebSocketServer::get().getPort();
    size_t clients = WebSocketServer::get().getClientCount();
    bool adbRev = AdbManager::get().isReverseActive();
    std::string lanIp = getLocalIP();

    // USB Cable URL
    std::string usbUrl = fmt::format("USB (Cable): http://localhost:{}", port);
    auto labelUsb = CCLabelBMFont::create(usbUrl.c_str(), "chatFont.fnt");
    labelUsb->setScale(0.82f);
    labelUsb->setColor({0, 255, 240}); // Neon Cyan
    labelUsb->setPosition(center.width, center.height + 55.f);
    m_mainLayer->addChild(labelUsb);

    // Wi-Fi Wireless URL
    std::string wifiUrl = fmt::format("Wi-Fi (Wireless): http://{}:{}", lanIp, port);
    auto labelWifi = CCLabelBMFont::create(wifiUrl.c_str(), "chatFont.fnt");
    labelWifi->setScale(0.82f);
    labelWifi->setColor({0, 255, 100}); // Neon Green
    labelWifi->setPosition(center.width, center.height + 30.f);
    m_mainLayer->addChild(labelWifi);

    // Connected Phones
    std::string clientStatus = fmt::format("Connected Phones: {}", clients);
    auto labelClients = CCLabelBMFont::create(clientStatus.c_str(), "chatFont.fnt");
    labelClients->setScale(0.75f);
    labelClients->setColor(clients > 0 ? ccColor3B{0, 255, 100} : ccColor3B{200, 200, 200});
    labelClients->setPosition(center.width, center.height + 5.f);
    m_mainLayer->addChild(labelClients);

    // USB Tunnel status
    std::string adbStatus = fmt::format("USB Tunnel (ADB): {}", 
        adbRev ? "Active (Connected)" : "Ready (Waiting for cable)");
    auto labelAdb = CCLabelBMFont::create(adbStatus.c_str(), "chatFont.fnt");
    labelAdb->setScale(0.72f);
    labelAdb->setColor(adbRev ? ccColor3B{0, 255, 100} : ccColor3B{255, 190, 60});
    labelAdb->setPosition(center.width, center.height - 18.f);
    m_mainLayer->addChild(labelAdb);

    // Tip
    auto labelTip = CCLabelBMFont::create("Open either URL in any phone browser!", "chatFont.fnt");
    labelTip->setScale(0.65f);
    labelTip->setColor({160, 160, 160});
    labelTip->setPosition(center.width, center.height - 42.f);
    m_mainLayer->addChild(labelTip);

    // Settings Button
    auto settingsSpr = ButtonSprite::create("Settings", "goldFont.fnt", "GJ_button_01.png", 0.7f);
    auto settingsBtn = CCMenuItemSpriteExtra::create(
        settingsSpr,
        this,
        menu_selector(StatusPopup::onOpenSettings)
    );
    settingsBtn->setPosition(0.f, -85.f);
    m_buttonMenu->addChild(settingsBtn);

    return true;
}

void StatusPopup::onOpenSettings(CCObject* sender) {
    geode::openSettingsPopup(Mod::get());
}
