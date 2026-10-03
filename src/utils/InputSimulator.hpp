#pragma once
#include <Geode/Geode.hpp>

#ifdef GEODE_IS_WINDOWS
#include <windows.h>
#endif

namespace InputSimulator {

    /**
     * Simulates a key down and key up event using Windows SendInput.
     * Compatible with MegaHack and other mods listening to key events.
     */
    inline void pressKey(char keyChar) {
#ifdef GEODE_IS_WINDOWS
        WORD vk = 0;
        switch (keyChar) {
            case 'Q': case 'q': vk = 'Q'; break;
            case 'E': case 'e': vk = 'E'; break;
            case 'R': case 'r': vk = 'R'; break;
            default: vk = static_cast<WORD>(VkKeyScanA(keyChar) & 0xFF); break;
        }

        if (vk == 0) return;

        INPUT inputs[2] = {};

        // Key Down
        inputs[0].type = INPUT_KEYBOARD;
        inputs[0].ki.wVk = vk;
        inputs[0].ki.wScan = static_cast<WORD>(MapVirtualKeyA(vk, MAPVK_VK_TO_VSC));
        inputs[0].ki.dwFlags = 0;

        // Key Up
        inputs[1].type = INPUT_KEYBOARD;
        inputs[1].ki.wVk = vk;
        inputs[1].ki.wScan = inputs[0].ki.wScan;
        inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;

        SendInput(2, inputs, sizeof(INPUT));
#endif
    }

    inline void triggerPrevStartPos() {
        pressKey('Q');
    }

    inline void triggerNextStartPos() {
        pressKey('E');
    }

    inline void triggerRespawn() {
        pressKey('R');
    }
}
