#pragma once
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/ScrollLayer.hpp>
#include <Geode/ui/Notification.hpp>
#include "../StreakData.h"
#include "../BannerNotification.h"
#include "../BadgeNotification.h"
#include "../RewardNotification.h"
#include "../FirebaseManager.h"
#include "../StatusSpinner.h"
#include "../utils/RoundedProgressBar.h"
#include "StreakChestPopup.h"
#include "PremiumUnlockAnim.h"
#include "GoldTicketMissionsPopup.h"
#include "PassPurchasePopup.h"
#include "PurchaseConfirmPopup.h"
#include "BuyGoldTicketsPopup.h"
#include "GiftPassPopup.h"
#include "../NameModifiers.h"

using namespace geode::prelude;

class StProgressPopup : public Popup {
protected:
    static constexpr int MONTHLY_GOAL_SP = 2500;
    static constexpr int PAID_TIER_STEP = 50;
    static constexpr int FREE_TIER_STEP = 100;
    static constexpr int TOTAL_PAID_TIERS = MONTHLY_GOAL_SP / PAID_TIER_STEP;
    static constexpr int MILESTONE_SP = 200;

    enum class PassRewardType { None, Tickets, Stars, Gems, Shields, DiscountTicket, Chest, Badge, Banner, NameItem };

    struct PassReward {
        PassRewardType type = PassRewardType::None;
        int amount = 0;
        std::string itemID;
    };

    struct AnimatedGradient {
        CCLayerGradient* layer;
        float hue;
    };

    struct GradientBg {
        CCNode* node;
        CCLayerGradient* gradient;
    };

    ScrollLayer* m_scrollLayer = nullptr;
    CCLabelBMFont* m_countdownLabel = nullptr;
    CCLabelBMFont* m_goldLabel = nullptr;
    RoundedProgressBar* m_progressBar = nullptr;
    CCNode* m_premiumStatusNode = nullptr;
    std::vector<AnimatedGradient> m_animatedGradients;
    CCLayerGradient* m_bgGradient = nullptr;
    float m_bgHue = 0.f;
    bool m_passEnded = false;
    bool m_initialScrollDone = false;
    StatusSpinner* m_spinner = nullptr;
    bool m_serverLoaded = false;

    CCMenuItemSpriteExtra* m_completeRewardBtn = nullptr;
    CCSprite* m_completeRewardCheck = nullptr;
    CCLabelBMFont* m_themeLabel = nullptr;
    static constexpr const char* COMPLETE_REWARD_SONG = "song_3";

    RoundedProgressBar* m_goldBuyBar = nullptr;
    CCLabelBMFont* m_goldBuyGoalLabel = nullptr;

    static PassRewardType typeFromString(const std::string& s) {
        if (s == "tickets")   return PassRewardType::Tickets;
        if (s == "stars")     return PassRewardType::Stars;
        if (s == "gems")      return PassRewardType::Gems;
        if (s == "shields")   return PassRewardType::Shields;
        if (s == "discount_ticket") return PassRewardType::DiscountTicket;
        if (s == "chest")     return PassRewardType::Chest;
        if (s == "badge")     return PassRewardType::Badge;
        if (s == "banner")    return PassRewardType::Banner;
        if (s == "name_item") return PassRewardType::NameItem;
        return PassRewardType::None;
    }

    static int discountPercentForReward(const PassReward& reward) {
        for (int percent : { 10, 25, 50, 80, 99 }) {
            if (reward.itemID == std::to_string(percent)) return percent;
        }
        return 0;
    }

    static PassReward getFreeReward(int tier) {
        PassReward r;
        if (tier <= 0 || tier > (int)g_streakData.freePassRewards.size()) return r;
        const auto& d = g_streakData.freePassRewards[tier - 1];
        r.type = typeFromString(d.type);
        r.amount = d.amount;
        r.itemID = d.itemID;
        return r;
    }

    static PassReward getPaidReward(int tier) {
        PassReward r;
        if (tier <= 0 || tier > (int)g_streakData.paidPassRewards.size()) return r;
        const auto& d = g_streakData.paidPassRewards[tier - 1];
        r.type = typeFromString(d.type);
        r.amount = d.amount;
        r.itemID = d.itemID;
        return r;
    }

    static const char* spriteForReward(PassRewardType t) {
        switch (t) {
            case PassRewardType::Tickets: return "star_tiket.png"_spr;
            case PassRewardType::Stars:   return "super_star.png"_spr;
            case PassRewardType::Gems:    return "gem.png"_spr;
            case PassRewardType::Shields: return "heart.png"_spr;
            default: return nullptr;
        }
    }

    static float scaleForReward(PassRewardType t) {
        switch (t) {
            case PassRewardType::Tickets: return 0.22f;
            case PassRewardType::Stars:   return 0.22f;
            case PassRewardType::Gems:    return 0.32f;
            case PassRewardType::Shields: return 0.30f;
            default: return 1.f;
        }
    }

    void grantReward(const PassReward& reward, CCPoint spawnPos) {
        if (reward.type == PassRewardType::Badge) {
            if (reward.itemID.empty()) return;
            g_streakData.unlockBadge(reward.itemID);
            BadgeNotification::show(reward.itemID);
            return;
        }
        if (reward.type == PassRewardType::Banner) {
            if (reward.itemID.empty()) return;
            g_streakData.unlockBanner(reward.itemID);
            auto info = g_streakData.getBannerInfo(reward.itemID);
            if (info) {
                BannerNotification::show(
                    reward.itemID, info->spriteName, info->displayName,
                    g_streakData.getCategoryName(info->rarity),
                    g_streakData.getCategoryColor(info->rarity)
                );
            }
            return;
        }
        if (reward.type == PassRewardType::NameItem) {
            if (reward.itemID.empty()) return;
            g_streakData.unlockNameItem(reward.itemID);
            FLAlertLayer::create(
                "Name Item",
                fmt::format("Unlocked <cy>{}</c>! Equip it from your name customization.", reward.itemID).c_str(),
                "OK"
            )->show();
            return;
        }
        if (reward.type == PassRewardType::Chest) {
            int rarity = std::clamp(reward.amount, 1, 6);
            int stars = 0, tickets = 0, gems = 0, xp = 0;
            StreakChestPopup::rollRewardsForRarity(rarity, stars, tickets, gems, xp);
            auto refresh = [this]() { this->refreshTrack(); };
            if (auto popup = StreakChestPopup::create(stars, tickets, gems, xp, rarity, refresh)) {
                popup->show();
            }
            return;
        }

        const char* spr = spriteForReward(reward.type);
        if (!spr || reward.amount <= 0) return;
        switch (reward.type) {
            case PassRewardType::Tickets: {
                int start = g_streakData.starTickets;
                g_streakData.starTickets += reward.amount;
                RewardNotification::show(spr, start, reward.amount, spawnPos);
                break;
            }
            case PassRewardType::Stars: {
                int start = g_streakData.superStars;
                g_streakData.superStars += reward.amount;
                RewardNotification::show(spr, start, reward.amount, spawnPos);
                break;
            }
            case PassRewardType::Gems: {
                int start = g_streakData.gems;
                g_streakData.gems += reward.amount;
                RewardNotification::show(spr, start, reward.amount, spawnPos);
                break;
            }
            case PassRewardType::Shields: {
                int start = g_streakData.streakShields;
                int accepted = std::min(reward.amount, std::max(0, STREAK_MAX_SHIELDS - start));
                int converted = std::max(0, reward.amount - accepted);
                g_streakData.streakShields += accepted;
                g_streakData.gems += converted * STREAK_SHIELD_OVERFLOW_GEMS;
                if (accepted > 0) RewardNotification::show(spr, start, accepted, spawnPos);
                showShieldConversionAlert(converted, converted * STREAK_SHIELD_OVERFLOW_GEMS);
                break;
            }
            default: break;
        }
    }

