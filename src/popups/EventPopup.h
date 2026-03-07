#pragma once
#include "StreakCommon.h"
#include "../StreakData.h"
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/ScrollLayer.hpp> 
#include <Geode/utils/web.hpp>
#include <Geode/utils/async.hpp>  
#include "../BadgeNotification.h"
#include "../RewardNotification.h"
#include "SharedVisuals.h" 
#include "../StatusSpinner.h"
#include <Geode/ui/Notification.hpp>

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
        auto req = web::WebRequest();

       
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
            if (m_spinCurrency != "stars" && g_streakData.starTickets < m_spinCost) {
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
        m_tempBadgeID = "";
        m_tempBannerID = "";

        if (rewardData.contains("super_stars")) {
            m_tempStars = rewardData["super_stars"].as<int>().unwrapOr(0);
        }
        if (rewardData.contains("star_tickets")) {
            m_tempTickets = rewardData["star_tickets"].as<int>().unwrapOr(0);
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

            g_streakData.superStars += m_tempStars;
            g_streakData.starTickets += m_tempTickets;

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

            auto showConsumables = [this, starsStart, ticketsStart]() {
                if (m_tempStars > 0) {
                    RewardNotification::show("super_star.png"_spr, starsStart, m_tempStars);
                }
                if (m_tempTickets > 0) {
                    RewardNotification::show("star_tiket.png"_spr, ticketsStart, m_tempTickets);
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