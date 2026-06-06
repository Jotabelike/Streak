#include "StreakData.h"
#include "StreakMusic.h"
#include "UpdateState.h"
#include "SystemNotification.h"
#include "Popups.h"
#include "FirebaseManager.h"
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/AppDelegate.hpp>
#include <Geode/modify/CommentCell.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/GJAccountManager.hpp>
#include <Geode/binding/GJUserScore.hpp>
#include <Geode/binding/GJComment.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/utils/async.hpp>
#include <Geode/loader/Event.hpp>
#include <matjson.hpp>
#include <Geode/utils/cocos.hpp>
#include <Geode/modify/OptionsLayer.hpp>
#include <Geode/ui/Notification.hpp> 
#include "WelcomeNotification.h"
#include "RewardNotification.h"
#include "HMACAuth.h"

class $modify(StreakAppDelegate, AppDelegate) {
    void applicationWillEnterForeground() {
        AppDelegate::applicationWillEnterForeground();
        StreakMusic::resync();
    }
};

class $modify(MyPlayLayer, PlayLayer) {
    struct Fields {
        int m_pointsGained = 0;
    };

    void levelComplete() {
        StreakDataBridge::pointsGained = 0;
        int percentBefore = this->m_level->m_normalPercent;
        PlayLayer::levelComplete();
        int percentAfter = this->m_level->m_normalPercent;
        if (this->m_isPracticeMode) return;
        if (percentBefore >= 100) return;
        if (percentAfter < 100) {
            log::warn("Anti-Cheat: Level completed, but the final percentage is only {}%", percentAfter);
            return;
        }

        int stars = this->m_level->m_stars;

        if (stars > 0) {
            int points = 0;
            if (stars <= 3) points = 1;       // auto - easy - normal
            else if (stars <= 5) points = 3;  // hard
            else if (stars <= 7) points = 4;  // harder
            else if (stars <= 9) points = 5;  // insane
            else points = 6;                  // demon

            if (points > 0) {
                int before = g_streakData.streakPointsToday;
                int required = g_streakData.getRequiredPoints();

                log::info("Legally completed level ({} stars -> {} points)", stars, points);
                g_streakData.addPoints(points);

                if (geode::Mod::get()->getSavedValue<int>("end_level_anim_mode", 2) != 0) {
                    StreakDataBridge::pointsGained = points;
                }
            }
        }
    }
};

class $modify(MyMenuLayer, MenuLayer) {

    struct Fields {
        async::TaskHolder<web::WebResponse> m_playerDataListener;
        bool m_isReconnecting = false;
    };

    enum class ButtonState { Loading, Active, Error };

    bool init() override {
        if (!MenuLayer::init()) return false;

        this->createStreakButton(ButtonState::Loading);
        this->loadPlayerData();

        StreakMusic::resync();

        StreakUpdate::checkOnce([] {
            SystemNotification::show(
                "New Update",
                fmt::format("v{}", StreakUpdate::s_latestVersion),
                "download_btn.png"_spr,
                0.5f
            );
        });

        return true;
    }

    void tryReconnect(float dt) {
        if (g_streakData.isDataLoaded) {
            this->unschedule(schedule_selector(MyMenuLayer::tryReconnect));
            m_fields->m_isReconnecting = false;
            this->createStreakButton(ButtonState::Active);
            return;
        }

        this->createStreakButton(ButtonState::Loading);
        this->loadPlayerData();
    }