    CCNode* createRewardIcon(const PassReward& reward) {
        auto node = CCNode::create();
        node->setContentSize({ 44.f, 44.f });
        node->setAnchorPoint({ 0.5f, 0.5f });

        if (reward.type == PassRewardType::DiscountTicket) {
            int percent = discountPercentForReward(reward);
            auto spr = percent > 0
                ? CCSprite::create(fmt::format("discount_ticket_{}.png"_spr, percent).c_str())
                : nullptr;
            if (!spr) spr = CCSprite::createWithSpriteFrameName("GJ_unknownBtn_001.png");
            if (spr) {
                float maxSize = 38.f;
                float scale = maxSize / std::max({ spr->getContentSize().width, spr->getContentSize().height, 1.f });
                spr->setScale(scale);
                spr->setPosition({ 22.f, 27.f });
                node->addChild(spr);
            }
            auto lbl = CCLabelBMFont::create(
                fmt::format("x{}", std::max(1, reward.amount)).c_str(),
                "bigFont.fnt"
            );
            lbl->setScale(0.28f);
            lbl->setAnchorPoint({ 0.5f, 1.f });
            lbl->setPosition({ 22.f, 9.f });
            node->addChild(lbl);
            return node;
        }

        if (reward.type == PassRewardType::Badge) {
            auto info = g_streakData.getBadgeInfo(reward.itemID);
            std::string spriteName = info ? info->spriteName : std::string("");
            CCSprite* spr = nullptr;
            if (!spriteName.empty()) spr = CCSprite::create(spriteName.c_str());
            if (!spr) spr = CCSprite::createWithSpriteFrameName("GJ_unknownBtn_001.png");
            if (spr) {
                float maxSize = 34.f;
                float scale = maxSize / std::max({ spr->getContentSize().width, spr->getContentSize().height, 1.f });
                spr->setScale(scale);
                spr->setPosition({ 22.f, 28.f });
                node->addChild(spr);
            }
            auto lbl = CCLabelBMFont::create("Badge", "bigFont.fnt");
            lbl->setScale(0.28f);
            lbl->setAnchorPoint({ 0.5f, 1.0f });
            lbl->setPosition({ 22.f, 9.f });
            node->addChild(lbl);
            return node;
        }

        if (reward.type == PassRewardType::Banner) {
            auto info = g_streakData.getBannerInfo(reward.itemID);
            std::string spriteName = info ? info->spriteName : std::string("");
            CCSprite* spr = nullptr;
            if (!spriteName.empty()) spr = CCSprite::create(spriteName.c_str());
            if (!spr) spr = CCSprite::createWithSpriteFrameName("GJ_unknownBtn_001.png");
            if (spr) {
                float maxW = 40.f;
                float scale = maxW / std::max(spr->getContentSize().width, 1.f);
                spr->setScale(scale);
                spr->setPosition({ 22.f, 26.f });
                node->addChild(spr);
            }
            auto lbl = CCLabelBMFont::create("Banner", "bigFont.fnt");
            lbl->setScale(0.28f);
            lbl->setAnchorPoint({ 0.5f, 1.0f });
            lbl->setPosition({ 22.f, 9.f });
            node->addChild(lbl);
            return node;
        }

        if (reward.type == PassRewardType::NameItem) {
            auto preview = CCLabelBMFont::create("Name", "bigFont.fnt");
            if (preview) {
                NameModifiers::applyColor(preview, reward.itemID);
                preview->setScale(0.35f);
                preview->setPosition({ 22.f, 26.f });
                node->addChild(preview);
            }
            auto lbl = CCLabelBMFont::create("Color", "bigFont.fnt");
            lbl->setScale(0.28f);
            lbl->setAnchorPoint({ 0.5f, 1.0f });
            lbl->setPosition({ 22.f, 9.f });
            node->addChild(lbl);
            return node;
        }

        if (reward.type == PassRewardType::Chest) {
            std::string path = fmt::format("{}/ChestStar{}.png", Mod::get()->getID(), std::clamp(reward.amount, 1, 6));
            auto chest = CCSprite::create(path.c_str());
            if (!chest) chest = CCSprite::createWithSpriteFrameName("chest_02_02_001.png");
            if (chest) {
                chest->setScale(0.20f);
                chest->setPosition({ 22.f, 26.f });
                node->addChild(chest);
            }
            auto lbl = CCLabelBMFont::create(
                fmt::format("{}*", reward.amount).c_str(),
                "goldFont.fnt"
            );
            lbl->setScale(0.32f);
            lbl->setAnchorPoint({ 0.5f, 1.0f });
            lbl->setPosition({ 22.f, 10.f });
            node->addChild(lbl);
            return node;
        }

        const char* sprName = spriteForReward(reward.type);
        if (!sprName || reward.amount <= 0) {
            auto qm = CCLabelBMFont::create("?", "bigFont.fnt");
            qm->setScale(0.5f);
            qm->setPosition({ 22.f, 22.f });
            qm->setOpacity(120);
            node->addChild(qm);
            return node;
        }

        auto spr = CCSprite::create(sprName);
        if (!spr) spr = CCSprite::createWithSpriteFrameName("GJ_unknownBtn_001.png");
        if (spr) {
            spr->setScale(scaleForReward(reward.type));
            spr->setPosition({ 22.f, 28.f });
            node->addChild(spr);
        }
        auto lbl = CCLabelBMFont::create(
            fmt::format("x{}", reward.amount).c_str(),
            "bigFont.fnt"
        );
        lbl->setScale(0.3f);
        lbl->setAnchorPoint({ 0.5f, 1.0f });
        lbl->setPosition({ 22.f, 10.f });
        node->addChild(lbl);
        return node;
    }

