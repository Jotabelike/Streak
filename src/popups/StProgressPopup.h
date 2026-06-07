#pragma once
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/ScrollLayer.hpp>
#include <Geode/ui/Notification.hpp>
#include "../StreakData.h"
#include "../BannerNotification.h"
#include "../BadgeNotification.h"
#include "../RewardNotification.h"
#include "../FirebaseManager.h"
#include "../utils/RoundedProgressBar.h"
#include "StreakChestPopup.h"
#include "PremiumUnlockAnim.h"
#include "GoldTicketMissionsPopup.h"
#include "PassPurchasePopup.h"
#include "../NameModifiers.h"

using namespace geode::prelude;

class StProgressPopup : public Popup {
protected:
    static constexpr int MONTHLY_GOAL_SP = 2000;
    static constexpr int PAID_TIER_STEP = 50;
    static constexpr int FREE_TIER_STEP = 100;
    static constexpr int TOTAL_PAID_TIERS = MONTHLY_GOAL_SP / PAID_TIER_STEP;
    static constexpr int MILESTONE_SP = 200;

    enum class PassRewardType { None, Tickets, Stars, Gems, Shields, Chest, Banner, NameItem };

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
    CCMenuItemSpriteExtra* m_claimAllBtn = nullptr;
    std::vector<AnimatedGradient> m_animatedGradients;
    CCLayerGradient* m_bgGradient = nullptr;
    float m_bgHue = 0.f;
    bool m_passEnded = false;
    bool m_initialScrollDone = false;

