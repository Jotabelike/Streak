#include "StreakData.h"
#include "Popups.h"
#include "FirebaseManager.h"

#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/CommentCell.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/GJAccountManager.hpp>
#include <Geode/binding/GJUserScore.hpp>
#include <Geode/binding/GJComment.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/loader/Event.hpp>
#include <matjson.hpp>
#include <Geode/utils/cocos.hpp>

class $modify(MyPlayLayer, PlayLayer) {
    void levelComplete() {

        int oldPercent = this->m_level->m_normalPercent;

        
        PlayLayer::levelComplete();

        
        if (oldPercent >= 100) {
            return; 
        }


        int stars = this->m_level->m_stars;
        int pointsGained = 0;
        if (stars > 0 && !this->m_isPracticeMode) {
            if (stars >= 1 && stars <= 3) pointsGained = 1;
            else if (stars >= 4 && stars <= 5) pointsGained = 3;
            else if (stars >= 6 && stars <= 7) pointsGained = 4;
            else if (stars >= 8 && stars <= 9) pointsGained = 5;
            else if (stars == 10) pointsGained = 6;

            if (pointsGained > 0) {
                int pointsBefore = g_streakData.streakPointsToday;
                g_streakData.addPoints(pointsGained);
                int pointsRequired = g_streakData.getRequiredPoints();

                if (auto scene = CCDirector::sharedDirector()->getRunningScene()) {
                    CCObject* obj = nullptr;
                    CCARRAY_FOREACH(scene->getChildren(), obj) {
                        if (auto popup = dynamic_cast<InfoPopup*>(obj)) {
                            popup->updateDisplay();
                            break;
                        }
                    }
                }

                auto progressBar = StreakProgressBar::create(pointsGained, pointsBefore, pointsRequired);
                if (auto scene = CCDirector::sharedDirector()->getRunningScene()) {
                    scene->addChild(progressBar, 100);
                }
            }
        }
    }
};

class $modify(MyMenuLayer, MenuLayer) {

    
    struct Fields {
        EventListener<web::WebTask> m_playerDataListener;
    };

    bool init() {
        if (!MenuLayer::init()) return false;
        this->createLoadingButton();

       
        this->loadPlayerData();

        return true;
    }

    void createLoadingButton() {
        auto menu = this->getChildByID("bottom-menu");
        if (!menu) return;

        if (auto oldBtn = menu->getChildByID("streak-button"_spr)) {
            oldBtn->removeFromParentAndCleanup(true);
        }

        auto loadingIcon = CCSprite::createWithSpriteFrameName("GJ_updateBtn_001.png");
        if (!loadingIcon) return;

        loadingIcon->runAction(CCRepeatForever::create(CCRotateBy::create(1.0f, 360)));
        loadingIcon->setScale(0.4f);

        auto circle = CircleButtonSprite::create(loadingIcon, CircleBaseColor::Gray, CircleBaseSize::Medium);
        if (!circle) return;

        auto btn = CCMenuItemSpriteExtra::create(circle, this, nullptr);
        btn->setEnabled(false);
        btn->setID("streak-button"_spr);
        btn->setPositionY(btn->getPositionY() + 5);
        menu->addChild(btn);
        menu->updateLayout();
    }

    void loadPlayerData() {
        auto accountManager = GJAccountManager::sharedState();
        if (accountManager->m_accountID == 0) {
            g_streakData.resetToDefault();
            g_streakData.m_initialized = true;
            g_streakData.isDataLoaded = true;
            this->createFinalButton();
            return;
        }

        m_fields->m_playerDataListener.bind([this](web::WebTask::Event* e) {

           
            if (web::WebResponse* res = e->getValue()) {
                if (res->ok() && res->json().isOk()) {
                    log::info("Player data found, parsing response.");
                    g_streakData.parseServerResponse(res->json().unwrap());
                    g_streakData.m_initialized = true;
                    g_streakData.isDataLoaded = true;
                    this->createFinalButton();
                }

                
                else if (res->code() == 404) {
                    log::info("New player detected. Creating profile.");
                    g_streakData.resetToDefault();
                    updatePlayerDataInFirebase();  

                    g_streakData.m_initialized = true;
                    g_streakData.isDataLoaded = true;
                    this->createFinalButton(); 
                }

               
                else {
                    log::error("Failed to load player data. Response code: {}", res->code());
                    this->createErrorButton();
                }

            }
            else if (e->isCancelled()) {
                log::error("Player data request cancelled or timed out.");
                this->createErrorButton();
            }
            });

       
        std::string url = fmt::format("https://streak-servidor.onrender.com/players/{}", accountManager->m_accountID);
        auto req = web::WebRequest();
        m_fields->m_playerDataListener.setFilter(req.get(url));
    }

