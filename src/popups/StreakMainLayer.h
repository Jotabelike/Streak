#pragma once
#include "StreakCommon.h"
#include "../StreakData.h"
#include "../StreakMusic.h"
#include "../UpdateState.h"
#include "../FirebaseManager.h"
#include <Geode/utils/async.hpp>
#include <Geode/ui/Popup.hpp>
#include "HistoryPopup.h"
#include "StatsPopups.h"
#include "CollectionPopups.h"
#include "MissionsPopup.h"
#include "AdminPopups.h"
#include "LeaderboardPopup.h"
#include "RankPopup.h"
#include "MailPopups.h"
#include "EventPopup.h"
#include "LevelProgressPopup.h"
#include "RoulettePopup.h"
#include "SendMessagePopup.h"
#include "../SystemNotification.h"
#include "XPPopup.h"
#include "SettingsPopup.h"
#include "ProfileCardPopup.h"
#include "TaskPopup.h"
#include "StProgressPopup.h"
#include "LevelLockPopup.h"
#include "DailyShopPopup.h"
#include "MilestonesPopup.h"
#include "TrendLevelsPopup.h"
#include "AchievementsPopup.h"
#include "StreakAnimations.h"
#include "RankDemotionAnimation.h"
#include "../utils/RoundedProgressBar.h"
#include "DiscordGoalPopup.h"
#include "RegisterPopup.h"
#include "ShieldsPopup.h"
#include "UpdatePopup.h"
#include "../NameModifiers.h"

class StreakMainLayer : public cocos2d::CCLayer {
protected:
    CCSize m_size;

    CCLabelBMFont* m_streakLabel = nullptr;
    RoundedProgressBar* m_streakBar = nullptr;
    CCLabelBMFont* m_barText = nullptr;
    RoundedProgressBar* m_xpBar = nullptr;
    CCLabelBMFont* m_xpLabel = nullptr;
    CCLabelBMFont* m_xpProgressLabel = nullptr;
    CCLabelBMFont* m_gemsLabel = nullptr;
    CCLabelBMFont* m_shieldsLabel = nullptr;

    cocos2d::extension::CCScale9Sprite* m_shieldsBg = nullptr;
    CCMenuItemSpriteExtra* m_shieldsBtn = nullptr;
    cocos2d::extension::CCScale9Sprite* m_gemsBg = nullptr;
    CCSprite* m_gemsIcon = nullptr;
    float m_countersStartX = 0.f;
    float m_countersY = 0.f;
    float m_shieldsIconW = 0.f;
    float m_gemsIconW = 0.f;

    CCMenuItemSpriteExtra* m_xpBtnRef = nullptr;
    CCMenuItemSpriteExtra* m_missionsBtnRef = nullptr;
    CCMenuItemSpriteExtra* m_updateBtnRef = nullptr;
    CCMenuItemSpriteExtra* m_levelProgBtnRef = nullptr;
    CCMenuItemSpriteExtra* m_msgBtnRef = nullptr;
    CCMenuItemSpriteExtra* m_passBtnRef = nullptr;

    std::vector<CCMenu*> m_bottomPages;
    int m_currentPage = 0;
    CCMenuItemSpriteExtra* m_leftArrowBtn = nullptr;
    CCMenuItemSpriteExtra* m_rightArrowBtn = nullptr;

    async::TaskHolder<web::WebResponse> m_msgCheckListener;

    static int s_lastPendingLevelCount;
    static int s_lastPendingDailyCount;
    static int s_lastNewMsgCount;

    void refreshTimer(float dt) {
        this->updateDisplay();
    }

    void applyRedTag(CCMenuItemSpriteExtra* btn, bool show) {
        if (!btn) return;
        const int TAG_ID = 9991;
        auto existing = btn->getChildByTag(TAG_ID);
        if (show) {
            if (existing) return;
            auto tag = CCSprite::create("red_tag.png"_spr);
            if (!tag) tag = CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
            if (!tag) return;
            tag->setScale(0.6f);
            auto size = btn->getContentSize();
            tag->setPosition({ size.width - 4.f, size.height - 4.f });
            tag->setTag(TAG_ID);
            btn->addChild(tag, 100);

            tag->runAction(CCRepeatForever::create(CCSequence::create(
                CCEaseSineInOut::create(CCScaleTo::create(0.6f, 0.7f)),
                CCEaseSineInOut::create(CCScaleTo::create(0.6f, 0.6f)),
                nullptr
            )));
        }
        else if (existing) {
            existing->removeFromParentAndCleanup(true);
        }
    }

    bool hasClaimablePassTiers() {
        if (!g_streakData.isPassActive()) return false;
        int gold = g_streakData.goldTickets;
        int freeMax = (int)g_streakData.freePassRewards.size();
        int paidMax = (int)g_streakData.paidPassRewards.size();
        for (int t = 1; t <= freeMax; ++t) {
            if (!g_streakData.isFreePassTierClaimed(t) && gold >= t * 100) return true;
        }
        if (g_streakData.isPremiumPassActive()) {
            for (int t = 1; t <= paidMax; ++t) {
                if (!g_streakData.isPaidPassTierClaimed(t) && gold >= t * 50) return true;
            }
        }
        if (!g_streakData.passCompleteRewardClaimed && gold >= 2000) return true;
        return false;
    }

    void refreshRedTags() {
        applyRedTag(m_xpBtnRef, g_streakData.hasPendingLevelRewards());
        applyRedTag(m_missionsBtnRef, g_streakData.hasPendingDailyMissions());
        applyRedTag(m_levelProgBtnRef, g_streakData.hasPendingLevelMissions());
        applyRedTag(m_msgBtnRef, g_streakData.hasNewMessages);
        applyRedTag(m_passBtnRef, hasClaimablePassTiers());
        applyRedTag(m_updateBtnRef, StreakUpdate::isAvailable());
    }

    void fitLabelToWidth(CCLabelBMFont* label, float maxW, float baseScale) {
        if (!label) return;
        float rawW = label->getContentSize().width;
        float scale = baseScale;
        if (rawW > 0 && rawW * baseScale > maxW) {
            scale = maxW / rawW;
        }
        label->setScale(scale);
    }

