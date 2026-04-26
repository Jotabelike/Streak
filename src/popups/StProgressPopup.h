#pragma once
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/ScrollLayer.hpp>
#include <Geode/ui/Notification.hpp>
#include "../StreakData.h"
#include "../BannerNotification.h"
#include "../BadgeNotification.h"
#include "../RewardNotification.h"

using namespace geode::prelude;

class StProgressPopup : public Popup {
protected:
    ScrollLayer* m_scrollLayer = nullptr;

    struct StreakGoal {
        int pointsRequired;
        int levelRequired;
        std::string title;
        int rewardTickets = 0;
        std::string rewardBannerID = "";
        std::string rewardBadgeID = "";
    };

    std::vector<StreakGoal> m_goals = {
        { 500,   5,  "Beginner Streak", 1000, "", "" },
        { 1500,  5,  "Rookie Streak", 0, "banner_28", "" },
        { 5000,  10, "Pro Streak", 0, "", "diamond_streak_badge" },
        { 9000,  20, "Master Streak", 25000, "", "" },
        { 15000, 30, "Legendary Streak", 0, "banner_29", "" },
        { 20000, 40, "Top Streak", 0, "banner_42", "" },
        { 30000, 50, "Mythic Streak", 0, "banner_44", "" }
    };

    void setOpacityToChildren(CCNode* node, GLubyte opacity) {
        if (!node) return;
        auto children = node->getChildren();
        if (!children) return;

        for (auto* child : CCArrayExt<CCNode*>(children)) {
            if (auto sprite = typeinfo_cast<CCSprite*>(child)) {
                sprite->setOpacity(opacity);
            }
            else if (auto label = typeinfo_cast<CCLabelBMFont*>(child)) {
                label->setOpacity(opacity);
            }
            else if (auto layer = typeinfo_cast<CCLayerColor*>(child)) {
                layer->setOpacity(opacity);
            }
            else if (auto menu = typeinfo_cast<CCMenu*>(child)) {
                setOpacityToChildren(menu, opacity);
            }
        }
    }

    bool isGoalClaimed(int index) {
        return g_streakData.isStreakGoalClaimed(index);
    }

    void setGoalClaimed(int index) {
        g_streakData.setStreakGoalClaimed(index);
        g_streakData.save();
    }

    CCNode* createRewardNode(const StreakGoal& goal) {
        auto node = CCNode::create();

        if (!goal.rewardBannerID.empty()) {
            auto info = g_streakData.getBannerInfo(goal.rewardBannerID);
            std::string spriteName = info ? info->spriteName : "GJ_button_01.png";

            auto spr = CCSprite::create(spriteName.c_str());
            if (!spr) {
                spr = CCSprite::createWithSpriteFrameName("GJ_unknownBtn_001.png");
            }

            if (spr) {
                float maxW = 64.f;
                float scale = maxW / spr->getContentSize().width;
                if (scale > 0.6f) {
                    scale = 0.6f;
                }
                spr->setScale(scale);
                spr->setPosition({ 0, 1.f });
                node->addChild(spr);
            }
        }
        else if (!goal.rewardBadgeID.empty()) {
            auto info = g_streakData.getBadgeInfo(goal.rewardBadgeID);
            std::string spriteName = info ? info->spriteName : "GJ_unknownBtn_001.png";

            auto spr = CCSprite::create(spriteName.c_str());
            if (!spr) {
                spr = CCSprite::createWithSpriteFrameName("GJ_unknownBtn_001.png");
            }

            if (spr) {
                spr->setScale(0.24f);
                spr->setPosition({ 0, 1.f });
                node->addChild(spr);
            }
        }
        else if (goal.rewardTickets > 0) {
            auto spr = CCSprite::create("star_tiket.png"_spr);
            spr->setScale(0.24f);
            spr->setPosition({ 0.f, 4.f });
            node->addChild(spr);

            auto lbl = CCLabelBMFont::create(
                fmt::format("x{}", goal.rewardTickets).c_str(),
                "goldFont.fnt"
            );
            lbl->setScale(0.35f);
            lbl->setAnchorPoint({ 0.5f, 1.0f });
            lbl->setPosition({ 0.f, -8.f });
            node->addChild(lbl);
        }

        return node;
    }

