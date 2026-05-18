#pragma once
#include "StreakCommon.h"
#include "../StreakData.h"
#include "../FirebaseManager.h"
#include "../StatusSpinner.h"
#include <Geode/ui/Popup.hpp>
#include <Geode/utils/cocos.hpp>

using namespace geode::prelude;

class ShieldsPopup : public Popup {
protected:
    std::function<void()> m_onChange;
    CCMenuItemToggler* m_toggle = nullptr;
    CCLabelBMFont* m_countLabel = nullptr;
    StatusSpinner* m_spinner = nullptr;
    bool m_busy = false;

    bool init() override {
        if (!Popup::init(240.f, 180.f, "GJ_square04.png")) return false;

        this->setTitle("Streak Shields");

        auto winSize = m_mainLayer->getContentSize();
        float cx = winSize.width / 2.f;
        float cy = winSize.height / 2.f;

        auto heart = CCSprite::create("heart.png"_spr);
        if (heart) {
            heart->setScale(0.3f);
            heart->setPosition({ cx, cy + 22.f });
            m_mainLayer->addChild(heart, 5);
        }

        m_countLabel = CCLabelBMFont::create(
            fmt::format("x{}", g_streakData.streakShields).c_str(),
            "bigFont.fnt"
        );
        m_countLabel->setScale(0.55f);
        m_countLabel->setColor({ 255, 130, 130 });
        m_countLabel->setPosition({ cx, cy - 8.f });
        m_mainLayer->addChild(m_countLabel, 5);

        auto infoSpr = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
        if (infoSpr) {
            infoSpr->setScale(0.6f);
            auto infoBtn = CCMenuItemSpriteExtra::create(
                infoSpr, this, menu_selector(ShieldsPopup::onInfoClick)
            );
            auto infoMenu = CCMenu::create();
            infoMenu->setPosition({ 0, 0 });
            infoBtn->setPosition({ winSize.width - 18.f, winSize.height - 18.f });
            infoMenu->addChild(infoBtn);
            m_mainLayer->addChild(infoMenu, 10);
        }

        auto onSpr  = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
        auto offSpr = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
        if (onSpr) onSpr->setScale(0.8f);
        if (offSpr) offSpr->setScale(0.8f);

        m_toggle = CCMenuItemToggler::create(
            offSpr, onSpr, this, menu_selector(ShieldsPopup::onToggle)
        );
        m_toggle->toggle(g_streakData.shieldsEnabled);

        auto toggleLabel = CCLabelBMFont::create("Activated", "bigFont.fnt");
        toggleLabel->setScale(0.4f);
        toggleLabel->setAnchorPoint({ 1.0f, 0.5f });

        auto toggleMenu = CCMenu::create();
        toggleMenu->setPosition({ cx, cy - 42.f });
        toggleLabel->setPosition({ -8.f, 0.f });
        m_toggle->setPosition({ 14.f, 0.f });
        toggleMenu->addChild(toggleLabel);
        toggleMenu->addChild(m_toggle);
        m_mainLayer->addChild(toggleMenu, 5);

        m_spinner = StatusSpinner::create();
        m_spinner->setPosition({ cx + 50.f, cy - 42.f });
        m_spinner->setScale(0.35f);
        m_mainLayer->addChild(m_spinner, 20);

        return true;
    }

    void onToggle(CCObject* sender) {
        if (m_busy) {
            if (m_toggle) m_toggle->toggle(g_streakData.shieldsEnabled);
            return;
        }

        bool desired = !g_streakData.shieldsEnabled;

        if (m_toggle) {
            m_toggle->toggle(g_streakData.shieldsEnabled);
            m_toggle->setEnabled(false);
        }
        m_busy = true;
        if (m_spinner) m_spinner->setLoading("");

        matjson::Value payload = matjson::Value::object();
        payload.set("enabled", desired);

        claimOnServer("/shields/toggle", payload, [this, desired](bool ok) {
            m_busy = false;
            if (m_toggle) m_toggle->setEnabled(true);

            if (ok) {
                g_streakData.shieldsEnabled = desired;
                if (m_toggle) m_toggle->toggle(desired);
                if (m_spinner) m_spinner->hide();
                if (m_onChange) m_onChange();
            } else {
                if (m_toggle) m_toggle->toggle(g_streakData.shieldsEnabled);
                if (m_spinner) m_spinner->setError("Error");
            }
        });
    }

    void onInfoClick(CCObject*) {
        const char* body =
            "Shields protect your streak when you miss days.\n\n"
            "If you lose <cr>N</c> days and have at least <cr>N</c> shields, "
            "they are consumed and your streak survives.\n\n"
            "If you don't have enough, your streak resets and no shield is used.";
        auto alert = FLAlertLayer::create(nullptr, "Streak Shields", body, "OK", nullptr, 360.f);
        if (alert) alert->show();
    }

public:
    static ShieldsPopup* create(std::function<void()> onChange = nullptr) {
        auto ret = new ShieldsPopup();
        ret->m_onChange = onChange;
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};
