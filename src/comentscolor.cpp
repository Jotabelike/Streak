#include <Geode/modify/CommentCell.hpp>
#include <cocos2d.h>
#include <Geode/binding/GJComment.hpp>
#include <Geode/binding/TextArea.hpp>
#include "StreakData.h"
#include <Geode/binding/GJAccountManager.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/utils/async.hpp>
#include <Geode/loader/Event.hpp>
#include <matjson.hpp>
#include "NameModifiers.h"
using namespace geode::prelude;

class $modify(MyColoredCommentCell, CommentCell) {
    struct Fields {
        float m_time = 0.f;
        async::TaskHolder<web::WebResponse> m_cosmeticsCheckListener;
    };

    void updateRainbowEffect(float dt) {
        m_fields->m_time += dt;

        float r = (sin(m_fields->m_time * 0.7f) + 1.0f) / 2.0f;
        float g = (sin(m_fields->m_time * 0.7f + 2.0f * M_PI / 3.0f) + 1.0f) / 2.0f;
        float b = (sin(m_fields->m_time * 0.7f + 4.0f * M_PI / 3.0f) + 1.0f) / 2.0f;

        ccColor3B color = {
            (GLubyte)(r * 255),
            (GLubyte)(g * 255),
            (GLubyte)(b * 255)
        };

        CCNode* textObject = nullptr;

        if (auto emojiLabel = this->m_mainLayer->getChildByIDRecursive("thesillydoggo.comment_emojis/comment-text-label")) {
            textObject = emojiLabel;
        }
        else if (auto standardLabel = this->m_mainLayer->getChildByIDRecursive("comment-text-label")) {
            textObject = standardLabel;
        }
        else if (auto textArea = this->m_mainLayer->getChildByIDRecursive("comment-text-area")) {
            textObject = textArea;
        }

        if (textObject) {
            if (auto label = typeinfo_cast<CCLabelBMFont*>(textObject)) {
                label->setColor(color);
            }
            else if (auto textArea = typeinfo_cast<TextArea*>(textObject)) {
                textArea->setColor(color);
                textArea->colorAllCharactersTo(color);
            }
        }
    }

    bool isBadgeMythic(const std::string & badgeID) {
        if (badgeID.empty() || badgeID == "none" || badgeID == "pending") return false;
        if (auto badgeInfo = g_streakData.getBadgeInfo(badgeID)) {
            return badgeInfo->category == StreakData::BadgeCategory::MYTHIC;
        }
        return false;
    }

    void addBannerSprite(const std::string & spriteName) {
        CCNode* bg = this->getChildByID("background");
        if (!bg && this->getChildrenCount() > 0) {
            bg = typeinfo_cast<CCNode*>(this->getChildren()->objectAtIndex(0));
        }

        if (!bg) return;

        
        auto bannerSprite = CCSprite::create(spriteName.c_str());  
        if (!bannerSprite) {
            bannerSprite = CCSprite::createWithSpriteFrameName(spriteName.c_str());
        }

        if (bannerSprite) {
            double opacityMult = Mod::get()->getSavedValue<double>("banner_opacity", 1.0);
            bannerSprite->setOpacity(static_cast<GLubyte>(255 * opacityMult)); 

           
            auto bgSize = bg->getContentSize();
            bannerSprite->setAnchorPoint({ 0.5f, 0.5f });
            bannerSprite->setPosition({ bgSize.width / 2.f, bgSize.height / 2.f });

            auto bannerSize = bannerSprite->getContentSize();

        
            if (bannerSize.width > 0 && bannerSize.height > 0) {
                bannerSprite->setScaleX(bgSize.width / bannerSize.width);
                bannerSprite->setScaleY(bgSize.height / bannerSize.height);
            }

            bannerSprite->setID("streak-banner-bg");

           
            bg->addChild(bannerSprite, 1);
        }
        else {
            log::error("Streak! Mod: The banner image could not be found.: {}", spriteName);
        }
    }

    void applyNameModifiers(const std::string& color, 
        const std::string& font,
        const std::string& effect, 
        const std::string& anim)
    {
       
        bool showNameEffects = Mod::get()->getSavedValue<bool>("enable_name_effects", true);

        CCLabelBMFont* usernameLabel = nullptr;

        if (auto menu = this->m_mainLayer->getChildByIDRecursive("username-menu")) {
            if (auto btn = menu->getChildByID("username-button")) {
                if (btn->getChildrenCount() > 0) {
                    usernameLabel = typeinfo_cast<CCLabelBMFont*>(btn->getChildren()->objectAtIndex(0));
                }
            }
        }

        if (!usernameLabel) {
            usernameLabel = typeinfo_cast<CCLabelBMFont*>(this->m_mainLayer->getChildByIDRecursive("username-label"));
        }

        if (usernameLabel) {
            usernameLabel->stopAllActions();
            usernameLabel->setRotation(0.f);

           
            if (!showNameEffects) {
                usernameLabel->setFntFile("goldFont.fnt");
                return;
            }
 
            if (font == "Default" || font == "None" || font.empty()) {
                usernameLabel->setFntFile("goldFont.fnt");
            }
            else {
                NameModifiers::applyFont(usernameLabel, font);
            }
            if (color != "Default" && color != "None" && !color.empty()) {
                NameModifiers::applyColor(usernameLabel, color);
            }
            if (anim != "Default" && anim != "None" && !anim.empty()) {
                NameModifiers::applyAnimation(usernameLabel, anim);
            }
            if (effect != "Default" && effect != "None" && !effect.empty()) {
                NameModifiers::applyEffect(usernameLabel, effect);
            }
        }
    }

