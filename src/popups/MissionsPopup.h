#pragma once
#include "StreakCommon.h"
#include "../StreakData.h"
#include <Geode/ui/Popup.hpp>
#include <Geode/utils/cocos.hpp>
#include "../RewardNotification.h"
#include "../SystemNotification.h"

using namespace geode::prelude;

class MissionsPopup : public Popup {
protected:
    int m_currentPage = 0;
    CCLayer* m_pageContainer = nullptr;
    CCMenuItemSpriteExtra* m_leftArrow = nullptr;
    CCMenuItemSpriteExtra* m_rightArrow = nullptr;
    bool m_isAnimating = false;

    std::function<void()> m_closeCallback;

    CCNode* createPointMissionNode(int missionID) {
        int targetPoints = 0;
        int rewardStars = 0;
        int rewardXP = 0;
        bool isClaimed = false;

        switch (missionID) {
        case 0:
            targetPoints = 5;  rewardStars = 2;  rewardXP = 10;
            isClaimed = g_streakData.pointMission1Claimed; break;
        case 1:
            targetPoints = 10; rewardStars = 3;  rewardXP = 15;
            isClaimed = g_streakData.pointMission2Claimed; break;
        case 2:
            targetPoints = 15; rewardStars = 5;  rewardXP = 20;
            isClaimed = g_streakData.pointMission3Claimed; break;
        case 3:
            targetPoints = 20; rewardStars = 8;  rewardXP = 35;
            isClaimed = g_streakData.pointMission4Claimed; break;
        case 4:
            targetPoints = 25; rewardStars = 9;  rewardXP = 50;
            isClaimed = g_streakData.pointMission5Claimed; break;
        case 5:
            targetPoints = 30; rewardStars = 10; rewardXP = 70;
            isClaimed = g_streakData.pointMission6Claimed; break;
        default: return nullptr;
        }

        bool isComplete = (g_streakData.streakPointsToday >= targetPoints);
        auto container = cocos2d::extension::CCScale9Sprite::create("geode.loader/GE_square03.png");
        container->setContentSize({ 250.f, 45.f });

        auto missionIcon = CCSprite::create("streak_point.png"_spr);
        missionIcon->setScale(0.25f);
        missionIcon->setPosition({ 20.f, 22.f });
        container->addChild(missionIcon);

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

        if (isClaimed) {
            container->setOpacity(100);
            auto claimedLabel = CCLabelBMFont::create("CLAIMED", "goldFont.fnt");
            claimedLabel->setScale(0.7f);
            claimedLabel->setPosition(container->getContentSize() / 2);
            container->addChild(claimedLabel);
        }
        else if (isComplete) {
            auto claimBtnSprite = ButtonSprite::create("Claim");
            claimBtnSprite->setScale(0.65f);

            auto claimBtn = CCMenuItemSpriteExtra::create(
                claimBtnSprite, this, menu_selector(MissionsPopup::onClaimReward)
            );
            claimBtn->setTag(missionID);

            auto menu = CCMenu::createWithItem(claimBtn);
            menu->setPosition({ 215.f, 22.5f });
            container->addChild(menu);
        }
        else {
            float iconX = 208.f;
            float textX = 218.f;
            float topY = 32.f;
            float botY = 13.f;

            auto starSprite = CCSprite::create("super_star.png"_spr);
            starSprite->setScale(0.15f);
            starSprite->setPosition({ iconX, topY });
            container->addChild(starSprite);

            auto starLabel = CCLabelBMFont::create(fmt::format("+{}", rewardStars).c_str(), "bigFont.fnt");
            starLabel->setScale(0.3f);
            starLabel->setAnchorPoint({ 0, 0.5f });
            starLabel->setPosition({ textX, topY });
            container->addChild(starLabel);

            auto xpSprite = CCSprite::create("xp.png"_spr);
            if (xpSprite) {
                xpSprite->setScale(0.15f);
                xpSprite->setPosition({ iconX, botY });
                container->addChild(xpSprite);
            }

            auto xpLabel = CCLabelBMFont::create(fmt::format("+{}", rewardXP).c_str(), "bigFont.fnt");
            xpLabel->setScale(0.3f);
            xpLabel->setAnchorPoint({ 0, 0.5f });
            xpLabel->setPosition({ textX, botY });
            container->addChild(xpLabel);
        }

        return container;
    }

    bool init() {
        if (!Popup::init(320.f, 240.f, "geode.loader/GE_square03.png")) return false;

        this->setTitle("Missions");
        auto winSize = m_mainLayer->getContentSize();
        g_streakData.load();

        auto background = cocos2d::extension::CCScale9Sprite::create("square02_001.png");
        background->setColor({ 0, 0, 0 });
        background->setOpacity(120);
        background->setContentSize({ 280.f, 160.f });
        background->setPosition({ winSize.width / 2, winSize.height / 2 - 15.f });
        m_mainLayer->addChild(background);

        m_pageContainer = CCLayer::create();
        m_pageContainer->setPosition(background->getPosition());
        m_mainLayer->addChild(m_pageContainer);

        auto leftSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
        m_leftArrow = CCMenuItemSpriteExtra::create(leftSpr, this, menu_selector(MissionsPopup::onSwitchPage));
        m_leftArrow->setTag(-1);

        auto rightSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
        rightSpr->setFlipX(true);
        m_rightArrow = CCMenuItemSpriteExtra::create(rightSpr, this, menu_selector(MissionsPopup::onSwitchPage));
        m_rightArrow->setTag(1);

        auto navMenu = CCMenu::create();
        navMenu->addChild(m_leftArrow);
        navMenu->addChild(m_rightArrow);
        navMenu->alignItemsHorizontallyWithPadding(background->getContentSize().width + 25.f);
        navMenu->setPosition(background->getPosition());
        m_mainLayer->addChild(navMenu);

        updatePage();

        auto starSprite = CCSprite::create("super_star.png"_spr);
        starSprite->setScale(0.18f);
        starSprite->setID("super-star-icon");
        starSprite->setAnchorPoint({ 0.f, 0.5f });
        m_mainLayer->addChild(starSprite);

        auto countLabel = CCLabelBMFont::create(std::to_string(g_streakData.superStars).c_str(), "goldFont.fnt");
        countLabel->setScale(0.5f);
        countLabel->setID("super-star-label");
        countLabel->setAnchorPoint({ 0.f, 0.5f });
        m_mainLayer->addChild(countLabel);

        CCPoint const counterPos = { 25.f, winSize.height - 25.f };
        starSprite->setPosition(counterPos);
        countLabel->setPosition(starSprite->getPosition() + CCPoint{ starSprite->getScaledContentSize().width + 5.f, 0 });

        return true;
    }

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

