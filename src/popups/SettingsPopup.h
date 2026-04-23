#pragma once
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/ScrollLayer.hpp>
#include <Geode/utils/cocos.hpp>
#include <Geode/utils/web.hpp> 
#include "../StreakData.h"
#include "HistoryPopup.h" 
#include <Geode/ui/TextInput.hpp>
#include "RegisterPopup.h"

using namespace geode::prelude;


class CreditsPopup : public Popup {
protected:
    bool init() override {
        if (!Popup::init(260.f, 200.f, "geode.loader/GE_square03.png")) return false;

        this->setTitle("Credits");
        auto winSize = m_mainLayer->getContentSize();
        CCPoint center = winSize / 2;


        auto nameLabel = CCLabelBMFont::create("XJotaBeLikeX", "goldFont.fnt");
        nameLabel->setPosition({ center.x, center.y + 45.f });
        nameLabel->setScale(0.8f);
        m_mainLayer->addChild(nameLabel);


        auto roleLabel = CCLabelBMFont::create("Creator & Developer", "chatFont.fnt");
        roleLabel->setPosition({ center.x, center.y + 25.f });
        roleLabel->setScale(0.5f);
        roleLabel->setColor({ 200, 200, 200 });
        m_mainLayer->addChild(roleLabel);


        auto cubeSpr = CCSprite::create("jotabelike_cube.png"_spr);
        if (!cubeSpr) cubeSpr = CCSprite::create("jotabelike_cube.png");

        if (cubeSpr) {
            cubeSpr->setPosition({ center.x, center.y - 5.f });
            if (cubeSpr->getContentSize().height > 50.f) {
                cubeSpr->setScale(50.f / cubeSpr->getContentSize().height);
            }
            m_mainLayer->addChild(cubeSpr);
        }


        auto separator = CCSprite::createWithSpriteFrameName("edit_vLine_001.png");
        if (separator) {
            separator->setPosition({ center.x, center.y - 22.f });
            separator->setRotation(90.f);
            separator->setScaleY(0.7f);
            separator->setOpacity(150);
            m_mainLayer->addChild(separator);
        }


        auto socialsMenu = CCMenu::create();
        socialsMenu->setContentSize({ 200.f, 40.f });
        socialsMenu->setPosition({ center.x, center.y - 65.f });



        auto xSpr = CCSprite::createWithSpriteFrameName("gj_twIcon_001.png");
        if (xSpr) {
            xSpr->setScale(0.8f);
            auto xBtn = CCMenuItemSpriteExtra::create(xSpr, this, menu_selector(CreditsPopup::onTwitter));
            socialsMenu->addChild(xBtn);
        }


        auto ytSpr = CCSprite::createWithSpriteFrameName("gj_ytIcon_001.png");
        if (ytSpr) {
            ytSpr->setScale(0.8f);
            auto ytBtn = CCMenuItemSpriteExtra::create(ytSpr, this, menu_selector(CreditsPopup::onYouTube));
            socialsMenu->addChild(ytBtn);
        }


        auto instaSpr = CCSprite::createWithSpriteFrameName("gj_instaIcon_001.png");
        if (!instaSpr) instaSpr = CCSprite::create("gj_instaIcon_001.png"_spr);
        if (instaSpr) {
            instaSpr->setScale(0.8f);
            auto instaBtn = CCMenuItemSpriteExtra::create(instaSpr, this, menu_selector(CreditsPopup::onInstagram));
            socialsMenu->addChild(instaBtn);
        }


        auto tiktokSpr = CCSprite::createWithSpriteFrameName("gj_tiktokIcon_001.png");
        if (!tiktokSpr) tiktokSpr = CCSprite::create("gj_tiktokIcon_001.png"_spr);
        if (tiktokSpr) {
            tiktokSpr->setScale(0.8f);
            auto tiktokBtn = CCMenuItemSpriteExtra::create(tiktokSpr, this, menu_selector(CreditsPopup::onTikTok));
            socialsMenu->addChild(tiktokBtn);
        }


        socialsMenu->setLayout(RowLayout::create()->setGap(15.f));
        m_mainLayer->addChild(socialsMenu);

        return true;
    }


    void onTwitter(CCObject*) {
        geode::utils::web::openLinkInBrowser("https://x.com/Jotabelike");
    }

