#pragma once
#include "StreakCommon.h"
#include "../StreakData.h"
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/ScrollLayer.hpp>
#include <Geode/utils/cocos.hpp>
#include "StreakChestPopup.h"
#include "../utils/RoundedProgressBar.h"
#include <random>

using namespace geode::prelude;

class MissionsPopup : public Popup {
protected:
    enum class Tab { Daily, Weekly };

    ScrollLayer* m_scrollLayer = nullptr;
    std::function<void()> m_closeCallback;
    Tab m_currentTab = Tab::Daily;
    CCMenuItemToggler* m_dailyTabBtn = nullptr;
    CCMenuItemToggler* m_weeklyTabBtn = nullptr;
    CCLabelBMFont* m_timerLabel = nullptr;

    // Both timers tick on the UTC-5 boundary (same offset the server uses).
    static long long secondsUntilDailyReset() {
        time_t t = time(nullptr) - 5 * 3600;
        tm* now = gmtime(&t);
        if (!now) return 0;
        long long passed = (long long)now->tm_hour * 3600 + (long long)now->tm_min * 60 + now->tm_sec;
        long long remaining = 86400 - passed;
        if (remaining < 0) remaining = 0;
        return remaining;
    }

    static long long secondsUntilWeeklyReset() {
        time_t t = time(nullptr) - 5 * 3600;
        tm* now = gmtime(&t);
        if (!now) return 0;
        int weekday = now->tm_wday; // 0 = Sunday
        int daysToMonday = (weekday == 0) ? 1 : (8 - weekday); // 1..7
        long long passed = (long long)now->tm_hour * 3600 + (long long)now->tm_min * 60 + now->tm_sec;
        long long remaining = (long long)daysToMonday * 86400 - passed;
        if (remaining < 0) remaining = 0;
        return remaining;
    }

    static std::string formatTimer(long long secs) {
        long long days = secs / 86400;
        long long hours = (secs / 3600) % 24;
        long long mins = (secs / 60) % 60;
        long long s = secs % 60;
        if (days > 0) return fmt::format("{}d {:02}:{:02}:{:02}", days, hours, mins, s);
        return fmt::format("{:02}:{:02}:{:02}", hours, mins, s);
    }

    void updateTimerLabel(float = 0.0f) {
        if (!m_timerLabel) return;
        long long secs = (m_currentTab == Tab::Weekly)
            ? secondsUntilWeeklyReset()
            : secondsUntilDailyReset();
        const char* prefix = (m_currentTab == Tab::Weekly) ? "Resets in " : "Resets in ";
        m_timerLabel->setString((std::string(prefix) + formatTimer(secs)).c_str());
    }

    struct MissionInfo {
        int targetPoints;
        int rarity;
    };
    static MissionInfo getMissionInfo(int missionID) {
        switch (missionID) {
        case 0:  return { 5,  1 };
        case 1:  return { 10, 1 };
        case 2:  return { 15, 1 };
        case 3:  return { 20, 1 };
        case 4:  return { 25, 1 };
        case 5:  return { 30, 1 };
        case 6:  return { 35, 2 };
        case 7:  return { 45, 2 };
        case 8:  return { 55, 2 };
        case 9:  return { 60, 2 };
        case 10: return { 75, 3 };
        case 11: return { 90, 3 };
        case 12: return { 100, 3 };
        }
        return { 0, 1 };
    }

    static MissionInfo getWeeklyMissionInfo(int missionID) {
        switch (missionID) {
        case 0: return { 300,  3 };
        case 1: return { 500,  3 };
        case 2: return { 700,  4 };
        case 3: return { 900,  4 };
        case 4: return { 1100, 4 };
        case 5: return { 1300, 4 };
        case 6: return { 1500, 4 };
        case 7: return { 1700, 4 };
        case 8: return { 1900, 5 };
        case 9: return { 2100, 5 };
        }
        return { 0, 1 };
    }

