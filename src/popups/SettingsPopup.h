#pragma once
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/ScrollLayer.hpp>
#include <Geode/utils/cocos.hpp>
#include <Geode/utils/web.hpp> 
#include "../StreakData.h"
#include "HistoryPopup.h" 
#include <Geode/ui/TextInput.hpp>


using namespace geode::prelude;

class SettingsPopup : public Popup<> {
protected:
    ScrollLayer* m_scrollLayer = nullptr;
    float m_listWidth = 230.0f;
    CCNode* m_posContainer = nullptr;
    TextInput* m_inputX = nullptr;
    TextInput* m_inputY = nullptr;
    TextInput* m_inputScale = nullptr;
 

    std::vector<std::string> m_listOptions = { "10", "50" };
    std::vector<std::string> m_pauseModes = { "On", "Off" };

    CCNode* createBaseCell(float height = 40.f) { 
        auto cell = CCNode::create();
        cell->setContentSize({ m_listWidth, height });

        auto cellBg = CCScale9Sprite::create("geode.loader/GE_square03.png");
        cellBg->setContentSize({ m_listWidth, height });
        cellBg->setOpacity(75);
        cellBg->setPosition(cell->getContentSize() / 2);

        cell->addChild(cellBg);
        return cell;
    }

    void addInfoButton(CCNode* parent, const std::string& text, float startX) {
        auto menu = CCMenu::create();
        menu->setPosition(0, 0);
        parent->addChild(menu);

        auto infoSpr = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
        infoSpr->setScale(0.35f);

        auto infoBtn = CCMenuItemSpriteExtra::create(
            infoSpr,
            this,
            menu_selector(SettingsPopup::onInfo)
        );
        infoBtn->setUserObject(CCString::create(text));
        infoBtn->setPosition({ startX + 12.f, 21.f });

        menu->addChild(infoBtn);
    }

    void addButtonSetting(const std::string& name, const std::string& btnText, SEL_MenuHandler callback, const std::string& infoText) {
        auto cell = createBaseCell();
        auto menu = CCMenu::create();
        menu->setPosition(0, 0);
        cell->addChild(menu);

        auto nameLabel = CCLabelBMFont::create(name.c_str(), "goldFont.fnt");
        nameLabel->setAnchorPoint({ 0.0f, 0.5f });
        nameLabel->setPosition({ 15.f, 20.f });
        nameLabel->setScale(0.5f);
        cell->addChild(nameLabel);

        addInfoButton(
            cell,
            infoText,
            15.f + nameLabel->getScaledContentSize().width
        );

        auto btnSpr = ButtonSprite::create(
            btnText.c_str(),
            0,
            0,
            "goldFont.fnt",
            "GJ_button_01.png",
            0,
            0.38f
        );

        auto btn = CCMenuItemSpriteExtra::create(
            btnSpr,
            this,
            callback
        );
        btn->setPosition({ m_listWidth - 35.f, 20.f });
        menu->addChild(btn);

        m_scrollLayer->m_contentLayer->addChild(cell);
    }

    void addImageButtonSetting(const std::string& name,
        const std::string& spriteName,
        SEL_MenuHandler callback,
        const std::string& infoText) {
        auto cell = createBaseCell();
        auto menu = CCMenu::create();
        menu->setPosition(0, 0);
        cell->addChild(menu);

        auto nameLabel = CCLabelBMFont::create(name.c_str(), "goldFont.fnt");
        nameLabel->setAnchorPoint({ 0.0f, 0.5f });
        nameLabel->setPosition({ 15.f, 20.f });
        nameLabel->setScale(0.5f);
        cell->addChild(nameLabel);

        addInfoButton(
            cell,
            infoText,
            15.f + nameLabel->getScaledContentSize().width
        );

        auto spr = CCSprite::create(spriteName.c_str());

        if (!spr) {
            spr = ButtonSprite::create(
                "?",
                0,
                0,
                "goldFont.fnt",
                "GJ_button_01.png",
                0,
                0.6f
            );
        }
        else {
            float maxHeight = 30.0f;
            if (spr->getContentSize().height > maxHeight) {
                spr->setScale(maxHeight / spr->getContentSize().height);
            }
            else {
                spr->setScale(0.8f);
            }
        }

        auto btn = CCMenuItemSpriteExtra::create(
            spr,
            this,
            callback
        );
        btn->setPosition({ m_listWidth - 30.f, 20.f });
        menu->addChild(btn);

        m_scrollLayer->m_contentLayer->addChild(cell);
    }