    static PassRewardType typeFromString(const std::string& s) {
        if (s == "tickets")   return PassRewardType::Tickets;
        if (s == "stars")     return PassRewardType::Stars;
        if (s == "gems")      return PassRewardType::Gems;
        if (s == "shields")   return PassRewardType::Shields;
        if (s == "chest")     return PassRewardType::Chest;
        if (s == "banner")    return PassRewardType::Banner;
        if (s == "name_item") return PassRewardType::NameItem;
        return PassRewardType::None;
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
                g_streakData.streakShields += reward.amount;
                RewardNotification::show(spr, start, reward.amount, spawnPos);
                break;
            }
            default: break;
        }
    }

    CCNode* createRewardIcon(const PassReward& reward) {
        auto node = CCNode::create();
        node->setContentSize({ 44.f, 44.f });
        node->setAnchorPoint({ 0.5f, 0.5f });

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

        claimOnServer("/streak-pass/tier/claim", payload,
            [this, isFree, tier, reward, spawnPos](bool ok) {
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
                               || reward.type == PassRewardType::Shields;
                if (isCurrency) {
                    // Balance already updated from the server response.
                    showPassRewardAnim(reward, spawnPos);
                } else {
                    // Chest / Banner / NameItem: grantReward opens the chest popup or
                    // unlocks the item locally (server already recorded the unlock).
                    grantReward(reward, spawnPos);
                }
                FMODAudioEngine::sharedEngine()->playEffect("dummyDestroy.ogg");
                refreshTrack();
            });
    }

    void onClaimAll(CCObject*) {
        if (m_passEnded) return;

        matjson::Value payload = matjson::Value::object();
        claimOnServer("/streak-pass/claim-all", payload, [this](bool ok) {
            if (!ok) {
                FLAlertLayer::create("Pass",
                    "Could not claim the rewards. Check your connection and try again.", "OK")->show();
                return;
            }
            // The server grants and persists every eligible currency tier; mirror the
            // same selection locally so the UI marks them claimed. Chest/banner/name
            // tiers are skipped and stay claimable individually.
            auto isCurrency = [](PassRewardType t) {
                return t == PassRewardType::Tickets || t == PassRewardType::Stars
                    || t == PassRewardType::Gems || t == PassRewardType::Shields;
            };
            int sp = g_streakData.goldTickets;
            for (int t = 1; t <= TOTAL_PAID_TIERS / 2; ++t) {
                if (sp < t * FREE_TIER_STEP) break;
                if (g_streakData.isFreePassTierClaimed(t)) continue;
                if (!isCurrency(getFreeReward(t).type)) continue;
                g_streakData.setFreePassTierClaimed(t);
            }
            if (g_streakData.isPremiumPassActive()) {
                for (int t = 1; t <= TOTAL_PAID_TIERS; ++t) {
                    if (sp < t * PAID_TIER_STEP) break;
                    if (g_streakData.isPaidPassTierClaimed(t)) continue;
                    if (!isCurrency(getPaidReward(t).type)) continue;
                    g_streakData.setPaidPassTierClaimed(t);
                }
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

    void onBuyPremium(CCObject*) {
        if (m_passEnded) return;
        if (g_streakData.isPremiumPassActive()) return;

        int price = std::max(0, g_streakData.passPrice);

        auto popup = PassPurchasePopup::create(price, [this, price]() {
            if (g_streakData.isPremiumPassActive()) return;
            if (price > 0 && g_streakData.gems < price) {
                FLAlertLayer::create(
                    "Not enough gems",
                    fmt::format("You need <cy>{}</c> gems to unlock the Premium Pass.", price).c_str(),
                    "OK"
                )->show();
                return;
            }
            matjson::Value payload = matjson::Value::object();
            claimOnServer("/streak-pass/buy-premium", payload, [this](bool ok) {
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

    bool hasAnyClaimable() {
        int sp = g_streakData.goldTickets;
        for (int t = 1; t <= TOTAL_PAID_TIERS / 2; ++t) {
            if (!g_streakData.isFreePassTierClaimed(t) && sp >= t * FREE_TIER_STEP) return true;
        }
        if (g_streakData.isPremiumPassActive()) {
            for (int t = 1; t <= TOTAL_PAID_TIERS; ++t) {
                if (!g_streakData.isPaidPassTierClaimed(t) && sp >= t * PAID_TIER_STEP) return true;
            }
        }
        return false;
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
        refreshClaimAll();
        if (m_goldLabel) {
            m_goldLabel->setString(fmt::format("x{}", g_streakData.goldTickets).c_str());
        }
    }

    void refreshClaimAll() {
        if (!m_claimAllBtn) return;
        bool active = !m_passEnded && hasAnyClaimable();
        m_claimAllBtn->setEnabled(active);
        m_claimAllBtn->setVisible(active);
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
        long long now = static_cast<long long>(std::time(nullptr)) * 1000LL;
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
        float goal = (float)MONTHLY_GOAL_SP;
        float step = (float)PAID_TIER_STEP;
        float halfCol = 0.5f / (float)TOTAL_PAID_TIERS;
        float pct = (g <= step)
            ? (g / step) * halfCol
            : (g / goal) - halfCol;
        return std::clamp(pct, 0.f, 1.f);
    }

    void refreshTrack() {
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
        m_progressBar = RoundedProgressBar::create(tiersWidth, barHeight);
        m_progressBar->setPosition({ 6.f + introWidth + tiersWidth / 2.f, barY });
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

        auto claimAllSpr = ButtonSprite::create("Claim All", 0, 0, "bigFont.fnt", "GJ_button_01.png", 0, 0.6f);
        claimAllSpr->setScale(0.5f);
        m_claimAllBtn = CCMenuItemSpriteExtra::create(
            claimAllSpr, this, menu_selector(StProgressPopup::onClaimAll)
        );
        auto claimMenu = CCMenu::createWithItem(m_claimAllBtn);
        claimMenu->setPosition({ 60.f, winSize.height - 30.f });
        m_mainLayer->addChild(claimMenu, 2);

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

        updateCountdown(0);
        refreshTrack();
        this->scheduleUpdate();
        this->schedule(schedule_selector(StProgressPopup::updateCountdown), 1.0f);

        return true;
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