    static bool isMissionClaimed(int missionID) {
        switch (missionID) {
        case 0: return g_streakData.pointMission1Claimed;
        case 1: return g_streakData.pointMission2Claimed;
        case 2: return g_streakData.pointMission3Claimed;
        case 3: return g_streakData.pointMission4Claimed;
        case 4: return g_streakData.pointMission5Claimed;
        case 5: return g_streakData.pointMission6Claimed;
        case 6: return g_streakData.pointMission7Claimed;
        case 7: return g_streakData.pointMission8Claimed;
        case 8: return g_streakData.pointMission9Claimed;
        case 9: return g_streakData.pointMission10Claimed;
        case 10: return g_streakData.pointMission11Claimed;
        case 11: return g_streakData.pointMission12Claimed;
        case 12: return g_streakData.pointMission13Claimed;
        }
        return true;
    }

    static void markMissionClaimed(int missionID) {
        switch (missionID) {
        case 0: g_streakData.pointMission1Claimed = true; break;
        case 1: g_streakData.pointMission2Claimed = true; break;
        case 2: g_streakData.pointMission3Claimed = true; break;
        case 3: g_streakData.pointMission4Claimed = true; break;
        case 4: g_streakData.pointMission5Claimed = true; break;
        case 5: g_streakData.pointMission6Claimed = true; break;
        case 6: g_streakData.pointMission7Claimed = true; break;
        case 7: g_streakData.pointMission8Claimed = true; break;
        case 8: g_streakData.pointMission9Claimed = true; break;
        case 9: g_streakData.pointMission10Claimed = true; break;
        case 10: g_streakData.pointMission11Claimed = true; break;
        case 11: g_streakData.pointMission12Claimed = true; break;
        case 12: g_streakData.pointMission13Claimed = true; break;
        }
    }

    static bool isWeeklyMissionClaimed(int missionID) {
        switch (missionID) {
        case 0: return g_streakData.weeklyMission1Claimed;
        case 1: return g_streakData.weeklyMission2Claimed;
        case 2: return g_streakData.weeklyMission3Claimed;
        case 3: return g_streakData.weeklyMission4Claimed;
        case 4: return g_streakData.weeklyMission5Claimed;
        case 5: return g_streakData.weeklyMission6Claimed;
        case 6: return g_streakData.weeklyMission7Claimed;
        case 7: return g_streakData.weeklyMission8Claimed;
        case 8: return g_streakData.weeklyMission9Claimed;
        case 9: return g_streakData.weeklyMission10Claimed;
        }
        return true;
    }

    static void markWeeklyMissionClaimed(int missionID) {
        switch (missionID) {
        case 0: g_streakData.weeklyMission1Claimed = true; break;
        case 1: g_streakData.weeklyMission2Claimed = true; break;
        case 2: g_streakData.weeklyMission3Claimed = true; break;
        case 3: g_streakData.weeklyMission4Claimed = true; break;
        case 4: g_streakData.weeklyMission5Claimed = true; break;
        case 5: g_streakData.weeklyMission6Claimed = true; break;
        case 6: g_streakData.weeklyMission7Claimed = true; break;
        case 7: g_streakData.weeklyMission8Claimed = true; break;
        case 8: g_streakData.weeklyMission9Claimed = true; break;
        case 9: g_streakData.weeklyMission10Claimed = true; break;
        }
    }

    std::vector<int> getAvailableMissionIDs() {
        std::vector<int> ids;
        for (int i = 0; i <= 12; ++i) {
            if (!isMissionClaimed(i)) ids.push_back(i);
        }
        return ids;
    }

    std::vector<int> getAvailableWeeklyMissionIDs() {
        std::vector<int> ids;
        for (int i = 0; i <= 9; ++i) {
            if (!isWeeklyMissionClaimed(i)) ids.push_back(i);
        }
        return ids;
    }

    static ccColor3B containerColorForRarity(int rarity) {
        switch (rarity) {
        case 1: return { 200, 200, 200 };
        case 2: return { 130, 190, 245 };
        case 3: return { 245, 215, 110 };
        case 4: return { 255, 175, 80 };
        case 5: return { 255, 110, 110 };
        }
        return { 255, 255, 255 };
    }

    static void gradientForRarity(int rarity, ccColor3B& outA, ccColor3B& outB) {
        switch (rarity) {
        case 1: outA = { 110, 220, 110 }; outB = { 60, 180, 60 };  break;
        case 2: outA = { 110, 180, 255 }; outB = { 60, 130, 220 }; break;
        case 3: outA = { 255, 220, 90 };  outB = { 230, 170, 40 }; break;
        case 4: outA = { 255, 180, 80 };  outB = { 220, 130, 30 }; break;
        case 5: outA = { 255, 130, 130 }; outB = { 220, 70, 70 };  break;
        default: outA = { 255, 255, 255 }; outB = { 200, 200, 200 }; break;
        }
    }