    void addArrowSetting(const std::string& name,
        const std::string& saveKey,
        const std::string& infoText) {
        auto cell = createBaseCell();
        auto menu = CCMenu::create();
        menu->setPosition(0, 0);
        cell->addChild(menu);

        auto nameLabel = CCLabelBMFont::create(name.c_str(), "goldFont.fnt");
        nameLabel->setAnchorPoint({ 0.0f, 0.5f });
        nameLabel->setPosition({ 15.f, 20.f });
        nameLabel->setScale(0.5f);
        cell->addChild(nameLabel);

        addInfoButton(
            cell,
            infoText,
            15.f + nameLabel->getScaledContentSize().width
        );

        int currentVal = Mod::get()->getSavedValue<int>(saveKey, 0);

        auto valLabel = CCLabelBMFont::create(
            m_listOptions[currentVal].c_str(),
            "bigFont.fnt"
        );
        valLabel->setScale(0.4f);
        valLabel->setColor({ 255, 255, 0 });
        valLabel->setPosition({ m_listWidth - 45.f, 20.f });
        cell->addChild(valLabel);

        auto arrowL = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        arrowL->setScale(0.35f);

        auto btnL = CCMenuItemSpriteExtra::create(
            arrowL,
            this,
            menu_selector(SettingsPopup::onArrowLeft)
        );
        btnL->setPosition({ m_listWidth - 70.f, 20.f });
        btnL->setUserObject(CCString::create(saveKey));
        btnL->setUserData(valLabel);
        menu->addChild(btnL);

        auto arrowR = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        arrowR->setFlipX(true);
        arrowR->setScale(0.35f);

        auto btnR = CCMenuItemSpriteExtra::create(
            arrowR,
            this,
            menu_selector(SettingsPopup::onArrowRight)
        );
        btnR->setPosition({ m_listWidth - 20.f, 20.f });
        btnR->setUserObject(CCString::create(saveKey));
        btnR->setUserData(valLabel);
        menu->addChild(btnR);

        m_scrollLayer->m_contentLayer->addChild(cell);
    }

    void addToggleSetting(const std::string& name, const std::string& saveKey, const std::string& infoText) {
        auto cell = createBaseCell();
        auto menu = CCMenu::create();
        menu->setPosition(0, 0);
        cell->addChild(menu);

        auto nameLabel = CCLabelBMFont::create(name.c_str(), "goldFont.fnt");
        nameLabel->setAnchorPoint({ 0.0f, 0.5f });
        nameLabel->setPosition({ 15.f, 20.f });
        nameLabel->setScale(0.5f);
        cell->addChild(nameLabel);

        addInfoButton(
            cell,
            infoText,
            15.f + nameLabel->getScaledContentSize().width
        );

        bool isEnabled = Mod::get()->getSavedValue<bool>(saveKey, true);
        auto toggle = CCMenuItemToggler::createWithStandardSprites(
            this,
            menu_selector(SettingsPopup::onToggle),
            0.5f
        );
        toggle->setPosition({ m_listWidth - 25.f, 20.f });
        toggle->toggle(isEnabled);
        toggle->setUserObject(CCString::create(saveKey));
        menu->addChild(toggle);

        m_scrollLayer->m_contentLayer->addChild(cell);
    }

    void addVersionSetting(const std::string& name, const std::string& versionText) {
        auto cell = createBaseCell();

        auto nameLabel = CCLabelBMFont::create(name.c_str(), "goldFont.fnt");
        nameLabel->setAnchorPoint({ 0.0f, 0.5f });
        nameLabel->setPosition({ 15.f, 20.f });
        nameLabel->setScale(0.5f);
        cell->addChild(nameLabel);

        auto verLabel = CCLabelBMFont::create(versionText.c_str(), "bigFont.fnt");
        verLabel->setColor({ 0, 255, 100 });
        verLabel->setAnchorPoint({ 1.0f, 0.5f });
        verLabel->setPosition({ m_listWidth - 15.f, 20.f });
        verLabel->setScale(0.35f);
        cell->addChild(verLabel);

        m_scrollLayer->m_contentLayer->addChild(cell);
    }

