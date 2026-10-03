#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include "../stats/StatsManager.hpp"
#include "../ui/ConnectionBadge.hpp"

using namespace geode::prelude;

class $modify(MobileStatsMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) {
            return false;
        }

        StatsManager::get().onMenu();

        // Add badge to bottom menu or right menu
        auto badge = ConnectionBadge::createButton(this);
        if (auto menu = this->getChildByID("bottom-menu")) {
            menu->addChild(badge);
            menu->updateLayout();
        } else if (auto rightMenu = this->getChildByID("right-side-menu")) {
            rightMenu->addChild(badge);
            rightMenu->updateLayout();
        }

        return true;
    }
};
