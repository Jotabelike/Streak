#pragma once
#include "StreakCommon.h"
#include "../StreakData.h"
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/ScrollLayer.hpp>
#include <Geode/utils/cocos.hpp>
#include "StreakChestPopup.h" 
#include <random> 

using namespace geode::prelude;

class MissionsPopup : public Popup {
protected:
    ScrollLayer* m_scrollLayer = nullptr;
    std::function<void()> m_closeCallback;

    std::vector<int> getAvailableMissionIDs() {
        std::vector<int> ids;
        if (!g_streakData.pointMission1Claimed) ids.push_back(0);
        if (!g_streakData.pointMission2Claimed) ids.push_back(1);
        if (!g_streakData.pointMission3Claimed) ids.push_back(2);
        if (!g_streakData.pointMission4Claimed) ids.push_back(3);
        if (!g_streakData.pointMission5Claimed) ids.push_back(4);
        if (!g_streakData.pointMission6Claimed) ids.push_back(5);
        return ids;
    }

    CCNode* createPointMissionNode(int missionID) {
        int targetPoints = 0;

        switch (missionID) {
        case 0: targetPoints = 5;  break;
        case 1: targetPoints = 10; break;
        case 2: targetPoints = 15; break;
        case 3: targetPoints = 20; break;
        case 4: targetPoints = 25; break;
        case 5: targetPoints = 30; break;
        default: return nullptr;
        }

        bool isComplete = (g_streakData.streakPointsToday >= targetPoints);

        auto container = cocos2d::extension::CCScale9Sprite::create("GJ_square02.png");
        container->setContentSize({ 250.f, 45.f });

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
        float barHeight = 8.f;
        CCPoint barPosition = { descLabel->getPositionX(), descLabel->getPositionY() - 20.f };

        auto barOuter = CCLayerColor::create({ 0, 0, 0, 150 }, barWidth + 4, barHeight + 4);
        barOuter->setPosition(barPosition + CCPoint{ -2, -2 });
        container->addChild(barOuter);

        auto barBorder = CCLayerColor::create({ 255, 255, 255, 100 }, barWidth + 2, barHeight + 2);
        barBorder->setPosition(barPosition + CCPoint{ -1, -1 });
        container->addChild(barBorder);

        auto barBg = CCLayerColor::create({ 40, 40, 40, 255 }, barWidth, barHeight);
        barBg->setPosition(barPosition);
        container->addChild(barBg);

        float progressPercent = std::min(1.f, static_cast<float>(g_streakData.streakPointsToday) / targetPoints);
        if (progressPercent > 0.f) {
            auto barFill = CCLayerColor::create({ 120, 255, 120, 255 });
            barFill->setContentSize({ barWidth * progressPercent, barHeight });
            barFill->setPosition(barPosition);
            container->addChild(barFill);
        }

        auto progressLabel = CCLabelBMFont::create(
            fmt::format("{}/{}", std::min(g_streakData.streakPointsToday, targetPoints), targetPoints).c_str(),
            "bigFont.fnt"
        );
        progressLabel->setScale(0.4f);
        progressLabel->setPosition(barPosition + CCPoint(barWidth / 2, barHeight / 2));
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
            auto chest = CCSprite::createWithSpriteFrameName("chest_02_02_001.png");
            if (chest) {
                chest->setScale(0.3f);
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

        int maxStars = 0;
        int maxXP = 0;

        switch (missionID) {
        case 0: maxStars = 2;  maxXP = 15;  g_streakData.pointMission1Claimed = true; break;
        case 1: maxStars = 3;  maxXP = 30;  g_streakData.pointMission2Claimed = true; break;
        case 2: maxStars = 5;  maxXP = 50;  g_streakData.pointMission3Claimed = true; break;
        case 3: maxStars = 8;  maxXP = 75;  g_streakData.pointMission4Claimed = true; break;
        case 4: maxStars = 9;  maxXP = 100; g_streakData.pointMission5Claimed = true; break;
        case 5: maxStars = 10; maxXP = 150; g_streakData.pointMission6Claimed = true; break;
        }

        g_streakData.save();

        this->refreshList();

        std::random_device rd;
        std::mt19937 gen(rd());

        std::uniform_int_distribution<> starDist(0, maxStars);
        int finalStars = starDist(gen);

        std::uniform_int_distribution<> xpDist(maxXP / 2, maxXP);
        int finalXP = xpDist(gen);

        std::uniform_int_distribution<> gemDist(1, 100);
        int gemRoll = gemDist(gen);
        int finalGems = 0;

        if (gemRoll == 1) {
            finalGems = 10;
        }
        else if (gemRoll <= 3) {
            finalGems = 4;
        }
        else if (gemRoll <= 8) {
            finalGems = 1;
        }

        StreakChestPopup::create(finalStars, 0, finalGems, finalXP, nullptr)->show();
    }

 
    void onInfoClick(CCObject* sender) {
        FLAlertLayer::create(
            "Chest Info",
            "Complete missions to earn points and open <co>chests</c>!\n\n"
            "Chests contain <cy>Stars</c> and <cg>XP</c>.\n"
            "You also have a chance to find <cb>Gems</c>:\n"
            "<cg>5%</c> chance to get <cb>1 Gem</c>\n"
            "<cy>2%</c> chance to get <cb>4 Gems</c>\n"
            "<cr>1%</c> chance to get <cb>10 Gems!</c>",
            "OK"
        )->show();
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