    void loadPlayerData() {
        if (g_streakData.isDataLoaded && g_streakData.m_initialized && !HMACAuth::getSessionToken().empty()) {
            this->createStreakButton(ButtonState::Active);
            return;
        }
        auto accountManager = GJAccountManager::sharedState();
        if (!accountManager || accountManager->m_accountID == 0) {
            g_streakData.m_initialized = true;
            g_streakData.isDataLoaded = true;
            g_streakData.dailyUpdate();
            this->createStreakButton(ButtonState::Active);
            return;
        }

        int accountID = accountManager->m_accountID;
        std::string url = fmt::format("https://streak-servidor.onrender.com/players/{}", accountID);
 
        HMACAuth::clearSessionToken();

        auto req = web::WebRequest();
        HMACAuth::signGetRequest(req, accountID);

        m_fields->m_playerDataListener.spawn(req.get(url), [this, accountID](web::WebResponse res) {
            if (res.ok() && res.json().isOk()) {
                auto data = res.json().unwrap();
                 
                if (data.contains("session_token")) {
                    std::string token = data["session_token"].as<std::string>().unwrapOr(std::string(""));
                    if (!token.empty()) {
                        HMACAuth::setSessionToken(token);
                        log::info("Session token received.");
                    }
                }

                g_streakData.parseServerResponse(data);
                if (g_streakData.isBanned) {
                    if (m_fields->m_isReconnecting) {
                        this->unschedule(schedule_selector(MyMenuLayer::tryReconnect));
                        m_fields->m_isReconnecting = false;
                    }
                    this->createStreakButton(ButtonState::Error);
                    return;
                }
                this->onLoadSuccess();
            }
            else if (res.code() == 404) {
                g_streakData.needsRegistration = true;
                this->onLoadSuccess();
            }
            else {
                log::warn("Load failed (Code: {})", res.code());
                this->onLoadFailed();
            }
            });
    }

    void onLoadSuccess() {
        if (m_fields->m_isReconnecting) {
            this->unschedule(schedule_selector(MyMenuLayer::tryReconnect));
            m_fields->m_isReconnecting = false;
        }
        g_streakData.isDataLoaded = true;
        g_streakData.m_initialized = true;
        g_streakData.dailyUpdate();
        this->createStreakButton(ButtonState::Active);
    }

    void onLoadFailed() {
        g_streakData.isDataLoaded = false;
        g_streakData.m_initialized = false;

        this->createStreakButton(ButtonState::Error);

        if (!m_fields->m_isReconnecting) {
            m_fields->m_isReconnecting = true;
            this->schedule(schedule_selector(MyMenuLayer::tryReconnect), 5.0f);
        }
    }

    void createStreakButton(ButtonState state) {
        auto menu = this->getChildByID("bottom-menu");
        if (!menu) return;
        if (auto oldBtn = menu->getChildByID("streak-button"_spr)) oldBtn->removeFromParentAndCleanup(true);

        CCSprite* icon = nullptr;
        CircleBaseColor color = CircleBaseColor::Gray;
        bool shouldRotate = false;

        switch (state) {
        case ButtonState::Loading:
            icon = CCSprite::create("loading.gif"_spr);
            if (icon) {
                shouldRotate = false;
            }
            else {
                icon = CCSprite::createWithSpriteFrameName("GJ_updateBtn_001.png");
                shouldRotate = true;
            }
            color = CircleBaseColor::Gray;
            break;

        case ButtonState::Error:
            icon = CCSprite::create("error_face.png"_spr);
            if (!icon) icon = CCSprite::createWithSpriteFrameName("exMark_001.png");
            shouldRotate = false;
            color = CircleBaseColor::Gray;
            break;

        case ButtonState::Active:
            std::string spriteName = g_streakData.getRachaSprite();
            if (!spriteName.empty()) icon = CCSprite::create(spriteName.c_str());
            if (!icon) icon = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
            shouldRotate = false;
            color = CircleBaseColor::Green;
            break;
        }

        if (!icon) return;

        if (state == ButtonState::Loading) {
            icon->setScale(1.0f);
        }
        else if (state == ButtonState::Error) {
            icon->setScale(0.8f);
        }
        else {
            icon->setScale(0.8f);
        }

        auto circle = CircleButtonSprite::create(icon, color, CircleBaseSize::Medium);

        if (shouldRotate) {
            icon->runAction(CCRepeatForever::create(CCRotateBy::create(1.0f, 360.f)));
        }

        if (state == ButtonState::Active) {
            int requiredPoints = g_streakData.getRequiredPoints();
            if (requiredPoints > 0 && g_streakData.streakPointsToday < requiredPoints) {
                auto alertSprite = CCSprite::createWithSpriteFrameName("exMark_001.png");
                if (alertSprite) {
                    alertSprite->setScale(0.5f);
                    alertSprite->setPosition({ circle->getContentSize().width - 10.f,
                    circle->getContentSize().height - 10.f
                        }
                    );

                    alertSprite->setZOrder(10);

                    auto tintRed = CCTintTo::create(0.5f, 255, 100, 100);
                    auto tintNormal = CCTintTo::create(0.5f, 255, 255, 255);
                    alertSprite->runAction(CCRepeatForever::create(CCSequence::create(tintRed, tintNormal, nullptr)));

                    circle->addChild(alertSprite);
                }
            }
        }

        SEL_MenuHandler callback = nullptr;
        if (state == ButtonState::Active) callback = menu_selector(MyMenuLayer::onOpenPopup);
        else if (state == ButtonState::Error) callback = menu_selector(MyMenuLayer::onErrorButtonClick);

        auto btn = CCMenuItemSpriteExtra::create(circle, this, callback);
        btn->setID("streak-button"_spr);

        if (state == ButtonState::Loading) {
            btn->setEnabled(false);
            btn->setOpacity(255);
        }

        menu->addChild(btn);
        menu->updateLayout();
    }

