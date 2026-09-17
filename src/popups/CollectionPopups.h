#pragma once
#include "StreakCommon.h"
#include "../StreakData.h"
#include "../FirebaseManager.h"
#include "../NameModifiers.h"
#include "../StreakMusic.h"
#include "../RemoteAssetManager.h"
#include "../PassNameCosmetics.h"
#include "../ProfileCardEffects.h"
#include "../ProfileCardStyles.h"
#include "../ProfileCardDraws.h"
#include "ProfileCardPopup.h"
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/ScrollLayer.hpp>
#include <Geode/ui/LoadingSpinner.hpp>
#include "QualityNode.h"
#include "../utils/ScrollbarUtils.h"


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

        
        auto qualityNode = QualityNode::create();
        qualityNode->setPosition({
            winSize.width / 2,
            winSize.height / 2 - 40
            });
        qualityNode->setCategory(static_cast<int>(badgeInfo->category), false);
        m_mainLayer->addChild(qualityNode);
      

        auto creditsSprite = CCSprite::createWithSpriteFrameName("communityCreditsBtn_001.png");
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
            g_streakData.equippedBadge.clear();
            FLAlertLayer::create("Success", "Badge unequipped!", "OK")->show();
        }
        else {
            g_streakData.equippedBadge = m_badgeID;
            FLAlertLayer::create("Success", "Badge equipped!", "OK")->show();
        }

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

      
        auto qualityNode = QualityNode::create();
        qualityNode->setPosition({
            winSize.width / 2,
            winSize.height / 2 - 45
            });
        qualityNode->setCategory(static_cast<int>(bannerInfo->rarity), false);
        m_mainLayer->addChild(qualityNode);
      

        auto creditsSprite = CCSprite::createWithSpriteFrameName("communityCreditsBtn_001.png");
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
            g_streakData.equippedBanner.clear();
            FLAlertLayer::create("Success", "Banner unequipped!", "OK")->show();
        }
        else {
            g_streakData.equippedBanner = m_bannerID;
            FLAlertLayer::create("Success", "Banner equipped!", "OK")->show();
        }

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


class EquipSongPopup : public Popup {
public:
    std::function<void()> onStateChanged = nullptr;

protected:
    std::string m_songID;
    bool m_isCurrentlyEquipped = false;
    bool m_isUnlocked = false;
    bool m_needsMusicRestore = false;

    void stopPreview() {
        auto eng = FMODAudioEngine::sharedEngine();
        if (eng) eng->stopChannel(STREAK_SONG_PREVIEW_CHANNEL, AudioTargetType::SFXChannel, false, 0.f);
    }

    void onPlay(CCObject*) {
        auto songInfo = g_streakData.getSongInfo(m_songID);
        if (!songInfo) return;
        auto eng = FMODAudioEngine::sharedEngine();
        if (!eng) return;

        eng->stopChannel(STREAK_SONG_PREVIEW_CHANNEL, AudioTargetType::SFXChannel, false, 0.f);
        eng->stopChannel(STREAK_MENU_MUSIC_CHANNEL, AudioTargetType::SFXChannel, false, 0.f);
        m_needsMusicRestore = true;

        float vol = StreakMusic::getVolume();
        eng->playEffectAdvanced(
            songInfo->fileName,
            1.0f, 1.0f, vol, 1.0f,
            false, false,
            0, 0, 0, 0,
            true,
            0, true, false,
            STREAK_SONG_PREVIEW_CHANNEL,
            0, 0.0f, 0
        );
        eng->setChannelVolume(STREAK_SONG_PREVIEW_CHANNEL, AudioTargetType::SFXChannel, vol);
    }

    void onStop(CCObject*) {
        stopPreview();
    }

    bool init(std::string songID) {
        if (!Popup::init(300.f, 250.f, "geode.loader/GE_square03.png")) return false;

        m_songID = songID;
        auto winSize = m_mainLayer->getContentSize();
        auto songInfo = g_streakData.getSongInfo(songID);
        if (!songInfo) return false;

        m_isUnlocked = g_streakData.isSongUnlocked(songID);
        m_isCurrentlyEquipped = (g_streakData.equippedSong == songID);

        this->setTitle(
            m_isCurrentlyEquipped ? "Song Equipped" : (m_isUnlocked ? "Equip Song" : "Locked Song")
        );

        auto icon = CCSprite::create(songInfo->iconName.c_str());
        if (!icon) icon = CCSprite::create("GJ_button_01.png");
        if (icon) {
            float maxSize = 80.f;
            float scale = maxSize / std::max(icon->getContentSize().width, icon->getContentSize().height);
            if (scale > 1.5f) scale = 1.5f;
            icon->setScale(scale);
            icon->setPosition({ winSize.width / 2, winSize.height - 60.f });
            if (!m_isUnlocked) icon->setColor({ 100, 100, 100 });
            m_mainLayer->addChild(icon);
        }

        auto nameLabel = CCLabelBMFont::create(songInfo->displayName.c_str(), "goldFont.fnt");
        nameLabel->setScale(0.65f);
        nameLabel->setPosition({ winSize.width / 2, winSize.height - 110.f });
        m_mainLayer->addChild(nameLabel);

        if (!songInfo->creator.empty()) {
            auto creatorLabel = CCLabelBMFont::create(
                fmt::format("by {}", songInfo->creator).c_str(), "chatFont.fnt");
            creatorLabel->setScale(0.4f);
            creatorLabel->setColor({ 180, 180, 180 });
            creatorLabel->setPosition({ winSize.width / 2, winSize.height - 128.f });
            m_mainLayer->addChild(creatorLabel);
        }

        auto descLabel = CCLabelBMFont::create(songInfo->description.c_str(), "chatFont.fnt");
        descLabel->setScale(0.5f);
        float maxDescWidth = winSize.width - 50.f;
        if (descLabel->getContentSize().width * descLabel->getScale() > maxDescWidth) {
            descLabel->setScale(maxDescWidth / descLabel->getContentSize().width);
        }
        descLabel->setPosition({ winSize.width / 2, winSize.height - 152.f });
        m_mainLayer->addChild(descLabel);

        auto playSpr = ButtonSprite::create("Play", "bigFont.fnt", "GJ_button_01.png", 0.8f);
        playSpr->setScale(0.7f);
        auto playBtn = CCMenuItemSpriteExtra::create(playSpr, this, menu_selector(EquipSongPopup::onPlay));
        playBtn->setPosition({ winSize.width / 2 - 55.f, 60.f });

        auto stopSpr = ButtonSprite::create("Stop", "bigFont.fnt", "GJ_button_06.png", 0.8f);
        stopSpr->setScale(0.7f);
        auto stopBtn = CCMenuItemSpriteExtra::create(stopSpr, this, menu_selector(EquipSongPopup::onStop));
        stopBtn->setPosition({ winSize.width / 2 + 55.f, 60.f });

        auto previewMenu = CCMenu::create();
        previewMenu->addChild(playBtn);
        previewMenu->addChild(stopBtn);
        previewMenu->setPosition({ 0, 0 });
        m_mainLayer->addChild(previewMenu);

        if (m_isUnlocked) {
            auto equipSpr = ButtonSprite::create(m_isCurrentlyEquipped ? "Unequip" : "Equip");
            auto equipBtn = CCMenuItemSpriteExtra::create(equipSpr, this, menu_selector(EquipSongPopup::onToggleEquip));
            auto equipMenu = CCMenu::createWithItem(equipBtn);
            equipMenu->setPosition({ winSize.width / 2, 25.f });
            m_mainLayer->addChild(equipMenu);
        }
        else {
            auto lockedLabel = CCLabelBMFont::create("Locked", "goldFont.fnt");
            lockedLabel->setScale(0.5f);
            lockedLabel->setColor({ 150, 150, 150 });
            lockedLabel->setPosition({ winSize.width / 2, 25.f });
            m_mainLayer->addChild(lockedLabel);
        }

        return true;
    }

    void onToggleEquip(CCObject*) {
        if (m_isCurrentlyEquipped) {
            g_streakData.equippedSong.clear();
            FLAlertLayer::create("Success", "Song unequipped!", "OK")->show();
        }
        else {
            g_streakData.equippedSong = m_songID;
            FLAlertLayer::create("Success", "Song equipped!", "OK")->show();
        }

        stopPreview();
        m_needsMusicRestore = false;
        StreakMusic::restartIfActive();

        if (onStateChanged) onStateChanged();
        this->onClose(nullptr);
    }

