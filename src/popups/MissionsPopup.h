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
    ScrollLayer* m_scrollLayer = nullptr;
    std::function<void()> m_closeCallback;

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

    std::vector<int> getAvailableMissionIDs() {
        std::vector<int> ids;
        for (int i = 0; i <= 12; ++i) {
            if (!isMissionClaimed(i)) ids.push_back(i);
        }
        return ids;
    }

    CCNode* createPointMissionNode(int missionID) {
        auto info = getMissionInfo(missionID);
        int targetPoints = info.targetPoints;
        int rarity = info.rarity;
        if (targetPoints == 0) return nullptr;

        bool isComplete = (g_streakData.streakPointsToday >= targetPoints);

        auto container = cocos2d::extension::CCScale9Sprite::create("GJ_square02.png");
        container->setContentSize({ 250.f, 45.f });

        switch (rarity) {
        case 1: container->setColor({ 200, 200, 200 }); break;
        case 2: container->setColor({ 130, 190, 245 }); break;
        case 3: container->setColor({ 245, 215, 110 }); break;
        }

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

        switch (rarity) {
        case 1: progressBar->setGradientColors({ 110, 220, 110 }, { 60, 180, 60 });    break;
        case 2: progressBar->setGradientColors({ 110, 180, 255 }, { 60, 130, 220 });   break;
        case 3: progressBar->setGradientColors({ 255, 220, 90 }, { 230, 170, 40 });    break;
        }

        float progressPercent = std::min(1.f, static_cast<float>(g_streakData.streakPointsToday) / targetPoints);
        progressBar->setProgress(progressPercent);
        container->addChild(progressBar);

        auto progressLabel = CCLabelBMFont::create(
            fmt::format("{}/{}", std::min(g_streakData.streakPointsToday, targetPoints), targetPoints).c_str(),
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
                claimBtnSprite, this, menu_selector(MissionsPopup::onClaimReward)
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

    bool init() override {
        if (!Popup::init(320.f, 240.f, "GJ_square04.png")) return false;

        this->setTitle("Missions");
        auto winSize = m_mainLayer->getContentSize();
        g_streakData.load();

        auto background = cocos2d::extension::CCScale9Sprite::create("square02_001.png");
        background->setColor({ 0, 0, 0 });
        background->setOpacity(120);
        background->setContentSize({ 280.f, 160.f });
        background->setPosition({ winSize.width / 2, winSize.height / 2 - 15.f });
        m_mainLayer->addChild(background);

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

    void refreshList() {
        if (m_scrollLayer) {
            m_scrollLayer->removeFromParent();
            m_scrollLayer = nullptr;
        }

        if (auto oldLabel = m_mainLayer->getChildByID("all-done-label")) {
            oldLabel->removeFromParent();
        }

        auto availableMissions = getAvailableMissionIDs();

        if (availableMissions.empty()) {
            auto winSize = m_mainLayer->getContentSize();
            auto allDoneText = "You have claimed all missions.\nCome back tomorrow.";
            auto allDoneLabel = CCLabelBMFont::create(allDoneText, "bigFont.fnt");
            allDoneLabel->setScale(0.45f);
            allDoneLabel->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
            allDoneLabel->setPosition({ winSize.width / 2, winSize.height / 2 - 15.f });
            allDoneLabel->setID("all-done-label");
            m_mainLayer->addChild(allDoneLabel);
            return;
        }

        auto winSize = m_mainLayer->getContentSize();
        float width = 280.f;
        float height = 160.f;

        m_scrollLayer = ScrollLayer::create({ width, height });
        m_scrollLayer->setPosition({ winSize.width / 2 - width / 2, winSize.height / 2 - height / 2 - 15.f });
        m_mainLayer->addChild(m_scrollLayer);

        float itemHeight = 55.f;
        float contentHeight = availableMissions.size() * itemHeight;

        if (contentHeight < height) contentHeight = height;
        m_scrollLayer->m_contentLayer->setContentSize({ width, contentHeight });

        for (size_t i = 0; i < availableMissions.size(); ++i) {
            int missionID = availableMissions[i];
            auto missionNode = createPointMissionNode(missionID);
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

 
    void onInfoClick(CCObject* sender) {
        auto alert = FLAlertLayer::create(
            nullptr,
            "Mission Tiers",
            "Earn points to claim chests of growing rarity.\n\n"
            "<cg>5 - 30 points</c>  -  <co>1-Star Chest</c>\n"
            "<cb>35 - 60 points</c>  -  <cb>2-Star Chest</c>\n"
            "<cy>65 - 100 points</c>  -  <cy>3-Star Chest</c>\n\n"
            "Higher tier chests reward better loot.",
            "OK", nullptr, 340.f
        );
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