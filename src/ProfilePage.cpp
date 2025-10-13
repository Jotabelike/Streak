#include "StreakData.h"
#include <Geode/modify/ProfilePage.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/GJAccountManager.hpp>
#include <Geode/binding/GJUserScore.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/loader/Event.hpp>
#include <matjson.hpp>

// IDs para identificar nuestros elementos de UI
const char* STREAK_BADGE_ID = "jotabelike.gd_racha/streak-badge-item";
const char* STREAK_STAT_ID = "jotabelike.gd_racha/streak-stat-item";



class $modify(MyProfilePage, ProfilePage) {
    struct Fields {
        EventListener<web::WebTask> m_playerDataListener;
    };

    void onOtherPlayerStreakStatClick(CCObject* sender) {
       
        auto userData = static_cast<CCArray*>(static_cast<CCNode*>(sender)->getUserObject());
        auto username = static_cast<CCString*>(userData->objectAtIndex(0))->getCString();
        int streakDays = static_cast<CCInteger*>(userData->objectAtIndex(1))->getValue();

        
        std::string alertContent = fmt::format(
            "<cy>{}</c> has a streak of\n<cg>{}</c> days!",
            username,
            streakDays
        );
        FLAlertLayer::create("Daily Streak", alertContent, "OK")->show();
    }