    CCNode* createMissionNode(int missionID, bool weekly) {
        auto info = weekly ? getWeeklyMissionInfo(missionID) : getMissionInfo(missionID);
        int targetPoints = info.targetPoints;
        int rarity = info.rarity;
        if (targetPoints == 0) return nullptr;

        int currentPoints = weekly ? g_streakData.streakPointsThisWeek : g_streakData.streakPointsToday;
        bool isComplete = (currentPoints >= targetPoints);

        auto container = cocos2d::extension::CCScale9Sprite::create("GJ_square02.png");
        container->setContentSize({ 250.f, 45.f });
        container->setColor(containerColorForRarity(rarity));

        auto missionIcon = CCSprite::create("streak_point.png"_spr);
        if (missionIcon) {
            missionIcon->setScale(0.25f);
            missionIcon->setPosition({ 20.f, 22.f });
            container->addChild(missionIcon);
        }

        auto descLabel = CCLabelBMFont::create(
            fmt::format("Get {} Points", targetPoints).c_str(),
            "goldFont.fnt"
        );
        descLabel->setScale(0.45f);
        descLabel->setAnchorPoint({ 0, 0.5f });
        descLabel->setPosition({ 40.f, 28.f });
        container->addChild(descLabel);

        float barWidth = 120.f;
        float barHeight = 12.f;
        CCPoint barCenter = { descLabel->getPositionX() + barWidth / 2.f,
                              descLabel->getPositionY() - 16.f };

        auto progressBar = RoundedProgressBar::create(barWidth, barHeight);
        progressBar->setPosition(barCenter);

        ccColor3B gA, gB;
        gradientForRarity(rarity, gA, gB);
        progressBar->setGradientColors(gA, gB);

        float progressPercent = std::min(1.f, static_cast<float>(currentPoints) / targetPoints);
        progressBar->setProgress(progressPercent);
        container->addChild(progressBar);

        auto progressLabel = CCLabelBMFont::create(
            fmt::format("{}/{}", std::min(currentPoints, targetPoints), targetPoints).c_str(),
            "bigFont.fnt"
        );
        progressLabel->setScale(0.4f);
        progressLabel->setPosition(barCenter);
        progressLabel->setZOrder(5);
        container->addChild(progressLabel);

        if (isComplete) {
            auto claimBtnSprite = CCSprite::createWithSpriteFrameName("GJ_rewardBtn_001.png");
            if (!claimBtnSprite) claimBtnSprite = CCSprite::create("GJ_rewardBtn_001.png");
            claimBtnSprite->setScale(0.6f);

            auto claimBtn = CCMenuItemSpriteExtra::create(
                claimBtnSprite, this,
                weekly ? menu_selector(MissionsPopup::onClaimWeeklyReward)
                       : menu_selector(MissionsPopup::onClaimReward)
            );
            claimBtn->setTag(missionID);

            auto menu = CCMenu::createWithItem(claimBtn);
            menu->setPosition({ 215.f, 22.5f });
            container->addChild(menu);
        }
        else {
            std::string chestPath = fmt::format("{}/ChestStar{}.png", Mod::get()->getID(), rarity);
            auto chest = CCSprite::create(chestPath.c_str());
            if (!chest) chest = CCSprite::createWithSpriteFrameName("chest_02_02_001.png");
            if (chest) {
                chest->setScale(0.27f);
                chest->setPosition({ 215.f, 22.5f });
                container->addChild(chest);
            }
        }

        return container;
    }

    CCNode* createPointMissionNode(int missionID) {
        return createMissionNode(missionID, false);
    }