    void layoutCounters() {
        float boxH = 27.f;
        float padL = 2.f;   // icons 4 px to the left vs before (was 6)
        float padR = 6.f;
        float gap = 4.f;
        float between = 6.f;
        float maxLabelW = 38.f; // hard cap → text auto-shrinks past this

        float x = m_countersStartX;

        if (m_shieldsBg && m_shieldsBtn && m_shieldsLabel) {
            fitLabelToWidth(m_shieldsLabel, maxLabelW, 0.5f);

            float iconW = m_shieldsIconW;
            float labelW = m_shieldsLabel->getScaledContentSize().width;
            float boxW = padL + iconW + gap + labelW + padR;
            float boxX = x + boxW / 2;

            m_shieldsBg->setContentSize({ boxW, boxH });
            m_shieldsBg->setPosition({ boxX, m_countersY });
            m_shieldsBtn->setPosition({ x + padL + iconW / 2, m_countersY });
            m_shieldsLabel->setPosition({ x + padL + iconW + gap, m_countersY });

            x = boxX + boxW / 2 + between;
        }

        if (m_gemsBg && m_gemsIcon && m_gemsLabel) {
            fitLabelToWidth(m_gemsLabel, maxLabelW, 0.5f);

            float iconW = m_gemsIconW;
            float labelW = m_gemsLabel->getScaledContentSize().width;
            float boxW = padL + iconW + gap + labelW + padR;
            float boxX = x + boxW / 2;

            m_gemsBg->setContentSize({ boxW, boxH });
            m_gemsBg->setPosition({ boxX, m_countersY });
            m_gemsIcon->setPosition({ x + padL + iconW / 2, m_countersY });
            m_gemsLabel->setPosition({ x + padL + iconW + gap, m_countersY });
        }
    }

    void checkAllNotifications() {
        int currentPendingLevel = 0;

        for (const auto& mission : g_levelMissions) {
            if (g_streakData.isLevelMissionClaimed(mission.levelID)) continue;
            auto level = GameLevelManager::sharedState()->getSavedLevel(mission.levelID);
            if (level && level->m_normalPercent >= 100) currentPendingLevel++;
        }

        if (currentPendingLevel > 0 && currentPendingLevel > s_lastPendingLevelCount) {
            SystemNotification::show(
                "Level Challenges",
                fmt::format("You have {} unclaimed rewards", currentPendingLevel),
                "level_progess_btn.png"_spr,
                0.6f
            );
        }
        s_lastPendingLevelCount = currentPendingLevel;

        int currentPendingDaily = 0;
        int p = g_streakData.streakPointsToday;

        if (p >= 5 && !g_streakData.pointMission1Claimed) currentPendingDaily++;
        if (p >= 10 && !g_streakData.pointMission2Claimed) currentPendingDaily++;
        if (p >= 15 && !g_streakData.pointMission3Claimed) currentPendingDaily++;
        if (p >= 20 && !g_streakData.pointMission4Claimed) currentPendingDaily++;
        if (p >= 25 && !g_streakData.pointMission5Claimed) currentPendingDaily++;
        if (p >= 30 && !g_streakData.pointMission6Claimed) currentPendingDaily++;

        if (currentPendingDaily > 0 && currentPendingDaily > s_lastPendingDailyCount) {
            SystemNotification::show(
                "Daily Missions",
                fmt::format("{} Rewards ready!", currentPendingDaily),
                "streak_point.png"_spr
            );
        }
        s_lastPendingDailyCount = currentPendingDaily;
    }

    void checkNewMessages() {
        auto req = web::WebRequest();
        m_msgCheckListener.spawn(
            req.get("https://streak-servidor.onrender.com/messages"),
            [this](web::WebResponse res) {
                this->onMessagesChecked(std::move(res));
            }
        );
    }

    void onMessagesChecked(web::WebResponse res) {
        if (!res.ok() || !res.json().isOk()) return;

        auto data = res.json().unwrap();
        if (!data.isArray()) return;

        auto messages = data.as<std::vector<matjson::Value>>().unwrap();
        double savedTime = Mod::get()->getSavedValue<double>("streak_last_chat_time", 0.0);
        long long lastSeenTime = (long long)savedTime;
        int currentNewCount = 0;

        for (const auto& msg : messages) {
            long long timestamp = msg["timestamp"].as<long long>().unwrapOr(0);
            if (timestamp > lastSeenTime) currentNewCount++;
        }

        if (currentNewCount > 0 && currentNewCount > s_lastNewMsgCount) {
            SystemNotification::show(
                "New Announcement",
                fmt::format("There are {} new messages", currentNewCount),
                "msm.png"_spr,
                0.6f
            );
        }
        s_lastNewMsgCount = currentNewCount;
        g_streakData.hasNewMessages = (currentNewCount > 0);
        refreshRedTags();
    }

    void onShieldsClick(CCObject*) {
        if (auto popup = ShieldsPopup::create([this]() { this->updateDisplay(); })) {
            popup->show();
        }
    }

    void onRankClick(CCObject* sender) {
        int rank = sender->getTag();
        std::string title = "Rank Info";
        std::string desc = "Unknown Rank";

        switch (rank) {
        case 1: title = "Moderator";       desc = "This user is a <cg>Moderator</c> of the Streak Mod."; break;
        case 2: title = "Content Creator"; desc = "This user is a recognized <cp>Content Creator</c>!";   break;
        case 3: title = "V.I.P";           desc = "This user is a <cy>V.I.P Member</c> with exclusive status."; break;
        case 4: title = "Stellar";         desc = "Legendary <cp>Stellar</c> User.";                       break;
        default:                           desc = "Special user rank.";                                   break;
        }

        FLAlertLayer::create(title.c_str(), desc.c_str(), "OK")->show();
    }

    void applyGDPSDisable(CCMenuItemSpriteExtra* btn) {
        auto am = GJAccountManager::sharedState();
        if (!am) return;

        std::string gdpsKey = fmt::format("is_gdps_player_{}", am->m_accountID);
        bool isGDPS = geode::Mod::get()->getSavedValue<bool>(gdpsKey, g_streakData.isGDPS);

        if (!isGDPS) return;
        btn->setEnabled(false);
        btn->setOpacity(60);
        btn->setColor({ 120, 120, 120 });
    }

