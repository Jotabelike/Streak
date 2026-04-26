#include "StreakData.h" 
#include "popups/ProfileCardPopup.h" 
#include "StatusSpinner.h"
#include <Geode/modify/ProfilePage.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/GJAccountManager.hpp>
#include <Geode/binding/GJUserScore.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/utils/async.hpp> 
#include <Geode/loader/Event.hpp>
#include <matjson.hpp>
#include "NameModifiers.h"

using namespace geode::prelude;

const char* STREAK_STAT_ID = "jotabelike.gd_racha/streak-stat-item";
const char* STREAK_BADGE_ID = "jotabelike.gd_racha/streak-badge-item";
const char* LOADING_STAT_ID = "jotabelike.gd_racha/loading-stat";
const char* LOADING_BADGE_ID = "jotabelike.gd_racha/loading-badge";

class StreakUserProfile : public CCObject {
public:
    ProfileData data;
    static StreakUserProfile* create(ProfileData d) {
        auto ret = new StreakUserProfile();
        ret->data = d;
        ret->autorelease();
        return ret;
    }
};

class $modify(MyProfilePage, ProfilePage) {
    struct Fields {
        async::TaskHolder<web::WebResponse> m_remoteDataListener;
    };

    CCMenuItem* createSpinnerItem(const char* id, float scale = 0.5f) {
        auto spinner = StatusSpinner::create();
        spinner->setLoading("");
        spinner->setScale(scale);

        auto dummyNode = CCNode::create();
        dummyNode->setContentSize({ 30.f, 30.f });

        auto container = CCMenuItemSprite::create(
            dummyNode,
            nullptr,
            nullptr
        );
        container->setContentSize({ 30.f, 30.f });
        container->setID(id);

        spinner->setPosition(container->getContentSize() / 2);
        container->addChild(spinner);

        return container;
    }

    void onOtherStreakStatClick(CCObject * sender) {
        auto btn = static_cast<CCNode*>(sender);
        if (auto userProfile = static_cast<StreakUserProfile*>(btn->getUserObject("user-profile"_spr))) {
            ProfileCardPopup::create(
                userProfile->data
            )->show();
        }
    }

    void onBadgeInfoClick(CCObject * sender) {
        auto node = static_cast<CCNode*>(sender);
        if (auto badgeID = static_cast<CCString*>(node->getUserObject("badge"_spr))) {
            if (auto badgeInfo = g_streakData.getBadgeInfo(badgeID->getCString())) {
                FLAlertLayer::create(
                    badgeInfo->displayName.c_str(),
                    "Badge",
                    "OK"
                )->show();
            }
        }
    }

    void onStreakStatClick(CCObject* sender) {
        g_streakData.load();

        ProfileData myData;
        myData.accountID = GJAccountManager::sharedState()->m_accountID;
        myData.username = GJAccountManager::sharedState()->m_username;
        myData.currentStreak = g_streakData.currentStreak;
        myData.totalSP = g_streakData.totalStreakPoints;
        myData.badgeID = g_streakData.equippedBadge;
        myData.level = g_streakData.currentLevel;
        myData.currentXP = g_streakData.currentXP;
        myData.superStars = g_streakData.superStars;
        myData.starTickets = g_streakData.starTickets;
        myData.bannerID = g_streakData.equippedBanner;
        myData.streakID = g_streakData.streakID;
        myData.globalRank = g_streakData.globalRank;

      
        myData.nameColor = g_streakData.equippedNameColor;
        myData.nameFont = g_streakData.equippedNameFont;
        myData.nameEffect = g_streakData.equippedNameEffect;
        myData.nameAnimation = g_streakData.equippedNameAnimation;

        myData.isPartialData = false;

        auto badge = g_streakData.getEquippedBadge();
        myData.isMythic = (badge && badge->category == StreakData::BadgeCategory::MYTHIC);

        ProfileCardPopup::create(myData)->show();
    }

