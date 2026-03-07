#pragma once
#include "StreakCommon.h"
#include "SharedVisuals.h"
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/ScrollLayer.hpp>
#include <Geode/binding/GameLevelManager.hpp>
#include <Geode/binding/LevelInfoLayer.hpp>
#include <Geode/binding/TextAlertPopup.hpp> 
#include <Geode/ui/Notification.hpp> 
#include <Geode/utils/cocos.hpp>
#include "../BadgeNotification.h"
#include "../RewardNotification.h"

using namespace geode::prelude;

class LevelProgressPopup : public Popup {
protected:
    ScrollLayer* m_scrollLayer = nullptr;

    void onViewLevel(CCObject* sender) {
        int levelID = sender->getTag();
        auto searchObject = GJSearchObject::create(SearchType::Search, std::to_string(levelID));
        auto scene = LevelBrowserLayer::scene(searchObject);
        CCDirector::sharedDirector()->replaceScene(
            CCTransitionFade::create(0.5f, scene)
        );
    }

    void onCopyLevelID(CCObject* sender) {
        int levelID = sender->getTag();
        geode::utils::clipboard::write(std::to_string(levelID));
        Notification::create(
            "ID Copied",
            NotificationIcon::None,
            0.5f
        )->show();

    }

    void onMissionInfo(CCObject*) {
        FLAlertLayer::create(
            "Info",
            "Complete levels to unlock rewards.\nLevels show your current % progress.",
            "OK"
        )->show();
    }

    void setupProgressBar() {
        auto winSize = m_mainLayer->getContentSize();
        if (auto oldBar = m_mainLayer->getChildByTag(999)) {
            oldBar->removeFromParent();
        }

        int total = g_levelMissions.size();
        int claimed = 0;
        for (const auto& m : g_levelMissions) {
            if (g_streakData.isLevelMissionClaimed(m.levelID)) {
                claimed++;
            }
        }
        float percent = total > 0 ? (float)claimed / (float)total : 0.f;

        auto barNode = CCNode::create();
        barNode->setTag(999);
        float barWidth = 260.f;
        float barHeight = 10.f;

        auto outer = CCLayerColor::create(
            { 0, 0, 0, 70 },
            barWidth + 6,
            barHeight + 6
        );
        outer->setPosition({ -(barWidth / 2) - 3, -(barHeight / 2) - 3 });
        barNode->addChild(outer);

        auto border = CCLayerColor::create(
            { 255, 255, 255, 120 },
            barWidth + 2,
            barHeight + 2
        );
        border->setPosition({ -(barWidth / 2) - 1, -(barHeight / 2) - 1 });
        barNode->addChild(border);

        auto barBg = CCLayerColor::create(
            { 45, 45, 45, 255 },
            barWidth,
            barHeight
        );
        barBg->setPosition({ -(barWidth / 2), -(barHeight / 2) });
        barNode->addChild(barBg);

        auto barFill = CCLayerGradient::create(
            { 0, 255, 255, 255 },
            { 0, 100, 200, 255 }
        );
        barFill->setContentSize({ barWidth * percent, barHeight });
        barFill->setVector({ 1, 0 });
        barFill->setPosition({ -(barWidth / 2), -(barHeight / 2) });
        barNode->addChild(barFill);

        int percentInt = static_cast<int>(percent * 100);
        auto percentLabel = CCLabelBMFont::create(
            fmt::format("{}%", percentInt).c_str(),
            "bigFont.fnt"
        );
        percentLabel->setScale(0.35f);
        percentLabel->setPosition({ 0, 2.0f });

        auto shadow = CCLabelBMFont::create(
            fmt::format("{}%", percentInt).c_str(),
            "bigFont.fnt"
        );
        shadow->setScale(0.35f);
        shadow->setColor({ 0,0,0 });
        shadow->setOpacity(150);
        shadow->setPosition({ 1.5f, 0.5f });

        barNode->addChild(shadow, 6);
        barNode->addChild(percentLabel, 7);

        auto countLabel = CCLabelBMFont::create(
            fmt::format("{}/{}", claimed, total).c_str(),
            "goldFont.fnt"
        );
        countLabel->setScale(0.45f);
        countLabel->setPosition({ 0, -15.f });

        barNode->addChild(countLabel);

        barNode->setPosition({ winSize.width / 2, winSize.height - 40.f });
        m_mainLayer->addChild(barNode);
    }

