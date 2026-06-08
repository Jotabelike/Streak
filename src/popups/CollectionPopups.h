#pragma once
#include "StreakCommon.h"
#include "../StreakData.h"
#include "../FirebaseManager.h"
#include "../NameModifiers.h"
#include "../StreakMusic.h"
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/ScrollLayer.hpp>
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
            g_streakData.unequipSong();
            FLAlertLayer::create("Success", "Song unequipped!", "OK")->show();
        }
        else {
            g_streakData.equipSong(m_songID);
            FLAlertLayer::create("Success", "Song equipped!", "OK")->show();
        }
        g_streakData.save();
        updatePlayerDataInFirebase();

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


class RewardsPopup : public Popup {
protected:
    enum DisplayMode { MODE_BADGES, MODE_BANNERS, MODE_NAMES, MODE_SONGS };
    DisplayMode m_currentMode = MODE_BADGES;
    int m_currentCategory = 0;
    QualityNode* m_qualityNode = nullptr;
    CCLabelBMFont* m_categoryLabel = nullptr;  
    CCMenu* m_badgeMenu = nullptr;
    CCNode* m_decorationNode = nullptr;
    CCNode* m_namesContainer = nullptr;
    CCLabelBMFont* m_namePreviewLabel = nullptr;
    CCMenu* m_catArrowMenu = nullptr;
    geode::ScrollLayer* m_gridScroll = nullptr;
    CCNode* m_gridScrollbar = nullptr;
    CCNode* m_cellNode = nullptr;
    cocos2d::extension::CCScale9Sprite* m_background = nullptr;
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

            g_streakData.save();
            updatePlayerDataInFirebase();
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
                if (m_eventOnlyLabel) m_eventOnlyLabel->setVisible(true);
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

            updatePlayerDataInFirebase();

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

        auto leftMenu = CCMenu::create();
        leftMenu->addChild(badgeBtn);
        leftMenu->addChild(bannerBtn);
        leftMenu->addChild(nameBtn);
        leftMenu->addChild(songBtn);
        leftMenu->alignItemsVerticallyWithPadding(6.f);
        leftMenu->setPosition({ -20.f, CENTER_Y + 50.f });
        m_mainLayer->addChild(leftMenu);

        m_background = cocos2d::extension::CCScale9Sprite::create("square02_001.png");
        m_background->setColor({ 0, 0, 0 });
        m_background->setOpacity(120);
        m_background->setContentSize({ 375.f, 130.f });
        m_background->setPosition({ CENTER_X, CENTER_Y - 25.f });
        m_mainLayer->addChild(m_background);

        float categoryY = POPUP_HEIGHT - 70.f;
        auto catLeftArrow = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        auto catLeftBtn = CCMenuItemSpriteExtra::create(catLeftArrow,
            this,
            menu_selector(RewardsPopup::onPreviousCategory));
        catLeftBtn->setPosition(-140.f, 0);

        auto catRightArrow = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        catRightArrow->setFlipX(true);
        auto catRightBtn = CCMenuItemSpriteExtra::create(catRightArrow,
            this,
            menu_selector(RewardsPopup::onNextCategory));
        catRightBtn->setPosition(140.f, 0); 

        m_catArrowMenu = CCMenu::create();
        m_catArrowMenu->addChild(catLeftBtn);
        m_catArrowMenu->addChild(catRightBtn);
        m_catArrowMenu->setPosition({ CENTER_X, categoryY });
        m_mainLayer->addChild(m_catArrowMenu);
        m_qualityNode = QualityNode::create();
        m_qualityNode->setPosition({ CENTER_X, categoryY });
        m_mainLayer->addChild(m_qualityNode, 9);
        m_categoryLabel = CCLabelBMFont::create("", "goldFont.fnt");
        m_categoryLabel->setScale(0.6f);
        m_categoryLabel->setPosition({ CENTER_X, categoryY });
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
        m_counterText->setScale(0.4f);
        m_counterText->setPosition({ CENTER_X, 25.f });
        m_mainLayer->addChild(m_counterText);

        auto infoSpr = CCSprite::create("info_btn.png"_spr);
        infoSpr->setScale(0.25f);
        auto infoBtn = CCMenuItemSpriteExtra::create(infoSpr,
            this,
            menu_selector(RewardsPopup::onCopyrightInfo));
        auto infoMenu = CCMenu::createWithItem(infoBtn);
        infoMenu->setPosition({ 25.f, POPUP_HEIGHT - 25.f });
        m_mainLayer->addChild(infoMenu);