    void addPauseModeSetting(const std::string& name, const std::string& saveKey, const std::string& infoText) {
        float cellHeight = 90.0f;
        auto cell = createBaseCell(cellHeight);

        auto menu = CCMenu::create();
        menu->setPosition(0, 0);
        cell->addChild(menu);
 
        float topRowY = 65.0f;

        auto nameLabel = CCLabelBMFont::create(name.c_str(), "goldFont.fnt");
        nameLabel->setAnchorPoint({ 0.0f, 0.5f });
        nameLabel->setPosition({ 15.f, topRowY });
        nameLabel->setScale(0.5f);
        cell->addChild(nameLabel);

        auto infoSpr = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
        infoSpr->setScale(0.35f);
        auto infoBtn = CCMenuItemSpriteExtra::create(infoSpr, this, menu_selector(SettingsPopup::onInfo));
        infoBtn->setPosition({ 15.f + nameLabel->getScaledContentSize().width + 12.f, topRowY });
        infoBtn->setUserObject(CCString::create(infoText));
        menu->addChild(infoBtn);

        int currentVal = Mod::get()->getSavedValue<int>(saveKey, 0);
        auto valLabel = CCLabelBMFont::create(m_pauseModes[currentVal].c_str(), "bigFont.fnt");
        valLabel->setScale(0.4f);
        valLabel->setColor({ 255, 255, 0 });
        valLabel->setPosition({ m_listWidth - 45.f, topRowY });
        cell->addChild(valLabel);

        auto arrowL = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        arrowL->setScale(0.35f);
        auto btnL = CCMenuItemSpriteExtra::create(arrowL, this, menu_selector(SettingsPopup::onPauseArrowLeft));
        btnL->setPosition({ m_listWidth - 70.f, topRowY });
        btnL->setUserObject(CCString::create(saveKey));
        btnL->setUserData(valLabel);
        menu->addChild(btnL);

        auto arrowR = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        arrowR->setFlipX(true);
        arrowR->setScale(0.35f);
        auto btnR = CCMenuItemSpriteExtra::create(arrowR, this, menu_selector(SettingsPopup::onPauseArrowRight));
        btnR->setPosition({ m_listWidth - 20.f, topRowY });
        btnR->setUserObject(CCString::create(saveKey));
        btnR->setUserData(valLabel);
        menu->addChild(btnR);

       
        m_posContainer = CCNode::create();
        m_posContainer->setContentSize({ m_listWidth, 40.f });
        m_posContainer->setPosition({ 0, 0 });
        m_posContainer->setVisible(true);
        cell->addChild(m_posContainer);

        float bottomRowY = 25.0f;

         
        float startX = 15.0f;   
        float gap = 55.0f;    
        float inputOff = 22.0f;  
       
        float xPos = startX;
        auto labelX = CCLabelBMFont::create("X:", "chatFont.fnt");
        labelX->setPosition({ xPos, bottomRowY });
        labelX->setScale(0.5f);
        m_posContainer->addChild(labelX);

        m_inputX = TextInput::create(35.f, "0.10", "chatFont.fnt");
        m_inputX->setFilter("0123456789.");
        m_inputX->setPosition({ xPos + inputOff, bottomRowY });
        m_inputX->setScale(0.6f);
        double savedX = Mod::get()->getSavedValue<double>("pause-pos-x", 0.10);
        m_inputX->setString(fmt::format("{:.2f}", savedX));
        m_inputX->setCallback([this](const std::string& val) {
            try { Mod::get()->setSavedValue<double>("pause-pos-x", std::stod(val)); }
            catch (...) {}
            });
        m_posContainer->addChild(m_inputX);

        
        float yPos = startX + gap;  
        auto labelY = CCLabelBMFont::create("Y:", "chatFont.fnt");
        labelY->setPosition({ yPos, bottomRowY });
        labelY->setScale(0.5f);
        m_posContainer->addChild(labelY);

        m_inputY = TextInput::create(35.f, "0.90", "chatFont.fnt");
        m_inputY->setFilter("0123456789.");
        m_inputY->setPosition({ yPos + inputOff, bottomRowY });
        m_inputY->setScale(0.6f);
        double savedY = Mod::get()->getSavedValue<double>("pause-pos-y", 0.90);
        m_inputY->setString(fmt::format("{:.2f}", savedY));
        m_inputY->setCallback([this](const std::string& val) {
            try { Mod::get()->setSavedValue<double>("pause-pos-y", std::stod(val)); }
            catch (...) {}
            });
        m_posContainer->addChild(m_inputY);

         
        float sPos = startX + (gap * 2); 
        auto labelS = CCLabelBMFont::create("S:", "chatFont.fnt");
        labelS->setPosition({ sPos, bottomRowY });
        labelS->setScale(0.5f);
        m_posContainer->addChild(labelS);

        m_inputScale = TextInput::create(35.f, "0.80", "chatFont.fnt");
        m_inputScale->setFilter("0123456789.");
        m_inputScale->setPosition({ sPos + inputOff, bottomRowY });
        m_inputScale->setScale(0.6f);
        double savedScale = Mod::get()->getSavedValue<double>("pause-scale", 0.80);
        m_inputScale->setString(fmt::format("{:.2f}", savedScale));
        m_inputScale->setCallback([this](const std::string& val) {
            try {
                double d = std::stod(val);
                if (d < 0.5) d = 0.5;
                if (d > 10.0) d = 10.0;
                Mod::get()->setSavedValue<double>("pause-scale", d);
            }
            catch (...) {}
            });
        m_posContainer->addChild(m_inputScale);

       
        float resetPos = startX + (gap * 3) + 5.0f;  
        auto resetSpr = CCSprite::createWithSpriteFrameName("GJ_updateBtn_001.png");
        resetSpr->setScale(0.4f);
        auto resetBtn = CCMenuItemSpriteExtra::create(resetSpr, this, menu_selector(SettingsPopup::onResetPausePos));
        resetBtn->setPosition({ resetPos, bottomRowY });

        auto controlsMenu = CCMenu::create();
        controlsMenu->setPosition(0, 0);
        controlsMenu->addChild(resetBtn);
        m_posContainer->addChild(controlsMenu);

        m_scrollLayer->m_contentLayer->addChild(cell);
    }

