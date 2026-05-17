#pragma once
#include "StreakCommon.h"
#include "../StreakData.h"
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/ScrollLayer.hpp>
#include <Geode/ui/TextInput.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/utils/async.hpp>
#include "../BadgeNotification.h"
#include "../BannerNotification.h"
#include "../RewardNotification.h"
#include "SharedVisuals.h"
#include "../StatusSpinner.h"
#include "../utils/RoundedProgressBar.h"
#include <Geode/ui/Notification.hpp>
#include "../HMACAuth.h"
#include "../NameModifiers.h"
#include <random>
#include <algorithm>
#include <set>

using namespace geode::prelude;

class RewardListPopup : public Popup {

protected:
    bool init(const std::map<std::string, matjson::Value>& rewards,
        const std::map<std::string, int>& weights) {


        if (!Popup::init(400.f, 280.f, "geode.loader/GE_square03.png")) return false;

        this->setTitle("Possible Awards");

        auto popupSize = m_mainLayer->getContentSize();
        auto listSize = CCSize{ 360.f, 210.f };

        auto listBg = cocos2d::extension::CCScale9Sprite::create("square02_001.png");
        listBg->setContentSize(listSize + CCSize{ 10.f, 10.f });
        listBg->setColor({ 0, 0, 0 });
        listBg->setOpacity(100);
        listBg->setPosition(popupSize / 2 - CCPoint{ 0.f, 15.f });
        m_mainLayer->addChild(listBg);

        double totalWeight = 0;
        int defaultWeight = weights.count("Common") ? weights.at("Common") : 100;

        for (auto const& [key, val] : rewards) {
            std::string rarity = val["rarity"].as<std::string>().unwrapOr("Common");
            int w = weights.count(rarity) ? weights.at(rarity) : defaultWeight;
            totalWeight += w;
        }
        if (totalWeight <= 0) {
            totalWeight = 1;
        }

        auto scroll = ScrollLayer::create(listSize);
        scroll->setPosition(
            (popupSize.width - listSize.width) / 2,
            (popupSize.height - listSize.height) / 2 - 15.f
        );

        float itemHeight = 60.f;
        float totalHeight = std::max(listSize.height, rewards.size() * itemHeight);

        scroll->m_contentLayer->setContentSize({ listSize.width, totalHeight });

        int index = 0;
        std::vector<std::pair<std::string, matjson::Value>> sortedRewards(rewards.begin(), rewards.end());

        for (const auto& [key, data] : sortedRewards) {
            float yPos = totalHeight - (index * itemHeight) - (itemHeight / 2);
            int opacity = (index % 2 == 0) ? 100 : 50;

            auto bg = CCLayerColor::create(
                { 0, 0, 0, (GLubyte)opacity },
                listSize.width,
                itemHeight
            );
            bg->setPosition({ 0.f, yPos - (itemHeight / 2) });
            scroll->m_contentLayer->addChild(bg);

            std::string rarity = data["rarity"].as<std::string>().unwrapOr("Common");
            int amount = 0;
            CCSprite* icon = nullptr;
            std::string name = "";
            bool owned = false;
            bool isConsumable = false;

            if (data.contains("badge")) {
                std::string bID = data["badge"].as<std::string>().unwrapOr("");
                if (auto binfo = g_streakData.getBadgeInfo(bID)) {
                    icon = CCSprite::create(binfo->spriteName.c_str());
                    name = "Badge";
                    if (g_streakData.isBadgeUnlocked(bID)) {
                        owned = true;
                    }
                }
            }
            else if (data.contains("banner")) {
                std::string bID = data["banner"].as<std::string>().unwrapOr("");
                if (auto binfo = g_streakData.getBannerInfo(bID)) {
                    icon = CCSprite::create(binfo->spriteName.c_str());
                    name = "Banner";
                    if (g_streakData.isBannerUnlocked(bID)) {
                        owned = true;
                    }
                }
            }
            else if (data.contains("super_stars")) {
                amount = data["super_stars"].as<int>().unwrapOr(0);
                icon = CCSprite::create("super_star.png"_spr);
                name = "Super Stars";
                isConsumable = true;
            }
            else if (data.contains("star_tickets")) {
                amount = data["star_tickets"].as<int>().unwrapOr(0);
                icon = CCSprite::create("star_tiket.png"_spr);
                name = "Tickets";
                isConsumable = true;
            }

            else if (data.contains("gems")) {
                amount = data["gems"].as<int>().unwrapOr(0);
                icon = CCSprite::create("gem.png"_spr);
                name = "Gems";
                isConsumable = true;
            }
            if (!icon) {
                icon = CCSprite::createWithSpriteFrameName("GJ_questionMark_001.png");
            }

            float scale;
            if (isConsumable) {
                scale = 0.5f;
                if (icon->getContentSize().width > 80.f) {
                    scale = 0.25f;
                }
            }
            else {
                scale = 0.75f;
                if (icon->getContentSize().width > 80.f) {
                    scale = 0.45f;
                }
            }

            icon->setScale(scale);
            icon->setPosition({ 40.f, yPos });
            scroll->m_contentLayer->addChild(icon);

            if (owned) {
                auto check = CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
                check->setScale(0.9f);
                check->setPosition({ 40.f, yPos });
                scroll->m_contentLayer->addChild(check, 10);
            }

            std::string desc = fmt::format(
                "{} x{}",
                name,
                amount > 0 ? std::to_string(amount) : "1"
            );

            auto labelName = CCLabelBMFont::create(desc.c_str(), "bigFont.fnt");
            labelName->setScale(0.4f);
            labelName->setAnchorPoint({ 0.f, 0.5f });
            labelName->setPosition({ 80.f, yPos + 12.f });
            scroll->m_contentLayer->addChild(labelName);

            auto labelRarity = CCLabelBMFont::create(rarity.c_str(), "goldFont.fnt");
            labelRarity->setScale(0.4f);
            labelRarity->setAnchorPoint({ 0.f, 0.5f });
            labelRarity->setPosition({ 80.f, yPos - 12.f });

            ccColor3B rColor = { 255, 255, 255 };
            std::string rLower = rarity;
            std::transform(rLower.begin(), rLower.end(), rLower.begin(), ::tolower);

            if (rLower == "common") rColor = { 255, 255, 255 };
            else if (rLower == "special") rColor = { 0, 255, 0 };
            else if (rLower == "epic") rColor = { 180, 50, 255 };
            else if (rLower == "legendary") rColor = { 255, 140, 0 };
            else if (rLower == "mythic") rColor = { 255, 0, 0 };

            labelRarity->setColor(rColor);
            scroll->m_contentLayer->addChild(labelRarity);

            int w = weights.count(rarity) ? weights.at(rarity) : defaultWeight;
            double prob = (w / totalWeight) * 100.0;

            auto labelProb = CCLabelBMFont::create(
                fmt::format("{:.1f}%", prob).c_str(),
                "goldFont.fnt"
            );
            labelProb->setScale(0.45f);
            labelProb->setAnchorPoint({ 1.f, 0.5f });
            labelProb->setPosition({ listSize.width - 30.f, yPos });
            labelProb->setColor({ 255, 255, 0 });
            scroll->m_contentLayer->addChild(labelProb);

            index++;
        }

        scroll->m_contentLayer->setPositionY(listSize.height - totalHeight);
        m_mainLayer->addChild(scroll);

        return true;
    }

public:
    static RewardListPopup* create(const std::map<std::string, matjson::Value>& rewards, const std::map<std::string, int>& weights) {
        auto ret = new RewardListPopup();
        if (ret && ret->init(rewards, weights)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

class EventRoulette : public CCLayer {
public:
    CCNode* m_scrollingLayer = nullptr;
    std::vector<std::string> m_rewardIDs;
    std::function<void()> m_onSpinRequest;
    std::function<void()> m_onAnimFinished;

    float m_itemWidth = 95.0f;
    int m_rejoiceCount = 40;

    bool m_isSpinning = false;
    int m_lastSoundIndex = -1;
    float m_soundTimer = 0.f;

    static EventRoulette* create(const std::vector<std::pair<std::string, matjson::Value>>& rewards,
        int cost, std::string currency, std::function<void()> onSpin) {
        auto ret = new EventRoulette();
        if (ret && ret->init(rewards, cost, currency, onSpin)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    ccColor3B getColorForRarity(std::string rarity) {
        std::transform(rarity.begin(), rarity.end(), rarity.begin(), ::tolower);
        if (rarity == "common") return { 255, 255, 255 };
        if (rarity == "special") return { 0, 255, 0 };
        if (rarity == "epic") return { 180, 50, 255 };
        if (rarity == "legendary") return { 255, 140, 0 };
        if (rarity == "mythic") return { 255, 0, 0 };
        return { 200, 200, 200 };
    }

    bool init(const std::vector<std::pair<std::string, matjson::Value>>& rewards,
        int cost, std::string currency, std::function<void()> onSpin) {
        if (!CCLayer::init()) return false;

        m_onSpinRequest = onSpin;
        this->setContentSize({ 340.f, 180.f });

        auto bg = cocos2d::extension::CCScale9Sprite::create("square02_001.png");
        if (bg) {
            bg->setContentSize({ 320.f, 90.f });
            bg->setColor({ 20, 20, 20 });
            bg->setOpacity(200);
            bg->setPosition({ this->getContentSize().width / 2, 100.f });
            this->addChild(bg);
        }

        auto clipper = CCClippingNode::create();
        if (clipper) {
            clipper->setContentSize({ 300.f, 80.f });
            clipper->setAnchorPoint({ 0.5f, 0.5f });
            clipper->setPosition({ this->getContentSize().width / 2, 100.f });

            auto stencil = CCLayerColor::create({ 255, 255, 255, 255 });
            if (stencil) {
                stencil->setContentSize({ 300.f, 80.f });
                clipper->setStencil(stencil);
            }
            this->addChild(clipper);

            m_scrollingLayer = CCNode::create();
            if (m_scrollingLayer) {
                m_scrollingLayer->setPosition({ 0.f, 52.f });
                clipper->addChild(m_scrollingLayer);
            }
        }

        if (!m_scrollingLayer) return true;
        if (rewards.empty()) return true;

        int totalItems = rewards.size();
        for (int cycle = 0; cycle < m_rejoiceCount; ++cycle) {
            for (int i = 0; i < totalItems; ++i) {
                if (cycle == 0) {
                    m_rewardIDs.push_back(rewards[i].first);
                }

                matjson::Value data = rewards[i].second;
                CCSprite* spr = nullptr;
                int amount = 0;
                std::string rarity = data["rarity"].as<std::string>().unwrapOr("Common");

                if (data.contains("badge")) {
                    std::string badgeID = data["badge"].as<std::string>().unwrapOr("");
                    auto binfo = g_streakData.getBadgeInfo(badgeID);
                    if (binfo) spr = CCSprite::create(binfo->spriteName.c_str());
                    if (!spr) spr = CCSprite::createWithSpriteFrameName("GJ_secretChest_001.png");
                }
                else if (data.contains("banner")) {
                    std::string bannerID = data["banner"].as<std::string>().unwrapOr("");
                    auto binfo = g_streakData.getBannerInfo(bannerID);
                    if (binfo) spr = CCSprite::create(binfo->spriteName.c_str());
                    if (!spr) spr = CCSprite::createWithSpriteFrameName("GJ_secretChest_001.png");
                }
                else if (data.contains("super_stars")) {
                    amount = data["super_stars"].as<int>().unwrapOr(0);
                    spr = CCSprite::create("super_star.png"_spr);
                }
                else if (data.contains("star_tickets")) {
                    amount = data["star_tickets"].as<int>().unwrapOr(0);
                    spr = CCSprite::create("star_tiket.png"_spr);
                }
                else if (data.contains("gems")) {
                    amount = data["gems"].as<int>().unwrapOr(0);
                    spr = CCSprite::create("gem.png"_spr);
                }

                if (!spr) spr = CCSprite::createWithSpriteFrameName("GJ_questionMark_001.png");
                if (!spr) spr = CCSprite::createWithSpriteFrameName("edit_delBtn_001.png");

                if (spr) {
                    float scale = 0.35f;
                    if (spr->getContentSize().width > 90.f) scale = 0.22f;
                    spr->setScale(scale);

                    float posX = (cycle * totalItems * m_itemWidth) + (i * m_itemWidth) + (m_itemWidth / 2);
                    spr->setPosition({ posX, 10.f });
                    m_scrollingLayer->addChild(spr);

                    if (amount > 0) {
                        auto label = CCLabelBMFont::create(
                            fmt::format("x{}", amount).c_str(),
                            "goldFont.fnt"
                        );
                        if (label) {
                            label->setScale(0.35f);
                            label->setPosition({ posX, -20.f });
                            m_scrollingLayer->addChild(label);
                        }
                    }
                    else if (data.contains("badge")) {
                        auto label = CCLabelBMFont::create("Badge", "goldFont.fnt");
                        if (label) {
                            label->setScale(0.35f);
                            label->setPosition({ posX, -20.f });
                            m_scrollingLayer->addChild(label);
                        }
                    }

                    auto rarityLbl = CCLabelBMFont::create(rarity.c_str(), "goldFont.fnt");
                    if (rarityLbl) {
                        rarityLbl->setScale(0.25f);
                        rarityLbl->setPosition({ posX, -35.f });
                        rarityLbl->setColor(getColorForRarity(rarity));
                        m_scrollingLayer->addChild(rarityLbl);
                    }
                }
            }
        }

        auto arrow = CCSprite::create("trist.png"_spr);
        if (arrow) {
            arrow->setPosition({ this->getContentSize().width / 2, 148.f });
            arrow->setRotation(180);
            arrow->setScale(0.8f);
            this->addChild(arrow, 10);
        }

        auto arrowBottom = CCSprite::create("trist.png"_spr);
        if (arrowBottom) {
            arrowBottom->setPosition({ this->getContentSize().width / 2, 52.f });
            arrowBottom->setScale(0.8f);
            this->addChild(arrowBottom, 10);
        }

        auto menu = CCMenu::create();
        if (menu) {
            menu->setPosition({ this->getContentSize().width / 2, -8.f });
            this->addChild(menu, 10);

            std::string btnText = "Spin";
            const char* currencySpr = "";

            if (cost > 0) {
                btnText = fmt::format("     {}", cost);
                if (currency == "stars") currencySpr = "super_star.png"_spr;
                else if (currency == "gems") currencySpr = "gem.png"_spr; 
                else currencySpr = "star_tiket.png"_spr;
            }

            auto btnSpr = ButtonSprite::create(
                btnText.c_str(),
                0,
                false,
                "goldFont.fnt",
                "GJ_button_01.png",
                0,
                0.8f
            );

            if (btnSpr) {
                if (cost > 0 && strlen(currencySpr) > 0) {
                    auto icon = CCSprite::create(currencySpr);
                    if (!icon) icon = CCSprite::create("star_tiket.png"_spr);

                    if (icon) {
                        icon->setScale(0.2f);
                        icon->setPosition({ 25.f, btnSpr->getContentSize().height / 2 });
                        btnSpr->addChild(icon);
                    }
                }

                auto btn = CCMenuItemSpriteExtra::create(
                    btnSpr,
                    this,
                    menu_selector(EventRoulette::onSpinClick)
                );
                if (btn) menu->addChild(btn);
            }
        }

        this->scheduleUpdate();
        return true;
    }

    void onSpinClick(CCObject*) {
        if (m_onSpinRequest) m_onSpinRequest();
    }

    void stopSpinning() {
        m_isSpinning = false;
    }

    void update(float dt) override {
        if (!m_isSpinning || !m_scrollingLayer) return;

        m_soundTimer += dt;

        float currentX = std::abs(m_scrollingLayer->getPositionX());
        float viewCenter = currentX + 150.f;

        int currentIndex = static_cast<int>(viewCenter / m_itemWidth);

        if (currentIndex != m_lastSoundIndex) {
            if (m_soundTimer > 0.06f) {
                FMODAudioEngine::sharedEngine()->playEffect("ruleta_sfx.mp3"_spr);
                m_soundTimer = 0.f;
            }
            m_lastSoundIndex = currentIndex;
        }
    }

    void spinToID(std::string winID, std::function<void()> onFinishAnim) {
        m_onAnimFinished = onFinishAnim;

        m_scrollingLayer->stopAllActions();
        m_scrollingLayer->setPosition({ 0.f, 52.f });

        int baseIndex = -1;
        for (size_t i = 0; i < m_rewardIDs.size(); ++i) {
            if (m_rewardIDs[i] == winID) {
                baseIndex = i;
                break;
            }
        }

        if (baseIndex == -1 || !m_scrollingLayer) {
            this->onAnimationDone();
            return;
        }

        m_isSpinning = true;
        m_soundTimer = 1.0f;

        int targetCycle = m_rejoiceCount - 5;
        int totalIndex = (targetCycle * m_rewardIDs.size()) + baseIndex;
        float targetX = -((totalIndex * m_itemWidth) + (m_itemWidth / 2)) + 150.f;

        auto move = CCEaseExponentialOut::create(
            CCMoveTo::create(6.0f, { targetX, 52.f })
        );

        auto seq = CCSequence::create(
            move,
            CCCallFunc::create(this, callfunc_selector(EventRoulette::stopSpinning)),
            CCDelayTime::create(0.5f),
            CCCallFunc::create(this, callfunc_selector(EventRoulette::onAnimationDone)),
            nullptr
        );
        m_scrollingLayer->runAction(seq);
    }

    void onAnimationDone() {
        if (m_onAnimFinished) m_onAnimFinished();
    }
};

class EventPopup : public Popup {
protected:
    CCLayer* m_contentLayer = nullptr;
    CCMenu* m_rewardMenu = nullptr;
    StatusSpinner* m_spinner = nullptr;
    EventRoulette* m_rouletteLayer = nullptr;
    CCLabelBMFont* m_ticketLabel = nullptr;

    std::string m_eventID;
    std::string m_eventName;
    std::string m_claimingID;
    std::string m_eventType = "days";

    int m_spinCost = 0;
    std::string m_spinCurrency = "tickets";

    matjson::Value m_eventRewards;
    matjson::Value m_playerProgress;
    std::map<std::string, int> m_rarityWeights;


    async::TaskHolder<web::WebResponse> m_loadTask;
    async::TaskHolder<web::WebResponse> m_claimTask;

    bool m_isClaiming = false;

    int m_tempStars = 0;
    int m_tempTickets = 0;
    int m_tempGems = 0;
    std::string m_tempBadgeID = "";
    std::string m_tempBannerID = "";
    bool m_tempSuccess = false;

    std::vector<CCLabelBMFont*> m_mythicLabels;
    std::vector<ccColor3B> m_mythicColors;
    int m_colorIndex = 0;
    float m_colorTransitionTime = 0.0f;
    ccColor3B m_currentColor;
    ccColor3B m_targetColor;

    bool init() override {
        if (!Popup::init(380.f, 280.f, "geode.loader/GE_square03.png")) return false;

        this->setTitle("Event");
        auto winSize = m_mainLayer->getContentSize();
        auto listSize = CCSize{ 340.f, 200.f };

        auto listBg = cocos2d::extension::CCScale9Sprite::create("square02_001.png");
        if (listBg) {
            listBg->setContentSize(listSize);
            listBg->setColor({ 0, 0, 0 });
            listBg->setOpacity(120);
            listBg->setPosition(winSize / 2);
        }

        m_contentLayer = CCLayer::create();
        m_contentLayer->setContentSize(listSize);
        if (listBg) {
            m_contentLayer->setPosition(listBg->getPosition() - listBg->getContentSize() / 2);
        }
        else {
            m_contentLayer->setPosition(winSize / 2 - listSize / 2);
        }
        m_mainLayer->addChild(m_contentLayer);

        auto bagSpr = CCSprite::create("bag.png"_spr);
        if (!bagSpr) {
            bagSpr = CCSprite::createWithSpriteFrameName("GJ_safeIcon_001.png");
        }

        if (bagSpr) {
            bagSpr->setScale(0.4f);
            auto bagBtn = CCMenuItemSpriteExtra::create(
                bagSpr,
                this,
                menu_selector(EventPopup::onRewardList)
            );
            auto bagMenu = CCMenu::create();
            bagMenu->addChild(bagBtn);

            float paddingX = 28.f;
            float paddingY = -10.f;

            bagMenu->setPosition({
                (winSize.width / 2) + (listSize.width / 2) - paddingX,
                (winSize.height / 2) - (listSize.height / 2) + paddingY
                });
            m_mainLayer->addChild(bagMenu, 50);
        }

        auto streakIcon = CCSprite::create(g_streakData.getRachaSprite().c_str());
        if (!streakIcon) streakIcon = CCSprite::create("racha0.png"_spr);
        if (streakIcon) {
            streakIcon->setScale(0.18f);
            streakIcon->setAnchorPoint({ 0.f, 0.5f });
        }

        auto streakLabel = CCLabelBMFont::create(
            std::to_string(g_streakData.currentStreak).c_str(),
            "goldFont.fnt"
        );
        if (streakLabel) {
            streakLabel->setScale(0.6f);
            streakLabel->setAnchorPoint({ 0.f, 0.5f });
        }

        float popupTopY = (winSize.height / 2) + (listSize.height / 2);
        float popupLeftX = (winSize.width / 2) - (listSize.width / 2);
        float popupBottomY = (winSize.height / 2) - (listSize.height / 2) - 15.f;

        if (streakIcon) {
            streakIcon->setPosition({ popupLeftX + 10.f, popupTopY + 20.f });
            m_mainLayer->addChild(streakIcon);
        }
        if (streakLabel && streakIcon) {
            streakLabel->setPosition({
                streakIcon->getPositionX() + 25.f,
                streakIcon->getPositionY()
                });
            m_mainLayer->addChild(streakLabel);
        }

        auto ticketIcon = CCSprite::create("star_tiket.png"_spr);
        if (ticketIcon) {
            ticketIcon->setScale(0.25f);
            ticketIcon->setPosition({ popupLeftX + 20.f, popupBottomY });
            m_mainLayer->addChild(ticketIcon);

            m_ticketLabel = CCLabelBMFont::create(
                std::to_string(g_streakData.starTickets).c_str(),
                "goldFont.fnt"
            );
            if (m_ticketLabel) {
                m_ticketLabel->setScale(0.5f);
                m_ticketLabel->setAnchorPoint({ 0.0f, 0.5f });
                m_ticketLabel->setPosition({
                    ticketIcon->getPositionX() + 15.f,
                    ticketIcon->getPositionY()
                    });
                m_mainLayer->addChild(m_ticketLabel);
            }
        }

        m_spinner = StatusSpinner::create();
        m_spinner->setPosition(winSize / 2);
        m_mainLayer->addChild(m_spinner, 20);

        m_mythicColors = {
            ccc3(255, 0, 0), ccc3(255, 165, 0), ccc3(255, 255, 0),
            ccc3(0, 255, 0), ccc3(0, 0, 255), ccc3(75, 0, 130),
            ccc3(238, 130, 238)
        };
        m_currentColor = m_mythicColors[0];
        m_targetColor = m_mythicColors[1];

        this->scheduleUpdate();
        this->loadEvent();
        return true;
    }

    void onRewardList(CCObject*) {
        if (!m_eventRewards.isObject()) return;

        std::map<std::string, matjson::Value> rewardsMap;
        try {
            rewardsMap = m_eventRewards.as<std::map<std::string, matjson::Value>>().unwrap();
        }
        catch (...) { return; }

        if (m_rarityWeights.empty()) {
            m_rarityWeights["Common"] = 100;
        }

        auto popup = RewardListPopup::create(rewardsMap, m_rarityWeights);
        popup->show();
    }

    void update(float dt) override {
        if (m_mythicLabels.empty()) return;
        m_colorTransitionTime += dt;
        if (m_colorTransitionTime >= 1.0f) {
            m_colorTransitionTime = 0.0f;
            m_colorIndex = (m_colorIndex + 1) % m_mythicColors.size();
            m_currentColor = m_targetColor;
            m_targetColor = m_mythicColors[(m_colorIndex + 1) % m_mythicColors.size()];
        }
        float progress = m_colorTransitionTime / 1.0f;
        ccColor3B interpolatedColor = {
            static_cast<GLubyte>(m_currentColor.r + (m_targetColor.r - m_currentColor.r) * progress),
            static_cast<GLubyte>(m_currentColor.g + (m_targetColor.g - m_currentColor.g) * progress),
            static_cast<GLubyte>(m_currentColor.b + (m_targetColor.b - m_currentColor.b) * progress)
        };
        for (auto* label : m_mythicLabels) {
            label->setColor(interpolatedColor);
        }
    }

    void onClose(CCObject* sender) override {
        this->unscheduleUpdate();
        Popup::onClose(sender);
    }

    void updateLabels() {
        if (m_ticketLabel) {
            m_ticketLabel->setString(
                std::to_string(g_streakData.starTickets).c_str()
            );
        }
    }

    void loadEvent() {
        m_spinner->setLoading("Loading...");
        m_contentLayer->setVisible(false);
        int accountID = GJAccountManager::sharedState()->m_accountID;
        auto req = web::WebRequest();
        HMACAuth::signGetRequest(req, accountID);

        m_loadTask.spawn(
            req.get(fmt::format(
                "https://streak-servidor.onrender.com/event/current/{}?type=roulette",
                GJAccountManager::sharedState()->m_accountID
            )),
            [this](web::WebResponse res) {
                this->onEventResponse(res);
            }
        );
    }

    void onEventResponse(web::WebResponse& res) {
        if (res.code() == 404) {
            m_spinner->setError("No active event");
            return;
        }

        if (res.ok() && res.json().isOk()) {
            auto data = res.json().unwrap();
            auto eventData = data["event"];

            bool activeNormal = eventData["isActive"].as<bool>().unwrapOr(false);
            bool activeRoulette = eventData["isActiveRoulette"].as<bool>().unwrapOr(false);

            if (!activeNormal && !activeRoulette) {
                m_spinner->setError("No active event");
                return;
            }

            m_eventID = eventData["eventID"].as<std::string>().unwrapOr("error_id");
            m_eventName = eventData["eventName"].as<std::string>().unwrapOr("Event");
            m_eventType = eventData["eventType"].as<std::string>().unwrapOr("days");

            m_spinCost = eventData["spinCost"].as<int>().unwrapOr(0);
            m_spinCurrency = eventData["spinCurrency"].as<std::string>().unwrapOr("tickets");

            m_eventRewards = eventData["rewards"];
            m_playerProgress = data["progress"];

            m_rarityWeights.clear();
            if (eventData.contains("rarityWeights")) {
                auto wMap = eventData["rarityWeights"].as<std::map<std::string, int>>().unwrapOr(std::map<std::string, int>());
                m_rarityWeights = wMap;
            }

            this->setTitle(m_eventName.c_str());

            m_spinner->hide();
            m_contentLayer->setVisible(true);

            if (m_eventType == "roulette") {
                this->buildRoulette();
            }
            else {
                this->buildList();
            }
        }
        else {
            m_spinner->setError("Error loading");
        }
    }

    void buildRoulette() {
        m_contentLayer->removeAllChildren();
        m_mythicLabels.clear();
        m_rouletteLayer = nullptr;
        m_rewardMenu = nullptr;

        if (!m_eventRewards.isObject()) return;

        std::vector<std::pair<std::string, matjson::Value>> rewardsVec;
        auto rewardsMap = m_eventRewards.as<std::map<std::string, matjson::Value>>().unwrap();
        for (const auto& pair : rewardsMap) {
            rewardsVec.push_back(pair);
        }

        m_rouletteLayer = EventRoulette::create(
            rewardsVec,
            m_spinCost,
            m_spinCurrency,
            [this]() { this->onClaimRoulette(); }
        );

        if (m_rouletteLayer) {
            m_rouletteLayer->setPosition({
                (m_contentLayer->getContentSize().width - m_rouletteLayer->getContentSize().width) / 2,
                (m_contentLayer->getContentSize().height - m_rouletteLayer->getContentSize().height) / 2
                });
            m_contentLayer->addChild(m_rouletteLayer);
        }
    }

    void onClaimRoulette() {
        if (m_isClaiming) return;

        if (m_spinCost > 0) {
            if (m_spinCurrency == "stars" && g_streakData.superStars < m_spinCost) {
                FLAlertLayer::create("Error", "You don't have enough Super Stars", "OK")->show();
                return;
            }
           
            if (m_spinCurrency == "gems" && g_streakData.gems < m_spinCost) {
                FLAlertLayer::create("Error", "You don't have enough Gems", "OK")->show();
                return;
            }
            if (m_spinCurrency != "stars" && m_spinCurrency != "gems" && g_streakData.starTickets < m_spinCost) {
                FLAlertLayer::create("Error", "You don't have enough Star Tickets", "OK")->show();
                return;
            }
        }

        m_isClaiming = true;
        m_spinner->setLoading("Connecting...");
        m_contentLayer->setVisible(false);

        matjson::Value payload = matjson::Value::object();
        payload.set("accountID", GJAccountManager::sharedState()->m_accountID);
        payload.set("eventID", m_eventID);
        payload.set("claimID", "roulette_spin");

        auto req = web::WebRequest();
        HMACAuth::signRequest(req, GJAccountManager::sharedState()->m_accountID, payload);
        req.bodyJSON(payload);

        m_claimTask.spawn(
            req.post("https://streak-servidor.onrender.com/event/claim"),
            [this](web::WebResponse res) {
                this->onClaimResponse(res);
            }
        );
    }

    void buildList() {
        m_contentLayer->removeAllChildren();
        m_mythicLabels.clear();
        m_rouletteLayer = nullptr;
        m_rewardMenu = nullptr;
        if (!m_eventRewards.isObject()) return;
        m_spinner->setError("List Mode Disabled in this version");
    }

    void onClaimResponse(web::WebResponse& res) {
        m_tempSuccess = res.ok();
        m_spinner->hide();
        m_contentLayer->setVisible(true);

        if (m_tempSuccess) {
            if (m_spinCost > 0) {
                if (m_spinCurrency == "stars") g_streakData.superStars -= m_spinCost;
                else if (m_spinCurrency == "gems") g_streakData.gems -= m_spinCost;  
                else g_streakData.starTickets -= m_spinCost;
                this->updateLabels();
            }
            auto jsonRes = res.json().unwrapOr(matjson::Value::object());
            m_claimingID = jsonRes["rewardID"].as<std::string>().unwrapOr("");

            if (m_eventRewards.contains(m_claimingID)) {
                auto rewardData = m_eventRewards[m_claimingID];
                parseRewardData(rewardData);
            }

            if (m_eventType == "roulette" && m_rouletteLayer) {
                m_rouletteLayer->spinToID(m_claimingID, [this]() { this->finalizeClaim(); });
                return;
            }
        }
        else {
            if (res.code() == 402) {
                FLAlertLayer::create("Error", "Insufficient Balance on Server", "OK")->show();
            }
            else {
                FLAlertLayer::create("Error", "Error processing turn", "OK")->show();
            }
        }

        float remainingTime = 1.0f;
        this->runAction(CCSequence::create(
            CCDelayTime::create(remainingTime),
            CCCallFunc::create(this, callfunc_selector(EventPopup::restoreUI)),
            nullptr
        ));
    }

    void parseRewardData(matjson::Value& rewardData) {
        m_tempStars = 0;
        m_tempTickets = 0;
        m_tempGems = 0;
        m_tempBadgeID = "";
        m_tempBannerID = "";

        if (rewardData.contains("super_stars")) {
            m_tempStars = rewardData["super_stars"].as<int>().unwrapOr(0);
        }
        if (rewardData.contains("star_tickets")) {
            m_tempTickets = rewardData["star_tickets"].as<int>().unwrapOr(0);
        }
        if (rewardData.contains("gems")) {
            m_tempGems = rewardData["gems"].as<int>().unwrapOr(0);
        }
        if (rewardData.contains("badge")) {
            m_tempBadgeID = rewardData["badge"].as<std::string>().unwrapOr("");
        }
        if (rewardData.contains("banner")) {
            m_tempBannerID = rewardData["banner"].as<std::string>().unwrapOr("");
        }
    }

    void finalizeClaim() {
        if (m_tempSuccess) {
            int starsStart = g_streakData.superStars;
            int ticketsStart = g_streakData.starTickets;
            int gemsStart = g_streakData.gems;
            g_streakData.superStars += m_tempStars;
            g_streakData.starTickets += m_tempTickets;
            g_streakData.gems += m_tempGems;

            bool isNewBadge = false;
            if (!m_tempBadgeID.empty()) {
                isNewBadge = !g_streakData.isBadgeUnlocked(m_tempBadgeID);
                if (isNewBadge) {
                    g_streakData.unlockBadge(m_tempBadgeID);
                }
            }

            bool isNewBanner = false;
            if (!m_tempBannerID.empty()) {
                isNewBanner = !g_streakData.isBannerUnlocked(m_tempBannerID);
                if (isNewBanner) {
                    g_streakData.unlockBanner(m_tempBannerID);
                }
            }

            g_streakData.save();
            this->updateLabels();

          
            CCPoint spawnPos = CCDirector::sharedDirector()->getWinSize() / 2;
            if (m_rouletteLayer) {
              
                spawnPos = m_rouletteLayer->convertToWorldSpace({ m_rouletteLayer->getContentSize().width / 2, 100.f });
            }

            auto showConsumables = [this, starsStart, ticketsStart, gemsStart, spawnPos]() {
                if (m_tempStars > 0) {
                    RewardNotification::show("super_star.png"_spr, starsStart, m_tempStars, spawnPos);
                }
                if (m_tempTickets > 0) {
                    RewardNotification::show("star_tiket.png"_spr, ticketsStart, m_tempTickets, spawnPos);
                }
                if (m_tempGems > 0) {
                    RewardNotification::show("gem.png"_spr, gemsStart, m_tempGems, spawnPos);
                }
                };

            if (!m_tempBadgeID.empty()) {
                if (isNewBadge) {
                    auto* badgeInfo = g_streakData.getBadgeInfo(m_tempBadgeID);
                    if (badgeInfo && badgeInfo->category == StreakData::BadgeCategory::MYTHIC) {
                        auto animLayer = MythicAnimationLayer::create(
                            *badgeInfo,
                            [this, showConsumables]() {
                                BadgeNotification::show(m_tempBadgeID);
                                showConsumables();
                            }
                        );
                        CCDirector::sharedDirector()->getRunningScene()->addChild(animLayer, 99999);
                    }
                    else {
                        BadgeNotification::show(m_tempBadgeID);
                        showConsumables();
                    }
                }
                else {
                    Notification::create("You already have this item", NotificationIcon::Info)->show();
                    showConsumables();
                }
            }
            else if (!m_tempBannerID.empty()) {
                if (isNewBanner) {
                    BadgeNotification::show(m_tempBannerID);
                    showConsumables();
                }
                else {
                    Notification::create("You already have this item", NotificationIcon::Info)->show();
                    showConsumables();
                }
            }
            else {
                showConsumables();
            }

            m_isClaiming = false;
        }
        else {
            this->restoreUI();
        }
    }

    void restoreUI() {
        m_isClaiming = false;
        m_spinner->hide();
        m_contentLayer->setVisible(true);
        this->loadEvent();
    }

public:
    static EventPopup* create() {
        auto ret = new EventPopup();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};


 
class KeysEventLayer : public CCLayer {
public:
    enum class State { Idle, Shuffling, WaitingPick, Revealing, Done };

    struct CardSlot {
        CCSprite* back = nullptr;
        CCSprite* faceIcon = nullptr;
        CCLabelBMFont* faceAmount = nullptr;
        CCMenuItemSpriteExtra* button = nullptr;
        CCPoint basePos;
        std::string rewardID;
        matjson::Value rewardData;
        bool revealed = false;
    };

    State m_state = State::Idle;
    std::vector<CardSlot> m_cards;
    CCMenu* m_cardMenu = nullptr;
    CCNode* m_cardContainer = nullptr;
    CCLabelBMFont* m_statusLabel = nullptr;
    CCMenuItemSpriteExtra* m_spinBtn = nullptr;
    ButtonSprite* m_spinBtnSpr = nullptr;
    CCMenuItemSpriteExtra* m_skipBtn = nullptr;
    std::function<void()> m_onSpinRequest;
    std::function<void(int)> m_onCardPicked;
    std::function<void()> m_onSkipRequest;
    int m_cardCount = 8;
    int m_cardsPerRow = 4;
    float m_cardSpacing = 65.f;
    int m_cost = 19;
    int m_originalWinningSlot = -1;
    std::string m_currency = "gems";

    static KeysEventLayer* create(int cost, const std::string& currency,
        std::function<void()> onSpin,
        std::function<void(int)> onPick,
        std::function<void()> onSkip) {
        auto ret = new KeysEventLayer();
        if (ret && ret->init(cost, currency, onSpin, onPick, onSkip)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    bool init(int cost, const std::string& currency,
        std::function<void()> onSpin,
        std::function<void(int)> onPick,
        std::function<void()> onSkip) {
        if (!CCLayer::init()) return false;

        m_cost = cost;
        m_currency = currency;
        m_onSpinRequest = onSpin;
        m_onCardPicked = onPick;
        m_onSkipRequest = onSkip;

        const float layerW = 340.f;
        const float layerH = 215.f;
        this->setContentSize({ layerW, layerH });

        auto bg = cocos2d::extension::CCScale9Sprite::create("square02_001.png");
        bg->setContentSize({ 320.f, 165.f });
        bg->setColor({ 20, 20, 20 });
        bg->setOpacity(200);
        bg->setPosition({ layerW / 2, layerH / 2 - 5.f });
        this->addChild(bg);

        m_cardContainer = CCNode::create();
        m_cardContainer->setContentSize({ layerW, 165.f });
        m_cardContainer->setPosition({ 0.f, 30.f });
        this->addChild(m_cardContainer);

        m_cardMenu = CCMenu::create();
        m_cardMenu->setPosition({ 0.f, 0.f });
        m_cardContainer->addChild(m_cardMenu, 5);

        const float rowTopY = 110.f;
        const float rowBottomY = 50.f;
        const float rowsTotalWidth = m_cardSpacing * m_cardsPerRow;

        for (int i = 0; i < m_cardCount; ++i) {
            CardSlot slot;
            int row = i / m_cardsPerRow;
            int col = i % m_cardsPerRow;
            float x = (m_cardSpacing * col) + (m_cardSpacing / 2) + ((layerW - rowsTotalWidth) / 2);
            float y = (row == 0) ? rowTopY : rowBottomY;
            slot.basePos = ccp(x, y);

            slot.back = CCSprite::create("keyscard.png"_spr);
            if (!slot.back) {
                slot.back = CCSprite::createWithSpriteFrameName("GJ_secretChest_001.png");
            }
            if (slot.back) {
                float scale = 0.5f;
                if (slot.back->getContentSize().width > 0) {
                    scale = 48.f / slot.back->getContentSize().width;
                    if (scale > 0.6f) scale = 0.6f;
                }
                slot.back->setScale(scale);
            }

            slot.button = CCMenuItemSpriteExtra::create(
                slot.back, this, menu_selector(KeysEventLayer::onCardClicked)
            );
            slot.button->setTag(i);
            slot.button->setPosition(slot.basePos);
            slot.button->setEnabled(false);
            m_cardMenu->addChild(slot.button);

            m_cards.push_back(slot);
        }

        m_statusLabel = CCLabelBMFont::create("Press Spin to shuffle the keys!", "goldFont.fnt");
        m_statusLabel->setScale(0.4f);
        m_statusLabel->setPosition({ layerW / 2, layerH - 12.f });
        this->addChild(m_statusLabel, 10);

        auto spinMenu = CCMenu::create();
        spinMenu->setPosition({ layerW / 2, 2.f });
        this->addChild(spinMenu, 10);

        std::string btnText = (m_cost > 0)
            ? fmt::format("    {}", m_cost)
            : std::string("Spin");

        m_spinBtnSpr = ButtonSprite::create(
            btnText.c_str(), 0, false, "goldFont.fnt",
            "GJ_button_01.png", 0, 0.8f
        );

        if (m_spinBtnSpr && m_cost > 0) {
            const char* currSpr = "gem.png"_spr;
            if (m_currency == "stars") currSpr = "super_star.png"_spr;
            else if (m_currency == "tickets") currSpr = "star_tiket.png"_spr;
            auto icon = CCSprite::create(currSpr);
            if (icon) {
                icon->setScale(0.2f);
                icon->setPosition({ 20.f, m_spinBtnSpr->getContentSize().height / 2 });
                m_spinBtnSpr->addChild(icon);
            }
        }

        m_spinBtn = CCMenuItemSpriteExtra::create(
            m_spinBtnSpr, this, menu_selector(KeysEventLayer::onSpinPressed)
        );
        spinMenu->addChild(m_spinBtn);

        auto skipSpr = ButtonSprite::create("Skip", 0, false, "goldFont.fnt", "GJ_button_04.png", 0, 0.6f);
        m_skipBtn = CCMenuItemSpriteExtra::create(
            skipSpr, this, menu_selector(KeysEventLayer::onSkipPressed)
        );
        m_skipBtn->setPosition({ 100.f, 0.f });
        m_skipBtn->setVisible(false);
        spinMenu->addChild(m_skipBtn);

        return true;
    }

    void onSkipPressed(CCObject*) {
        if (m_state != State::Shuffling) return;
        if (m_onSkipRequest) m_onSkipRequest();
        this->stopAllActions();
        for (auto& slot : m_cards) {
            if (slot.button) {
                slot.button->stopAllActions();
                slot.button->setScale(1.f);
                slot.button->setRotation(0.f);
                slot.button->setPosition(slot.basePos);
            }
            if (slot.back) {
                slot.back->stopAllActions();
                slot.back->setVisible(true);
            }
            if (slot.faceIcon) slot.faceIcon->setVisible(false);
            if (slot.faceAmount) slot.faceAmount->setVisible(false);
            slot.revealed = false;
        }
        this->finishShuffle();
        if (m_skipBtn) m_skipBtn->setVisible(false);
    }

    void onSpinPressed(CCObject*) {
        if (m_state != State::Idle && m_state != State::Done) return;
        if (m_onSpinRequest) m_onSpinRequest();
    }

    void onCardClicked(CCObject* sender) {
        if (m_state != State::WaitingPick) return;
        auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
        int idx = btn->getTag();
        if (idx < 0 || idx >= (int)m_cards.size()) return;
        m_state = State::Revealing;
        for (auto& c : m_cards) {
            if (c.button) c.button->setEnabled(false);
        }
        if (m_onCardPicked) m_onCardPicked(idx);
    }

    void resetCards() {
        m_originalWinningSlot = -1;
        for (auto& slot : m_cards) {
            if (slot.faceIcon) {
                slot.faceIcon->removeFromParent();
                slot.faceIcon = nullptr;
            }
            if (slot.faceAmount) {
                slot.faceAmount->removeFromParent();
                slot.faceAmount = nullptr;
            }
            if (slot.button) {
                slot.button->stopAllActions();
                slot.button->setPosition(slot.basePos);
                slot.button->setScale(1.f);
                slot.button->setEnabled(false);
                slot.button->setVisible(true);
            }
            if (slot.back) {
                slot.back->stopAllActions();
                CCSize sz = slot.back->getContentSize();
                if (slot.button) {
                    CCSize bsz = slot.button->getContentSize();
                    slot.back->setPosition({ bsz.width / 2, bsz.height / 2 });
                } else {
                    slot.back->setPosition({ sz.width / 2, sz.height / 2 });
                }
                slot.back->setRotation(0.f);
                slot.back->setVisible(true);
                slot.back->setOpacity(255);
            }
            slot.revealed = false;
            slot.rewardID.clear();
            slot.rewardData = matjson::Value::object();
        }
    }

    void startShuffle(const std::vector<std::pair<std::string, matjson::Value>>& decoys,
        const std::string& winningID,
        const matjson::Value& winningData) {
        resetCards();
        m_state = State::Shuffling;
        if (m_skipBtn) m_skipBtn->setVisible(true);

        int winSlot = rand() % m_cardCount;
        m_originalWinningSlot = winSlot;

        for (int i = 0; i < (int)m_cards.size(); ++i) {
            if (i == winSlot) {
                m_cards[i].rewardID = winningID;
                m_cards[i].rewardData = winningData;
            } else if (!decoys.empty()) {
                int decoyIdx = (i < winSlot ? i : i - 1) % (int)decoys.size();
                m_cards[i].rewardID = decoys[decoyIdx].first;
                m_cards[i].rewardData = decoys[decoyIdx].second;
            }
            attachRewardFace(m_cards[i]);
        }

        m_statusLabel->setString("Revealing prizes...");
        flipAllCards(true, [this]() {
            m_statusLabel->setString("Memorize the prizes!");
            this->runAction(CCSequence::create(
                CCDelayTime::create(3.0f),
                CCCallFunc::create(this,
                    callfunc_selector(KeysEventLayer::onPreviewDone)),
                nullptr));
        });
    }

    void onPreviewDone() {
        m_statusLabel->setString("Watch carefully...");
        flipAllCards(false, [this]() {
            this->beginShuffleInternal();
        });
    }

    void flipAllCards(bool toFaceUp, std::function<void()> done) {
        m_pendingFlipAllCallback = done;

        const float stagger = 0.05f;
        const float halfFlip = 0.15f;
        const float perCard = halfFlip * 2.f;

        for (size_t i = 0; i < m_cards.size(); ++i) {
            auto& slot = m_cards[i];
            if (!slot.button) continue;
            float origY = slot.button->getScaleY();

            auto shrink = CCEaseIn::create(
                CCScaleTo::create(halfFlip, 0.f, origY), 2.0f);
            auto grow = CCEaseOut::create(
                CCScaleTo::create(halfFlip, 1.f, origY), 2.0f);
            auto swap = CCCallFuncN::create(this, toFaceUp
                ? callfuncN_selector(KeysEventLayer::onUncoverHalfway)
                : callfuncN_selector(KeysEventLayer::onCoverHalfway));
            auto delay = CCDelayTime::create((float)i * stagger);

            slot.button->runAction(CCSequence::create(
                delay, shrink, swap, grow, nullptr));
        }

        float totalTime = ((float)(m_cards.size() - 1) * stagger) + perCard + 0.05f;
        this->runAction(CCSequence::create(
            CCDelayTime::create(totalTime),
            CCCallFunc::create(this,
                callfunc_selector(KeysEventLayer::onFlipAllDone)),
            nullptr));
    }

    void onUncoverHalfway(CCNode* sender) {
        if (!sender) return;
        int idx = sender->getTag();
        if (idx < 0 || idx >= (int)m_cards.size()) return;
        auto& slot = m_cards[idx];
        if (slot.back) slot.back->setVisible(false);
        if (slot.faceIcon) slot.faceIcon->setVisible(true);
        if (slot.faceAmount) slot.faceAmount->setVisible(true);
    }

    void onCoverHalfway(CCNode* sender) {
        if (!sender) return;
        int idx = sender->getTag();
        if (idx < 0 || idx >= (int)m_cards.size()) return;
        auto& slot = m_cards[idx];
        if (slot.faceIcon) slot.faceIcon->setVisible(false);
        if (slot.faceAmount) slot.faceAmount->setVisible(false);
        if (slot.back) slot.back->setVisible(true);
    }

    void onFlipAllDone() {
        if (m_pendingFlipAllCallback) {
            auto cb = m_pendingFlipAllCallback;
            m_pendingFlipAllCallback = nullptr;
            cb();
        }
    }

    void beginShuffleInternal() {
        m_statusLabel->setString("Shuffling the keys...");

        const int totalSwaps = 31;
        const float swapInterval = 0.30f;

        CCArray* actions = CCArray::create();
        for (int s = 0; s < totalSwaps; ++s) {
            actions->addObject(CCCallFunc::create(this,
                callfunc_selector(KeysEventLayer::doRandomSwap)));
            actions->addObject(CCDelayTime::create(swapInterval));
        }
        actions->addObject(CCCallFunc::create(this,
            callfunc_selector(KeysEventLayer::finishShuffle)));

        this->runAction(CCSequence::create(actions));
    }

    void setWinningReward(int slotIdx, const std::string& rewardID,
        const matjson::Value& rewardData) {
        if (slotIdx < 0 || slotIdx >= (int)m_cards.size()) return;

        if (slotIdx != m_originalWinningSlot &&
            m_originalWinningSlot >= 0 &&
            m_originalWinningSlot < (int)m_cards.size()) {
            auto displacedID   = m_cards[slotIdx].rewardID;
            auto displacedData = m_cards[slotIdx].rewardData;
            m_cards[m_originalWinningSlot].rewardID   = displacedID;
            m_cards[m_originalWinningSlot].rewardData = displacedData;
            refreshSlotFace(m_originalWinningSlot);
        }

        m_cards[slotIdx].rewardID   = rewardID;
        m_cards[slotIdx].rewardData = rewardData;
        refreshSlotFace(slotIdx);
    }

    void refreshSlotFace(int idx) {
        if (idx < 0 || idx >= (int)m_cards.size()) return;
        auto& slot = m_cards[idx];
        if (slot.faceIcon) {
            slot.faceIcon->removeFromParent();
            slot.faceIcon = nullptr;
        }
        if (slot.faceAmount) {
            slot.faceAmount->removeFromParent();
            slot.faceAmount = nullptr;
        }
        attachRewardFace(slot);
    }

    void doRandomSwap() {
        if (m_cards.size() < 2) return;
        int a = rand() % m_cards.size();
        int b = rand() % m_cards.size();
        if (a == b) b = (b + 1) % m_cards.size();

        auto& cardA = m_cards[a];
        auto& cardB = m_cards[b];
        if (!cardA.button || !cardB.button) return;

        CCPoint posA = cardA.button->getPosition();
        CCPoint posB = cardB.button->getPosition();

        float arcHeight = 25.f;
        CCPoint midA = { (posA.x + posB.x) / 2.f, posA.y + arcHeight };
        CCPoint midB = { (posA.x + posB.x) / 2.f, posA.y - arcHeight };

        auto moveA = CCSequence::create(
            CCEaseInOut::create(CCMoveTo::create(0.10f, midA), 2.0f),
            CCEaseInOut::create(CCMoveTo::create(0.10f, posB), 2.0f),
            nullptr
        );
        auto moveB = CCSequence::create(
            CCEaseInOut::create(CCMoveTo::create(0.10f, midB), 2.0f),
            CCEaseInOut::create(CCMoveTo::create(0.10f, posA), 2.0f),
            nullptr
        );

        cardA.button->stopAllActions();
        cardB.button->stopAllActions();
        cardA.button->runAction(moveA);
        cardB.button->runAction(moveB);
    }

    void finishShuffle() {
        if (m_skipBtn) m_skipBtn->setVisible(false);

        std::vector<int> waveOrder;
        waveOrder.reserve(m_cards.size());
        for (int i = 0; i < (int)m_cards.size(); ++i) waveOrder.push_back(i);
        std::sort(waveOrder.begin(), waveOrder.end(), [this](int a, int b) {
            if (!m_cards[a].button || !m_cards[b].button) return a < b;
            auto pa = m_cards[a].button->getPosition();
            auto pb = m_cards[b].button->getPosition();
            if (pa.y != pb.y) return pa.y > pb.y;
            return pa.x < pb.x;
        });

        const float waveStagger = 0.08f;
        const float waveScale = 1.35f;
        const float waveHalf = 0.18f;

        for (size_t i = 0; i < waveOrder.size(); ++i) {
            int idx = waveOrder[i];
            auto& c = m_cards[idx];
            if (!c.button) continue;
            c.button->stopAllActions();
            float delay = (float)i * waveStagger;
            auto grow = CCEaseSineOut::create(
                CCScaleTo::create(waveHalf, waveScale));
            auto shrink = CCEaseSineIn::create(
                CCScaleTo::create(waveHalf, 1.f));
            c.button->runAction(CCSequence::create(
                CCDelayTime::create(delay),
                grow,
                shrink,
                CCCallFuncN::create(this, callfuncN_selector(KeysEventLayer::onCardSettled)),
                nullptr));
        }

        m_state = State::WaitingPick;
        m_statusLabel->setString("Pick a key!");
    }

    void onCardSettled(CCNode* sender) {
        int idx = sender ? sender->getTag() : -1;
        if (idx < 0 || idx >= (int)m_cards.size()) return;
        auto& slot = m_cards[idx];
        if (!slot.button) return;
        slot.button->setScale(1.f);
        slot.button->setEnabled(true);
        startFloating(slot, idx);
    }

    void startFloating(CardSlot& slot, int idx) {
        float phase = (float)(idx % 4) * 0.22f + (idx >= 4 ? 0.55f : 0.f);
        const float amp = 7.5f;
        const float halfPeriod = 1.1f;
        auto makeBob = [&]() {
            auto up = CCEaseInOut::create(
                CCMoveBy::create(halfPeriod, { 0.f, amp }), 2.4f);
            auto down = CCEaseInOut::create(
                CCMoveBy::create(halfPeriod, { 0.f, -amp }), 2.4f);
            return CCSequence::create(up, down, nullptr);
        };
        auto makeSway = [&]() {
            auto cw = CCEaseInOut::create(
                CCRotateTo::create(halfPeriod * 2.f, 3.f), 2.0f);
            auto ccw = CCEaseInOut::create(
                CCRotateTo::create(halfPeriod * 2.f, -3.f), 2.0f);
            return CCSequence::create(cw, ccw, nullptr);
        };
        if (slot.back) {
            slot.back->stopAllActions();
            slot.back->setRotation(0.f);
            slot.back->runAction(CCSequence::create(
                CCDelayTime::create(phase),
                CCRepeatForever::create(makeBob()),
                nullptr));
            slot.back->runAction(CCSequence::create(
                CCDelayTime::create(phase * 0.5f),
                CCRepeatForever::create(makeSway()),
                nullptr));
        }
    }

    void revealPicked(int slotIdx, std::function<void()> onDone) {
        if (slotIdx < 0 || slotIdx >= (int)m_cards.size()) {
            if (onDone) onDone();
            return;
        }
        auto& slot = m_cards[slotIdx];
        attachRewardFace(slot);
        flipCard(slot, [this, slotIdx, onDone]() {
            this->scheduleOnce(schedule_selector(KeysEventLayer::revealOthersTick), 0.35f);
            m_pendingDoneCallback = onDone;
            m_pendingExcludeIdx = slotIdx;
        });
    }

    void revealOthersTick(float dt) {
        for (int i = 0; i < (int)m_cards.size(); ++i) {
            if (i == m_pendingExcludeIdx) continue;
            auto& s = m_cards[i];
            if (s.revealed) continue;
            attachRewardFace(s);
            flipCard(s, nullptr);
        }
        this->scheduleOnce(schedule_selector(KeysEventLayer::finalizeReveal), 0.75f);
    }

    void finalizeReveal(float dt) {
        m_state = State::Done;
        if (m_pendingDoneCallback) {
            auto cb = m_pendingDoneCallback;
            m_pendingDoneCallback = nullptr;
            cb();
        }
    }

    void attachRewardFace(CardSlot& slot) {
        if (slot.faceIcon || !slot.button) return;

        CCSprite* icon = nullptr;
        int amount = 0;
        bool showAmount = false;
        auto& data = slot.rewardData;
        if (data.contains("fragment")) {
            icon = CCSprite::create("fragment.png"_spr);
            amount = data["fragment"].as<int>().unwrapOr(1);
            if (amount < 1) amount = 1;
            showAmount = true;
        } else if (data.contains("super_stars")) {
            icon = CCSprite::create("super_star.png"_spr);
            amount = data["super_stars"].as<int>().unwrapOr(0);
            showAmount = amount > 0;
        } else if (data.contains("star_tickets")) {
            icon = CCSprite::create("star_tiket.png"_spr);
            amount = data["star_tickets"].as<int>().unwrapOr(0);
            showAmount = amount > 0;
        } else if (data.contains("gems")) {
            icon = CCSprite::create("gem.png"_spr);
            amount = data["gems"].as<int>().unwrapOr(0);
            showAmount = amount > 0;
        } else if (data.contains("banner")) {
            std::string bID = data["banner"].as<std::string>().unwrapOr("");
            if (auto info = g_streakData.getBannerInfo(bID)) {
                icon = CCSprite::create(info->spriteName.c_str());
            }
        } else if (data.contains("badge")) {
            std::string bID = data["badge"].as<std::string>().unwrapOr("");
            if (auto info = g_streakData.getBadgeInfo(bID)) {
                icon = CCSprite::create(info->spriteName.c_str());
            }
        }
        if (!icon) icon = CCSprite::createWithSpriteFrameName("GJ_questionMark_001.png");
        if (!icon) return;

        float maxSize = 36.f;
        float w = icon->getContentSize().width;
        float h = icon->getContentSize().height;
        float scale = (w > h ? maxSize / w : maxSize / h);
        if (scale > 0.55f) scale = 0.55f;
        icon->setScale(scale);
        CCPoint cardPos = slot.button ? slot.button->getPosition() : slot.basePos;
        icon->setPosition(cardPos);
        icon->setVisible(false);
        m_cardContainer->addChild(icon, 6);
        slot.faceIcon = icon;

        if (showAmount) {
            auto lbl = CCLabelBMFont::create(fmt::format("x{}", amount).c_str(), "goldFont.fnt");
            lbl->setScale(0.4f);
            lbl->setAnchorPoint({ 0.5f, 1.f });
            lbl->setPosition({ cardPos.x, cardPos.y - 22.f });
            lbl->setVisible(false);
            m_cardContainer->addChild(lbl, 7);
            slot.faceAmount = lbl;
        }
    }

    void flipCard(CardSlot& slot, std::function<void()> after) {
        if (!slot.button || slot.revealed) {
            if (after) after();
            return;
        }
        slot.revealed = true;

        slot.button->stopAllActions();
        slot.button->setScale(1.f);
        if (slot.back) {
            slot.back->stopAllActions();
            CCSize bsz = slot.button->getContentSize();
            slot.back->setPosition({ bsz.width / 2, bsz.height / 2 });
            slot.back->setRotation(0.f);
        }
        float origY = slot.button->getScaleY();

        auto lift     = CCEaseSineOut::create(CCMoveBy::create(0.08f, { 0.f, 6.f }));
        auto shrinkX  = CCEaseIn::create(CCScaleTo::create(0.18f, 0.f, origY), 2.0f);
        auto swap     = CCCallFuncN::create(this, callfuncN_selector(KeysEventLayer::onFlipHalfway));
        auto growX    = CCEaseOut::create(CCScaleTo::create(0.18f, 1.f, origY), 2.0f);
        auto settle   = CCEaseElasticOut::create(
            CCScaleTo::create(0.18f, 1.f, origY), 0.5f);
        auto lower    = CCEaseSineIn::create(CCMoveBy::create(0.08f, { 0.f, -6.f }));

        m_flipMap[static_cast<CCNode*>(slot.button)] = &slot;

        if (after) {
            m_pendingFlipCallback = after;
            slot.button->runAction(CCSequence::create(
                lift, shrinkX, swap, growX, settle, lower,
                CCCallFunc::create(this, callfunc_selector(KeysEventLayer::onFlipDoneSingle)),
                nullptr));
        } else {
            slot.button->runAction(CCSequence::create(
                lift, shrinkX, swap, growX, settle, lower, nullptr));
        }
    }

    void onFlipHalfway(CCNode* sender) {
        FMODAudioEngine::sharedEngine()->playEffect("magicExplosion_03.ogg");
        auto it = m_flipMap.find(sender);
        if (it == m_flipMap.end()) return;
        CardSlot* slot = it->second;
        if (!slot) return;
        if (slot->back) slot->back->setVisible(false);

        CCPoint cardPos = slot->button ? slot->button->getPosition() : slot->basePos;

        if (slot->faceIcon) {
            slot->faceIcon->setPosition(cardPos);
            float targetScale = slot->faceIcon->getScale();
            slot->faceIcon->setScale(targetScale * 0.35f);
            slot->faceIcon->setVisible(true);
            slot->faceIcon->runAction(CCEaseElasticOut::create(
                CCScaleTo::create(0.35f, targetScale), 0.45f));
        }
        if (slot->faceAmount) {
            slot->faceAmount->setPosition({ cardPos.x, cardPos.y - 22.f });
            slot->faceAmount->setOpacity(0);
            slot->faceAmount->setVisible(true);
            slot->faceAmount->runAction(CCFadeIn::create(0.25f));
        }
    }

    void onFlipDoneSingle() {
        if (m_pendingFlipCallback) {
            auto cb = m_pendingFlipCallback;
            m_pendingFlipCallback = nullptr;
            cb();
        }
    }

    void setSpinEnabled(bool enabled) {
        if (m_spinBtn) m_spinBtn->setEnabled(enabled);
        if (m_spinBtnSpr) m_spinBtnSpr->setCascadeOpacityEnabled(true);
    }

    void setStatusText(const std::string& s) {
        if (m_statusLabel) m_statusLabel->setString(s.c_str());
    }

    int getCardCount() const { return m_cardCount; }

private:
    std::map<CCNode*, CardSlot*> m_flipMap;
    std::function<void()> m_pendingFlipCallback;
    std::function<void()> m_pendingDoneCallback;
    std::function<void()> m_pendingFlipAllCallback;
    int m_pendingExcludeIdx = -1;
};


class GiftFragmentsPopup : public Popup {
protected:
    TextInput* m_targetInput = nullptr;
    TextInput* m_amountInput = nullptr;
    CCMenuItemSpriteExtra* m_sendBtn = nullptr;
    StatusSpinner* m_spinner = nullptr;
    bool m_isSending = false;
    std::function<void(int)> m_onSent;
    async::TaskHolder<web::WebResponse> m_sendTask;

    bool init(std::function<void(int)> onSent) {
        if (!Popup::init(260.f, 180.f, "geode.loader/GE_square03.png")) return false;
        m_onSent = onSent;
        this->setTitle("Gift Fragments");
        auto size = m_mainLayer->getContentSize();

        m_targetInput = TextInput::create(190.f, "Streak ID", "chatFont.fnt");
        m_targetInput->setPosition({ size.width / 2, size.height / 2 + 18.f });
        m_targetInput->setMaxCharCount(32);
        m_targetInput->setFilter("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_");
        m_mainLayer->addChild(m_targetInput);

        const float amountY = size.height / 2 - 22.f;

        m_amountInput = TextInput::create(52.f, "0", "bigFont.fnt");
        m_amountInput->setPosition({ size.width / 2, amountY });
        m_amountInput->setMaxCharCount(4);
        m_amountInput->setFilter("0123456789");
        m_mainLayer->addChild(m_amountInput);

        auto fragIcon = CCSprite::create("fragment.png"_spr);
        if (fragIcon) {
            fragIcon->setScale(40.f / std::max(1.f, fragIcon->getContentSize().width));
            fragIcon->setPosition({ size.width / 2 - 76.f, amountY });
            m_mainLayer->addChild(fragIcon);
        }

        auto controlMenu = CCMenu::create();
        controlMenu->setPosition({ 0.f, 0.f });
        m_mainLayer->addChild(controlMenu);

        auto minusSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
        if (minusSpr) {
            minusSpr->setScale(0.7f);
            auto minusBtn = CCMenuItemSpriteExtra::create(
                minusSpr, this, menu_selector(GiftFragmentsPopup::onMinusPressed));
            minusBtn->setPosition({ size.width / 2 - 36.f, amountY });
            controlMenu->addChild(minusBtn);
        }

        auto plusSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
        if (plusSpr) {
            plusSpr->setFlipX(true);
            plusSpr->setScale(0.7f);
            auto plusBtn = CCMenuItemSpriteExtra::create(
                plusSpr, this, menu_selector(GiftFragmentsPopup::onPlusPressed));
            plusBtn->setPosition({ size.width / 2 + 36.f, amountY });
            controlMenu->addChild(plusBtn);
        }

        auto sendSpr = ButtonSprite::create("Send", 0, false, "goldFont.fnt", "GJ_button_01.png", 0, 0.7f);
        m_sendBtn = CCMenuItemSpriteExtra::create(
            sendSpr, this, menu_selector(GiftFragmentsPopup::onSendPressed));
        auto sendMenu = CCMenu::create();
        sendMenu->addChild(m_sendBtn);
        sendMenu->setPosition({ size.width / 2, 24.f });
        m_mainLayer->addChild(sendMenu);

        auto infoSpr = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
        if (infoSpr) {
            infoSpr->setScale(0.55f);
            auto infoBtn = CCMenuItemSpriteExtra::create(
                infoSpr, this, menu_selector(GiftFragmentsPopup::onInfoPressed));
            auto infoMenu = CCMenu::create();
            infoMenu->addChild(infoBtn);
            infoMenu->setPosition({ size.width - 14.f, size.height - 14.f });
            m_mainLayer->addChild(infoMenu);
        }

        m_spinner = StatusSpinner::create();
        m_spinner->setPosition({ size.width / 2, size.height / 2 });
        m_spinner->hide();
        m_mainLayer->addChild(m_spinner, 100);

        return true;
    }

    int currentAmount() {
        if (!m_amountInput) return 0;
        try { return std::stoi(std::string(m_amountInput->getString())); }
        catch (...) { return 0; }
    }

    void setAmountValue(int v) {
        if (!m_amountInput) return;
        if (v < 0) v = 0;
        if (v > 9999) v = 9999;
        m_amountInput->setString(std::to_string(v).c_str());
    }

    void onPlusPressed(CCObject*) {
        setAmountValue(currentAmount() + 1);
    }

    void onMinusPressed(CCObject*) {
        setAmountValue(currentAmount() - 1);
    }

    void onInfoPressed(CCObject*) {
        FLAlertLayer::create(
            "Gift Fragments",
            "Send your fragments to another player using their <cy>Streak ID</c>. "
            "The recipient receives the amount instantly and can use them to claim "
            "event milestones.\n\n"
            "<cr>Cap:</c> you can gift up to <cy>5 fragments per event</c>. "
            "The limit resets when a new event starts.",
            "OK")->show();
    }

    void setBusy(bool busy) {
        m_isSending = busy;
        if (m_sendBtn) m_sendBtn->setEnabled(!busy);
        if (busy) m_spinner->setLoading("Sending...");
        else m_spinner->hide();
    }

    void onSendPressed(CCObject*) {
        if (m_isSending) return;
        std::string target = m_targetInput ? std::string(m_targetInput->getString()) : "";
        std::string amountStr = m_amountInput ? std::string(m_amountInput->getString()) : "";
        if (target.empty()) {
            FLAlertLayer::create("Error", "Enter a Streak ID.", "OK")->show();
            return;
        }
        int amount = 0;
        try { amount = std::stoi(amountStr); } catch (...) { amount = 0; }
        if (amount <= 0) {
            FLAlertLayer::create("Error", "Enter a valid amount.", "OK")->show();
            return;
        }
        if (amount > g_streakData.fragments) {
            FLAlertLayer::create("Error", "You don't have that many fragments.", "OK")->show();
            return;
        }

        int accountID = GJAccountManager::sharedState()->m_accountID;
        matjson::Value payload = matjson::Value::object();
        payload.set("targetStreakID", target);
        payload.set("amount", amount);

        auto req = web::WebRequest();
        HMACAuth::signRequest(req, accountID, payload);
        req.bodyJSON(payload);

        setBusy(true);
        m_sendTask.spawn(
            req.post("https://streak-servidor.onrender.com/fragments/gift"),
            [this, amount](web::WebResponse res) { this->onSendResponse(res, amount); }
        );
    }

    void onSendResponse(web::WebResponse& res, int amount) {
        setBusy(false);
        if (!res.ok() || !res.json().isOk()) {
            std::string msg = "Could not send fragments.";
            if (res.code() == 404) msg = "Streak ID not found.";
            else if (res.code() == 402) msg = "Not enough fragments.";
            else if (res.code() == 400) msg = "Invalid request.";
            else if (res.code() == 403) msg = "You cannot send to that player.";
            else if (res.code() == 429) {
                msg = "You have already gifted 5 fragments this event. "
                      "The cap resets when a new event starts.";
                if (res.json().isOk()) {
                    auto data = res.json().unwrap();
                    int remaining = data["remaining"].as<int>().unwrapOr(-1);
                    if (remaining >= 0) {
                        msg = fmt::format(
                            "Gift cap reached for this event. You can still gift {} more.",
                            remaining);
                    }
                }
            }
            else if (res.code() == 503) msg = "No active event right now.";
            FLAlertLayer::create("Error", msg.c_str(), "OK")->show();
            return;
        }
        auto data = res.json().unwrap();
        int newFragments = data["fragments"].as<int>().unwrapOr(g_streakData.fragments - amount);
        g_streakData.fragments = newFragments;
        g_streakData.save();
        if (m_onSent) m_onSent(amount);
        Notification::create("Fragments sent!", NotificationIcon::Success)->show();
        this->onClose(nullptr);
    }

public:
    static GiftFragmentsPopup* create(std::function<void(int)> onSent) {
        auto ret = new GiftFragmentsPopup();
        if (ret && ret->init(onSent)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};


 
class KeysGoalPopup : public Popup {
public:
    enum class RewardKind {
        Gems, Stars, Tickets,
        Banner, Badge,
        NameColor, NameEffect, NameAnimation, NameFont,
        Chest
    };

    struct Milestone {
        int frags;
        RewardKind kind;
        int amount;
        std::string itemID;
    };

protected:
    std::string m_eventID;
    std::vector<std::string> m_fragmentBanners;
    std::set<int> m_claimedMilestones;
    std::vector<Milestone> m_milestones;
    RoundedProgressBar* m_progressBar = nullptr;
    CCLabelBMFont* m_progressLabel = nullptr;
    std::function<void()> m_onChanged;
    async::TaskHolder<web::WebResponse> m_claimTask;
    bool m_isClaiming = false;
    int m_pendingMilestone = -1;
    CCNode* m_grid = nullptr;
    std::vector<CCLayerGradient*> m_bannerGradients;
    float m_gradientTime = 0.f;

    bool init(const std::string& eventID,
        const std::vector<std::string>& fragmentBanners,
        const std::vector<Milestone>& milestones,
        const std::set<int>& claimedMilestones,
        std::function<void()> onChanged) {
        if (!Popup::init(336.f, 256.f, "geode.loader/GE_square03.png")) return false;
        m_eventID = eventID;
        m_fragmentBanners = fragmentBanners;
        m_claimedMilestones = claimedMilestones;
        m_onChanged = onChanged;
        m_milestones = milestones;
        if (m_milestones.empty()) buildDefaultMilestones();
        this->setTitle("Fragment Goal");
        auto size = m_mainLayer->getContentSize();

        m_progressBar = RoundedProgressBar::create(190.f, 12.f);
        m_progressBar->setPosition({ size.width / 2 + 10.f, size.height - 42.f });
        m_progressBar->setGradientColors({ 90, 200, 255 }, { 200, 120, 255 });
        m_mainLayer->addChild(m_progressBar);

        m_progressLabel = CCLabelBMFont::create("", "goldFont.fnt");
        m_progressLabel->setScale(0.35f);
        m_progressLabel->setPosition({ size.width / 2 + 10.f, size.height - 42.f });
        m_mainLayer->addChild(m_progressLabel, 5);

        auto fragIcon = CCSprite::create("fragment.png"_spr);
        if (fragIcon) {
            fragIcon->setScale(22.f / std::max(1.f, fragIcon->getContentSize().width));
            fragIcon->setPosition({ size.width / 2 - 105.f, size.height - 42.f });
            m_mainLayer->addChild(fragIcon, 5);
        }

        m_grid = CCNode::create();
        m_grid->setContentSize({ 316.f, 186.f });
        m_grid->setPosition({ (size.width - 316.f) / 2.f, 12.f });
        m_mainLayer->addChild(m_grid);

        rebuild();
        this->scheduleUpdate();
        return true;
    }

    int totalThreshold() const {
        int t = 0;
        for (auto& m : m_milestones) if (m.frags > t) t = m.frags;
        return t > 0 ? t : 9;
    }

    void buildDefaultMilestones() {
        m_milestones.clear();
        m_milestones.push_back({ 1, RewardKind::Gems,    50,  "" });
        m_milestones.push_back({ 2, RewardKind::Stars,   5,   "" });
        m_milestones.push_back({ 3, RewardKind::Tickets, 5,   "" });
        m_milestones.push_back({ 4, RewardKind::Gems,    100, "" });
        m_milestones.push_back({ 5, RewardKind::Stars,   10,  "" });
        m_milestones.push_back({ 6, RewardKind::Chest,   5,   "" });
        m_milestones.push_back({ 7, RewardKind::NameColor, 0, "Galaxy Wave" });

        std::string b8 = m_fragmentBanners.size() > 0 ? m_fragmentBanners[0] : "banner_60";
        std::string b9 = m_fragmentBanners.size() > 1 ? m_fragmentBanners[1] : "banner_59";
        m_milestones.push_back({ 8, RewardKind::Banner, 0, b8 });
        m_milestones.push_back({ 9, RewardKind::Banner, 0, b9 });
    }

    void rebuild() {
        if (!m_grid) return;
        m_grid->removeAllChildren();
        m_bannerGradients.clear();

        int fragments = g_streakData.fragments;
        int maxFrags = totalThreshold();
        float progress = std::min(1.0f, fragments / (float)std::max(1, maxFrags));
        if (m_progressBar) m_progressBar->setProgress(progress);
        if (m_progressLabel) {
            m_progressLabel->setString(
                fmt::format("{}/{}", std::min(maxFrags, fragments), maxFrags).c_str());
        }

        const int cols = 3;
        const int rows = (int)std::max((size_t)1, (m_milestones.size() + cols - 1) / cols);
        const float gridW = m_grid->getContentSize().width;
        const float gridH = m_grid->getContentSize().height;
        const float cellW = std::min(105.f, gridW / cols);
        const float cellH = std::min(62.f, gridH / std::max(rows, 1));
        const float gridWidth = cols * cellW;
        const float startX = (gridW - gridWidth) / 2.f + cellW / 2.f;
        const float topY = gridH - cellH / 2.f - 2.f;

        auto menu = CCMenu::create();
        menu->setPosition({ 0.f, 0.f });
        m_grid->addChild(menu);

        for (size_t i = 0; i < m_milestones.size(); ++i) {
            int row = (int)(i / cols);
            int col = (int)(i % cols);
            float x = startX + col * cellW;
            float y = topY - row * cellH;
            auto cell = makeCell(m_milestones[i], fragments);
            cell->setPosition({ x, y });
            menu->addChild(cell);
        }
    }

    static bool isNameCosmetic(RewardKind k) {
        return k == RewardKind::NameColor
            || k == RewardKind::NameEffect
            || k == RewardKind::NameAnimation
            || k == RewardKind::NameFont;
    }
    static bool isPreviewKind(RewardKind k) {
        return k == RewardKind::Banner
            || k == RewardKind::Badge
            || isNameCosmetic(k);
    }

    CCMenuItemSpriteExtra* makeCell(const Milestone& ms, int fragments) {
        bool claimed = m_claimedMilestones.count(ms.frags) > 0;
        bool reached = fragments >= ms.frags;

        bool owned = false;
        if (ms.kind == RewardKind::Banner) {
            owned = g_streakData.isBannerUnlocked(ms.itemID);
        } else if (ms.kind == RewardKind::Badge) {
            owned = g_streakData.isBadgeUnlocked(ms.itemID);
        } else if (isNameCosmetic(ms.kind)) {
            owned = g_streakData.isNameItemUnlocked(ms.itemID);
        }
        bool claimable = reached && !claimed && !owned;

        const float hw = 95.f;
        const float hh = 56.f;

        auto holder = CCNode::create();
        holder->setContentSize({ hw, hh });

        if (isPreviewKind(ms.kind)) {
            auto stencil = cocos2d::extension::CCScale9Sprite::create("square02_001.png");
            stencil->setContentSize({ hw, hh });
            stencil->setPosition({ hw / 2.f, hh / 2.f });

            auto clipper = CCClippingNode::create(stencil);
            clipper->setAlphaThreshold(0.05f);
            clipper->setContentSize({ hw, hh });

            ccColor4B grad1 = ccc4(120, 60, 200, 220);
            ccColor4B grad2 = ccc4(220, 80, 180, 220);
            if (ms.kind == RewardKind::NameColor) {
                grad1 = ccc4(40, 60, 180, 220);
                grad2 = ccc4(160, 80, 220, 220);
            } else if (ms.kind == RewardKind::NameEffect) {
                grad1 = ccc4(200, 80, 40, 220);
                grad2 = ccc4(255, 160, 60, 220);
            } else if (ms.kind == RewardKind::NameAnimation) {
                grad1 = ccc4(40, 160, 100, 220);
                grad2 = ccc4(100, 220, 160, 220);
            } else if (ms.kind == RewardKind::NameFont) {
                grad1 = ccc4(60, 60, 60, 220);
                grad2 = ccc4(140, 140, 160, 220);
            } else if (ms.kind == RewardKind::Badge) {
                grad1 = ccc4(180, 120, 40, 220);
                grad2 = ccc4(240, 200, 80, 220);
            }
            auto gradient = CCLayerGradient::create(grad1, grad2);
            gradient->setContentSize({ hw, hh });
            gradient->ignoreAnchorPointForPosition(false);
            gradient->setAnchorPoint({ 0.f, 0.f });
            gradient->setPosition({ 0.f, 0.f });
            clipper->addChild(gradient);
            holder->addChild(clipper, -1);
            m_bannerGradients.push_back(gradient);

            auto frame = cocos2d::extension::CCScale9Sprite::create("square02_001.png");
            frame->setContentSize({ hw, hh });
            frame->setOpacity(claimed ? 200 : 90);
            frame->setColor(claimed ? ccColor3B{ 60, 100, 60 } : ccColor3B{ 0, 0, 0 });
            frame->setPosition({ hw / 2.f, hh / 2.f });
            holder->addChild(frame, 0);
        } else {
            auto bg = cocos2d::extension::CCScale9Sprite::create("square02_001.png");
            bg->setContentSize({ hw, hh });
            bg->setOpacity(reached || claimed ? 160 : 90);
            bg->setColor(claimed ? ccColor3B{ 40, 70, 40 } : ccColor3B{ 0, 0, 0 });
            bg->setPosition({ hw / 2.f, hh / 2.f });
            holder->addChild(bg, -1);
        }

        std::string sprite;
        switch (ms.kind) {
            case RewardKind::Gems:    sprite = "gem.png"_spr;        break;
            case RewardKind::Stars:   sprite = "super_star.png"_spr; break;
            case RewardKind::Tickets: sprite = "star_tiket.png"_spr; break;
            case RewardKind::Chest: {
                int r = std::max(1, std::min(5, ms.amount));
                sprite = fmt::format("{}/ChestStar{}.png", Mod::get()->getID(), r);
                break;
            }
            case RewardKind::Banner: {
                auto info = g_streakData.getBannerInfo(ms.itemID);
                if (info) sprite = info->spriteName;
                break;
            }
            case RewardKind::Badge: {
                auto info = g_streakData.getBadgeInfo(ms.itemID);
                if (info) sprite = info->spriteName;
                break;
            }
            default: break;
        }

        if (isNameCosmetic(ms.kind)) {
            auto preview = CCLabelBMFont::create("Name", "bigFont.fnt");
            preview->setScale(0.55f);
            preview->setPosition({ hw / 2.f, hh * 0.62f });
            if      (ms.kind == RewardKind::NameColor)     NameModifiers::applyColor(preview, ms.itemID);
            else if (ms.kind == RewardKind::NameFont)      NameModifiers::applyFont(preview, ms.itemID);
            else if (ms.kind == RewardKind::NameEffect)    NameModifiers::applyEffect(preview, ms.itemID);
            else if (ms.kind == RewardKind::NameAnimation) NameModifiers::applyAnimation(preview, ms.itemID);
            holder->addChild(preview, 5);
        } else {
            auto icon = sprite.empty()
                ? CCSprite::createWithSpriteFrameName("GJ_questionMark_001.png")
                : CCSprite::create(sprite.c_str());
            if (!icon) icon = CCSprite::createWithSpriteFrameName("GJ_questionMark_001.png");

            if (ms.kind == RewardKind::Banner) {
                float maxDim = 44.f;
                float w = icon->getContentSize().width;
                float h = icon->getContentSize().height;
                float scale = (w > h ? maxDim / w : maxDim / h);
                if (scale > 0.75f) scale = 0.75f;
                icon->setScale(scale);
                icon->setPosition({ hw / 2.f, hh * 0.58f });
                holder->addChild(icon, 5);
            } else if (ms.kind == RewardKind::Badge) {
                float maxDim = 40.f;
                float w = icon->getContentSize().width;
                float h = icon->getContentSize().height;
                float scale = (w > h ? maxDim / w : maxDim / h);
                if (scale > 0.9f) scale = 0.9f;
                icon->setScale(scale);
                icon->setPosition({ hw / 2.f, hh * 0.58f });
                holder->addChild(icon, 5);
            } else if (ms.kind == RewardKind::Chest) {
                icon->setScale(0.2f);
                icon->setPosition({ hw * 0.5f, hh * 0.5f });
                holder->addChild(icon, 5);
            } else {
                float maxDim = 30.f;
                float w = icon->getContentSize().width;
                float h = icon->getContentSize().height;
                float scale = (w > h ? maxDim / w : maxDim / h);
                if (scale > 0.7f) scale = 0.7f;
                icon->setScale(scale);
                icon->setPosition({ hw * 0.5f, hh * 0.68f });
                holder->addChild(icon, 5);

                auto amtLabel = CCLabelBMFont::create(
                    fmt::format("x{}", ms.amount).c_str(), "bigFont.fnt");
                amtLabel->setScale(0.42f);
                amtLabel->setAnchorPoint({ 0.5f, 0.5f });
                amtLabel->setPosition({ hw * 0.5f, hh * 0.36f });
                amtLabel->limitLabelWidth(hw * 0.7f, 0.42f, 0.2f);
                holder->addChild(amtLabel, 5);
            }
        }

     
        int shown = std::min(fragments, ms.frags);
        auto cIcon = CCSprite::create("fragment.png"_spr);
        if (cIcon) {
            cIcon->setScale(9.f / std::max(1.f, cIcon->getContentSize().width));
            cIcon->setAnchorPoint({ 0.f, 1.f });
            cIcon->setPosition({ 3.f, hh - 3.f });
            holder->addChild(cIcon, 8);
        }
        auto cLabel = CCLabelBMFont::create(
            fmt::format("{}/{}", shown, ms.frags).c_str(), "goldFont.fnt");
        cLabel->setScale(0.28f);
        cLabel->setAnchorPoint({ 0.f, 1.f });
        cLabel->setPosition({ 14.f, hh - 4.f });
        if (claimed)        cLabel->setColor({ 180, 255, 180 });
        else if (reached)   cLabel->setColor({ 130, 255, 130 });
        else                cLabel->setColor({ 255, 220, 90 });
        holder->addChild(cLabel, 8);

       
        float pct = ms.frags > 0 ? std::min(1.f, (float)fragments / (float)ms.frags) : 0.f;
        auto miniBar = RoundedProgressBar::create(74.f, 5.f);
        miniBar->setProgress(pct);
        if (claimed) {
            miniBar->setGradientColors({ 80, 220, 110 }, { 140, 240, 140 });
        } else if (reached) {
            miniBar->setGradientColors({ 90, 255, 110 }, { 200, 255, 130 });
        } else {
            miniBar->setGradientColors({ 90, 180, 255 }, { 200, 120, 255 });
        }
        miniBar->setPosition({ hw / 2.f, hh * 0.13f });
        holder->addChild(miniBar, 5);

        if (claimable) {
            auto claimLabel = CCLabelBMFont::create("CLAIM", "bigFont.fnt");
            claimLabel->setScale(0.32f);
            claimLabel->setColor({ 90, 255, 110 });
            claimLabel->setAnchorPoint({ 1.f, 1.f });
            claimLabel->setPosition({ hw - 3.f, hh - 2.f });
            holder->addChild(claimLabel, 8);

            auto pulse = CCSequence::create(
                CCScaleTo::create(0.55f, 0.38f),
                CCScaleTo::create(0.55f, 0.32f),
                nullptr);
            claimLabel->runAction(CCRepeatForever::create(pulse));
        }

        if (claimed) {
            auto check = CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
            if (check) {
                check->setScale(0.4f);
                check->setAnchorPoint({ 1.f, 1.f });
                check->setPosition({ hw - 2.f, hh - 2.f });
                holder->addChild(check, 10);
            }
        }

        auto btn = CCMenuItemSpriteExtra::create(holder, this, menu_selector(KeysGoalPopup::onClaimPressed));
        btn->setTag(ms.frags);
        btn->setEnabled(claimable);
        return btn;
    }

    void update(float dt) override {
        m_gradientTime += dt;
        for (size_t i = 0; i < m_bannerGradients.size(); ++i) {
            auto* grad = m_bannerGradients[i];
            if (!grad) continue;
            float t = m_gradientTime * 0.18f + (float)i * 0.5f;
            float h1 = t - std::floor(t);
            float t2 = t + 0.22f;
            float h2 = t2 - std::floor(t2);
            ccColor3B c1 = HSVtoRGB(h1, 0.75f, 0.95f);
            ccColor3B c2 = HSVtoRGB(h2, 0.85f, 0.75f);
            grad->setStartColor(c1);
            grad->setEndColor(c2);
            float ang = m_gradientTime * 0.6f + (float)i * 0.9f;
            grad->setVector(ccp(std::cos(ang), std::sin(ang)));
        }
    }

    void onClaimPressed(CCObject* sender) {
        if (m_isClaiming) return;
        int milestone = static_cast<CCNode*>(sender)->getTag();
        if (milestone < 1 || milestone > 9) return;
        if (m_claimedMilestones.count(milestone)) return;
        if (g_streakData.fragments < milestone) return;

        m_isClaiming = true;
        m_pendingMilestone = milestone;

        int accountID = GJAccountManager::sharedState()->m_accountID;
        matjson::Value payload = matjson::Value::object();
        payload.set("eventID", m_eventID);
        payload.set("milestone", milestone);

        auto req = web::WebRequest();
        HMACAuth::signRequest(req, accountID, payload);
        req.bodyJSON(payload);

        m_claimTask.spawn(
            req.post("https://streak-servidor.onrender.com/fragments/milestone/claim"),
            [this](web::WebResponse res) { this->onClaimResponse(res); }
        );
    }

    void onClaimResponse(web::WebResponse& res) {
        m_isClaiming = false;
        int milestone = m_pendingMilestone;
        m_pendingMilestone = -1;

        if (!res.ok() || !res.json().isOk()) {
            std::string msg = "Could not claim milestone.";
            if (res.code() == 402) msg = "Not enough fragments.";
            else if (res.code() == 409) msg = "Already claimed.";
            else if (res.code() == 404) msg = "Milestone not found.";
            FLAlertLayer::create("Error", msg.c_str(), "OK")->show();
            return;
        }

        auto data = res.json().unwrap();
        if (data.contains("balances")) {
            auto bal = data["balances"];
            g_streakData.gems = bal["gems"].as<int>().unwrapOr(g_streakData.gems);
            g_streakData.starTickets = bal["star_tickets"].as<int>().unwrapOr(g_streakData.starTickets);
            g_streakData.superStars = bal["super_stars"].as<int>().unwrapOr(g_streakData.superStars);
            g_streakData.fragments = bal["fragments"].as<int>().unwrapOr(g_streakData.fragments);
        }

        m_claimedMilestones.insert(milestone);

        const Milestone* matched = nullptr;
        for (auto& m : m_milestones) if (m.frags == milestone) { matched = &m; break; }
        if (matched) {
            auto winSize = CCDirector::sharedDirector()->getWinSize();
            CCPoint spawn = winSize / 2;
            FMODAudioEngine::sharedEngine()->playEffect("buy_obj.mp3"_spr);
            switch (matched->kind) {
                case RewardKind::Gems:
                    RewardNotification::show("gem.png"_spr,
                        g_streakData.gems - matched->amount, matched->amount, spawn);
                    break;
                case RewardKind::Stars:
                    RewardNotification::show("super_star.png"_spr,
                        g_streakData.superStars - matched->amount, matched->amount, spawn);
                    break;
                case RewardKind::Tickets:
                    RewardNotification::show("star_tiket.png"_spr,
                        g_streakData.starTickets - matched->amount, matched->amount, spawn);
                    break;
                case RewardKind::Banner: {
                    if (!g_streakData.isBannerUnlocked(matched->itemID)) {
                        g_streakData.unlockBanner(matched->itemID);
                    }
                    auto info = g_streakData.getBannerInfo(matched->itemID);
                    if (info) {
                        std::string rTxt = g_streakData.getCategoryName(info->rarity);
                        ccColor3B rCol = g_streakData.getCategoryColor(info->rarity);
                        if (info->rarity == StreakData::BadgeCategory::MYTHIC) {
                            auto anim = MythicBannerAnimationLayer::create(
                                *info, [id = matched->itemID, info, rTxt, rCol]() {
                                    BannerNotification::show(id, info->spriteName, info->displayName, rTxt, rCol);
                                });
                            CCDirector::sharedDirector()->getRunningScene()->addChild(anim, 99999);
                        } else {
                            BannerNotification::show(matched->itemID, info->spriteName, info->displayName, rTxt, rCol);
                        }
                    }
                    break;
                }
                case RewardKind::NameColor:
                case RewardKind::NameEffect:
                case RewardKind::NameAnimation:
                case RewardKind::NameFont: {
                    g_streakData.unlockNameItem(matched->itemID);
                    const char* category = "name item";
                    switch (matched->kind) {
                        case RewardKind::NameColor:     category = "name color";     break;
                        case RewardKind::NameEffect:    category = "name effect";    break;
                        case RewardKind::NameAnimation: category = "name animation"; break;
                        case RewardKind::NameFont:      category = "name font";      break;
                        default: break;
                    }
                    Notification::create(
                        fmt::format("Unlocked {}: {}", category, matched->itemID).c_str(),
                        NotificationIcon::Success)->show();
                    break;
                }
                case RewardKind::Badge: {
                    bool wasNew = !g_streakData.isBadgeUnlocked(matched->itemID);
                    if (wasNew) g_streakData.unlockBadge(matched->itemID);
                    auto info = g_streakData.getBadgeInfo(matched->itemID);
                    if (info && info->category == StreakData::BadgeCategory::MYTHIC) {
                        std::string id = matched->itemID;
                        auto anim = MythicAnimationLayer::create(
                            *info, [id]() { BadgeNotification::show(id); });
                        CCDirector::sharedDirector()->getRunningScene()->addChild(anim, 99999);
                    } else {
                        BadgeNotification::show(matched->itemID);
                    }
                    break;
                }
                case RewardKind::Chest: {
                    int rolledStars = 0, rolledTickets = 0, rolledGems = 0, rolledXP = 0;
                    if (data.contains("reward") && data["reward"].contains("rolled")) {
                        auto rolled = data["reward"]["rolled"];
                        rolledStars   = rolled["superStars"].as<int>().unwrapOr(0);
                        rolledTickets = rolled["starTickets"].as<int>().unwrapOr(0);
                        rolledGems    = rolled["gems"].as<int>().unwrapOr(0);
                        rolledXP      = rolled["xp"].as<int>().unwrapOr(0);
                    }
                    if (data.contains("balances")) {
                        auto bal = data["balances"];
                        g_streakData.currentXP = bal["current_xp"].as<int>().unwrapOr(g_streakData.currentXP);
                        g_streakData.currentLevel = bal["current_level"].as<int>().unwrapOr(g_streakData.currentLevel);
                    }
                    int rarity = matched->amount;
                    if (auto popup = StreakChestPopup::createOpen(
                            rolledStars, rolledTickets, rolledGems, rolledXP, rarity, nullptr)) {
                        popup->show();
                    }
                    break;
                }
            }
        }

        g_streakData.save();
        rebuild();
        if (m_onChanged) m_onChanged();
    }

public:
    static KeysGoalPopup* create(const std::string& eventID,
        const std::vector<std::string>& fragmentBanners,
        const std::vector<Milestone>& milestones,
        const std::set<int>& claimedMilestones,
        std::function<void()> onChanged) {
        auto ret = new KeysGoalPopup();
        if (ret && ret->init(eventID, fragmentBanners, milestones, claimedMilestones, onChanged)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

 
class KeysEventPopup : public Popup {
protected:
    CCLayer* m_contentLayer = nullptr;
    StatusSpinner* m_spinner = nullptr;
    KeysEventLayer* m_keysLayer = nullptr;
    CCLabelBMFont* m_gemsLabel = nullptr;
    CCLabelBMFont* m_fragLabel = nullptr;

    std::string m_eventID;
    std::string m_eventName;
    int m_spinCost = 19;
    std::string m_spinCurrency = "gems";
    int m_fragmentThreshold = 9;
    std::vector<std::string> m_fragmentBanners;
    std::vector<KeysGoalPopup::Milestone> m_milestones;
    std::set<int> m_claimedMilestones;

    matjson::Value m_eventRewards;
    std::map<std::string, int> m_rarityWeights;

    async::TaskHolder<web::WebResponse> m_loadTask;
    async::TaskHolder<web::WebResponse> m_claimTask;

    bool m_isClaiming = false;

    std::string m_lastRewardID;
    matjson::Value m_lastRewardData;
    int m_lastFragmentsAfter = 0;

    bool init() override {
        if (!Popup::init(320.f, 248.f, "geode.loader/GE_square03.png")) return false;

        this->setTitle("Keys Event");
        auto winSize = m_mainLayer->getContentSize();
        auto listSize = CCSize{ 288.f, 196.f };

        auto bgStencil = cocos2d::extension::CCScale9Sprite::create("square02_001.png");
        bgStencil->setContentSize(listSize);
        bgStencil->setAnchorPoint({ 0.5f, 0.5f });
        bgStencil->setPosition({ listSize.width / 2, listSize.height / 2 });

        auto limboClipper = CCClippingNode::create(bgStencil);
        limboClipper->setAlphaThreshold(0.05f);
        limboClipper->setContentSize(listSize);
        limboClipper->setAnchorPoint({ 0.5f, 0.5f });
        limboClipper->ignoreAnchorPointForPosition(false);
        limboClipper->setPosition(winSize / 2);
        m_mainLayer->addChild(limboClipper);

        auto limboBg = CCSprite::create("limbo_bg.png"_spr);
        if (limboBg) {
            float bgW = std::max(1.f, limboBg->getContentSize().width);
            float bgH = std::max(1.f, limboBg->getContentSize().height);
            float coverScale = std::max(
                listSize.height * 1.05f / bgH,
                listSize.width * 1.25f / bgW);
            limboBg->setScale(coverScale);
            limboBg->setPosition({ listSize.width / 2, listSize.height / 2 });
            limboClipper->addChild(limboBg);

            float scaledW = bgW * coverScale;
            float slideRange = std::max(24.f, scaledW - listSize.width);
            float leftX  = listSize.width / 2 - slideRange / 2;
            float rightX = listSize.width / 2 + slideRange / 2;
            float centerY = listSize.height / 2;

            auto moveRight = CCEaseInOut::create(
                CCMoveTo::create(5.5f, ccp(rightX, centerY)), 2.0f);
            auto moveLeft = CCEaseInOut::create(
                CCMoveTo::create(5.5f, ccp(leftX, centerY)), 2.0f);
            limboBg->setPosition({ leftX, centerY });
            limboBg->runAction(CCRepeatForever::create(
                CCSequence::create(moveRight, moveLeft, nullptr)));
        }

        auto listBg = cocos2d::extension::CCScale9Sprite::create("square02_001.png");
        listBg->setContentSize(listSize);
        listBg->setColor({ 0, 0, 0 });
        listBg->setOpacity(70);
        listBg->setPosition(winSize / 2);
        m_mainLayer->addChild(listBg);

        m_contentLayer = CCLayer::create();
        m_contentLayer->setContentSize(listSize);
        m_contentLayer->setPosition(listBg->getPosition() - listSize / 2);
        m_mainLayer->addChild(m_contentLayer);

        float leftX = (winSize.width / 2) - (listSize.width / 2);
        float bottomY = (winSize.height / 2) - (listSize.height / 2) - 12.f;

        auto gemIcon = CCSprite::create("gem.png"_spr);
        if (gemIcon) {
            gemIcon->setScale(0.190f);
            gemIcon->setPosition({ leftX + 12.f, bottomY });
            m_mainLayer->addChild(gemIcon);
            m_gemsLabel = CCLabelBMFont::create(
                std::to_string(g_streakData.gems).c_str(), "goldFont.fnt");
            m_gemsLabel->setScale(0.4f);
            m_gemsLabel->setAnchorPoint({ 0.f, 0.5f });
            m_gemsLabel->setPosition({ gemIcon->getPositionX() + 12.f, bottomY });
            m_mainLayer->addChild(m_gemsLabel);
        }

        auto fragIcon = CCSprite::create("fragment.png"_spr);
        if (fragIcon) {
            float scale = 0.28f;
            if (fragIcon->getContentSize().width > 60.f) scale = 20.f / fragIcon->getContentSize().width;
            fragIcon->setScale(scale);
            fragIcon->setPosition({ (winSize.width / 2) + (listSize.width / 2) - 40.f, bottomY });
            m_mainLayer->addChild(fragIcon);
            m_fragLabel = CCLabelBMFont::create(
                fmt::format("{}/{}", g_streakData.fragments, m_fragmentThreshold).c_str(),
                "goldFont.fnt");
            m_fragLabel->setScale(0.4f);
            m_fragLabel->setAnchorPoint({ 0.f, 0.5f });
            m_fragLabel->setPosition({ fragIcon->getPositionX() + 12.f, bottomY });
            m_mainLayer->addChild(m_fragLabel);
        }

        auto sideMenu = CCMenu::create();
        const float sideBtnTarget = 30.f;

        auto fragSpr = CCSprite::create("frag_btn.png"_spr);
        if (!fragSpr) fragSpr = CCSprite::createWithSpriteFrameName("GJ_safeIcon_001.png");
        if (fragSpr) {
            float s = sideBtnTarget / std::max(1.f, fragSpr->getContentSize().width);
            if (s > 1.f) s = 1.f;
            fragSpr->setScale(s);
            auto fragBtn = CCMenuItemSpriteExtra::create(
                fragSpr, this, menu_selector(KeysEventPopup::onShowGoal)
            );
            sideMenu->addChild(fragBtn);
        }

        auto giftSpr = CCSprite::create("frag_gift_btn.png"_spr);
        if (!giftSpr) giftSpr = CCSprite::createWithSpriteFrameName("GJ_chatBtn_001.png");
        if (giftSpr) {
            float s = sideBtnTarget / std::max(1.f, giftSpr->getContentSize().width);
            if (s > 1.f) s = 1.f;
            giftSpr->setScale(s);
            auto giftBtn = CCMenuItemSpriteExtra::create(
                giftSpr, this, menu_selector(KeysEventPopup::onOpenGift)
            );
            sideMenu->addChild(giftBtn);
        }

        sideMenu->alignItemsVerticallyWithPadding(6.f);
        sideMenu->setPosition({
           (winSize.width / 2) - (listSize.width / 2) - 5.f,
            winSize.height / 2
        });
        m_mainLayer->addChild(sideMenu, 50);

        m_spinner = StatusSpinner::create();
        m_spinner->setPosition(winSize / 2);
        m_mainLayer->addChild(m_spinner, 20);

        this->loadEvent();
        return true;
    }

    void onClose(CCObject* sender) override {
        FMODAudioEngine::sharedEngine()->stopAllMusic(false);
        Popup::onClose(sender);
    }

    void onShowGoal(CCObject*) {
        if (m_fragmentBanners.empty()) {
            FLAlertLayer::create("Info", "No active event loaded yet.", "OK")->show();
            return;
        }
        auto popup = KeysGoalPopup::create(m_eventID, m_fragmentBanners,
            m_milestones, m_claimedMilestones, [this]() { this->updateLabels(); });
        if (popup) popup->show();
    }

    void onOpenGift(CCObject*) {
        auto popup = GiftFragmentsPopup::create([this](int) {
            this->updateLabels();
        });
        if (popup) popup->show();
    }

    void updateLabels() {
        if (m_gemsLabel) m_gemsLabel->setString(std::to_string(g_streakData.gems).c_str());
        if (m_fragLabel) {
            m_fragLabel->setString(
                fmt::format("{}/{}", g_streakData.fragments, m_fragmentThreshold).c_str()
            );
        }
    }

    void loadEvent() {
        m_spinner->setLoading("Loading...");
        if (m_contentLayer) m_contentLayer->setVisible(false);

        int accountID = GJAccountManager::sharedState()->m_accountID;
        auto req = web::WebRequest();
        HMACAuth::signGetRequest(req, accountID);

        m_loadTask.spawn(
            req.get(fmt::format(
                "https://streak-servidor.onrender.com/event/current/{}?type=keys",
                accountID)),
            [this](web::WebResponse res) { this->onEventResponse(res); }
        );
    }

    void onEventResponse(web::WebResponse& res) {
        if (res.code() == 404) {
            m_spinner->setError("No active keys event");
            return;
        }
        if (!res.ok() || !res.json().isOk()) {
            m_spinner->setError("Error loading");
            return;
        }
        auto data = res.json().unwrap();
        auto eventData = data["event"];

        if (eventData["isActiveKeys"].as<bool>().unwrapOr(true) == false) {
            m_spinner->setError("No active keys event");
            return;
        }

        m_eventID = eventData["eventID"].as<std::string>().unwrapOr("error_id");
        m_eventName = eventData["eventName"].as<std::string>().unwrapOr("Keys Event");
        m_spinCost = eventData["spinCost"].as<int>().unwrapOr(19);
        m_spinCurrency = eventData["spinCurrency"].as<std::string>().unwrapOr("gems");
        m_fragmentThreshold = eventData["fragmentThreshold"].as<int>().unwrapOr(9);

        m_fragmentBanners.clear();
        if (eventData.contains("fragmentBanners")) {
            auto arr = eventData["fragmentBanners"].as<std::vector<matjson::Value>>();
            if (arr.isOk()) {
                for (auto& v : arr.unwrap()) {
                    auto s = v.as<std::string>().unwrapOr("");
                    if (!s.empty()) m_fragmentBanners.push_back(s);
                }
            }
        } else if (eventData.contains("fragmentBanner")) {
            auto s = eventData["fragmentBanner"].as<std::string>().unwrapOr("");
            if (!s.empty()) m_fragmentBanners.push_back(s);
        }
        if (m_fragmentBanners.empty()) {
            m_fragmentBanners = { "banner_60", "banner_59" };
        }

        m_eventRewards = eventData["rewards"];

        m_rarityWeights.clear();
        if (eventData.contains("rarityWeights")) {
            m_rarityWeights = eventData["rarityWeights"]
                .as<std::map<std::string, int>>().unwrapOr(std::map<std::string, int>());
        }

        m_milestones.clear();
        if (eventData.contains("milestones")) {
            auto arrResult = eventData["milestones"].as<std::vector<matjson::Value>>();
            if (arrResult.isOk()) {
                auto arr = arrResult.unwrap();
                int bannerFallbackIdx = 0;
                for (auto& v : arr) {
                    KeysGoalPopup::Milestone ms;
                    ms.frags = v["frags"].as<int>().unwrapOr(0);
                    std::string kind = v["kind"].as<std::string>().unwrapOr("");
                    if      (kind == "gems")          ms.kind = KeysGoalPopup::RewardKind::Gems;
                    else if (kind == "stars")         ms.kind = KeysGoalPopup::RewardKind::Stars;
                    else if (kind == "tickets")       ms.kind = KeysGoalPopup::RewardKind::Tickets;
                    else if (kind == "banner")        ms.kind = KeysGoalPopup::RewardKind::Banner;
                    else if (kind == "badge")         ms.kind = KeysGoalPopup::RewardKind::Badge;
                    else if (kind == "namecolor")     ms.kind = KeysGoalPopup::RewardKind::NameColor;
                    else if (kind == "nameeffect")    ms.kind = KeysGoalPopup::RewardKind::NameEffect;
                    else if (kind == "nameanimation") ms.kind = KeysGoalPopup::RewardKind::NameAnimation;
                    else if (kind == "namefont")      ms.kind = KeysGoalPopup::RewardKind::NameFont;
                    else if (kind == "chest")         ms.kind = KeysGoalPopup::RewardKind::Chest;
                    else continue;
                    ms.amount = v["amount"].as<int>().unwrapOr(0);
                    if (ms.kind == KeysGoalPopup::RewardKind::Chest) {
                        int r = v["rarity"].as<int>().unwrapOr(ms.amount);
                        if (r < 1 || r > 5) r = 1;
                        ms.amount = r;
                    }
                    ms.itemID = v["itemID"].as<std::string>().unwrapOr("");
                    if (ms.kind == KeysGoalPopup::RewardKind::Banner && ms.itemID.empty()) {
                        if ((int)m_fragmentBanners.size() > bannerFallbackIdx) {
                            ms.itemID = m_fragmentBanners[bannerFallbackIdx];
                            bannerFallbackIdx++;
                        }
                    }
                    if (ms.frags > 0) m_milestones.push_back(ms);
                }
            }
        }

        if (data.contains("fragments")) {
            g_streakData.fragments = data["fragments"].as<int>().unwrapOr(g_streakData.fragments);
        }

        m_claimedMilestones.clear();
        if (data.contains("keysMilestonesClaimed")) {
            auto arr = data["keysMilestonesClaimed"].as<std::vector<matjson::Value>>();
            if (arr.isOk()) {
                for (auto& v : arr.unwrap()) {
                    m_claimedMilestones.insert(v.as<int>().unwrapOr(-1));
                }
            }
        }

        this->setTitle(m_eventName.c_str());

        m_spinner->hide();
        m_contentLayer->setVisible(true);

        this->buildKeysUI();
        this->updateLabels();
    }

    void buildKeysUI() {
        m_contentLayer->removeAllChildren();
        m_keysLayer = KeysEventLayer::create(
            m_spinCost, m_spinCurrency,
            [this]() { this->onSpinPressed(); },
            [this](int slotIdx) { this->onCardPicked(slotIdx); },
            [this]() {
                FMODAudioEngine::sharedEngine()->stopAllMusic(false);
            }
        );
        if (!m_keysLayer) return;
        m_keysLayer->setScale(0.8f);
        m_keysLayer->setPosition({
            (m_contentLayer->getContentSize().width - m_keysLayer->getContentSize().width) / 2,
            (m_contentLayer->getContentSize().height - m_keysLayer->getContentSize().height) / 2
        });
        m_contentLayer->addChild(m_keysLayer);
    }

    void onSpinPressed() {
        if (m_isClaiming) return;
        if (m_spinCost > 0) {
            const char* errMsg = nullptr;
            if (m_spinCurrency == "stars" && g_streakData.superStars < m_spinCost) errMsg = "You don't have enough Super Stars";
            else if (m_spinCurrency == "tickets" && g_streakData.starTickets < m_spinCost) errMsg = "You don't have enough Star Tickets";
            else if (m_spinCurrency == "gems" && g_streakData.gems < m_spinCost) errMsg = "You don't have enough Gems";
            if (errMsg) { FLAlertLayer::create("Error", errMsg, "OK")->show(); return; }
        }

        m_isClaiming = true;
        if (m_keysLayer) m_keysLayer->setStatusText("Connecting...");

        FMODAudioEngine::sharedEngine()->stopAllMusic(false);

        matjson::Value payload = matjson::Value::object();
        payload.set("accountID", GJAccountManager::sharedState()->m_accountID);
        payload.set("eventID", m_eventID);
        payload.set("claimID", "keys_spin");

        auto req = web::WebRequest();
        HMACAuth::signRequest(req, GJAccountManager::sharedState()->m_accountID, payload);
        req.bodyJSON(payload);

        m_claimTask.spawn(
            req.post("https://streak-servidor.onrender.com/event/claim"),
            [this](web::WebResponse res) { this->onClaimResponse(res); }
        );
    }

    void onClaimResponse(web::WebResponse& res) {
        if (!res.ok()) {
            FMODAudioEngine::sharedEngine()->stopAllMusic(false);
            if (res.code() == 402) {
                FLAlertLayer::create("Error", "Insufficient Balance on Server", "OK")->show();
            } else {
                FLAlertLayer::create("Error", "Error processing turn", "OK")->show();
            }
            m_isClaiming = false;
            if (m_keysLayer) m_keysLayer->setStatusText("Press Spin to shuffle the keys!");
            return;
        }

        if (m_spinCost > 0) {
            if (m_spinCurrency == "stars") g_streakData.superStars -= m_spinCost;
            else if (m_spinCurrency == "tickets") g_streakData.starTickets -= m_spinCost;
            else g_streakData.gems -= m_spinCost;
        }
        this->updateLabels();

        auto jsonRes = res.json().unwrapOr(matjson::Value::object());
        m_lastRewardID = jsonRes["rewardID"].as<std::string>().unwrapOr("");
        m_lastRewardData = m_eventRewards.contains(m_lastRewardID)
            ? m_eventRewards[m_lastRewardID]
            : matjson::Value::object();
        m_lastFragmentsAfter = jsonRes["fragments"].as<int>().unwrapOr(g_streakData.fragments);

        std::vector<std::pair<std::string, matjson::Value>> decoys;
        if (m_eventRewards.isObject()) {
            auto map = m_eventRewards.as<std::map<std::string, matjson::Value>>().unwrapOr(
                std::map<std::string, matjson::Value>());
            for (auto& [k, v] : map) {
                if (k != m_lastRewardID) decoys.push_back({ k, v });
            }
        }
        std::shuffle(decoys.begin(), decoys.end(),
            std::mt19937(std::random_device{}()));

        FMODAudioEngine::sharedEngine()->playMusic(
            "limbokeys.mp3"_spr, false, 0.0f, 0);

        if (m_keysLayer) {
            m_keysLayer->startShuffle(decoys, m_lastRewardID, m_lastRewardData);
        }
    }

    void onCardPicked(int slotIdx) {
        if (!m_keysLayer) return;
        m_keysLayer->setWinningReward(slotIdx, m_lastRewardID, m_lastRewardData);
        m_keysLayer->setStatusText("Revealing...");
        m_keysLayer->revealPicked(slotIdx, [this]() {
            this->finalizeReward();
        });
    }

    void finalizeReward() {
        if (!m_lastRewardData.isObject()) {
            m_isClaiming = false;
            if (m_keysLayer) m_keysLayer->setStatusText("Press Spin to shuffle the keys!");
            return;
        }

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        CCPoint spawnPos = winSize / 2;
        if (m_keysLayer) {
            spawnPos = m_keysLayer->convertToWorldSpace({
                m_keysLayer->getContentSize().width / 2, 60.f
            });
        }

        auto& r = m_lastRewardData;

        if (r.contains("super_stars")) {
            int amt = r["super_stars"].as<int>().unwrapOr(0);
            if (amt > 0) {
                int start = g_streakData.superStars;
                g_streakData.superStars += amt;
                RewardNotification::show("super_star.png"_spr, start, amt, spawnPos);
            }
        }
        if (r.contains("star_tickets")) {
            int amt = r["star_tickets"].as<int>().unwrapOr(0);
            if (amt > 0) {
                int start = g_streakData.starTickets;
                g_streakData.starTickets += amt;
                RewardNotification::show("star_tiket.png"_spr, start, amt, spawnPos);
            }
        }
        if (r.contains("gems")) {
            int amt = r["gems"].as<int>().unwrapOr(0);
            if (amt > 0) {
                int start = g_streakData.gems;
                g_streakData.gems += amt;
                RewardNotification::show("gem.png"_spr, start, amt, spawnPos);
            }
        }
        if (r.contains("badge")) {
            std::string id = r["badge"].as<std::string>().unwrapOr("");
            if (!id.empty()) {
                if (!g_streakData.isBadgeUnlocked(id)) {
                    g_streakData.unlockBadge(id);
                    BadgeNotification::show(id);
                } else {
                    Notification::create("You already have this item", NotificationIcon::Info)->show();
                }
            }
        }
        if (r.contains("banner")) {
            std::string id = r["banner"].as<std::string>().unwrapOr("");
            if (!id.empty()) {
                if (!g_streakData.isBannerUnlocked(id)) {
                    g_streakData.unlockBanner(id);
                    BadgeNotification::show(id);
                } else {
                    Notification::create("You already have this item", NotificationIcon::Info)->show();
                }
            }
        }
        if (r.contains("fragment")) {
            int gained = r["fragment"].as<int>().unwrapOr(1);
            int start = g_streakData.fragments;
            g_streakData.fragments = m_lastFragmentsAfter;
            RewardNotification::show("fragment.png"_spr, start, gained, spawnPos);
        }

        g_streakData.save();
        this->updateLabels();

        m_isClaiming = false;
        if (m_keysLayer) m_keysLayer->setStatusText("Press Spin to shuffle again!");
    }

public:
    static KeysEventPopup* create() {
        auto ret = new KeysEventPopup();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};