        m_totalStatsLabel = CCLabelBMFont::create("Total 0/0", "goldFont.fnt");
        m_totalStatsLabel->setScale(0.4f);
        m_totalStatsLabel->setAnchorPoint({ 1.0f, 0.5f });
        m_totalStatsLabel->setPosition({ POPUP_WIDTH - 15.f, 25.f });
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

        float colOffsets[4] = { -132.f, -44.f, 44.f, 132.f };

        auto effectsTitle = CCLabelBMFont::create("Effects", "goldFont.fnt");
        effectsTitle->setScale(0.45f);
        effectsTitle->setPosition({ colOffsets[0], 52.f });
        m_namesContainer->addChild(effectsTitle);

        auto animsTitle = CCLabelBMFont::create("Anims", "goldFont.fnt");
        animsTitle->setScale(0.45f);
        animsTitle->setPosition({ colOffsets[1], 52.f });
        m_namesContainer->addChild(animsTitle);

        auto colorsTitle = CCLabelBMFont::create("Colors", "goldFont.fnt");
        colorsTitle->setScale(0.45f);
        colorsTitle->setPosition({ colOffsets[2], 52.f });
        m_namesContainer->addChild(colorsTitle);

        auto fontsTitle = CCLabelBMFont::create("Fonts", "goldFont.fnt");
        fontsTitle->setScale(0.45f);
        fontsTitle->setPosition({ colOffsets[3], 52.f });
        m_namesContainer->addChild(fontsTitle);

        std::vector<std::string> effects = {
        "None", "Blood", "Bubbles", "ConcertLights", "Confetti", "Electric",
        "Ember", "Explosion", "Fire", "Fireflies", "Galaxy", "Glitch",
        "Heartbeat", "Holy", "Ice", "Lava", "Matrix", "Meteor", "Career",
        "Orbit", "Plasma", "Poison", "Pulse", "Rain", "Rainbow", "Sakura",
        "Scanner", "Shadow", "Shockwave", "Smoke", "Snow", "Sparkle",
        "Spotlight", "Stars", "Supernova", "Toxic", "Void"
        };

        std::vector<std::string> animations = {
              "None", "Blink", "Bounce", "Domino", "DVD", "Dynamic Jump", "Float",
              "Glitch", "Heartbeat", "Jelly", "Pulse", "Shake", "Spin",
              "Spiral", "Squish", "Swing", "Tremble", "Wave", "Wobble"
        };
        std::vector<std::string> colors = {
                "Default", "Black", "Blue", "Brown", "Cyan", "Gold", "Green",
                "Lime", "Magenta", "Maroon", "Mint", "Navy", "Orange",
                "Peach", "Pink", "Purple", "Red", "Silver", "Teal", "Yellow",
                "Crazy Wave",
                "Cyberpunk Wave", "Fire Wave", "Galaxy Wave", "Golden Wave", "Ice Wave", "Ocean Wave",
                "Rainbow", "Rainbow Wave", "Royal Wave", "Sunset Wave", "Toxic Wave",
                "Abyss Wave","Disco Blink", "Synthwave", "Pastel Wave", "Aurora Wave",
                "Static Blood", "Static Deep Sea", "Static Toxic", "Static Vaporwave"
        };

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
        auto createColumn = [this](const std::vector<std::string>& items, float xOffset, int tag) {
            float listWidth = 72.f;
            float listHeight = 92.f;
            float itemHeight = 30.f;
            float totalHeight = items.size() * itemHeight;
            if (totalHeight < listHeight) totalHeight = listHeight;

            float columnCenterY = -14.f;

             
            auto listBg = cocos2d::extension::CCScale9Sprite::create("square02_001.png");
            listBg->setContentSize({ listWidth, listHeight });
            listBg->setPosition({ xOffset, columnCenterY });
            listBg->setColor({ 0, 0, 0 });
            listBg->setOpacity(80);
            m_namesContainer->addChild(listBg);

           
            auto stencil = cocos2d::extension::CCScale9Sprite::create("square02_001.png");
            stencil->setContentSize({ listWidth, listHeight });
            stencil->setPosition({ xOffset, columnCenterY });

            auto clipper = CCClippingNode::create();
            clipper->setStencil(stencil);
            clipper->setAlphaThreshold(0.05f);
            clipper->setPosition({ 0, 0 });  
            m_namesContainer->addChild(clipper);

        
            auto scrollLayer = geode::ScrollLayer::create({ listWidth, listHeight });
            scrollLayer->setPosition({ xOffset - listWidth / 2.f, columnCenterY - listHeight / 2.f });

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
                label->setScale(0.55f);

                if (tag == 3) NameModifiers::applyColor(label, item);
                if (tag == 4) NameModifiers::applyFont(label, item);

                float maxLabelWidth = listWidth - 6.f;
                if (label->getContentSize().width * label->getScale() > maxLabelWidth) {
                    label->setScale(maxLabelWidth / label->getContentSize().width);
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
            clipper->addChild(scrollLayer);
            addScrollbar(scrollLayer, 4.f, m_namesContainer);
            };

        createColumn(effects, colOffsets[0], 1);
        createColumn(animations, colOffsets[1], 2);
        createColumn(colors, colOffsets[2], 3);
        createColumn(fonts, colOffsets[3], 4); 
        updateCellHighlights();
        m_namePreviewLabel = CCLabelBMFont::create("Player", "bigFont.fnt");
        m_namePreviewLabel->setPosition({ 0.f, 95.f });
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

        m_buyNameBtn = CCMenuItemSpriteExtra::create(buyBtnSprite,
            this,
            menu_selector(RewardsPopup::onBuyNameItem));
        m_buyNameBtn->setVisible(false);

        auto buyMenu = CCMenu::create();
        buyMenu->addChild(m_buyNameBtn);
        buyMenu->setPosition({ 0.f, -95.f });
        m_namesContainer->addChild(buyMenu);

        m_eventOnlyLabel = CCLabelBMFont::create("Event reward", "goldFont.fnt");
        m_eventOnlyLabel->setScale(0.5f);
        m_eventOnlyLabel->setPosition({ 0.f, -95.f });
        m_eventOnlyLabel->setVisible(false);
        m_namesContainer->addChild(m_eventOnlyLabel);
        updateCategoryDisplay();
        this->updateNamePreview();
        this->updateCellHighlights();
        return true;
    }

