#pragma once
#include "StreakCommon.h"
#include "../StreakData.h"
#include "../FirebaseManager.h"
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/MDTextArea.hpp>
#include <fstream>
#include <sstream>
 

class TermsScrollPopup : public Popup {
protected:
    bool          m_isSpanish = false;
    MDTextArea* m_mdArea = nullptr;
    CCScale9Sprite* m_bgBox = nullptr;
    CCMenuItemSpriteExtra* m_langBtn = nullptr;
    CCSize m_mdSize;



    std::string readTermsFile(bool spanish) {
        std::string filename = spanish ? "terms_es.md" : "terms_en.md";
        auto path = Mod::get()->getResourcesDir() / filename;

        std::ifstream file(path);
        if (!file.is_open()) {
            return spanish
                ? "# Error\nNo se pudo cargar el archivo de terminos."
                : "# Error\nCould not load the terms file.";
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    void buildMDArea() {
        if (m_mdArea) {
            m_mdArea->removeFromParent();
            m_mdArea = nullptr;
        }

        auto winSize = m_mainLayer->getContentSize();
        float cx = winSize.width / 2.f;
        float cy = m_bgBox->getPositionY();

        std::string content = readTermsFile(m_isSpanish);

        m_mdArea = MDTextArea::create(content, m_mdSize);
        m_mdArea->setPosition({ cx, cy });
        m_mainLayer->addChild(m_mdArea, 1);
    }   

    bool init() override {
        if (!Popup::init(330.f, 265.f, "geode.loader/GE_square03.png")) {
            return false;
        }

        this->setTitle("Terms & Conditions");
        auto winSize = m_mainLayer->getContentSize();
        float cx = winSize.width / 2.f;
        float mdW = winSize.width - 28.f;
        float mdH = winSize.height - 50.f;
        float boxY = 10.f + mdH / 2.f;
        m_mdSize = CCSize(mdW - 12.f, mdH - 12.f);    
        m_bgBox = CCScale9Sprite::create("square02_001.png");
        m_bgBox->setContentSize(CCSize(mdW, mdH));
        m_bgBox->setPosition({ cx, boxY });
        m_bgBox->setColor({ 0, 0, 0 });
        m_bgBox->setOpacity(160);
        m_mainLayer->addChild(m_bgBox, 0);   
        auto langBtnSpr = ButtonSprite::create(
            "ES", 0, 0, "bigFont.fnt", "GJ_button_05.png", 0, 0.55f
        );
        m_langBtn = CCMenuItemSpriteExtra::create(
            langBtnSpr,
            this,
            menu_selector(TermsScrollPopup::onToggleLang)
        );

        auto langMenu = CCMenu::createWithItem(m_langBtn);
        langMenu->setPosition({
            winSize.width - 22.f,
            winSize.height - 16.f
            });
        m_mainLayer->addChild(langMenu, 2);

  
        buildMDArea();
        return true;
    }

    

    void onToggleLang(CCObject*) {
        m_isSpanish = !m_isSpanish;

        this->setTitle(
            m_isSpanish ? "Terminos y Condiciones" : "Terms & Conditions"
        );

        if (auto spr = dynamic_cast<ButtonSprite*>(m_langBtn->getNormalImage())) {
            spr->setString(m_isSpanish ? "EN" : "ES");
        }

        buildMDArea();
    }

public:
    static TermsScrollPopup* create() {
        auto ret = new TermsScrollPopup();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

 
class RegisterPopup : public Popup {
protected:
    bool m_acceptedTerms = false;
    bool m_isGDPS = false;

    CCMenuItemToggler* m_termsToggle = nullptr;
    CCMenuItemToggler* m_gdpsToggle = nullptr;
    CCMenuItemSpriteExtra* m_createBtn = nullptr;

    bool init() override {
        if (!Popup::init(310.f, 280.f, "geode.loader/GE_square03.png")) {
            return false;
        }

        auto winSize = m_mainLayer->getContentSize();
        float cx = winSize.width / 2.f;
        float cy = winSize.height / 2.f;

       
        auto logo = CCSprite::create("logo.png"_spr);
        if (logo) {
            float maxLogoW = 200.f;
            float scale = maxLogoW / logo->getContentSize().width;
            if (scale > 0.85f) scale = 0.85f;
            logo->setScale(scale);
            logo->setPosition({ cx, winSize.height - 42.f });
            m_mainLayer->addChild(logo, 1);
        }

 
        auto subLabel = CCLabelBMFont::create(
            "Create your profile and start\ntracking your daily streak!",
            "chatFont.fnt"
        );
        subLabel->setAlignment(kCCTextAlignmentCenter);
        subLabel->setScale(0.62f);
        subLabel->setPosition({ cx, cy + 50.f });
        m_mainLayer->addChild(subLabel);

       
        auto separator = CCLayerColor::create(
            { 255, 255, 255, 40 },
            winSize.width - 40.f,
            1.f
        );
        separator->setPosition({ 20.f, cy + 28.f });
        m_mainLayer->addChild(separator);   
        buildGDPSRow(cx, cy + 8.f);
        buildAcceptRow(cx, cy - 22.f);

        
        auto createBtnSpr = ButtonSprite::create(
            "Create Profile", 0, 0, "goldFont.fnt", "GJ_button_01.png", 0, 1.0f
        );
        m_createBtn = CCMenuItemSpriteExtra::create(
            createBtnSpr,
            this,
            menu_selector(RegisterPopup::onCreateProfile)
        );

        auto createMenu = CCMenu::createWithItem(m_createBtn);
        createMenu->setPosition({ cx, cy - 68.f });
        m_mainLayer->addChild(createMenu);

        refreshCreateButton();
        return true;
    }
 
    void buildGDPSRow(float cx, float y) {
        auto menu = CCMenu::create();
        menu->setPosition({ cx, y });
        m_mainLayer->addChild(menu);

        m_gdpsToggle = CCMenuItemToggler::createWithStandardSprites(
            this,
            menu_selector(RegisterPopup::onToggleGDPS),
            0.7f
        );
        m_gdpsToggle->setPosition({ -105.f, 0.f });
        menu->addChild(m_gdpsToggle);

        auto label = CCLabelBMFont::create("I'm a GDPS player", "bigFont.fnt");
        label->setScale(0.43f);
        label->setAnchorPoint({ 0.f, 0.5f });
        label->setPosition({ -90.f, 0.f });
        menu->addChild(label);

        auto infoSpr = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
        infoSpr->setScale(0.6f);
        auto infoBtn = CCMenuItemSpriteExtra::create(
            infoSpr,
            this,
            menu_selector(RegisterPopup::onGDPSInfo)
        );
        infoBtn->setPosition({ 88.f, 0.f });
        menu->addChild(infoBtn);
    }

    void buildAcceptRow(float cx, float y) {
        auto menu = CCMenu::create();
        menu->setPosition({ cx, y });
        m_mainLayer->addChild(menu);

        m_termsToggle = CCMenuItemToggler::createWithStandardSprites(
            this,
            menu_selector(RegisterPopup::onToggleTerms),
            0.7f
        );
        m_termsToggle->setPosition({ -105.f, 0.f });
        menu->addChild(m_termsToggle);

        auto acceptLabel = CCLabelBMFont::create(
            "I have read and accept\nthe Terms & Conditions",
            "bigFont.fnt"
        );
        acceptLabel->setAlignment(kCCTextAlignmentLeft);
        acceptLabel->setScale(0.38f);
        acceptLabel->setAnchorPoint({ 0.f, 0.5f });
        acceptLabel->setPosition({ -90.f, 0.f });
        menu->addChild(acceptLabel);

        auto tcInfoSpr = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
        tcInfoSpr->setScale(0.6f);
        auto tcInfoBtn = CCMenuItemSpriteExtra::create(
            tcInfoSpr,
            this,
            menu_selector(RegisterPopup::onOpenTerms)
        );
        tcInfoBtn->setPosition({ 88.f, 0.f });
        menu->addChild(tcInfoBtn);
    }

 

    void refreshCreateButton() {
        if (!m_createBtn) {
            return;
        }
        m_createBtn->setOpacity(m_acceptedTerms ? 255 : 110);
    }

    void onToggleTerms(CCObject*) {
        m_acceptedTerms = !m_acceptedTerms;
        refreshCreateButton();
    }

    void onToggleGDPS(CCObject*) {
        m_isGDPS = !m_isGDPS;
    }

    void onGDPSInfo(CCObject*) {
        FLAlertLayer::create(
            "GDPS Player",
            "Mark this if you play on a <cy>private server</c> (GDPS).\n\n"
            "This <cg>will NOT get you banned</c>.\n"
            "It simply labels your account so we\n"
            "can handle it correctly on our side.",
            "OK"
        )->show();
    }

    void onOpenTerms(CCObject*) {
        TermsScrollPopup::create()->show();
    }

    void onCreateProfile(CCObject*) {
        if (!m_acceptedTerms) {
            FLAlertLayer::create(
                "Hold on!",
                "Please read and accept the\n<cy>Terms & Conditions</c>\nbefore creating your profile.",
                "OK"
            )->show();
            return;
        }

        auto am = GJAccountManager::sharedState();
        if (!am || am->m_accountID == 0) {
            FLAlertLayer::create(
                "Error",
                "Invalid account data.\nPlease log in to GD again.",
                "OK"
            )->show();
            return;
        }

        g_streakData.resetToDefault();
        g_streakData.needsRegistration = false;
        g_streakData.isGDPS = m_isGDPS;
        geode::Mod::get()->setSavedValue<bool>("is_gdps_player", m_isGDPS);

        g_streakData.dailyUpdate();
        updatePlayerDataInFirebase();

        this->onClose(nullptr);
    }

public:
    static RegisterPopup* create() {
        auto ret = new RegisterPopup();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};