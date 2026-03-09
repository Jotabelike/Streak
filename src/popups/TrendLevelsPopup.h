#pragma once
#include <Geode/ui/Popup.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/utils/async.hpp>
#include "../StreakData.h"
#include "../RewardNotification.h"
#include "../StatusSpinner.h"
#include "StreakChestPopup.h"

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
    CCMenuItemSpriteExtra* m_chestBtn = nullptr;

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
        int percent = level ? level->m_normalPercent : 0;
        bool isCompleted = (percent >= 100);

     
        auto menu = CCMenu::create();
        menu->setPosition({ 0, 0 });
        this->addChild(menu, 2);

       
        auto face = CCSprite::createWithSpriteFrameName(m_data.difficultySprite.c_str());
        if (!face) {
            face = CCSprite::createWithSpriteFrameName("difficulty_00_btn_001.png");
        }
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
        title->setPosition({ 75.0f, height - 20.0f });

        float maxTitleW = width - 150.0f;
        if (title->getScaledContentSize().width > maxTitleW) {
            title->setScale(title->getScale() * (maxTitleW / title->getScaledContentSize().width));
        }
        this->addChild(title, 1);

        auto creator = CCLabelBMFont::create(fmt::format("By {}", data.creator).c_str(), "goldFont.fnt");
        creator->setScale(0.45f);
        creator->setAnchorPoint({ 0, 0.5f });
        creator->setPosition({ 75.0f, height - 33.0f });
        this->addChild(creator, 1);

      
        float barW = 140.0f;
        float barH = 14.0f;
        float barX = 75.0f;
        float barY = height / 2 - 12.f;

        auto borderOuter = CCLayerColor::create({ 0, 0, 0, 255 }, barW + 4.f, barH + 4.f);
        borderOuter->setPosition({ barX - 2.f, barY - 2.f });
        this->addChild(borderOuter, 0);

        auto borderInner = CCLayerColor::create({ 255, 255, 255, 120 }, barW + 2.f, barH + 2.f);
        borderInner->setPosition({ barX - 1.f, barY - 1.f });
        this->addChild(borderInner, 1);

        auto barBg = CCLayerColor::create({ 15, 15, 15, 255 }, barW, barH);
        barBg->setPosition({ barX, barY });
        this->addChild(barBg, 2);

        if (percent > 0) {
            ccColor4B topColor = ccColor4B{ 100, 200, 255, 255 };
            ccColor4B botColor = ccColor4B{ 0, 100, 255, 255 };
            float fillW = barW * (std::min(percent, 100) / 100.0f);
            auto barFg = CCLayerGradient::create(topColor, botColor, { 0, -1 });
            barFg->setContentSize({ fillW, barH });
            barFg->setPosition({ barX, barY });
            this->addChild(barFg, 3);
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

       
        CCPoint actionBtnPos = { width - 40.0f, height / 2 };

        if (data.isClaimed) {
         
            auto check = CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
            check->setScale(1.0f);
            check->setPosition(actionBtnPos);
            this->addChild(check);
        }
        else {
            if (isCompleted) {
                
                auto claimSpr = CCSprite::createWithSpriteFrameName("GJ_rewardBtn_001.png");
                claimSpr->setScale(1.0f);
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

      
        float offset = 0.0f; 

        auto cornerTL = CCSprite::createWithSpriteFrameName("rewardCorner_001.png");
        if (cornerTL) {
            cornerTL->setAnchorPoint({0, 1});
            cornerTL->setPosition({offset, m_size.height - offset});
            cornerTL->setFlipY(true);
            m_mainLayer->addChild(cornerTL, 2);
        }

        auto cornerTR = CCSprite::createWithSpriteFrameName("rewardCorner_001.png");
        if (cornerTR) {
            cornerTR->setAnchorPoint({1, 1});
            cornerTR->setPosition({m_size.width - offset, m_size.height - offset});
            cornerTR->setFlipX(true);
            cornerTR->setFlipY(true);
            m_mainLayer->addChild(cornerTR, 2);
        }

        auto cornerBL = CCSprite::createWithSpriteFrameName("rewardCorner_001.png");
        if (cornerBL) {
            cornerBL->setAnchorPoint({0, 0});
            cornerBL->setPosition({offset, offset});
            m_mainLayer->addChild(cornerBL, 2);
        }

        auto cornerBR = CCSprite::createWithSpriteFrameName("rewardCorner_001.png");
        if (cornerBR) {
            cornerBR->setAnchorPoint({1, 0});
            cornerBR->setPosition({m_size.width - offset, offset});
            cornerBR->setFlipX(true);
            m_mainLayer->addChild(cornerBR, 2);
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