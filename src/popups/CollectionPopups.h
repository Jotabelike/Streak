#pragma once
#include "StreakCommon.h"
#include "../StreakData.h"
#include "../FirebaseManager.h"
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/ScrollLayer.hpp>

using namespace geode::prelude;

class EquipBadgePopup : public Popup<std::string> {
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

    bool setup(std::string badgeID) override {
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
        if (ret && ret->initAnchored(250.f, 200.f, badgeID, "geode.loader/GE_square03.png")) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

class EquipBannerPopup : public Popup<std::string> {
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

    bool setup(std::string bannerID) override {
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
        if (ret && ret->initAnchored(280.f, 220.f, bannerID, "geode.loader/GE_square03.png")) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};


class RewardsPopup : public Popup<> {
protected:
    enum DisplayMode { MODE_BADGES, MODE_BANNERS };
    DisplayMode m_currentMode = MODE_BADGES;
    int m_currentCategory = 0;
    int m_currentPage = 0;
    int m_totalPages = 0;

    CCLabelBMFont* m_categoryLabel = nullptr;
    CCMenu* m_badgeMenu = nullptr;
    CCNode* m_decorationNode = nullptr;

    CCMenuItemSpriteExtra* m_pageLeftArrow = nullptr;
    CCMenuItemSpriteExtra* m_pageRightArrow = nullptr;

  
    CCLabelBMFont* m_counterText = nullptr;

   
    CCLabelBMFont* m_totalStatsLabel = nullptr;

    CCMenuItemToggler* m_badgesToggle = nullptr;
    CCMenuItemToggler* m_bannersToggle = nullptr;

    int m_colorIndex = 0;
    float m_colorTransitionTime = 0.0f;
    std::vector<ccColor3B> m_mythicColors;
    ccColor3B m_currentColor;
    ccColor3B m_targetColor;

    
    void onCopyrightInfo(CCObject*) {
        std::string title = "Asset Design Disclaimer";
        std::string desc =
            "Most <cp>Banners</c> are sourced from public resources (e.g. Google Images) and are not owned by the mod creator.\n"
            "However, the majority of <cy>Badges</c> are <cg>original designs</c> made by the mod creator.\n"
            "Special thanks to community members who contributed <cl>exclusive designs</c> for this mod!";

        FLAlertLayer::create(title.c_str(), desc, "OK")->show();
    }

    bool setup() override {
        this->setTitle("Cosmetics");
        auto winSize = m_mainLayer->getContentSize();
        g_streakData.load();

        
        m_mythicColors = {
            ccc3(255, 0, 0), ccc3(255, 165, 0), ccc3(255, 255, 0),
            ccc3(0, 255, 0), ccc3(0, 0, 255), ccc3(75, 0, 130),
            ccc3(238, 130, 238), ccc3(255, 105, 180), ccc3(255, 215, 0), ccc3(192, 192, 192)
        };
        m_currentColor = m_mythicColors[0];
        m_targetColor = m_mythicColors[1];

        
        auto badgeTabOn = ButtonSprite::create("Badges", 60, true, "goldFont.fnt", "GJ_button_01.png", 30.f, 0.45f);
        auto badgeTabOff = ButtonSprite::create("Badges", 60, true, "goldFont.fnt", "GJ_button_01.png", 30.f, 0.45f);
        auto bannerTabOn = ButtonSprite::create("Banners", 60, true, "goldFont.fnt", "GJ_button_01.png", 30.f, 0.45f);
        auto bannerTabOff = ButtonSprite::create("Banners", 60, true, "goldFont.fnt", "GJ_button_01.png", 30.f, 0.45f);

        badgeTabOn->setScale(0.8f); badgeTabOff->setScale(0.8f);
        bannerTabOn->setScale(0.8f); bannerTabOff->setScale(0.8f);

        m_badgesToggle = CCMenuItemToggler::create(badgeTabOff, badgeTabOn, this, menu_selector(RewardsPopup::onSwitchToBadges));
        m_bannersToggle = CCMenuItemToggler::create(bannerTabOff, bannerTabOn, this, menu_selector(RewardsPopup::onSwitchToBanners));

        m_badgesToggle->setPosition({ winSize.width / 2 - 40.f, winSize.height - 45.f });
        m_bannersToggle->setPosition({ winSize.width / 2 + 40.f, winSize.height - 45.f });

        m_badgesToggle->toggle(true);
        m_bannersToggle->toggle(false);

        auto tabMenu = CCMenu::create();
        tabMenu->addChild(m_badgesToggle);
        tabMenu->addChild(m_bannersToggle);
        tabMenu->setPosition(0, 0);
        m_mainLayer->addChild(tabMenu);

       
        auto background = cocos2d::extension::CCScale9Sprite::create("square02_001.png");
        background->setColor({ 0, 0, 0 });
        background->setOpacity(120);
        background->setContentSize({ 320.f, 130.f });
        background->setPosition({ winSize.width / 2, winSize.height / 2 - 25.f });
        m_mainLayer->addChild(background);

       
        float categoryY = winSize.height - 70.f;
        auto catLeftArrow = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        auto catLeftBtn = CCMenuItemSpriteExtra::create(catLeftArrow, this, menu_selector(RewardsPopup::onPreviousCategory));
        catLeftBtn->setPosition(-110.f, 0);

        auto catRightArrow = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        catRightArrow->setFlipX(true);
        auto catRightBtn = CCMenuItemSpriteExtra::create(catRightArrow, this, menu_selector(RewardsPopup::onNextCategory));
        catRightBtn->setPosition(110.f, 0);

        auto catArrowMenu = CCMenu::create();
        catArrowMenu->addChild(catLeftBtn);
        catArrowMenu->addChild(catRightBtn);
        catArrowMenu->setPosition({ winSize.width / 2, categoryY });
        m_mainLayer->addChild(catArrowMenu);

        m_categoryLabel = CCLabelBMFont::create("", "goldFont.fnt");
        m_categoryLabel->setScale(0.6f);
        m_categoryLabel->setPosition({ winSize.width / 2, categoryY });
        m_mainLayer->addChild(m_categoryLabel);

        
        auto pageLeftArrowSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
        m_pageLeftArrow = CCMenuItemSpriteExtra::create(pageLeftArrowSprite, this, menu_selector(RewardsPopup::onPreviousBadgePage));
        m_pageLeftArrow->setPosition(-background->getContentSize().width / 2 - 15.f, 0);

        auto pageRightArrowSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
        pageRightArrowSprite->setFlipX(true);
        m_pageRightArrow = CCMenuItemSpriteExtra::create(pageRightArrowSprite, this, menu_selector(RewardsPopup::onNextBadgePage));
        m_pageRightArrow->setPosition(background->getContentSize().width / 2 + 15.f, 0);

        auto pageArrowMenu = CCMenu::create();
        pageArrowMenu->addChild(m_pageLeftArrow);
        pageArrowMenu->addChild(m_pageRightArrow);
        pageArrowMenu->setPosition(background->getPosition());
        m_mainLayer->addChild(pageArrowMenu);

       
        m_badgeMenu = CCMenu::create();
        m_badgeMenu->setPosition({ 0, 0 });
        m_mainLayer->addChild(m_badgeMenu);

        m_decorationNode = CCNode::create();
        m_decorationNode->setPosition({ 0, 0 });
        m_mainLayer->addChild(m_decorationNode, 5);

       
        m_counterText = CCLabelBMFont::create("", "bigFont.fnt");
        m_counterText->setScale(0.4f);
        m_counterText->setPosition({ winSize.width / 2, 25.f });
        m_mainLayer->addChild(m_counterText);

        
        auto infoSpr = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
        infoSpr->setScale(0.7f);
        auto infoBtn = CCMenuItemSpriteExtra::create(
            infoSpr,
            this,
            menu_selector(RewardsPopup::onCopyrightInfo)
        );
        auto infoMenu = CCMenu::createWithItem(infoBtn);
        infoMenu->setPosition({ 25.f, 25.f });  
        m_mainLayer->addChild(infoMenu);

      
        m_totalStatsLabel = CCLabelBMFont::create("Total 0/0", "goldFont.fnt");
        m_totalStatsLabel->setScale(0.4f);
        m_totalStatsLabel->setAnchorPoint({ 1.0f, 0.5f }); 
        m_totalStatsLabel->setPosition({ winSize.width - 15.f, 25.f }); 
        m_mainLayer->addChild(m_totalStatsLabel);

        this->scheduleUpdate();
        updateCategoryDisplay();
        return true;
    }

    void onSwitchToBadges(CCObject*) {
        if (m_currentMode == MODE_BADGES) {
            m_badgesToggle->toggle(true);
            return;
        }
        m_currentMode = MODE_BADGES;

        m_badgesToggle->toggle(true);
        m_bannersToggle->toggle(false);

        m_currentPage = 0;
        updateCategoryDisplay();
    }

    void onSwitchToBanners(CCObject*) {
        if (m_currentMode == MODE_BANNERS) {
            m_bannersToggle->toggle(true);
            return;
        }
        m_currentMode = MODE_BANNERS;

        m_bannersToggle->toggle(true);
        m_badgesToggle->toggle(false);

        m_currentPage = 0;
        updateCategoryDisplay();
    }

    void update(float dt) override {
      
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

        auto winSize = m_mainLayer->getContentSize();
        float totalGridWidth = (itemsPerRow - 1) * horizontalSpacing;
        CCPoint startPosition = {
            (winSize.width / 2) - (totalGridWidth / 2),
            winSize.height / 2
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
            btn->setUserObject(
                "item_id"_spr,
                CCString::create(item.id)
            );

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
            popup->onStateChanged = [this]() {
                this->updateCategoryDisplay();
                };
            popup->show();
        }
        else {
            auto popup = EquipBannerPopup::create(id);
            popup->onStateChanged = [this]() {
                this->updateCategoryDisplay();
                };
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
        if (ret && ret->initAnchored(360.f, 280.f, "geode.loader/GE_square03.png")) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};