    void createFinalButton() {
      
        auto menu = this->getChildByID("bottom-menu");
        if (!menu) return;

        if (auto oldBtn = menu->getChildByID("streak-button"_spr)) {
            oldBtn->removeFromParentAndCleanup(true);
        }

        std::string spriteName = g_streakData.getRachaSprite();
        CCSprite* icon = CCSprite::create(spriteName.c_str());

        if (!icon) {
            icon = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
        }
        if (!icon) return; 

        icon->setScale(0.5f);
        auto circle = CircleButtonSprite::create(icon, CircleBaseColor::Green, CircleBaseSize::Medium);
        if (!circle) return;

        int requiredPoints = g_streakData.getRequiredPoints();
        bool streakInactive = (g_streakData.streakPointsToday < requiredPoints);

        if (streakInactive) {
            auto alertSprite = CCSprite::createWithSpriteFrameName("exMark_001.png");
            if (alertSprite) {
                alertSprite->setScale(0.4f);
                alertSprite->setPosition({ circle->getContentSize().width - 12, circle->getContentSize().height - 12 });
                alertSprite->setZOrder(10);
                alertSprite->runAction(CCRepeatForever::create(CCBlink::create(2.0f, 3)));
                circle->addChild(alertSprite);
            }
        }

        auto newBtn = CCMenuItemSpriteExtra::create(circle, this, menu_selector(MyMenuLayer::onOpenPopup));
        if (!newBtn) return;

        newBtn->setID("streak-button"_spr);
        newBtn->setPositionY(newBtn->getPositionY() + 5);
        newBtn->setEnabled(true);
        menu->addChild(newBtn);
        menu->updateLayout();
    }

    void createErrorButton() {
       
        auto menu = this->getChildByID("bottom-menu");
        if (!menu) return;

        if (auto oldBtn = menu->getChildByID("streak-button"_spr)) {
            oldBtn->removeFromParentAndCleanup(true);
        }

        auto errorIcon = CCSprite::createWithSpriteFrameName("exMark_001.png");
        if (!errorIcon) return;

        errorIcon->setScale(0.5f);
        auto circle = CircleButtonSprite::create(errorIcon, CircleBaseColor::Gray, CircleBaseSize::Medium);
        if (!circle) return;

        auto errorBtn = CCMenuItemSpriteExtra::create(circle, this, nullptr); 
        errorBtn->setEnabled(false); 
        errorBtn->setID("streak-button"_spr);
        errorBtn->setPositionY(errorBtn->getPositionY() + 5);
        menu->addChild(errorBtn);
        menu->updateLayout();
    }

    void onOpenPopup(CCObject * sender) {
        if (g_streakData.isInitialized()) {
            InfoPopup::create()->show();
        }
        else {
            FLAlertLayer::create("Loading", "The streak data is still loading. Please wait.", "OK")->show();
        }
    }
};

class $modify(MyCommentCell, CommentCell) {
    struct Fields {
        CCMenuItemSpriteExtra* badgeButton = nullptr;
        EventListener<web::WebTask> m_badgeListener;
    };

    void onBadgeInfoClick(CCObject * sender) {
        if (auto badgeID = static_cast<CCString*>(static_cast<CCNode*>(sender)->getUserObject())) {
            if (auto badgeInfo = g_streakData.getBadgeInfo(badgeID->getCString())) {
                std::string title = badgeInfo->displayName;
                std::string category = g_streakData.getCategoryName(badgeInfo->category);
                std::string message = fmt::format(
                    "<cy>{}</c>\n\n<cg>Unlocked at {} days</c>",
                    category,
                    badgeInfo->daysRequired
                );
                FLAlertLayer::create(title.c_str(), message, "OK")->show();
            }
        }
    }

