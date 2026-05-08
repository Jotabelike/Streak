#pragma once
#include <Geode/ui/Popup.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/utils/async.hpp>
#include "../StreakData.h"
#include "../RewardNotification.h"
#include "../StatusSpinner.h"
#include "StreakChestPopup.h"
#include "../utils/RoundedProgressBar.h"
#include "../HMACAuth.h"

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

    CCMenuItemSpriteExtra* m_claimBtn = nullptr;

    bool init(const TrendLevelDef& data, float width, std::function<void()> reloadCallback) {
        if (!CCLayerColor::init()) return false;
        m_data = data;
        m_reloadFunc = reloadCallback;

        float height = 90.0f;
        this->setContentSize({ width, height });
        this->setAnchorPoint({ 0, 0 });

        auto bg = cocos2d::extension::CCScale9Sprite::create("GJ_square02.png");
        bg->setContentSize({ width, height - 2.0f });
        bg->setPosition({ width / 2, height / 2 });
        this->addChild(bg, 0);

        auto glm = GameLevelManager::sharedState();
        auto level = glm->getSavedLevel(data.levelID);

        int normalPercent = level ? level->m_normalPercent : 0;
        int practicePercent = level ? level->m_practicePercent : 0;
        bool isCompleted = (normalPercent >= 100);

        auto menu = CCMenu::create();
        menu->setPosition({ 0, 0 });
        this->addChild(menu, 2);

        
        auto face = CCSprite::createWithSpriteFrameName(m_data.difficultySprite.c_str());
        if (!face) face = CCSprite::createWithSpriteFrameName("difficulty_00_btn_001.png");
        if (face) {
            face->setPosition({ 35.f, height / 2 + 15.f });
            face->setScale(0.95f);
            this->addChild(face, 1);
        }
 
        auto playSpr = CCSprite::create("play_btn.png"_spr);
        if (!playSpr) playSpr = CCSprite::createWithSpriteFrameName("GJ_playBtn2_001.png");
        playSpr->setScale(0.45f);
        auto playBtn = CCMenuItemSpriteExtra::create(playSpr, this, menu_selector(TrendLevelCell::onPlayClick));
        playBtn->setPosition({ 35.f, height / 2 - 20.f });
        menu->addChild(playBtn);
 
        auto title = CCLabelBMFont::create(data.name.c_str(), "bigFont.fnt");
        title->setScale(0.65f);
        title->setAnchorPoint({ 0, 0.5f });
        title->setPosition({ 75.0f, height - 18.0f });
        this->addChild(title, 1);

        auto creator = CCLabelBMFont::create(fmt::format("By {}", data.creator).c_str(), "goldFont.fnt");
        creator->setScale(0.40f);
        creator->setAnchorPoint({ 0, 0.5f });
        creator->setPosition({ 75.0f, height - 31.0f });
        this->addChild(creator, 1);

       
        float barW = 140.0f;
        float barX = 75.0f;

      
        float normalBarH = 20.0f;
        float normalY = 30.0f;
        auto normalBar = RoundedProgressBar::create(barW, normalBarH);
        normalBar->setPosition({ barX + barW / 2, normalY + normalBarH / 2 });
        normalBar->setGradientColors({ 100, 200, 255 }, { 0, 100, 255 });
        normalBar->setProgress(std::min(normalPercent, 100) / 100.0f);
        this->addChild(normalBar, 2);

        auto normalLbl = CCLabelBMFont::create(fmt::format("{}%", normalPercent).c_str(), "bigFont.fnt");
        normalLbl->setScale(0.30f);
        normalLbl->setPosition({ barX + barW / 2, normalY + normalBarH / 2 + 0.5f });
        this->addChild(normalLbl, 5);

     
        float practiceBarH = 13.0f;
        float practiceY = 15.0f;
        auto practiceBar = RoundedProgressBar::create(barW, practiceBarH);
        practiceBar->setPosition({ barX + barW / 2, practiceY + practiceBarH / 2 });
        practiceBar->setGradientColors({ 150, 255, 150 }, { 40, 200, 40 });
        practiceBar->setProgress(std::min(practicePercent, 100) / 100.0f);
        this->addChild(practiceBar, 2);
 
        auto practiceLbl = CCLabelBMFont::create(fmt::format("{}% Practice", practicePercent).c_str(), "bigFont.fnt");
        practiceLbl->setScale(0.28f);
        practiceLbl->setPosition({ barX + barW / 2, practiceY + practiceBarH / 2 + 0.5f });
        this->addChild(practiceLbl, 5);

       
        CCPoint actionBtnPos = { width - 40.0f, height / 2 };

        if (data.isClaimed) {
            auto check = CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
            check->setPosition(actionBtnPos);
            this->addChild(check);
        }
        else {
            if (isCompleted) {
                auto claimSpr = CCSprite::createWithSpriteFrameName("GJ_rewardBtn_001.png");
                m_claimBtn = CCMenuItemSpriteExtra::create(claimSpr, this, menu_selector(TrendLevelCell::onClaimClick));
                m_claimBtn->setPosition(actionBtnPos);
                menu->addChild(m_claimBtn);
            }
            else {
                auto chest = CCSprite::createWithSpriteFrameName("chest_02_02_001.png");
                if (chest) {
                    chest->setScale(0.5f);
                    chest->setPosition(actionBtnPos);
                    this->addChild(chest);
                }
            }
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
        if (m_claimBtn) m_claimBtn->setEnabled(false);

        auto am = GJAccountManager::sharedState();
        matjson::Value body = matjson::Value::object();
        body.set("accountID", am->m_accountID);
        body.set("levelID", m_data.levelID);

        auto req = web::WebRequest();
        HMACAuth::signRequest(req, am->m_accountID, body);
        req.bodyJSON(body);

        m_claimTask.spawn(
            req.post("https://streak-servidor.onrender.com/trending-level/claim"),
            [this](web::WebResponse res) {
                if (res.ok()) {
                    m_data.isClaimed = true;
                    if (m_claimBtn) {
                        m_claimBtn->removeFromParent();
                        m_claimBtn = nullptr;
                    }

                    auto check = CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
                    check->setScale(0.8f);
                    check->setPosition({ this->getContentSize().width - 40.0f, this->getContentSize().height / 2 });
                    this->addChild(check);

                    StreakChestPopup::create(m_data.superStars, m_data.starTickets, m_data.gems, 0, m_reloadFunc)->show();
                }
                else {
                    if (m_claimBtn) m_claimBtn->setEnabled(true);
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

    bool init() override {
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
        HMACAuth::signGetRequest(req, am->m_accountID);
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
        def.name = data["name"].as<std::string>().unwrapOr("Unknown");
        def.creator = data["creator"].as<std::string>().unwrapOr("-");
        def.difficultySprite = data["difficultySprite"].as<std::string>().unwrapOr("difficulty_00_btn_001.png");
        def.isClaimed = data["isClaimed"].as<bool>().unwrapOr(false);

        if (data.contains("rewards")) {
            def.superStars = data["rewards"]["super_stars"].as<int>().unwrapOr(0);
            def.starTickets = data["rewards"]["star_tickets"].as<int>().unwrapOr(0);
            def.gems = data["rewards"]["gems"].as<int>().unwrapOr(0);
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