    void updatePage() {
        m_pageContainer->removeAllChildren();
        auto availableMissions = getAvailableMissionIDs();

        if (availableMissions.empty()) {
            m_leftArrow->setVisible(false);
            m_rightArrow->setVisible(false);

            auto allDoneText = "You have claimed all missions.\nCome back tomorrow.";
            auto allDoneLabel = CCLabelBMFont::create(allDoneText, "bigFont.fnt");
            allDoneLabel->setScale(0.5f);
            allDoneLabel->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
            allDoneLabel->setPosition({ 0.f, 0.f });
            allDoneLabel->setID("all-done-label");
            m_pageContainer->addChild(allDoneLabel);
            return;
        }

        int missionsPerPage = 3;
        int startIndex = m_currentPage * missionsPerPage;
        float topY = 50.f;
        float spacing = 50.f;

        for (int i = 0; i < missionsPerPage; ++i) {
            int missionIndex = startIndex + i;
            if (missionIndex < availableMissions.size()) {
                int missionID = availableMissions[missionIndex];
                if (auto missionNode = createPointMissionNode(missionID)) {
                    missionNode->setPosition(0, topY - (i * spacing));
                    missionNode->setTag(missionID);
                    m_pageContainer->addChild(missionNode);
                }
            }
        }

        int totalPages = static_cast<int>(ceil(static_cast<float>(availableMissions.size()) / missionsPerPage));
        if (totalPages == 0) totalPages = 1;
        if (m_currentPage >= totalPages && m_currentPage > 0) m_currentPage = totalPages - 1;

        m_leftArrow->setVisible(m_currentPage > 0);
        m_rightArrow->setVisible(m_currentPage < totalPages - 1);
    }

    void onSwitchPage(CCObject* sender) {
        if (m_isAnimating) return;
        int direction = sender->getTag();
        m_currentPage += direction;
        updatePage();
    }

    void onClaimReward(CCObject* sender) {
        if (m_isAnimating) return;
        m_isAnimating = true;

        FMODAudioEngine::sharedEngine()->playEffect("claim_mission.mp3"_spr);

        int missionID = sender->getTag();
        this->setTag(missionID);

        auto claimedNode = m_pageContainer->getChildByTag(missionID);
        if (!claimedNode) {
            m_isAnimating = false;
            return;
        }

        claimedNode->runAction(CCSequence::create(
            CCSpawn::create(
                CCEaseSineIn::create(CCMoveBy::create(0.3f, { 20, 0 })),
                CCFadeOut::create(0.3f),
                nullptr
            ),
            CCRemoveSelf::create(),
            nullptr
        ));

        this->runAction(CCSequence::create(
            CCDelayTime::create(0.4f),
            CCCallFunc::create(this, callfunc_selector(MissionsPopup::onAnimationEnd)),
            nullptr
        ));
    }

    void onAnimationEnd() {
        int missionID = this->getTag();
        int rewardStars = 0;
        int rewardXP = 0;
        int preStars = g_streakData.superStars;
        int preXP = g_streakData.currentXP;

        switch (missionID) {
        case 0: rewardStars = 2;  rewardXP = 15;  g_streakData.pointMission1Claimed = true; break;
        case 1: rewardStars = 3;  rewardXP = 30;  g_streakData.pointMission2Claimed = true; break;
        case 2: rewardStars = 5;  rewardXP = 50;  g_streakData.pointMission3Claimed = true; break;
        case 3: rewardStars = 8;  rewardXP = 75;  g_streakData.pointMission4Claimed = true; break;
        case 4: rewardStars = 9;  rewardXP = 100; g_streakData.pointMission5Claimed = true; break;
        case 5: rewardStars = 10; rewardXP = 150; g_streakData.pointMission6Claimed = true; break;
        }

        g_streakData.superStars += rewardStars;
        g_streakData.addXP(rewardXP);
        g_streakData.save();

        if (rewardStars > 0) {
            RewardNotification::show("super_star.png"_spr, preStars, rewardStars);
        }
        if (rewardXP > 0) {
            RewardNotification::show("xp.png"_spr, preXP, rewardXP);
        }

        auto countLabel = static_cast<CCLabelBMFont*>(this->m_mainLayer->getChildByIDRecursive("super-star-label"));
        if (countLabel) {
            countLabel->setString(std::to_string(g_streakData.superStars).c_str());
        }

        auto availableMissions = getAvailableMissionIDs();
        int missionsPerPage = 3;
        int startIndex = m_currentPage * missionsPerPage;
        if (startIndex >= availableMissions.size() && m_currentPage > 0) {
            m_currentPage = 0;
        }
        updatePage();
        m_isAnimating = false;
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