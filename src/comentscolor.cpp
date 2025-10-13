#include <Geode/modify/CommentCell.hpp>
#include <cocos2d.h>
#include <Geode/binding/GJComment.hpp>
#include <Geode/binding/TextArea.hpp>
#include "StreakData.h"
#include <Geode/binding/GJAccountManager.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/loader/Event.hpp>
#include <matjson.hpp>

using namespace geode::prelude;

class $modify(MyColoredCommentCell, CommentCell) {
    struct Fields {
        float m_time = 0.f;
        
        EventListener<web::WebTask> m_mythicCheckListener;
    };

    void updateRainbowEffect(float dt) {
        m_fields->m_time += dt;

        float r = (sin(m_fields->m_time * 0.7f) + 1.0f) / 2.0f;
        float g = (sin(m_fields->m_time * 0.7f + 2.0f * M_PI / 3.0f) + 1.0f) / 2.0f;
        float b = (sin(m_fields->m_time * 0.7f + 4.0f * M_PI / 3.0f) + 1.0f) / 2.0f;
        ccColor3B color = { (GLubyte)(r * 255), (GLubyte)(g * 255), (GLubyte)(b * 255) };

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
            if (auto label = dynamic_cast<CCLabelBMFont*>(textObject)) {
                label->setColor(color);
            }
            else if (auto textArea = dynamic_cast<TextArea*>(textObject)) {
                textArea->setColor(color);
                textArea->colorAllCharactersTo(color);
            }
        }
    }

    void loadFromComment(GJComment * comment) {
        CommentCell::loadFromComment(comment);

        
        this->unschedule(schedule_selector(MyColoredCommentCell::updateRainbowEffect));

        int accountID = comment->m_accountID;

       
        if (accountID > 0 && accountID == GJAccountManager::sharedState()->m_accountID) {
            g_streakData.load();
            
            if (auto* equippedBadge = g_streakData.getEquippedBadge()) {
                if (equippedBadge->category == StreakData::BadgeCategory::MYTHIC) {
                    this->schedule(schedule_selector(MyColoredCommentCell::updateRainbowEffect));
                }
            }
        }
       
        else if (accountID > 0) {
            std::string url = fmt::format(
                "https://streak-44a83-default-rtdb.firebaseio.com/players/{}.json",
                accountID
            );

            m_fields->m_mythicCheckListener.bind([this](web::WebTask::Event* e) {
                if (web::WebResponse* res = e->getValue()) {
                    if (res->ok() && res->json().isOk()) {
                        try {
                            auto playerData = res->json().unwrap();
                            
                            bool hasMythic = playerData["has_mythic_color"].as<bool>().unwrapOr(false);
                            if (hasMythic) {
                               
                                this->schedule(schedule_selector(MyColoredCommentCell::updateRainbowEffect));
                            }
                        }
                        catch (const std::exception&) {
                           
                        }
                    }
                }
                });

            
            auto req = web::WebRequest();
            m_fields->m_mythicCheckListener.setFilter(req.get(url));
        }
    }

    static void onModify(auto& self) {
        (void)self.setHookPriority("CommentCell::loadFromComment", -100);
    }
};