    void onBadgeInfoClick(CCObject* sender) {
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

    void onStreakStatClick(CCObject* sender) {
        g_streakData.load();
        std::string alertContent = fmt::format(
            "You have a streak of\n<cy>{}</c> days!",
            g_streakData.currentStreak
        );
        FLAlertLayer::create("Daily Streak", alertContent, "OK")->show();
    }

    void loadPageFromUserInfo(GJUserScore* score) {
        ProfilePage::loadPageFromUserInfo(score);

        
        if (score->m_accountID == GJAccountManager::get()->get()->m_accountID) {
             if (auto statsMenu = m_mainLayer->getChildByIDRecursive("stats-menu")) {
                if (auto oldStat = statsMenu->getChildByID(STREAK_STAT_ID)) { oldStat->removeFromParent(); }

                g_streakData.load();
                auto pointsLabel = CCLabelBMFont::create(std::to_string(g_streakData.totalStreakPoints).c_str(), "bigFont.fnt");
                pointsLabel->setScale(0.6f);
                auto pointIcon = CCSprite::create("streak_point.png"_spr);
                pointIcon->setScale(0.2f);
                auto canvas = CCSprite::create();
                canvas->setContentSize({pointsLabel->getScaledContentSize().width + pointIcon->getScaledContentSize().width + 2.f, 28.0f});
                canvas->setOpacity(0);
                float totalInnerWidth = pointsLabel->getScaledContentSize().width + pointIcon->getScaledContentSize().width + 2.f;
                pointsLabel->setPosition(ccp((canvas->getContentSize().width / 2) - (totalInnerWidth / 2) + (pointsLabel->getScaledContentSize().width / 2), canvas->getContentSize().height / 2));
                pointIcon->setPosition(ccp(pointsLabel->getPositionX() + (pointsLabel->getScaledContentSize().width / 2) + (pointIcon->getScaledContentSize().width / 2) + 2.f, canvas->getContentSize().height / 2));
                canvas->addChild(pointsLabel);
                canvas->addChild(pointIcon);
                auto statItem = CCMenuItemSpriteExtra::create(canvas, this, menu_selector(MyProfilePage::onStreakStatClick));
                statItem->setID(STREAK_STAT_ID);
                statsMenu->addChild(statItem);
                statsMenu->updateLayout();
            }

            if (auto username_menu = m_mainLayer->getChildByIDRecursive("username-menu")) {
                if (auto oldBadge = username_menu->getChildByID(STREAK_BADGE_ID)) { oldBadge->removeFromParent(); }
                g_streakData.load();
                auto equippedBadge = g_streakData.getEquippedBadge();
                if (equippedBadge) {
                    auto badgeSprite = CCSprite::create(equippedBadge->spriteName.c_str());
                    if (badgeSprite) {
                        badgeSprite->setScale(0.2f);
                        auto badgeButton = CCMenuItemSpriteExtra::create(badgeSprite, this, menu_selector(MyProfilePage::onBadgeInfoClick));
                        badgeButton->setUserObject(CCString::create(equippedBadge->badgeID));
                        badgeButton->setID(STREAK_BADGE_ID);
                        username_menu->addChild(badgeButton);
                        username_menu->updateLayout();
                    }
                }
            }
        }
        
        else {
           
            if (auto username_menu = m_mainLayer->getChildByIDRecursive("username-menu")) {
                if (auto oldBadge = username_menu->getChildByID(STREAK_BADGE_ID)) { oldBadge->removeFromParent(); }
            }
            if (auto statsMenu = m_mainLayer->getChildByIDRecursive("stats-menu")) {
                if (auto oldStat = statsMenu->getChildByID(STREAK_STAT_ID)) { oldStat->removeFromParent(); }
            }

            
            std::string url = fmt::format("https://streak-44a83-default-rtdb.firebaseio.com/players/{}.json", score->m_accountID);

            m_fields->m_playerDataListener.bind([this, score](web::WebTask::Event* e) {

               

                if (web::WebResponse* res = e->getValue()) {
                    if (res->ok() && res->json().isOk()) {
                        auto playerData = res->json().unwrap();
                        try {
                            std::string badgeId = playerData["equipped_badge_id"].as<std::string>().unwrap();
                            if (!badgeId.empty()) {
                                auto badgeInfo = g_streakData.getBadgeInfo(badgeId);
                                if (badgeInfo) {
                                    if (auto username_menu = m_mainLayer->getChildByIDRecursive("username-menu")) {
                                        auto badgeSprite = CCSprite::create(badgeInfo->spriteName.c_str());
                                        badgeSprite->setScale(0.2f);
                                        auto badgeButton = CCMenuItemSpriteExtra::create(badgeSprite, this, menu_selector(MyProfilePage::onBadgeInfoClick));
                                        badgeButton->setUserObject(CCString::create(badgeInfo->badgeID));
                                        badgeButton->setID(STREAK_BADGE_ID);
                                        username_menu->addChild(badgeButton);
                                        username_menu->updateLayout();
                                    }
                                }
                            }
                        } catch (const std::exception&) {}
                        try {
                            int totalPoints = playerData["total_streak_points"].as<int>().unwrap();
                            int streakDays = playerData["current_streak"].as<int>().unwrap();
                            if (auto statsMenu = m_mainLayer->getChildByIDRecursive("stats-menu")) {
                                auto pointsLabel = CCLabelBMFont::create(std::to_string(totalPoints).c_str(), "bigFont.fnt");
                                pointsLabel->setScale(0.6f);
                                auto pointIcon = CCSprite::create("streak_point.png"_spr);
                                pointIcon->setScale(0.2f);
                                auto canvas = CCSprite::create();
                                canvas->setContentSize({pointsLabel->getScaledContentSize().width + pointIcon->getScaledContentSize().width + 2.f, 28.0f});
                                canvas->setOpacity(0);
                                float totalInnerWidth = pointsLabel->getScaledContentSize().width + pointIcon->getScaledContentSize().width + 2.f;
                                pointsLabel->setPosition(ccp((canvas->getContentSize().width / 2) - (totalInnerWidth / 2) + (pointsLabel->getScaledContentSize().width / 2), canvas->getContentSize().height / 2));
                                pointIcon->setPosition(ccp(pointsLabel->getPositionX() + (pointsLabel->getScaledContentSize().width / 2) + (pointIcon->getScaledContentSize().width / 2) + 2.f, canvas->getContentSize().height / 2));
                                canvas->addChild(pointsLabel);
                                canvas->addChild(pointIcon);
                                auto statItem = CCMenuItemSpriteExtra::create(canvas, this, menu_selector(MyProfilePage::onOtherPlayerStreakStatClick));
                                statItem->setID(STREAK_STAT_ID);
                                auto userData = CCArray::create();
                                userData->addObject(CCString::create(score->m_userName));
                                userData->addObject(CCInteger::create(streakDays));
                                statItem->setUserObject(userData);
                                statsMenu->addChild(statItem);
                                statsMenu->updateLayout();
                            }
                        } catch (const std::exception&) {}
                    }
                }
            });

            m_fields->m_playerDataListener.setFilter(web::WebRequest().get(url));
        }
    }
};