    void onYouTube(CCObject*) {
        geode::utils::web::openLinkInBrowser("https://www.youtube.com/@jotabelike");
    }

    void onInstagram(CCObject*) {
        geode::utils::web::openLinkInBrowser("https://www.instagram.com/josue_david0024");
    }

    void onTikTok(CCObject*) {
        geode::utils::web::openLinkInBrowser("https://www.tiktok.com/@xjotabelikex");
    }

public:
    static CreditsPopup* create() {
        auto ret = new CreditsPopup();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};


class SettingsPopup : public Popup {
protected:
    ScrollLayer* m_scrollLayer = nullptr;
    float m_listWidth = 230.0f;
    CCNode* m_posContainer = nullptr;
    Slider* m_sliderX = nullptr;
    Slider* m_sliderY = nullptr;
    Slider* m_sliderScale = nullptr;
    CCLabelBMFont* m_sliderXLabel = nullptr;
    CCLabelBMFont* m_sliderYLabel = nullptr;
    CCLabelBMFont* m_sliderScaleLabel = nullptr;
    Slider* m_opacitySlider = nullptr;
    CCLabelBMFont* m_opacityLabel = nullptr;

    std::vector<std::string> m_listOptions = { "10", "50" };
    std::vector<std::string> m_pauseModes = { "On", "Off" };
    std::vector<std::string> m_animModes = { "None", "Bar", "Label" };

    CCNode* createBaseCell(float height = 40.f) {
        auto cell = CCNode::create();
        cell->setContentSize({ m_listWidth, height });

        auto cellBg = CCLayerColor::create({ 0, 0, 0, 80 }, m_listWidth, height);
        cellBg->ignoreAnchorPointForPosition(false);
        cellBg->setAnchorPoint({ 0.5f, 0.5f });
        cellBg->setPosition(cell->getContentSize() / 2);
        cell->addChild(cellBg);

        auto separator = CCLayerColor::create({ 255, 255, 255, 20 }, m_listWidth, 1.f);
        separator->ignoreAnchorPointForPosition(false);
        separator->setAnchorPoint({ 0.5f, 0.f });
        separator->setPosition({ m_listWidth / 2.f, 0.f });
        cell->addChild(separator);

        return cell;
    }

    void addInfoButton(CCNode* parent, const std::string& text, float startX, float posY = 20.f) {
        auto menu = CCMenu::create();
        menu->setPosition(0, 0);
        parent->addChild(menu);

        auto infoSpr = CCSprite::create("info_btn.png"_spr);
        infoSpr->setScale(0.15f);

        auto infoBtn = CCMenuItemSpriteExtra::create(
            infoSpr,
            this,
            menu_selector(SettingsPopup::onInfo)
        );
        infoBtn->setUserObject(CCString::create(text));

      
        infoBtn->setPosition({ startX + 12.f, posY });

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

        addInfoButton(cell, infoText, 15.f + nameLabel->getScaledContentSize().width);

        auto btnSpr = ButtonSprite::create(btnText.c_str(), 0, 0, "goldFont.fnt", "GJ_button_01.png", 0, 0.38f);
        auto btn = CCMenuItemSpriteExtra::create(btnSpr, this, callback);
        btn->setPosition({ m_listWidth - 35.f, 20.f });
        menu->addChild(btn);

        m_scrollLayer->m_contentLayer->addChild(cell);
    }

    void addImageButtonSetting(const std::string& name, const std::string& spriteName, SEL_MenuHandler callback, const std::string& infoText) {
        auto cell = createBaseCell();
        auto menu = CCMenu::create();
        menu->setPosition(0, 0);
        cell->addChild(menu);

        auto nameLabel = CCLabelBMFont::create(name.c_str(), "goldFont.fnt");
        nameLabel->setAnchorPoint({ 0.0f, 0.5f });
        nameLabel->setPosition({ 15.f, 20.f });
        nameLabel->setScale(0.5f);
        cell->addChild(nameLabel);

        addInfoButton(cell, infoText, 15.f + nameLabel->getScaledContentSize().width);

        auto spr = CCSprite::create(spriteName.c_str());
        if (!spr) {
            spr = ButtonSprite::create("?", 0, 0, "goldFont.fnt", "GJ_button_01.png", 0, 0.6f);
        }
        else {
            float maxHeight = 30.0f;
            if (spr->getContentSize().height > maxHeight) spr->setScale(maxHeight / spr->getContentSize().height);
            else spr->setScale(0.8f);
        }

        auto btn = CCMenuItemSpriteExtra::create(spr, this, callback);
        btn->setPosition({ m_listWidth - 30.f, 20.f });
        menu->addChild(btn);

        m_scrollLayer->m_contentLayer->addChild(cell);
    }