    GradientBg makeRoundedGradient(float w, float h, ccColor3B a, ccColor3B b, GLubyte opacity) {
        auto wrap = CCNode::create();
        wrap->setContentSize({ w, h });
        wrap->ignoreAnchorPointForPosition(false);
        wrap->setAnchorPoint({ 0.5f, 0.5f });

        auto stencil = cocos2d::extension::CCScale9Sprite::create("square02_small.png");
        stencil->setContentSize({ w, h });
        stencil->setAnchorPoint({ 0.5f, 0.5f });
        stencil->setPosition({ w / 2.f, h / 2.f });

        auto clipper = CCClippingNode::create(stencil);
        clipper->setAlphaThreshold(0.05f);

        auto gradient = CCLayerGradient::create(
            ccc4(a.r, a.g, a.b, opacity),
            ccc4(b.r, b.g, b.b, opacity),
            ccp(1, -1)
        );
        gradient->setContentSize({ w, h });
        clipper->addChild(gradient);

        wrap->addChild(clipper);
        return { wrap, gradient };
    }

    CCNode* makeRoundedSolidBG(float w, float h, ccColor3B color, GLubyte opacity) {
        auto bg = cocos2d::extension::CCScale9Sprite::create("square02b_001.png");
        bg->setContentSize({ w, h });
        bg->setColor(color);
        bg->setOpacity(opacity);
        return bg;
    }

    void onClaimFree(CCObject* sender) {
        if (m_passEnded) return;
        auto btn = static_cast<CCNode*>(sender);
        int tier = btn->getTag();
        CCPoint spawnPos = btn->convertToWorldSpaceAR(CCPointZero);
        requestClaimTier("free", tier, spawnPos);
    }

    void onClaimPaid(CCObject* sender) {
        if (m_passEnded) return;
        auto btn = static_cast<CCNode*>(sender);
        int tier = btn->getTag();
        CCPoint spawnPos = btn->convertToWorldSpaceAR(CCPointZero);
        if (!g_streakData.isPremiumPassActive()) {
            FLAlertLayer::create("Premium Pass", "Unlock the Premium Pass to claim this reward.", "OK")->show();
            return;
        }
        requestClaimTier("paid", tier, spawnPos);
    }

    // Plays the currency gain animation for a reward whose balance was already
    // updated from the server response.
    void showPassRewardAnim(const PassReward& reward, CCPoint spawnPos) {
        if (reward.type == PassRewardType::DiscountTicket) {
            int percent = discountPercentForReward(reward);
            if (percent <= 0 || reward.amount <= 0) return;
            std::string spr = fmt::format("discount_ticket_{}.png"_spr, percent);
            int current = g_streakData.getDiscountTicketCount(percent);
            RewardNotification::show(spr.c_str(), std::max(0, current - reward.amount), reward.amount, spawnPos);
            return;
        }

        const char* spr = spriteForReward(reward.type);
        if (!spr || reward.amount <= 0) return;
        int current = 0;
        switch (reward.type) {
            case PassRewardType::Tickets: current = g_streakData.starTickets; break;
            case PassRewardType::Stars:   current = g_streakData.superStars; break;
            case PassRewardType::Gems:    current = g_streakData.gems; break;
            case PassRewardType::Shields: current = g_streakData.streakShields; break;
            default: return;
        }
        int start = std::max(0, current - reward.amount);
        RewardNotification::show(spr, start, reward.amount, spawnPos);
    }

    // Pass rewards are granted and recorded by the server so they actually persist.
    // The claimed-tier lists are server-authoritative; we only mirror them locally
    // for the UI. Chest tiers are recorded by the server too, then their contents are
    // delivered through the interactive chest flow (/chest/claim).
    void requestClaimTier(const std::string& track, int tier, CCPoint spawnPos) {
        const bool isFree = (track == "free");
        if (isFree) {
            if (tier <= 0 || tier > TOTAL_PAID_TIERS / 2) return;
            if (g_streakData.isFreePassTierClaimed(tier)) return;
            if (g_streakData.goldTickets < tier * FREE_TIER_STEP) return;
        } else {
            if (tier <= 0 || tier > TOTAL_PAID_TIERS) return;
            if (g_streakData.isPaidPassTierClaimed(tier)) return;
            if (g_streakData.goldTickets < tier * PAID_TIER_STEP) return;
        }

        PassReward reward = isFree ? getFreeReward(tier) : getPaidReward(tier);

        matjson::Value payload = matjson::Value::object();
        payload.set("track", track);
        payload.set("tier", tier);

        claimOnServerEx("/streak-pass/tier/claim", payload,
            [this, isFree, tier, reward, spawnPos, keepAlive = Ref<CCNode>(this)](
                bool ok, int, const matjson::Value& data
            ) {
                if (!ok) {
                    FLAlertLayer::create("Pass",
                        "Could not claim the reward. Check your connection and try again.", "OK")->show();
                    return;
                }
                if (isFree) g_streakData.setFreePassTierClaimed(tier);
                else        g_streakData.setPaidPassTierClaimed(tier);

                bool isCurrency = reward.type == PassRewardType::Tickets
                               || reward.type == PassRewardType::Stars
                               || reward.type == PassRewardType::Gems
                               || reward.type == PassRewardType::Shields
                               || reward.type == PassRewardType::DiscountTicket;
                if (isCurrency) {
                    // Balance already updated from the server response.
                    PassReward animatedReward = reward;
                    if (reward.type == PassRewardType::Shields &&
                        data.contains("shield_conversion") && !data["shield_conversion"].isNull()) {
                        int converted = data["shield_conversion"]["converted_shields"].as<int>().unwrapOr(0);
                        animatedReward.amount = std::max(0, reward.amount - converted);
                    }
                    showPassRewardAnim(animatedReward, spawnPos);
                } else {
                    // Chest / Banner / NameItem: grantReward opens the chest popup or
                    // unlocks the item locally (server already recorded the unlock).
                    grantReward(reward, spawnPos);
                }
                FMODAudioEngine::sharedEngine()->playEffect("dummyDestroy.ogg");
                refreshTrack();
            });
    }

    void onMissions(CCObject*) {
        if (auto popup = GoldTicketMissionsPopup::create([this]() {
            this->refreshHeader();
            this->refreshTrack();
        })) {
            popup->show();
        }
    }

