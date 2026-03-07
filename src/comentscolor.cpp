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
        async::TaskHolder<web::WebResponse> m_mythicCheckListener;
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
        if (badgeID.empty() || badgeID == "none") return false;
        if (auto badgeInfo = g_streakData.getBadgeInfo(badgeID)) {
            return badgeInfo->category == StreakData::BadgeCategory::MYTHIC;
        }
        return false;
    }

    void loadFromComment(GJComment * comment) {
        CommentCell::loadFromComment(comment);

        this->unschedule(schedule_selector(MyColoredCommentCell::updateRainbowEffect));

        int accountID = comment->m_accountID;
        if (accountID <= 0) return;

        if (accountID == GJAccountManager::sharedState()->m_accountID) {
            if (auto* equippedBadge = g_streakData.getEquippedBadge()) {
                if (equippedBadge->category == StreakData::BadgeCategory::MYTHIC) {
                    this->schedule(schedule_selector(MyColoredCommentCell::updateRainbowEffect));
                }
            }
            return;
        }

        std::string cachedBadge = g_streakData.getCachedBadge(accountID);

        if (!cachedBadge.empty()) {
            if (isBadgeMythic(cachedBadge)) {
                this->schedule(schedule_selector(MyColoredCommentCell::updateRainbowEffect));
            }
            return;
        }

        std::string url = fmt::format("https://streak-servidor.onrender.com/players/{}", accountID);

        auto req = web::WebRequest();
        m_fields->m_mythicCheckListener.spawn(req.get(url), [this, accountID](web::WebResponse res) {
            if (res.ok() && res.json().isOk()) {
                auto playerData = res.json().unwrap();

                std::string badgeId = playerData["equipped_badge_id"].as<std::string>().unwrapOr("");

                if (badgeId.empty()) {
                    g_streakData.cacheUserBadge(accountID, "none");
                }
                else {
                    g_streakData.cacheUserBadge(accountID, badgeId);
                }

                if (this->isBadgeMythic(badgeId)) {
                    this->schedule(schedule_selector(MyColoredCommentCell::updateRainbowEffect));
                }
            }
            else if (res.code() == 404) {
                g_streakData.cacheUserBadge(accountID, "none");
            }
            });
    }

    static void onModify(auto& self) {
        (void)self.setHookPriority("CommentCell::loadFromComment", -100);
    }
};