    bool init() override {
        if (!CCLayer::init()) return false;

        m_size = CCDirector::sharedDirector()->getWinSize();
        this->setKeypadEnabled(true);

        CCSprite* bg = nullptr;
        bool tintBg = true;

        std::string customBg = Mod::get()->getSavedValue<std::string>("streak_layer_bg_path", "");
        if (!customBg.empty()) {
            std::error_code ec;
            auto path = std::filesystem::path(customBg);
            if (std::filesystem::exists(path, ec)) {
                if (auto cache = CCTextureCache::sharedTextureCache()) {
                    auto tex = cache->addImage(customBg.c_str(), false);
                    if (tex) {
                        bg = CCSprite::createWithTexture(tex);
                        if (bg) tintBg = false;
                    }
                }
            }
        }

        if (!bg) {
            bg = CCSprite::create("game_bg_01_001.png");
            tintBg = true;
        }

        if (bg) {
            bg->setPosition(m_size / 2);
            auto bgSize = bg->getContentSize();
            if (bgSize.width > 0 && bgSize.height > 0) {
                bg->setScaleX(m_size.width / bgSize.width);
                bg->setScaleY(m_size.height / bgSize.height);
            }
            if (tintBg) bg->setColor({ 40, 40, 60 });
            this->addChild(bg, -10);
        }

        auto am = GJAccountManager::sharedState();

        // back button (bottom-left)
        auto backMenu = CCMenu::create();
        backMenu->setPosition({ 0, 0 });
        this->addChild(backMenu, 20);

        auto backSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
        auto backBtn = CCMenuItemSpriteExtra::create(
            backSpr, this, menu_selector(StreakMainLayer::onBack)
        );
        backBtn->setPosition({ 25.f, m_size.height - 25.f });
        backMenu->addChild(backBtn);

        if (!am || am->m_accountID == 0) {
            auto errorLabel = CCLabelBMFont::create(
                "Please log in to\nGeometry Dash first!",
                "bigFont.fnt"
            );
            errorLabel->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
            errorLabel->setScale(0.8f);
            errorLabel->setPosition(m_size / 2);
            errorLabel->setColor({ 255, 100, 100 });
            this->addChild(errorLabel);
            return true;
        }

        // top-left row: badge | banner+name | hearts | gems
        float rowY = m_size.height - 25.f;
        float cursorX = 55.f; // start right after the back button

        auto topMenu = CCMenu::create();
        topMenu->setPosition({ 0, 0 });
        this->addChild(topMenu, 10);

        // unified box: banner fills the whole bg (clipped), badge sits on top of it
        float profileBgH = 27.f;
        float profileBgW = 150.f;
        float profileBgX = cursorX + profileBgW / 2 - 8.f; // -8 shift requested

        auto profileNode = CCNode::create();
        profileNode->setContentSize({ profileBgW, profileBgH });
        profileNode->setAnchorPoint({ 0.5f, 0.5f });
        profileNode->ignoreAnchorPointForPosition(false);
        profileNode->setPosition({ profileBgX, rowY });
        this->addChild(profileNode, 4);

        auto profileBg = cocos2d::extension::CCScale9Sprite::create("square02_001.png");
        profileBg->setContentSize({ profileBgW, profileBgH });
        profileBg->setPosition({ profileBgW / 2.f, profileBgH / 2.f });
        profileBg->setOpacity(110);
        profileBg->setColor({ 0, 0, 0 });
        profileNode->addChild(profileBg, 0);

        auto stencil = cocos2d::extension::CCScale9Sprite::create("square02b_001.png");
        stencil->setContentSize({ profileBgW, profileBgH });
        stencil->setPosition({ profileBgW / 2.f, profileBgH / 2.f });

        auto clipNode = CCClippingNode::create(stencil);
        clipNode->setAlphaThreshold(0.05f);
        clipNode->setContentSize({ profileBgW, profileBgH });
        clipNode->setPosition({ 0.f, 0.f });
        profileNode->addChild(clipNode, 1);

        // banner fills the whole clipped area, starting from the left edge
        if (auto bannerInfo = g_streakData.getBannerInfo(g_streakData.equippedBanner)) {
            auto bannerSpr = CCSprite::create(bannerInfo->spriteName.c_str());
            if (bannerSpr) {
                bannerSpr->setPosition({ profileBgW / 2.f, profileBgH / 2.f });
                auto bs = bannerSpr->getContentSize();
                bannerSpr->setScaleX(profileBgW / bs.width);
                bannerSpr->setScaleY(profileBgH / bs.height);
                clipNode->addChild(bannerSpr);
            }
        }

        // badge on top of the banner, inside the box (left side, +2 from edge after -5px shift)
        if (auto badgeInfo = g_streakData.getBadgeInfo(g_streakData.equippedBadge)) {
            auto badgeSpr = CCSprite::create(badgeInfo->spriteName.c_str());
            if (badgeSpr) {
                badgeSpr->setScale(0.17f);
                badgeSpr->setPosition({ 2.f + badgeSpr->getScaledContentSize().width / 2.f, profileBgH / 2.f });
                clipNode->addChild(badgeSpr);
            }
        }

        // name on top, slightly to the right of the badge
        std::string userName = am->m_username.empty() ? "Player" : am->m_username;
        auto nameLabel = CCLabelBMFont::create(userName.c_str(), "bigFont.fnt");
        nameLabel->setScale(0.55f);
        nameLabel->setAnchorPoint({ 0.0f, 0.5f });
        nameLabel->setPosition({ 28.f, profileBgH / 2.f });
        clipNode->addChild(nameLabel);

        NameModifiers::applyFont(nameLabel, g_streakData.equippedNameFont);
        NameModifiers::applyColor(nameLabel, g_streakData.equippedNameColor);
        NameModifiers::applyAnimation(nameLabel, g_streakData.equippedNameAnimation);
        NameModifiers::applyEffect(nameLabel, g_streakData.equippedNameEffect);

        auto profileHit = cocos2d::extension::CCScale9Sprite::create("square02_001.png");
        profileHit->setContentSize({ profileBgW, profileBgH });
        profileHit->setOpacity(0);
        auto profileBtn = CCMenuItemSpriteExtra::create(
            profileHit, this, menu_selector(StreakMainLayer::onOpenAccount)
        );
        profileBtn->setPosition({ profileBgX, rowY });
        topMenu->addChild(profileBtn);

        m_countersStartX = profileBgX + profileBgW / 2 + 6.f;
        m_countersY = rowY;

        // hearts (shields) counter — bg grows with the label
        m_shieldsBg = cocos2d::extension::CCScale9Sprite::create("square02_001.png");
        m_shieldsBg->setOpacity(90);
        m_shieldsBg->setColor({ 0, 0, 0 });
        this->addChild(m_shieldsBg, 9);

        {
            auto heartSpr = CCSprite::create("heart.png"_spr);
            if (heartSpr) {
                heartSpr->setScale(0.22f);
                m_shieldsIconW = heartSpr->getContentSize().width * 0.22f;
                m_shieldsBtn = CCMenuItemSpriteExtra::create(
                    heartSpr, this, menu_selector(StreakMainLayer::onShieldsClick)
                );
                topMenu->addChild(m_shieldsBtn);
            }
        }

        m_shieldsLabel = CCLabelBMFont::create("0", "bigFont.fnt");
        m_shieldsLabel->setScale(0.5f);
        m_shieldsLabel->setAnchorPoint({ 0.0f, 0.5f });
        m_shieldsLabel->setColor({ 130, 200, 255 });
        this->addChild(m_shieldsLabel, 10);

        // gems counter — bg grows with the label
        m_gemsBg = cocos2d::extension::CCScale9Sprite::create("square02_001.png");
        m_gemsBg->setOpacity(90);
        m_gemsBg->setColor({ 0, 0, 0 });
        this->addChild(m_gemsBg, 9);

        m_gemsIcon = CCSprite::create("gem.png"_spr);
        if (!m_gemsIcon) m_gemsIcon = CCSprite::createWithSpriteFrameName("GJ_bigDiamond_001.png");
        if (m_gemsIcon) {
            m_gemsIcon->setScale(0.22f);
            m_gemsIconW = m_gemsIcon->getContentSize().width * 0.22f;
            this->addChild(m_gemsIcon, 10);
        }

        m_gemsLabel = CCLabelBMFont::create(
            std::to_string(g_streakData.gems).c_str(), "bigFont.fnt"
        );
        m_gemsLabel->setScale(0.5f);
        m_gemsLabel->setAnchorPoint({ 0.0f, 0.5f });
        m_gemsLabel->setColor({ 255, 220, 60 });
        this->addChild(m_gemsLabel, 10);

        m_shieldsLabel->setString(std::to_string(g_streakData.streakShields).c_str());
        layoutCounters();

        // top-right buttons (rank / account / settings)
        auto cornerMenu = CCMenu::create();
        cornerMenu->setPosition({ 0, 0 });
        this->addChild(cornerMenu, 10);

        if (g_streakData.specialRank > 0) {
            std::string badgeSpriteName = "";
            switch (g_streakData.specialRank) {
            case 1: badgeSpriteName = "moderator_badge.png"_spr; break;
            case 2: badgeSpriteName = "creator_badge.png"_spr;  break;
            case 3: badgeSpriteName = "vip_badge.png"_spr;      break;
            case 4: badgeSpriteName = "stellar_badge.png"_spr;  break;
            default: badgeSpriteName = "reward5.png"_spr;       break;
            }

            auto rankSprite = CCSprite::create(badgeSpriteName.c_str());
            if (!rankSprite) rankSprite = CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
            rankSprite->setScale(0.28f);

            auto rankBtn = CCMenuItemSpriteExtra::create(
                rankSprite, this, menu_selector(StreakMainLayer::onRankClick)
            );
            rankBtn->setTag(g_streakData.specialRank);
            rankBtn->setPosition({ m_size.width - 100.f, m_size.height - 22.f });
            cornerMenu->addChild(rankBtn);
        }

        auto downloadIcon = CCSprite::create("download_btn.png"_spr);
        if (!downloadIcon) downloadIcon = ButtonSprite::create("Update");
        else downloadIcon->setScale(0.75f);
        auto downloadBtn = CCMenuItemSpriteExtra::create(
            downloadIcon, this, menu_selector(StreakMainLayer::onOpenUpdate)
        );
        downloadBtn->setPosition({ m_size.width - 60.f, m_size.height - 22.f });
        cornerMenu->addChild(downloadBtn);
        m_updateBtnRef = downloadBtn;

        auto settingsIcon = CCSprite::createWithSpriteFrameName("accountBtn_settings_001.png");
        settingsIcon->setScale(0.75f);
        auto settingsBtn = CCMenuItemSpriteExtra::create(
            settingsIcon, this, menu_selector(StreakMainLayer::onOpenSettings)
        );
        settingsBtn->setPosition({ m_size.width - 25.f, m_size.height - 22.f });
        cornerMenu->addChild(settingsBtn);

        // center: racha sprite
        float contentCenterY = m_size.height / 2 + 50.f;

        auto rachaSprite = CCSprite::create(g_streakData.getRachaSprite().c_str());
        if (rachaSprite) {
            rachaSprite->setScale(0.6f);

            auto rachaBtn = CCMenuItemSpriteExtra::create(
                rachaSprite, this, menu_selector(StreakMainLayer::onRachaClick)
            );
            auto menuRacha = CCMenu::createWithItem(rachaBtn);
            menuRacha->setPosition({ m_size.width / 2, contentCenterY });
            this->addChild(menuRacha, 3);

            StreakAnimations::applyPremiumHover(rachaSprite);
        }

        // bars (streak + xp)
        float barWidth = 220.f;
        float barHeight = 24.f;
        float barY = contentCenterY - 105.f;

        m_streakLabel = CCLabelBMFont::create("Daily streak: ?", "goldFont.fnt");
        m_streakLabel->setScale(0.65f);
        m_streakLabel->setPosition({ m_size.width / 2, contentCenterY - 65.f });
        this->addChild(m_streakLabel);

        m_streakBar = RoundedProgressBar::create(barWidth, barHeight);
        m_streakBar->setPosition({ m_size.width / 2, barY + (barHeight / 2) });
        m_streakBar->setGradientColors({ 250, 225, 60 }, { 255, 165, 0 });
        m_streakBar->setBackgroundColor({ 45, 45, 45 });
        this->addChild(m_streakBar, 1);

        m_barText = CCLabelBMFont::create("? / ?", "bigFont.fnt");
        m_barText->setScale(0.5f);
        m_barText->setPosition({ m_size.width / 2, barY + (barHeight / 2) });
        this->addChild(m_barText, 8);

        float xpBarHeight = 12.f;
        float xpY = barY - 18.f;

        m_xpBar = RoundedProgressBar::create(barWidth, xpBarHeight);
        m_xpBar->setPosition({ m_size.width / 2, xpY + (xpBarHeight / 2) });
        m_xpBar->setGradientColors({ 0, 255, 255 }, { 0, 100, 255 });
        m_xpBar->setBackgroundColor({ 20, 20, 40 });
        this->addChild(m_xpBar, 1);

        m_xpLabel = CCLabelBMFont::create("Lvl. ?", "goldFont.fnt");
        m_xpLabel->setScale(0.45f);
        m_xpLabel->setPosition({ m_size.width / 2 - barWidth / 2 - 25.f, xpY + (xpBarHeight / 2) });
        this->addChild(m_xpLabel);

        m_xpProgressLabel = CCLabelBMFont::create("0/0", "chatFont.fnt");
        m_xpProgressLabel->setScale(0.45f);
        m_xpProgressLabel->setPosition({ m_size.width / 2, xpY + (xpBarHeight / 2) });
        this->addChild(m_xpProgressLabel, 8);

        // side buttons (stats/rewards/tasks at right, missions/roulette/xp at left)
        float sideY = m_size.height / 2 + 40.f;
        float sideSpacing = 50.f;
        float rightX = m_size.width - 35.f;
        float leftX = 35.f;

        auto statsIcon = CCSprite::create("BtnStats.png"_spr);
        statsIcon->setScale(0.9f);
        auto statsBtn = CCMenuItemSpriteExtra::create(
            statsIcon, this, menu_selector(StreakMainLayer::onOpenStats)
        );
        statsBtn->setPosition({ rightX, sideY });
        applyGDPSDisable(statsBtn);
        cornerMenu->addChild(statsBtn);

        auto rewardsIcon = CCSprite::create("RewardsBtn.png"_spr);
        rewardsIcon->setScale(0.9f);
        auto rewardsBtn = CCMenuItemSpriteExtra::create(
            rewardsIcon, this, menu_selector(StreakMainLayer::onOpenRewards)
        );
        rewardsBtn->setPosition({ rightX, sideY - sideSpacing });
        cornerMenu->addChild(rewardsBtn);

        if (g_streakData.isTaskEnabled) {
            auto taskIcon = CCSprite::create("task_btn.png"_spr);
            taskIcon->setScale(0.9f);
            auto taskBtn = CCMenuItemSpriteExtra::create(
                taskIcon, this, menu_selector(StreakMainLayer::onOpenTasks)
            );
            taskBtn->setPosition({ rightX, sideY - sideSpacing * 2 });
            cornerMenu->addChild(taskBtn);
        }
        else if (g_streakData.isDiscordGoalEnabled) {
            auto dcIcon = CCSprite::create("discord_goal_btn.png"_spr);
            if (!dcIcon) dcIcon = CCSprite::createWithSpriteFrameName("GJ_chatBtn_001.png");
            dcIcon->setScale(0.9f);
            auto dcBtn = CCMenuItemSpriteExtra::create(
                dcIcon, this, menu_selector(StreakMainLayer::onOpenDiscordGoal)
            );
            dcBtn->setPosition({ rightX, sideY - sideSpacing * 2 });
            cornerMenu->addChild(dcBtn);
        }

        auto missionsIcon = CCSprite::create("super_star_btn.png"_spr);
        missionsIcon->setScale(0.9f);
        auto missionsBtn = CCMenuItemSpriteExtra::create(
            missionsIcon, this, menu_selector(StreakMainLayer::onOpenMissions)
        );
        missionsBtn->setPosition({ leftX, sideY });
        cornerMenu->addChild(missionsBtn);
        m_missionsBtnRef = missionsBtn;

        auto rouletteIcon = CCSprite::create("boton_ruleta.png"_spr);
        rouletteIcon->setScale(0.9f);
        auto rouletteBtn = CCMenuItemSpriteExtra::create(
            rouletteIcon, this, menu_selector(StreakMainLayer::onOpenRoulette)
        );
        rouletteBtn->setPosition({ leftX, sideY - sideSpacing });
        cornerMenu->addChild(rouletteBtn);

        auto xpBtnIcon = CCSprite::create("xp_btn.png"_spr);
        xpBtnIcon->setScale(0.9f);
        auto xpBtn = CCMenuItemSpriteExtra::create(
            xpBtnIcon, this, menu_selector(StreakMainLayer::onOpenXP)
        );
        xpBtn->setPosition({ leftX, sideY - sideSpacing * 2 });
        cornerMenu->addChild(xpBtn);
        m_xpBtnRef = xpBtn;

        // Streak rank button (left column, below xp)
        auto rankIcon = CCSprite::create("rank_btn.png"_spr);
        if (!rankIcon) rankIcon = CCSprite::createWithSpriteFrameName("GJ_starsIcon_001.png");
        rankIcon->setScale(0.9f); // 160px source -> ~51px, matches the side column
        auto rankBtn = CCMenuItemSpriteExtra::create(
            rankIcon, this, menu_selector(StreakMainLayer::onOpenRank)
        );
        rankBtn->setPosition({ leftX, sideY - sideSpacing * 3 });
        cornerMenu->addChild(rankBtn);

        // bottom paginated menu
        std::vector<CCMenuItemSpriteExtra*> allBottomBtns;

        auto eventIcon = CCSprite::create("event_boton.png"_spr);
        if (!eventIcon || eventIcon->getContentSize().width == 0) {
            eventIcon = CCSprite::createWithSpriteFrameName("GJ_top100Btn_001.png");
        }
        eventIcon->setScale(0.9f);
        allBottomBtns.push_back(CCMenuItemSpriteExtra::create(
            eventIcon, this, menu_selector(StreakMainLayer::onOpenEvent)
        ));

        auto keysEventIcon = CCSprite::create("limbo_btn.png"_spr);
        if (!keysEventIcon || keysEventIcon->getContentSize().width == 0) {
            keysEventIcon = CCSprite::createWithSpriteFrameName("GJ_secretChest_001.png");
        }
        if (keysEventIcon) {
            float w = keysEventIcon->getContentSize().width;
            float targetW = 50.f;
            float scale = (w > 0) ? (targetW / w) : 0.9f;
            if (scale > 0.9f) scale = 0.9f;
            keysEventIcon->setScale(scale);
        }
        allBottomBtns.push_back(CCMenuItemSpriteExtra::create(
            keysEventIcon, this, menu_selector(StreakMainLayer::onOpenKeysEvent)
        ));

        auto stIcon = CCSprite::create("pass_btn.png"_spr);
        if (!stIcon) stIcon = ButtonSprite::create("St");
        else stIcon->setScale(0.9f);
        auto stBtn = CCMenuItemSpriteExtra::create(
            stIcon, this, menu_selector(StreakMainLayer::onOpenStProgress)
        );
        m_passBtnRef = stBtn;
        allBottomBtns.push_back(stBtn);

        auto levelProgIcon = CCSprite::create("level_progess_btn.png"_spr);
        if (!levelProgIcon) levelProgIcon = ButtonSprite::create("Lvls");
        else levelProgIcon->setScale(0.9f);
        auto levelProgBtn = CCMenuItemSpriteExtra::create(
            levelProgIcon, this, menu_selector(StreakMainLayer::onOpenLevelProgress)
        );
        applyGDPSDisable(levelProgBtn);
        allBottomBtns.push_back(levelProgBtn);
        m_levelProgBtnRef = levelProgBtn;

        auto topIcon = CCSprite::create("top_btn.png"_spr);
        if (!topIcon) topIcon = ButtonSprite::create("Top");
        else topIcon->setScale(0.9f);
        auto topBtn = CCMenuItemSpriteExtra::create(
            topIcon, this, menu_selector(StreakMainLayer::onOpenLeaderboard)
        );
        applyGDPSDisable(topBtn);
        allBottomBtns.push_back(topBtn);

        auto msgIcon = CCSprite::create("msm.png"_spr);
        if (!msgIcon) msgIcon = CCSprite::createWithSpriteFrameName("GJ_chatBtn_001.png");
        msgIcon->setScale(0.9f);
        auto msgBtn = CCMenuItemSpriteExtra::create(
            msgIcon, this, menu_selector(StreakMainLayer::onOpenMessages)
        );
        allBottomBtns.push_back(msgBtn);
        m_msgBtnRef = msgBtn;

        auto redeemIcon = CCSprite::create("redemcode_btn.png"_spr);
        if (!redeemIcon) redeemIcon = ButtonSprite::create("Code");
        else redeemIcon->setScale(0.9f);
        allBottomBtns.push_back(CCMenuItemSpriteExtra::create(
            redeemIcon, this, menu_selector(StreakMainLayer::onRedeemCode)
        ));

        auto shopIcon = CCSprite::create("daily_shop.png"_spr);
        if (!shopIcon) shopIcon = ButtonSprite::create("Shop");
        else shopIcon->setScale(0.9f);
        allBottomBtns.push_back(CCMenuItemSpriteExtra::create(
            shopIcon, this, menu_selector(StreakMainLayer::onOpenDailyShop)
        ));

        auto kofiIcon = CCSprite::create("ko-fi_btn.png"_spr);
        if (!kofiIcon) kofiIcon = ButtonSprite::create("Donate");
        else kofiIcon->setScale(0.9f);
        allBottomBtns.push_back(CCMenuItemSpriteExtra::create(
            kofiIcon, this, menu_selector(StreakMainLayer::onOpenDonations)
        ));

        auto achievementsIcon = CCSprite::create("achievements_btn.png"_spr);
        if (!achievementsIcon) achievementsIcon = ButtonSprite::create("Logros");
        else achievementsIcon->setScale(0.9f);
        allBottomBtns.push_back(CCMenuItemSpriteExtra::create(
            achievementsIcon, this, menu_selector(StreakMainLayer::onOpenAchievements)
        ));

        auto trendIcon = CCSprite::create("tendencies_btn.png"_spr);
        if (!trendIcon) trendIcon = ButtonSprite::create("Trend");
        else trendIcon->setScale(0.9f);
        auto trendBtn = CCMenuItemSpriteExtra::create(
            trendIcon, this, menu_selector(StreakMainLayer::onOpenTrending)
        );
        applyGDPSDisable(trendBtn);
        allBottomBtns.push_back(trendBtn);

        int btnsPerPage = 6;
        int totalPages = (int)std::ceil((float)allBottomBtns.size() / btnsPerPage);

        for (int i = 0; i < totalPages; ++i) {
            auto pageMenu = CCMenu::create();
            pageMenu->setPosition({ m_size.width / 2, 40.f });

            for (int j = 0; j < btnsPerPage; ++j) {
                int index = (i * btnsPerPage) + j;
                if (index < (int)allBottomBtns.size()) {
                    pageMenu->addChild(allBottomBtns[index]);
                }
            }

            pageMenu->alignItemsHorizontallyWithPadding(8.f);
            pageMenu->setVisible(i == 0);
            this->addChild(pageMenu, 10);
            m_bottomPages.push_back(pageMenu);
        }

        auto arrowMenu = CCMenu::create();
        arrowMenu->setPosition({ 0, 0 });
        this->addChild(arrowMenu, 15);

        auto leftSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        leftSpr->setScale(0.7f);
        m_leftArrowBtn = CCMenuItemSpriteExtra::create(
            leftSpr, this, menu_selector(StreakMainLayer::onPrevPage)
        );
        // Tucked in close to the central button row (instead of the screen edges)
        // so they don't overlap the side column buttons.
        m_leftArrowBtn->setPosition({ m_size.width / 2 - 168.f, 40.f });
        arrowMenu->addChild(m_leftArrowBtn);

        auto rightSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        rightSpr->setFlipX(true);
        rightSpr->setScale(0.7f);
        m_rightArrowBtn = CCMenuItemSpriteExtra::create(
            rightSpr, this, menu_selector(StreakMainLayer::onNextPage)
        );
        m_rightArrowBtn->setPosition({ m_size.width / 2 + 168.f, 40.f });
        arrowMenu->addChild(m_rightArrowBtn);

        updateArrowVisibility();
        this->updateDisplay();

        if (g_streakData.pendingRankAnim) {
            int oldIdx = g_streakData.pendingRankAnimOld;
            int newIdx = StreakData::getRankIndexForTokens(g_streakData.streakTokens);
            g_streakData.pendingRankAnim = false;

            matjson::Value payload = matjson::Value::object();
            claimOnServer("/rank/anim/ack", payload, [](bool) {});

            Loader::get()->queueInMainThread([this, oldIdx, newIdx] {
                if (auto anim = RankDemotionAnimation::create(oldIdx, newIdx)) {
                    this->addChild(anim, 2000);
                }
            });
        }
        else if (g_streakData.shouldShowAnimation()) {
            bool showAnim = Mod::get()->getSavedValue<bool>("enable_streak_anim", true);
            if (showAnim) this->showStreakAnimation(g_streakData.currentStreak);
            g_streakData.lastStreakAnimated = g_streakData.currentStreak;
            g_streakData.save();
        }

        Loader::get()->queueInMainThread([this] { this->checkAllNotifications(); });
        Loader::get()->queueInMainThread([this] { this->checkNewMessages(); });

        this->schedule(schedule_selector(StreakMainLayer::refreshTimer), 0.2f);

        return true;
    }