    void loadPageFromUserInfo(GJUserScore* score) {
        ProfilePage::loadPageFromUserInfo(score);

        
        if (m_mainLayer) {
            if (auto pLayer = m_mainLayer->getChildByID("main-layer")) {
            
            }
            else {
            
                for (auto child : CCArrayExt<CCNode*>(m_mainLayer->getChildren())) {
                    if (typeid(*child) == typeid(CCLayer)) {
                        child->setID("main-layer");
                        break;
                    }
                }
            }
        }
 
        if (score->m_accountID == GJAccountManager::get()->get()->m_accountID) {

          
            if (m_usernameLabel) {
                NameModifiers::applyFont(m_usernameLabel, g_streakData.equippedNameFont);
                NameModifiers::applyColor(m_usernameLabel, g_streakData.equippedNameColor);
            }

            if (auto statsMenu = m_mainLayer->getChildByIDRecursive("stats-menu")) {
                if (auto oldStat = statsMenu->getChildByID(STREAK_STAT_ID)) oldStat->removeFromParent();
                g_streakData.load();

                auto pointsLabel = CCLabelBMFont::create(std::to_string(g_streakData.totalStreakPoints).c_str(), "bigFont.fnt");
                pointsLabel->setScale(0.6f);

                auto pointIcon = CCSprite::create("streak_point.png"_spr);
                pointIcon->setScale(0.2f);

                auto canvas = CCSprite::create();
                float totalInnerWidth = pointsLabel->getScaledContentSize().width + pointIcon->getScaledContentSize().width + 2.f;

                canvas->setContentSize({ totalInnerWidth, 28.0f });
                canvas->setOpacity(0);

                pointsLabel->setPosition(ccp(
                    (canvas->getContentSize().width / 2) - (totalInnerWidth / 2) + (pointsLabel->getScaledContentSize().width / 2),
                    canvas->getContentSize().height / 2
                ));

                pointIcon->setPosition(ccp(
                    pointsLabel->getPositionX() + (pointsLabel->getScaledContentSize().width / 2) + (pointIcon->getScaledContentSize().width / 2) + 2.f,
                    canvas->getContentSize().height / 2
                ));

                canvas->addChild(pointsLabel);
                canvas->addChild(pointIcon);

                auto statItem = CCMenuItemSpriteExtra::create(
                    canvas,
                    this,
                    menu_selector(MyProfilePage::onStreakStatClick)
                );
                statItem->setID(STREAK_STAT_ID);
                statsMenu->addChild(statItem);
                statsMenu->updateLayout();
            }

           
            if (auto username_menu = m_mainLayer->getChildByIDRecursive("username-menu")) {
                if (auto oldBadge = username_menu->getChildByID(STREAK_BADGE_ID)) oldBadge->removeFromParent();
                g_streakData.load();
                auto equippedBadge = g_streakData.getEquippedBadge();

                if (equippedBadge) {
                    auto badgeSprite = CCSprite::create(equippedBadge->spriteName.c_str());

                    if (badgeSprite) {
                        badgeSprite->setScale(0.2f);
                        auto badgeButton = CCMenuItemSpriteExtra::create(
                            badgeSprite,
                            this,
                            menu_selector(MyProfilePage::onBadgeInfoClick)
                        );
                        badgeButton->setUserObject("badge"_spr, CCString::create(equippedBadge->badgeID));
                        badgeButton->setID(STREAK_BADGE_ID);
                        username_menu->addChild(badgeButton);

                    
                        username_menu->updateLayout();
                    }
                }
            }

         
            if (m_usernameLabel) {
                NameModifiers::applyEffect(m_usernameLabel, g_streakData.equippedNameEffect);
                NameModifiers::applyAnimation(m_usernameLabel, g_streakData.equippedNameAnimation);
            }
        }
      
        else {
            if (auto username_menu = m_mainLayer->getChildByIDRecursive("username-menu")) {
                if (auto oldBadge = username_menu->getChildByID(STREAK_BADGE_ID)) oldBadge->removeFromParent();
                if (auto oldLoad = username_menu->getChildByID(LOADING_BADGE_ID)) oldLoad->removeFromParent();
                username_menu->updateLayout();
            }

            if (auto statsMenu = m_mainLayer->getChildByIDRecursive("stats-menu")) {
                if (auto oldStat = statsMenu->getChildByID(STREAK_STAT_ID)) oldStat->removeFromParent();
                if (auto oldLoad = statsMenu->getChildByID(LOADING_STAT_ID)) oldLoad->removeFromParent();
                statsMenu->addChild(this->createSpinnerItem(LOADING_STAT_ID));
                statsMenu->updateLayout();
            }

            std::string url = fmt::format("https://streak-servidor.onrender.com/players/{}", score->m_accountID);

            auto req = web::WebRequest();
            m_fields->m_remoteDataListener.spawn(
                req.get(url),
                [this, score](web::WebResponse res) {
                    if (auto username_menu = m_mainLayer->getChildByIDRecursive("username-menu")) {
                        if (auto loader = username_menu->getChildByID(LOADING_BADGE_ID)) loader->removeFromParent();
                    }
                    if (auto statsMenu = m_mainLayer->getChildByIDRecursive("stats-menu")) {
                        if (auto loader = statsMenu->getChildByID(LOADING_STAT_ID)) loader->removeFromParent();
                    }

                    if (res.ok() && res.json().isOk()) {
                        auto json = res.json().unwrap();

                      
                        std::string nameFont = json["equipped_name_font"].as<std::string>().unwrapOr("Default");
                        std::string nameColor = json["equipped_name_color"].as<std::string>().unwrapOr("Default");
                        std::string nameAnim = json["equipped_name_animation"].as<std::string>().unwrapOr("None");
                        std::string nameEffect = json["equipped_name_effect"].as<std::string>().unwrapOr("None");

                     
                        if (m_usernameLabel) {
                            NameModifiers::applyFont(m_usernameLabel, nameFont);
                            NameModifiers::applyColor(m_usernameLabel, nameColor);
                        }

                        ProfileData pData;
                        pData.accountID = score->m_accountID;
                        pData.username = score->m_userName;
                        pData.currentStreak = json["current_streak_days"].as<int>().unwrapOr(0);
                        pData.totalSP = json["total_streak_points"].as<int>().unwrapOr(0);

                        std::string badgeId = json["equipped_badge_id"].as<std::string>().unwrapOr(std::string(""));
                        pData.badgeID = badgeId;
                        pData.nameFont = nameFont;
                        pData.nameColor = nameColor;
                        pData.nameEffect = nameEffect;
                        pData.nameAnimation = nameAnim;
                        pData.isPartialData = true;

                  
                        if (!badgeId.empty() && g_streakData.getBadgeInfo(badgeId)) {
                            if (auto username_menu = m_mainLayer->getChildByIDRecursive("username-menu")) {
                                auto badgeSprite = CCSprite::create(g_streakData.getBadgeInfo(badgeId)->spriteName.c_str());
                                badgeSprite->setScale(0.2f);
                                auto badgeButton = CCMenuItemSpriteExtra::create(
                                    badgeSprite,
                                    this,
                                    menu_selector(MyProfilePage::onBadgeInfoClick)
                                );
                                badgeButton->setUserObject("badge"_spr, CCString::create(badgeId));
                                badgeButton->setID(STREAK_BADGE_ID);
                                username_menu->addChild(badgeButton);

                              
                                username_menu->updateLayout();
                            }
                        }

                     
                        if (m_usernameLabel) {
                            NameModifiers::applyEffect(m_usernameLabel, nameEffect);
                            NameModifiers::applyAnimation(m_usernameLabel, nameAnim);
                        }

                        if (auto statsMenu = m_mainLayer->getChildByIDRecursive("stats-menu")) {
                            auto pointsLabel = CCLabelBMFont::create(std::to_string(pData.totalSP).c_str(), "bigFont.fnt");
                            pointsLabel->setScale(0.6f);
                            auto pointIcon = CCSprite::create("streak_point.png"_spr);
                            pointIcon->setScale(0.2f);

                            auto canvas = CCSprite::create();
                            float totalInnerWidth = pointsLabel->getScaledContentSize().width + pointIcon->getScaledContentSize().width + 2.f;
                            canvas->setContentSize({ totalInnerWidth, 28.0f });
                            canvas->setOpacity(0);

                            pointsLabel->setPosition(ccp(
                                (canvas->getContentSize().width / 2) - (totalInnerWidth / 2) + (pointsLabel->getScaledContentSize().width / 2),
                                canvas->getContentSize().height / 2
                            ));
                            pointIcon->setPosition(ccp(
                                pointsLabel->getPositionX() + (pointsLabel->getScaledContentSize().width / 2) + (pointIcon->getScaledContentSize().width / 2) + 2.f,
                                canvas->getContentSize().height / 2
                            ));

                            canvas->addChild(pointsLabel);
                            canvas->addChild(pointIcon);

                            auto statItem = CCMenuItemSpriteExtra::create(
                                canvas,
                                this,
                                menu_selector(MyProfilePage::onOtherStreakStatClick)
                            );
                            statItem->setUserObject("user-profile"_spr, StreakUserProfile::create(pData));
                            statItem->setID(STREAK_STAT_ID);

                            statsMenu->addChild(statItem);
                            statsMenu->updateLayout();
                        }
                    }
                }
            );
        }
    }
};