    bool init() override {
        if (!Popup::init(320.f, 240.f, "GJ_square04.png")) return false;

        this->setTitle("Missions");
        auto winSize = m_mainLayer->getContentSize();
        g_streakData.load();

        auto background = cocos2d::extension::CCScale9Sprite::create("square02_001.png");
        background->setColor({ 0, 0, 0 });
        background->setOpacity(120);
        background->setContentSize({ 280.f, 145.f });
        background->setPosition({ winSize.width / 2, winSize.height / 2 - 25.f });
        m_mainLayer->addChild(background);

        auto tabMenu = CCMenu::create();
        tabMenu->setLayout(RowLayout::create()->setGap(8.f));

        auto dailyOn  = ButtonSprite::create("Daily",  60, true, "bigFont.fnt", "GJ_button_01.png", 22, 0.45f);
        auto dailyOff = ButtonSprite::create("Daily",  60, true, "bigFont.fnt", "GJ_button_01.png", 22, 0.45f);
        m_dailyTabBtn = CCMenuItemToggler::create(dailyOff, dailyOn, this, menu_selector(MissionsPopup::onTabToggled));
        m_dailyTabBtn->setTag(0);

        auto weeklyOn  = ButtonSprite::create("Weekly", 60, true, "bigFont.fnt", "GJ_button_01.png", 22, 0.45f);
        auto weeklyOff = ButtonSprite::create("Weekly", 60, true, "bigFont.fnt", "GJ_button_01.png", 22, 0.45f);
        m_weeklyTabBtn = CCMenuItemToggler::create(weeklyOff, weeklyOn, this, menu_selector(MissionsPopup::onTabToggled));
        m_weeklyTabBtn->setTag(1);

        m_dailyTabBtn->toggle(true);

        tabMenu->addChild(m_dailyTabBtn);
        tabMenu->addChild(m_weeklyTabBtn);
        tabMenu->updateLayout();
        tabMenu->setPosition({ winSize.width / 2, winSize.height - 48.f });
        m_mainLayer->addChild(tabMenu);

        m_timerLabel = CCLabelBMFont::create("Resets in --:--:--", "bigFont.fnt");
        m_timerLabel->setScale(0.4f);
        m_timerLabel->setColor({ 255, 220, 120 });
        m_timerLabel->setAnchorPoint({ 1.f, 0.f });
        m_timerLabel->setPosition({ winSize.width - 8.f, 8.f });
        m_mainLayer->addChild(m_timerLabel, 5);
        updateTimerLabel();
        this->schedule(schedule_selector(MissionsPopup::updateTimerLabel), 1.0f);

        refreshList();

        auto infoSpr = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
        if (infoSpr) {
            infoSpr->setScale(0.7f);

            auto infoBtn = CCMenuItemSpriteExtra::create(
                infoSpr, this, menu_selector(MissionsPopup::onInfoClick)
            );

            auto topMenu = CCMenu::create();
            topMenu->setPosition({ 0, 0 });
            m_mainLayer->addChild(topMenu);

            infoBtn->setPosition({ winSize.width - 20.f, winSize.height - 20.f });
            topMenu->addChild(infoBtn);
        }

        return true;
    }

    void onTabToggled(CCObject* sender) {
        int tag = sender->getTag();
        Tab newTab = (tag == 1) ? Tab::Weekly : Tab::Daily;
        if (newTab == m_currentTab) {
            if (m_dailyTabBtn) m_dailyTabBtn->toggle(m_currentTab == Tab::Daily);
            if (m_weeklyTabBtn) m_weeklyTabBtn->toggle(m_currentTab == Tab::Weekly);
            return;
        }
        m_currentTab = newTab;
        if (m_dailyTabBtn) m_dailyTabBtn->toggle(m_currentTab == Tab::Daily);
        if (m_weeklyTabBtn) m_weeklyTabBtn->toggle(m_currentTab == Tab::Weekly);
        updateTimerLabel();
        refreshList();
    }