    void onNextPage(CCObject*) {
        if (m_currentPage < (int)m_bottomPages.size() - 1) {
            m_bottomPages[m_currentPage]->setVisible(false);
            m_currentPage++;
            m_bottomPages[m_currentPage]->setVisible(true);
            updateArrowVisibility();
        }
    }

    void onPrevPage(CCObject*) {
        if (m_currentPage > 0) {
            m_bottomPages[m_currentPage]->setVisible(false);
            m_currentPage--;
            m_bottomPages[m_currentPage]->setVisible(true);
            updateArrowVisibility();
        }
    }

    void updateArrowVisibility() {
        if (m_leftArrowBtn)  m_leftArrowBtn->setVisible(m_currentPage > 0);
        if (m_rightArrowBtn) m_rightArrowBtn->setVisible(m_currentPage < (int)m_bottomPages.size() - 1);
    }

    void updateDisplay() {
        if (!m_streakLabel || !m_streakBar || !m_barText) return;

        int currentStreak = g_streakData.currentStreak;
        int pointsToday = g_streakData.streakPointsToday;
        int requiredPoints = g_streakData.getRequiredPoints();

        float percent = 0.0f;
        if (requiredPoints > 0) {
            percent = static_cast<float>(pointsToday) / static_cast<float>(requiredPoints);
        }
        percent = std::clamp(percent, 0.0f, 1.0f);

        m_streakLabel->setString(fmt::format("Daily streak: {}", currentStreak).c_str());

        if (m_streakBar) {
            m_streakBar->setProgress(percent);
            if (percent >= 1.0f) {
                m_streakBar->setRainbowMode(true);
            }
            else {
                m_streakBar->setRainbowMode(false);
                m_streakBar->setGradientColors({ 250, 225, 60 }, { 255, 165, 0 });
            }
        }

        m_barText->setString(fmt::format("{}/{}", pointsToday, requiredPoints).c_str());

        if (m_shieldsLabel) {
            m_shieldsLabel->setString(std::to_string(g_streakData.streakShields).c_str());
        }

        if (m_gemsLabel) {
            m_gemsLabel->setString(std::to_string(g_streakData.gems).c_str());
        }

        layoutCounters();

        if (m_xpBar && m_xpLabel && m_xpProgressLabel) {
            float xpPercent = 0.0f;

            if (g_streakData.currentLevel >= 100) {
                xpPercent = 1.0f;
                m_xpProgressLabel->setString("MAX");
            }
            else {
                xpPercent = g_streakData.getXPPercentage();
                int currentXP = g_streakData.currentXP;
                int requiredXP = g_streakData.getXPRequiredForNextLevel();
                m_xpProgressLabel->setString(fmt::format("{}/{}", currentXP, requiredXP).c_str());
            }

            m_xpBar->setProgress(xpPercent);
            m_xpLabel->setString(fmt::format("Lvl. {}", g_streakData.currentLevel).c_str());
        }

        refreshRedTags();
    }

