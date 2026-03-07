#pragma once
#include "StreakCommon.h"
#include "../StreakData.h"
#include "../FirebaseManager.h"
#include "../NameModifiers.h"  
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/ScrollLayer.hpp>

using namespace geode::prelude;

class EquipBadgePopup : public Popup {
public:
    std::function<void()> onStateChanged = nullptr;

protected:
    std::string m_badgeID;
    bool m_isCurrentlyEquipped;

    void onCredits(CCObject*) {
        auto badgeInfo = g_streakData.getBadgeInfo(m_badgeID);
        if (badgeInfo) {
            std::string text = "Created by: <cp>" + badgeInfo->creator + "</c>";
            FLAlertLayer::create("Credits", text.c_str(), "OK")->show();
        }
    }

    bool init(std::string badgeID) {
        if (!Popup::init(250.f, 200.f, "geode.loader/GE_square03.png")) return false;

        m_badgeID = badgeID;
        auto winSize = m_mainLayer->getContentSize();
        auto badgeInfo = g_streakData.getBadgeInfo(badgeID);

        if (!badgeInfo) {
            return false;
        }

        bool isUnlocked = g_streakData.isBadgeUnlocked(badgeID);
        auto equippedBadge = g_streakData.getEquippedBadge();
        m_isCurrentlyEquipped = (equippedBadge && equippedBadge->badgeID == badgeID);

        this->setTitle(
            m_isCurrentlyEquipped ? "Badge Equipped" : (isUnlocked ? "Equip Badge" : "Locked Badge")
        );

        auto badgeSprite = CCSprite::create(badgeInfo->spriteName.c_str());
        if (badgeSprite) {
            badgeSprite->setScale(0.3f);
            badgeSprite->setPosition({
                winSize.width / 2,
                winSize.height / 2 + 20
                });

            if (!isUnlocked) {
                badgeSprite->setColor({ 100, 100, 100 });
            }

            m_mainLayer->addChild(badgeSprite);
        }

        auto nameLabel = CCLabelBMFont::create(
            badgeInfo->displayName.c_str(),
            "goldFont.fnt"
        );
        nameLabel->setScale(0.6f);
        nameLabel->setPosition({
            winSize.width / 2,
            winSize.height / 2 - 20
            });
        m_mainLayer->addChild(nameLabel);

        auto categoryLabel = CCLabelBMFont::create(
            g_streakData.getCategoryName(badgeInfo->category).c_str(),
            "bigFont.fnt"
        );
        categoryLabel->setScale(0.4f);
        categoryLabel->setPosition({
            winSize.width / 2,
            winSize.height / 2 - 40
            });
        categoryLabel->setColor(
            g_streakData.getCategoryColor(badgeInfo->category)
        );
        m_mainLayer->addChild(categoryLabel);

        auto creditsSprite = CCSprite::create("credits_btn.png"_spr);
        creditsSprite->setScale(0.7f);

        auto creditsBtn = CCMenuItemSpriteExtra::create(
            creditsSprite,
            this,
            menu_selector(EquipBadgePopup::onCredits)
        );

        auto creditsMenu = CCMenu::create();
        creditsMenu->addChild(creditsBtn);
        creditsMenu->setPosition({
            winSize.width / 2 - 125.f + 30.f,
            winSize.height / 2 - 100.f + 30.f
            });
        m_mainLayer->addChild(creditsMenu);

        if (isUnlocked) {
            auto mainBtn = CCMenuItemSpriteExtra::create(
                ButtonSprite::create(m_isCurrentlyEquipped ? "Unequip" : "Equip"),
                this,
                menu_selector(EquipBadgePopup::onToggleEquip)
            );
            auto menu = CCMenu::create();
            menu->addChild(mainBtn);
            menu->setPosition(
                winSize.width / 2,
                winSize.height / 2 - 70
            );
            m_mainLayer->addChild(menu);
        }
        else {
            auto lockedLabel = CCLabelBMFont::create(
                "Locked",
                "goldFont.fnt"
            );
            lockedLabel->setScale(0.5f);
            lockedLabel->setColor({ 150, 150, 150 });
            lockedLabel->setPosition({
                winSize.width / 2,
                winSize.height / 2 - 70
                });
            m_mainLayer->addChild(lockedLabel);
        }

        return true;
    }