    void addArrowSetting(const std::string& name, const std::string& saveKey, const std::string& infoText) {
        auto cell = createBaseCell();
        auto menu = CCMenu::create();
        menu->setPosition(0, 0);
        cell->addChild(menu);

        auto nameLabel = CCLabelBMFont::create(name.c_str(), "goldFont.fnt");
        nameLabel->setAnchorPoint({ 0.0f, 0.5f });
        nameLabel->setPosition({ 15.f, 20.f });
        nameLabel->setScale(0.5f);
        cell->addChild(nameLabel);

        addInfoButton(cell, infoText, 15.f + nameLabel->getScaledContentSize().width);

        int currentVal = Mod::get()->getSavedValue<int>(saveKey, 0);
        auto valLabel = CCLabelBMFont::create(m_listOptions[currentVal].c_str(), "bigFont.fnt");
        valLabel->setScale(0.4f);
        valLabel->setColor({ 255, 255, 0 });
        valLabel->setPosition({ m_listWidth - 45.f, 20.f });
        cell->addChild(valLabel);

        auto arrowL = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        arrowL->setScale(0.35f);
        auto btnL = CCMenuItemSpriteExtra::create(arrowL, this, menu_selector(SettingsPopup::onArrowLeft));
        btnL->setPosition({ m_listWidth - 70.f, 20.f });
        btnL->setUserObject(CCString::create(saveKey));
        btnL->setUserData(valLabel);
        menu->addChild(btnL);

        auto arrowR = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        arrowR->setFlipX(true);
        arrowR->setScale(0.35f);
        auto btnR = CCMenuItemSpriteExtra::create(arrowR, this, menu_selector(SettingsPopup::onArrowRight));
        btnR->setPosition({ m_listWidth - 20.f, 20.f });
        btnR->setUserObject(CCString::create(saveKey));
        btnR->setUserData(valLabel);
        menu->addChild(btnR);

        m_scrollLayer->m_contentLayer->addChild(cell);
    }

    void addAnimModeSetting(const std::string& name, const std::string& saveKey, const std::string& infoText) {
        auto cell = createBaseCell();
        auto menu = CCMenu::create();
        menu->setPosition(0, 0);
        cell->addChild(menu);

        auto nameLabel = CCLabelBMFont::create(name.c_str(), "goldFont.fnt");
        nameLabel->setAnchorPoint({ 0.0f, 0.5f });
        nameLabel->setPosition({ 15.f, 20.f });
        nameLabel->setScale(0.5f);
        cell->addChild(nameLabel);

        addInfoButton(cell, infoText, 15.f + nameLabel->getScaledContentSize().width);

        int currentVal = Mod::get()->getSavedValue<int>(saveKey, 2); 
        auto valLabel = CCLabelBMFont::create(m_animModes[currentVal].c_str(), "bigFont.fnt");
        valLabel->setScale(0.4f);
        valLabel->setColor({ 255, 255, 0 });
        valLabel->setPosition({ m_listWidth - 45.f, 20.f });
        cell->addChild(valLabel);

        auto arrowL = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        arrowL->setScale(0.35f);
        auto btnL = CCMenuItemSpriteExtra::create(arrowL, this, menu_selector(SettingsPopup::onAnimArrowLeft));
        btnL->setPosition({ m_listWidth - 70.f, 20.f });
        btnL->setUserObject(CCString::create(saveKey));
        btnL->setUserData(valLabel);
        menu->addChild(btnL);

        auto arrowR = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        arrowR->setFlipX(true);
        arrowR->setScale(0.35f);
        auto btnR = CCMenuItemSpriteExtra::create(arrowR, this, menu_selector(SettingsPopup::onAnimArrowRight));
        btnR->setPosition({ m_listWidth - 20.f, 20.f });
        btnR->setUserObject(CCString::create(saveKey));
        btnR->setUserData(valLabel);
        menu->addChild(btnR);

        m_scrollLayer->m_contentLayer->addChild(cell);
    }