    void onOpenAccount(CCObject*) {
        ProfileData myData;
        auto am = GJAccountManager::sharedState();

        myData.username = am->m_username.empty() ? "Player" : am->m_username;
        g_streakData.load();
        myData.currentStreak = g_streakData.currentStreak;
        myData.level = g_streakData.currentLevel;
        myData.currentXP = g_streakData.currentXP;
        myData.totalSP = g_streakData.totalStreakPoints;
        myData.superStars = g_streakData.superStars;
        myData.starTickets = g_streakData.starTickets;
        myData.gems = g_streakData.gems;
        myData.bannerID = g_streakData.equippedBanner;
        myData.badgeID = g_streakData.equippedBadge;
        myData.streakID = g_streakData.streakID;
        myData.globalRank = g_streakData.globalRank;
        myData.nameColor = g_streakData.equippedNameColor;
        myData.nameFont = g_streakData.equippedNameFont;
        myData.nameEffect = g_streakData.equippedNameEffect;
        myData.nameAnimation = g_streakData.equippedNameAnimation;
        ProfileCardPopup::create(myData)->show();
    }

    void onOpenStProgress(CCObject*) {
        if (!g_streakData.passEnabled) {
            FLAlertLayer::create(
                "Streak Pass",
                "The pass is currently <cy>under maintenance</c>.\nCheck back in a moment.",
                "OK"
            )->show();
            return;
        }
        if (!g_streakData.isPassActive()) {
            FLAlertLayer::create(
                "Streak Pass",
                "The current pass isn't available for this version. <cy>Update the mod</c> to access the latest pass.",
                "OK"
            )->show();
            return;
        }
        StProgressPopup::create()->show();
    }
    void onOpenStats(CCObject*)      { DayProgressPopup::create()->show(); }
    void onOpenRewards(CCObject*)    { RewardsPopup::create()->show(); }
    void onOpenTasks(CCObject*)      { TaskPopup::create()->show(); }
    void onOpenDiscordGoal(CCObject*){ DiscordGoalPopup::create()->show(); }
    void onOpenSettings(CCObject*)   { SettingsPopup::create()->show(); }
    void onRachaClick(CCObject*)     { AllRachasPopup::create()->show(); }

