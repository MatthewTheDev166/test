#include "StatusPopup.hpp"
#include "../server/WebSocketServer.hpp"
#include "../server/AdbManager.hpp"
#include <Geode/Geode.hpp>

using namespace geode::prelude;

StatusPopup* StatusPopup::create() {
    auto ret = new StatusPopup();
    if (ret && ret->init(340.f, 220.f)) {
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

    auto winSize = CCDirector::get()->getWinSize();
    auto center = m_mainLayer->getContentSize() / 2;

    // Server Info
    uint16_t port = WebSocketServer::get().getPort();
    size_t clients = WebSocketServer::get().getClientCount();
    bool adbRev = AdbManager::get().isReverseActive();
    bool devConnected = AdbManager::get().isDeviceConnected();

    std::string serverStatus = fmt::format("Server: Port {} ({})", port, 
        WebSocketServer::get().isRunning() ? "Active" : "Stopped");
    auto labelServer = CCLabelBMFont::create(serverStatus.c_str(), "chatFont.fnt");
    labelServer->setScale(0.8f);
    labelServer->setPosition(center.width, center.height + 40.f);
    m_mainLayer->addChild(labelServer);

    // Clients connected info
    std::string clientStatus = fmt::format("Connected Phones: {}", clients);
    auto labelClients = CCLabelBMFont::create(clientStatus.c_str(), "chatFont.fnt");
    labelClients->setScale(0.8f);
    labelClients->setColor(clients > 0 ? ccColor3B{0, 255, 100} : ccColor3B{200, 200, 200});
    labelClients->setPosition(center.width, center.height + 15.f);
    m_mainLayer->addChild(labelClients);

    // USB / ADB Reverse status
    std::string adbStatus = fmt::format("USB Cable (ADB): {}", 
        adbRev ? "Tunnel Active" : (devConnected ? "Device Detected" : "Waiting for USB"));
    auto labelAdb = CCLabelBMFont::create(adbStatus.c_str(), "chatFont.fnt");
    labelAdb->setScale(0.75f);
    labelAdb->setColor(adbRev ? ccColor3B{0, 255, 100} : ccColor3B{255, 200, 80});
    labelAdb->setPosition(center.width, center.height - 10.f);
    m_mainLayer->addChild(labelAdb);

    // Tip
    auto labelTip = CCLabelBMFont::create("Connect phone via USB-C or Wi-Fi", "chatFont.fnt");
    labelTip->setScale(0.65f);
    labelTip->setColor({160, 160, 160});
    labelTip->setPosition(center.width, center.height - 35.f);
    m_mainLayer->addChild(labelTip);

    // Settings Button
    auto settingsSpr = ButtonSprite::create("Settings", "goldFont.fnt", "GJ_button_01.png", 0.7f);
    auto settingsBtn = CCMenuItemSpriteExtra::create(
        settingsSpr,
        this,
        menu_selector(StatusPopup::onOpenSettings)
    );
    settingsBtn->setPosition(0.f, -70.f);
    m_buttonMenu->addChild(settingsBtn);

    return true;
}

void StatusPopup::onOpenSettings(CCObject* sender) {
    geode::openSettingsPopup(Mod::get());
}