    void onAnimArrowLeft(CCObject* sender) {
        auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
        auto keyStr = static_cast<CCString*>(btn->getUserObject());
        auto label = static_cast<CCLabelBMFont*>(btn->getUserData());
        if (!keyStr) return;

        int current = Mod::get()->getSavedValue<int>(keyStr->getCString(), 2);
        int next = (current - 1 + m_animModes.size()) % m_animModes.size();
        Mod::get()->setSavedValue<int>(keyStr->getCString(), next);
        label->setString(m_animModes[next].c_str());
    }

    void onAnimArrowRight(CCObject* sender) {
        auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
        auto keyStr = static_cast<CCString*>(btn->getUserObject());
        auto label = static_cast<CCLabelBMFont*>(btn->getUserData());
        if (!keyStr) return;

        int current = Mod::get()->getSavedValue<int>(keyStr->getCString(), 2);
        int next = (current + 1) % m_animModes.size();
        Mod::get()->setSavedValue<int>(keyStr->getCString(), next);
        label->setString(m_animModes[next].c_str());
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

        addInfoButton(cell, infoText, 15.f + nameLabel->getScaledContentSize().width);

        bool isEnabled = Mod::get()->getSavedValue<bool>(saveKey, true);
        auto toggle = CCMenuItemToggler::createWithStandardSprites(this, menu_selector(SettingsPopup::onToggle), 0.5f);
        toggle->setPosition({ m_listWidth - 25.f, 20.f });
        toggle->toggle(isEnabled);
        toggle->setUserObject(CCString::create(saveKey));
        menu->addChild(toggle);

        m_scrollLayer->m_contentLayer->addChild(cell);
    }

