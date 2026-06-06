#pragma once
#include "StreakCommon.h"
#include "../StreakData.h"
#include "../FirebaseManager.h"
#include "../RewardNotification.h"
#include "../utils/RoundedProgressBar.h"
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/ScrollLayer.hpp>
#include <Geode/utils/cocos.hpp>

using namespace geode::prelude;

class GoldTicketMissionsPopup : public Popup {
protected:
    enum class Tab { Daily, Weekly, Season };

    ScrollLayer* m_scrollLayer = nullptr;
    std::function<void()> m_closeCallback;
    Tab m_currentTab = Tab::Daily;
    CCMenuItemToggler* m_dailyTabBtn = nullptr;
    CCMenuItemToggler* m_weeklyTabBtn = nullptr;
    CCMenuItemToggler* m_seasonTabBtn = nullptr;
    CCLabelBMFont* m_timerLabel = nullptr;
    bool m_busy = false;

    static std::string scopeString(Tab tab) {
        switch (tab) {
            case Tab::Weekly: return "weekly";
            case Tab::Season: return "season";
            default:          return "daily";
        }
    }

    static long long secondsUntilDailyReset() {
        time_t t = time(nullptr) - 5 * 3600;
        tm* now = gmtime(&t);
        if (!now) return 0;
        long long passed = (long long)now->tm_hour * 3600 + (long long)now->tm_min * 60 + now->tm_sec;
        long long remaining = 86400 - passed;
        return remaining < 0 ? 0 : remaining;
    }

    static long long secondsUntilWeeklyReset() {
        time_t t = time(nullptr) - 5 * 3600;
        tm* now = gmtime(&t);
        if (!now) return 0;
        int weekday = now->tm_wday;
        int daysToMonday = (weekday == 0) ? 1 : (8 - weekday);
        long long passed = (long long)now->tm_hour * 3600 + (long long)now->tm_min * 60 + now->tm_sec;
        long long remaining = (long long)daysToMonday * 86400 - passed;
        return remaining < 0 ? 0 : remaining;
    }

    static long long secondsUntilSeasonEnd() {
        long long endMs = g_streakData.getSeasonEndTime();
        if (endMs <= 0) return 0;
        long long now = (long long)std::time(nullptr) * 1000LL;
        long long remaining = (endMs - now) / 1000;
        return remaining < 0 ? 0 : remaining;
    }

    static std::string formatTimer(long long secs) {
        long long days = secs / 86400;
        long long hours = (secs / 3600) % 24;
        long long mins = (secs / 60) % 60;
        long long s = secs % 60;
        if (days > 0) return fmt::format("{}d {:02}:{:02}:{:02}", days, hours, mins, s);
        return fmt::format("{:02}:{:02}:{:02}", hours, mins, s);
    }

    long long secondsUntilReset() const {
        switch (m_currentTab) {
            case Tab::Weekly: return secondsUntilWeeklyReset();
            case Tab::Season: return secondsUntilSeasonEnd();
            default:          return secondsUntilDailyReset();
        }
    }

    void updateTimerLabel(float = 0.0f) {
        if (!m_timerLabel) return;
        m_timerLabel->setString(("Resets in " + formatTimer(secondsUntilReset())).c_str());
    }

