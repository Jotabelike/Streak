#pragma once
#include "StreakCommon.h"
#include "../StreakData.h"
#include "../NameModifiers.h"
#include "../utils/ScrollbarUtils.h"
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/ScrollLayer.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/utils/async.hpp>
#include <Geode/binding/GJAccountManager.hpp>
#include <Geode/binding/ProfilePage.hpp>
#include <Geode/binding/FLAlertLayer.hpp>
#include <matjson.hpp>
#include "../StatusSpinner.h"
#include "../RewardNotification.h"

using namespace geode::prelude;

class LeaderboardCell : public CCLayer {
protected:
    matjson::Value m_playerData;
    CCLabelBMFont* m_nameLabel = nullptr;
    std::string m_nameAnimation;
    std::string m_nameEffect;

    bool init(matjson::Value playerData, int rank, bool isLocalPlayer, bool showHighlight = true) {
        if (!CCLayer::init()) return false;
        this->m_playerData = playerData;

        const float cellWidth = 340.f;
        const float cellHeight = 40.f;
        this->setContentSize({ cellWidth, cellHeight });

        auto bg = CCLayerColor::create(
            { 0, 0, 0, 80 },
            cellWidth,
            cellHeight - 2
        );
        bg->setPosition({ 0, 1 });
        this->addChild(bg, -2);

        std::string bannerID = playerData["equipped_banner_id"].as<std::string>().unwrapOr("");
        if (isLocalPlayer) {
            bannerID = g_streakData.equippedBanner;
        }

        StreakData::BannerInfo* bannerInfo = nullptr;
        if (!bannerID.empty()) {
            bannerInfo = g_streakData.getBannerInfo(bannerID);
        }

        if (bannerInfo) {
            auto bannerSprite = CCSprite::create(bannerInfo->spriteName.c_str());
            if (bannerSprite) {
                float scaleX = cellWidth / bannerSprite->getContentSize().width;
                float targetHeight = cellHeight - 2.0f;
                float scaleY = targetHeight / bannerSprite->getContentSize().height;
                bannerSprite->setScaleX(scaleX);
                bannerSprite->setScaleY(scaleY);
                bannerSprite->setPosition({ cellWidth / 2, cellHeight / 2 });
                this->addChild(bannerSprite, -1);
            }
        }

        if (rank >= 1 && rank <= 3) {
            std::string spriteName = fmt::format("top{}.png"_spr, rank);
            auto rankSprite = CCSprite::create(spriteName.c_str());
            if (rankSprite) {
                rankSprite->setScale(0.35f);
                rankSprite->setPosition({ 25.f, cellHeight / 2 });
                this->addChild(rankSprite, 1);

                if (rank == 1) {
                    auto glow = CCSprite::createWithSpriteFrameName("shineBurst_001.png");
                    if (glow) {
                        glow->setColor({ 255, 200, 0 });
                        glow->setScale(3.0f);
                        glow->setOpacity(150);
                        glow->setPosition(rankSprite->getContentSize() / 2);
                        glow->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });
                        glow->runAction(CCRepeatForever::create(
                            CCRotateBy::create(4.0f, 360.f)
                        ));
                        rankSprite->addChild(glow, -1);
                    }
                }
            }
            else {
                auto rLabel = CCLabelBMFont::create(fmt::format("#{}", rank).c_str(), "goldFont.fnt");
                rLabel->setPosition({ 25.f, cellHeight / 2 });
                rLabel->setScale(0.7f);
                this->addChild(rLabel, 1);
            }
        }
        else {
            std::string rankTxt = (rank > 0) ? fmt::format("#{}", rank) : "-";
            auto rankLabel = CCLabelBMFont::create(rankTxt.c_str(), "goldFont.fnt");
            rankLabel->setScale(rank > 999 ? 0.5f : 0.7f);
            rankLabel->limitLabelWidth(45.f, rankLabel->getScale(), 0.1f);
            rankLabel->setPosition({ 25.f, cellHeight / 2 });
            this->addChild(rankLabel, 1);
        }

        std::string badgeID = playerData["equipped_badge_id"].as<std::string>().unwrapOr("");

        if (!badgeID.empty()) {
            if (auto badgeInfo = g_streakData.getBadgeInfo(badgeID)) {
                auto badgeSprite = CCSprite::create(badgeInfo->spriteName.c_str());
                if (badgeSprite) {
                    badgeSprite->setScale(0.15f);
                    badgeSprite->setPosition({ 60.f, cellHeight / 2 });
                    this->addChild(badgeSprite, 1);
                }
            }
        }

        std::string username = playerData["username"].as<std::string>().unwrapOr("-");

        std::string nameColor = playerData["equipped_name_color"].as<std::string>().unwrapOr("Default");
        std::string nameFont = playerData["equipped_name_font"].as<std::string>().unwrapOr("Default");
        m_nameEffect = playerData["equipped_name_effect"].as<std::string>().unwrapOr("None");
        m_nameAnimation = playerData["equipped_name_animation"].as<std::string>().unwrapOr("None");

        if (isLocalPlayer) {
            nameColor = g_streakData.equippedNameColor;
            nameFont = g_streakData.equippedNameFont;
            m_nameEffect = g_streakData.equippedNameEffect;
            m_nameAnimation = g_streakData.equippedNameAnimation;
        }

        m_nameLabel = CCLabelBMFont::create(username.c_str(), "goldFont.fnt");
        m_nameLabel->limitLabelWidth(105.f, 0.6f, 0.1f);
        m_nameLabel->setColor({ 255, 255, 255 });

        NameModifiers::applyColor(m_nameLabel, nameColor);
        NameModifiers::applyFont(m_nameLabel, nameFont);

        auto usernameBtn = CCMenuItemSpriteExtra::create(
            m_nameLabel,
            this,
            menu_selector(LeaderboardCell::onViewProfile)
        );
        usernameBtn->setAnchorPoint({ 0.f, 0.5f });
        usernameBtn->setPosition({ 85.f, cellHeight / 2 + 7.f });

        auto menu = CCMenu::createWithItem(usernameBtn);
        menu->setPosition({ 0, 0 });
        this->addChild(menu, 1);

        NameModifiers::applyEffect(m_nameLabel, m_nameEffect);
        NameModifiers::applyAnimation(m_nameLabel, m_nameAnimation);

        int playerLevel = playerData["current_level"].as<int>().unwrapOr(1);
        auto levelNode = CCNode::create();
        levelNode->setAnchorPoint({ 0.f, 0.5f });
        levelNode->setPosition({ 85.f, cellHeight / 2 - 9.f });
        this->addChild(levelNode, 1);

        float cursorX = 0.f;
        auto xpIcon = CCSprite::create("xp.png"_spr);
        if (!xpIcon) xpIcon = CCSprite::createWithSpriteFrameName("GJ_starsIcon_001.png");

        if (xpIcon) {
            xpIcon->setScale(0.10f);
            xpIcon->setAnchorPoint({ 0.f, 0.5f });
            xpIcon->setPosition({ cursorX, 0.f });
            levelNode->addChild(xpIcon);
            cursorX += (xpIcon->getContentSize().width * xpIcon->getScaleX()) + 5.f;
        }

        auto levelLabel = CCLabelBMFont::create(fmt::format("Lvl {}", playerLevel).c_str(), "bigFont.fnt");
        levelLabel->setScale(0.3f);
        levelLabel->setColor({ 0, 255, 255 });
        levelLabel->setAnchorPoint({ 0.f, 0.5f });
        levelLabel->setPosition({ cursorX, 0.f });
        levelNode->addChild(levelLabel);

        int streakDays = playerData["current_streak_days"].as<int>().unwrapOr(0);
        auto streakIcon = CCSprite::create(g_streakData.getRachaSprite(streakDays).c_str());
        if (!streakIcon) streakIcon = CCSprite::create("racha0.png"_spr);

        streakIcon->setScale(0.15f);
        streakIcon->setPosition({ cellWidth - 80.f, cellHeight / 2 });
        this->addChild(streakIcon, 1);

        auto streakLabel = CCLabelBMFont::create(std::to_string(streakDays).c_str(), "bigFont.fnt");
        streakLabel->setScale(0.35f);
        streakLabel->setAnchorPoint({ 0.f, 0.5f });
        streakLabel->setPosition({ streakIcon->getPositionX() + 12.f, cellHeight / 2 });
        this->addChild(streakLabel, 1);

        int streakPoints = playerData["total_streak_points"].as<int>().unwrapOr(0);
        auto pointIcon = CCSprite::create("streak_point.png"_spr);
        pointIcon->setScale(0.12f);
        pointIcon->setPosition({ cellWidth - 30.f, cellHeight / 2 + 8.f });
        this->addChild(pointIcon, 1);

        auto pointsLabel = CCLabelBMFont::create(std::to_string(streakPoints).c_str(), "goldFont.fnt");
        pointsLabel->setScale(0.3f);
        pointsLabel->setAnchorPoint({ 0.5f, 0.5f });
        pointsLabel->setPosition({ cellWidth - 30.f, cellHeight / 2 - 8.f });
        this->addChild(pointsLabel, 1);

        return true;
    }

    void onViewProfile(CCObject*) {
        int accountID = m_playerData["accountID"].as<int>().unwrapOr(0);
        if (accountID != 0) {
            ProfilePage::create(accountID, false)->show();
        }
    }

