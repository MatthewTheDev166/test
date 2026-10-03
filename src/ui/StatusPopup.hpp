#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>

class StatusPopup : public geode::Popup {
public:
    static StatusPopup* create();
    bool init(float width, float height);
    void onOpenSettings(cocos2d::CCObject* sender);
};