    void buildCompleteReward(CCSize winSize) {
        float xIcon = winSize.width - 36.f;
        float yBand = winSize.height - 70.f;

        CCSprite* songIcon = nullptr;
        if (auto info = g_streakData.getSongInfo(COMPLETE_REWARD_SONG)) {
            songIcon = CCSprite::create(info->iconName.c_str());
        }
        if (!songIcon) songIcon = CCSprite::createWithSpriteFrameName("GJ_musicJob_001.png");
        if (songIcon) {
            float target = 38.f;
            songIcon->setScale(target / std::max(songIcon->getContentSize().width, songIcon->getContentSize().height));
        }
        m_completeRewardBtn = CCMenuItemSpriteExtra::create(
            songIcon, this, menu_selector(StProgressPopup::onClaimCompleteReward));
        m_completeRewardBtn->setPosition({ xIcon, yBand });

        float panelW = 96.f, panelH = 18.f;
        CCPoint panelCenter = { xIcon - 20.f - panelW / 2.f, yBand };

        auto infoSpr = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
        infoSpr->setScale(0.45f);
        auto infoBtn = CCMenuItemSpriteExtra::create(
            infoSpr, this, menu_selector(StProgressPopup::onCompleteRewardInfo));
        infoBtn->setPosition({ panelCenter.x - panelW / 2.f - 12.f, yBand });

        auto menu = CCMenu::create();
        menu->addChild(m_completeRewardBtn);
        menu->addChild(infoBtn);
        menu->setPosition({ 0, 0 });
        m_mainLayer->addChild(menu, 6);

        m_completeRewardCheck = CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
        if (m_completeRewardCheck) {
            m_completeRewardCheck->setScale(0.55f);
            m_completeRewardCheck->setPosition({ xIcon + 12.f, yBand - 12.f });
            m_mainLayer->addChild(m_completeRewardCheck, 7);
        }

        auto panelBg = cocos2d::extension::CCScale9Sprite::create("square02_001.png");
        panelBg->setContentSize({ panelW, panelH });
        panelBg->setColor({ 0, 0, 0 });
        panelBg->setOpacity(140);
        panelBg->setPosition(panelCenter);
        m_mainLayer->addChild(panelBg, 5);

        auto stencil = cocos2d::extension::CCScale9Sprite::create("square02_001.png");
        stencil->setContentSize({ panelW, panelH });
        stencil->setPosition(panelCenter);
        auto clipper = CCClippingNode::create(stencil);
        clipper->setAlphaThreshold(0.05f);
        clipper->setPosition({ 0, 0 });
        m_mainLayer->addChild(clipper, 6);

        std::string themeName = "Streak Theme";
        if (auto info = g_streakData.getSongInfo(COMPLETE_REWARD_SONG)) themeName = info->displayName;
        m_themeLabel = CCLabelBMFont::create(themeName.c_str(), "bigFont.fnt");
        m_themeLabel->setScale(0.4f);
        m_themeLabel->setAnchorPoint({ 0.f, 0.5f });
        clipper->addChild(m_themeLabel);
        NameModifiers::applyColor(m_themeLabel, "Rainbow Wave");

        float labelW = m_themeLabel->getContentSize().width * m_themeLabel->getScale();
        float startX = panelCenter.x - panelW / 2.f - labelW;
        float endX = panelCenter.x + panelW / 2.f;
        float travel = endX - startX;
        m_themeLabel->setPosition({ startX, panelCenter.y });
        float dur = std::max(2.f, travel / 28.f);
        m_themeLabel->runAction(CCRepeatForever::create(CCSequence::create(
            CCMoveBy::create(dur, { travel, 0.f }),
            CCMoveBy::create(0.f, { -travel, 0.f }),
            nullptr
        )));

        auto upgradeSpr = CCSprite::create("upgrade_btn.png"_spr);
        if (!upgradeSpr) upgradeSpr = ButtonSprite::create("+");
        {
            float maxH = 30.f;
            upgradeSpr->setScale(maxH / std::max(upgradeSpr->getContentSize().height, 1.f));
        }
        auto upgradeBtn = CCMenuItemSpriteExtra::create(
            upgradeSpr, this, menu_selector(StProgressPopup::onUpgrade));
        upgradeBtn->setPosition({ 40.f, yBand });
        auto upgradeMenu = CCMenu::createWithItem(upgradeBtn);
        upgradeMenu->setPosition({ 0, 0 });
        m_mainLayer->addChild(upgradeMenu, 6);

        m_goldBuyBar = RoundedProgressBar::create(80.f, 10.f);
        m_goldBuyBar->setPosition({ 110.f, yBand - 3.f });
        m_goldBuyBar->setGradientColors({ 250, 225, 60 }, { 255, 165, 0 });
        m_mainLayer->addChild(m_goldBuyBar, 6);

        m_goldBuyGoalLabel = CCLabelBMFont::create("", "bigFont.fnt");
        m_goldBuyGoalLabel->setScale(0.3f);
        m_goldBuyGoalLabel->setPosition({ 110.f, yBand + 9.f });
        m_mainLayer->addChild(m_goldBuyGoalLabel, 6);

        refreshGoldBuyBar();
        refreshCompleteReward();
    }

    void refreshGoldBuyBar() {
        int g = g_streakData.goldTickets;
        int goal = std::min(MONTHLY_GOAL_SP, ((g / FREE_TIER_STEP) + 1) * FREE_TIER_STEP);
        int segStart = std::max(0, goal - FREE_TIER_STEP);
        if (g >= MONTHLY_GOAL_SP) { goal = MONTHLY_GOAL_SP; segStart = MONTHLY_GOAL_SP - FREE_TIER_STEP; }

        if (m_goldBuyBar) {
            float denom = (float)std::max(1, goal - segStart);
            m_goldBuyBar->setProgress((g >= MONTHLY_GOAL_SP) ? 1.f : (float)(g - segStart) / denom);
        }
        if (m_goldBuyGoalLabel) {
            m_goldBuyGoalLabel->setString(
                (g >= MONTHLY_GOAL_SP) ? "MAX" : fmt::format("Next: {}", goal).c_str());
        }
    }

    void onUpgrade(CCObject*) {
        if (m_passEnded) {
            FLAlertLayer::create("Pass", "This pass has ended.", "OK")->show();
            return;
        }
        auto popup = BuyGoldTicketsPopup::create();
        popup->onPurchased = [this]() {
            this->refreshHeader();
            this->refreshTrack();
        };
        popup->show();
    }

    void onGiftPass(CCObject*) {
        auto popup = GiftPassPopup::create();
        popup->onGifted = [this]() {
            this->refreshHeader();
        };
        popup->show();
    }

    void refreshCompleteReward() {
        if (!m_completeRewardBtn) return;
        bool claimed = g_streakData.passCompleteRewardClaimed;
        bool complete = g_streakData.goldTickets >= g_streakData.getPassCompleteGoal();

        if (auto icon = static_cast<CCSprite*>(m_completeRewardBtn->getNormalImage())) {
            if (claimed) icon->setColor({ 150, 150, 150 });
            else if (complete) icon->setColor({ 255, 255, 255 });
            else icon->setColor({ 110, 110, 110 });
        }
        if (m_completeRewardCheck) m_completeRewardCheck->setVisible(claimed);
        m_completeRewardBtn->setEnabled(complete && !claimed);
    }

    void onCompleteRewardInfo(CCObject*) {
        std::string songName = "season";
        if (auto info = g_streakData.getSongInfo(COMPLETE_REWARD_SONG)) songName = info->displayName;
        FLAlertLayer::create(
            "Pass Completion Reward",
            fmt::format(
                "Reach the pass goal on either the <cg>Free</c> or <cy>VIP</c> track to unlock the <cl>{}</c> song.\n"
                "It's a bonus, separate from the regular tier rewards.",
                songName
            ).c_str(),
            "OK"
        )->show();
    }

