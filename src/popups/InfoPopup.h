#pragma once
#include "StreakCommon.h"
#include "../StreakData.h"
#include "../FirebaseManager.h"
#include <Geode/ui/Popup.hpp>
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

class InfoPopup : public Popup<> {
protected:
    CCLabelBMFont* m_streakLabel = nullptr;
    CCLayerGradient* m_barFg = nullptr;
    CCLabelBMFont* m_barText = nullptr;
    CCSprite* m_rachaIndicator = nullptr;
    CCSprite* m_pointIcon = nullptr;
    CCLayerGradient* m_xpBarFg = nullptr;
    CCLabelBMFont* m_xpLabel = nullptr;
    CCSprite* m_xpIndicator = nullptr;
    CCLabelBMFont* m_xpProgressLabel = nullptr;
    CCLabelBMFont* m_gemsLabel = nullptr;

    EventListener<web::WebTask> m_msgCheckListener;

    static int s_lastPendingLevelCount;
    static int s_lastPendingDailyCount;
    static int s_lastNewMsgCount;

    void refreshTimer(float dt) {
        this->updateDisplay();
    }

    void onCreateProfile(CCObject*) {
        auto am = GJAccountManager::sharedState();
        if (!am || am->m_accountID == 0) {
            FLAlertLayer::create(
                "Error",
                "Invalid account data. Please log in again.",
                "OK"
            )->show();
            return;
        }
        g_streakData.resetToDefault();
        g_streakData.needsRegistration = false;
        g_streakData.dailyUpdate();
        updatePlayerDataInFirebase();
        this->onClose(nullptr);
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
        m_msgCheckListener.setFilter(req.get(
            "https://streak-servidor.onrender.com/messages"
        ));
    }