    void addBannerOpacitySetting(const std::string& name, const std::string& saveKey, const std::string& infoText) {
        float cellHeight = 70.0f;
        auto cell = createBaseCell(cellHeight);

       
        float topY = cellHeight - 20.0f;
        auto nameLabel = CCLabelBMFont::create(name.c_str(), "goldFont.fnt");
        nameLabel->setAnchorPoint({ 0.0f, 0.5f });
        nameLabel->setPosition({ 15.f, topY });
        nameLabel->setScale(0.5f);
        cell->addChild(nameLabel); 
        addInfoButton(cell, infoText, 15.f + nameLabel->getScaledContentSize().width, topY);
 
        float bottomY = 22.0f;
        double savedOpacity = Mod::get()->getSavedValue<double>(saveKey, 1.0);

        m_opacitySlider = Slider::create(this, menu_selector(SettingsPopup::onBannerOpacitySlider), 0.6f);
        m_opacitySlider->setPosition({ 85.f, bottomY });  
        m_opacitySlider->setValue(static_cast<float>(savedOpacity));
        m_opacitySlider->setBarVisibility(true);
        cell->addChild(m_opacitySlider);


        m_opacityLabel = CCLabelBMFont::create(fmt::format("{}%", static_cast<int>(savedOpacity * 100)).c_str(), "bigFont.fnt");
        m_opacityLabel->setAnchorPoint({ 0.0f, 0.5f });
        m_opacityLabel->setPosition({ 155.f, bottomY });  
        m_opacityLabel->setScale(0.35f);
        m_opacityLabel->setColor({ 255, 255, 0 });
        cell->addChild(m_opacityLabel);
        m_scrollLayer->m_contentLayer->addChild(cell);
    }

   
    void onBannerOpacitySlider(CCObject* sender) {
        auto slider = static_cast<SliderThumb*>(sender);
        float val = slider->getValue();
        double clamped = std::clamp(static_cast<double>(val), 0.0, 1.0);
        Mod::get()->setSavedValue<double>("banner_opacity", clamped);
        if (m_opacityLabel) m_opacityLabel->setString(fmt::format("{}%", static_cast<int>(clamped * 100)).c_str());
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
        float cellHeight = 130.0f;
        auto cell = createBaseCell(cellHeight);

        auto menu = CCMenu::create();
        menu->setPosition(0, 0);
        cell->addChild(menu);

        float topRowY = cellHeight - 25.0f;

        auto nameLabel = CCLabelBMFont::create(name.c_str(), "goldFont.fnt");
        nameLabel->setAnchorPoint({ 0.0f, 0.5f });
        nameLabel->setPosition({ 15.f, topRowY });
        nameLabel->setScale(0.5f);
        cell->addChild(nameLabel);

        auto infoSpr = CCSprite::create("info_btn.png"_spr);
        infoSpr->setScale(0.15f);
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
        m_posContainer->setContentSize({ m_listWidth, 80.f });
        m_posContainer->setPosition({ 0, 0 });
        m_posContainer->setVisible(true);
        cell->addChild(m_posContainer);

        float sliderWidth = 90.f;
        float sliderLeftX = 40.f;
        float sliderRightX = sliderLeftX + sliderWidth;

        float row1Y = 65.f;
        auto labelX = CCLabelBMFont::create("X:", "goldFont.fnt");
        labelX->setAnchorPoint({ 1.0f, 0.5f });
        labelX->setPosition({ sliderLeftX - 20.f, row1Y });
        labelX->setScale(0.5f);
        m_posContainer->addChild(labelX);

        double savedX = Mod::get()->getSavedValue<double>("pause-pos-x", 0.10);
        m_sliderX = Slider::create(this, menu_selector(SettingsPopup::onSliderX), 0.6f);
        m_sliderX->setPosition({ sliderLeftX + sliderWidth / 2.f, row1Y });
        m_sliderX->setValue(static_cast<float>(savedX));
        m_sliderX->setBarVisibility(true);
        m_posContainer->addChild(m_sliderX);

        m_sliderXLabel = CCLabelBMFont::create(fmt::format("{:.2f}", savedX).c_str(), "bigFont.fnt");
        m_sliderXLabel->setAnchorPoint({ 0.0f, 0.5f });
        m_sliderXLabel->setPosition({ sliderRightX + 20.f, row1Y });
        m_sliderXLabel->setScale(0.35f);
        m_sliderXLabel->setColor({ 255, 255, 0 });
        m_posContainer->addChild(m_sliderXLabel);

        float row2Y = 40.f;
        auto labelY = CCLabelBMFont::create("Y:", "goldFont.fnt");
        labelY->setAnchorPoint({ 1.0f, 0.5f });
        labelY->setPosition({ sliderLeftX - 20.f, row2Y });
        labelY->setScale(0.5f);
        m_posContainer->addChild(labelY);

        double savedY = Mod::get()->getSavedValue<double>("pause-pos-y", 0.90);
        m_sliderY = Slider::create(this, menu_selector(SettingsPopup::onSliderY), 0.6f);
        m_sliderY->setPosition({ sliderLeftX + sliderWidth / 2.f, row2Y });
        m_sliderY->setValue(static_cast<float>(savedY));
        m_sliderY->setBarVisibility(true);
        m_posContainer->addChild(m_sliderY);

        m_sliderYLabel = CCLabelBMFont::create(fmt::format("{:.2f}", savedY).c_str(), "bigFont.fnt");
        m_sliderYLabel->setAnchorPoint({ 0.0f, 0.5f });
        m_sliderYLabel->setPosition({ sliderRightX + 20.f, row2Y });
        m_sliderYLabel->setScale(0.35f);
        m_sliderYLabel->setColor({ 255, 255, 0 });
        m_posContainer->addChild(m_sliderYLabel);

        float row3Y = 15.f;
        auto labelS = CCLabelBMFont::create("S:", "goldFont.fnt");
        labelS->setAnchorPoint({ 1.0f, 0.5f });
        labelS->setPosition({ sliderLeftX - 20.f, row3Y });
        labelS->setScale(0.5f);
        m_posContainer->addChild(labelS);

        double savedScale = Mod::get()->getSavedValue<double>("pause-scale", 0.80);
        m_sliderScale = Slider::create(this, menu_selector(SettingsPopup::onSliderScale), 0.6f);
        m_sliderScale->setPosition({ sliderLeftX + sliderWidth / 2.f, row3Y });
        m_sliderScale->setValue(static_cast<float>((savedScale - 0.5) / 9.5));
        m_sliderScale->setBarVisibility(true);
        m_posContainer->addChild(m_sliderScale);

        m_sliderScaleLabel = CCLabelBMFont::create(fmt::format("{:.2f}", savedScale).c_str(), "bigFont.fnt");
        m_sliderScaleLabel->setAnchorPoint({ 0.0f, 0.5f });
        m_sliderScaleLabel->setPosition({ sliderRightX + 20.f, row3Y });
        m_sliderScaleLabel->setScale(0.35f);
        m_sliderScaleLabel->setColor({ 255, 255, 0 });
        m_posContainer->addChild(m_sliderScaleLabel);

        auto resetSpr = CCSprite::createWithSpriteFrameName("GJ_updateBtn_001.png");
        resetSpr->setScale(0.4f);
        auto resetBtn = CCMenuItemSpriteExtra::create(resetSpr, this, menu_selector(SettingsPopup::onResetPausePos));
        resetBtn->setPosition({ m_listWidth - 20.f, row3Y });

        auto controlsMenu = CCMenu::create();
        controlsMenu->setPosition(0, 0);
        controlsMenu->addChild(resetBtn);
        m_posContainer->addChild(controlsMenu);

        m_scrollLayer->m_contentLayer->addChild(cell);
    }