    void refreshList() {
        if (m_scrollLayer) {
            m_scrollLayer->removeFromParent();
            m_scrollLayer = nullptr;
        }

        if (auto oldLabel = m_mainLayer->getChildByID("all-done-label")) {
            oldLabel->removeFromParent();
        }

        bool weekly = (m_currentTab == Tab::Weekly);
        auto availableMissions = weekly ? getAvailableWeeklyMissionIDs() : getAvailableMissionIDs();

        auto winSize = m_mainLayer->getContentSize();
        float width = 280.f;
        float height = 145.f;
        float listCenterY = winSize.height / 2 - 25.f;

        if (availableMissions.empty()) {
            auto allDoneText = weekly
                ? "You have claimed all weekly missions.\nCome back next week."
                : "You have claimed all missions.\nCome back tomorrow.";
            auto allDoneLabel = CCLabelBMFont::create(allDoneText, "bigFont.fnt");
            allDoneLabel->setScale(0.45f);
            allDoneLabel->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
            allDoneLabel->setPosition({ winSize.width / 2, listCenterY });
            allDoneLabel->setID("all-done-label");
            m_mainLayer->addChild(allDoneLabel);
            return;
        }

        m_scrollLayer = ScrollLayer::create({ width, height });
        m_scrollLayer->setPosition({ winSize.width / 2 - width / 2, listCenterY - height / 2 });
        m_mainLayer->addChild(m_scrollLayer);

        float itemHeight = 55.f;
        float contentHeight = availableMissions.size() * itemHeight;

        if (contentHeight < height) contentHeight = height;
        m_scrollLayer->m_contentLayer->setContentSize({ width, contentHeight });

        for (size_t i = 0; i < availableMissions.size(); ++i) {
            int missionID = availableMissions[i];
            auto missionNode = createMissionNode(missionID, weekly);
            if (missionNode) {
                missionNode->setPosition({ width / 2, contentHeight - (i * itemHeight) - itemHeight / 2 });
                m_scrollLayer->m_contentLayer->addChild(missionNode);
            }
        }

        m_scrollLayer->moveToTop();
    }

    void onClaimReward(CCObject* sender) {
        FMODAudioEngine::sharedEngine()->playEffect("buyItem03.ogg");

        auto btn = static_cast<CCNode*>(sender);
        int missionID = btn->getTag();
        auto info = getMissionInfo(missionID);
        if (info.targetPoints == 0) return;

        markMissionClaimed(missionID);
        g_streakData.save();
        this->refreshList();

        int stars = 0, tickets = 0, gems = 0, xp = 0;
        StreakChestPopup::rollRewardsForRarity(info.rarity, stars, tickets, gems, xp);

        auto refresh = [this]() { this->refreshList(); };
        if (auto popup = StreakChestPopup::create(stars, tickets, gems, xp, info.rarity, refresh)) {
            popup->show();
        }
    }

    void onClaimWeeklyReward(CCObject* sender) {
        FMODAudioEngine::sharedEngine()->playEffect("buyItem03.ogg");

        auto btn = static_cast<CCNode*>(sender);
        int missionID = btn->getTag();
        auto info = getWeeklyMissionInfo(missionID);
        if (info.targetPoints == 0) return;

        markWeeklyMissionClaimed(missionID);
        g_streakData.save();
        this->refreshList();

        int stars = 0, tickets = 0, gems = 0, xp = 0;
        StreakChestPopup::rollRewardsForRarity(info.rarity, stars, tickets, gems, xp);

        auto refresh = [this]() { this->refreshList(); };
        if (auto popup = StreakChestPopup::create(stars, tickets, gems, xp, info.rarity, refresh)) {
            popup->show();
        }
    }


    void onInfoClick(CCObject* sender) {
        const char* title;
        const char* body;
        if (m_currentTab == Tab::Weekly) {
            title = "Weekly Missions";
            body =
                "Earn points across the week to claim bigger chests.\n"
                "Resets every Monday.\n\n"
                "<cy>300 - 500 points</c>  -  <cy>3-Star Chest</c>\n"
                "<co>700 - 1700 points</c>  -  <co>4-Star Chest</c>\n"
                "<cr>1900 - 2100 points</c>  -  <cr>5-Star Chest</c>";
        } else {
            title = "Daily Missions";
            body =
                "Earn points to claim chests of growing rarity.\n"
                "Resets every day.\n\n"
                "<cg>5 - 30 points</c>  -  <c>1-Star Chest</c>\n"
                "<cb>35 - 60 points</c>  -  <cb>2-Star Chest</c>\n"
                "<cy>65 - 100 points</c>  -  <cy>3-Star Chest</c>";
        }
        auto alert = FLAlertLayer::create(nullptr, title, body, "OK", nullptr, 340.f);
        if (alert) alert->show();
    }

    void onClose(CCObject* sender) override {
        if (m_closeCallback) {
            m_closeCallback();
        }
        Popup::onClose(sender);
    }

public:
    static MissionsPopup* create(std::function<void()> callback = nullptr) {
        auto ret = new MissionsPopup();
        ret->m_closeCallback = callback;
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};
