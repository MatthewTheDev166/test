#include "ConnectionBadge.hpp"
#include "StatusPopup.hpp"
#include "../server/WebSocketServer.hpp"
#include "../server/AdbManager.hpp"
#include <Geode/Geode.hpp>

using namespace geode::prelude;

CCMenuItemSpriteExtra* ConnectionBadge::createButton(CCObject* target) {
    // Base icon
    auto icon = CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png");
    if (!icon) {
        icon = CCSprite::create();
    }
    icon->setScale(0.6f);

    // Add a connection indicator dot
    bool hasClients = WebSocketServer::get().getClientCount() > 0;
    bool hasDevice = AdbManager::get().isDeviceConnected();

    auto dot = CCSprite::createWithSpriteFrameName("bonusGem_001.png");
    if (dot) {
        dot->setScale(0.5f);
        if (hasClients) {
            dot->setColor({0, 255, 100}); // Green
        } else if (hasDevice) {
            dot->setColor({255, 200, 50}); // Yellow
        } else {
            dot->setColor({120, 120, 120}); // Gray
        }
        dot->setPosition({icon->getContentSize().width - 5.f, 5.f});
        icon->addChild(dot);
    }

    auto btn = CCMenuItemSpriteExtra::create(
        icon,
        target,
        menu_selector(ConnectionBadge::onButtonClicked)
    );
    btn->setID("mobile-stats-badge"_spr);

    return btn;
}

void ConnectionBadge::onButtonClicked(CCObject* sender) {
    auto popup = StatusPopup::create();
    if (popup) {
        popup->show();
    }
}