    void onClaim(CCObject* sender) {
      
        auto btn = static_cast<CCNode*>(sender);
        CCPoint spawnPos = btn->convertToWorldSpaceAR(CCPointZero);

        int index = btn->getTag();
        if (index < 0 || index >= m_goals.size()) return;
        if (isGoalClaimed(index)) return;

        const auto& goal = m_goals[index];

        if (!goal.rewardBannerID.empty()) {
            g_streakData.unlockBanner(goal.rewardBannerID);
            auto info = g_streakData.getBannerInfo(goal.rewardBannerID);
            if (info) {
                BannerNotification::show(
                    goal.rewardBannerID,
                    info->spriteName,
                    info->displayName,
                    g_streakData.getCategoryName(info->rarity),
                    g_streakData.getCategoryColor(info->rarity)
                );
            }
        }
        else if (!goal.rewardBadgeID.empty()) {
            g_streakData.unlockBadge(goal.rewardBadgeID);
            BadgeNotification::show(goal.rewardBadgeID);
        }
        else if (goal.rewardTickets > 0) {
            int start = g_streakData.starTickets;
            g_streakData.starTickets += goal.rewardTickets;

       
            RewardNotification::show("star_tiket.png"_spr, start, goal.rewardTickets, spawnPos);
        }

        setGoalClaimed(index);
        g_streakData.save();
        FMODAudioEngine::sharedEngine()->playEffect("dummyDestroy.ogg");
        refreshList();
    }

    CCNode* createGoalCell(const StreakGoal& goal, int index) {
        auto cell = CCNode::create();

        float width = 260.f;
        float height = 54.f;
        cell->setContentSize({ width, height });

        cell->ignoreAnchorPointForPosition(false);
        cell->setAnchorPoint({ 0.5f, 0.5f });

        auto bg = cocos2d::extension::CCScale9Sprite::create("square02_001.png");
        bg->setContentSize({ width, height });
        bg->setOpacity(60);
        bg->setPosition(width / 2, height / 2);
        cell->addChild(bg);

        bool isUnlocked = (g_streakData.currentLevel >= goal.levelRequired);
        int currentPoints = g_streakData.totalStreakPoints;
        bool isReached = (currentPoints >= goal.pointsRequired);
        bool isClaimed = isGoalClaimed(index);

        float leftMargin = 20.f;
        float textMargin = 45.f;

        auto rewardNode = createRewardNode(goal);
        rewardNode->setPosition({ width - 35.f, height / 2 });
        cell->addChild(rewardNode);

        if (!isUnlocked) {
            auto lockIcon = CCSprite::createWithSpriteFrameName("GJ_lockGray_001.png");
            lockIcon->setPosition({ leftMargin, height / 2 });
            lockIcon->setScale(0.6f);
            cell->addChild(lockIcon);

            auto title = CCLabelBMFont::create(goal.title.c_str(), "goldFont.fnt");
            title->setScale(0.5f);
            title->setAnchorPoint({ 0, 0.5f });
            title->setPosition({ textMargin, height / 2 + 8.f });
            title->setColor({ 150, 150, 150 });
            cell->addChild(title);

            auto reqLabel = CCLabelBMFont::create(
                fmt::format("Requires Level {}", goal.levelRequired).c_str(),
                "bigFont.fnt"
            );
            reqLabel->setScale(0.3f);
            reqLabel->setAnchorPoint({ 0, 0.5f });
            reqLabel->setPosition({ textMargin, height / 2 - 10.f });
            reqLabel->setColor({ 255, 100, 100 });
            cell->addChild(reqLabel);

            if (rewardNode) {
                setOpacityToChildren(rewardNode, 100);
            }
        }
        else {
            auto icon = CCSprite::create("streak_point.png"_spr);
            if (!icon) {
                icon = CCSprite::createWithSpriteFrameName("GJ_starsIcon_001.png");
            }
            icon->setPosition({ leftMargin, height / 2 });
            icon->setScale(0.3f);
            cell->addChild(icon);

            auto title = CCLabelBMFont::create(goal.title.c_str(), "goldFont.fnt");
            title->setScale(0.5f);
            title->setAnchorPoint({ 0, 0.5f });
            title->setPosition({ textMargin, height / 2 + 10.f });
            cell->addChild(title);

            float barWidth = 125.f;
            float barHeight = 8.f;
            float percent = std::clamp(
                (float)currentPoints / (float)goal.pointsRequired,
                0.f,
                1.f
            );
            float barYPos = height / 2 - 12.f;

            auto barBorder = CCLayerColor::create(
                { 255, 255, 255, 100 },
                barWidth + 2.f,
                barHeight + 2.f
            );
            barBorder->setPosition({ textMargin - 1.f, barYPos - 1.f });
            cell->addChild(barBorder);

            auto barBg = CCLayerColor::create(
                { 0, 0, 0, 150 },
                barWidth,
                barHeight
            );
            barBg->setPosition({ textMargin, barYPos });
            cell->addChild(barBg);

            ccColor4B startColor = isReached ? ccColor4B{ 0, 255, 0, 255 } : ccColor4B{ 255, 200, 0, 255 };
            ccColor4B endColor = isReached ? ccColor4B{ 0, 200, 0, 255 } : ccColor4B{ 255, 100, 0, 255 };

            auto barFill = CCLayerGradient::create(startColor, endColor);
            barFill->setContentSize({ barWidth * percent, barHeight });
            barFill->setPosition({ textMargin, barYPos });
            cell->addChild(barFill);

            auto progressLabel = CCLabelBMFont::create(
                fmt::format("{}/{}", currentPoints, goal.pointsRequired).c_str(),
                "bigFont.fnt"
            );
            progressLabel->setScale(0.3f);
            progressLabel->setPosition({
                textMargin + barWidth / 2,
                barYPos + barHeight / 2 + 1.f
                });

            auto shadow = CCLabelBMFont::create(progressLabel->getString(), "bigFont.fnt");
            shadow->setScale(0.3f);
            shadow->setColor({ 0,0,0 });
            shadow->setOpacity(150);
            shadow->setPosition(progressLabel->getPosition() + ccp(1, -1));

            cell->addChild(shadow);
            cell->addChild(progressLabel);

            bool showReward = true;

            if (isClaimed) {
                auto check = CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
                check->setPosition({ width - 25.f, height / 2 });
                check->setScale(0.6f);
                cell->addChild(check, 10);

                if (rewardNode) {
                    setOpacityToChildren(rewardNode, 100);
                }
            }
            else if (isReached) {
                auto claimSpr = ButtonSprite::create(
                    "Claim", 0, 0, "goldFont.fnt", "GJ_button_01.png", 0, 0.5f
                );
                claimSpr->setScale(0.65f);

                auto claimBtn = CCMenuItemSpriteExtra::create(
                    claimSpr,
                    this,
                    menu_selector(StProgressPopup::onClaim)
                );
                claimBtn->setTag(index);

                auto menu = CCMenu::createWithItem(claimBtn);
                menu->setPosition({ width - 35.f, height / 2 });
                cell->addChild(menu, 10);

                claimBtn->runAction(CCRepeatForever::create(CCSequence::create(
                    CCScaleTo::create(0.5f, 1.1f),
                    CCScaleTo::create(0.5f, 1.0f),
                    nullptr
                )));

                showReward = false;
            }

            if (rewardNode) {
                rewardNode->setVisible(showReward);
            }
        }

        return cell;
    }