    void onMessagesChecked(web::WebTask::Event* e) {
        if (web::WebResponse* res = e->getValue()) {
            if (res->ok() && res->json().isOk()) {
                auto data = res->json().unwrap();
                if (data.isArray()) {
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
                            "Nuevos Anuncios",
                            fmt::format("There are {} new messages", currentNewCount),
                            "msm.png"_spr,
                            0.6f
                        );
                    }
                    s_lastNewMsgCount = currentNewCount;
                }
            }
        }
    }

    bool setup() override {
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

        m_msgCheckListener.bind(this, &InfoPopup::onMessagesChecked);

        if (g_streakData.needsRegistration) {
            this->setTitle("Welcome!");
            auto infoLabel = CCLabelBMFont::create("Join the Streak Mod!", "goldFont.fnt");
            infoLabel->setPosition({ winSize.width / 2, winSize.height / 2 + 25.f });
            infoLabel->setScale(0.7f);
            m_mainLayer->addChild(infoLabel);

            auto btnSpr = ButtonSprite::create(
                "Create Profile", 0, 0, "goldFont.fnt", "GJ_button_01.png", 0, 1.0f
            );
            auto createBtn = CCMenuItemSpriteExtra::create(
                btnSpr,
                this,
                menu_selector(InfoPopup::onCreateProfile)
            );
            auto menu = CCMenu::createWithItem(createBtn);
            menu->setPosition({ winSize.width / 2, winSize.height / 2 - 35.f });
            m_mainLayer->addChild(menu);
            return true;
        }

        this->setTitle("Streak");

        auto gemSprite = CCSprite::create("gem.png"_spr);
        if (gemSprite) {
            gemSprite->setScale(0.2f);
            gemSprite->setPosition({ 25.0f, winSize.height - 25.0f });
            m_mainLayer->addChild(gemSprite, 10);

            m_gemsLabel = CCLabelBMFont::create("0", "goldFont.fnt");
            m_gemsLabel->setScale(0.5f);
            m_gemsLabel->setAnchorPoint({ 0.0f, 0.5f });
            m_gemsLabel->setPosition({ 45.0f, winSize.height - 25.0f });
            m_mainLayer->addChild(m_gemsLabel, 10);
        }

        float contentCenterY = winSize.height / 2 + 25.0f;

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

            rachaSprite->runAction(CCRepeatForever::create(CCSequence::create(
                CCMoveBy::create(1.5f, { 0, 6 }),
                CCMoveBy::create(1.5f, { 0, -6 }),
                nullptr
            )));
        }

        m_streakLabel = CCLabelBMFont::create("Daily streak: ?", "goldFont.fnt");
        m_streakLabel->setScale(0.50f);
        m_streakLabel->setPosition({ winSize.width / 2, contentCenterY - 45 });
        m_mainLayer->addChild(m_streakLabel);

        float barWidth = 140.0f;
        float barHeight = 16.0f;
        float barY = contentCenterY - 70;

        auto barBg = CCLayerColor::create({ 45, 45, 45, 255 }, barWidth, barHeight);
        barBg->setPosition({ winSize.width / 2 - barWidth / 2, barY });
        m_mainLayer->addChild(barBg, 1);

        m_barFg = CCLayerGradient::create({ 250, 225, 60, 255 }, { 255, 165, 0, 255 });
        m_barFg->setContentSize({ 0, barHeight });
        m_barFg->setPosition({ winSize.width / 2 - barWidth / 2, barY });
        m_mainLayer->addChild(m_barFg, 2);

        auto border = CCLayerColor::create(
            { 255, 255, 255, 120 },
            barWidth + 2,
            barHeight + 2
        );
        border->setPosition({ winSize.width / 2 - barWidth / 2 - 1, barY - 1 });
        m_mainLayer->addChild(border, 4);

        auto outer = CCLayerColor::create(
            { 0, 0, 0, 70 },
            barWidth + 6,
            barHeight + 6
        );
        outer->setPosition({ winSize.width / 2 - barWidth / 2 - 3, barY - 3 });
        m_mainLayer->addChild(outer, 0);

        m_pointIcon = CCSprite::create("streak_point.png"_spr);
        m_pointIcon->setScale(0.14f);
        m_pointIcon->setPosition({ winSize.width / 2 - barWidth / 2, barY + 8 });
        m_mainLayer->addChild(m_pointIcon, 6);

        m_barText = CCLabelBMFont::create("? / ?", "bigFont.fnt");
        m_barText->setScale(0.45f);
        m_barText->setPosition({ winSize.width / 2, barY + 8 });
        m_mainLayer->addChild(m_barText, 8);

        m_rachaIndicator = CCSprite::create("racha0.png"_spr);
        m_rachaIndicator->setScale(0.14f);
        m_rachaIndicator->setPosition({ winSize.width / 2 + barWidth / 2, barY + 8 });
        m_mainLayer->addChild(m_rachaIndicator, 7);

        float xpBarHeight = 6.0f;
        float xpY = barY - 12.0f;

        auto xpOuter = CCLayerColor::create(
            { 0, 0, 0, 70 },
            barWidth + 6,
            xpBarHeight + 6
        );
        xpOuter->setPosition({ winSize.width / 2 - barWidth / 2 - 3, xpY - 3 });
        m_mainLayer->addChild(xpOuter, 0);

        auto xpBorder = CCLayerColor::create(
            { 255, 255, 255, 120 },
            barWidth + 2,
            xpBarHeight + 2
        );
        xpBorder->setPosition({ winSize.width / 2 - barWidth / 2 - 1, xpY - 1 });
        m_mainLayer->addChild(xpBorder, 1);

        auto xpBg = CCLayerColor::create({ 20, 20, 40, 150 }, barWidth, xpBarHeight);
        xpBg->setPosition({ winSize.width / 2 - barWidth / 2, xpY });
        m_mainLayer->addChild(xpBg, 1);

        m_xpBarFg = CCLayerGradient::create({ 0, 100, 255, 255 }, { 0, 255, 255, 255 });
        m_xpBarFg->setContentSize({ 0, xpBarHeight });
        m_xpBarFg->setPosition({ winSize.width / 2 - barWidth / 2, xpY });
        m_mainLayer->addChild(m_xpBarFg, 2);

        m_xpIndicator = CCSprite::create("xp.png"_spr);
        m_xpIndicator->setScale(0.08f);
        m_xpIndicator->setPosition({ winSize.width / 2 - barWidth / 2, xpY + 3 });
        m_mainLayer->addChild(m_xpIndicator, 6);

        m_xpLabel = CCLabelBMFont::create("Lvl. ?", "goldFont.fnt");
        m_xpLabel->setScale(0.35f);
        m_xpLabel->setPosition({ winSize.width / 2 - barWidth / 2 - 20, xpY + 3 });
        m_mainLayer->addChild(m_xpLabel);

        m_xpProgressLabel = CCLabelBMFont::create("0/0", "chatFont.fnt");
        m_xpProgressLabel->setScale(0.35f);
        m_xpProgressLabel->setPosition({ winSize.width / 2, xpY + 3 });
        m_mainLayer->addChild(m_xpProgressLabel, 8);

        float sideBtnY = winSize.height / 2 + 30.0f;
        auto cornerMenu = CCMenu::create();
        cornerMenu->setPosition(0, 0);
        m_mainLayer->addChild(cornerMenu, 10);

        auto statsIcon = CCSprite::create("BtnStats.png"_spr);
        statsIcon->setScale(0.7f);
        auto statsBtn = CCMenuItemSpriteExtra::create(
            statsIcon,
            this,
            menu_selector(InfoPopup::onOpenStats)
        );
        statsBtn->setPosition({ winSize.width - 22, sideBtnY + 15 });
        cornerMenu->addChild(statsBtn);

        auto rewardsIcon = CCSprite::create("RewardsBtn.png"_spr);
        rewardsIcon->setScale(0.7f);
        auto rewardsBtn = CCMenuItemSpriteExtra::create(
            rewardsIcon,
            this,
            menu_selector(InfoPopup::onOpenRewards)
        );
        rewardsBtn->setPosition({ winSize.width - 22, sideBtnY - 22 });
        cornerMenu->addChild(rewardsBtn);

        if (g_streakData.isTaskEnabled) {
            auto taskIcon = CCSprite::create("task_btn.png"_spr);
            taskIcon->setScale(0.7f);
            auto taskBtn = CCMenuItemSpriteExtra::create(
                taskIcon,
                this,
                menu_selector(InfoPopup::onOpenTasks)
            );
            taskBtn->setPosition({ winSize.width - 22, sideBtnY - 59 });
            cornerMenu->addChild(taskBtn);
        }

        auto missionsIcon = CCSprite::create("super_star_btn.png"_spr);
        missionsIcon->setScale(0.7f);
        auto missionsBtn = CCMenuItemSpriteExtra::create(
            missionsIcon,
            this,
            menu_selector(InfoPopup::onOpenMissions)
        );
        missionsBtn->setPosition({ 22, sideBtnY + 15 });
        cornerMenu->addChild(missionsBtn);

        auto rouletteIcon = CCSprite::create("boton_ruleta.png"_spr);
        rouletteIcon->setScale(0.7f);
        auto rouletteBtn = CCMenuItemSpriteExtra::create(
            rouletteIcon,
            this,
            menu_selector(InfoPopup::onOpenRoulette)
        );
        rouletteBtn->setPosition({ 22, sideBtnY - 22 });
        cornerMenu->addChild(rouletteBtn);

        auto xpBtnIcon = CCSprite::create("xp_btn.png"_spr);
        xpBtnIcon->setScale(0.7f);
        auto xpBtn = CCMenuItemSpriteExtra::create(
            xpBtnIcon,
            this,
            menu_selector(InfoPopup::onOpenXP)
        );
        xpBtn->setPosition({ 22, sideBtnY - 59 });
        cornerMenu->addChild(xpBtn);

        auto infoIcon = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
        infoIcon->setScale(0.6f);
        auto infoBtn = CCMenuItemSpriteExtra::create(
            infoIcon,
            this,
            menu_selector(InfoPopup::onInfo)
        );
        infoBtn->setPosition({ winSize.width - 80, winSize.height - 20 });
        cornerMenu->addChild(infoBtn);

        auto accountIcon = CCSprite::create("account_btn.png"_spr);
        accountIcon->setScale(0.56f);
        auto accountBtn = CCMenuItemSpriteExtra::create(
            accountIcon,
            this,
            menu_selector(InfoPopup::onOpenAccount)
        );
        accountBtn->setPosition({ winSize.width - 50, winSize.height - 20 });
        cornerMenu->addChild(accountBtn);

        auto settingsIcon = CCSprite::createWithSpriteFrameName("accountBtn_settings_001.png");
        settingsIcon->setScale(0.6f);
        auto settingsBtn = CCMenuItemSpriteExtra::create(
            settingsIcon,
            this,
            menu_selector(InfoPopup::onOpenSettings)
        );
        settingsBtn->setPosition({ winSize.width - 20, winSize.height - 20 });
        cornerMenu->addChild(settingsBtn);

        auto bottomMenu = CCMenu::create();
        m_mainLayer->addChild(bottomMenu, 10);

        auto eventIcon = CCSprite::create("event_boton.png"_spr);
        if (!eventIcon || eventIcon->getContentSize().width == 0) {
            eventIcon = CCSprite::createWithSpriteFrameName("GJ_top100Btn_001.png");
        }
        eventIcon->setScale(0.7f);
        bottomMenu->addChild(CCMenuItemSpriteExtra::create(
            eventIcon,
            this,
            menu_selector(InfoPopup::onOpenEvent)
        ));

        auto stIcon = CCSprite::create("st_progress.png"_spr);
        if (!stIcon) stIcon = ButtonSprite::create("St");
        else stIcon->setScale(0.7f);
        bottomMenu->addChild(CCMenuItemSpriteExtra::create(
            stIcon,
            this,
            menu_selector(InfoPopup::onOpenStProgress)
        ));

        auto levelProgIcon = CCSprite::create("level_progess_btn.png"_spr);
        if (!levelProgIcon) levelProgIcon = ButtonSprite::create("Lvls");
        else levelProgIcon->setScale(0.7f);
        bottomMenu->addChild(CCMenuItemSpriteExtra::create(
            levelProgIcon,
            this,
            menu_selector(InfoPopup::onOpenLevelProgress)
        ));

        auto topIcon = CCSprite::create("top_btn.png"_spr);
        if (!topIcon) topIcon = ButtonSprite::create("Top");
        else topIcon->setScale(0.7f);
        bottomMenu->addChild(CCMenuItemSpriteExtra::create(
            topIcon,
            this,
            menu_selector(InfoPopup::onOpenLeaderboard)
        ));

        auto msgIcon = CCSprite::create("msm.png"_spr);
        if (!msgIcon) msgIcon = CCSprite::createWithSpriteFrameName("GJ_chatBtn_001.png");
        msgIcon->setScale(0.7f);
        bottomMenu->addChild(CCMenuItemSpriteExtra::create(
            msgIcon,
            this,
            menu_selector(InfoPopup::onOpenMessages)
        ));

        auto redeemIcon = CCSprite::create("redemcode_btn.png"_spr);
        redeemIcon->setScale(0.7f);
        bottomMenu->addChild(CCMenuItemSpriteExtra::create(
            redeemIcon,
            this,
            menu_selector(InfoPopup::onRedeemCode)
        ));

        auto kofiIcon = CCSprite::create("ko-fi_btn.png"_spr);
        if (!kofiIcon) kofiIcon = ButtonSprite::create("Donate");
        else kofiIcon->setScale(0.7f);
        bottomMenu->addChild(CCMenuItemSpriteExtra::create(
            kofiIcon,
            this,
            menu_selector(InfoPopup::onOpenDonations)
        ));

        bottomMenu->alignItemsHorizontallyWithPadding(5.0f);
        bottomMenu->setPosition({ winSize.width / 2, 25.f });

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

    void updateDisplay() {
        if (!m_mainLayer || !m_streakLabel || !m_barFg || !m_barText || !m_rachaIndicator) {
            return;
        }

        int currentStreak = g_streakData.currentStreak;
        int pointsToday = g_streakData.streakPointsToday;
        int requiredPoints = g_streakData.getRequiredPoints();
        float percent = (requiredPoints > 0) ? std::min(static_cast<float>(pointsToday) / requiredPoints, 1.0f) : 0.f;

        float barWidth = 140.0f;
        float barHeight = 16.0f;

        m_streakLabel->setString(fmt::format("Daily streak: {}", currentStreak).c_str());
        m_barFg->setContentSize({ barWidth * percent, barHeight });
        m_barText->setString(fmt::format("{}/{}", pointsToday, requiredPoints).c_str());

        if (m_pointIcon) {
            float startX = (m_mainLayer->getContentSize().width / 2) - (barWidth / 2);
            float newX = startX + (barWidth * percent);
            m_pointIcon->setPositionX(newX);
        }

        std::string indicatorSpriteName = (pointsToday >= requiredPoints) ? g_streakData.getRachaSprite() : "racha0.png"_spr;

        if (auto newSprite = CCSprite::create(indicatorSpriteName.c_str())) {
            if (auto newTexture = newSprite->getTexture()) {
                m_rachaIndicator->setTexture(newTexture);
                m_rachaIndicator->setTextureRect(
                    newSprite->getTextureRect(),
                    newSprite->isTextureRectRotated(),
                    newSprite->getContentSize()
                );
            }
        }

        if (m_gemsLabel) {
            m_gemsLabel->setString(std::to_string(g_streakData.gems).c_str());
        }

        if (m_xpBarFg && m_xpLabel && m_xpProgressLabel) {
            float xpBarWidth = 140.0f;
            float xpBarHeight = 6.0f;
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

            m_xpBarFg->setContentSize({ xpBarWidth * xpPercent, xpBarHeight });
            m_xpLabel->setString(
                fmt::format("Lvl. {}", g_streakData.currentLevel).c_str()
            );

            if (m_xpIndicator) {
                float xpStartX = (m_mainLayer->getContentSize().width / 2) - (xpBarWidth / 2);
                float xpNewX = xpStartX + (xpBarWidth * xpPercent);
                m_xpIndicator->setPositionX(xpNewX);
            }
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
        myData.bannerID = g_streakData.equippedBanner;
        myData.badgeID = g_streakData.equippedBadge;
        myData.streakID = g_streakData.streakID;
        myData.globalRank = g_streakData.globalRank;

        myData.isMythic = false;
        auto badge = g_streakData.getEquippedBadge();
        if (badge && badge->category == StreakData::BadgeCategory::MYTHIC) {
            myData.isMythic = true;
        }

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

    void onOpenLevelProgress(CCObject*) {
        LevelProgressPopup::create()->show();
    }

    void onOpenXP(CCObject*) {
        XPPopup::create()->show();
    }

    void onInfo(CCObject*) {
        FLAlertLayer::create(
            "About Streak!",
            "Complete rated levels to earn points!\nAut/Easy/Norm: 1pt\nHard: 3pt\nHarder: 4pt\nInsane: 5pt\nDemon: 6pt",
            "OK"
        )->show();
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
        Popup<>::onClose(sender);
    }

public:
    static InfoPopup* create() {
        auto ret = new InfoPopup();
        if (ret && ret->initAnchored(260.f, 220.f, "geode.loader/GE_square03.png")) {
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