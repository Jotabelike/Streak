#pragma once
#include <Geode/ui/Popup.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/utils/async.hpp>
#include "../StreakData.h"
#include "../RewardNotification.h"
#include "../StatusSpinner.h"

using namespace geode::prelude;

struct TrendLevelDef {
    int levelID;
    std::string name;
    std::string creator;
    int superStars;
    int starTickets;
    int gems;
    std::string difficultySprite; 
    bool isClaimed;
};

class TrendLevelCell : public CCLayerColor {
protected:
    TrendLevelDef m_data;
    std::function<void()> m_reloadFunc;
    async::TaskHolder<web::WebResponse> m_claimTask;

    bool init(const TrendLevelDef& data, float width, std::function<void()> reloadCallback) {
        if (!CCLayerColor::init()) return false;
        m_data = data;
        m_reloadFunc = reloadCallback;

        float height = 90.0f;
        this->setContentSize({ width, height });
        this->setAnchorPoint({ 0, 0 });

        auto bg = cocos2d::extension::CCScale9Sprite::create("square02_small.png");
        bg->setContentSize({ width, height - 2.0f });
        bg->setOpacity(70);
        bg->setPosition({ width / 2, height / 2 });
        this->addChild(bg, 0);

        auto glm = GameLevelManager::sharedState();
        auto level = glm->getSavedLevel(data.levelID);
        int percent = level ? level->m_normalPercent : 0;
        bool isCompleted = (percent >= 100);

       
        auto face = CCSprite::createWithSpriteFrameName(m_data.difficultySprite.c_str());
        if (!face) {
          
            face = CCSprite::createWithSpriteFrameName("difficulty_00_btn_001.png");
        }

        if (face) {
            face->setPosition({ 35.f, height / 2 + 5.f });
            face->setScale(1.15f);
            this->addChild(face, 1);
        }

      
        auto title = CCLabelBMFont::create(data.name.c_str(), "goldFont.fnt");
        title->setScale(0.65f);
        title->setAnchorPoint({ 0, 0.5f });
        title->setPosition({ 75.0f, height - 20.0f });

        float maxTitleW = width - 150.0f;
        if (title->getScaledContentSize().width > maxTitleW) {
            title->setScale(title->getScale() * (maxTitleW / title->getScaledContentSize().width));
        }
        this->addChild(title, 1);

        auto creator = CCLabelBMFont::create(fmt::format("By {}", data.creator).c_str(), "chatFont.fnt");
        creator->setScale(0.45f);
        creator->setAnchorPoint({ 0, 0.5f });
        creator->setColor({ 200, 200, 200 });
        creator->setPosition({ 75.0f, height - 35.0f });
        this->addChild(creator, 1);

         
        float barW = 140.0f;
        float barH = 14.0f;
        float barX = 75.0f;
        float barY = height / 2 - 12.f;

       
        auto barBorder = CCLayerColor::create({ 255, 255, 255, 80 }, barW + 2.f, barH + 2.f);
        barBorder->setPosition({ barX - 1.f, barY - 1.f });
        this->addChild(barBorder, 0);

       
        auto barBg = CCLayerColor::create({ 0, 0, 0, 200 }, barW, barH);
        barBg->setPosition({ barX, barY });
        this->addChild(barBg, 1);

      
        if (percent > 0) {
            ccColor4B fillColor = isCompleted ? ccColor4B{ 50, 255, 50, 255 } : ccColor4B{ 50, 150, 255, 255 };
            float fillW = barW * (std::min(percent, 100) / 100.0f);
            auto barFg = CCLayerColor::create(fillColor, fillW, barH);
            barFg->setPosition({ barX, barY });
            this->addChild(barFg, 2);
        }

        
        auto pctLbl = CCLabelBMFont::create(fmt::format("{}%", percent).c_str(), "bigFont.fnt");
        pctLbl->setScale(0.32f);
        pctLbl->setAnchorPoint({ 0.5f, 0.5f });
        pctLbl->setPosition({ barX + barW / 2, barY + barH / 2 + 0.5f });
        this->addChild(pctLbl, 5);

       
        float rewardY = 15.0f;
        float rewardX = 75.0f;
        float gap = 60.0f;

        if (data.superStars > 0) {
            auto starIcon = CCSprite::create("super_star.png"_spr);
            if (!starIcon) starIcon = CCSprite::createWithSpriteFrameName("GJ_starsIcon_001.png");
            starIcon->setScale(0.18f);
            starIcon->setPosition({ rewardX + 5.f, rewardY });
            this->addChild(starIcon);

            auto starLbl = CCLabelBMFont::create(fmt::format("+{}", data.superStars).c_str(), "bigFont.fnt");
            starLbl->setScale(0.3f);
            starLbl->setAnchorPoint({ 0, 0.5f });
            starLbl->setPosition({ rewardX + 15.f, rewardY });
            this->addChild(starLbl);

            rewardX += gap;
        }

        if (data.starTickets > 0) {
            auto ticketIcon = CCSprite::create("star_tiket.png"_spr);
            if (!ticketIcon) ticketIcon = CCSprite::createWithSpriteFrameName("GJ_pointsIcon_001.png");
            ticketIcon->setScale(0.18f);
            ticketIcon->setPosition({ rewardX + 5.f, rewardY });
            this->addChild(ticketIcon);

            auto ticketLbl = CCLabelBMFont::create(fmt::format("+{}", data.starTickets).c_str(), "bigFont.fnt");
            ticketLbl->setScale(0.3f);
            ticketLbl->setAnchorPoint({ 0, 0.5f });
            ticketLbl->setPosition({ rewardX + 15.f, rewardY });
            this->addChild(ticketLbl);

            rewardX += gap;
        }

        if (data.gems > 0) {
            auto gemIcon = CCSprite::create("gem.png"_spr);
            if (!gemIcon) gemIcon = CCSprite::createWithSpriteFrameName("GJ_diamondsIcon_001.png");
            gemIcon->setScale(0.18f);  
            gemIcon->setPosition({ rewardX + 5.f, rewardY });
            this->addChild(gemIcon);

            auto gemLbl = CCLabelBMFont::create(fmt::format("+{}", data.gems).c_str(), "bigFont.fnt");
            gemLbl->setScale(0.3f);
            gemLbl->setAnchorPoint({ 0, 0.5f });
            gemLbl->setPosition({ rewardX + 15.f, rewardY });
            this->addChild(gemLbl);
        }

      
        auto menu = CCMenu::create();
        menu->setPosition({ 0, 0 });
        this->addChild(menu, 2);

        CCPoint actionBtnPos = { width - 40.0f, height / 2 };

        if (data.isClaimed) {
            auto check = CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
            check->setScale(0.8f);
            check->setPosition(actionBtnPos);
            this->addChild(check);
        }
        else if (isCompleted) {
            auto btnSpr = ButtonSprite::create("Claim", 40, true, "goldFont.fnt", "GJ_button_02.png", 30.0f, 0.6f);
            auto btn = CCMenuItemSpriteExtra::create(btnSpr, this, menu_selector(TrendLevelCell::onClaimClick));
            btn->setPosition(actionBtnPos);
            menu->addChild(btn);
        }
        else {
         
            auto playSpr = CCSprite::createWithSpriteFrameName("GJ_playBtn2_001.png");
            playSpr->setScale(0.70);
            auto playBtn = CCMenuItemSpriteExtra::create(playSpr, this, menu_selector(TrendLevelCell::onPlayClick));
            playBtn->setPosition(actionBtnPos);
            menu->addChild(playBtn);
        }

        return true;
    }