    CCNode* createLevelMissionNode(const LevelMission& mission) {
        GJGameLevel* level = GameLevelManager::sharedState()->getSavedLevel(mission.levelID);
        int normalPercent = level ? level->m_normalPercent : 0;
        bool isComplete = (normalPercent >= 100);
        bool isMissionClaimed = g_streakData.isLevelMissionClaimed(mission.levelID);

        auto container = cocos2d::extension::CCScale9Sprite::create("GJ_square02.png");
        container->setContentSize({ 300.f, 55.f });

        auto badgeInfo = g_streakData.getBadgeInfo(mission.rewardBadgeID);
        ccColor3B titleColor = badgeInfo ? g_streakData.getCategoryColor(badgeInfo->category) : ccColor3B{ 255, 255, 255 };

        auto nameLabel = CCLabelBMFont::create(
            mission.levelName.c_str(),
            "goldFont.fnt"
        );
        nameLabel->limitLabelWidth(130.f, 0.6f, 0.1f);
        nameLabel->setAnchorPoint({ 0, 0.5f });
        nameLabel->setPosition({ 12.f, 41.f });
        nameLabel->setColor(titleColor);
        container->addChild(nameLabel);

        CCMenu* btnMenu = CCMenu::create();
        btnMenu->setPosition({ 0, 0 });
        container->addChild(btnMenu);

        auto idLabel = CCLabelBMFont::create(
            fmt::format("ID: {}", mission.levelID).c_str(),
            "chatFont.fnt"
        );
        idLabel->setScale(0.4f);
        idLabel->setColor({ 180, 180, 180 });
        idLabel->setOpacity(200);

        auto idBtn = CCMenuItemSpriteExtra::create(
            idLabel,
            this,
            menu_selector(LevelProgressPopup::onCopyLevelID)
        );
        idBtn->setTag(mission.levelID);

        float idBtnWidth = idLabel->getScaledContentSize().width;
        idBtn->setPosition({ 12.f + (idBtnWidth / 2), 28.f });
        btnMenu->addChild(idBtn);

        float lvlBarWidth = 100.f;
        float lvlBarHeight = 6.f;
        float percentDecimal = std::min(100, std::max(0, normalPercent)) / 100.f;

        auto lvlOuter = CCLayerColor::create(
            { 0, 0, 0, 100 },
            lvlBarWidth + 2,
            lvlBarHeight + 2
        );
        lvlOuter->setPosition({ 12.f - 1.f, 12.f - 1.f });
        container->addChild(lvlOuter);

        auto lvlBorder = CCLayerColor::create(
            { 255, 255, 255, 100 },
            lvlBarWidth + 1,
            lvlBarHeight + 1
        );
        lvlBorder->setPosition({ 12.f - 0.5f, 12.f - 0.5f });
        container->addChild(lvlBorder);

        auto lvlBarBg = CCLayerColor::create(
            { 40, 40, 40, 255 },
            lvlBarWidth,
            lvlBarHeight
        );
        lvlBarBg->setPosition({ 12.f, 12.f });
        container->addChild(lvlBarBg);

        ccColor4B startColor = (percentDecimal >= 1.f) ? ccColor4B{ 0, 255, 0, 255 } : ccColor4B{ 255, 200, 0, 255 };
        ccColor4B endColor = (percentDecimal >= 1.f) ? ccColor4B{ 0, 180, 0, 255 } : ccColor4B{ 255, 120, 0, 255 };

        auto lvlBarFill = CCLayerGradient::create(startColor, endColor);
        lvlBarFill->setContentSize({ lvlBarWidth * percentDecimal, lvlBarHeight });
        lvlBarFill->setVector({ 1, 0 });
        lvlBarFill->setPosition({ 12.f, 12.f });
        container->addChild(lvlBarFill);

        auto percentTxt = CCLabelBMFont::create(
            fmt::format("{}%", normalPercent).c_str(),
            "chatFont.fnt"
        );
        percentTxt->setScale(0.3f);
        percentTxt->setPosition({ 12.f + lvlBarWidth + 15.f, 14.5f });
        container->addChild(percentTxt);

        auto rewardNode = RewardCycleNode::create(40.f, 40.f);
        if (rewardNode) {
            rewardNode->setPosition({ 210.f, 27.5f });
            container->addChild(rewardNode);
            if (badgeInfo) {
                rewardNode->addReward(badgeInfo->spriteName, "Badge", 0.18f);
            }

            if (mission.secondaryRewardType != LevelRewardType::None) {
                rewardNode->addReward(
                    mission.secondaryRewardSprite,
                    fmt::format("x{}", mission.secondaryRewardQuantity),
                    0.20f
                );
            }
        }

        float buttonsX = 278.f;
        float statusY = 15.f;

        auto viewSpr = CCSprite::createWithSpriteFrameName("GJ_playBtn2_001.png");
        viewSpr->setScale(0.28f);
        auto viewBtn = CCMenuItemSpriteExtra::create(
            viewSpr,
            this,
            menu_selector(LevelProgressPopup::onViewLevel)
        );
        viewBtn->setTag(mission.levelID);
        viewBtn->setPosition({ buttonsX, 38.f });
        btnMenu->addChild(viewBtn);

        if (isMissionClaimed) {
            auto checkmark = CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
            checkmark->setScale(0.6f);
            checkmark->setPosition({ buttonsX, statusY });
            container->addChild(checkmark);
            container->setOpacity(150);
        }
        else if (isComplete) {
            auto claimBtnSprite = ButtonSprite::create(
                "Claim", 0, 0, "goldFont.fnt", "GJ_button_01.png", 0, 0.5f
            );
            claimBtnSprite->setScale(0.7f);

            auto claimBtn = CCMenuItemSpriteExtra::create(
                claimBtnSprite,
                this,
                menu_selector(LevelProgressPopup::onClaimMission)
            );
            claimBtn->setTag(mission.levelID);
            claimBtn->setPosition({ buttonsX, statusY });
            btnMenu->addChild(claimBtn);
        }
        else {
            auto lockIcon = CCSprite::createWithSpriteFrameName("GJ_lockGray_001.png");
            if (!lockIcon) {
                lockIcon = CCSprite::createWithSpriteFrameName("GJ_lock_001.png");
            }
            lockIcon->setScale(0.5f);
            lockIcon->setPosition({ buttonsX, statusY });
            container->addChild(lockIcon);
        }
        return container;
    }

