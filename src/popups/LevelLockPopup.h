#pragma once
#include <Geode/Geode.hpp>
#include "StreakData.h"

using namespace geode::prelude;

class LevelLockPopup : public Popup<> {
protected:
    bool setup() override {
        auto popupSize = m_mainLayer->getContentSize();

        // --- SECCIÓN DEL TÍTULO (SOLO IMAGEN) ---

        // 1. Creamos el sprite usando el sufijo _spr
        auto titleIcon = CCSprite::create("competitive.png"_spr);

        // Altura donde se ubicará el título (parte superior)
        float titlePosY = popupSize.height - 48.f;

        if (titleIcon) {
            // 2. Lo hacemos más grande (antes 0.6f -> ahora 0.85f)
            titleIcon->setScale(1.5f);

            // 3. Lo centramos horizontalmente
            titleIcon->setPosition({ popupSize.width / 2, titlePosY });
            m_mainLayer->addChild(titleIcon);
        }
        else {
            // Fallback de seguridad: Si la imagen falla al cargar, mostramos texto simple
            // para que no quede vacío, pero normalmente se verá la imagen.
            auto fallback = CCLabelBMFont::create("Locked", "goldFont.fnt");
            fallback->setPosition({ popupSize.width / 2, titlePosY });
            fallback->setScale(0.7f);
            m_mainLayer->addChild(fallback);
        }
        // --- FIN TÍTULO ---


        // 4. Icono de candado principal
        auto lockSprite = CCSprite::createWithSpriteFrameName("GJ_lockGray_001.png");
        if (lockSprite) {
            lockSprite->setScale(1.2f);
            lockSprite->setPosition({ popupSize.width / 2, popupSize.height / 2 + 30.f });
            m_mainLayer->addChild(lockSprite);
        }

        // 5. Texto explicativo
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

        // 6. Estado actual (Nivel)
        int currentLvl = g_streakData.currentLevel;
        auto statusLabel = CCLabelBMFont::create(
            fmt::format("Current Level: {} / 7", currentLvl).c_str(),
            "goldFont.fnt"
        );
        statusLabel->setScale(0.6f);
        statusLabel->setPosition({ popupSize.width / 2, popupSize.height / 2 - 55.f });
        m_mainLayer->addChild(statusLabel);

        // 7. Botón OK
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
        if (ret && ret->initAnchored(280.f, 240.f, "geode.loader/GE_square01.png")) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};