    void onOpenMissions(CCObject*) {
        MissionsPopup::create([this]() { this->updateDisplay(); })->show();
    }

    void onRedeemCode(CCObject*) {
        RedeemCodePopup::create([this]() { this->updateDisplay(); })->show();
    }

    void onOpenLeaderboard(CCObject*) {
        if (g_streakData.currentLevel < 7) {
            LevelLockPopup::create()->show();
            return;
        }
        LeaderboardPopup::create()->show();
    }

    void onOpenRank(CCObject*) {
        RankPopup::create()->show();
    }

    void onOpenMessages(CCObject*)   { SendMessagePopup::create()->show(); }
    void onOpenDonations(CCObject*)  { DonationPopup::create()->show(); }
    void onOpenEvent(CCObject*)      { EventPopup::create()->show(); }
    void onOpenKeysEvent(CCObject*)  { KeysEventPopup::create()->show(); }
    void onOpenLevelProgress(CCObject*) { LevelProgressPopup::create()->show(); }
    void onOpenXP(CCObject*)         { XPPopup::create()->show(); }
    void onOpenDailyShop(CCObject*)  { DailyShopPopup::create()->show(); }
    void onOpenAchievements(CCObject*) { AchievementsPopup::create()->show(); }
    void onOpenTrending(CCObject*)   { TrendLevelsPopup::create()->show(); }
    void onOpenUpdate(CCObject*)     { UpdatePopup::create()->show(); }