    void onClose(CCObject* sender) override {
        stopPreview();
        if (m_needsMusicRestore) {
            StreakMusic::restartIfActive();
            m_needsMusicRestore = false;
        }
        Popup::onClose(sender);
    }

public:
    static EquipSongPopup* create(std::string songID) {
        auto ret = new EquipSongPopup();
        if (ret && ret->init(songID)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

class NameCosmeticsScrollbar : public geode::Scrollbar {
protected:
    bool ccTouchBegan(CCTouch* touch, CCEvent* event) override {
        if (!this->getTarget()) return false;

        auto size = this->getContentSize();
        auto local = this->convertToNodeSpace(touch->getLocation());
        constexpr float horizontalPadding = 12.f;
        constexpr float verticalPadding = 4.f;
        CCRect grabArea{
            -horizontalPadding,
            -verticalPadding,
            size.width + horizontalPadding * 2.f,
            size.height + verticalPadding * 2.f
        };
        if (!grabArea.containsPoint(local)) return false;

        geode::Scrollbar::ccTouchMoved(touch, event);
        return true;
    }

public:
    static NameCosmeticsScrollbar* create(CCScrollLayerExt* target) {
        auto ret = new NameCosmeticsScrollbar();
        if (ret && ret->init(target)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};


class RewardsPopup : public Popup {
protected:
    enum DisplayMode { MODE_BADGES, MODE_BANNERS, MODE_NAMES, MODE_SONGS, MODE_PROFILE };
    DisplayMode m_currentMode = MODE_BADGES;
    int m_currentCategory = 0;
    int m_currentProfileCategory = 0;
    QualityNode* m_qualityNode = nullptr;
    CCLabelBMFont* m_categoryLabel = nullptr;  
    CCMenu* m_badgeMenu = nullptr;
    CCNode* m_decorationNode = nullptr;
    CCNode* m_namesContainer = nullptr;
    std::vector<CCNode*> m_nameCategoryContainers;
    std::vector<geode::ScrollLayer*> m_nameCategoryScrolls;
    std::vector<geode::Scrollbar*> m_nameCategoryScrollbars;
    std::vector<std::vector<std::string>> m_nameCategoryItems;
    int m_currentNameCategory = 0;
    CCLabelBMFont* m_namePreviewLabel = nullptr;
    CCMenu* m_catArrowMenu = nullptr;
    geode::ScrollLayer* m_gridScroll = nullptr;
    CCNode* m_gridScrollbar = nullptr;
    CCNode* m_cellNode = nullptr;
    cocos2d::extension::CCScale9Sprite* m_background = nullptr;
    cocos2d::extension::CCScale9Sprite* m_controlsBackground = nullptr;
    CCMenu* m_infoMenu = nullptr;
    CCMenu* m_profilePassInfoMenu = nullptr;
    CCLabelBMFont* m_counterText = nullptr;
    CCLabelBMFont* m_totalStatsLabel = nullptr;
    const float POPUP_WIDTH = 410.f;  
    const float POPUP_HEIGHT = 280.f;
    const float CENTER_X = 205.f;    
    const float CENTER_Y = 140.f;
    std::string m_previewEffect;
    std::string m_previewAnimation;
    std::string m_previewColor;
    std::string m_previewFont;
    std::map<std::string, cocos2d::CCLayerColor*> m_cellBackgrounds;
    std::string m_selectedLockedItem;
    int m_selectedLockedTag = 0;
    CCMenuItemSpriteExtra* m_buyNameBtn = nullptr;
    CCLabelBMFont* m_buyPriceLabel = nullptr;
    CCLabelBMFont* m_eventOnlyLabel = nullptr;
    CCMenuItemSpriteExtra* m_selectedLockedBtn = nullptr;
    CCNode* m_profileEffectPreview = nullptr;
    cocos2d::extension::CCScale9Sprite* m_profilePopupPreview = nullptr;
    CCNode* m_profileDrawPreview = nullptr;
    CCMenu* m_profileViewMenu = nullptr;
    std::vector<std::pair<RemoteAssets::Type, std::string>> m_visibleAssets;
    bool m_waitingForVisibleAssets = false;
    bool m_closeSaveHandled = false;

    struct CosmeticState {
        std::string badge;
        std::string banner;
        std::string song;
        std::string nameColor;
        std::string nameFont;
        std::string nameEffect;
        std::string nameAnimation;
        std::string profileEffect;
        std::string profileCard;
        std::vector<std::string> profileDraws;
    };

    CosmeticState m_initialCosmetics;

    CosmeticState captureCosmeticState() const {
        return {
            g_streakData.equippedBadge,
            g_streakData.equippedBanner,
            g_streakData.equippedSong,
            g_streakData.equippedNameColor,
            g_streakData.equippedNameFont,
            g_streakData.equippedNameEffect,
            g_streakData.equippedNameAnimation,
            g_streakData.equippedProfileEffect,
            g_streakData.equippedProfilePopup,
            g_streakData.equippedProfileDraws
        };
    }

    bool cosmeticsChanged() const {
        auto current = captureCosmeticState();
        return current.badge != m_initialCosmetics.badge ||
            current.banner != m_initialCosmetics.banner ||
            current.song != m_initialCosmetics.song ||
            current.nameColor != m_initialCosmetics.nameColor ||
            current.nameFont != m_initialCosmetics.nameFont ||
            current.nameEffect != m_initialCosmetics.nameEffect ||
            current.nameAnimation != m_initialCosmetics.nameAnimation ||
            current.profileEffect != m_initialCosmetics.profileEffect ||
            current.profileCard != m_initialCosmetics.profileCard ||
            current.profileDraws != m_initialCosmetics.profileDraws;
    }

    RemoteAssets::Type currentAssetType() const {
        if (m_currentMode == MODE_BANNERS) return RemoteAssets::Type::Banner;
        if (m_currentMode == MODE_SONGS) return RemoteAssets::Type::Song;
        return RemoteAssets::Type::Badge;
    }

    void pollVisibleAssets(float) {
        if (!m_waitingForVisibleAssets || m_visibleAssets.empty()) return;
        bool ready = std::ranges::all_of(m_visibleAssets, [](auto const& asset) {
            return RemoteAssets::isInstalled(asset.first, asset.second);
        });
        if (ready) {
            m_waitingForVisibleAssets = false;
            updateCategoryDisplay();
        }
    }

    void updateCellHighlights() {
        
        for (auto const& [key, bg] : m_cellBackgrounds) {
            if (bg) {
                bg->setColor({ 0, 0, 0 });
                bg->setOpacity(40);
            }
        }

        
        auto highlight = [this](int tag, const std::string& item) {
            std::string key = fmt::format("{}_{}", tag, item);
            if (m_cellBackgrounds.count(key) && m_cellBackgrounds[key]) {
                m_cellBackgrounds[key]->setColor({ 255, 255, 255 });
                m_cellBackgrounds[key]->setOpacity(60);  
            }
            };

     
        highlight(1, m_previewEffect);
        highlight(2, m_previewAnimation);
        highlight(3, m_previewColor);
        highlight(4, m_previewFont);
    }

    void onCopyrightInfo(CCObject*) {
        std::string title = "Asset Design Disclaimer";
        std::string desc =
            "Most <cp>Banners</c> are sourced from public resources (e.g. Google Images) and are not owned by the mod creator.\n"
            "However, the majority of <cy>Badges</c> are <cg>original designs</c> made by the mod creator.\n"
            "Special thanks to community members who contributed <cl>exclusive designs</c> for this mod!";
        FLAlertLayer::create(title.c_str(), desc, "OK")->show();
    }

    void onProfilePassInfo(CCObject*) {
        bool active = g_streakData.isStellarPassActive();
        FLAlertLayer::create(
            "Stellar Profile",
            active
                ? "<cg>Stellar Pass is active.</c> You can equip Profile FX, Cards and Draws."
                : "Profile customization is a <cp>Stellar Pass</c> benefit. You may preview every item, but <cy>Use</c> and <cy>Equip</c> require an active pass.",
            "OK"
        )->show();
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
        updateCellHighlights();

        if (g_streakData.isNameItemUnlocked(id)) {
            if (tag == 1) g_streakData.equippedNameEffect = id;
            else if (tag == 2) g_streakData.equippedNameAnimation = id;
            else if (tag == 3) g_streakData.equippedNameColor = id;
            else if (tag == 4) g_streakData.equippedNameFont = id;

            if (m_buyNameBtn) m_buyNameBtn->setVisible(false);
            if (m_eventOnlyLabel) m_eventOnlyLabel->setVisible(false);
        }
        else {
            m_selectedLockedItem = id;
            m_selectedLockedTag = tag;
            m_selectedLockedBtn = btn;

            int price = g_streakData.getNameItemPrice(id);

            if (price <= 0) {
                if (m_buyNameBtn) m_buyNameBtn->setVisible(false);
                if (m_eventOnlyLabel) {
                    m_eventOnlyLabel->setString(
                        StreakData::isPassExclusiveNameItem(id) ? "Stellar Pass required" : "Only obtainable in events"
                    );
                    m_eventOnlyLabel->setVisible(true);
                }
            } else {
                if (m_buyPriceLabel) m_buyPriceLabel->setString(fmt::format("{}", price).c_str());
                if (m_buyNameBtn) m_buyNameBtn->setVisible(true);
                if (m_eventOnlyLabel) m_eventOnlyLabel->setVisible(false);
            }
        }
    }

    void onBuyNameItem(CCObject*) {
        int price = g_streakData.getNameItemPrice(m_selectedLockedItem);

        if (g_streakData.gems < price) {
            FLAlertLayer::create("Oops!", "You don't have enough gems.", "OK")->show();
            return;
        }

        if (m_buyNameBtn) m_buyNameBtn->setEnabled(false);
        matjson::Value payload = matjson::Value::object();
        payload.set("itemID", m_selectedLockedItem);

        std::string itemID = m_selectedLockedItem;
        int tag = m_selectedLockedTag;
        auto btn = m_selectedLockedBtn;

        claimOnServer("/name-item/purchase", payload, [this, itemID, tag, btn, keepAlive = Ref<CCNode>(this)](bool ok) {
            if (m_buyNameBtn) m_buyNameBtn->setEnabled(true);
            if (!ok) {
                FLAlertLayer::create("Error", "Could not complete purchase.", "OK")->show();
                return;
            }

            g_streakData.unlockNameItem(itemID);

            if (tag == 1) g_streakData.equippedNameEffect = itemID;
            else if (tag == 2) g_streakData.equippedNameAnimation = itemID;
            else if (tag == 3) g_streakData.equippedNameColor = itemID;
            else if (tag == 4) g_streakData.equippedNameFont = itemID;

            FMODAudioEngine::sharedEngine()->playEffect("buyItem01.ogg");

            if (btn) {
                if (auto label = static_cast<CCLabelBMFont*>(btn->getNormalImage())) {
                    label->setColor({ 255, 255, 255 });
                    if (auto lock = label->getChildByTag(888)) {
                        lock->removeFromParent();
                    }

                    if (tag == 3) NameModifiers::applyColor(label, itemID);
                    if (tag == 4) NameModifiers::applyFont(label, itemID);
                    if (tag == 1) NameModifiers::applyEffect(label, itemID);
                    if (tag == 2) NameModifiers::applyAnimation(label, itemID);
                }
            }

            if (m_buyNameBtn) m_buyNameBtn->setVisible(false);
            FLAlertLayer::create("Success!", "Cosmetic purchased and equipped.", "OK")->show();
        });
    }

    void updateNamePreview() {
        if (!m_namePreviewLabel) return;
        std::string playerName = GJAccountManager::sharedState()->m_username;
        if (playerName.empty()) playerName = "Player"; 

        m_namePreviewLabel->setString(playerName.c_str());
        m_namePreviewLabel->stopAllActions();
        m_namePreviewLabel->setPosition({ 208.f, 35.f });
        m_namePreviewLabel->setScale(0.8f);
        m_namePreviewLabel->setRotation(0.f);
        m_namePreviewLabel->setSkewX(0.f);
        m_namePreviewLabel->setSkewY(0.f);
        m_namePreviewLabel->setOpacity(255);

        NameModifiers::applyFont(m_namePreviewLabel, m_previewFont);
        NameModifiers::applyColor(m_namePreviewLabel, m_previewColor);

        constexpr float maxPreviewWidth = 84.f;
        constexpr float maxPreviewHeight = 28.f;
        auto previewSize = m_namePreviewLabel->getContentSize();
        if (previewSize.width > 0.f && previewSize.height > 0.f) {
            float fittedScale = std::min({
                0.8f,
                maxPreviewWidth / previewSize.width,
                maxPreviewHeight / previewSize.height
            });
            m_namePreviewLabel->setScale(fittedScale);
        }

        // Animations cache the current scale, so apply them only after fitting
        // the selected font and username inside the preview panel.
        NameModifiers::applyAnimation(m_namePreviewLabel, m_previewAnimation);
        NameModifiers::applyEffect(m_namePreviewLabel, m_previewEffect);
    }

    void updateNameCategoryDisplay() {
        static const char* categoryNames[] = { "Effects", "Animations", "Colors", "Fonts" };

        for (size_t i = 0; i < m_nameCategoryContainers.size(); ++i) {
            bool active = static_cast<int>(i) == m_currentNameCategory;
            if (m_nameCategoryContainers[i]) {
                m_nameCategoryContainers[i]->setVisible(active);
            }
            if (i < m_nameCategoryScrolls.size() && m_nameCategoryScrolls[i]) {
                m_nameCategoryScrolls[i]->setTouchEnabled(active);
                m_nameCategoryScrolls[i]->setMouseEnabled(active);
                m_nameCategoryScrolls[i]->enableScrollWheel(active);
            }
            if (i < m_nameCategoryScrollbars.size() && m_nameCategoryScrollbars[i]) {
                m_nameCategoryScrollbars[i]->setVisible(active && m_currentMode == MODE_NAMES);
                m_nameCategoryScrollbars[i]->setTouchEnabled(active);
                m_nameCategoryScrollbars[i]->setMouseEnabled(active);
            }
        }

        if (m_categoryLabel) {
            m_categoryLabel->setString(categoryNames[m_currentNameCategory]);
            m_categoryLabel->setVisible(true);
            m_categoryLabel->limitLabelWidth(62.f, 0.45f, 0.25f);
        }

        int unlocked = 0;
        int total = 0;
        if (m_currentNameCategory < static_cast<int>(m_nameCategoryItems.size())) {
            const auto& items = m_nameCategoryItems[m_currentNameCategory];
            total = static_cast<int>(items.size());
            for (const auto& item : items) {
                if (g_streakData.isNameItemUnlocked(item)) unlocked++;
            }
        }

        if (m_counterText) {
            m_counterText->setString(fmt::format("Owned: {}/{}", unlocked, total).c_str());
            m_counterText->limitLabelWidth(92.f, 0.26f, 0.16f);
        }
        if (m_totalStatsLabel) {
            m_totalStatsLabel->setString(fmt::format("Category {}/4", m_currentNameCategory + 1).c_str());
            m_totalStatsLabel->limitLabelWidth(92.f, 0.26f, 0.16f);
        }

        if (m_buyNameBtn) m_buyNameBtn->setVisible(false);
        if (m_eventOnlyLabel) m_eventOnlyLabel->setVisible(false);
    }

    void updateProfileEffectsDisplay(bool resetScroll = false) {
        m_badgeMenu->removeAllChildren();
        m_decorationNode->removeAllChildren();
        if (m_cellNode) m_cellNode->removeAllChildren();
        if (m_profilePopupPreview) {
            m_profilePopupPreview->removeFromParentAndCleanup(true);
            m_profilePopupPreview = nullptr;
        }
        if (m_profileDrawPreview) {
            m_profileDrawPreview->removeFromParentAndCleanup(true);
            m_profileDrawPreview = nullptr;
        }

        float gridW = m_background->getContentSize().width;
        float gridH = m_background->getContentSize().height;
        float contentHeight = std::max(
            gridH,
            static_cast<float>(ProfileCardEffects::EFFECTS.size()) * 64.f + 4.f
        );
        m_gridScroll->m_contentLayer->setContentSize({ gridW, contentHeight });
        m_badgeMenu->setContentSize({ gridW, contentHeight });
        m_decorationNode->setContentSize({ gridW, contentHeight });
        if (m_cellNode) m_cellNode->setContentSize({ gridW, contentHeight });

        constexpr float cellHeight = 58.f;
        float currentY = contentHeight - 34.f;
        for (auto const& effect : ProfileCardEffects::EFFECTS) {
            bool equipped = g_streakData.equippedProfileEffect == effect.id;

            auto cell = cocos2d::extension::CCScale9Sprite::create("square02_001.png");
            cell->setContentSize({ gridW - 18.f, cellHeight });
            cell->setPosition({ gridW / 2.f, currentY });
            cell->setColor(equipped ? ccColor3B{ 45, 160, 220 } : ccColor3B{ 0, 0, 0 });
            cell->setOpacity(equipped ? 145 : 75);
            m_cellNode->addChild(cell);

            // Match the profile card's aspect ratio so waves, grids, and
            // diagonal effects keep the same composition in the list.
            auto miniPreview = ProfileCardEffects::createEffect(effect.id, { 64.f, 40.f });
            miniPreview->setPosition({ 8.f, currentY - 20.f });
            m_decorationNode->addChild(miniPreview, 2);

            auto title = CCLabelBMFont::create(effect.name, "goldFont.fnt");
            title->setScale(0.38f);
            title->setAnchorPoint({ 0.f, 0.5f });
            title->setPosition({ 76.f, currentY + 10.f });
            title->limitLabelWidth(135.f, 0.38f, 0.24f);
            m_cellNode->addChild(title, 3);

            if (g_streakData.isStellarPassActive()) {
                auto useSprite = ButtonSprite::create(
                    "Use", 42, true, "bigFont.fnt",
                    equipped ? "GJ_button_02.png" : "GJ_button_01.png",
                    18, 0.38f
                );
                auto button = CCMenuItemSpriteExtra::create(
                    useSprite,
                    this,
                    menu_selector(RewardsPopup::onProfileEffectClicked)
                );
                button->setUserObject("profile-effect-id"_spr, CCString::create(effect.id));
                button->setPosition({ 107.f, currentY - 11.f });
                m_badgeMenu->addChild(button, 5);
            }

            if (equipped) {
                auto check = CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
                check->setScale(0.55f);
                check->setPosition({ gridW - 20.f, currentY + 15.f });
                m_decorationNode->addChild(check, 6);
            }

            currentY -= 64.f;
        }

        if (m_profileEffectPreview) {
            m_profileEffectPreview->removeFromParentAndCleanup(true);
            m_profileEffectPreview = nullptr;
        }
        if (g_streakData.equippedProfileEffect != "None") {
            m_profileEffectPreview = ProfileCardEffects::createEffect(
                g_streakData.equippedProfileEffect,
                { 82.f, 52.f }
            );
            m_profileEffectPreview->setPosition({ 300.f, 127.f });
            m_mainLayer->addChild(m_profileEffectPreview, 20);
        }

        if (m_categoryLabel) {
            m_categoryLabel->setString("Profile FX");
            m_categoryLabel->limitLabelWidth(86.f, 0.45f, 0.28f);
            m_categoryLabel->setVisible(true);
        }
        if (m_counterText) {
            m_counterText->setString(fmt::format("{} effects", ProfileCardEffects::EFFECTS.size()).c_str());
            m_counterText->setVisible(true);
        }
        if (m_totalStatsLabel) {
            auto info = ProfileCardEffects::getInfo(g_streakData.equippedProfileEffect);
            m_totalStatsLabel->setString(info ? info->name : "None equipped");
            m_totalStatsLabel->limitLabelWidth(88.f, 0.26f, 0.17f);
            m_totalStatsLabel->setVisible(true);
        }
        if (m_infoMenu) m_infoMenu->setVisible(false);
        if (m_profileViewMenu) m_profileViewMenu->setVisible(true);
        if (m_gridScrollbar) m_gridScrollbar->setVisible(true);
        m_gridScroll->setTouchEnabled(true);
        m_gridScroll->setMouseEnabled(true);
        m_gridScroll->enableScrollWheel(true);
        if (resetScroll) m_gridScroll->scrollToTop();
    }

    void updateProfilePopupDisplay(bool resetScroll = false) {
        m_badgeMenu->removeAllChildren();
        m_decorationNode->removeAllChildren();
        if (m_cellNode) m_cellNode->removeAllChildren();

        if (m_profileEffectPreview) {
            m_profileEffectPreview->removeFromParentAndCleanup(true);
            m_profileEffectPreview = nullptr;
        }
        if (m_profilePopupPreview) {
            m_profilePopupPreview->removeFromParentAndCleanup(true);
            m_profilePopupPreview = nullptr;
        }
        if (m_profileDrawPreview) {
            m_profileDrawPreview->removeFromParentAndCleanup(true);
            m_profileDrawPreview = nullptr;
        }

        float gridW = m_background->getContentSize().width;
        float gridH = m_background->getContentSize().height;
        float contentHeight = std::max(
            gridH,
            static_cast<float>(ProfileCardStyles::STYLES.size()) * 64.f + 4.f
        );
        m_gridScroll->m_contentLayer->setContentSize({ gridW, contentHeight });
        m_badgeMenu->setContentSize({ gridW, contentHeight });
        m_decorationNode->setContentSize({ gridW, contentHeight });
        if (m_cellNode) m_cellNode->setContentSize({ gridW, contentHeight });

        constexpr float cellHeight = 58.f;
        float currentY = contentHeight - 34.f;
        for (auto const& style : ProfileCardStyles::STYLES) {
            bool equipped = g_streakData.equippedProfilePopup == style.id;

            auto cell = cocos2d::extension::CCScale9Sprite::create("square02_001.png");
            cell->setContentSize({ gridW - 18.f, cellHeight });
            cell->setPosition({ gridW / 2.f, currentY });
            cell->setColor(equipped ? ccColor3B{ 45, 160, 220 } : ccColor3B{ 0, 0, 0 });
            cell->setOpacity(equipped ? 145 : 75);
            m_cellNode->addChild(cell);

            auto preview = cocos2d::extension::CCScale9Sprite::create(style.id);
            if (preview) {
                preview->setContentSize({ 64.f, 40.f });
                preview->setPosition({ 40.f, currentY });
                m_decorationNode->addChild(preview, 2);
            }

            auto title = CCLabelBMFont::create(style.name, "goldFont.fnt");
            title->setScale(0.38f);
            title->setAnchorPoint({ 0.f, 0.5f });
            title->setPosition({ 76.f, currentY + 10.f });
            title->limitLabelWidth(135.f, 0.38f, 0.24f);
            m_cellNode->addChild(title, 3);

            if (g_streakData.isStellarPassActive()) {
                auto useSprite = ButtonSprite::create(
                    "Use", 42, true, "bigFont.fnt",
                    equipped ? "GJ_button_02.png" : "GJ_button_01.png",
                    18, 0.38f
                );
                auto button = CCMenuItemSpriteExtra::create(
                    useSprite,
                    this,
                    menu_selector(RewardsPopup::onProfilePopupClicked)
                );
                button->setUserObject("profile-popup-id"_spr, CCString::create(style.id));
                button->setPosition({ 107.f, currentY - 11.f });
                m_badgeMenu->addChild(button, 5);
            }

            if (equipped) {
                auto check = CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
                check->setScale(0.55f);
                check->setPosition({ gridW - 20.f, currentY + 15.f });
                m_decorationNode->addChild(check, 6);
            }

            currentY -= 64.f;
        }

        m_profilePopupPreview = cocos2d::extension::CCScale9Sprite::create(
            ProfileCardStyles::resolve(g_streakData.equippedProfilePopup)
        );
        if (m_profilePopupPreview) {
            m_profilePopupPreview->setContentSize({ 82.f, 52.f });
            m_profilePopupPreview->setPosition({ 342.f, 153.f });
            m_mainLayer->addChild(m_profilePopupPreview, 20);
        }

        if (m_categoryLabel) {
            m_categoryLabel->setString("Card");
            m_categoryLabel->limitLabelWidth(86.f, 0.45f, 0.25f);
            m_categoryLabel->setVisible(true);
        }
        if (m_counterText) {
            m_counterText->setString(fmt::format("{} cards", ProfileCardStyles::STYLES.size()).c_str());
            m_counterText->setVisible(true);
        }
        if (m_totalStatsLabel) {
            auto info = ProfileCardStyles::getInfo(g_streakData.equippedProfilePopup);
            m_totalStatsLabel->setString(info ? info->name : "Default");
            m_totalStatsLabel->limitLabelWidth(88.f, 0.26f, 0.17f);
            m_totalStatsLabel->setVisible(true);
        }
        if (m_infoMenu) m_infoMenu->setVisible(false);
        if (m_profileViewMenu) m_profileViewMenu->setVisible(true);
        if (m_gridScrollbar) m_gridScrollbar->setVisible(true);
        m_gridScroll->setTouchEnabled(true);
        m_gridScroll->setMouseEnabled(true);
        m_gridScroll->enableScrollWheel(true);
        if (resetScroll) m_gridScroll->scrollToTop();
    }

    CCNode* createProfileDrawNode(const ProfileCardDraws::DrawInfo& draw, const CCSize& size, float extraScale = 1.f) {
        auto sprite = CCSprite::create(ProfileCardDraws::resolveSprite(draw).c_str());
        if (!sprite || sprite->getContentSize().width <= 0.f || sprite->getContentSize().height <= 0.f) return nullptr;
        auto node = CCNode::create();
        node->setContentSize(size);
        float layoutScale = size.height / 180.f;
        sprite->setScale((draw.displayHeight * layoutScale / sprite->getContentSize().height) * extraScale);
        sprite->setPosition({
            draw.centerX * (size.width / 280.f),
            draw.centerY * layoutScale
        });
        node->addChild(sprite);
        return node;
    }

    void updateProfileDrawsDisplay(bool resetScroll = false) {
        m_badgeMenu->removeAllChildren();
        m_decorationNode->removeAllChildren();
        if (m_cellNode) m_cellNode->removeAllChildren();

        if (m_profileEffectPreview) {
            m_profileEffectPreview->removeFromParentAndCleanup(true);
            m_profileEffectPreview = nullptr;
        }
        if (m_profilePopupPreview) {
            m_profilePopupPreview->removeFromParentAndCleanup(true);
            m_profilePopupPreview = nullptr;
        }
        if (m_profileDrawPreview) {
            m_profileDrawPreview->removeFromParentAndCleanup(true);
            m_profileDrawPreview = nullptr;
        }

        float gridW = m_background->getContentSize().width;
        float gridH = m_background->getContentSize().height;
        float contentHeight = std::max(gridH, static_cast<float>(ProfileCardDraws::DRAWS.size()) * 64.f + 4.f);
        m_gridScroll->m_contentLayer->setContentSize({ gridW, contentHeight });
        m_badgeMenu->setContentSize({ gridW, contentHeight });
        m_decorationNode->setContentSize({ gridW, contentHeight });
        if (m_cellNode) m_cellNode->setContentSize({ gridW, contentHeight });

        constexpr float cellHeight = 58.f;
        float currentY = contentHeight - 34.f;
        for (auto const& draw : ProfileCardDraws::DRAWS) {
            bool equipped = std::find(
                g_streakData.equippedProfileDraws.begin(),
                g_streakData.equippedProfileDraws.end(),
                draw.id
            ) != g_streakData.equippedProfileDraws.end();

            auto cell = cocos2d::extension::CCScale9Sprite::create("square02_001.png");
            cell->setContentSize({ gridW - 18.f, cellHeight });
            cell->setPosition({ gridW / 2.f, currentY });
            cell->setColor(equipped ? ccColor3B{ 45, 160, 220 } : ccColor3B{ 0, 0, 0 });
            cell->setOpacity(equipped ? 145 : 75);
            m_cellNode->addChild(cell);

            if (auto preview = createProfileDrawNode(draw, { 64.f, 40.f })) {
                preview->setPosition({ 8.f, currentY - 20.f });
                m_decorationNode->addChild(preview, 2);
            }

            auto title = CCLabelBMFont::create(draw.name, "goldFont.fnt");
            title->setScale(0.36f);
            title->setAnchorPoint({ 0.f, 0.5f });
            title->setPosition({ 76.f, currentY + 11.f });
            title->limitLabelWidth(118.f, 0.36f, 0.23f);
            m_cellNode->addChild(title, 3);

            auto typeLabel = CCLabelBMFont::create(
                draw.type == ProfileCardDraws::DrawType::Masked ? "Masked" : "Overlay",
                "chatFont.fnt"
            );
            typeLabel->setScale(0.42f);
            typeLabel->setAnchorPoint({ 0.f, 0.5f });
            typeLabel->setColor(draw.type == ProfileCardDraws::DrawType::Masked
                ? ccColor3B{ 90, 220, 255 }
                : ccColor3B{ 255, 205, 80 });
            typeLabel->setPosition({ 76.f, currentY - 11.f });
            m_cellNode->addChild(typeLabel, 3);

            if (g_streakData.isStellarPassActive()) {
                auto useSprite = ButtonSprite::create(
                    equipped ? "Remove" : "Use", 54, true, "bigFont.fnt",
                    equipped ? "GJ_button_06.png" : "GJ_button_01.png",
                    18, 0.34f
                );
                auto button = CCMenuItemSpriteExtra::create(
                    useSprite,
                    this,
                    menu_selector(RewardsPopup::onProfileDrawClicked)
                );
                button->setUserObject("profile-draw-id"_spr, CCString::create(draw.id));
                button->setPosition({ 180.f, currentY - 10.f });
                m_badgeMenu->addChild(button, 5);
            }

            if (equipped) {
                auto check = CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
                check->setScale(0.55f);
                check->setPosition({ gridW - 20.f, currentY + 15.f });
                m_decorationNode->addChild(check, 6);
            }
            currentY -= 64.f;
        }

        m_profileDrawPreview = CCNode::create();
        m_profileDrawPreview->setContentSize({ 82.f, 52.f });
        m_profileDrawPreview->setPosition({ 301.f, 127.f });

        auto clippedPreview = CCClippingNode::create();
        clippedPreview->setContentSize({ 82.f, 52.f });
        clippedPreview->setAlphaThreshold(0.08f);
        auto previewStencil = cocos2d::extension::CCScale9Sprite::create(
            ProfileCardStyles::resolve(g_streakData.equippedProfilePopup)
        );
        previewStencil->setContentSize({ 80.f, 50.f });
        previewStencil->setPosition({ 41.f, 26.f });
        clippedPreview->setStencil(previewStencil);
        m_profileDrawPreview->addChild(clippedPreview, 1);

        for (auto const& drawID : g_streakData.equippedProfileDraws) {
            auto info = ProfileCardDraws::getInfo(drawID);
            if (!info) continue;
            if (auto drawNode = createProfileDrawNode(*info, { 82.f, 52.f }, info->type == ProfileCardDraws::DrawType::Overlay ? 1.04f : 1.f)) {
                if (info->type == ProfileCardDraws::DrawType::Masked) clippedPreview->addChild(drawNode);
                else m_profileDrawPreview->addChild(drawNode, 2);
            }
        }
        m_mainLayer->addChild(m_profileDrawPreview, 20);

        if (m_categoryLabel) {
            m_categoryLabel->setString("Draws");
            m_categoryLabel->limitLabelWidth(86.f, 0.45f, 0.28f);
            m_categoryLabel->setVisible(true);
        }
        if (m_counterText) {
            m_counterText->setString(fmt::format("Equipped: {}/{}", g_streakData.equippedProfileDraws.size(), ProfileCardDraws::MAX_EQUIPPED).c_str());
            m_counterText->limitLabelWidth(92.f, 0.26f, 0.16f);
            m_counterText->setVisible(true);
        }
        if (m_totalStatsLabel) {
            m_totalStatsLabel->setString("5 Masked / 0 Overlay");
            m_totalStatsLabel->limitLabelWidth(92.f, 0.26f, 0.16f);
            m_totalStatsLabel->setVisible(true);
        }
        if (m_infoMenu) m_infoMenu->setVisible(false);
        if (m_profileViewMenu) m_profileViewMenu->setVisible(true);
        if (m_gridScrollbar) m_gridScrollbar->setVisible(true);
        m_gridScroll->setTouchEnabled(true);
        m_gridScroll->setMouseEnabled(true);
        m_gridScroll->enableScrollWheel(true);
        if (resetScroll) m_gridScroll->scrollToTop();
    }

    void onProfileEffectClicked(CCObject* sender) {
        if (!g_streakData.isStellarPassActive()) return;
        auto node = static_cast<CCNode*>(sender);
        auto value = static_cast<CCString*>(node->getUserObject("profile-effect-id"_spr));
        if (!value) return;

        std::string selected = value->getCString();
        g_streakData.equippedProfileEffect =
            g_streakData.equippedProfileEffect == selected ? "None" : selected;
        updateProfileEffectsDisplay();
        FMODAudioEngine::sharedEngine()->playEffect("clickSound.ogg");
    }

    void onProfilePopupClicked(CCObject* sender) {
        if (!g_streakData.isStellarPassActive()) return;
        auto node = static_cast<CCNode*>(sender);
        auto value = static_cast<CCString*>(node->getUserObject("profile-popup-id"_spr));
        if (!value) return;

        g_streakData.equippedProfilePopup = value->getCString();
        updateProfilePopupDisplay();
        FMODAudioEngine::sharedEngine()->playEffect("clickSound.ogg");
    }

    void onProfileDrawClicked(CCObject* sender) {
        if (!g_streakData.isStellarPassActive()) return;
        auto node = static_cast<CCNode*>(sender);
        auto value = static_cast<CCString*>(node->getUserObject("profile-draw-id"_spr));
        if (!value) return;

        std::string selected = value->getCString();
        auto& equipped = g_streakData.equippedProfileDraws;
        auto it = std::find(equipped.begin(), equipped.end(), selected);
        if (it != equipped.end()) {
            equipped.erase(it);
        }
        else {
            if (equipped.size() >= ProfileCardDraws::MAX_EQUIPPED) {
                Notification::create("You can equip up to 5 draws", NotificationIcon::Warning)->show();
                return;
            }
            equipped.push_back(selected);
        }

        updateProfileDrawsDisplay();
        FMODAudioEngine::sharedEngine()->playEffect("clickSound.ogg");
    }

    void onViewProfileEffect(CCObject*) {
        ProfileData data;
        auto account = GJAccountManager::sharedState();
        data.accountID = account ? account->m_accountID : 0;
        data.username = account && !account->m_username.empty() ? account->m_username : "Player";
        data.currentStreak = g_streakData.currentStreak;
        data.totalSP = g_streakData.totalStreakPoints;
        data.badgeID = g_streakData.equippedBadge;
        data.level = g_streakData.currentLevel;
        data.currentXP = g_streakData.currentXP;
        data.streakTokens = g_streakData.streakTokens;
        data.superStars = g_streakData.superStars;
        data.starTickets = g_streakData.starTickets;
        data.gems = g_streakData.gems;
        data.bannerID = g_streakData.equippedBanner;
        data.streakID = g_streakData.streakID;
        data.globalRank = g_streakData.globalRank;
        data.nameColor = g_streakData.equippedNameColor;
        data.nameFont = g_streakData.equippedNameFont;
        data.nameEffect = g_streakData.equippedNameEffect;
        data.nameAnimation = g_streakData.equippedNameAnimation;
        data.profileEffect = g_streakData.equippedProfileEffect;
        data.profilePopup = g_streakData.equippedProfilePopup;
        data.profileDraws = g_streakData.equippedProfileDraws;

        ProfileCardPopup::create(data)->show();
    }

    bool init() override {
        if (!Popup::init(POPUP_WIDTH, POPUP_HEIGHT, "geode.loader/GE_square03.png")) return false;

        this->setTitle("Cosmetics");
        g_streakData.load();
        m_initialCosmetics = captureCosmeticState();

        auto badgeBtnSpr = CCSprite::create("badges_btn_c.png"_spr);
        if (!badgeBtnSpr) badgeBtnSpr = ButtonSprite::create("Badges");
        badgeBtnSpr->setScale(0.25f);
        auto badgeBtn = CCMenuItemSpriteExtra::create(badgeBtnSpr,
            this,
            menu_selector(RewardsPopup::onSwitchToBadges));

        auto bannerBtnSpr = CCSprite::create("banners_btn_c.png"_spr);
        if (!bannerBtnSpr) bannerBtnSpr = ButtonSprite::create("Banners");
        bannerBtnSpr->setScale(0.25f);
        auto bannerBtn = CCMenuItemSpriteExtra::create(bannerBtnSpr,
            this,
            menu_selector(RewardsPopup::onSwitchToBanners));

        auto nameBtnSpr = CCSprite::create("name_btn_c.png"_spr);
        if (!nameBtnSpr) nameBtnSpr = ButtonSprite::create("Name");
        nameBtnSpr->setScale(0.25f);
        auto nameBtn = CCMenuItemSpriteExtra::create(nameBtnSpr,
            this,
            menu_selector(RewardsPopup::onSwitchToNames));

        auto songBtnSpr = CCSprite::create("song_btn.png"_spr);
        if (!songBtnSpr) songBtnSpr = ButtonSprite::create("Songs");
        songBtnSpr->setScale(0.25f);
        auto songBtn = CCMenuItemSpriteExtra::create(songBtnSpr,
            this,
            menu_selector(RewardsPopup::onSwitchToSongs));

        auto profileBtnSpr = CCSprite::create("perfil_animation_btn.png"_spr);
        if (!profileBtnSpr) profileBtnSpr = ButtonSprite::create("Profile FX");
        profileBtnSpr->setScale(0.25f);
        auto profileBtn = CCMenuItemSpriteExtra::create(profileBtnSpr,
            this,
            menu_selector(RewardsPopup::onSwitchToProfile));

        auto leftMenu = CCMenu::create();
        leftMenu->addChild(badgeBtn);
        leftMenu->addChild(bannerBtn);
        leftMenu->addChild(nameBtn);
        leftMenu->addChild(songBtn);
        leftMenu->addChild(profileBtn);
        leftMenu->alignItemsVerticallyWithPadding(4.f);
        leftMenu->setPosition({ -20.f, CENTER_Y + 42.f });
        m_mainLayer->addChild(leftMenu);

        m_background = cocos2d::extension::CCScale9Sprite::create("square02_001.png");
        m_background->setColor({ 0, 0, 0 });
        m_background->setOpacity(120);
        m_background->setContentSize({ 235.f, 205.f });
        m_background->setPosition({ 134.f, 125.f });
        m_mainLayer->addChild(m_background);

        m_controlsBackground = cocos2d::extension::CCScale9Sprite::create("square02_001.png");
        m_controlsBackground->setColor({ 0, 0, 0 });
        m_controlsBackground->setOpacity(120);
        m_controlsBackground->setContentSize({ 101.f, 205.f });
        m_controlsBackground->setPosition({ 342.f, 125.f });
        m_mainLayer->addChild(m_controlsBackground);

        float categoryY = 201.f;
        auto catLeftArrow = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        catLeftArrow->setScale(0.6f);
        auto catLeftBtn = CCMenuItemSpriteExtra::create(catLeftArrow,
            this,
            menu_selector(RewardsPopup::onPreviousCategory));
        catLeftBtn->setPosition(-44.f, 0);

        auto catRightArrow = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        catRightArrow->setFlipX(true);
        catRightArrow->setScale(0.6f);
        auto catRightBtn = CCMenuItemSpriteExtra::create(catRightArrow,
            this,
            menu_selector(RewardsPopup::onNextCategory));
        catRightBtn->setPosition(44.f, 0);

        m_catArrowMenu = CCMenu::create();
        m_catArrowMenu->addChild(catLeftBtn);
        m_catArrowMenu->addChild(catRightBtn);
        m_catArrowMenu->setPosition({ 342.f, categoryY });
        m_mainLayer->addChild(m_catArrowMenu);
        m_qualityNode = QualityNode::create();
        m_qualityNode->setScale(0.72f);
        m_qualityNode->setPosition({ 342.f, 201.f });
        m_mainLayer->addChild(m_qualityNode, 9);
        m_categoryLabel = CCLabelBMFont::create("", "goldFont.fnt");
        m_categoryLabel->setScale(0.45f);
        m_categoryLabel->setPosition({ 342.f, 201.f });
        m_categoryLabel->setVisible(false);
        m_mainLayer->addChild(m_categoryLabel, 10);

        float gridW = m_background->getContentSize().width;
        float gridH = m_background->getContentSize().height;
        m_gridScroll = geode::ScrollLayer::create({ gridW, gridH });
        m_gridScroll->setPosition({
            m_background->getPositionX() - gridW / 2.f,
            m_background->getPositionY() - gridH / 2.f
            });
        m_mainLayer->addChild(m_gridScroll, 14);

        m_cellNode = CCNode::create();
        m_cellNode->setPosition({ 0, 0 });
        m_gridScroll->m_contentLayer->addChild(m_cellNode, 1);

        m_badgeMenu = CCMenu::create();
        m_badgeMenu->setPosition({ 0, 0 });
        m_gridScroll->m_contentLayer->addChild(m_badgeMenu, 2);

        m_decorationNode = CCNode::create();
        m_decorationNode->setPosition({ 0, 0 });
        m_gridScroll->m_contentLayer->addChild(m_decorationNode, 3);

        addScrollbar(m_gridScroll, 6.f, m_mainLayer);
        m_gridScrollbar = static_cast<CCNode*>(m_gridScroll->getUserObject("scrollbar"_spr));

        m_counterText = CCLabelBMFont::create("", "bigFont.fnt");
        m_counterText->setScale(0.26f);
        m_counterText->setPosition({ 342.f, 112.f });
        m_mainLayer->addChild(m_counterText);

        auto infoSpr = CCSprite::create("info_btn.png"_spr);
        infoSpr->setScale(0.25f);
        auto infoBtn = CCMenuItemSpriteExtra::create(infoSpr,
            this,
            menu_selector(RewardsPopup::onCopyrightInfo));
        m_infoMenu = CCMenu::createWithItem(infoBtn);
        m_infoMenu->setPosition({ 342.f, 42.f });
        m_mainLayer->addChild(m_infoMenu);

        auto profileInfoSprite = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
        profileInfoSprite->setScale(0.65f);
        auto profileInfoButton = CCMenuItemSpriteExtra::create(
            profileInfoSprite,
            this,
            menu_selector(RewardsPopup::onProfilePassInfo)
        );
        m_profilePassInfoMenu = CCMenu::createWithItem(profileInfoButton);
        m_profilePassInfoMenu->setPosition({ POPUP_WIDTH - 20.f, POPUP_HEIGHT - 22.f });
        m_profilePassInfoMenu->setVisible(false);
        m_mainLayer->addChild(m_profilePassInfoMenu, 30);

        auto viewSprite = ButtonSprite::create("View", "bigFont.fnt", "GJ_button_01.png", 0.75f);
        viewSprite->setScale(0.72f);
        auto viewButton = CCMenuItemSpriteExtra::create(
            viewSprite,
            this,
            menu_selector(RewardsPopup::onViewProfileEffect)
        );
        m_profileViewMenu = CCMenu::createWithItem(viewButton);
        m_profileViewMenu->setPosition({ 342.f, 43.f });
        m_profileViewMenu->setVisible(false);
        m_mainLayer->addChild(m_profileViewMenu, 21);

        m_totalStatsLabel = CCLabelBMFont::create("Total 0/0", "goldFont.fnt");
        m_totalStatsLabel->setScale(0.26f);
        m_totalStatsLabel->setPosition({ 342.f, 78.f });
        m_mainLayer->addChild(m_totalStatsLabel);

        m_namesContainer = CCNode::create();
        m_namesContainer->setPosition({ -9999.f, -9999.f });
        m_namesContainer->setVisible(false);
        m_mainLayer->addChild(m_namesContainer, 5);
        m_badgeMenu->setZOrder(2);
        m_decorationNode->setZOrder(3);
        m_catArrowMenu->setZOrder(15);

        m_previewEffect = g_streakData.equippedNameEffect;
        m_previewAnimation = g_streakData.equippedNameAnimation;
        m_previewColor = g_streakData.equippedNameColor;
        m_previewFont = g_streakData.equippedNameFont;

        std::vector<std::string> effects = {
        "None", "Blood", "Bubbles", "ConcertLights", "Confetti", "Electric",
        "Ember", "Explosion", "Fire", "Fireflies", "Galaxy", "Glitch",
        "Heartbeat", "Holy", "Ice", "Lava", "Matrix", "Meteor", "Career",
        "Orbit", "Plasma", "Poison", "Pulse", "Rain", "Rainbow", "Sakura",
        "Scanner", "Shadow", "Shockwave", "Smoke", "Snow", "Sparkle",
        "Spotlight", "Stars", "Supernova", "Toxic", "Void"
        };
        for (auto item : PassNameCosmetics::EFFECTS) effects.emplace_back(item);

        std::vector<std::string> animations = {
              "None", "Blink", "Bounce", "Domino", "DVD", "Dynamic Jump", "Float",
              "Glitch", "Heartbeat", "Jelly", "Pulse", "Shake", "Spin",
              "Spiral", "Squish", "Swing", "Tremble", "Wave", "Wobble"
        };
        for (auto item : PassNameCosmetics::ANIMATIONS) animations.emplace_back(item);
        std::vector<std::string> colors = {
                "Default", "Black", "Blue", "Brown", "Cyan", "Gold", "Green",
                "Lime", "Magenta", "Maroon", "Mint", "Navy", "Orange",
                "Peach", "Pink", "Purple", "Red", "Silver", "Teal", "Yellow",
                "Crazy Wave",
                "Cyberpunk Wave", "Fire Wave", "Galaxy Wave", "Limbo Fracture", "Golden Wave", "Ice Wave", "Ocean Wave",
                "Rainbow", "Rainbow Wave", "Royal Wave", "Sunset Wave", "Toxic Wave",
                "Abyss Wave","Disco Blink", "Synthwave", "Pastel Wave", "Aurora Wave",
                "Static Blood", "Static Deep Sea", "Static Toxic", "Static Vaporwave"
        };
        for (auto item : PassNameCosmetics::COLORS) colors.emplace_back(item);

        std::vector<std::string> fonts = {
            "Default", "Chat", "Gold", "Pusab",
            "Font1", "Font2", "Font3", "Font4", "Font5", "Font6",
            "Font7", "Font8", "Font9", "Font10",
            "Font11", "Font12", "Font13", "Font14", "Font15", "Font16",
            "Font17", "Font18", "Font19", "Font20", "Font21", "Font22",
            "Font23", "Font24", "Font25", "Font26", "Font27", "Font28",
            "Font29", "Font30", "Font31", "Font32", "Font33", "Font34",
            "Font35", "Font36", "Font37", "Font38", "Font39", "Font40",
            "Font41", "Font42", "Font43", "Font44", "Font45", "Font46",
            "Font47", "Font48", "Font49", "Font50", "Font51", "Font52",
            "Font53", "Font54", "Font55", "Font56", "Font57", "Font58",
            "Font59"
        };
        m_nameCategoryItems = { effects, animations, colors, fonts };

        auto createNameCategory = [this](const std::vector<std::string>& items, int tag) {
            float listWidth = 215.f;
            float listHeight = 185.f;
            float itemHeight = 32.f;
            float totalHeight = items.size() * itemHeight;
            if (totalHeight < listHeight) totalHeight = listHeight;

            auto categoryContainer = CCNode::create();
            categoryContainer->setVisible(tag == 1);
            m_namesContainer->addChild(categoryContainer);
            m_nameCategoryContainers.push_back(categoryContainer);

            auto listBg = cocos2d::extension::CCScale9Sprite::create("square02_001.png");
            listBg->setContentSize({ listWidth, listHeight });
            listBg->setPosition({ 0.f, 0.f });
            listBg->setColor({ 0, 0, 0 });
            listBg->setOpacity(45);
            categoryContainer->addChild(listBg);

            auto scrollLayer = geode::ScrollLayer::create({ listWidth, listHeight });
            scrollLayer->setPosition({ -listWidth / 2.f, -listHeight / 2.f });
            scrollLayer->setTouchEnabled(false);
            scrollLayer->setMouseEnabled(false);
            scrollLayer->enableScrollWheel(false);
            scrollLayer->setStealingTouches(true);
            m_nameCategoryScrolls.push_back(scrollLayer);

            auto menu = CCMenu::create();
            menu->setContentSize({ listWidth, totalHeight });
            menu->setPosition({ 0, 0 });

            float currentY = totalHeight - (itemHeight / 2.f);
            for (const auto& item : items) {
                auto cellBg = CCLayerColor::create({ 0, 0, 0, 40 }, listWidth, itemHeight - 2.f);
                cellBg->setPosition({ 0.f, currentY - (itemHeight - 2.f) / 2.f });
                scrollLayer->m_contentLayer->addChild(cellBg);
                m_cellBackgrounds[fmt::format("{}_{}", tag, item)] = cellBg;

                auto label = CCLabelBMFont::create(item.c_str(), "chatFont.fnt");
                label->setScale(0.65f);

                if (tag == 3) NameModifiers::applyColor(label, item);
                if (tag == 4) NameModifiers::applyFont(label, item);

                bool isPassExclusive = StreakData::isPassExclusiveNameItem(item);
                float maxLabelWidth = isPassExclusive ? listWidth - 52.f : listWidth - 6.f;
                if (label->getContentSize().width * label->getScale() > maxLabelWidth) {
                    label->setScale(maxLabelWidth / label->getContentSize().width);
                }

                if (isPassExclusive) {
                    auto passIcon = CCSprite::create("stellarpass.png"_spr);
                    if (!passIcon) passIcon = CCSprite::create("stellarpass.png");
                    if (passIcon) {
                        constexpr float targetSize = 19.f;
                        float maxSide = std::max(passIcon->getContentSize().width, passIcon->getContentSize().height);
                        if (maxSide > 0.f) passIcon->setScale(targetSize / maxSide);
                        passIcon->setPosition({ 16.f, currentY });
                        scrollLayer->m_contentLayer->addChild(passIcon, 4);
                    }
                }

                if (!g_streakData.isNameItemUnlocked(item)) {
                    label->setColor({ 150, 150, 150 });
                    auto lockIcon = CCSprite::createWithSpriteFrameName("GJ_lock_001.png");
                    lockIcon->setScale(0.5f);
                    lockIcon->setPosition({ -10.f, label->getContentSize().height / 2.f });
                    lockIcon->setTag(888);
                    label->addChild(lockIcon);
                }

                auto btn = CCMenuItemSpriteExtra::create(label,
                    this,
                    menu_selector(RewardsPopup::onNameOptionClicked));
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
            categoryContainer->addChild(scrollLayer);
            auto scrollbar = NameCosmeticsScrollbar::create(scrollLayer);
            if (scrollbar) {
                scrollbar->setAnchorPoint({ 0.f, 0.f });
                scrollbar->ignoreAnchorPointForPosition(true);
                scrollbar->setPosition({
                    134.f + scrollLayer->getPositionX() + listWidth + 15.f,
                    125.f + scrollLayer->getPositionY()
                });
                scrollbar->setZOrder(100);
                scrollbar->setVisible(false);
                scrollbar->setTouchEnabled(false);
                scrollbar->setMouseEnabled(false);
                m_mainLayer->addChild(scrollbar);
            }
            m_nameCategoryScrollbars.push_back(scrollbar);
            };

        createNameCategory(effects, 1);
        createNameCategory(animations, 2);
        createNameCategory(colors, 3);
        createNameCategory(fonts, 4);
        updateCellHighlights();
        m_namePreviewLabel = CCLabelBMFont::create("Player", "bigFont.fnt");
        m_namePreviewLabel->setPosition({ 208.f, 35.f });
        m_namePreviewLabel->setScale(0.65f);
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

        m_buyNameBtn = CCMenuItemSpriteExtra::create(buyBtnSprite,
            this,
            menu_selector(RewardsPopup::onBuyNameItem));
        m_buyNameBtn->setVisible(false);

        auto buyMenu = CCMenu::create();
        buyMenu->addChild(m_buyNameBtn);
        buyMenu->setPosition({ 208.f, -77.f });
        m_namesContainer->addChild(buyMenu);

        m_eventOnlyLabel = CCLabelBMFont::create("Only obtainable in events", "goldFont.fnt");
        m_eventOnlyLabel->setScale(0.3f);
        m_eventOnlyLabel->setPosition({ 208.f, -77.f });
        m_eventOnlyLabel->limitLabelWidth(92.f, 0.3f, 0.16f);
        m_eventOnlyLabel->setVisible(false);
        m_namesContainer->addChild(m_eventOnlyLabel);
        updateCategoryDisplay();
        this->schedule(schedule_selector(RewardsPopup::pollVisibleAssets), 0.5f);
        this->updateNamePreview();
        this->updateCellHighlights();
        return true;
    }

    void toggleUIVisibility(bool isNames) {
        m_namesContainer->setVisible(isNames);

        if (m_currentMode != MODE_PROFILE && m_profileEffectPreview) {
            m_profileEffectPreview->removeFromParentAndCleanup(true);
            m_profileEffectPreview = nullptr;
        }
        if (m_currentMode != MODE_PROFILE && m_profilePopupPreview) {
            m_profilePopupPreview->removeFromParentAndCleanup(true);
            m_profilePopupPreview = nullptr;
        }
        if (m_currentMode != MODE_PROFILE && m_profileDrawPreview) {
            m_profileDrawPreview->removeFromParentAndCleanup(true);
            m_profileDrawPreview = nullptr;
        }
        if (m_profileViewMenu) m_profileViewMenu->setVisible(m_currentMode == MODE_PROFILE);
        if (m_profilePassInfoMenu) m_profilePassInfoMenu->setVisible(m_currentMode == MODE_PROFILE);

        if (isNames) {
            m_background->setContentSize({ 235.f, 205.f });
            m_background->setPosition({ 134.f, 125.f });
            m_namesContainer->setPosition(m_background->getPosition());
        }
        else {
            m_background->setContentSize({ 235.f, 205.f });
            m_background->setPosition({ 134.f, 125.f });
            m_namesContainer->setPosition({ -9999.f, -9999.f });
        }

        m_badgeMenu->setVisible(!isNames);
        m_decorationNode->setVisible(!isNames);
        m_catArrowMenu->setVisible(m_currentMode != MODE_SONGS);
        if (m_gridScroll) {
            m_gridScroll->setVisible(!isNames);
            m_gridScroll->setTouchEnabled(!isNames);
        }
        if (!isNames) {
            for (auto* scroll : m_nameCategoryScrolls) {
                if (scroll) {
                    scroll->setTouchEnabled(false);
                    scroll->setMouseEnabled(false);
                    scroll->enableScrollWheel(false);
                }
            }
            for (auto* scrollbar : m_nameCategoryScrollbars) {
                if (scrollbar) {
                    scrollbar->setVisible(false);
                    scrollbar->setTouchEnabled(false);
                    scrollbar->setMouseEnabled(false);
                }
            }
        }
        if (m_gridScrollbar) m_gridScrollbar->setVisible(!isNames);
        if (m_controlsBackground) m_controlsBackground->setVisible(true);
        m_counterText->setVisible(true);
        m_totalStatsLabel->setVisible(true);
        if (m_infoMenu) m_infoMenu->setVisible(!isNames);

        if (m_qualityNode) {
            m_qualityNode->setVisible(!isNames);
            if (!isNames) m_qualityNode->setCategory(m_currentCategory, false);
        }

        if (m_categoryLabel) {
            m_categoryLabel->setVisible(isNames);
        }

        if (isNames) updateNameCategoryDisplay();
    }

    void onSwitchToBadges(CCObject*) {
        if (m_currentMode == MODE_BADGES) return;
        m_currentMode = MODE_BADGES;
        toggleUIVisibility(false);
        updateCategoryDisplay();
    }

    void onSwitchToBanners(CCObject*) {
        if (m_currentMode == MODE_BANNERS) return;
        m_currentMode = MODE_BANNERS;
        if (m_currentCategory >= 5) m_currentCategory = 4;
        toggleUIVisibility(false);
        updateCategoryDisplay();
    }

    void onSwitchToNames(CCObject*) {
        if (m_currentMode == MODE_NAMES) return;
        m_currentMode = MODE_NAMES;
        toggleUIVisibility(true);
    }

    void onSwitchToSongs(CCObject*) {
        if (m_currentMode == MODE_SONGS) return;
        m_currentMode = MODE_SONGS;
        toggleUIVisibility(false);
        updateCategoryDisplay();
    }

    void onSwitchToProfile(CCObject*) {
        if (m_currentMode == MODE_PROFILE) return;
        m_currentMode = MODE_PROFILE;
        toggleUIVisibility(false);
        updateCategoryDisplay();
    }

    void updateCategoryDisplay() {
        if (m_currentMode == MODE_NAMES) return;

        bool isSongs = (m_currentMode == MODE_SONGS);
        bool isProfile = (m_currentMode == MODE_PROFILE);
        if (m_catArrowMenu) m_catArrowMenu->setVisible(!isSongs);
        if (m_qualityNode) m_qualityNode->setVisible(!isSongs && !isProfile);
        if (m_categoryLabel) m_categoryLabel->setVisible(isProfile);
        if (isProfile) {
            if (m_currentProfileCategory == 0) updateProfileEffectsDisplay(true);
            else if (m_currentProfileCategory == 1) updateProfilePopupDisplay(true);
            else updateProfileDrawsDisplay(true);
            return;
        }
        if (m_profileViewMenu) m_profileViewMenu->setVisible(false);
        if (m_infoMenu) m_infoMenu->setVisible(true);
        if (m_gridScrollbar) m_gridScrollbar->setVisible(true);

        m_badgeMenu->removeAllChildren();
        m_decorationNode->removeAllChildren();
        if (m_cellNode) m_cellNode->removeAllChildren();


        if (m_qualityNode && !isSongs) {
            m_qualityNode->setCategory(m_currentCategory, false);
        }

        StreakData::BadgeCategory currentCat = static_cast<StreakData::BadgeCategory>(m_currentCategory);

        std::string equippedID = "";
        if (m_currentMode == MODE_BADGES) {
            auto eq = g_streakData.getEquippedBadge();
            if (eq) equippedID = eq->badgeID;
        }
        else if (m_currentMode == MODE_BANNERS) {
            auto eq = g_streakData.getEquippedBanner();
            if (eq) equippedID = eq->bannerID;
        }
        else {
            equippedID = g_streakData.equippedSong;
        }

        struct DisplayItem {
            std::string id;
            std::string spriteName;
            bool unlocked;
            bool isFromRoulette;
            int daysRequired;
            bool installed;
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
                        badge.daysRequired,
                        RemoteAssets::isInstalled(RemoteAssets::Type::Badge, badge.badgeID)
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
        else if (m_currentMode == MODE_BANNERS) {
            for (auto& banner : g_streakData.banners) {
                if (banner.rarity == currentCat) {
                    itemsToShow.push_back({
                        banner.bannerID,
                        banner.spriteName,
                        g_streakData.isBannerUnlocked(banner.bannerID),
                        true,
                        0,
                        RemoteAssets::isInstalled(RemoteAssets::Type::Banner, banner.bannerID)
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
        else {
            for (auto& song : g_streakData.songs) {
                itemsToShow.push_back({
                    song.songID,
                    song.iconName,
                    g_streakData.isSongUnlocked(song.songID),
                    true,
                    0,
                    RemoteAssets::isInstalled(RemoteAssets::Type::Song, song.songID)
                    });
            }
            globalTotal = g_streakData.songs.size();
            for (auto& s : g_streakData.songs) {
                if (g_streakData.isSongUnlocked(s.songID)) globalUnlocked++;
            }
            if (m_totalStatsLabel) {
                m_totalStatsLabel->setString(fmt::format("Songs: {}/{}", globalUnlocked, globalTotal).c_str());
            }
        }

        m_visibleAssets.clear();
        m_waitingForVisibleAssets = false;
        auto visibleType = currentAssetType();
        for (auto const& item : itemsToShow) {
            m_visibleAssets.emplace_back(visibleType, item.id);
            if (!item.installed) {
                m_waitingForVisibleAssets = true;
                RemoteAssets::ensure(visibleType, item.id);
            }
        }

        float gridW = m_background->getContentSize().width;
        float gridH = m_background->getContentSize().height;

        int itemsPerRow = (m_currentMode == MODE_BANNERS) ? 2 : 3;
        float colSpacing = (m_currentMode == MODE_BANNERS) ? 110.f : 65.f;
        float rowSpacing = (m_currentMode == MODE_BANNERS) ? 64.f : 55.f;
        if (isSongs) { itemsPerRow = 2; colSpacing = 110.f; rowSpacing = 80.f; }

        int itemCount = static_cast<int>(itemsToShow.size());
        int numRows = (itemCount + itemsPerRow - 1) / itemsPerRow;
        if (numRows < 1) numRows = 1;

        float topPad = isSongs ? 12.f : 8.f;
        float contentHeight = std::max(gridH, numRows * rowSpacing + topPad * 2.f);

        m_gridScroll->m_contentLayer->setContentSize({ gridW, contentHeight });
        if (m_cellNode) m_cellNode->setContentSize({ gridW, contentHeight });
        m_badgeMenu->setContentSize({ gridW, contentHeight });
        m_decorationNode->setContentSize({ gridW, contentHeight });

        float startX = gridW / 2.f - ((itemsPerRow - 1) * colSpacing) / 2.f;
        float topY = contentHeight - topPad - rowSpacing / 2.f;

        for (int i = 0; i < itemCount; ++i) {
            int row = i / itemsPerRow;
            int col = i % itemsPerRow;
            auto& item = itemsToShow[i];

            CCPoint position = { startX + col * colSpacing, topY - row * rowSpacing };

            if (isSongs) {
                auto cell = cocos2d::extension::CCScale9Sprite::create("square02_001.png");
                cell->setContentSize({ colSpacing - 14.f, rowSpacing - 12.f });
                cell->setColor({ 0, 0, 0 });
                cell->setOpacity(item.unlocked ? 110 : 70);
                cell->setPosition(position);
                m_cellNode->addChild(cell);
            }

            CCNode* itemVisual = nullptr;
            if (item.installed) {
                auto sprite = CCSprite::create(item.spriteName.c_str());
                if (!sprite) sprite = CCSprite::create("GJ_button_01.png");

                float scale = 0.3f;
                if (m_currentMode == MODE_BANNERS) {
                    float targetWidth = 120.f;
                    scale = targetWidth / sprite->getContentSize().width;
                }
                else if (isSongs) {
                    float targetSize = 46.f;
                    scale = targetSize / std::max(sprite->getContentSize().width, sprite->getContentSize().height);
                }
                sprite->setScale(scale);
                if (!item.unlocked) sprite->setColor({ 100, 100, 100 });
                itemVisual = sprite;
            }
            else {
                // Native Geometry Dash loading-circle artwork, wrapped by
                // Geode so it sizes correctly inside an individual grid cell.
                itemVisual = LoadingSpinner::create(26.f);
            }

            CCMenuItemSpriteExtra* btn = CCMenuItemSpriteExtra::create(
                itemVisual,
                this,
                menu_selector(RewardsPopup::onItemClick)
            );
            btn->setUserObject("item_id"_spr, CCString::create(item.id));

            CCPoint iconPos = isSongs ? CCPoint{ position.x, position.y + 12.f } : position;
            btn->setPosition(iconPos);
            m_badgeMenu->addChild(btn);

            if (isSongs) {
                auto info = g_streakData.getSongInfo(item.id);
                if (info) {
                    auto nameLbl = CCLabelBMFont::create(info->displayName.c_str(), "bigFont.fnt");
                    nameLbl->setScale(0.32f);
                    float maxW = colSpacing - 18.f;
                    if (nameLbl->getContentSize().width * nameLbl->getScale() > maxW) {
                        nameLbl->setScale(maxW / nameLbl->getContentSize().width);
                    }
                    if (!item.unlocked) nameLbl->setColor({ 150, 150, 150 });
                    nameLbl->setPosition({ position.x, position.y - 22.f });
                    m_cellNode->addChild(nameLbl);
                }
            }

            if (item.id == equippedID) {
                auto check = CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
                check->setPosition(iconPos + (isSongs ? CCPoint{ 20.f, 12.f } : CCPoint{ 15.f, -15.f }));
                check->setScale(0.6f);
                m_decorationNode->addChild(check);
            }

            if (!item.unlocked) {
                auto lockIcon = CCSprite::createWithSpriteFrameName("GJ_lock_001.png");
                lockIcon->setPosition(iconPos);
                lockIcon->setScale(0.7f);
                m_decorationNode->addChild(lockIcon);
            }
        }

        m_gridScroll->scrollToTop();

        int categoryUnlockedCount = 0;
        for (auto& it : itemsToShow) {
            if (it.unlocked) categoryUnlockedCount++;
        }
        if (m_counterText) {
            m_counterText->setString(
                fmt::format("Category: {}/{}", categoryUnlockedCount, itemsToShow.size()).c_str()
            );
        }
    }

    void onItemClick(CCObject* sender) {
        auto id = static_cast<CCString*>(static_cast<CCNode*>(sender)->getUserObject("item_id"_spr))->getCString();

        auto assetType = currentAssetType();
        if (!RemoteAssets::isInstalled(assetType, id)) {
            Notification::create("Asset is loading...", NotificationIcon::Loading, 1.5f)->show();
            return;
        }

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
        else if (m_currentMode == MODE_SONGS) {
            auto popup = EquipSongPopup::create(id);
            popup->onStateChanged = [this]() { this->updateCategoryDisplay(); };
            popup->show();
        }
    }

    void onNextCategory(CCObject*) {
        if (m_currentMode == MODE_NAMES) {
            m_currentNameCategory = (m_currentNameCategory + 1) % 4;
            updateNameCategoryDisplay();
            return;
        }
        if (m_currentMode == MODE_PROFILE) {
            m_currentProfileCategory = (m_currentProfileCategory + 1) % 3;
            updateCategoryDisplay();
            return;
        }
        const int categoryCount = m_currentMode == MODE_BADGES ? 6 : 5;
        m_currentCategory = (m_currentCategory + 1) % categoryCount;
        updateCategoryDisplay();
    }

    void onPreviousCategory(CCObject*) {
        if (m_currentMode == MODE_NAMES) {
            m_currentNameCategory = (m_currentNameCategory + 3) % 4;
            updateNameCategoryDisplay();
            return;
        }
        if (m_currentMode == MODE_PROFILE) {
            m_currentProfileCategory = (m_currentProfileCategory + 2) % 3;
            updateCategoryDisplay();
            return;
        }
        const int categoryCount = m_currentMode == MODE_BADGES ? 6 : 5;
        m_currentCategory = (m_currentCategory - 1 + categoryCount) % categoryCount;
        updateCategoryDisplay();
    }

    void onClose(CCObject* sender) override {
        if (!m_closeSaveHandled) {
            m_closeSaveHandled = true;
            if (cosmeticsChanged()) {
                // StreakData::save performs the single authenticated server
                // update containing every equipped cosmetic field.
                g_streakData.save();
            }
        }
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