    // Marca el premio como reclamado en local y desbloquea la cancion. Se usa
    // tanto en el exito como al recibir un 409 (el server ya lo tenia dado).
    void applyCompleteRewardUnlocked(bool announce) {
        g_streakData.passCompleteRewardClaimed = true;
        g_streakData.unlockSong(COMPLETE_REWARD_SONG);

        if (announce) {
            if (auto info = g_streakData.getSongInfo(COMPLETE_REWARD_SONG)) {
                BannerNotification::show(
                    COMPLETE_REWARD_SONG, info->iconName, info->displayName,
                    "MUSIC", { 255, 200, 80 }, "SONG UNLOCKED!");
            }
        }
        refreshCompleteReward();
    }

    void onClaimCompleteReward(CCObject*) {
        // Aqui no se corta por m_passEnded: quien completa el pase justo al
        // final debe poder reclamar. Si la temporada acabo de verdad, el server
        // ya habra reiniciado el progreso y respondera que no llega a la meta.
        if (g_streakData.passCompleteRewardClaimed) return;
        if (g_streakData.goldTickets < g_streakData.getPassCompleteGoal()) {
            FLAlertLayer::create("Pass Reward", "Complete the pass to unlock this song.", "OK")->show();
            return;
        }
        if (m_completeRewardBtn) m_completeRewardBtn->setEnabled(false);

        matjson::Value payload = matjson::Value::object();
        claimOnServerEx("/streak-pass/complete-reward/claim", payload,
            [this, keepAlive = Ref<CCNode>(this)](bool ok, int code, const matjson::Value&) {
                if (ok) {
                    applyCompleteRewardUnlocked(true);
                    return;
                }
                if (code == 409) {
                    // El server ya lo tenia reclamado (por ejemplo, se perdio la
                    // respuesta de un intento anterior). No es un error.
                    applyCompleteRewardUnlocked(false);
                    return;
                }
                if (code == 403) {
                    FLAlertLayer::create("Pass Reward",
                        "The pass isn't complete on the server yet.\n"
                        "If you just finished it, reopen the pass and try again.", "OK")->show();
                    refreshCompleteReward();
                    return;
                }
                FLAlertLayer::create("Pass", "Could not claim the reward. Try again.", "OK")->show();
                refreshCompleteReward();
            });
    }

    void onBuyPremium(CCObject*) {
        if (m_passEnded) return;
        if (g_streakData.isPremiumPassActive()) return;

        int price = std::max(0, g_streakData.passPrice);

        auto popup = PurchaseConfirmPopup::create(
            "pass_img.png"_spr, "Premium Pass", price, PurchaseCurrency::Gems,
            [this, price](int discountPercent) {
            if (g_streakData.isPremiumPassActive()) return;
            int finalPrice = discountedPurchasePrice(price, discountPercent);
            if (finalPrice > 0 && g_streakData.gems < finalPrice) {
                FLAlertLayer::create(
                    "Not enough gems",
                    fmt::format("You need <cy>{}</c> gems to unlock the Premium Pass.", finalPrice).c_str(),
                    "OK"
                )->show();
                return;
            }
            matjson::Value payload = matjson::Value::object();
            payload.set("discount_percent", discountPercent);
            claimOnServer("/streak-pass/buy-premium", payload, [this, keepAlive = Ref<CCNode>(this)](bool ok) {
                if (!ok) {
                    FLAlertLayer::create("Error", "Purchase failed. Try again.", "OK")->show();
                    return;
                }
                g_streakData.premiumPassMonth = g_streakData.getCurrentMonth();
                PremiumUnlockAnim::show();
                refreshHeader();
                refreshTrack();
            });
        });
        if (popup) popup->show();
    }

    enum class TierState { Locked, Claimable, Claimed };

    TierState freeTierState(int tier) const {
        int reqSP = tier * FREE_TIER_STEP;
        if (g_streakData.isFreePassTierClaimed(tier)) return TierState::Claimed;
        if (g_streakData.goldTickets >= reqSP) return TierState::Claimable;
        return TierState::Locked;
    }

    TierState paidTierState(int tier) {
        int reqSP = tier * PAID_TIER_STEP;
        if (g_streakData.isPaidPassTierClaimed(tier)) return TierState::Claimed;
        if (!g_streakData.isPremiumPassActive()) return TierState::Locked;
        if (g_streakData.goldTickets >= reqSP) return TierState::Claimable;
        return TierState::Locked;
    }

    void attachClaimAnimation(CCNode* cell, CCNode* icon, int tier, float rowY, float bgW, float bgH, SEL_MenuHandler handler) {
        float colWidth = cell->getContentSize().width;

        auto particles = new CCParticleSystemQuad();
        if (particles && particles->initWithFile("chestOpened.plist", false)) {
            particles->autorelease();
            particles->setPosition({ colWidth / 2.f, rowY });
            particles->setScale(0.55f);
            cell->addChild(particles, 1);
        } else {
            CC_SAFE_DELETE(particles);
        }

        icon->runAction(CCRepeatForever::create(CCSequence::create(
            CCEaseSineInOut::create(CCScaleTo::create(0.45f, 1.15f)),
            CCEaseSineInOut::create(CCScaleTo::create(0.45f, 1.0f)),
            nullptr
        )));

        auto clickSpr = CCSprite::create();
        clickSpr->setContentSize({ bgW, bgH });
        auto clickBtn = CCMenuItemSpriteExtra::create(clickSpr, this, handler);
        clickBtn->setTag(tier);
        clickBtn->setContentSize({ bgW, bgH });
        auto clickMenu = CCMenu::createWithItem(clickBtn);
        clickMenu->setPosition({ colWidth / 2.f, rowY });
        cell->addChild(clickMenu, 5);
    }