    void onToggleEquip(CCObject*) {
        if (m_isCurrentlyEquipped) {
            g_streakData.unequipBadge();
            FLAlertLayer::create("Success", "Badge unequipped!", "OK")->show();
        }
        else {
            g_streakData.equipBadge(m_badgeID);
            FLAlertLayer::create("Success", "Badge equipped!", "OK")->show();
        }
        g_streakData.save();
        updatePlayerDataInFirebase();

        if (onStateChanged) {
            onStateChanged();
        }

        this->onClose(nullptr);
    }

public:
    static EquipBadgePopup* create(std::string badgeID) {
        auto ret = new EquipBadgePopup();
        if (ret && ret->init(badgeID)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

class EquipBannerPopup : public Popup {
public:
    std::function<void()> onStateChanged = nullptr;

protected:
    std::string m_bannerID;
    bool m_isCurrentlyEquipped;

    void onCredits(CCObject*) {
        auto bannerInfo = g_streakData.getBannerInfo(m_bannerID);
        if (bannerInfo) {
            std::string text = "Created by: <cp>" + bannerInfo->creator + "</c>";
            FLAlertLayer::create("Credits", text.c_str(), "OK")->show();
        }
    }

    bool init(std::string bannerID) {
        if (!Popup::init(280.f, 220.f, "geode.loader/GE_square03.png")) return false;

        m_bannerID = bannerID;
        auto winSize = m_mainLayer->getContentSize();
        auto bannerInfo = g_streakData.getBannerInfo(bannerID);

        if (!bannerInfo) {
            return false;
        }

        bool isUnlocked = g_streakData.isBannerUnlocked(bannerID);
        auto equippedBanner = g_streakData.getEquippedBanner();
        m_isCurrentlyEquipped = (equippedBanner && equippedBanner->bannerID == bannerID);

        this->setTitle(
            m_isCurrentlyEquipped ? "Banner Equipped" : (isUnlocked ? "Equip Banner" : "Locked Banner")
        );

        auto bannerSprite = CCSprite::create(bannerInfo->spriteName.c_str());
        if (bannerSprite) {
            float maxWidth = 220.f;
            float scale = maxWidth / bannerSprite->getContentSize().width;
            if (scale > 0.8f) {
                scale = 0.8f;
            }

            bannerSprite->setScale(scale);
            bannerSprite->setPosition({
                winSize.width / 2,
                winSize.height / 2 + 15
                });

            if (!isUnlocked) {
                bannerSprite->setColor({ 100, 100, 100 });
            }

            m_mainLayer->addChild(bannerSprite);
        }

        auto nameLabel = CCLabelBMFont::create(
            bannerInfo->displayName.c_str(),
            "goldFont.fnt"
        );
        nameLabel->setScale(0.6f);
        nameLabel->setPosition({
            winSize.width / 2,
            winSize.height / 2 - 25
            });
        m_mainLayer->addChild(nameLabel);

        auto categoryLabel = CCLabelBMFont::create(
            g_streakData.getCategoryName(bannerInfo->rarity).c_str(),
            "bigFont.fnt"
        );
        categoryLabel->setScale(0.4f);
        categoryLabel->setPosition({
            winSize.width / 2,
            winSize.height / 2 - 45
            });
        categoryLabel->setColor(
            g_streakData.getCategoryColor(bannerInfo->rarity)
        );
        m_mainLayer->addChild(categoryLabel);

        auto creditsSprite = CCSprite::create("credits_btn.png"_spr);
        creditsSprite->setScale(0.7f);

        auto creditsBtn = CCMenuItemSpriteExtra::create(
            creditsSprite,
            this,
            menu_selector(EquipBannerPopup::onCredits)
        );

        auto creditsMenu = CCMenu::create();
        creditsMenu->addChild(creditsBtn);
        creditsMenu->setPosition({
            winSize.width / 2 - 140.f + 30.f,
            winSize.height / 2 - 110.f + 30.f
            });
        m_mainLayer->addChild(creditsMenu);

        if (isUnlocked) {
            auto mainBtn = CCMenuItemSpriteExtra::create(
                ButtonSprite::create(m_isCurrentlyEquipped ? "Unequip" : "Equip"),
                this,
                menu_selector(EquipBannerPopup::onToggleEquip)
            );
            auto menu = CCMenu::create();
            menu->addChild(mainBtn);
            menu->setPosition(
                winSize.width / 2,
                winSize.height / 2 - 75
            );
            m_mainLayer->addChild(menu);
        }
        else {
            auto lockedLabel = CCLabelBMFont::create(
                "Locked",
                "goldFont.fnt"
            );
            lockedLabel->setScale(0.5f);
            lockedLabel->setColor({ 150, 150, 150 });
            lockedLabel->setPosition({
                winSize.width / 2,
                winSize.height / 2 - 75
                });
            m_mainLayer->addChild(lockedLabel);
        }

        return true;
    }

    void onToggleEquip(CCObject*) {
        if (m_isCurrentlyEquipped) {
            g_streakData.unequipBanner();
            FLAlertLayer::create("Success", "Banner unequipped!", "OK")->show();
        }
        else {
            g_streakData.equipBanner(m_bannerID);
            FLAlertLayer::create("Success", "Banner equipped!", "OK")->show();
        }
        g_streakData.save();
        updatePlayerDataInFirebase();

        if (onStateChanged) {
            onStateChanged();
        }

        this->onClose(nullptr);
    }

public:
    static EquipBannerPopup* create(std::string bannerID) {
        auto ret = new EquipBannerPopup();
        if (ret && ret->init(bannerID)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};


class RewardsPopup : public Popup {
protected:
    enum DisplayMode { MODE_BADGES, MODE_BANNERS, MODE_NAMES };
    DisplayMode m_currentMode = MODE_BADGES;
    int m_currentCategory = 0;
    int m_currentPage = 0;
    int m_totalPages = 0;

    CCLabelBMFont* m_categoryLabel = nullptr;
    CCMenu* m_badgeMenu = nullptr;
    CCNode* m_decorationNode = nullptr;
    CCNode* m_namesContainer = nullptr;
    CCLabelBMFont* m_namePreviewLabel = nullptr;

    CCMenu* m_catArrowMenu = nullptr;
    CCMenu* m_pageArrowMenu = nullptr;
    cocos2d::extension::CCScale9Sprite* m_background = nullptr;

    CCMenuItemSpriteExtra* m_pageLeftArrow = nullptr;
    CCMenuItemSpriteExtra* m_pageRightArrow = nullptr;

    CCLabelBMFont* m_counterText = nullptr;
    CCLabelBMFont* m_totalStatsLabel = nullptr;

    int m_colorIndex = 0;
    float m_colorTransitionTime = 0.0f;
    std::vector<ccColor3B> m_mythicColors;
    ccColor3B m_currentColor;
    ccColor3B m_targetColor;

    const float POPUP_WIDTH = 360.f;
    const float POPUP_HEIGHT = 280.f;
    const float CENTER_X = 180.f;
    const float CENTER_Y = 140.f;

   
    std::string m_previewEffect;
    std::string m_previewAnimation;
    std::string m_previewColor;
    std::string m_previewFont;

    std::string m_selectedLockedItem;
    int m_selectedLockedTag = 0;
    CCMenuItemSpriteExtra* m_buyNameBtn = nullptr;
    CCLabelBMFont* m_buyPriceLabel = nullptr;
    CCMenuItemSpriteExtra* m_selectedLockedBtn = nullptr;  

    void onCopyrightInfo(CCObject*) {
        std::string title = "Asset Design Disclaimer";
        std::string desc =
            "Most <cp>Banners</c> are sourced from public resources (e.g. Google Images) and are not owned by the mod creator.\n"
            "However, the majority of <cy>Badges</c> are <cg>original designs</c> made by the mod creator.\n"
            "Special thanks to community members who contributed <cl>exclusive designs</c> for this mod!";

        FLAlertLayer::create(title.c_str(), desc, "OK")->show();
    }

    void onNameOptionClicked(CCObject* sender) {
        auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
        auto id = static_cast<CCString*>(btn->getUserObject())->getCString();
        int tag = btn->getTag();

     
        if (tag == 1) m_previewEffect = id;
        else if (tag == 2) m_previewAnimation = id;
        else if (tag == 3) m_previewColor = id;
        else if (tag == 4) m_previewFont = id;

        updateNamePreview();

      
        if (g_streakData.isNameItemUnlocked(id)) {
       
            if (tag == 1) g_streakData.equippedNameEffect = id;
            else if (tag == 2) g_streakData.equippedNameAnimation = id;
            else if (tag == 3) g_streakData.equippedNameColor = id;
            else if (tag == 4) g_streakData.equippedNameFont = id;

            g_streakData.save();
            updatePlayerDataInFirebase();
            if (m_buyNameBtn) m_buyNameBtn->setVisible(false);

         
            
        }
        else {
      
            m_selectedLockedItem = id;
            m_selectedLockedTag = tag;
            m_selectedLockedBtn = btn; 

            int price = g_streakData.getNameItemPrice(id);
            if (m_buyPriceLabel) {
                m_buyPriceLabel->setString(fmt::format("{}", price).c_str());
            }

            if (m_buyNameBtn) m_buyNameBtn->setVisible(true);
        }
    }

    void onBuyNameItem(CCObject*) {
        int price = g_streakData.getNameItemPrice(m_selectedLockedItem);

        if (g_streakData.gems >= price) {
         
            g_streakData.gems -= price;
            g_streakData.unlockNameItem(m_selectedLockedItem);

          
            if (m_selectedLockedTag == 1) g_streakData.equippedNameEffect = m_selectedLockedItem;
            else if (m_selectedLockedTag == 2) g_streakData.equippedNameAnimation = m_selectedLockedItem;
            else if (m_selectedLockedTag == 3) g_streakData.equippedNameColor = m_selectedLockedItem;
            else if (m_selectedLockedTag == 4) g_streakData.equippedNameFont = m_selectedLockedItem;

            g_streakData.save();
            updatePlayerDataInFirebase();

     
            FMODAudioEngine::sharedEngine()->playEffect("buyItem01.ogg");

           
            if (m_selectedLockedBtn) {
                if (auto label = static_cast<CCLabelBMFont*>(m_selectedLockedBtn->getNormalImage())) {
                    label->setColor({ 255, 255, 255 });  
                    if (auto lock = label->getChildByTag(888)) {  
                        lock->removeFromParent();  
                    }
                  
                    if (m_selectedLockedTag == 3) NameModifiers::applyColor(label, m_selectedLockedItem);
                    if (m_selectedLockedTag == 4) NameModifiers::applyFont(label, m_selectedLockedItem);
                    if (m_selectedLockedTag == 1) NameModifiers::applyEffect(label, m_selectedLockedItem);
                    if (m_selectedLockedTag == 2) NameModifiers::applyAnimation(label, m_selectedLockedItem);
                }
            }

            m_buyNameBtn->setVisible(false);
            FLAlertLayer::create("Success!", "Cosmetic purchased and equipped.", "OK")->show();
        }
        else {
            FLAlertLayer::create("Oops!", "You don't have enough gems.", "OK")->show();
        }
    }

    void updateNamePreview() {
        if (!m_namePreviewLabel) return;

        m_namePreviewLabel->setString("PlayerName");

       
        m_namePreviewLabel->stopAllActions();
        m_namePreviewLabel->setScale(0.8f);
        m_namePreviewLabel->setRotation(0.f);

        NameModifiers::applyFont(m_namePreviewLabel, m_previewFont);
        NameModifiers::applyColor(m_namePreviewLabel, m_previewColor);
        NameModifiers::applyAnimation(m_namePreviewLabel, m_previewAnimation);
        NameModifiers::applyEffect(m_namePreviewLabel, m_previewEffect);
    }

    bool init() override {
        if (!Popup::init(POPUP_WIDTH, POPUP_HEIGHT, "geode.loader/GE_square03.png")) return false;

        this->setTitle("Cosmetics");
        g_streakData.load();

        m_mythicColors = {
            ccc3(255, 0, 0),
            ccc3(255, 165, 0),
            ccc3(255, 255, 0),
            ccc3(0, 255, 0),
            ccc3(0, 0, 255), 
            ccc3(75, 0, 130),
            ccc3(238, 130, 238), 
            ccc3(255, 105, 180), 
            ccc3(255, 215, 0), 
            ccc3(192, 192, 192)
        };
        m_currentColor = m_mythicColors[0];
        m_targetColor = m_mythicColors[1];

        auto badgeBtnSpr = CCSprite::create("badges_btn_c.png"_spr);
        if (!badgeBtnSpr) badgeBtnSpr = ButtonSprite::create("Badges");
        badgeBtnSpr->setScale(0.25f);
        auto badgeBtn = CCMenuItemSpriteExtra::create(badgeBtnSpr, this, menu_selector(RewardsPopup::onSwitchToBadges));

        auto bannerBtnSpr = CCSprite::create("banners_btn_c.png"_spr);
        if (!bannerBtnSpr) bannerBtnSpr = ButtonSprite::create("Banners");
        bannerBtnSpr->setScale(0.25f);
        auto bannerBtn = CCMenuItemSpriteExtra::create(bannerBtnSpr, this, menu_selector(RewardsPopup::onSwitchToBanners));

        auto nameBtnSpr = CCSprite::create("name_btn_c.png"_spr);
        if (!nameBtnSpr) nameBtnSpr = ButtonSprite::create("Name");
        nameBtnSpr->setScale(0.25f);
        auto nameBtn = CCMenuItemSpriteExtra::create(nameBtnSpr, this, menu_selector(RewardsPopup::onSwitchToNames));

        auto leftMenu = CCMenu::create();
        leftMenu->addChild(badgeBtn);
        leftMenu->addChild(bannerBtn);
        leftMenu->addChild(nameBtn);
        leftMenu->alignItemsVerticallyWithPadding(6.f);
        leftMenu->setPosition({ -20.f, CENTER_Y + 50.f });
        m_mainLayer->addChild(leftMenu);

        m_background = cocos2d::extension::CCScale9Sprite::create("square02_001.png");
        m_background->setColor({ 0, 0, 0 });
        m_background->setOpacity(120);
        m_background->setContentSize({ 320.f, 130.f });
        m_background->setPosition({ CENTER_X, CENTER_Y - 25.f });
        m_mainLayer->addChild(m_background);

        float categoryY = POPUP_HEIGHT - 70.f;
        auto catLeftArrow = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        auto catLeftBtn = CCMenuItemSpriteExtra::create(catLeftArrow, this, menu_selector(RewardsPopup::onPreviousCategory));
        catLeftBtn->setPosition(-110.f, 0);

        auto catRightArrow = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        catRightArrow->setFlipX(true);
        auto catRightBtn = CCMenuItemSpriteExtra::create(catRightArrow, this, menu_selector(RewardsPopup::onNextCategory));
        catRightBtn->setPosition(110.f, 0);

        m_catArrowMenu = CCMenu::create();
        m_catArrowMenu->addChild(catLeftBtn);
        m_catArrowMenu->addChild(catRightBtn);
        m_catArrowMenu->setPosition({ CENTER_X, categoryY });
        m_mainLayer->addChild(m_catArrowMenu);

        m_categoryLabel = CCLabelBMFont::create("", "goldFont.fnt");
        m_categoryLabel->setScale(0.6f);
        m_categoryLabel->setPosition({ CENTER_X, categoryY });
        m_mainLayer->addChild(m_categoryLabel);

        auto pageLeftArrowSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
        m_pageLeftArrow = CCMenuItemSpriteExtra::create(pageLeftArrowSprite, this, menu_selector(RewardsPopup::onPreviousBadgePage));
        m_pageLeftArrow->setPosition(-m_background->getContentSize().width / 2 - 15.f, 0);

        auto pageRightArrowSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
        pageRightArrowSprite->setFlipX(true);
        m_pageRightArrow = CCMenuItemSpriteExtra::create(pageRightArrowSprite, this, menu_selector(RewardsPopup::onNextBadgePage));
        m_pageRightArrow->setPosition(m_background->getContentSize().width / 2 + 15.f, 0);

        m_pageArrowMenu = CCMenu::create();
        m_pageArrowMenu->addChild(m_pageLeftArrow);
        m_pageArrowMenu->addChild(m_pageRightArrow);
        m_pageArrowMenu->setPosition(m_background->getPosition());
        m_mainLayer->addChild(m_pageArrowMenu);

        m_badgeMenu = CCMenu::create();
        m_badgeMenu->setPosition({ 0, 0 });
        m_mainLayer->addChild(m_badgeMenu);

        m_decorationNode = CCNode::create();
        m_decorationNode->setPosition({ 0, 0 });
        m_mainLayer->addChild(m_decorationNode, 5);

        m_counterText = CCLabelBMFont::create("", "bigFont.fnt");
        m_counterText->setScale(0.4f);
        m_counterText->setPosition({ CENTER_X, 25.f });
        m_mainLayer->addChild(m_counterText);

        auto infoSpr = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
        infoSpr->setScale(0.7f);
        auto infoBtn = CCMenuItemSpriteExtra::create(infoSpr, this, menu_selector(RewardsPopup::onCopyrightInfo));
        auto infoMenu = CCMenu::createWithItem(infoBtn);
        infoMenu->setPosition({ 25.f, POPUP_HEIGHT - 25.f });
        m_mainLayer->addChild(infoMenu);

        m_totalStatsLabel = CCLabelBMFont::create("Total 0/0", "goldFont.fnt");
        m_totalStatsLabel->setScale(0.4f);
        m_totalStatsLabel->setAnchorPoint({ 1.0f, 0.5f });
        m_totalStatsLabel->setPosition({ POPUP_WIDTH - 15.f, 25.f });
        m_mainLayer->addChild(m_totalStatsLabel);

        m_namesContainer = CCNode::create();
        m_namesContainer->setPosition(m_background->getPosition());
        m_namesContainer->setVisible(false);
        m_mainLayer->addChild(m_namesContainer, 10);

        m_previewEffect = g_streakData.equippedNameEffect;
        m_previewAnimation = g_streakData.equippedNameAnimation;
        m_previewColor = g_streakData.equippedNameColor;
        m_previewFont = g_streakData.equippedNameFont;

        float colOffsets[4] = { -125.f, -42.f, 42.f, 125.f };

        auto effectsTitle = CCLabelBMFont::create("Effects", "goldFont.fnt");
        effectsTitle->setScale(0.5f);
        effectsTitle->setPosition({ colOffsets[0], 50.f });
        m_namesContainer->addChild(effectsTitle);

        auto animsTitle = CCLabelBMFont::create("Anims", "goldFont.fnt");
        animsTitle->setScale(0.5f);
        animsTitle->setPosition({ colOffsets[1], 50.f });
        m_namesContainer->addChild(animsTitle);

        auto colorsTitle = CCLabelBMFont::create("Colors", "goldFont.fnt");
        colorsTitle->setScale(0.5f);
        colorsTitle->setPosition({ colOffsets[2], 50.f });
        m_namesContainer->addChild(colorsTitle);

        auto fontsTitle = CCLabelBMFont::create("Fonts", "goldFont.fnt");
        fontsTitle->setScale(0.5f);
        fontsTitle->setPosition({ colOffsets[3], 50.f });
        m_namesContainer->addChild(fontsTitle);

        std::vector<std::string> effects = {
               "None", "Sparkle", "Fire", "Snow", "Poison", "Stars", "Void",
               "Rain", "Blood", "Holy", "Toxic", "Multy3D"
        };
        std::vector<std::string> animations = {
            "None", "Dynamic jump", "Wave", "Pulse", "Bounce", "Swing",
            "Glitch", "Shake", "Spin", "Jelly", "Heartbeat", "Float","DVD","Domino"
        };
        std::vector<std::string> colors = {
            "Default", "Rainbow", "Red", "Blue", "Green", "Yellow",
            "Purple", "Orange", "Black", "Cyan", "Pink", "Lime", "Magenta"
        };
        std::vector<std::string> fonts = {
            "Default", "Pusab", "Gold", "Chat", "Serigrafia",
            "Pixel", "Blocky", "Mecano", "Monster", "Tech"
        };

        auto createColumn = [this](const std::vector<std::string>& items, float xOffset, int tag) {
            float listWidth = 80.f;
            float listHeight = 100.f;
            float itemHeight = 24.f;
            float totalHeight = items.size() * itemHeight;
            if (totalHeight < listHeight) totalHeight = listHeight;

            auto scrollLayer = geode::ScrollLayer::create({ listWidth, listHeight });
            scrollLayer->setPosition({ xOffset - listWidth / 2.f, -55.f });

            auto menu = CCMenu::create();
            menu->setContentSize({ listWidth, totalHeight });
            menu->setPosition({ 0, 0 });

            float currentY = totalHeight - (itemHeight / 2.f);

            for (const auto& item : items) {
                auto label = CCLabelBMFont::create(item.c_str(), "chatFont.fnt");
                label->setScale(0.5f);

                if (tag == 3) NameModifiers::applyColor(label, item);
                if (tag == 4) NameModifiers::applyFont(label, item);

                float maxLabelWidth = listWidth - 5.f;
                if (label->getContentSize().width * label->getScale() > maxLabelWidth) {
                    label->setScale(maxLabelWidth / label->getContentSize().width);
                }

                if (!g_streakData.isNameItemUnlocked(item)) {
                    label->setColor({ 150, 150, 150 });
                    auto lockIcon = CCSprite::createWithSpriteFrameName("GJ_lock_001.png");
                    lockIcon->setScale(0.5f);
                    lockIcon->setPosition({ -8.f, label->getContentSize().height / 2.f });
                    lockIcon->setTag(888);  
                    label->addChild(lockIcon);
                }

                auto btn = CCMenuItemSpriteExtra::create(label, this, menu_selector(RewardsPopup::onNameOptionClicked));
                btn->setUserObject(CCString::create(item));
                btn->setTag(tag);
                btn->setPosition({ listWidth / 2.f, currentY });
                menu->addChild(btn);

                if (tag == 1) NameModifiers::applyEffect(label, item);
                if (tag == 2) NameModifiers::applyAnimation(label, item);

                currentY -= itemHeight;
            }

            scrollLayer->m_contentLayer->addChild(menu);
            scrollLayer->m_contentLayer->setContentSize({ listWidth, totalHeight });
            scrollLayer->scrollToTop();

            m_namesContainer->addChild(scrollLayer);
            };

        createColumn(effects, colOffsets[0], 1);
        createColumn(animations, colOffsets[1], 2);
        createColumn(colors, colOffsets[2], 3);
        createColumn(fonts, colOffsets[3], 4);

        m_namePreviewLabel = CCLabelBMFont::create("PlayerName", "bigFont.fnt");
        m_namePreviewLabel->setPosition({ -50.f, -90.f });
        m_namePreviewLabel->setScale(0.8f);
        m_namesContainer->addChild(m_namePreviewLabel);

        auto buyBtnSprite = ButtonSprite::create("        ", 0, false, "bigFont.fnt", "GJ_button_01.png", 25.f, 0.6f);

        m_buyPriceLabel = CCLabelBMFont::create("100", "bigFont.fnt");
        m_buyPriceLabel->setScale(0.6f);
        m_buyPriceLabel->setPosition({ buyBtnSprite->getContentSize().width / 2.f - 12.f, buyBtnSprite->getContentSize().height / 2.f });
        buyBtnSprite->addChild(m_buyPriceLabel);

        auto gemIcon = CCSprite::create("gem.png"_spr);
        if (!gemIcon) gemIcon = CCSprite::createWithSpriteFrameName("GJ_diamondsIcon_001.png");
        gemIcon->setScale(0.2f);
        gemIcon->setPosition({ buyBtnSprite->getContentSize().width / 2.f + 18.f, buyBtnSprite->getContentSize().height / 2.f });
        buyBtnSprite->addChild(gemIcon);

        m_buyNameBtn = CCMenuItemSpriteExtra::create(buyBtnSprite, this, menu_selector(RewardsPopup::onBuyNameItem));
        m_buyNameBtn->setVisible(false);

        auto buyMenu = CCMenu::create();
        buyMenu->addChild(m_buyNameBtn);
        buyMenu->setPosition({ 100.f, -90.f });
        m_namesContainer->addChild(buyMenu);

        updateNamePreview();

        this->scheduleUpdate();
        updateCategoryDisplay();
        return true;
    }

    void toggleUIVisibility(bool isNames) {
        m_namesContainer->setVisible(isNames);
        m_badgeMenu->setVisible(!isNames);
        m_decorationNode->setVisible(!isNames);
        m_catArrowMenu->setVisible(!isNames);
        m_pageArrowMenu->setVisible(!isNames);
        m_counterText->setVisible(!isNames);
        m_totalStatsLabel->setVisible(!isNames);
    }

    void onSwitchToBadges(CCObject*) {
        if (m_currentMode == MODE_BADGES) return;
        m_currentMode = MODE_BADGES;
        m_currentPage = 0;
        toggleUIVisibility(false);
        updateCategoryDisplay();
    }

    void onSwitchToBanners(CCObject*) {
        if (m_currentMode == MODE_BANNERS) return;
        m_currentMode = MODE_BANNERS;
        m_currentPage = 0;
        toggleUIVisibility(false);
        updateCategoryDisplay();
    }

    void onSwitchToNames(CCObject*) {
        if (m_currentMode == MODE_NAMES) return;
        m_currentMode = MODE_NAMES;
        toggleUIVisibility(true);
        m_categoryLabel->setString("Name Customization");
        m_categoryLabel->setColor({ 255, 255, 255 });
    }

    void update(float dt) override {
        if (m_currentMode == MODE_NAMES) return;

        if (static_cast<StreakData::BadgeCategory>(m_currentCategory) != StreakData::BadgeCategory::MYTHIC || !m_categoryLabel) {
            m_categoryLabel->setColor(
                g_streakData.getCategoryColor(static_cast<StreakData::BadgeCategory>(m_currentCategory))
            );
            return;
        }
        m_colorTransitionTime += dt;
        if (m_colorTransitionTime >= 1.0f) {
            m_colorTransitionTime = 0.0f;
            m_colorIndex = (m_colorIndex + 1) % m_mythicColors.size();
            m_currentColor = m_targetColor;
            m_targetColor = m_mythicColors[(m_colorIndex + 1) % m_mythicColors.size()];
        }
        float progress = m_colorTransitionTime / 1.0f;
        ccColor3B interpolatedColor = {
            static_cast<GLubyte>(m_currentColor.r + (m_targetColor.r - m_currentColor.r) * progress),
            static_cast<GLubyte>(m_currentColor.g + (m_targetColor.g - m_currentColor.g) * progress),
            static_cast<GLubyte>(m_currentColor.b + (m_targetColor.b - m_currentColor.b) * progress)
        };
        m_categoryLabel->setColor(interpolatedColor);
    }

    void updateCategoryDisplay() {
        if (m_currentMode == MODE_NAMES) return;

        m_badgeMenu->removeAllChildren();
        m_decorationNode->removeAllChildren();

        StreakData::BadgeCategory currentCat = static_cast<StreakData::BadgeCategory>(m_currentCategory);
        m_categoryLabel->setString(g_streakData.getCategoryName(currentCat).c_str());

        std::string equippedID = "";
        if (m_currentMode == MODE_BADGES) {
            auto eq = g_streakData.getEquippedBadge();
            if (eq) equippedID = eq->badgeID;
        }
        else {
            auto eq = g_streakData.getEquippedBanner();
            if (eq) equippedID = eq->bannerID;
        }

        struct DisplayItem {
            std::string id;
            std::string spriteName;
            bool unlocked;
            bool isFromRoulette;
            int daysRequired;
        };
        std::vector<DisplayItem> itemsToShow;

        int globalTotal = 0;
        int globalUnlocked = 0;

        if (m_currentMode == MODE_BADGES) {
            for (auto& badge : g_streakData.badges) {
                if (badge.category == currentCat) {
                    itemsToShow.push_back({
                        badge.badgeID,
                        badge.spriteName,
                        g_streakData.isBadgeUnlocked(badge.badgeID),
                        badge.isFromRoulette,
                        badge.daysRequired
                        });
                }
            }
            globalTotal = g_streakData.badges.size();
            for (auto& b : g_streakData.badges) {
                if (g_streakData.isBadgeUnlocked(b.badgeID)) globalUnlocked++;
            }
            if (m_totalStatsLabel) {
                m_totalStatsLabel->setString(fmt::format("Badges: {}/{}", globalUnlocked, globalTotal).c_str());
            }
        }
        else {
            for (auto& banner : g_streakData.banners) {
                if (banner.rarity == currentCat) {
                    itemsToShow.push_back({
                        banner.bannerID,
                        banner.spriteName,
                        g_streakData.isBannerUnlocked(banner.bannerID),
                        true,
                        0
                        });
                }
            }
            globalTotal = g_streakData.banners.size();
            for (auto& b : g_streakData.banners) {
                if (g_streakData.isBannerUnlocked(b.bannerID)) globalUnlocked++;
            }
            if (m_totalStatsLabel) {
                m_totalStatsLabel->setString(fmt::format("Banners: {}/{}", globalUnlocked, globalTotal).c_str());
            }
        }

        int itemsPerPage = 6;
        int itemsPerRow = 3;

        if (m_currentMode == MODE_BANNERS) {
            itemsPerRow = 2;
            itemsPerPage = 4;
        }

        m_totalPages = static_cast<int>(ceil(static_cast<float>(itemsToShow.size()) / itemsPerPage));
        if (m_totalPages == 0) {
            m_currentPage = 0;
        }
        else if (m_currentPage >= m_totalPages) {
            m_currentPage = m_totalPages - 1;
        }

        int startIndex = m_currentPage * itemsPerPage;
        int endIndex = std::min(static_cast<int>(itemsToShow.size()), startIndex + itemsPerPage);

        float horizontalSpacing = (m_currentMode == MODE_BANNERS) ? 130.f : 65.f;
        float verticalSpacing = 55.f;

        float totalGridWidth = (itemsPerRow - 1) * horizontalSpacing;

        CCPoint startPosition = {
            CENTER_X - (totalGridWidth / 2),
            CENTER_Y + 5.f
        };

        for (int i = startIndex; i < endIndex; ++i) {
            int indexOnPage = i - startIndex;
            int row = indexOnPage / itemsPerRow;
            int col = indexOnPage % itemsPerRow;
            auto& item = itemsToShow[i];

            auto sprite = CCSprite::create(item.spriteName.c_str());
            if (!sprite) {
                sprite = CCSprite::create("GJ_button_01.png");
            }

            float scale = 0.3f;
            if (m_currentMode == MODE_BANNERS) {
                float targetWidth = 110.f;
                scale = targetWidth / sprite->getContentSize().width;
            }
            sprite->setScale(scale);

            if (!item.unlocked) {
                sprite->setColor({ 100, 100, 100 });
            }

            CCMenuItemSpriteExtra* btn = CCMenuItemSpriteExtra::create(
                sprite,
                this,
                menu_selector(RewardsPopup::onItemClick)
            );
            btn->setUserObject("item_id"_spr, CCString::create(item.id));

            CCPoint position = {
                startPosition.x + col * horizontalSpacing,
                startPosition.y - row * verticalSpacing
            };

            btn->setPosition(position);
            m_badgeMenu->addChild(btn);

            if (item.id == equippedID) {
                auto check = CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
                check->setPosition(position + CCPoint{ 15.f, -15.f });
                check->setScale(0.6f);
                m_decorationNode->addChild(check);
            }

            if (!item.unlocked) {
                auto lockIcon = CCSprite::createWithSpriteFrameName("GJ_lock_001.png");
                lockIcon->setPosition(position);
                lockIcon->setScale(0.7f);
                m_decorationNode->addChild(lockIcon);
            }
        }

        m_pageLeftArrow->setVisible(m_currentPage > 0);
        m_pageRightArrow->setVisible(m_currentPage < m_totalPages - 1);

        int categoryUnlockedCount = 0;
        for (auto& it : itemsToShow) {
            if (it.unlocked) categoryUnlockedCount++;
        }
        m_counterText->setString(
            fmt::format("Category: {}/{}", categoryUnlockedCount, itemsToShow.size()).c_str()
        );
    }

    void onItemClick(CCObject* sender) {
        auto id = static_cast<CCString*>(static_cast<CCNode*>(sender)->getUserObject("item_id"_spr))->getCString();

        if (m_currentMode == MODE_BADGES) {
            auto popup = EquipBadgePopup::create(id);
            popup->onStateChanged = [this]() { this->updateCategoryDisplay(); };
            popup->show();
        }
        else if (m_currentMode == MODE_BANNERS) {
            auto popup = EquipBannerPopup::create(id);
            popup->onStateChanged = [this]() { this->updateCategoryDisplay(); };
            popup->show();
        }
    }

    void onNextBadgePage(CCObject*) {
        if (m_currentPage < m_totalPages - 1) {
            m_currentPage++;
            updateCategoryDisplay();
        }
    }

    void onPreviousBadgePage(CCObject*) {
        if (m_currentPage > 0) {
            m_currentPage--;
            updateCategoryDisplay();
        }
    }

    void onNextCategory(CCObject*) {
        m_currentCategory = (m_currentCategory + 1) % 5;
        m_currentPage = 0;
        updateCategoryDisplay();
    }

    void onPreviousCategory(CCObject*) {
        m_currentCategory = (m_currentCategory - 1 + 5) % 5;
        m_currentPage = 0;
        updateCategoryDisplay();
    }

    void onClose(CCObject* sender) override {
        this->unscheduleUpdate();
        Popup::onClose(sender);
    }

public:
    static RewardsPopup* create() {
        auto ret = new RewardsPopup();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};