    void onErrorButtonClick(CCObject*) {
        if (g_streakData.isBanned) {
            createQuickPopup("ACCOUNT BANNED",
                "You have been <cr>BANNED</c> from Streak Mod.\nReason: <cy>" + g_streakData.banReason + "</c>",
                "OK", "Discord",
                [](FLAlertLayer*,
                    bool btn2) {
                        if (btn2) cocos2d::CCApplication::sharedApplication()->openURL("https://discord.gg/vEPWBuFEn5");
                }
            );
            return;
        }
        FLAlertLayer::create("Connection Failed",
            "<cr>Internet connection required.</c>\nRetrying in background...",
            "OK")->show();
    }

    void onOpenPopup(CCObject * sender) {
        if (!g_streakData.isDataLoaded && !g_streakData.needsRegistration) {
            this->onErrorButtonClick(nullptr);
            return;
        }
        if (g_streakData.isBanned) {
            this->onErrorButtonClick(nullptr);
            return;
        }
        if (g_streakData.needsRegistration) {
            RegisterPopup::create()->show();
            return;
        }
        auto scene = StreakMainLayer::scene();
        auto transition = CCTransitionFade::create(0.5f, scene);
        CCDirector::sharedDirector()->pushScene(transition);
    }
};

class $modify(MyCommentCell, CommentCell) {
    struct Fields {
        CCMenuItemSpriteExtra* badgeButton = nullptr;
        async::TaskHolder<web::WebResponse> m_badgeListener;
    };