    void loadFromComment(GJComment * p0) {
        CommentCell::loadFromComment(p0);
        if (p0->m_accountID == GJAccountManager::get()->get()->m_accountID) {
            if (auto username_menu = m_mainLayer->getChildByIDRecursive("username-menu")) {
                auto equippedBadge = g_streakData.getEquippedBadge();
                if (equippedBadge) {
                    auto badgeSprite = CCSprite::create(equippedBadge->spriteName.c_str());
                    if (badgeSprite) {
                        badgeSprite->setScale(0.15f);
                        auto badgeButton = CCMenuItemSpriteExtra::create(
                            badgeSprite, this, menu_selector(MyCommentCell::onBadgeInfoClick)
                        );
                        badgeButton->setUserObject(CCString::create(equippedBadge->badgeID));
                        badgeButton->setID("streak-badge"_spr);
                        username_menu->addChild(badgeButton);
                        username_menu->updateLayout();
                    }
                }
            }
        }
        else {
            std::string url = fmt::format("https://streak-servidor.onrender.com/players/{}", p0->m_accountID);
            m_fields->m_badgeListener.bind([this, p0](web::WebTask::Event* e) {
                if (web::WebResponse* res = e->getValue()) {
                    if (res->ok() && res->json().isOk()) {
                        try {
                            auto playerData = res->json().unwrap();
                            std::string badgeId = playerData["equipped_badge_id"].as<std::string>().unwrap();
                            if (!badgeId.empty()) {
                                auto badgeInfo = g_streakData.getBadgeInfo(badgeId);
                                if (badgeInfo) {
                                    if (auto username_menu = m_mainLayer->getChildByIDRecursive("username-menu")) {
                                        auto badgeSprite = CCSprite::create(badgeInfo->spriteName.c_str());
                                        badgeSprite->setScale(0.15f);
                                        auto badgeButton = CCMenuItemSpriteExtra::create(
                                            badgeSprite, this, menu_selector(MyCommentCell::onBadgeInfoClick)
                                        );
                                        badgeButton->setUserObject(CCString::create(badgeInfo->badgeID));
                                        username_menu->addChild(badgeButton);
                                        username_menu->updateLayout();
                                    }
                                }
                            }
                        }
                        catch (const std::exception& ex) {
                            log::debug("Player {} (commenter) has no badge: {}", p0->m_accountID, ex.what());
                        }
                    }
                }
                });
            auto req = web::WebRequest();
            m_fields->m_badgeListener.setFilter(req.get(url));
        }
    }
};

class $modify(MyPauseLayer, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();
        if (geode::Mod::get()->getSettingValue<bool>("show-in-pause")) {
            auto winSize = cocos2d::CCDirector::sharedDirector()->getWinSize();
            double posX = Mod::get()->getSettingValue<double>("pause-pos-x");
            double posY = Mod::get()->getSettingValue<double>("pause-pos-y");
            int pointsToday = g_streakData.streakPointsToday;
            int requiredPoints = g_streakData.getRequiredPoints();
            int streakDays = g_streakData.currentStreak;
            auto streakNode = CCNode::create();
            auto streakIcon = CCSprite::create(g_streakData.getRachaSprite().c_str());
            streakIcon->setScale(0.2f);
            streakNode->addChild(streakIcon);
            auto daysLabel = CCLabelBMFont::create(
                CCString::createWithFormat("Day %d", streakDays)->getCString(), "goldFont.fnt"
            );
            daysLabel->setScale(0.35f);
            daysLabel->setPosition({ 0, -22 });
            streakNode->addChild(daysLabel);
            auto pointCounterNode = CCNode::create();
            pointCounterNode->setPosition({ 0, -37 });
            streakNode->addChild(pointCounterNode);
            auto pointLabel = CCLabelBMFont::create(
                CCString::createWithFormat("%d / %d", pointsToday, requiredPoints)->getCString(), "bigFont.fnt"
            );
            pointLabel->setScale(0.35f);
            pointCounterNode->addChild(pointLabel);
            auto pointIcon = CCSprite::create("streak_point.png"_spr);
            pointIcon->setScale(0.18f);
            pointCounterNode->addChild(pointIcon);
            pointCounterNode->setContentSize({ pointLabel->getScaledContentSize().width + pointIcon->getScaledContentSize().width + 5, pointLabel->getScaledContentSize().height });
            pointLabel->setPosition({ -pointIcon->getScaledContentSize().width / 2, 0 });
            pointIcon->setPosition({ pointLabel->getScaledContentSize().width / 2 + 5, 0 });
            streakNode->setPosition({ winSize.width * static_cast<float>(posX), winSize.height * static_cast<float>(posY) });
            this->addChild(streakNode);
        }
    }
};