    void onSliderX(CCObject* sender) {
        auto slider = static_cast<SliderThumb*>(sender);
        float val = slider->getValue();
        double clamped = std::clamp(static_cast<double>(val), 0.0, 1.0);
        Mod::get()->setSavedValue<double>("pause-pos-x", clamped);
        if (m_sliderXLabel) m_sliderXLabel->setString(fmt::format("{:.2f}", clamped).c_str());
    }

    void onSliderY(CCObject* sender) {
        auto slider = static_cast<SliderThumb*>(sender);
        float val = slider->getValue();
        double clamped = std::clamp(static_cast<double>(val), 0.0, 1.0);
        Mod::get()->setSavedValue<double>("pause-pos-y", clamped);
        if (m_sliderYLabel) m_sliderYLabel->setString(fmt::format("{:.2f}", clamped).c_str());
    }

    void onSliderScale(CCObject* sender) {
        auto slider = static_cast<SliderThumb*>(sender);
        float val = slider->getValue();
        double mapped = 0.5 + static_cast<double>(val) * 9.5;
        mapped = std::clamp(mapped, 0.5, 10.0);
        Mod::get()->setSavedValue<double>("pause-scale", mapped);
        if (m_sliderScaleLabel) m_sliderScaleLabel->setString(fmt::format("{:.2f}", mapped).c_str());
    }

    void onResetPausePos(CCObject*) {
        double defX = 0.10;
        double defY = 0.90;
        double defScale = 0.80;

        Mod::get()->setSavedValue<double>("pause-pos-x", defX);
        Mod::get()->setSavedValue<double>("pause-pos-y", defY);
        Mod::get()->setSavedValue<double>("pause-scale", defScale);

        if (m_sliderX) m_sliderX->setValue(static_cast<float>(defX));
        if (m_sliderY) m_sliderY->setValue(static_cast<float>(defY));
        if (m_sliderScale) m_sliderScale->setValue(static_cast<float>((defScale - 0.5) / 9.5));

        if (m_sliderXLabel) m_sliderXLabel->setString("0.10");
        if (m_sliderYLabel) m_sliderYLabel->setString("0.90");
        if (m_sliderScaleLabel) m_sliderScaleLabel->setString("0.80");

        Notification::create("Layout Reset", NotificationIcon::Success)->show();
    }