    CCNode* createMissionNode(const StreakData::PassMissionDef& mission, const std::string& scope, int index) {
        int progress = g_streakData.getPassMissionProgress(scope);
        bool claimed = g_streakData.isPassMissionClaimed(scope, mission.id);
        bool complete = (progress >= mission.target);

        auto container = cocos2d::extension::CCScale9Sprite::create("GJ_square02.png");
        container->setContentSize({ 250.f, 45.f });
        container->setColor(claimed ? ccColor3B{ 120, 120, 120 } : ccColor3B{ 245, 215, 110 });

        auto icon = CCSprite::create("gold_ticket.png"_spr);
        if (icon) {
            icon->setScale(0.22f);
            icon->setPosition({ 20.f, 22.f });
            container->addChild(icon);
        }

        auto descLabel = CCLabelBMFont::create(
            fmt::format("Complete {} levels", mission.target).c_str(),
            "goldFont.fnt"
        );
        descLabel->setScale(0.42f);
        descLabel->setAnchorPoint({ 0, 0.5f });
        descLabel->setPosition({ 40.f, 30.f });
        container->addChild(descLabel);

        auto rewardLabel = CCLabelBMFont::create(
            fmt::format("+{}", mission.reward).c_str(),
            "bigFont.fnt"
        );
        rewardLabel->setScale(0.38f);
        rewardLabel->setAnchorPoint({ 0, 0.5f });
        rewardLabel->setColor({ 255, 235, 120 });
        rewardLabel->setPosition({ 40.f, 13.f });
        container->addChild(rewardLabel);

        auto rewardIcon = CCSprite::create("gold_ticket.png"_spr);
        if (rewardIcon) {
            rewardIcon->setScale(0.14f);
            rewardIcon->setAnchorPoint({ 0, 0.5f });
            rewardIcon->setPosition({ 40.f + rewardLabel->getScaledContentSize().width + 6.f, 13.f });
            container->addChild(rewardIcon);
        }

        float barWidth = 90.f;
        float barHeight = 10.f;
        CCPoint barCenter = { 150.f, 13.f };
        auto progressBar = RoundedProgressBar::create(barWidth, barHeight);
        progressBar->setPosition(barCenter);
        progressBar->setGradientColors({ 255, 220, 90 }, { 230, 170, 40 });
        progressBar->setProgress(mission.target > 0 ? std::min(1.f, (float)progress / mission.target) : 0.f);
        container->addChild(progressBar);

        auto progressLabel = CCLabelBMFont::create(
            fmt::format("{}/{}", std::min(progress, mission.target), mission.target).c_str(),
            "bigFont.fnt"
        );
        progressLabel->setScale(0.32f);
        progressLabel->setPosition(barCenter);
        progressLabel->setZOrder(5);
        container->addChild(progressLabel);

        if (claimed) {
            auto check = CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
            if (check) {
                check->setScale(0.6f);
                check->setPosition({ 222.f, 22.5f });
                container->addChild(check);
            }
        } else if (complete) {
            auto claimBtnSprite = CCSprite::createWithSpriteFrameName("GJ_rewardBtn_001.png");
            if (!claimBtnSprite) claimBtnSprite = CCSprite::create("GJ_rewardBtn_001.png");
            claimBtnSprite->setScale(0.6f);
            auto claimBtn = CCMenuItemSpriteExtra::create(
                claimBtnSprite, this, menu_selector(GoldTicketMissionsPopup::onClaim)
            );
            claimBtn->setTag(index);
            auto menu = CCMenu::createWithItem(claimBtn);
            menu->setPosition({ 222.f, 22.5f });
            container->addChild(menu);
        } else {
            auto lock = CCSprite::createWithSpriteFrameName("GJ_lockGray_001.png");
            if (lock) {
                lock->setScale(0.5f);
                lock->setPosition({ 222.f, 22.5f });
                container->addChild(lock);
            }
        }

        return container;
    }

    void refreshList() {
        if (m_scrollLayer) {
            m_scrollLayer->removeFromParent();
            m_scrollLayer = nullptr;
        }
        if (auto oldLabel = m_mainLayer->getChildByID("empty-label")) {
            oldLabel->removeFromParent();
        }

        std::string scope = scopeString(m_currentTab);
        const auto& missions = g_streakData.getPassMissions(scope);

        auto winSize = m_mainLayer->getContentSize();
        float width = 280.f;
        float height = 145.f;
        float listCenterY = winSize.height / 2 - 25.f;

        if (missions.empty()) {
            auto label = CCLabelBMFont::create(
                "No missions available.\nCheck back soon.", "bigFont.fnt"
            );
            label->setScale(0.45f);
            label->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
            label->setPosition({ winSize.width / 2, listCenterY });
            label->setID("empty-label");
            m_mainLayer->addChild(label);
            return;
        }

        m_scrollLayer = ScrollLayer::create({ width, height });
        m_scrollLayer->setPosition({ winSize.width / 2 - width / 2, listCenterY - height / 2 });
        m_mainLayer->addChild(m_scrollLayer);

        float itemHeight = 55.f;
        float contentHeight = std::max(height, missions.size() * itemHeight);
        m_scrollLayer->m_contentLayer->setContentSize({ width, contentHeight });

        for (size_t i = 0; i < missions.size(); ++i) {
            auto node = createMissionNode(missions[i], scope, (int)i);
            if (node) {
                node->setPosition({ width / 2, contentHeight - (i * itemHeight) - itemHeight / 2 });
                m_scrollLayer->m_contentLayer->addChild(node);
            }
        }
        m_scrollLayer->moveToTop();
    }

