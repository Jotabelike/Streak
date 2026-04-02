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
            bannerSprite->setOpacity(120);

           
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

    void loadFromComment(GJComment * comment) {
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

       
        if (accountID == GJAccountManager::sharedState()->m_accountID) {
            if (auto* equippedBadge = g_streakData.getEquippedBadge()) {
                if (equippedBadge->category == StreakData::BadgeCategory::MYTHIC) {
                    this->schedule(schedule_selector(MyColoredCommentCell::updateRainbowEffect));
                }
            }

            if (showBanners) {
                if (auto* equippedBanner = g_streakData.getEquippedBanner()) {
                    this->addBannerSprite(equippedBanner->spriteName);
                }
            }
            return;
        }

        
        std::string cachedBadge = g_streakData.getCachedBadge(accountID);
        std::string cachedBanner = g_streakData.getCachedBanner(accountID);

        if (!cachedBadge.empty() && !cachedBanner.empty()) {
            if (isBadgeMythic(cachedBadge)) {
                this->schedule(schedule_selector(MyColoredCommentCell::updateRainbowEffect));
            }
            if (showBanners && cachedBanner != "none" && cachedBanner != "pending") {
                if (auto bannerInfo = g_streakData.getBannerInfo(cachedBanner)) {
                    this->addBannerSprite(bannerInfo->spriteName);
                }
            }
            return;
        }

         
        g_streakData.cacheUserBadge(accountID, "pending");
        g_streakData.cacheUserBanner(accountID, "pending");
        std::string url = fmt::format("https://streak-servidor.onrender.com/players/{}", accountID);

        auto req = web::WebRequest();
        m_fields->m_cosmeticsCheckListener.spawn(req.get(url), [this, accountID, showBanners](web::WebResponse res) {
            if (res.ok() && res.json().isOk()) {
                auto playerData = res.json().unwrap();

                std::string badgeId = playerData["equipped_badge_id"].as<std::string>().unwrapOr("");
                std::string bannerId = playerData["equipped_banner_id"].as<std::string>().unwrapOr("");

                g_streakData.cacheUserBadge(accountID, badgeId.empty() ? "none" : badgeId);
                g_streakData.cacheUserBanner(accountID, bannerId.empty() ? "none" : bannerId);

                if (this->isBadgeMythic(badgeId)) {
                    this->schedule(schedule_selector(MyColoredCommentCell::updateRainbowEffect));
                }

                if (showBanners && !bannerId.empty()) {
                    if (auto bannerInfo = g_streakData.getBannerInfo(bannerId)) {
                        this->addBannerSprite(bannerInfo->spriteName);
                    }
                }
            }
            else if (res.code() == 404) {
                g_streakData.cacheUserBadge(accountID, "none");
                g_streakData.cacheUserBanner(accountID, "none");
            }
            });
    }

    static void onModify(auto& self) {
        (void)self.setHookPriority("CommentCell::loadFromComment", -100);
    }
};