    void onPlayClick(CCObject*) {
        auto searchObj = GJSearchObject::create(SearchType::Search, std::to_string(m_data.levelID));
        auto browserLayer = LevelBrowserLayer::scene(searchObj);
        auto transition = CCTransitionFade::create(0.5f, browserLayer);
        CCDirector::sharedDirector()->replaceScene(transition);
    }

    void onClaimClick(CCObject* sender) {
        auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
        btn->setEnabled(false);

        auto am = GJAccountManager::sharedState();
        matjson::Value body = matjson::Value::object();
        body.set("accountID", am->m_accountID);
        body.set("levelID", m_data.levelID);

        auto req = web::WebRequest();
        req.bodyJSON(body);

        m_claimTask.spawn(
            req.post("https://streak-servidor.onrender.com/trending-level/claim"),
            [this](web::WebResponse res) {
                if (res.ok()) {
                    if (m_data.superStars > 0) {
                        int startStars = g_streakData.superStars;
                        g_streakData.superStars += m_data.superStars;
                        RewardNotification::show("super_star.png"_spr, startStars, m_data.superStars);
                    }
                    if (m_data.starTickets > 0) {
                        int startTickets = g_streakData.starTickets;
                        g_streakData.starTickets += m_data.starTickets;
                        RewardNotification::show("star_tiket.png"_spr, startTickets, m_data.starTickets);
                    }
                    if (m_data.gems > 0) {
                        int startGems = g_streakData.gems;
                        g_streakData.gems += m_data.gems;
                        RewardNotification::show("gem.png"_spr, startGems, m_data.gems);
                    }

                    g_streakData.save();
                    FMODAudioEngine::sharedEngine()->playEffect("buyItem01.ogg");

                    if (m_reloadFunc) m_reloadFunc();
                }
                else {
                    FLAlertLayer::create("Error", "Failed to claim reward.", "OK")->show();
                }
            }
        );
    }

public:
    static TrendLevelCell* create(const TrendLevelDef& data, float width, std::function<void()> reloadCallback) {
        auto ret = new TrendLevelCell();
        if (ret && ret->init(data, width, reloadCallback)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

class TrendLevelsPopup : public Popup {
protected:
    StatusSpinner* m_spinner = nullptr;
    CCNode* m_contentNode = nullptr;
    async::TaskHolder<web::WebResponse> m_fetchTask;

    bool init() {
        if (!Popup::init(350.f, 180.f, "geode.loader/GE_square03.png")) return false;      
        auto titleSprite = CCSprite::create("trend_title.png"_spr);
        if (titleSprite) {      
            titleSprite->setPosition({ m_size.width / 2, m_size.height - 15.f });
            m_mainLayer->addChild(titleSprite, 1);
        }

        m_spinner = StatusSpinner::create();
        m_spinner->setPosition(m_size / 2);
        m_mainLayer->addChild(m_spinner, 10);

        fetchData();

        return true;
    }



    void fetchData() {
        if (m_contentNode) {
            m_contentNode->removeFromParent();
            m_contentNode = nullptr;
        }

        m_spinner->setLoading("Fetching level...");

        auto am = GJAccountManager::sharedState();
        std::string url = fmt::format("https://streak-servidor.onrender.com/trending-level/{}", am->m_accountID);

        auto req = web::WebRequest();
        m_fetchTask.spawn(
            req.get(url),
            [this](web::WebResponse res) {
                if (res.ok() && res.json().isOk()) {
                    m_spinner->hide();
                    buildLevel(res.json().unwrap());
                }
                else {
                    m_spinner->setError("Failed to load level");
                }
            }
        );
    }

    void buildLevel(const matjson::Value& data) {
        m_contentNode = CCNode::create();
        m_mainLayer->addChild(m_contentNode);

        if (data.isNull()) {
            auto emptyLbl = CCLabelBMFont::create("No trending level right now.", "chatFont.fnt");
            emptyLbl->setPosition(m_size / 2);
            emptyLbl->setColor({ 200, 200, 200 });
            m_contentNode->addChild(emptyLbl);
            return;
        }

        TrendLevelDef def;
        def.levelID = data["levelID"].as<int>().unwrapOr(0);
        def.name = data["name"].as<std::string>().unwrapOr(std::string("Unknown"));
        def.creator = data["creator"].as<std::string>().unwrapOr(std::string("-"));
        def.difficultySprite = data["difficultySprite"].as<std::string>().unwrapOr(std::string("difficulty_00_btn_001.png")); 
        def.isClaimed = data["isClaimed"].as<bool>().unwrapOr(false);

        if (data.contains("rewards")) {
            def.superStars = data["rewards"]["super_stars"].as<int>().unwrapOr(0);
            def.starTickets = data["rewards"]["star_tickets"].as<int>().unwrapOr(0);
            def.gems = data["rewards"]["gems"].as<int>().unwrapOr(0);
        }
        else {
            def.superStars = 0;
            def.starTickets = 0;
            def.gems = 0;
        }

        float cellWidth = 320.0f;
        auto cell = TrendLevelCell::create(def, cellWidth, [this]() {
            this->fetchData();
            });

        cell->setPosition({ (m_size.width - cellWidth) / 2, (m_size.height - 90.f) / 2 - 10.f });
        m_contentNode->addChild(cell);
    }

public:
    static TrendLevelsPopup* create() {
        auto ret = new TrendLevelsPopup();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};