    void onTabToggled(CCObject* sender) {
        int tag = sender->getTag();
        Tab newTab = (tag == 1) ? Tab::Weekly : (tag == 2) ? Tab::Season : Tab::Daily;
        m_currentTab = newTab;
        if (m_dailyTabBtn)  m_dailyTabBtn->toggle(m_currentTab == Tab::Daily);
        if (m_weeklyTabBtn) m_weeklyTabBtn->toggle(m_currentTab == Tab::Weekly);
        if (m_seasonTabBtn) m_seasonTabBtn->toggle(m_currentTab == Tab::Season);
        updateTimerLabel();
        refreshList();
    }

    void onClaim(CCObject* sender) {
        if (m_busy) return;
        auto btn = static_cast<CCNode*>(sender);
        int index = btn->getTag();
        std::string scope = scopeString(m_currentTab);

        const auto& missions = g_streakData.getPassMissions(scope);
        if (index < 0 || index >= (int)missions.size()) return;
        const StreakData::PassMissionDef& def = missions[index];
        std::string missionID = def.id;
        if (g_streakData.isPassMissionClaimed(scope, missionID)) return;
        if (g_streakData.getPassMissionProgress(scope) < def.target) return;

        m_busy = true;
        int reward = def.reward;
        int startTickets = g_streakData.goldTickets;

        matjson::Value payload = matjson::Value::object();
        payload.set("scope", scope);
        payload.set("mission_id", missionID);

        claimOnServer("/streak-pass/mission/claim", payload,
            [this, scope, missionID, reward, startTickets](bool ok) {
                m_busy = false;
                if (!ok) {
                    FLAlertLayer::create("Error", "Could not claim this mission. Try again.", "OK")->show();
                    return;
                }
                g_streakData.markPassMissionClaimed(scope, missionID);
                FMODAudioEngine::sharedEngine()->playEffect("buyItem03.ogg");
                RewardNotification::show("gold_ticket.png"_spr, startTickets, reward);
                this->refreshList();
                if (m_closeCallback) m_closeCallback();
            }
        );
    }

    bool init() override {
        if (!Popup::init(320.f, 240.f, "GJ_square04.png")) return false;
        this->setTitle("Pass Missions");
        auto winSize = m_mainLayer->getContentSize();

        auto background = cocos2d::extension::CCScale9Sprite::create("square02_001.png");
        background->setColor({ 0, 0, 0 });
        background->setOpacity(120);
        background->setContentSize({ 280.f, 145.f });
        background->setPosition({ winSize.width / 2, winSize.height / 2 - 25.f });
        m_mainLayer->addChild(background);

        auto tabMenu = CCMenu::create();
        tabMenu->setLayout(RowLayout::create()->setGap(6.f));

        auto makeTab = [&](const char* label, int tag) -> CCMenuItemToggler* {
            auto on  = ButtonSprite::create(label, 48, true, "bigFont.fnt", "GJ_button_01.png", 22, 0.42f);
            auto off = ButtonSprite::create(label, 48, true, "bigFont.fnt", "GJ_button_01.png", 22, 0.42f);
            auto btn = CCMenuItemToggler::create(off, on, this, menu_selector(GoldTicketMissionsPopup::onTabToggled));
            btn->setTag(tag);
            return btn;
        };

        m_dailyTabBtn  = makeTab("Daily", 0);
        m_weeklyTabBtn = makeTab("Weekly", 1);
        m_seasonTabBtn = makeTab("Season", 2);
        m_dailyTabBtn->toggle(true);

        tabMenu->addChild(m_dailyTabBtn);
        tabMenu->addChild(m_weeklyTabBtn);
        tabMenu->addChild(m_seasonTabBtn);
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
        this->schedule(schedule_selector(GoldTicketMissionsPopup::updateTimerLabel), 1.0f);

        refreshList();
        return true;
    }

    void onClose(CCObject* sender) override {
        if (m_closeCallback) m_closeCallback();
        Popup::onClose(sender);
    }

public:
    static GoldTicketMissionsPopup* create(std::function<void()> callback = nullptr) {
        auto ret = new GoldTicketMissionsPopup();
        ret->m_closeCallback = callback;
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};