    void onOpenRoulette(CCObject*) {
        if (g_streakData.currentStreak < 1) {
            FLAlertLayer::create(
                "Roulette Locked",
                "You need a streak of at least <cg>1 day</c>.",
                "OK"
            )->show();
            return;
        }
        RoulettePopup::create()->show();
    }

    void showStreakAnimation(int streakLevel) {
        auto winSize = CCDirector::sharedDirector()->getWinSize();

        auto bgLayer = CCLayerColor::create({ 0, 0, 0, 0 });
        bgLayer->setTag(110);
        this->addChild(bgLayer, 1000);
        bgLayer->runAction(CCFadeTo::create(0.5f, 200));

        auto contentLayer = CCLayer::create();
        contentLayer->setTag(111);
        contentLayer->ignoreAnchorPointForPosition(false);
        contentLayer->setAnchorPoint({ 0.5f, 0.5f });
        contentLayer->setPosition(winSize / 2);
        contentLayer->setContentSize(winSize);
        this->addChild(contentLayer, 1001);

        auto shineGlow = CCSprite::createWithSpriteFrameName("particle_171_001.png");
        if (shineGlow) {
            shineGlow->setPosition(winSize / 2);
            shineGlow->setScale(0.0f);
            shineGlow->setColor({ 255, 255, 255 });
            shineGlow->setOpacity(200);
            shineGlow->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });
            contentLayer->addChild(shineGlow, 1);

