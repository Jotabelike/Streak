#pragma once
#include "StreakCommon.h"
#include "../StreakData.h"
#include "../FirebaseManager.h"
#include <Geode/ui/Popup.hpp>
#include <Geode/utils/async.hpp>
#include "HistoryPopup.h"
#include "StatsPopups.h"
#include "CollectionPopups.h"
#include "MissionsPopup.h"
#include "AdminPopups.h"
#include "LeaderboardPopup.h"
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
#include "../utils/RoundedProgressBar.h"
#include "DiscordGoalPopup.h"
#include "RegisterPopup.h"
#include "ShieldsPopup.h"

class InfoPopup : public Popup {
protected:
    CCLabelBMFont* m_streakLabel = nullptr;
    RoundedProgressBar* m_streakBar = nullptr;
    CCLabelBMFont* m_barText = nullptr;
    RoundedProgressBar* m_xpBar = nullptr;
    CCLabelBMFont* m_xpLabel = nullptr;
    CCLabelBMFont* m_xpProgressLabel = nullptr;
    CCLabelBMFont* m_gemsLabel = nullptr;
    CCLabelBMFont* m_shieldsLabel = nullptr;

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
 

    void checkAllNotifications() {
        int currentPendingLevel = 0;

        for (const auto& mission : g_levelMissions) {
            if (g_streakData.isLevelMissionClaimed(mission.levelID)) {
                continue;
            }
            auto level = GameLevelManager::sharedState()->getSavedLevel(mission.levelID);
            if (level && level->m_normalPercent >= 100) {
                currentPendingLevel++;
            }
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
        if (!res.ok() || !res.json().isOk()) {
            return;
        }

        auto data = res.json().unwrap();
        if (!data.isArray()) {
            return;
        }

        auto messages = data.as<std::vector<matjson::Value>>().unwrap();
        double savedTime = Mod::get()->getSavedValue<double>("streak_last_chat_time", 0.0);
        long long lastSeenTime = (long long)savedTime;
        int currentNewCount = 0;

        for (const auto& msg : messages) {
            long long timestamp = msg["timestamp"].as<long long>().unwrapOr(0);
            if (timestamp > lastSeenTime) {
                currentNewCount++;
            }
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
        case 1:
            title = "Moderator";
            desc = "This user is a <cg>Moderator</c> of the Streak Mod.";
            break;
        case 2:
            title = "Content Creator";
            desc = "This user is a recognized <cp>Content Creator</c>!";
            break;
        case 3:
            title = "V.I.P";
            desc = "This user is a <cy>V.I.P Member</c> with exclusive status.";
            break;
        case 4:
            title = "Stellar";
            desc = "Legendary <cp>Stellar</c> User.";
            break;
        default:
            desc = "Special user rank.";
            break;
        }

        FLAlertLayer::create(title.c_str(), desc.c_str(), "OK")->show();
    }

  

    void applyGDPSDisable(CCMenuItemSpriteExtra* btn) {
        auto am = GJAccountManager::sharedState();
        if (!am) return;  

        std::string gdpsKey = fmt::format("is_gdps_player_{}", am->m_accountID);
        bool isGDPS = geode::Mod::get()->getSavedValue<bool>(gdpsKey, g_streakData.isGDPS);

        if (!isGDPS) {
            return;
        }
        btn->setEnabled(false);
        btn->setOpacity(60);
        btn->setColor({ 120, 120, 120 });
    }

    bool init() override {
        if (!Popup::init(280.f, 220.f, "geode.loader/GE_square03.png")) {
            return false;
        }

        auto am = GJAccountManager::sharedState();
        auto winSize = m_mainLayer->getContentSize();

        if (!am || am->m_accountID == 0) {
            this->setTitle("Error");

            auto errorLabel = CCLabelBMFont::create(
                "Please log in to\nGeometry Dash first!",
                "bigFont.fnt"
            );
            errorLabel->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
            errorLabel->setScale(0.5f);
            errorLabel->setPosition(winSize / 2);
            errorLabel->setColor({ 255, 100, 100 });
            m_mainLayer->addChild(errorLabel);
            return true;
        }

        this->setTitle("Streak");

      
        auto heartSpr = CCSprite::create("heart.png"_spr);
        if (heartSpr) {
            heartSpr->setScale(0.18f);
            auto shieldsBtn = CCMenuItemSpriteExtra::create(
                heartSpr, this, menu_selector(InfoPopup::onShieldsClick)
            );
            shieldsBtn->setPosition({ 25.0f, winSize.height - 25.0f });

            auto shieldsMenu = CCMenu::create();
            shieldsMenu->setPosition({ 0, 0 });
            shieldsMenu->addChild(shieldsBtn);
            m_mainLayer->addChild(shieldsMenu, 10);

            m_shieldsLabel = CCLabelBMFont::create("0/5", "bigFont.fnt");
            m_shieldsLabel->setScale(0.5f);
            m_shieldsLabel->setAnchorPoint({ 0.0f, 0.5f });
            m_shieldsLabel->setColor({ 255, 130, 130 });
            m_shieldsLabel->setPosition({ 45.0f, winSize.height - 25.0f });
            m_mainLayer->addChild(m_shieldsLabel, 10);
        }

        float contentCenterY = winSize.height / 2 + 30.0f;

     
        auto rachaSprite = CCSprite::create(g_streakData.getRachaSprite().c_str());
        if (rachaSprite) {
            rachaSprite->setScale(0.35f);

            auto rachaBtn = CCMenuItemSpriteExtra::create(
                rachaSprite,
                this,
                menu_selector(InfoPopup::onRachaClick)
            );
            auto menuRacha = CCMenu::createWithItem(rachaBtn);
            menuRacha->setPosition({ winSize.width / 2, contentCenterY });
            m_mainLayer->addChild(menuRacha, 3);

            StreakAnimations::applyPremiumHover(rachaSprite);
        }
 
        float barWidth = 140.0f;
        float barHeight = 20.0f;
        float barY = contentCenterY - 78.0f;

        m_streakLabel = CCLabelBMFont::create("Daily streak: ?", "goldFont.fnt");
        m_streakLabel->setScale(0.50f);
        m_streakLabel->setPosition({ winSize.width / 2, contentCenterY - 45 });
        m_mainLayer->addChild(m_streakLabel);

        m_streakBar = RoundedProgressBar::create(barWidth, barHeight);
        m_streakBar->setPosition({ winSize.width / 2, barY + (barHeight / 2) });
        m_streakBar->setGradientColors({ 250, 225, 60 }, { 255, 165, 0 });
        m_streakBar->setBackgroundColor({ 45, 45, 45 });
        m_mainLayer->addChild(m_streakBar, 1);

        m_barText = CCLabelBMFont::create("? / ?", "bigFont.fnt");
        m_barText->setScale(0.40f);
        m_barText->setPosition({ winSize.width / 2, barY + (barHeight / 2) });
        m_mainLayer->addChild(m_barText, 8);

      
        float xpBarHeight = 10.0f;
        float xpY = barY - 12.0f;

        m_xpBar = RoundedProgressBar::create(barWidth, xpBarHeight);
        m_xpBar->setPosition({ winSize.width / 2, xpY + (xpBarHeight / 2) });
        m_xpBar->setGradientColors({ 0, 255, 255 }, { 0, 100, 255 });
        m_xpBar->setBackgroundColor({ 20, 20, 40 });
        m_mainLayer->addChild(m_xpBar, 1);

        m_xpLabel = CCLabelBMFont::create("Lvl. ?", "goldFont.fnt");
        m_xpLabel->setScale(0.35f);
        m_xpLabel->setPosition({ winSize.width / 2 - barWidth / 2 - 20, xpY + (xpBarHeight / 2) });
        m_mainLayer->addChild(m_xpLabel);

        m_xpProgressLabel = CCLabelBMFont::create("0/0", "chatFont.fnt");
        m_xpProgressLabel->setScale(0.35f);
        m_xpProgressLabel->setPosition({ winSize.width / 2, xpY + (xpBarHeight / 2) });
        m_mainLayer->addChild(m_xpProgressLabel, 8);

       
        float sideBtnY = winSize.height / 2 + 30.0f;
        auto cornerMenu = CCMenu::create();
        cornerMenu->setPosition(0, 0);
        m_mainLayer->addChild(cornerMenu, 10);

        auto statsIcon = CCSprite::create("BtnStats.png"_spr);
        statsIcon->setScale(0.7f);
        auto statsBtn = CCMenuItemSpriteExtra::create(
            statsIcon, this, menu_selector(InfoPopup::onOpenStats)
        );
        statsBtn->setPosition({ winSize.width - 22, sideBtnY + 15 });
        applyGDPSDisable(statsBtn);
        cornerMenu->addChild(statsBtn);

        auto rewardsIcon = CCSprite::create("RewardsBtn.png"_spr);
        rewardsIcon->setScale(0.7f);
        auto rewardsBtn = CCMenuItemSpriteExtra::create(
            rewardsIcon, this, menu_selector(InfoPopup::onOpenRewards)
        );
        rewardsBtn->setPosition({ winSize.width - 22, sideBtnY - 22 });
        cornerMenu->addChild(rewardsBtn);

        if (g_streakData.isTaskEnabled) {
            auto taskIcon = CCSprite::create("task_btn.png"_spr);
            taskIcon->setScale(0.7f);
            auto taskBtn = CCMenuItemSpriteExtra::create(
                taskIcon, this, menu_selector(InfoPopup::onOpenTasks)
            );
            taskBtn->setPosition({ winSize.width - 22, sideBtnY - 59 });
            cornerMenu->addChild(taskBtn);
        }
        else if (g_streakData.isDiscordGoalEnabled) {
            auto dcIcon = CCSprite::create("discord_goal_btn.png"_spr);
            if (!dcIcon) {
                dcIcon = CCSprite::createWithSpriteFrameName("GJ_chatBtn_001.png");
            }
            dcIcon->setScale(0.7f);
            auto dcBtn = CCMenuItemSpriteExtra::create(
                dcIcon, this, menu_selector(InfoPopup::onOpenDiscordGoal)
            );
            dcBtn->setPosition({ winSize.width - 22, sideBtnY - 59 });
            cornerMenu->addChild(dcBtn);
        }

        auto missionsIcon = CCSprite::create("super_star_btn.png"_spr);
        missionsIcon->setScale(0.7f);
        auto missionsBtn = CCMenuItemSpriteExtra::create(
            missionsIcon, this, menu_selector(InfoPopup::onOpenMissions)
        );
        missionsBtn->setPosition({ 22, sideBtnY + 15 });
        cornerMenu->addChild(missionsBtn);

        auto rouletteIcon = CCSprite::create("boton_ruleta.png"_spr);
        rouletteIcon->setScale(0.7f);
        auto rouletteBtn = CCMenuItemSpriteExtra::create(
            rouletteIcon, this, menu_selector(InfoPopup::onOpenRoulette)
        );
        rouletteBtn->setPosition({ 22, sideBtnY - 22 });
        cornerMenu->addChild(rouletteBtn);

        auto xpBtnIcon = CCSprite::create("xp_btn.png"_spr);
        xpBtnIcon->setScale(0.7f);
        auto xpBtn = CCMenuItemSpriteExtra::create(
            xpBtnIcon, this, menu_selector(InfoPopup::onOpenXP)
        );
        xpBtn->setPosition({ 22, sideBtnY - 59 });
        cornerMenu->addChild(xpBtn);
 
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
            if (!rankSprite) {
                rankSprite = CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
            }
            rankSprite->setScale(0.2f);

            auto rankBtn = CCMenuItemSpriteExtra::create(
                rankSprite, this, menu_selector(InfoPopup::onRankClick)
            );
            rankBtn->setTag(g_streakData.specialRank);
            rankBtn->setPosition({ winSize.width - 80, winSize.height - 20 });
            cornerMenu->addChild(rankBtn);
        }

        auto accountIcon = CCSprite::create("account_btn.png"_spr);
        accountIcon->setScale(0.56f);
        auto accountBtn = CCMenuItemSpriteExtra::create(
            accountIcon, this, menu_selector(InfoPopup::onOpenAccount)
        );
        accountBtn->setPosition({ winSize.width - 50, winSize.height - 20 });
        cornerMenu->addChild(accountBtn);

        auto settingsIcon = CCSprite::createWithSpriteFrameName("accountBtn_settings_001.png");
        settingsIcon->setScale(0.6f);
        auto settingsBtn = CCMenuItemSpriteExtra::create(
            settingsIcon, this, menu_selector(InfoPopup::onOpenSettings)
        );
        settingsBtn->setPosition({ winSize.width - 20, winSize.height - 20 });
        cornerMenu->addChild(settingsBtn);

     
        std::vector<CCMenuItemSpriteExtra*> allBottomBtns;

        auto eventIcon = CCSprite::create("event_boton.png"_spr);
        if (!eventIcon || eventIcon->getContentSize().width == 0) {
            eventIcon = CCSprite::createWithSpriteFrameName("GJ_top100Btn_001.png");
        }
        eventIcon->setScale(0.7f);
        allBottomBtns.push_back(CCMenuItemSpriteExtra::create(
            eventIcon, this, menu_selector(InfoPopup::onOpenEvent)
        ));

        auto keysEventIcon = CCSprite::create("limbo_btn.png"_spr);   
        if (!keysEventIcon || keysEventIcon->getContentSize().width == 0) {
            keysEventIcon = CCSprite::createWithSpriteFrameName("GJ_secretChest_001.png"); 
        }
        if (keysEventIcon) {
            float w = keysEventIcon->getContentSize().width;
            float targetW = 38.f;
            float scale = (w > 0) ? (targetW / w) : 0.7f;
            if (scale > 0.7f) scale = 0.7f;
            keysEventIcon->setScale(scale);
        }
        allBottomBtns.push_back(CCMenuItemSpriteExtra::create(
            keysEventIcon, this, menu_selector(InfoPopup::onOpenKeysEvent)
        ));

        auto stIcon = CCSprite::create("st_progress.png"_spr);
        if (!stIcon) {
            stIcon = ButtonSprite::create("St");
        }
        else {
            stIcon->setScale(0.7f);
        }
        allBottomBtns.push_back(CCMenuItemSpriteExtra::create(
            stIcon, this, menu_selector(InfoPopup::onOpenStProgress)
        ));

        auto levelProgIcon = CCSprite::create("level_progess_btn.png"_spr);
        if (!levelProgIcon) {
            levelProgIcon = ButtonSprite::create("Lvls");
        }
        else {
            levelProgIcon->setScale(0.7f);
        }
        auto levelProgBtn = CCMenuItemSpriteExtra::create(
            levelProgIcon, this, menu_selector(InfoPopup::onOpenLevelProgress)
        );
        applyGDPSDisable(levelProgBtn);
        allBottomBtns.push_back(levelProgBtn);

        auto topIcon = CCSprite::create("top_btn.png"_spr);
        if (!topIcon) {
            topIcon = ButtonSprite::create("Top");
        }
        else {
            topIcon->setScale(0.7f);
        }
       auto topBtn = CCMenuItemSpriteExtra::create(
            topIcon, this, menu_selector(InfoPopup::onOpenLeaderboard)
        );
        applyGDPSDisable(topBtn);
        allBottomBtns.push_back(topBtn);

        auto msgIcon = CCSprite::create("msm.png"_spr);
        if (!msgIcon) {
            msgIcon = CCSprite::createWithSpriteFrameName("GJ_chatBtn_001.png");
        }
        msgIcon->setScale(0.7f);
        allBottomBtns.push_back(CCMenuItemSpriteExtra::create(
            msgIcon, this, menu_selector(InfoPopup::onOpenMessages)
        ));

        auto redeemIcon = CCSprite::create("redemcode_btn.png"_spr);
        if (!redeemIcon) {
            redeemIcon = ButtonSprite::create("Code");
        }
        redeemIcon->setScale(0.7f);
        allBottomBtns.push_back(CCMenuItemSpriteExtra::create(
            redeemIcon, this, menu_selector(InfoPopup::onRedeemCode)
        ));

        auto shopIcon = CCSprite::create("daily_shop.png"_spr);
        if (!shopIcon) {
            shopIcon = ButtonSprite::create("Shop");
        }
        else {
            shopIcon->setScale(0.7f);
        }
        allBottomBtns.push_back(CCMenuItemSpriteExtra::create(
            shopIcon, this, menu_selector(InfoPopup::onOpenDailyShop)
        ));

        auto kofiIcon = CCSprite::create("ko-fi_btn.png"_spr);
        if (!kofiIcon) {
            kofiIcon = ButtonSprite::create("Donate");
        }
        else {
            kofiIcon->setScale(0.7f);
        }
        allBottomBtns.push_back(CCMenuItemSpriteExtra::create(
            kofiIcon, this, menu_selector(InfoPopup::onOpenDonations)
        ));

        auto achievementsIcon = CCSprite::create("achievements_btn.png"_spr);
        if (!achievementsIcon) {
            achievementsIcon = ButtonSprite::create("Logros");
        }
        else {
            achievementsIcon->setScale(0.7f);
        }
        allBottomBtns.push_back(CCMenuItemSpriteExtra::create(
            achievementsIcon, this, menu_selector(InfoPopup::onOpenAchievements)
        ));

        auto trendIcon = CCSprite::create("tendencies_btn.png"_spr);
        if (!trendIcon) {
            trendIcon = ButtonSprite::create("Trend");
        }
        else {
            trendIcon->setScale(0.7f);
        }
        auto trendBtn = CCMenuItemSpriteExtra::create(
            trendIcon, this, menu_selector(InfoPopup::onOpenTrending)
        );
        applyGDPSDisable(trendBtn);
        allBottomBtns.push_back(trendBtn);

        int btnsPerPage = 5;
        int totalPages = std::ceil((float)allBottomBtns.size() / btnsPerPage);

        for (int i = 0; i < totalPages; ++i) {
            auto pageMenu = CCMenu::create();
            pageMenu->setPosition({ winSize.width / 2, 25.f });

            for (int j = 0; j < btnsPerPage; ++j) {
                int index = (i * btnsPerPage) + j;
                if (index < (int)allBottomBtns.size()) {
                    pageMenu->addChild(allBottomBtns[index]);
                }
            }

            pageMenu->alignItemsHorizontallyWithPadding(5.0f);
            pageMenu->setVisible(i == 0);
            m_mainLayer->addChild(pageMenu, 10);
            m_bottomPages.push_back(pageMenu);
        }

    
        auto arrowMenu = CCMenu::create();
        arrowMenu->setPosition({ 0, 0 });
        m_mainLayer->addChild(arrowMenu, 15);

        auto leftSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        leftSpr->setScale(0.5f);
        m_leftArrowBtn = CCMenuItemSpriteExtra::create(
            leftSpr, this, menu_selector(InfoPopup::onPrevPage)
        );
        m_leftArrowBtn->setPosition({ 40.f, 25.f });
        arrowMenu->addChild(m_leftArrowBtn);

        auto rightSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        rightSpr->setFlipX(true);
        rightSpr->setScale(0.5f);
        m_rightArrowBtn = CCMenuItemSpriteExtra::create(
            rightSpr, this, menu_selector(InfoPopup::onNextPage)
        );
        m_rightArrowBtn->setPosition({ winSize.width - 40.f, 25.f });
        arrowMenu->addChild(m_rightArrowBtn);

        updateArrowVisibility();
        this->updateDisplay();

        if (g_streakData.shouldShowAnimation()) {
            bool showAnim = Mod::get()->getSavedValue<bool>("enable_streak_anim", true);
            if (showAnim) {
                this->showStreakAnimation(g_streakData.currentStreak);
            }
            g_streakData.lastStreakAnimated = g_streakData.currentStreak;
            g_streakData.save();
        }

        Loader::get()->queueInMainThread([this] {
            this->checkAllNotifications();
            });

        Loader::get()->queueInMainThread([this] {
            this->checkNewMessages();
            });

        this->schedule(schedule_selector(InfoPopup::refreshTimer), 0.2f);

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
        if (m_leftArrowBtn) {
            m_leftArrowBtn->setVisible(m_currentPage > 0);
        }
        if (m_rightArrowBtn) {
            m_rightArrowBtn->setVisible(m_currentPage < (int)m_bottomPages.size() - 1);
        }
    }

 