    CCNode* buildIntroCell(float colWidth, float cellHeight) {
        auto cell = CCNode::create();
        cell->setContentSize({ colWidth, cellHeight });
        cell->ignoreAnchorPointForPosition(false);
        cell->setAnchorPoint({ 0.5f, 0.5f });

        float freeY = cellHeight - 40.f;
        float paidY = 40.f;
        float bgW = colWidth - 6.f;
        float bgH = 60.f;

        auto freeBg = makeRoundedSolidBG(bgW, bgH, { 60, 90, 150 }, 200);
        freeBg->setPosition({ colWidth / 2.f, freeY });
        cell->addChild(freeBg);

        auto freeSpr = CCSprite::create("free_pass.png"_spr);
        if (!freeSpr) freeSpr = CCSprite::createWithSpriteFrameName("GJ_unknownBtn_001.png");
        if (freeSpr) {
            float maxW = bgW - 6.f;
            float maxH = bgH - 6.f;
            float sx = maxW / std::max(freeSpr->getContentSize().width, 1.f);
            float sy = maxH / std::max(freeSpr->getContentSize().height, 1.f);
            freeSpr->setScale(std::min(sx, sy));
            freeSpr->setPosition({ colWidth / 2.f, freeY });
            cell->addChild(freeSpr);
        }

        auto vipBg = makeRoundedGradient(bgW, bgH, { 130, 200, 255 }, { 255, 170, 220 }, 220);
        vipBg.node->setPosition({ colWidth / 2.f, paidY });
        cell->addChild(vipBg.node);

        bool premiumOn = g_streakData.isPremiumPassActive();
        float vipSprY = (premiumOn || m_passEnded) ? paidY : paidY + 12.f;

        auto vipSpr = CCSprite::create("vip_pass.png"_spr);
        if (!vipSpr) vipSpr = CCSprite::createWithSpriteFrameName("GJ_unknownBtn_001.png");
        if (vipSpr) {
            float maxW = bgW;
            float maxH = (premiumOn || m_passEnded) ? (bgH - 4.f) : 40.f;
            float sx = maxW / std::max(vipSpr->getContentSize().width, 1.f);
            float sy = maxH / std::max(vipSpr->getContentSize().height, 1.f);
            vipSpr->setScale(std::min(sx, sy));
            vipSpr->setPosition({ colWidth / 2.f, vipSprY });
            cell->addChild(vipSpr, 2);
        }

        if (m_passEnded) {
            auto lbl = CCLabelBMFont::create("ENDED", "goldFont.fnt");
            lbl->setScale(0.35f);
            lbl->setColor({ 255, 100, 100 });
            lbl->setPosition({ colWidth / 2.f, paidY - 22.f });
            cell->addChild(lbl, 3);
        } else if (premiumOn) {
            auto lbl = CCLabelBMFont::create("ACTIVE", "goldFont.fnt");
            lbl->setScale(0.4f);
            lbl->setColor({ 120, 255, 140 });
            lbl->setPosition({ colWidth / 2.f, paidY - 22.f });
            cell->addChild(lbl, 3);
        } else {
            auto btnSpr = CCSprite::create("streak_pass_vip_btn.png"_spr);
            if (!btnSpr) btnSpr = CCSprite::createWithSpriteFrameName("GJ_button_01.png");
            if (btnSpr) {
                float maxH = 40.f;
                float scale = maxH / std::max(btnSpr->getContentSize().height, 1.f);
                btnSpr->setScale(scale);
            }
            auto btn = CCMenuItemSpriteExtra::create(
                btnSpr, this, menu_selector(StProgressPopup::onBuyPremium)
            );
            auto menu = CCMenu::createWithItem(btn);
            menu->setPosition({ colWidth / 2.f, paidY - 14.f });
            cell->addChild(menu, 3);
        }

        return cell;
    }

    CCNode* buildTierCell(int paidTier, float colWidth, float cellHeight) {
        auto cell = CCNode::create();
        cell->setContentSize({ colWidth, cellHeight });
        cell->ignoreAnchorPointForPosition(false);
        cell->setAnchorPoint({ 0.5f, 0.5f });

        float midY = cellHeight / 2.f;
        bool hasFree = (paidTier % 2 == 0);
        int freeTier = paidTier / 2;
        int paidSP = paidTier * PAID_TIER_STEP;
        bool isPaidMilestone = (paidSP % MILESTONE_SP == 0);
        bool isFreeMilestone = hasFree && ((freeTier * FREE_TIER_STEP) % MILESTONE_SP == 0);

        float freeY = cellHeight - 40.f;
        float paidY = 40.f;
        float bgW = colWidth - 6.f;
        float bgH = 60.f;

        if (hasFree) {
            if (isFreeMilestone) {
                auto bg = makeRoundedGradient(bgW, bgH, { 90, 200, 255 }, { 200, 120, 255 }, 220);
                bg.node->setPosition({ colWidth / 2.f, freeY });
                cell->addChild(bg.node);
                m_animatedGradients.push_back({ bg.gradient, 0.f });
            } else {
                auto bg = makeRoundedSolidBG(bgW, bgH, { 60, 90, 150 }, 160);
                bg->setPosition({ colWidth / 2.f, freeY });
                cell->addChild(bg);
            }

            auto reward = getFreeReward(freeTier);
            auto state = freeTierState(freeTier);

            auto icon = createRewardIcon(reward);
            icon->setPosition({ colWidth / 2.f, freeY });
            cell->addChild(icon, 3);

            if (state == TierState::Claimable && !m_passEnded) {
                attachClaimAnimation(cell, icon, freeTier, freeY, bgW, bgH,
                    menu_selector(StProgressPopup::onClaimFree));
            } else if (state == TierState::Claimed) {
                auto check = CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
                check->setScale(0.45f);
                check->setPosition({ colWidth - 10.f, freeY + 22.f });
                cell->addChild(check, 10);
            }
        } else {
            auto dash = CCLabelBMFont::create("--", "bigFont.fnt");
            dash->setScale(0.4f);
            dash->setOpacity(80);
            dash->setPosition({ colWidth / 2.f, freeY });
            cell->addChild(dash);
        }

        auto tierLbl = CCLabelBMFont::create(
            fmt::format("{}", paidSP).c_str(),
            "goldFont.fnt"
        );
        tierLbl->setScale(0.3f);
        tierLbl->setPosition({ colWidth / 2.f, midY - 16.f });
        cell->addChild(tierLbl);

        auto tierIdx = CCLabelBMFont::create(
            fmt::format("T{}", paidTier).c_str(),
            "bigFont.fnt"
        );
        tierIdx->setScale(0.32f);
        tierIdx->setOpacity(180);
        tierIdx->setPosition({ colWidth / 2.f, midY + 16.f });
        cell->addChild(tierIdx);

        auto reward = getPaidReward(paidTier);
        auto state = paidTierState(paidTier);
        bool premiumOn = g_streakData.isPremiumPassActive();

        if (isPaidMilestone) {
            auto bg = makeRoundedGradient(bgW, bgH, { 90, 200, 255 }, { 200, 120, 255 }, 230);
            bg.node->setPosition({ colWidth / 2.f, paidY });
            cell->addChild(bg.node);
            m_animatedGradients.push_back({ bg.gradient, 0.f });
        } else {
            auto bg = makeRoundedGradient(
                bgW, bgH,
                { 130, 200, 255 },
                { 255, 170, 220 },
                premiumOn ? 220 : 110
            );
            bg.node->setPosition({ colWidth / 2.f, paidY });
            cell->addChild(bg.node);
        }

        auto icon = createRewardIcon(reward);
        icon->setPosition({ colWidth / 2.f, paidY });
        cell->addChild(icon, 3);

        if (state == TierState::Claimable && !m_passEnded) {
            attachClaimAnimation(cell, icon, paidTier, paidY, bgW, bgH,
                menu_selector(StProgressPopup::onClaimPaid));
        } else if (state == TierState::Claimed) {
            auto check = CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
            check->setScale(0.45f);
            check->setPosition({ colWidth - 10.f, paidY + 22.f });
            cell->addChild(check, 10);
        } else if (!premiumOn) {
            auto lock = CCSprite::createWithSpriteFrameName("GJ_lockGray_001.png");
            lock->setScale(0.32f);
            lock->setPosition({ colWidth - 10.f, paidY + 22.f });
            cell->addChild(lock, 10);
        }

        return cell;
    }