    void onResetPausePos(CCObject*) {
        double defX = 0.10;
        double defY = 0.90;
        double defScale = 0.80;  

        Mod::get()->setSavedValue<double>("pause-pos-x", defX);
        Mod::get()->setSavedValue<double>("pause-pos-y", defY);
        Mod::get()->setSavedValue<double>("pause-scale", defScale);

        if (m_inputX) m_inputX->setString("0.10");
        if (m_inputY) m_inputY->setString("0.90");
        if (m_inputScale) m_inputScale->setString("0.80"); 

        Notification::create("Layout Reset", NotificationIcon::Success)->show();
    }

    void updatePosVisibility(int mode) {
       
        if (m_posContainer) {
            m_posContainer->setVisible(true);
        }
    }

 

    void onPauseArrowLeft(CCObject* sender) {
        auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
        auto keyStr = static_cast<CCString*>(btn->getUserObject());
        auto label = static_cast<CCLabelBMFont*>(btn->getUserData());
        if (!keyStr) return;

        int current = Mod::get()->getSavedValue<int>(keyStr->getCString(), 0);
        int next = (current - 1 + m_pauseModes.size()) % m_pauseModes.size();
        Mod::get()->setSavedValue<int>(keyStr->getCString(), next);
        label->setString(m_pauseModes[next].c_str());

        
        updatePosVisibility(next);
    }

    void onPauseArrowRight(CCObject* sender) {
        auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
        auto keyStr = static_cast<CCString*>(btn->getUserObject());
        auto label = static_cast<CCLabelBMFont*>(btn->getUserData());
        if (!keyStr) return;

        int current = Mod::get()->getSavedValue<int>(keyStr->getCString(), 0);
        int next = (current + 1) % m_pauseModes.size();
        Mod::get()->setSavedValue<int>(keyStr->getCString(), next);
        label->setString(m_pauseModes[next].c_str());

       
        updatePosVisibility(next);
    }

