#pragma once
#include <Geode/ui/Popup.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/utils/async.hpp>
#include <Geode/binding/GJAccountManager.hpp>
#include "../StreakData.h"
#include "../utils/RoundedProgressBar.h"
#include "../RewardNotification.h"
#include "ChestRewardHelper.h" 
#include "../StatusSpinner.h" 
#include "../HMACAuth.h"

using namespace geode::prelude;

extern void updatePlayerDataInFirebase();

class DiscordGoalPopup : public Popup {
protected:
    RoundedProgressBar* m_progressBar = nullptr;
    CCLabelBMFont* m_countLabel = nullptr;
    CCLabelBMFont* m_nextGoalLabel = nullptr;
    CCMenu* m_milestoneMenu = nullptr;
    CCNode* m_uiContainer = nullptr;
    CCNode* m_linesContainer = nullptr;

    StatusSpinner* m_spinner = nullptr;
    async::TaskHolder<web::WebResponse> m_fetchTask;

    bool init() override {
        if (!Popup::init(340.f, 220.f, "geode.loader/GE_square03.png")) return false;

        this->setTitle("Discord Event");
        auto winSize = m_mainLayer->getContentSize();
        float offsetY = -20.0f;

        m_spinner = StatusSpinner::create();
        m_spinner->setPosition(winSize / 2);
        m_mainLayer->addChild(m_spinner, 100);

        auto reloadSpr = CCSprite::createWithSpriteFrameName("GJ_updateBtn_001.png");
        auto reloadBtn = CCMenuItemSpriteExtra::create(reloadSpr, this, menu_selector(DiscordGoalPopup::onRefresh));
        reloadBtn->setPosition({ 20.0f, 20.0f });

        auto topMenu = CCMenu::create();
        topMenu->setPosition({ 0, 0 });
        topMenu->addChild(reloadBtn);
        m_mainLayer->addChild(topMenu, 101);

        m_uiContainer = CCNode::create();
        m_uiContainer->setContentSize(winSize);
        m_uiContainer->setAnchorPoint({ 0, 0 });
        m_mainLayer->addChild(m_uiContainer);

        auto communityLogo = CCSprite::create("discord_comunity.png"_spr);
        if (communityLogo) {
            communityLogo->setScale(0.5f);
            communityLogo->setPosition({ winSize.width / 2, (winSize.height / 2) + 75.f + offsetY });
            m_uiContainer->addChild(communityLogo);
        }

        m_countLabel = CCLabelBMFont::create("0/0 Members!", "goldFont.fnt");
        m_countLabel->setScale(0.60f);
        m_countLabel->setPosition({ winSize.width / 2, (winSize.height / 2) + 35.f + offsetY });
        m_uiContainer->addChild(m_countLabel);

        m_progressBar = RoundedProgressBar::create(260.0f, 25.0f);
        m_progressBar->setPosition({ winSize.width / 2, (winSize.height / 2) + offsetY });
        m_progressBar->setRainbowMode(true);
        m_uiContainer->addChild(m_progressBar);

        m_linesContainer = CCNode::create();
        m_uiContainer->addChild(m_linesContainer, 6);

        m_milestoneMenu = CCMenu::create();
        m_milestoneMenu->setPosition({ 0, 0 });
        m_uiContainer->addChild(m_milestoneMenu, 10);

        auto joinSpr = ButtonSprite::create("Join Discord!", "goldFont.fnt", "GJ_button_01.png", 0.7f);
        auto joinBtn = CCMenuItemSpriteExtra::create(joinSpr, this, menu_selector(DiscordGoalPopup::onJoinDiscord));
        auto joinMenu = CCMenu::create();
        joinMenu->addChild(joinBtn);
        joinMenu->setPosition({ winSize.width / 2, -2.0f });
        m_uiContainer->addChild(joinMenu);

        m_nextGoalLabel = CCLabelBMFont::create("Next\n0/0", "goldFont.fnt");
        m_nextGoalLabel->setScale(0.45f);
        m_nextGoalLabel->setAlignment(kCCTextAlignmentRight);
        m_nextGoalLabel->setAnchorPoint({ 1.0f, 1.0f });
        m_nextGoalLabel->setPosition({ winSize.width - 15.0f, winSize.height - 15.0f });
        m_uiContainer->addChild(m_nextGoalLabel);

        this->refreshUI();
        return true;
    }