    void updateDisplay() {
        if (!m_mainLayer || !m_streakLabel || !m_streakBar || !m_barText) {
            return;
        }

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
            m_shieldsLabel->setString(
                fmt::format("{}/{}", g_streakData.streakShields, STREAK_MAX_SHIELDS).c_str()
            );
        }

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
                m_xpProgressLabel->setString(
                    fmt::format("{}/{}", currentXP, requiredXP).c_str()
                );
            }

            m_xpBar->setProgress(xpPercent);
            m_xpLabel->setString(fmt::format("Lvl. {}", g_streakData.currentLevel).c_str());
        }
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
        StProgressPopup::create()->show();
    }

    void onOpenStats(CCObject*) {
        DayProgressPopup::create()->show();
    }

    void onOpenRewards(CCObject*) {
        RewardsPopup::create()->show();
    }

    void onOpenTasks(CCObject*) {
        TaskPopup::create()->show();
    }

    void onOpenDiscordGoal(CCObject*) {
        DiscordGoalPopup::create()->show();
    }

    void onOpenSettings(CCObject*) {
        SettingsPopup::create()->show();
    }

    void onRachaClick(CCObject*) {
        AllRachasPopup::create()->show();
    }

    void onOpenMissions(CCObject*) {
        MissionsPopup::create([this]() {
            this->updateDisplay();
            })->show();
    }

    void onRedeemCode(CCObject*) {
        RedeemCodePopup::create([this]() {
            this->updateDisplay();
            })->show();
    }

    void onOpenLeaderboard(CCObject*) {
        if (g_streakData.currentLevel < 7) {
            LevelLockPopup::create()->show();
            return;
        }
        LeaderboardPopup::create()->show();
    }

    void onOpenMessages(CCObject*) {
        SendMessagePopup::create()->show();
    }

    void onOpenDonations(CCObject*) {
        DonationPopup::create()->show();
    }

    void onOpenEvent(CCObject*) {
        EventPopup::create()->show();
    }

    void onOpenKeysEvent(CCObject*) {
        KeysEventPopup::create()->show();
    }

    void onOpenLevelProgress(CCObject*) {
        LevelProgressPopup::create()->show();
    }

    void onOpenXP(CCObject*) {
        XPPopup::create()->show();
    }

    void onOpenDailyShop(CCObject*) {
        DailyShopPopup::create()->show();
    }

    void onOpenAchievements(CCObject*) {
        AchievementsPopup::create()->show();
    }

    void onOpenTrending(CCObject*) {
        TrendLevelsPopup::create()->show();
    }

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
            shineGlow->runAction(
                CCRepeatForever::create(CCRotateBy::create(15.0f, 360.f))
            );
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
            CCCallFunc::create(this, callfunc_selector(InfoPopup::playExtraStreakSound)),
            nullptr
        ));
        contentLayer->runAction(CCSequence::create(
            CCDelayTime::create(6.0f),
            CCCallFunc::create(this, callfunc_selector(InfoPopup::onAnimationExit)),
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

    void onClose(CCObject* sender) override {
        Popup::onClose(sender);
    }

 

public:
    void show() override {
        if (g_streakData.needsRegistration) {
            RegisterPopup::create()->show();
            return;
        }
        Popup::show();
    }

    static InfoPopup* create() {
        auto ret = new InfoPopup();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

int InfoPopup::s_lastPendingLevelCount = 0;
int InfoPopup::s_lastPendingDailyCount = 0;
int InfoPopup::s_lastNewMsgCount = 0;