    void loadFromComment(GJComment* comment) {
        CommentCell::loadFromComment(comment);

        this->unschedule(schedule_selector(MyColoredCommentCell::updateRainbowEffect));

        CCNode* bg = this->getChildByID("background");
        if (!bg && this->getChildrenCount() > 0) {
            bg = typeinfo_cast<CCNode*>(this->getChildren()->objectAtIndex(0));
        }

        if (bg) {
            if (auto oldBanner = bg->getChildByID("streak-banner-bg")) {
                oldBanner->removeFromParent();
            }
        }

        int accountID = comment->m_accountID;
        if (accountID <= 0) return;

        bool showBanners = Mod::get()->getSavedValue<bool>("enable_comment_banners", true);
         
        bool showRainbow = Mod::get()->getSavedValue<bool>("enable_rainbow_effect", true);

     
        if (accountID == GJAccountManager::sharedState()->m_accountID) {
            if (auto* equippedBadge = g_streakData.getEquippedBadge()) {
              
                if (equippedBadge->category == StreakData::BadgeCategory::MYTHIC && showRainbow) {
                    this->schedule(schedule_selector(MyColoredCommentCell::updateRainbowEffect));
                }
            }

            if (showBanners) {
                if (auto* equippedBanner = g_streakData.getEquippedBanner()) {
                    this->addBannerSprite(equippedBanner->spriteName);
                }
            }

            this->applyNameModifiers(g_streakData.equippedNameColor,
                g_streakData.equippedNameFont,
                g_streakData.equippedNameEffect,
                g_streakData.equippedNameAnimation);
            return;
        }

      
        std::string cachedBadge = g_streakData.getCachedBadge(accountID);
        std::string cachedBanner = g_streakData.getCachedBanner(accountID);

        if (!cachedBadge.empty() && !cachedBanner.empty() && cachedBadge != "pending") {
            if (isBadgeMythic(cachedBadge) && showRainbow) {
                this->schedule(schedule_selector(MyColoredCommentCell::updateRainbowEffect));
            }
            if (showBanners && cachedBanner != "none") {
                if (auto bannerInfo = g_streakData.getBannerInfo(cachedBanner)) {
                    this->addBannerSprite(bannerInfo->spriteName);
                }
            }

            auto cachedNames = g_streakData.getCachedNameCosmetics(accountID);
            this->applyNameModifiers(cachedNames.color, cachedNames.font, cachedNames.effect, cachedNames.animation);
            return;
        }

        g_streakData.cacheUserBadge(accountID, "pending");
        g_streakData.cacheUserBanner(accountID, "pending");
        this->applyNameModifiers("Default", "Default", "None", "None");
        std::string url = fmt::format("https://streak-servidor.onrender.com/players/{}", accountID);
        auto req = web::WebRequest();
        m_fields->m_cosmeticsCheckListener.spawn(req.get(url), [this, accountID, showBanners](web::WebResponse res) {
         
            bool showRainbowCb = Mod::get()->getSavedValue<bool>("enable_rainbow_effect", true);

            if (res.ok() && res.json().isOk()) {
                auto playerData = res.json().unwrap();

                std::string badgeId = playerData["equipped_badge_id"].as<std::string>().unwrapOr("");
                std::string bannerId = playerData["equipped_banner_id"].as<std::string>().unwrapOr("");
                std::string nameColor = playerData["equipped_name_color"].as<std::string>().unwrapOr("Default");
                std::string nameFont = playerData["equipped_name_font"].as<std::string>().unwrapOr("Default");
                std::string nameEffect = playerData["equipped_name_effect"].as<std::string>().unwrapOr("None");
                std::string nameAnim = playerData["equipped_name_animation"].as<std::string>().unwrapOr("None");

                g_streakData.cacheUserBadge(accountID, badgeId.empty() ? "none" : badgeId);
                g_streakData.cacheUserBanner(accountID, bannerId.empty() ? "none" : bannerId);
                g_streakData.cacheUserNameCosmetics(accountID, nameColor, nameFont, nameEffect, nameAnim);

            
                if (this->isBadgeMythic(badgeId) && showRainbowCb) {
                    this->schedule(schedule_selector(MyColoredCommentCell::updateRainbowEffect));
                }

                if (showBanners && !bannerId.empty()) {
                    if (auto bannerInfo = g_streakData.getBannerInfo(bannerId)) {
                        this->addBannerSprite(bannerInfo->spriteName);
                    }
                }

                this->applyNameModifiers(nameColor, nameFont, nameEffect, nameAnim);
            }
            else if (res.code() == 404) {
                g_streakData.cacheUserBadge(accountID, "none");
                g_streakData.cacheUserBanner(accountID, "none");
                g_streakData.cacheUserNameCosmetics(accountID, "Default", "Default", "None", "None");
                this->applyNameModifiers("Default", "Default", "None", "None");
            }
            });
    }

    static void onModify(auto& self) {
        (void)self.setHookPriority("CommentCell::loadFromComment", -100);
    }
};