#pragma once
#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/utils/async.hpp>
#include <Geode/binding/GJAccountManager.hpp>
#include <matjson.hpp>
#include "../StreakData.h"
#include "../RewardNotification.h"
#include "../HMACAuth.h"
#include "../utils/RoundedProgressBar.h"
#include "../StatusSpinner.h"
#include "StreakChestPopup.h"

using namespace geode::prelude;

class XPPopup : public Popup {
protected:
    ScrollLayer* m_list = nullptr;
    CCMenu* m_claimMenu = nullptr;
    CCNode* m_listContainer = nullptr;
    StatusSpinner* m_spinner = nullptr;
    async::TaskHolder<web::WebResponse> m_claimListener;

    void onClaimLevelReward(CCObject* sender) {
        auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
        int level = btn->getTag();
        if (level <= 0) return;

        auto am = GJAccountManager::sharedState();
        if (!am || am->m_accountID == 0) return;
        int accountID = am->m_accountID;

        // disable while the request is in flight
        btn->setEnabled(false);
        if (auto img = dynamic_cast<CCRGBAProtocol*>(btn->getNormalImage())) img->setOpacity(120);

        int startGems = g_streakData.gems;
        int startStars = g_streakData.superStars;
        int startTickets = g_streakData.starTickets;
        int startShields = g_streakData.streakShields;

        matjson::Value payload = matjson::Value::object();
        payload.set("level", level);

        auto req = web::WebRequest();
        HMACAuth::signRequest(req, accountID, payload);

        std::string url = fmt::format("https://streak-servidor.onrender.com/players/{}/claim-level-reward", accountID);

        // Retain the button: the request resolves on the main thread later, and if the
        // popup is closed in the meantime its children are cleaned up. A raw pointer would
        // dangle and crash; the Ref keeps it alive (orphaned, guarded by the parent check).
        Ref<CCMenuItemSpriteExtra> btnRef = btn;
        m_claimListener.spawn(
            req.bodyJSON(payload).post(url),
            [this, btnRef, level, startGems, startStars, startTickets, startShields](web::WebResponse res) {
                CCMenuItemSpriteExtra* btn = btnRef;
                int httpCode = res.code();
                if (!res.ok() || !res.json().isOk()) {
                    btn->setEnabled(true);
                    if (auto img = dynamic_cast<CCRGBAProtocol*>(btn->getNormalImage())) img->setOpacity(255);

                    if (httpCode == 404) {
                        for (auto it = g_streakData.pendingLevelRewards.begin();
                             it != g_streakData.pendingLevelRewards.end(); ) {
                            if (it->level == level) it = g_streakData.pendingLevelRewards.erase(it);
                            else ++it;
                        }
                        btn->setVisible(false);
                        FLAlertLayer::create(
                            "Already Claimed",
                            "This level reward was already claimed. Refreshing list.",
                            "OK"
                        )->show();
                    } else {
                        FLAlertLayer::create(
                            "Claim Failed",
                            fmt::format("Could not claim reward (code {}). Try again.", httpCode).c_str(),
                            "OK"
                        )->show();
                    }
                    return;
                }

                auto data = res.json().unwrap();
                if (!data["success"].as<bool>().unwrapOr(false)) {
                    btn->setEnabled(true);
                    if (auto img = dynamic_cast<CCRGBAProtocol*>(btn->getNormalImage())) img->setOpacity(255);
                    return;
                }

                // Apply server-confirmed balances
                if (data.contains("balances")) {
                    auto bal = data["balances"];
                    g_streakData.superStars   = bal["super_stars"].as<int>().unwrapOr(g_streakData.superStars);
                    g_streakData.starTickets  = bal["star_tickets"].as<int>().unwrapOr(g_streakData.starTickets);
                    g_streakData.gems         = bal["gems"].as<int>().unwrapOr(g_streakData.gems);
                    g_streakData.streakShields = bal["streak_shields"].as<int>().unwrapOr(g_streakData.streakShields);
                    g_streakData.currentXP    = bal["current_xp"].as<int>().unwrapOr(g_streakData.currentXP);
                    g_streakData.currentLevel = bal["current_level"].as<int>().unwrapOr(g_streakData.currentLevel);
                }

                // Sync pending list
                g_streakData.pendingLevelRewards.clear();
                if (data.contains("pending_level_rewards")) {
                    auto arr = data["pending_level_rewards"].as<std::vector<matjson::Value>>();
                    if (arr.isOk()) {
                        for (const auto& item : arr.unwrap()) {
                            int lvl = item["level"].as<int>().unwrapOr(0);
                            if (lvl <= 0) continue;
                            StreakData::PendingLevelReward p;
                            p.level = lvl;
                            p.stars       = item["stars"].as<int>().unwrapOr(0);
                            p.tickets     = item["tickets"].as<int>().unwrapOr(0);
                            p.gems        = item["gems"].as<int>().unwrapOr(0);
                            p.shields     = item["shields"].as<int>().unwrapOr(0);
                            p.chestRarity = item["chestRarity"].as<int>().unwrapOr(0);
                            g_streakData.pendingLevelRewards.push_back(p);
                        }
                    }
                }

                int gainedGems = g_streakData.gems - startGems;
                int gainedStars = g_streakData.superStars - startStars;
                int gainedTickets = g_streakData.starTickets - startTickets;
                int gainedShields = g_streakData.streakShields - startShields;

                auto winSize = CCDirector::sharedDirector()->getWinSize();
                CCPoint spawnPos = winSize / 2;

                bool chestOpened = data.contains("chestRolled") && !data["chestRolled"].isNull();

                if (chestOpened) {
                    // Es un cofre: abrir StreakChestPopup con la animacion (igual que misiones diarias).
                    int chestStars   = data["chestRolled"]["superStars"].as<int>().unwrapOr(0);
                    int chestTickets = data["chestRolled"]["starTickets"].as<int>().unwrapOr(0);
                    int chestGems    = data["chestRolled"]["gems"].as<int>().unwrapOr(0);
                    int chestXP      = data["chestRolled"]["xp"].as<int>().unwrapOr(0);
                    int rarity = (data.contains("claimed") && data["claimed"].contains("chestRarity"))
                        ? data["claimed"]["chestRarity"].as<int>().unwrapOr(4)
                        : 4;

                    if (auto popup = StreakChestPopup::createOpen(chestStars, chestTickets, chestGems, chestXP, rarity, nullptr)) {
                        popup->show();
                    }
                }
                else {
                    if (gainedGems > 0)    RewardNotification::show("gem.png"_spr,        startGems,    gainedGems,    spawnPos);
                    if (gainedStars > 0)   RewardNotification::show("super_star.png"_spr, startStars,   gainedStars,   spawnPos);
                    if (gainedTickets > 0) RewardNotification::show("star_tiket.png"_spr, startTickets, gainedTickets, spawnPos);
                    if (gainedShields > 0) RewardNotification::show("heart.png"_spr,      startShields, gainedShields, spawnPos);
                }

                auto parent = btn->getParent();
                if (parent) {
                    auto pos = btn->getPosition();
                    btn->removeFromParentAndCleanup(true);
                    auto check = CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
                    if (check) {
                        check->setScale(0.6f);
                        check->setPosition(pos);
                        parent->addChild(check);
                    }
                }
            }
        );
    }

