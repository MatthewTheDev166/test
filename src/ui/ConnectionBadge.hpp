#pragma once
#include <Geode/Geode.hpp>

class ConnectionBadge {
public:
    static CCMenuItemSpriteExtra* createButton(cocos2d::CCObject* target);
    static void onButtonClicked(cocos2d::CCObject* sender);
};