public:
    static LeaderboardCell* create(matjson::Value playerData, int rank, bool isLocalPlayer, bool showHighlight = true) {
        auto ret = new LeaderboardCell();
        if (ret && ret->init(playerData, rank, isLocalPlayer, showHighlight)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

class LeaderboardPopup : public Popup {
protected:
    struct Fields {
        async::TaskHolder<web::WebResponse> m_leaderboardListener;
        async::TaskHolder<web::WebResponse> m_claimListener;
        StatusSpinner* m_spinner = nullptr;
        ScrollLayer* m_scrollLayer = nullptr;

        CCSprite* m_myRankIconBg = nullptr;
        CCLabelBMFont* m_myRankNumLabel = nullptr;

        CCLabelBMFont* m_timerLabel = nullptr;
        CCMenuItemSpriteExtra* m_claimBtn = nullptr;
        long long m_seasonEndTimestamp = 0;
    };

    Fields m_fields;

    void onRewardsInfo(CCObject*) {
        std::string rewardText =
            "<cg>Monthly Season Rewards:</c>\n\n"
            "<cy>Rank #1:</c> 100 Gems, 250 Super Stars, 25k Tickets\n"
            "<co>Rank #2:</c> 40 Gems, 100 Super Stars, 15k Tickets\n"
            "<cr>Rank #3:</c> 25 Gems, 60 Super Stars, 5k Tickets\n\n"
            "<cl>Important:</c> Rewards must be claimed within <cr>24 hours</c> after the season ends!";

        FLAlertLayer::create("Season Rewards", rewardText, "OK")->show();
    }

    void refreshTimer(float dt) {
        if (!this->m_fields.m_timerLabel || this->m_fields.m_seasonEndTimestamp == 0) return;

        auto now = std::chrono::system_clock::now();
        auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
        long long diff = this->m_fields.m_seasonEndTimestamp - now_ms;

        if (diff <= 0) {
            this->m_fields.m_timerLabel->setString("Season Ended!");
            this->m_fields.m_timerLabel->setColor({ 255, 100, 100 });
            return;
        }

        long long seconds = diff / 1000;
        long long minutes = seconds / 60;
        long long hours = minutes / 60;
        long long days = hours / 24;

        std::string timeStr;
        if (days > 0) {
            timeStr = fmt::format("{} Days Left", days);
        }
        else if (hours > 0) {
            timeStr = fmt::format("{}h {}m Left", hours % 24, minutes % 60);
        }
        else {
            timeStr = fmt::format("{}m {}s Left", minutes % 60, seconds % 60);
        }

        this->m_fields.m_timerLabel->setString(timeStr.c_str());
    }

    void onClaimSeasonReward(CCObject* sender) {
        auto am = GJAccountManager::sharedState();
        if (!am || am->m_accountID == 0) return;

        CCPoint spawnPos = CCDirector::sharedDirector()->getWinSize() / 2;
        if (auto btn = static_cast<CCNode*>(sender)) {
            spawnPos = btn->convertToWorldSpaceAR(CCPointZero);
        }

        this->m_fields.m_claimBtn->setVisible(false);
        this->m_fields.m_spinner->setVisible(true);

        std::string url = "https://streak-servidor.onrender.com/season/claim";
        matjson::Value body = matjson::Value::object();
        body.set("accountID", am->m_accountID);

        auto req = web::WebRequest();

        this->m_fields.m_claimListener.spawn(req.bodyJSON(body).post(url), [this, spawnPos](web::WebResponse res) {
            this->m_fields.m_spinner->setVisible(false);
            if (res.ok() && res.json().isOk()) {
                auto data = res.json().unwrap();
                if (data["success"].as<bool>().unwrapOr(false)) {
                    auto reward = data["reward"];
                    int gems = reward["gems"].as<int>().unwrapOr(0);
                    int stars = reward["super_stars"].as<int>().unwrapOr(0);
                    int tickets = reward["star_tickets"].as<int>().unwrapOr(0);
                    int rank = data["rank"].as<int>().unwrapOr(0);

                    g_streakData.gems += gems;
                    g_streakData.superStars += stars;
                    g_streakData.starTickets += tickets;
                    g_streakData.pendingSeasonRank = 0;
                    g_streakData.save();

                    Notification::create(fmt::format("Season Reward Rank #{}", rank), NotificationIcon::Success)->show();

                    if (gems > 0) RewardNotification::show("gem.png"_spr, g_streakData.gems - gems, gems, spawnPos);
                    if (stars > 0) RewardNotification::show("super_star.png"_spr, g_streakData.superStars - stars, stars, spawnPos);
                    if (tickets > 0) RewardNotification::show("star_tiket.png"_spr, g_streakData.starTickets - tickets, tickets, spawnPos);

                    FMODAudioEngine::sharedEngine()->playEffect("chest07.mp3");
                }
            }
            else {
                this->m_fields.m_claimBtn->setVisible(true);
                Notification::create("Error claiming reward (Expired?)", NotificationIcon::Error)->show();
            }
            });
    }

    void handleLeaderboardResponse(web::WebResponse res) {
        if (res.ok() && res.json().isOk()) {
            if (this->m_fields.m_spinner) {
                this->m_fields.m_spinner->hide();
            }

            auto data = res.json().unwrap();

            if (data.contains("seasonEnd")) {
                double endVal = data["seasonEnd"].as<double>().unwrapOr(0.0);
                this->m_fields.m_seasonEndTimestamp = (long long)endVal;
                this->schedule(schedule_selector(LeaderboardPopup::refreshTimer), 1.0f);
                this->refreshTimer(0);
            }

            if (data.contains("list") && data["list"].isArray()) {
                auto playersVec = data["list"].as<std::vector<matjson::Value>>().unwrap();
                this->populateScroll(playersVec);
                this->updateMyRankUI(playersVec);
            }
        }
        else {
            if (this->m_fields.m_spinner) {
                this->m_fields.m_spinner->setError("Failed to load.");
            }
        }
    }

    void populateScroll(const std::vector<matjson::Value>& players) {
        if (!this->m_fields.m_scrollLayer) return;

        auto contentLayer = this->m_fields.m_scrollLayer->m_contentLayer;
        contentLayer->removeAllChildren();

        int capacityIdx = Mod::get()->getSavedValue<int>("leaderboard_capacity_idx", 0);
        size_t limit = (capacityIdx == 0) ? 10 : 50;
        size_t countToShow = std::min(players.size(), limit);

        float cellHeight = 42.f;
        float totalHeight = std::max(
            this->m_fields.m_scrollLayer->getContentSize().height,
            countToShow * cellHeight
        );

        contentLayer->setContentSize({ 340.f, totalHeight });

        int localAccountID = GJAccountManager::sharedState()->m_accountID;

        for (size_t i = 0; i < countToShow; ++i) {
            int rank = i + 1;
            const auto& playerJson = players[i];

            int playerAccountID = 0;
            if (playerJson["accountID"].isNumber()) {
                playerAccountID = playerJson["accountID"].as<int>().unwrapOr(0);
            }
            else if (playerJson["accountID"].isString()) {
                try {
                    playerAccountID = std::stoi(playerJson["accountID"].as<std::string>().unwrapOr("0"));
                }
                catch (...) {}
            }

            bool isMe = (localAccountID != 0 && playerAccountID == localAccountID);

            auto cell = LeaderboardCell::create(playerJson, rank, isMe);
            float yPos = totalHeight - (i * cellHeight) - (cellHeight / 2);
            cell->setPosition(0, yPos - (cellHeight / 2));

            contentLayer->addChild(cell);
        }

        contentLayer->setPositionY(
            this->m_fields.m_scrollLayer->getContentSize().height - totalHeight
        );

        addScrollbar(this->m_fields.m_scrollLayer, 8.f, this->m_mainLayer);
    }

    void updateMyRankUI(const std::vector<matjson::Value>& players) {
        int myRank = -1;
        int localAccountID = GJAccountManager::sharedState()->m_accountID;

        if (localAccountID != 0) {
            for (size_t i = 0; i < players.size(); ++i) {
                int playerID = 0;
                if (players[i]["accountID"].isNumber()) {
                    playerID = players[i]["accountID"].as<int>().unwrapOr(0);
                }
                else if (players[i]["accountID"].isString()) {
                    try {
                        playerID = std::stoi(players[i]["accountID"].as<std::string>().unwrapOr("0"));
                    }
                    catch (...) {}
                }
                if (playerID == localAccountID) {
                    myRank = static_cast<int>(i) + 1;
                    break;
                }
            }
        }

        if (this->m_fields.m_myRankIconBg && this->m_fields.m_myRankNumLabel) {
            this->m_fields.m_myRankIconBg->setVisible(true);
            this->m_fields.m_myRankNumLabel->setVisible(true);

            if (myRank != -1) {
                this->m_fields.m_myRankNumLabel->setString(std::to_string(myRank).c_str());
                this->m_fields.m_myRankNumLabel->setColor((myRank <= 3) ? ccColor3B{ 255, 215, 0 } : ccColor3B{ 255, 255, 255 });
                this->m_fields.m_myRankNumLabel->setScale(myRank > 99 ? 0.4f : 0.5f);
            }
            else {
                this->m_fields.m_myRankNumLabel->setString("-");
                this->m_fields.m_myRankNumLabel->setColor({ 150, 150, 150 });
                this->m_fields.m_myRankNumLabel->setScale(0.5f);
            }
        }

        if (auto idLabel = typeinfo_cast<CCLabelBMFont*>(m_mainLayer->getChildByID("streak-id-label"))) {
            if (!g_streakData.streakID.empty()) {
                idLabel->setString(
                    fmt::format("ID: {}", g_streakData.streakID).c_str()
                );
            }
        }
    }

    bool init() override {
        if (!Popup::init(390.f, 280.f, "geode.loader/GE_square03.png")) return false;

        this->setTitle("Top Streaks");
        auto winSize = this->m_mainLayer->getContentSize();

        auto infoSpr = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
        infoSpr->setScale(0.7f);
        auto infoBtn = CCMenuItemSpriteExtra::create(
            infoSpr,
            this,
            menu_selector(LeaderboardPopup::onRewardsInfo)
        );
        auto infoMenu = CCMenu::createWithItem(infoBtn);
        infoMenu->setPosition({ 25.f, winSize.height - 25.f });
        m_mainLayer->addChild(infoMenu);

        this->m_fields.m_myRankIconBg = CCSprite::createWithSpriteFrameName("rankIcon_1_001.png");
        this->m_fields.m_myRankIconBg->setPosition({ winSize.width - 55.f, winSize.height - 25.f });
        this->m_fields.m_myRankIconBg->setScale(0.8f);
        this->m_fields.m_myRankIconBg->setVisible(false);
        m_mainLayer->addChild(this->m_fields.m_myRankIconBg);

        this->m_fields.m_myRankNumLabel = CCLabelBMFont::create("?", "bigFont.fnt");
        this->m_fields.m_myRankNumLabel->setAnchorPoint({ 0.f, 0.5f });
        this->m_fields.m_myRankNumLabel->setPosition({ winSize.width - 40.f, winSize.height - 25.f });
        this->m_fields.m_myRankNumLabel->setScale(0.5f);
        this->m_fields.m_myRankNumLabel->setVisible(false);
        m_mainLayer->addChild(this->m_fields.m_myRankNumLabel);

        auto listBg = cocos2d::extension::CCScale9Sprite::create("square02_001.png");
        listBg->setColor({ 0, 0, 0 });
        listBg->setOpacity(120);
        CCSize listSize = { 340.f, 190.f };
        listBg->setContentSize(listSize + CCSize{ 10.f, 10.f });
        listBg->setPosition({ winSize.width / 2, winSize.height / 2 - 15.f });
        this->m_mainLayer->addChild(listBg);

        this->m_fields.m_scrollLayer = ScrollLayer::create(listSize);
        this->m_fields.m_scrollLayer->setPosition({
            (winSize.width - listSize.width) / 2,
            (winSize.height - listSize.height) / 2 - 15.f
            });
        this->m_mainLayer->addChild(this->m_fields.m_scrollLayer);

        this->m_fields.m_timerLabel = CCLabelBMFont::create("Loading Season...", "goldFont.fnt");
        this->m_fields.m_timerLabel->setScale(0.55f);
        this->m_fields.m_timerLabel->setPosition({ winSize.width / 2, winSize.height - 45.f });
        this->m_fields.m_timerLabel->setColor({ 0, 255, 255 });
        m_mainLayer->addChild(this->m_fields.m_timerLabel);

        auto claimSpr = ButtonSprite::create("Claim Reward!", "goldFont.fnt", "GJ_button_01.png", .8f);
        this->m_fields.m_claimBtn = CCMenuItemSpriteExtra::create(
            claimSpr,
            this,
            menu_selector(LeaderboardPopup::onClaimSeasonReward)
        );
        auto btnMenu = CCMenu::createWithItem(this->m_fields.m_claimBtn);
        btnMenu->setPosition({ winSize.width / 2, 25.f });
        m_mainLayer->addChild(btnMenu, 10);

        if (g_streakData.pendingSeasonRank > 0) {
            this->m_fields.m_claimBtn->setVisible(true);
            this->m_fields.m_claimBtn->runAction(CCRepeatForever::create(CCSequence::create(
                CCScaleTo::create(0.5f, 1.1f),
                CCScaleTo::create(0.5f, 1.0f),
                nullptr
            )));
        }
        else {
            this->m_fields.m_claimBtn->setVisible(false);
        }

        std::string idText = g_streakData.streakID.empty() ? "Loading ID..." : g_streakData.streakID;
        auto idLabel = CCLabelBMFont::create(fmt::format("ID: {}", idText).c_str(), "chatFont.fnt");
        idLabel->setScale(0.45f);
        idLabel->setColor({ 150, 150, 150 });
        idLabel->setAnchorPoint({ 0.f, 0.5f });
        idLabel->setPosition({ 22.f, 18.f });
        idLabel->setID("streak-id-label");
        m_mainLayer->addChild(idLabel);

        this->m_fields.m_spinner = StatusSpinner::create();
        this->m_fields.m_spinner->setPosition(winSize / 2);
        this->m_mainLayer->addChild(this->m_fields.m_spinner, 100);
        this->m_fields.m_spinner->setLoading("Loading...");

        auto req = web::WebRequest();
        this->m_fields.m_leaderboardListener.spawn(
            req.get("https://streak-servidor.onrender.com/leaderboard"),
            [this](web::WebResponse res) {
                this->handleLeaderboardResponse(std::move(res));
            }
        );

        return true;
    }

public:
    static LeaderboardPopup* create() {
        auto ret = new LeaderboardPopup();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};