    void loadList() {
        if (!m_scrollLayer) return;

        auto content = CCMenu::create();
        content->setPosition({ 0, 0 });

        float itemHeight = 60.f;
        float totalHeight = g_levelMissions.size() * itemHeight;

        if (totalHeight < m_scrollLayer->getContentSize().height) {
            totalHeight = m_scrollLayer->getContentSize().height;
        }

        content->setContentSize({ m_scrollLayer->getContentSize().width, totalHeight });

        for (size_t i = 0; i < g_levelMissions.size(); ++i) {
            if (auto cell = createLevelMissionNode(g_levelMissions[i])) {
                float yPos = totalHeight - (i * itemHeight) - (itemHeight / 2);
                cell->setPosition({ m_scrollLayer->getContentSize().width / 2, yPos });
                content->addChild(cell);
            }
        }

        m_scrollLayer->m_contentLayer->addChild(content);
        m_scrollLayer->m_contentLayer->setContentSize(content->getContentSize());
        m_scrollLayer->moveToTop();
    }

    void onClaimMission(CCObject* sender) {
        int levelID = sender->getTag();
        if (g_streakData.isLevelMissionClaimed(levelID)) return;

        const LevelMission* missionPtr = nullptr;
        for (const auto& m : g_levelMissions) {
            if (m.levelID == levelID) {
                missionPtr = &m;
                break;
            }
        }

        if (missionPtr) {
            g_streakData.completedLevelMissions.insert(levelID);

            if (!missionPtr->rewardBadgeID.empty()) {
                bool alreadyHadBadge = g_streakData.isBadgeUnlocked(missionPtr->rewardBadgeID);
                g_streakData.unlockBadge(missionPtr->rewardBadgeID);
                if (!alreadyHadBadge) {
                    BadgeNotification::show(missionPtr->rewardBadgeID);
                }
            }

            if (missionPtr->secondaryRewardType == LevelRewardType::SuperStars) {
                int start = g_streakData.superStars;
                g_streakData.superStars += missionPtr->secondaryRewardQuantity;
                RewardNotification::show(
                    "super_star.png"_spr,
                    start,
                    missionPtr->secondaryRewardQuantity
                );
            }

            if (missionPtr->secondaryRewardType == LevelRewardType::StarTickets) {
                int start = g_streakData.starTickets;
                g_streakData.starTickets += missionPtr->secondaryRewardQuantity;
                RewardNotification::show(
                    "star_tiket.png"_spr,
                    start,
                    missionPtr->secondaryRewardQuantity
                );
            }
            g_streakData.save();
        }

        if (m_scrollLayer) {
            m_scrollLayer->m_contentLayer->removeAllChildren();
        }
        loadList();
        setupProgressBar();
    }

    bool init() override {
        if (!Popup::init(360.f, 270.f, "geode.loader/GE_square03.png")) return false;
        this->setTitle("Level Challenges");
        auto winSize = m_mainLayer->getContentSize();

        auto background = cocos2d::extension::CCScale9Sprite::create("square02_001.png");
        background->setColor({ 0, 0, 0 });
        background->setOpacity(100);
        background->setContentSize({ 320.f, 185.f });
        background->setPosition({ winSize.width / 2, winSize.height / 2 - 15.f });
        m_mainLayer->addChild(background);

        m_scrollLayer = ScrollLayer::create(background->getContentSize());
        m_scrollLayer->setPosition(
            background->getPosition() - background->getContentSize() / 2
        );
        m_mainLayer->addChild(m_scrollLayer);

        auto infoIcon = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
        infoIcon->setScale(0.7f);
        auto infoBtn = CCMenuItemSpriteExtra::create(
            infoIcon,
            this,
            menu_selector(LevelProgressPopup::onMissionInfo)
        );

        auto infoMenu = CCMenu::createWithItem(infoBtn);
        infoMenu->setPosition({ winSize.width - 25.f, winSize.height - 25.f });
        m_mainLayer->addChild(infoMenu);

        loadList();
        setupProgressBar();
        return true;
    }

public:
    static LevelProgressPopup* create() {
        auto ret = new LevelProgressPopup();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};