    void refreshList() {
        if (!m_scrollLayer) return;
        m_scrollLayer->m_contentLayer->removeAllChildren();

        auto listSize = m_scrollLayer->getContentSize();
        auto content = CCMenu::create();

        float itemHeight = 58.f;
        float topPadding = 10.f;
        float bottomPadding = 10.f;
        float totalHeight = std::max(
            listSize.height,
            (m_goals.size() * itemHeight) + topPadding + bottomPadding
        );

        content->setContentSize({ listSize.width, totalHeight });
        content->setPosition(0, 0);

        for (size_t i = 0; i < m_goals.size(); ++i) {
            auto cell = createGoalCell(m_goals[i], i);
            float yPos = totalHeight - topPadding - (i * itemHeight) - (itemHeight / 2);
            cell->setPosition(listSize.width / 2, yPos);
            content->addChild(cell);
        }

        m_scrollLayer->m_contentLayer->addChild(content);
        m_scrollLayer->m_contentLayer->setContentSize(content->getContentSize());
    }

    bool init() override {
        if (!Popup::init(320.f, 220.f, "geode.loader/GE_square03.png")) return false;
        this->setTitle("Streak Points Progress");
        auto winSize = m_mainLayer->getContentSize();

        auto listSize = CCSize{ 280.f, 130.f };

        auto bg = cocos2d::extension::CCScale9Sprite::create("square02_001.png");
        bg->setContentSize(listSize);
        bg->setColor({ 0, 0, 0 });
        bg->setOpacity(80);
        bg->setPosition(winSize.width / 2, winSize.height / 2 - 12.f);
        m_mainLayer->addChild(bg);

        m_scrollLayer = ScrollLayer::create(listSize);
        m_scrollLayer->setPosition((winSize - listSize) / 2);
        m_scrollLayer->setPositionY(bg->getPositionY() - listSize.height / 2);
        m_mainLayer->addChild(m_scrollLayer);

        refreshList();
        m_scrollLayer->moveToTop();

        return true;
    }

public:
    static StProgressPopup* create() {
        auto ret = new StProgressPopup();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};