    void toggleUIVisibility(bool isNames) {
        m_namesContainer->setVisible(isNames);
 
        if (isNames) {
            m_namesContainer->setPosition(m_background->getPosition());
        }
        else {
            m_namesContainer->setPosition({ -9999.f, -9999.f });
        }

        m_badgeMenu->setVisible(!isNames);
        m_decorationNode->setVisible(!isNames);
        m_catArrowMenu->setVisible(!isNames);
        if (m_gridScroll) m_gridScroll->setVisible(!isNames);
        if (m_gridScrollbar) m_gridScrollbar->setVisible(!isNames);
        m_counterText->setVisible(!isNames);
        m_totalStatsLabel->setVisible(!isNames);

        if (m_qualityNode) {
            m_qualityNode->setVisible(true);
            m_qualityNode->setCategory(m_currentCategory, isNames);
        }

        if (m_categoryLabel) {
            m_categoryLabel->setVisible(false);
        }
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

    void updateCategoryDisplay() {
        if (m_currentMode == MODE_NAMES) return;

        bool isSongs = (m_currentMode == MODE_SONGS);
        if (m_catArrowMenu) m_catArrowMenu->setVisible(!isSongs);
        if (m_qualityNode) m_qualityNode->setVisible(!isSongs);

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
        else if (m_currentMode == MODE_BANNERS) {
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
        else {
            for (auto& song : g_streakData.songs) {
                itemsToShow.push_back({
                    song.songID,
                    song.iconName,
                    g_streakData.isSongUnlocked(song.songID),
                    true,
                    0
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

        float gridW = m_background->getContentSize().width;
        float gridH = m_background->getContentSize().height;

        int itemsPerRow = (m_currentMode == MODE_BANNERS) ? 2 : 3;
        float colSpacing = (m_currentMode == MODE_BANNERS) ? 150.f : 65.f;
        float rowSpacing = (m_currentMode == MODE_BANNERS) ? 64.f : 55.f;
        if (isSongs) { itemsPerRow = 3; colSpacing = 110.f; rowSpacing = 80.f; }

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

            auto sprite = CCSprite::create(item.spriteName.c_str());
            if (!sprite) {
                sprite = CCSprite::create("GJ_button_01.png");
            }

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

            if (!item.unlocked) {
                sprite->setColor({ 100, 100, 100 });
            }

            CCMenuItemSpriteExtra* btn = CCMenuItemSpriteExtra::create(
                sprite,
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
        m_currentCategory = (m_currentCategory + 1) % 5;
        updateCategoryDisplay();
    }

    void onPreviousCategory(CCObject*) {
        m_currentCategory = (m_currentCategory - 1 + 5) % 5;
        updateCategoryDisplay();
    }

    void onClose(CCObject* sender) override {
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