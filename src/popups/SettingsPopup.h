#pragma once
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/ScrollLayer.hpp>
#include <Geode/utils/cocos.hpp>
#include <Geode/utils/web.hpp> 
#include "../StreakData.h"
#include "HistoryPopup.h" 

using namespace geode::prelude;

class SettingsPopup : public Popup<> {
protected:
    ScrollLayer* m_scrollLayer = nullptr;
    float m_listWidth = 230.0f;

    std::vector<std::string> m_listOptions = { "10", "50" };

    CCNode* createBaseCell() {
        auto cell = CCNode::create();
        cell->setContentSize({ m_listWidth, 40.f });

        auto cellBg = CCScale9Sprite::create("geode.loader/GE_square03.png");
        cellBg->setContentSize({ m_listWidth, 40.f });
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

        addImageButtonSetting(
            "History",
            "historial_btn.png"_spr,
            menu_selector(SettingsPopup::onOpenHistory),
            "Check your daily points history"
        );

        addArrowSetting(
            "Expand Top List",
            "leaderboard_capacity_idx",
            "Show 10 or 50 players"
        );

        addImageButtonSetting(
            "Need Help?",
            "discord_btn.png"_spr,
            menu_selector(SettingsPopup::onJoinDiscord),
            "Join our Discord Server!"
        );

        addButtonSetting(
            "Streak ID",
            "Copy",
            menu_selector(SettingsPopup::onCopyStreakID),
            "Copy your unique Streak ID"
        );

        addVersionSetting("Mod Version", "1.10.30-beta1");

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