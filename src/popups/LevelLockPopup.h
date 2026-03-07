#pragma once
#include <Geode/Geode.hpp>
#include "../StreakData.h"

using namespace geode::prelude;

class LevelLockPopup : public Popup {
protected:
    bool init() override {
        if (!Popup::init(280.f, 240.f, "geode.loader/GE_square01.png")) return false;

        auto popupSize = m_mainLayer->getContentSize();

        auto titleIcon = CCSprite::create("competitive.png"_spr);
        float titlePosY = popupSize.height - 48.f;

        if (titleIcon) {
            titleIcon->setScale(1.5f);
            titleIcon->setPosition({ popupSize.width / 2, titlePosY });
            m_mainLayer->addChild(titleIcon);
        }
        else {
            auto fallback = CCLabelBMFont::create("Locked", "goldFont.fnt");
            fallback->setPosition({ popupSize.width / 2, titlePosY });
            fallback->setScale(0.7f);
            m_mainLayer->addChild(fallback);
        }

        auto lockSprite = CCSprite::createWithSpriteFrameName("GJ_lockGray_001.png");
        if (lockSprite) {
            lockSprite->setScale(1.2f);
            lockSprite->setPosition({ popupSize.width / 2, popupSize.height / 2 + 30.f });
            m_mainLayer->addChild(lockSprite);
        }

        auto infoLabel = CCLabelBMFont::create(
            "You need to reach Level 7\nto unlock the Global Leaderboard!",
            "bigFont.fnt",
            popupSize.width - 40.0f,
            CCTextAlignment::kCCTextAlignmentCenter
        );
        infoLabel->setScale(0.45f);
        infoLabel->setColor({ 255, 100, 100 });
        infoLabel->setPosition({ popupSize.width / 2, popupSize.height / 2 - 25.f });
        m_mainLayer->addChild(infoLabel);

        int currentLvl = g_streakData.currentLevel;
        auto statusLabel = CCLabelBMFont::create(
            fmt::format("Current Level: {} / 7", currentLvl).c_str(),
            "goldFont.fnt"
        );
        statusLabel->setScale(0.6f);
        statusLabel->setPosition({ popupSize.width / 2, popupSize.height / 2 - 55.f });
        m_mainLayer->addChild(statusLabel);

        auto btnSpr = ButtonSprite::create("OK");
        auto okBtn = CCMenuItemSpriteExtra::create(
            btnSpr,
            this,
            menu_selector(LevelLockPopup::onClose)
        );
        auto menu = CCMenu::createWithItem(okBtn);
        menu->setPosition({ popupSize.width / 2, 30.f });
        m_mainLayer->addChild(menu);

        return true;
    }

public:
    static LevelLockPopup* create() {
        auto ret = new LevelLockPopup();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};