#pragma once
#include <Geode/Geode.hpp>
#include "StreakData.h"

using namespace geode::prelude;

namespace StreakMusic {
    enum Mode { Normal = 0, Off = 1 };

    inline bool s_playing = false;
    inline bool s_inStreakMenu = false;

    inline int getMode() {
        return Mod::get()->getSavedValue<int>("streak_music_mode", 0);
    }

    inline float getVolume() {
        double v = Mod::get()->getSavedValue<double>(STREAK_MENU_MUSIC_VOLUME_KEY, 0.5);
        return static_cast<float>(std::clamp(v, 0.0, 1.0));
    }

    inline void start() {
        auto eng = FMODAudioEngine::sharedEngine();
        if (!eng) return;
        eng->pauseAllMusic(true);
        float volF = getVolume();
        eng->stopChannel(STREAK_MENU_MUSIC_CHANNEL, AudioTargetType::SFXChannel, false, 0.f);
        eng->playEffectAdvanced(
            std::string("streak_meu_loop.mp3"_spr),
            1.0f, 1.0f, volF, 1.0f,
            false, false,
            0, 0, 0, 0,
            true,
            0, true, false,
            STREAK_MENU_MUSIC_CHANNEL,
            0, 0.0f, 0
        );
        eng->setChannelVolume(STREAK_MENU_MUSIC_CHANNEL, AudioTargetType::SFXChannel, volF);
        s_playing = true;
    }

    inline void stop() {
        auto eng = FMODAudioEngine::sharedEngine();
        if (!eng) return;
        eng->stopChannel(STREAK_MENU_MUSIC_CHANNEL, AudioTargetType::SFXChannel, false, 0.f);
        eng->resumeAllMusic();
        s_playing = false;
    }

    inline void resync() {
        if (!s_playing) return;
        auto eng = FMODAudioEngine::sharedEngine();
        if (!eng) return;
        eng->pauseAllMusic(true);
        eng->setChannelVolume(STREAK_MENU_MUSIC_CHANNEL, AudioTargetType::SFXChannel, getVolume());
    }

    inline void onMenuEnter() {
        s_inStreakMenu = true;
        if (getMode() == Off) return;
        if (s_playing) resync();
        else start();
    }

    inline void onMenuExit() {
        s_inStreakMenu = false;
        stop();
    }

    inline void applyModeChange() {
        if (getMode() == Off) {
            stop();
        } else if (s_inStreakMenu) {
            if (s_playing) resync();
            else start();
        }
    }
}