    void refreshUI() {
        if (!m_uiContainer) return;

        int count = g_streakData.discordCount;
        int max = g_streakData.discordGoalMax;
        auto& milestones = g_streakData.m_discordMilestones;
        auto winSize = m_mainLayer->getContentSize();
        float offsetY = -20.0f;

        m_milestoneMenu->removeAllChildren();
        m_linesContainer->removeAllChildren();

        float progress = (max > 0) ? (float)count / (float)max : 0;
        if (progress > 1.0f) progress = 1.0f;

        m_progressBar->setProgress(progress);
        m_countLabel->setString(fmt::format("{}/{} Members!", count, max).c_str());

        int nextMilestone = max;
        for (const auto& m : milestones) {
            if (m.requirement > count) {
                nextMilestone = m.requirement;
                break;
            }
        }

        if (m_nextGoalLabel) {
            if (count >= max) {
                m_nextGoalLabel->setString("Goal\nReached!");
            }
            else {
                m_nextGoalLabel->setString(fmt::format("Next\n{}/{}", count, nextMilestone).c_str());
            }
        }

        for (auto& m : milestones) {
            float posX = (winSize.width / 2 - 130.0f) + (260.0f * ((float)m.requirement / max));

            auto line = CCSprite::createWithSpriteFrameName("edit_vLine_001.png");
            if (line) {
                line->setScaleY(m.isChest ? 0.35f : 0.65f);
                line->setOpacity(100);
                line->setPosition({ posX, (winSize.height / 2) + offsetY });
                m_linesContainer->addChild(line);
            }

            bool isReached = count >= m.requirement;
            bool isClaimed = g_streakData.claimedDiscordMilestones.count(m.requirement);

            CCSprite* spr = nullptr;
            if (m.isChest) {
                spr = CCSprite::createWithSpriteFrameName(m.spriteName.c_str());
            }
            else {
                std::string modSpriteName = fmt::format("{}/{}", Mod::get()->getID(), m.spriteName);
                spr = CCSprite::create(modSpriteName.c_str());
            }

            if (!spr) spr = CCSprite::createWithSpriteFrameName("GJ_lock_001.png");

            float baseScale = m.isChest ? 0.16f : 0.23f;
            spr->setScale(baseScale);

            if (isClaimed) {
                spr->setOpacity(120);
                auto check = CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
                check->setPosition(spr->getContentSize() / 2);
                spr->addChild(check);
            }
            else if (!isReached) {
                spr->setColor({ 100, 100, 100 });
            }

            auto btn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(DiscordGoalPopup::onClaimReward));
            btn->setTag(m.requirement);
            btn->setPosition({ posX, (winSize.height / 2) - 32.0f + offsetY });
            btn->setEnabled(isReached && !isClaimed);

            if (isReached && !isClaimed) {
                auto pulse = CCRepeatForever::create(CCSequence::create(
                    CCScaleTo::create(0.6f, baseScale * 1.15f),
                    CCScaleTo::create(0.6f, baseScale),
                    nullptr
                ));
                spr->runAction(pulse);
            }

            m_milestoneMenu->addChild(btn);

            if (!m.isChest) {
                std::string rewardText = "";
                if (m.gems > 0) rewardText = fmt::format("x{}", m.gems);
                else if (m.stars > 0) rewardText = fmt::format("x{}", m.stars);
                else if (m.tickets > 0) rewardText = fmt::format("x{}", m.tickets);

                if (!rewardText.empty()) {
                    auto lbl = CCLabelBMFont::create(rewardText.c_str(), "chatFont.fnt");
                    lbl->setScale(0.32f);
                    lbl->setPosition({ posX, (winSize.height / 2) - 55.0f + offsetY });
                    m_milestoneMenu->addChild(lbl);
                }
            }
        }
    }

    void onClaimReward(CCObject* sender) {
        auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
        if (!btn) return;

        int req = btn->getTag();
        auto& milestones = g_streakData.m_discordMilestones;
        auto it = std::find_if(milestones.begin(), milestones.end(), [req](const DiscordMilestone& m) {
            return m.requirement == req;
            });

        if (it != milestones.end()) {
            if (g_streakData.claimedDiscordMilestones.count(req)) return;

            btn->setEnabled(false);

           
            if (auto spr = static_cast<CCSprite*>(btn->getNormalImage())) {
                spr->stopAllActions(); 
                float baseScale = it->isChest ? 0.16f : 0.23f;
                spr->setScale(baseScale);
                spr->setOpacity(120);

                auto check = CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
                check->setPosition(spr->getContentSize() / 2);
                spr->addChild(check);
            }

            g_streakData.claimedDiscordMilestones.insert(req);
            g_streakData.save();
            updatePlayerDataInFirebase();

            if (it->isChest) {
         
                ChestRewardHelper::openRandomChest([this]() { this->refreshUI(); });
            }
            else {
                if (it->tickets > 0) {
                    int start = g_streakData.starTickets;
                    g_streakData.starTickets += it->tickets;
                    RewardNotification::show("star_tiket.png"_spr, start, it->tickets);
                }
                if (it->stars > 0) {
                    int start = g_streakData.superStars;
                    g_streakData.superStars += it->stars;
                    RewardNotification::show("super_star.png"_spr, start, it->stars);
                }
                if (it->gems > 0) {
                    int start = g_streakData.gems;
                    g_streakData.gems += it->gems;
                    RewardNotification::show("gem.png"_spr, start, it->gems);
                }
                g_streakData.save();
                updatePlayerDataInFirebase();
                FMODAudioEngine::sharedEngine()->playEffect("buyItem01.ogg");
                this->refreshUI();
            }
        }
    }

    void onJoinDiscord(CCObject*) {
        geode::utils::web::openLinkInBrowser("https://discord.gg/dykf3y6HWw");
    }

    void onRefresh(CCObject*) {
        auto am = GJAccountManager::sharedState();
        if (!am || am->m_accountID == 0) return;

        m_uiContainer->setVisible(false);
        m_spinner->setLoading("Updating...");

        std::string url = fmt::format("https://streak-servidor.onrender.com/players/{}", am->m_accountID);
        auto req = web::WebRequest();
        HMACAuth::signGetRequest(req, am->m_accountID);

        m_fetchTask.spawn(req.get(url), [this](web::WebResponse res) {
            if (res.ok() && res.json().isOk()) {
                auto data = res.json().unwrap();
                g_streakData.parseServerResponse(data);
                this->refreshUI();
                m_spinner->hide();
                m_uiContainer->setVisible(true);
            }
            else {
                m_spinner->setError("Try again in 1m");
                this->runAction(CCSequence::create(
                    CCDelayTime::create(1.5f),
                    CallFuncExt::create([this]() { m_spinner->hide(); m_uiContainer->setVisible(true); }),
                    nullptr
                ));
            }
            });
    }

public:
    static DiscordGoalPopup* create() {
        auto ret = new DiscordGoalPopup();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};