    void updatePosVisibility(int mode) {
        if (m_posContainer) m_posContainer->setVisible(true);
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

    bool init() {
        if (!Popup::init(270.f, 230.f, "geode.loader/GE_square03.png")) return false;

        this->setTitle("Settings");
        auto winSize = m_mainLayer->getContentSize();

        CCSize scrollSize = { m_listWidth, 150.f };
        float scrollX = (winSize.width - scrollSize.width) / 2;
        float scrollY = 40.f;

        auto bg = CCScale9Sprite::create("square02b_001.png");
        bg->setColor({ 0, 0, 0 });
        bg->setOpacity(100);
        bg->setContentSize(scrollSize);
        bg->setPosition(winSize.width / 2, scrollY + scrollSize.height / 2);
        m_mainLayer->addChild(bg);

        auto stencil = CCScale9Sprite::create("square02b_001.png");
        stencil->setContentSize(scrollSize);
        stencil->setAnchorPoint({ 0.f, 0.f });

        auto clipNode = CCClippingNode::create(stencil);
        clipNode->setAlphaThreshold(0.05f);
        clipNode->setContentSize(scrollSize);
        clipNode->setPosition({ scrollX, scrollY });
        m_mainLayer->addChild(clipNode);

        m_scrollLayer = ScrollLayer::create(scrollSize);
        m_scrollLayer->setPosition(0.f, 0.f);
        clipNode->addChild(m_scrollLayer);

        auto content = m_scrollLayer->m_contentLayer;
        content->setLayout(
            ColumnLayout::create()
            ->setAxisReverse(false)
            ->setGap(0.f)
            ->setAxisAlignment(AxisAlignment::Start)
            ->setAutoGrowAxis(scrollSize.height)
        );
        addButtonSetting("Terms & Conditions", "T&C", menu_selector(SettingsPopup::onOpenTerms), "Read the Terms and Conditions of the mod.");
        addButtonSetting("Streak ID", "Copy", menu_selector(SettingsPopup::onCopyStreakID), "Copy your unique Streak ID");
        addImageButtonSetting("History", "historial_btn.png"_spr, menu_selector(SettingsPopup::onOpenHistory), "Check your daily points history");
        addImageButtonSetting("Need Help?", "discord_btn.png"_spr, menu_selector(SettingsPopup::onJoinDiscord), "Join our Discord Server!");
        addArrowSetting("Expand Top List", "leaderboard_capacity_idx", "Show 10 or 50 players");
        addPauseModeSetting("Streak counter", "pause_hud_mode", "Streak counter in the pause menu");
        addAnimModeSetting("End Animation", "end_level_anim_mode", "Choose the streak animation at the end of a level.");
        addToggleSetting("Welcome Notification", "enable_welcome_notif", "Toggle the welcome message on startup");
        addToggleSetting("Streak Animation", "enable_streak_anim", "Disables the Streak animation");
        addToggleSetting("Level Points","enable_level_points","Show how many Streak points a level gives.");
        addToggleSetting("Banners in Comments", "enable_comment_banners", "Show player banners in level comments.");
        addBannerOpacitySetting("Banner Opacity", "banner_opacity", "Adjust the transparency of the banners in comments.");
        addToggleSetting("Name Effects", "enable_name_effects", "Show custom name effects and colors in comments.");
        addVersionSetting("Mod Version", fmt::format("{}", Mod::get()->getVersion()));

        content->updateLayout();
        m_scrollLayer->scrollToTop();

        auto creditsSpr = CCSprite::createWithSpriteFrameName("communityCreditsBtn_001.png");
        if (creditsSpr) {
            creditsSpr->setScale(0.8f);
            auto creditsBtn = CCMenuItemSpriteExtra::create(creditsSpr, this, menu_selector(SettingsPopup::onCredits));
            auto cornerMenu = CCMenu::create();
            cornerMenu->addChild(creditsBtn);

            float paddingX = 18.f;
            float paddingY = 8.f;
            cornerMenu->setPosition({
                winSize.width - (creditsSpr->getScaledContentSize().width / 2) - paddingX,
                (creditsSpr->getScaledContentSize().height / 2) + paddingY
                });

            m_mainLayer->addChild(cornerMenu);
        }

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
        Notification::create("ID Copied to Clipboard", NotificationIcon::Success)->show();
    }

    void onOpenHistory(CCObject*) {
        HistoryPopup::create()->show();
    }

    void onToggle(CCObject* sender) {
        auto toggleBtn = static_cast<CCMenuItemToggler*>(sender);
        auto keyStr = static_cast<CCString*>(toggleBtn->getUserObject());
        if (keyStr) Mod::get()->setSavedValue<bool>(keyStr->getCString(), !toggleBtn->isToggled());
    }

    void onJoinDiscord(CCObject*) {
        geode::utils::web::openLinkInBrowser("https://discord.gg/dykf3y6HWw");
    }

    void onOpenTerms(CCObject*) {
        TermsScrollPopup::create()->show();
    }

    void onInfo(CCObject* sender) {
        auto btn = static_cast<CCNode*>(sender);
        auto text = static_cast<CCString*>(btn->getUserObject());
        FLAlertLayer::create("Info", text ? text->getCString() : "Info", "OK")->show();
    }

    void onCredits(CCObject*) {
        CreditsPopup::create()->show();
    }

public:
    static SettingsPopup* create() {
        auto ret = new SettingsPopup();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};