    void refreshHeader() {
        if (m_goldLabel) {
            m_goldLabel->setString(fmt::format("x{}", g_streakData.goldTickets).c_str());
        }
        refreshCompleteReward();
        refreshGoldBuyBar();
    }

    std::string formatCountdown(long long secondsLeft) {
        if (secondsLeft <= 0) return "ENDED";
        long long days = secondsLeft / 86400;
        long long hours = (secondsLeft % 86400) / 3600;
        long long mins = (secondsLeft % 3600) / 60;
        if (days > 0) return fmt::format("{}d {}h {}m", days, hours, mins);
        if (hours > 0) return fmt::format("{}h {}m", hours, mins);
        return fmt::format("{}m", mins);
    }

    void updateCountdown(float) {
        if (!m_countdownLabel) return;
        long long endMs = g_streakData.getSeasonEndTime();
        long long now = g_streakData.getServerNowMs();
        long long remainingMs = endMs - now;
        long long remainingS = remainingMs / 1000;
        bool ended = (endMs > 0 && remainingS <= 0);

        m_countdownLabel->setString(
            ended
                ? "Pass: ENDED"
                : fmt::format("Pass: {}", formatCountdown(remainingS)).c_str()
        );
        m_countdownLabel->setColor(ended ? ccColor3B{ 255, 100, 100 } : ccColor3B{ 255, 255, 255 });

        if (ended != m_passEnded) {
            m_passEnded = ended;
            refreshTrack();
        }
    }

    float passProgressPct() const {
        float g = (float)std::clamp(g_streakData.goldTickets, 0, MONTHLY_GOAL_SP);
        float step = (float)PAID_TIER_STEP;
        float denom = (float)TOTAL_PAID_TIERS - 0.5f;
        float t = g / step;
        float pct = (t <= 1.f)
            ? t * (0.5f / denom)
            : (t - 0.5f) / denom;
        return std::clamp(pct, 0.f, 1.f);
    }

    void refreshTrack() {
        if (!m_serverLoaded) return;
        refreshHeader();
        if (!m_scrollLayer) return;

        float preservedX = m_scrollLayer->m_contentLayer->getPositionX();

        m_scrollLayer->m_contentLayer->removeAllChildren();
        m_animatedGradients.clear();
        m_progressBar = nullptr;

        auto listSize = m_scrollLayer->getContentSize();
        float colWidth = 58.f;
        float introWidth = 90.f;
        float cellHeight = listSize.height;
        float tiersWidth = colWidth * (float)TOTAL_PAID_TIERS;
        float totalWidth = 6.f + introWidth + tiersWidth + 6.f;

        m_scrollLayer->m_contentLayer->setContentSize({ totalWidth, cellHeight });

        float barHeight = 14.f;
        float barY = cellHeight / 2.f;
        bool premiumOn = g_streakData.isPremiumPassActive();
        float barWidth = tiersWidth - colWidth / 2.f;
        m_progressBar = RoundedProgressBar::create(barWidth, barHeight);
        m_progressBar->setPosition({ 6.f + introWidth + barWidth / 2.f, barY });
        if (premiumOn) {
            m_progressBar->setRainbowMode(true);
        } else {
            m_progressBar->setGradientColors({ 250, 225, 60 }, { 255, 165, 0 });
        }
        m_progressBar->setProgress(passProgressPct());
        m_scrollLayer->m_contentLayer->addChild(m_progressBar, 1);

        auto intro = buildIntroCell(introWidth, cellHeight);
        intro->setPosition({ 6.f + introWidth / 2.f, cellHeight / 2.f });
        m_scrollLayer->m_contentLayer->addChild(intro);

        for (int t = 1; t <= TOTAL_PAID_TIERS; ++t) {
            auto cell = buildTierCell(t, colWidth, cellHeight);
            cell->setPosition({
                6.f + introWidth + colWidth / 2.f + (t - 1) * colWidth,
                cellHeight / 2.f
            });
            m_scrollLayer->m_contentLayer->addChild(cell);
        }

        float maxScrollX = std::max(0.f, totalWidth - listSize.width);
        if (!m_initialScrollDone) {
            int currentTier = std::clamp(g_streakData.goldTickets / PAID_TIER_STEP, 0, TOTAL_PAID_TIERS);
            float scrollX = std::max(0.f, introWidth + (currentTier - 2) * colWidth);
            scrollX = std::min(scrollX, maxScrollX);
            m_scrollLayer->m_contentLayer->setPositionX(-scrollX);
            m_initialScrollDone = true;
        } else {
            float clampedX = std::clamp(preservedX, -maxScrollX, 0.f);
            m_scrollLayer->m_contentLayer->setPositionX(clampedX);
        }
    }

    void animateGradients(float dt) {
        for (auto& ag : m_animatedGradients) {
            if (!ag.layer) continue;
            ag.hue += dt * 0.35f;
            if (ag.hue > 1.f) ag.hue -= 1.f;
            float h2 = ag.hue + 0.18f;
            if (h2 > 1.f) h2 -= 1.f;
            auto c1 = HSVtoRGB(ag.hue, 0.55f, 1.f);
            auto c2 = HSVtoRGB(h2, 0.65f, 1.f);
            ag.layer->setStartColor(c1);
            ag.layer->setEndColor(c2);
        }
    }