    bool setup() override {
        this->setTitle("Settings");
        auto winSize = m_mainLayer->getContentSize();

        CCSize scrollSize = { m_listWidth, 150.f };
        m_scrollLayer = ScrollLayer::create(scrollSize);
        m_scrollLayer->setPosition(
            (winSize.width - scrollSize.width) / 2,
            40.f
        );

        auto bg = CCScale9Sprite::create("square02b_001.png");
        bg->setColor({ 0, 0, 0 });
        bg->setOpacity(100);
        bg->setContentSize(scrollSize);
        bg->setPosition(
            winSize.width / 2,
            m_scrollLayer->getPositionY() + scrollSize.height / 2
        );
        m_mainLayer->addChild(bg);

        auto content = m_scrollLayer->m_contentLayer;

        content->setLayout(
            ColumnLayout::create()
            ->setAxisReverse(false)
            ->setGap(5.f)
            ->setAxisAlignment(AxisAlignment::Start)
            ->setAutoGrowAxis(scrollSize.height)
        );

        addButtonSetting(
            "Streak ID",
            "Copy",
            menu_selector(SettingsPopup::onCopyStreakID),
            "Copy your unique Streak ID"
        );



        addImageButtonSetting(
            "History",
            "historial_btn.png"_spr,
            menu_selector(SettingsPopup::onOpenHistory),
            "Check your daily points history"
        );

        addImageButtonSetting(
            "Need Help?",
            "discord_btn.png"_spr,
            menu_selector(SettingsPopup::onJoinDiscord),
            "Join our Discord Server!"
        );

     
        addArrowSetting(
            "Expand Top List",
            "leaderboard_capacity_idx",
            "Show 10 or 50 players"
        );

       
        addPauseModeSetting(
            "Streak counter",
            "pause_hud_mode",
            "Streak counter in the pause menu"
        );

        addToggleSetting(
            "Streak Bar ",
            "enable_streak_bar",
            "Show the Streak progress bar you complete levels."
        );

        addToggleSetting(
            "Welcome Noti",
            "enable_welcome_notif",
            "Toggle the welcome message on startup"
        );

        addToggleSetting(
            "Streak Ani",
            "enable_streak_anim",
            "Disables the Streak animation"
        );
     

        addVersionSetting("Mod Version", "1.10.34-alpha.1");

        content->updateLayout();
        m_mainLayer->addChild(m_scrollLayer);
        m_scrollLayer->scrollToTop();

        return true;
    }

    void updateArrowLabel(CCNode* sender, int newVal) {
        auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
        auto label = static_cast<CCLabelBMFont*>(btn->getUserData());
        if (label) {
            newVal = std::clamp(newVal, 0, (int)m_listOptions.size() - 1);
            label->setString(m_listOptions[newVal].c_str());
        }
    }

    void onArrowLeft(CCObject* sender) {
        auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
        auto keyStr = static_cast<CCString*>(btn->getUserObject());
        if (!keyStr) return;

        int current = Mod::get()->getSavedValue<int>(keyStr->getCString(), 0);
        int next = (current - 1 + m_listOptions.size()) % m_listOptions.size();

        Mod::get()->setSavedValue<int>(keyStr->getCString(), next);
        updateArrowLabel(btn, next);
    }

    void onArrowRight(CCObject* sender) {
        auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
        auto keyStr = static_cast<CCString*>(btn->getUserObject());
        if (!keyStr) return;

        int current = Mod::get()->getSavedValue<int>(keyStr->getCString(), 0);
        int next = (current + 1) % m_listOptions.size();

        Mod::get()->setSavedValue<int>(keyStr->getCString(), next);
        updateArrowLabel(btn, next);
    }

    void onCopyStreakID(CCObject*) {
        utils::clipboard::write(g_streakData.streakID);
        Notification::create(
            "ID Copied to Clipboard",
            NotificationIcon::Success
        )->show();
    }

    void onOpenHistory(CCObject*) {
        HistoryPopup::create()->show();
    }

    void onToggle(CCObject* sender) {
        auto toggleBtn = static_cast<CCMenuItemToggler*>(sender);
        auto keyStr = static_cast<CCString*>(toggleBtn->getUserObject());
        if (keyStr) {
            Mod::get()->setSavedValue<bool>(
                keyStr->getCString(),
                !toggleBtn->isToggled()
            );
        }
    }

    void onJoinDiscord(CCObject*) {
        geode::utils::web::openLinkInBrowser(
            "https://discord.gg/dykf3y6HWw"
        );
    }

    void onInfo(CCObject* sender) {
        auto btn = static_cast<CCNode*>(sender);
        auto text = static_cast<CCString*>(btn->getUserObject());
        FLAlertLayer::create(
            "Info",
            text ? text->getCString() : "Info",
            "OK"
        )->show();
    }

public:
    static SettingsPopup* create() {
        auto ret = new SettingsPopup();
        if (ret && ret->initAnchored(270.f, 230.f, "geode.loader/GE_square03.png")) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};