    void onBadgeInfoClick(CCObject * sender) {
        if (auto badgeID = static_cast<CCString*>(static_cast<CCNode*>(sender)->getUserObject("badge"_spr))) {
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
                        badgeButton->setUserObject("badge"_spr, CCString::create(equippedBadge->badgeID));
                        badgeButton->setID("streak-badge"_spr);
                        username_menu->addChild(badgeButton);
                        username_menu->updateLayout();
                    }
                }
            }
            return;
        }

        std::string cachedBadge = g_streakData.getCachedBadge(p0->m_accountID);

        if (!cachedBadge.empty()) {
            if (cachedBadge == "none") return;

            if (auto username_menu = m_mainLayer->getChildByIDRecursive("username-menu")) {
                if (auto badgeInfo = g_streakData.getBadgeInfo(cachedBadge)) {
                    auto badgeSprite = CCSprite::create(badgeInfo->spriteName.c_str());
                    if (badgeSprite) {
                        badgeSprite->setScale(0.15f);
                        auto badgeButton = CCMenuItemSpriteExtra::create(
                            badgeSprite, this, menu_selector(MyCommentCell::onBadgeInfoClick)
                        );
                        badgeButton->setUserObject("badge"_spr, CCString::create(badgeInfo->badgeID));
                        username_menu->addChild(badgeButton);
                        username_menu->updateLayout();
                    }
                }
            }
            return;
        }
 
        std::string url = fmt::format("https://streak-servidor.onrender.com/players/{}/public-profile", p0->m_accountID);
        auto req = web::WebRequest();

        m_fields->m_badgeListener.spawn(req.get(url), [this, p0](web::WebResponse res) {
            if (res.ok() && res.json().isOk()) {
                auto playerData = res.json().unwrap();
                std::string badgeId = playerData["equipped_badge_id"].as<std::string>().unwrapOr("");

                if (badgeId.empty()) {
                    g_streakData.cacheUserBadge(p0->m_accountID, "none");
                }
                else {
                    g_streakData.cacheUserBadge(p0->m_accountID, badgeId);
                }

                if (!badgeId.empty()) {
                    if (auto badgeInfo = g_streakData.getBadgeInfo(badgeId)) {
                        if (auto username_menu = m_mainLayer->getChildByIDRecursive("username-menu")) {
                            if (username_menu->getChildByID("streak-badge-dynamic"_spr)) return;

                            auto badgeSprite = CCSprite::create(badgeInfo->spriteName.c_str());
                            if (badgeSprite) {
                                badgeSprite->setScale(0.15f);
                                auto badgeButton = CCMenuItemSpriteExtra::create(
                                    badgeSprite, this, menu_selector(MyCommentCell::onBadgeInfoClick)
                                );
                                badgeButton->setUserObject("badge"_spr, CCString::create(badgeInfo->badgeID));
                                badgeButton->setID("streak-badge-dynamic"_spr);
                                username_menu->addChild(badgeButton);
                                username_menu->updateLayout();
                            }
                        }
                    }
                }
            }
            else {
                if (res.code() == 404) {
                    g_streakData.cacheUserBadge(p0->m_accountID, "none");
                }
            }
            });
    }
};

class $modify(MyPauseLayer, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();
        int mode = geode::Mod::get()->getSavedValue<int>("pause_hud_mode", 0);
        if (mode == 1)
            return;

        auto winSize = cocos2d::CCDirector::sharedDirector()->getWinSize();
        double posX = Mod::get()->getSavedValue<double>("pause-pos-x", 0.10);
        double posY = Mod::get()->getSavedValue<double>("pause-pos-y", 0.90);
        double scale = Mod::get()->getSavedValue<double>("pause-scale", 0.80);

        if (scale < 0.5) scale = 0.5;
        if (scale > 10.0) scale = 10.0;
        int pointsToday = g_streakData.streakPointsToday;
        int requiredPoints = g_streakData.getRequiredPoints();
        int streakDays = g_streakData.currentStreak;
        auto streakNode = CCNode::create();

        streakNode->setID("streak-hud-node"_spr);
        streakNode->setScale(static_cast<float>(scale));
        std::string spriteName = g_streakData.getRachaSprite();
        CCSprite* streakIcon = nullptr;


        if (!spriteName.empty()) {
            streakIcon = CCSprite::create(spriteName.c_str());
        }
        if (!streakIcon) {
            streakIcon = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
        }
        if (streakIcon) {
            streakIcon->setScale(0.2f);
            streakNode->addChild(streakIcon);
        }
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
        if (!pointIcon) pointIcon = CCSprite::createWithSpriteFrameName("starSmall_001.png");

        pointIcon->setScale(0.12f);
        pointCounterNode->addChild(pointIcon);
        pointCounterNode->setContentSize({
            pointLabel->getScaledContentSize().width + pointIcon->getScaledContentSize().width + 5,
            pointLabel->getScaledContentSize().height
            });

        pointLabel->setPosition({ -pointIcon->getScaledContentSize().width / 2, 0 });
        pointIcon->setPosition({ pointLabel->getScaledContentSize().width / 2 + 5, 0 });
        streakNode->setPosition({
            winSize.width * static_cast<float>(posX),
            winSize.height * static_cast<float>(posY)
            });

        this->addChild(streakNode, 100);
    }
};