    bool init() override {
        if (!Popup::init(460.f, 300.f, "geode.loader/GE_square03.png")) return false;
        auto winSize = m_mainLayer->getContentSize();

        {
            auto stencil = cocos2d::extension::CCScale9Sprite::create("geode.loader/GE_square03.png");
            stencil->setContentSize({ winSize.width, winSize.height });
            stencil->setAnchorPoint({ 0.5f, 0.5f });
            stencil->setPosition({ winSize.width / 2.f, winSize.height / 2.f });

            auto bgClipper = CCClippingNode::create(stencil);
            bgClipper->setAlphaThreshold(0.05f);

            m_bgGradient = CCLayerGradient::create(
                ccc4(120, 50, 200, 110),
                ccc4(40, 100, 220, 110),
                ccp(1, -1)
            );
            m_bgGradient->setContentSize({ winSize.width, winSize.height });
            bgClipper->addChild(m_bgGradient);

            m_mainLayer->addChild(bgClipper, 1);
        }

        {
            auto headerStencil = cocos2d::extension::CCScale9Sprite::create("geode.loader/GE_square03.png");
            headerStencil->setContentSize({ winSize.width, winSize.height });
            headerStencil->setAnchorPoint({ 0.5f, 0.5f });
            headerStencil->setPosition({ winSize.width / 2.f, winSize.height / 2.f });

            auto headerClipper = CCClippingNode::create(headerStencil);
            headerClipper->setAlphaThreshold(0.05f);

            auto headerBG = CCSprite::create("StreakPassBG_1.png"_spr);
            if (headerBG) {
                float scale = winSize.width / std::max(headerBG->getContentSize().width, 1.f);
                headerBG->setScale(scale);
                headerBG->setAnchorPoint({ 0.5f, 1.f });
                headerBG->setPosition({ winSize.width / 2.f, winSize.height });
                headerBG->setOpacity(110);
                headerClipper->addChild(headerBG);
            }

            m_mainLayer->addChild(headerClipper, 2);
        }

        {
            auto titleSpr = CCSprite::create("StreakPassTitle.png"_spr);
            if (titleSpr) {
                float maxW = winSize.width * 1.4f;
                float maxH = 130.f;
                float sx = maxW / std::max(titleSpr->getContentSize().width, 1.f);
                float sy = maxH / std::max(titleSpr->getContentSize().height, 1.f);
                titleSpr->setScale(std::min(sx, sy));
                titleSpr->setPosition({ winSize.width / 2.f, winSize.height - 13.f });
                m_mainLayer->addChild(titleSpr, 3);
            }
        }

        if (m_buttonMenu) m_buttonMenu->setZOrder(10);


        m_countdownLabel = CCLabelBMFont::create("Pass: --", "bigFont.fnt");
        m_countdownLabel->setScale(0.4f);
        m_countdownLabel->setAnchorPoint({ 0.5f, 0.5f });
        m_countdownLabel->setPosition({ winSize.width / 2.f, winSize.height - 40.f });
        m_mainLayer->addChild(m_countdownLabel, 2);

        {
            auto goldIcon = CCSprite::create("gold_ticket.png"_spr);
            float iconW = 0.f;
            if (goldIcon) {
                goldIcon->setScale(0.16f);
                goldIcon->setPosition({ winSize.width / 2.f - 20.f, winSize.height - 56.f });
                m_mainLayer->addChild(goldIcon, 2);
                iconW = goldIcon->getScaledContentSize().width;
            }
            m_goldLabel = CCLabelBMFont::create(
                fmt::format("x{}", g_streakData.goldTickets).c_str(), "bigFont.fnt"
            );
            m_goldLabel->setScale(0.4f);
            m_goldLabel->setAnchorPoint({ 0.f, 0.5f });
            m_goldLabel->setPosition({ winSize.width / 2.f - 20.f + iconW / 2.f + 4.f, winSize.height - 56.f });
            m_mainLayer->addChild(m_goldLabel, 2);
        }

        auto missionsSpr = CCSprite::create("gold_ticket_btn.png"_spr);
        if (!missionsSpr) missionsSpr = CCSprite::createWithSpriteFrameName("GJ_button_01.png");
        if (missionsSpr) {
            float maxH = 34.f;
            missionsSpr->setScale(maxH / std::max(missionsSpr->getContentSize().height, 1.f));
        }
        auto missionsBtn = CCMenuItemSpriteExtra::create(
            missionsSpr, this, menu_selector(StProgressPopup::onMissions)
        );
        auto missionsMenu = CCMenu::createWithItem(missionsBtn);
        missionsMenu->setPosition({ winSize.width - 36.f, winSize.height - 30.f });
        m_mainLayer->addChild(missionsMenu, 2);

        auto giftSpr = CCSprite::create("gif_pass_btn.png"_spr);
        if (!giftSpr) giftSpr = ButtonSprite::create("Gift");
        if (giftSpr) {
            float maxH = 34.f;
            giftSpr->setScale(maxH / std::max(giftSpr->getContentSize().height, 1.f));
        }
        auto giftBtn = CCMenuItemSpriteExtra::create(
            giftSpr, this, menu_selector(StProgressPopup::onGiftPass));
        auto giftMenu = CCMenu::createWithItem(giftBtn);
        giftMenu->setPosition({ winSize.width - 80.f, winSize.height - 30.f });
        m_mainLayer->addChild(giftMenu, 2);

        buildCompleteReward(winSize);

        auto listSize = CCSize{ 420.f, 200.f };
        auto bg = cocos2d::extension::CCScale9Sprite::create("square02_001.png");
        bg->setContentSize(listSize);
        bg->setColor({ 0, 0, 0 });
        bg->setOpacity(80);
        bg->setPosition({ winSize.width / 2.f, listSize.height / 2.f + 12.f });
        m_mainLayer->addChild(bg, 2);

        m_scrollLayer = ScrollLayer::create(listSize, true, false);
        m_scrollLayer->setPosition({ (winSize.width - listSize.width) / 2.f, 12.f });
        m_mainLayer->addChild(m_scrollLayer, 2);

        m_spinner = StatusSpinner::create();
        m_spinner->setPosition({ winSize.width / 2.f, listSize.height / 2.f + 12.f });
        m_mainLayer->addChild(m_spinner, 30);

        this->scheduleUpdate();
        this->schedule(schedule_selector(StProgressPopup::updateCountdown), 1.0f);

        loadFromServer();

        return true;
    }

    // The pass is server-authoritative: never trust the local cache. Every time the
    // popup opens we re-fetch the player's pass data (gold tickets, claimed tiers,
    // premium status, reward config) and only build the track once it arrives.
    void loadFromServer() {
        if (m_spinner) m_spinner->setLoading("Loading pass...");
        if (m_goldLabel) m_goldLabel->setString("x...");

        Ref<StProgressPopup> self = this;
        refreshPlayerDataFromServer([self](bool ok) {
            if (!self->isRunning()) return;
            if (!ok) {
                if (self->m_spinner) self->m_spinner->setError("Couldn't load the pass.\nCheck your connection.");
                return;
            }
            self->m_serverLoaded = true;
            if (self->m_spinner) self->m_spinner->hide();
            self->updateCountdown(0);
            self->refreshTrack();
        });
    }

    void update(float dt) override {
        if (m_scrollLayer && m_scrollLayer->m_contentLayer) {
            float contentW = m_scrollLayer->m_contentLayer->getContentSize().width;
            float viewW = m_scrollLayer->getContentSize().width;
            float minX = std::min(0.f, viewW - contentW);
            float currentX = m_scrollLayer->m_contentLayer->getPositionX();
            if (currentX > 0.f) m_scrollLayer->m_contentLayer->setPositionX(0.f);
            else if (currentX < minX) m_scrollLayer->m_contentLayer->setPositionX(minX);
        }
        animateGradients(dt);

        if (m_bgGradient) {
            m_bgHue += dt * 0.08f;
            if (m_bgHue > 1.f) m_bgHue -= 1.f;
            auto c1 = HSVtoRGB(m_bgHue, 0.55f, 0.85f);
            auto c2 = HSVtoRGB(m_bgHue + 0.25f, 0.6f, 0.85f);
            m_bgGradient->setStartColor(c1);
            m_bgGradient->setEndColor(c2);
            m_bgGradient->setStartOpacity(110);
            m_bgGradient->setEndOpacity(110);
        }
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