    void rebuildList() {
        if (!m_claimMenu) return;
        m_claimMenu->removeAllChildren();

        int maxLevel = 100;
        float itemHeight = 40.f;
        auto contentSize = m_claimMenu->getContentSize();
        float totalHeight = contentSize.height;
        float listWidth = contentSize.width;

        for (int i = 1; i <= maxLevel; i++) {
            float yPos = totalHeight - ((i - 1) * itemHeight) - (itemHeight / 2);
            if (g_streakData.isLevelRewardPending(i)) {
                auto claimSpr = ButtonSprite::create("Claim", "bigFont.fnt", "GJ_button_01.png", 0.6f);
                if (claimSpr) claimSpr->setScale(0.55f);
                auto claimBtn = CCMenuItemSpriteExtra::create(
                    claimSpr, this, menu_selector(XPPopup::onClaimLevelReward)
                );
                claimBtn->setTag(i);
                claimBtn->setPosition({ listWidth - 30.f, yPos });
                m_claimMenu->addChild(claimBtn);
            }
        }
    }

    bool init() override {
        if (!Popup::init(380.f, 260.f, "geode.loader/GE_square01.png")) return false;
        this->setTitle("Level Rewards");

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        auto popupSize = m_mainLayer->getContentSize();
        auto gm = GameManager::sharedState();


        float bottomHeight = 80.f;
        float topMargin = 50.f;

        CCSize listSize = { 340.f, popupSize.height - bottomHeight - topMargin };

        m_listContainer = CCNode::create();
        m_listContainer->setContentSize(popupSize);
        m_listContainer->setPosition({ 0.f, 0.f });
        m_listContainer->setVisible(false);
        m_mainLayer->addChild(m_listContainer);

        m_spinner = StatusSpinner::create();
        if (m_spinner) {
            float listCenter = bottomHeight + (listSize.height / 2) + 5.f;
            m_spinner->setPosition({ popupSize.width / 2.f, listCenter });
            m_mainLayer->addChild(m_spinner, 10);
            m_spinner->setLoading("Loading rewards...");
        }

        refreshPendingLevelRewardsFromServer([this, keepAlive = Ref<CCNode>(this)](bool ok) {
            if (m_spinner) m_spinner->hide();
            if (m_listContainer) m_listContainer->setVisible(true);
            if (!ok) {
                if (m_spinner) m_spinner->setError("Connection failed");
                return;
            }
            this->rebuildList();
        });

        auto listBg = cocos2d::extension::CCScale9Sprite::create("square02_001.png");
        listBg->setContentSize(listSize + CCSize{ 10.f, 10.f });
        listBg->setColor({ 0, 0, 0 });
        listBg->setOpacity(100);

        float listCenterY = bottomHeight + (listSize.height / 2) + 5.f;
        listBg->setPosition({ popupSize.width / 2, listCenterY });
        m_listContainer->addChild(listBg);

        m_list = ScrollLayer::create(listSize);
        m_list->setPosition({ (popupSize.width - listSize.width) / 2, bottomHeight + 5.f });

        int maxLevel = 100;
        float itemHeight = 40.f;
        float totalHeight = std::max(listSize.height, maxLevel * itemHeight);

        m_list->m_contentLayer->setContentSize({ listSize.width, totalHeight });

        m_claimMenu = CCMenu::create();
        m_claimMenu->setPosition({ 0, 0 });
        m_claimMenu->setContentSize({ listSize.width, totalHeight });
        m_list->m_contentLayer->addChild(m_claimMenu, 5);

        for (int i = 1; i <= maxLevel; i++) {
            float yPos = totalHeight - ((i - 1) * itemHeight) - (itemHeight / 2);
            bool isPast = i < g_streakData.currentLevel;
            bool isCurrent = i == g_streakData.currentLevel;

            int opacity = (i % 2 == 0) ? 100 : 50;
            if (isCurrent) opacity = 180;

            auto rowBg = CCLayerColor::create({ 0, 0, 0, (GLubyte)opacity }, listSize.width, itemHeight);
            rowBg->setPosition({ 0.f, yPos - (itemHeight / 2) });
            m_list->m_contentLayer->addChild(rowBg);

            auto lblLvl = CCLabelBMFont::create(fmt::format("Lv.{}", i).c_str(), "goldFont.fnt");
            lblLvl->setScale(0.5f);
            lblLvl->setAnchorPoint({ 0.f, 0.5f });
            lblLvl->setPosition({ 15.f, yPos });
            if (isPast) lblLvl->setColor({ 150, 150, 150 });
            if (isCurrent) lblLvl->setColor({ 255, 255, 0 });
            m_list->m_contentLayer->addChild(lblLvl);

            auto rewards = g_streakData.getRewardsForLevel(i);
            float iconX = 90.f;

            // Niveles multiplos de 10: solo cofre, sin las demas recompensas.
            if (rewards.chestRarity > 0) {
                std::string chestPath = fmt::format("{}/ChestStar{}.png", Mod::get()->getID(), rewards.chestRarity);
                auto chest = CCSprite::create(chestPath.c_str());
                if (!chest) chest = CCSprite::createWithSpriteFrameName("chest_02_02_001.png");
                if (chest) {
                    chest->setScale(0.3f);
                    chest->setPosition({ listSize.width / 2.f, yPos + 4.f });
                    m_list->m_contentLayer->addChild(chest);
                }
            }
            else if (rewards.gems > 0) {

                auto gemIcon = CCSprite::create("gem.png"_spr);
                gemIcon->setScale(0.25f);
                gemIcon->setPosition({ iconX, yPos });
                m_list->m_contentLayer->addChild(gemIcon);

                auto gemTxt = CCLabelBMFont::create(fmt::format("x{}", rewards.gems).c_str(), "bigFont.fnt");
                gemTxt->setScale(0.35f);
                gemTxt->setAnchorPoint({ 0.f, 0.5f });
                gemTxt->setPosition({ iconX + 15.f, yPos });
                m_list->m_contentLayer->addChild(gemTxt);

                iconX += 70.f;
            }

            if (rewards.chestRarity == 0 && rewards.stars > 0) {
                auto starIcon = CCSprite::create("super_star.png"_spr);
                starIcon->setScale(0.25f);
                starIcon->setPosition({ iconX, yPos });
                m_list->m_contentLayer->addChild(starIcon);

                auto starTxt = CCLabelBMFont::create(fmt::format("x{}", rewards.stars).c_str(), "bigFont.fnt");
                starTxt->setScale(0.35f);
                starTxt->setAnchorPoint({ 0.f, 0.5f });
                starTxt->setPosition({ iconX + 15.f, yPos });
                m_list->m_contentLayer->addChild(starTxt);

                iconX += 70.f;
            }

            // 1 vida (shield) por nivel — reemplaza al antiguo ticket. Solo si no es chest.
            if (rewards.chestRarity == 0 && rewards.shields > 0) {
                auto heartIcon = CCSprite::create("heart.png"_spr);
                if (!heartIcon) heartIcon = CCSprite::createWithSpriteFrameName("GJ_heartIcon_001.png");
                if (heartIcon) {
                    heartIcon->setScale(0.22f);
                    heartIcon->setPosition({ iconX, yPos });
                    m_list->m_contentLayer->addChild(heartIcon);

                    auto heartTxt = CCLabelBMFont::create(fmt::format("x{}", rewards.shields).c_str(), "bigFont.fnt");
                    heartTxt->setScale(0.35f);
                    heartTxt->setAnchorPoint({ 0.f, 0.5f });
                    heartTxt->setPosition({ iconX + 15.f, yPos });
                    m_list->m_contentLayer->addChild(heartTxt);
                    iconX += 50.f;
                }
            }
           
            float indicatorX = listSize.width - 30.f;

            if (g_streakData.isLevelRewardPending(i)) {
                auto claimSpr = ButtonSprite::create("Claim", "bigFont.fnt", "GJ_button_01.png", 0.6f);
                if (claimSpr) claimSpr->setScale(0.55f);
                auto claimBtn = CCMenuItemSpriteExtra::create(
                    claimSpr, this, menu_selector(XPPopup::onClaimLevelReward)
                );
                claimBtn->setTag(i);
                claimBtn->setPosition({ indicatorX, yPos });
                m_claimMenu->addChild(claimBtn);
            }
            else if (isPast) {
                auto check = CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
                check->setScale(0.6f);
                check->setPosition({ indicatorX, yPos });
                m_list->m_contentLayer->addChild(check);
            }
            else if (isCurrent) {
              
                auto playerContainer = CCNode::create();
                playerContainer->setPosition({ indicatorX, yPos - 5.f });
                m_list->m_contentLayer->addChild(playerContainer);

               
                float floorWidth = 30.f;
                float floorHeight = 2.f;
                auto floorLine = CCLayerColor::create({ 255, 255, 255, 200 }, floorWidth, floorHeight);

                
                floorLine->setPosition({ -floorWidth / 2, -floorHeight - 6.f });

                playerContainer->addChild(floorLine);
 
                auto playerIcon = SimplePlayer::create(gm->getPlayerFrame());
                playerIcon->updatePlayerFrame(gm->getPlayerFrame(), IconType::Cube);
                playerIcon->setColor(gm->colorForIdx(gm->getPlayerColor()));
                playerIcon->setSecondColor(gm->colorForIdx(gm->getPlayerColor2()));

               
                if (gm->getPlayerGlow()) {
                    playerIcon->setGlowOutline(gm->colorForIdx(gm->getPlayerGlowColor()));
                }
                else {
                    playerIcon->disableGlowOutline();
                }

                playerIcon->updateColors();
                playerIcon->setScale(0.45f);
 
                float scaledHalfHeight = (playerIcon->getContentSize().height * playerIcon->getScale()) / 2;
                playerIcon->setPosition({ 0.f, scaledHalfHeight });

                playerContainer->addChild(playerIcon);

             
                auto jumpAction = CCJumpBy::create(0.7f, { 0.f, 0.f }, 12.f, 1);
                auto delayAction = CCDelayTime::create(0.1f);
                auto sequence = CCSequence::create(jumpAction, delayAction, nullptr);

                playerIcon->runAction(CCRepeatForever::create(sequence));
            }
        }

       
        if (g_streakData.currentLevel > 1) {
            float targetRow = g_streakData.currentLevel - 1;
            float targetY = totalHeight - (targetRow * itemHeight) - (itemHeight / 2);
            float layerY = listSize.height / 2 - targetY;
            float minY = listSize.height - totalHeight;
            if (layerY < minY) layerY = minY;
            if (layerY > 0) layerY = 0;
            m_list->m_contentLayer->setPositionY(layerY);
        }
        else {
            m_list->moveToTop();
        }
        m_listContainer->addChild(m_list);




        float centerX = popupSize.width / 2;
        float barWidth = 220.0f;
        float barHeight = 18.0f;
        float barCenterY = bottomHeight / 2 + 2.f;

        float percentage = g_streakData.getXPPercentage();

        auto xpBar = RoundedProgressBar::create(barWidth, barHeight);
        xpBar->setPosition({ centerX, barCenterY });
        xpBar->setGradientColors({ 0, 255, 255 }, { 0, 100, 255 });
        xpBar->setBackgroundColor({ 20, 20, 40 });
        xpBar->setProgress(percentage);
        m_mainLayer->addChild(xpBar, 1);

        auto xpText = CCLabelBMFont::create(
            fmt::format("{} / {} ({:.1f}%)",
                g_streakData.currentXP,
                g_streakData.getXPRequiredForNextLevel(),
                percentage * 100.0f
            ).c_str(),
            "bigFont.fnt"
        );
        xpText->setPosition({ centerX, barCenterY });
        xpText->setScale(0.4f);
        xpText->setZOrder(10);
        m_mainLayer->addChild(xpText, 8);

        auto lvlTxt = CCLabelBMFont::create(fmt::format("Current: Level {}", g_streakData.currentLevel).c_str(), "goldFont.fnt");
        lvlTxt->setScale(0.5f);
        lvlTxt->setPosition({ centerX, barCenterY + barHeight + 12.f });
        m_mainLayer->addChild(lvlTxt);

        return true;
    }

public:
    static XPPopup* create() {
        auto ret = new XPPopup();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};