            shineGlow->runAction(CCSequence::create(
                CCDelayTime::create(0.1f),
                CCEaseSineOut::create(CCScaleTo::create(1.0f, 7.5f)),
                nullptr
            ));
            shineGlow->runAction(CCRepeatForever::create(CCRotateBy::create(15.0f, 360.f)));
        }

        auto titleSprite = CCSprite::create("NewStreak.png"_spr);
        if (titleSprite) {
            titleSprite->setPosition({ winSize.width / 2, winSize.height / 2 + 110.f });
            titleSprite->setScale(0.0f);
            contentLayer->addChild(titleSprite, 3);

            titleSprite->runAction(CCSequence::create(
                CCDelayTime::create(0.2f),
                CCEaseBackOut::create(CCScaleTo::create(0.5f, 1.0f)),
                nullptr
            ));
            titleSprite->runAction(CCSequence::create(
                CCDelayTime::create(1.0f),
                CCRepeatForever::create(CCSequence::create(
                    CCEaseSineInOut::create(CCScaleTo::create(1.5f, 1.1f)),
                    CCEaseSineInOut::create(CCScaleTo::create(1.5f, 1.0f)),
                    nullptr
                )),
                nullptr
            ));
        }

        auto rachaSprite = CCSprite::create(g_streakData.getRachaSprite().c_str());
        if (rachaSprite) {
            rachaSprite->setPosition(winSize / 2);
            rachaSprite->setScale(0.0f);
            contentLayer->addChild(rachaSprite, 2);

            rachaSprite->runAction(CCSequence::create(
                CCDelayTime::create(0.3f),
                CCEaseElasticOut::create(CCScaleTo::create(1.2f, 1.0f), 0.6f),
                nullptr
            ));
            rachaSprite->runAction(CCSequence::create(
                CCDelayTime::create(1.5f),
                CCRepeatForever::create(CCSequence::create(
                    CCMoveBy::create(1.5f, { 0, 15.f }),
                    CCMoveBy::create(1.5f, { 0, -15.f }),
                    nullptr
                )),
                nullptr
            ));
            rachaSprite->runAction(CCSequence::create(
                CCDelayTime::create(1.5f),
                CCRepeatForever::create(CCSequence::create(
                    CCEaseSineInOut::create(CCScaleTo::create(1.5f, 1.15f)),
                    CCEaseSineInOut::create(CCScaleTo::create(1.5f, 1.0f)),
                    nullptr
                )),
                nullptr
            ));
        }

        auto daysLabel = CCLabelBMFont::create(
            fmt::format("Day {}!", streakLevel).c_str(),
            "goldFont.fnt"
        );
        daysLabel->setPosition({ winSize.width / 2, winSize.height / 2 - 120.f });
        daysLabel->setScale(0.0f);
        contentLayer->addChild(daysLabel, 3);

        daysLabel->runAction(CCSequence::create(
            CCDelayTime::create(0.8f),
            CCEaseBackOut::create(CCScaleTo::create(0.5f, 1.0f)),
            nullptr
        ));

        FMODAudioEngine::sharedEngine()->playEffect("achievement.mp3"_spr);

        contentLayer->runAction(CCSequence::create(
            CCDelayTime::create(0.5f),
            CCCallFunc::create(this, callfunc_selector(StreakMainLayer::playExtraStreakSound)),
            nullptr
        ));
        contentLayer->runAction(CCSequence::create(
            CCDelayTime::create(6.0f),
            CCCallFunc::create(this, callfunc_selector(StreakMainLayer::onAnimationExit)),
            nullptr
        ));
    }

    void playExtraStreakSound() {
        FMODAudioEngine::sharedEngine()->playEffect("mcsfx.mp3"_spr);
    }

    void onAnimationExit() {
        if (auto bgLayer = this->getChildByTag(110)) {
            bgLayer->runAction(CCSequence::create(
                CCFadeOut::create(0.5f),
                CCRemoveSelf::create(),
                nullptr
            ));
        }
        if (auto contentLayer = this->getChildByTag(111)) {
            contentLayer->runAction(CCSequence::create(
                CCSpawn::create(
                    CCFadeOut::create(0.5f),
                    CCEaseBackIn::create(CCScaleTo::create(0.5f, 0.0f)),
                    nullptr
                ),
                CCRemoveSelf::create(),
                nullptr
            ));
        }
    }

    void onBack(CCObject*) {
        CCDirector::sharedDirector()->popSceneWithTransition(0.5f, PopTransition::kPopTransitionFade);
    }

    void keyBackClicked() override {
        onBack(nullptr);
    }

    void onEnter() override {
        CCLayer::onEnter();
        StreakMusic::onMenuEnter();
    }

    void onExit() override {
        StreakMusic::onMenuExit();
        CCLayer::onExit();
    }

public:
    static StreakMainLayer* create() {
        auto ret = new StreakMainLayer();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    static cocos2d::CCScene* scene() {
        auto sc = cocos2d::CCScene::create();
        auto layer = StreakMainLayer::create();
        sc->addChild(layer);
        return sc;
    }
};

int StreakMainLayer::s_lastPendingLevelCount = 0;
int StreakMainLayer::s_lastPendingDailyCount = 0;
int StreakMainLayer::s_lastNewMsgCount = 0;
