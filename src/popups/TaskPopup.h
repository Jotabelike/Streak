#pragma once
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/ScrollLayer.hpp>
#include <Geode/ui/TextInput.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/utils/async.hpp>  
#include <Geode/binding/CCTextInputNode.hpp>
#include "../StreakData.h"
#include "../BadgeNotification.h"
#include "../RewardNotification.h"
#include "../BannerNotification.h"
#include "../HMACAuth.h"

using namespace geode::prelude;

class SubmitInputPopup : public Popup {
protected:
    TextInput* m_input;
    std::string m_taskID;
    std::function<void()> m_callback;
    CCMenuItemSpriteExtra* m_sendBtn = nullptr;
    bool m_isSending = false;

   
    async::TaskHolder<web::WebResponse> m_reqTask;

    bool init(std::string taskID, std::string title) {
     
        if (!Popup::init(280.f, 180.f, "geode.loader/GE_square03.png")) return false;

        m_taskID = taskID;
        this->setTitle(title);

        CCPoint center = m_size / 2;

        m_input = TextInput::create(200.0f, "Paste video link here...", "chatFont.fnt");
        m_input->setPosition({ center.x, center.y + 10.0f });
        m_input->ignoreAnchorPointForPosition(false);
        m_input->setFilter("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-._~:/?#[]@!$&'()*+,;=");
        m_mainLayer->addChild(m_input);

        auto btnSpr = ButtonSprite::create(
            "Send",
            "goldFont.fnt",
            "GJ_button_01.png",
            0.8f
        );
        m_sendBtn = CCMenuItemSpriteExtra::create(
            btnSpr,
            this,
            menu_selector(SubmitInputPopup::onSend)
        );

        auto menu = CCMenu::create();
        menu->addChild(m_sendBtn);
        menu->setPosition({ center.x, center.y - 45.0f });
        m_mainLayer->addChild(menu);

        return true;
    }

    void onSend(CCObject*) {
        if (m_isSending) return;

        std::string url = m_input->getString();
        if (url.length() < 5) {
            FLAlertLayer::create("Error", "Invalid Link", "OK")->show();
            return;
        }

        m_isSending = true;
        if (m_sendBtn) {
            m_sendBtn->setEnabled(false);
            auto spr = ButtonSprite::create("Sending...", "goldFont.fnt", "GJ_button_01.png", 0.8f);
            m_sendBtn->setNormalImage(spr);
        }

        auto am = GJAccountManager::sharedState();
        std::string endpoint = "https://streak-servidor.onrender.com/tasks/submit";

        matjson::Value body = matjson::Value::object();
        body.set("accountID", am->m_accountID);
        body.set("username", std::string(am->m_username));
        body.set("taskID", m_taskID);
        body.set("url", url);

        auto req = web::WebRequest();
        HMACAuth::signRequest(req, am->m_accountID, body);
        req.bodyJSON(body);

        m_reqTask.spawn(
            req.post(endpoint),
            [this](web::WebResponse res) {
                if (res.ok()) {
                    g_streakData.setTaskStatus(m_taskID, "pending");
                    if (m_callback) {
                        m_callback();
                    }
                    this->onClose(nullptr);
                    FLAlertLayer::create("Success", "Sent for review!", "OK")->show();
                }
                else {
                    m_isSending = false;
                    if (m_sendBtn) {
                        m_sendBtn->setEnabled(true);
                        auto spr = ButtonSprite::create("Send", "goldFont.fnt", "GJ_button_01.png", 0.8f);
                        m_sendBtn->setNormalImage(spr);
                    }
                    FLAlertLayer::create("Error", "Failed to send.", "OK")->show();
                }
            }
        );
    }

public:
    static SubmitInputPopup* create(std::string taskID, std::function<void()> callback) {
        auto ret = new SubmitInputPopup();
        ret->m_callback = callback;
        if (ret && ret->init(taskID, "Send Proof")) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

struct TaskDef {
    std::string id;
    std::string title;
    std::string description;
    int stars = 0;
    std::string badgeID = "";
    std::string bannerID = "";
    std::string url = "";
};

class TaskCell : public CCLayerColor {
protected:
    TaskDef m_data;
    std::function<void()> m_reloadFunc;

    bool init(const TaskDef& data, float width, std::function<void()> reloadCallback) {
        if (!CCLayerColor::init()) return false;
        m_data = data;
        m_reloadFunc = reloadCallback;

        float height = 55.0f;
        this->setContentSize({ width, height });
        this->setAnchorPoint({ 0, 0 });

        auto bg = cocos2d::extension::CCScale9Sprite::create("square02_small.png");
        bg->setContentSize({ width, height - 2.0f });
        bg->setOpacity(70);
        bg->setPosition({ width / 2, height / 2 });
        this->addChild(bg, 0);

        if (!m_data.bannerID.empty()) {
            std::string bannerSprite = "banner1.png"_spr;
            if (auto bInfo = g_streakData.getBannerInfo(m_data.bannerID)) {
                bannerSprite = bInfo->spriteName;
            }

            auto spr = CCSprite::create(bannerSprite.c_str());
            if (spr) {
                float targetH = height - 4.0f;
                float scale = targetH / spr->getContentSize().height;
                spr->setScale(scale);
                spr->setOpacity(255);
                spr->setAnchorPoint({ 1.0f, 0.5f });
                spr->setPosition({ width - 5.0f, height / 2 });
                spr->setColor({ 200, 200, 200 });
                this->addChild(spr, 1);
            }
        }

        float contentRightEdge = width - 110.0f;

        std::string status = g_streakData.getTaskStatus(data.id);
        auto menu = CCMenu::create();
        menu->setPosition({ 0, 0 });
        this->addChild(menu, 15);

        CCPoint actionBtnPos = { width - 35.0f, height / 2 };

        if (status == "" || status == "none") {
            auto btnSpr = ButtonSprite::create("Send", 40, true, "goldFont.fnt", "GJ_button_01.png", 30.0f, 0.6f);
            auto btn = CCMenuItemSpriteExtra::create(btnSpr, this, menu_selector(TaskCell::onSendClick));
            btn->setPosition(actionBtnPos);
            menu->addChild(btn);
        }
        else if (status == "pending") {
            auto clock = CCSprite::createWithSpriteFrameName("GJ_timeIcon_001.png");
            clock->setScale(0.6f);
            clock->setPosition({ width - 25.0f, height / 2 + 8.0f });
            this->addChild(clock, 15);

            auto lbl = CCLabelBMFont::create("Pending...", "goldFont.fnt");
            lbl->setScale(0.35f);
            lbl->setPosition({ width - 25.0f, height / 2 - 12.0f });
            this->addChild(lbl, 15);

            auto editSpr = CCSprite::create("edit_btn_link.png"_spr);
            if (!editSpr) editSpr = CCSprite::create("edit_btn_link.png");

            if (editSpr) {
                editSpr->setScale(0.7f);
                auto editBtn = CCMenuItemSpriteExtra::create(editSpr, this, menu_selector(TaskCell::onEditClick));
            
                editBtn->setPosition({ width - 70.0f, height / 2 });
                menu->addChild(editBtn);
            }
        }
        else if (status == "approved") {
            auto btnSpr = ButtonSprite::create("Claim", 40, true, "goldFont.fnt", "GJ_button_02.png", 30.0f, 0.6f);
            auto btn = CCMenuItemSpriteExtra::create(btnSpr, this, menu_selector(TaskCell::onClaimClick));
            btn->setPosition(actionBtnPos);
            menu->addChild(btn);
        }
        else if (status == "rejected" || status == "reject") {
            auto xIcon = CCSprite::createWithSpriteFrameName("GJ_deleteIcon_001.png");
            xIcon->setScale(0.7f);
            xIcon->setPosition({ width - 25.0f, height / 2 + 10.0f });
            this->addChild(xIcon, 15);

            auto lbl = CCLabelBMFont::create("Rejected", "bigFont.fnt");
            lbl->setScale(0.3f);
            lbl->setColor({ 255, 100, 100 });
            lbl->setPosition({ width - 25.0f, height / 2 - 15.0f });
            this->addChild(lbl, 15);

            auto trashSpr = CCSprite::createWithSpriteFrameName("edit_delBtn_001.png");
            trashSpr->setScale(0.7f);
            auto trashBtn = CCMenuItemSpriteExtra::create(trashSpr, this, menu_selector(TaskCell::onResetClick));
            trashBtn->setPosition({ width - 60.0f, height / 2 });
            menu->addChild(trashBtn);
        }
        else if (status == "claimed") {
            auto check = CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
            check->setScale(0.7f);
            check->setPosition({ width - 25.0f, height / 2 });
            this->addChild(check, 15);
        }

      
        if (!m_data.badgeID.empty()) {
            std::string badgeSprite = "reward5.png"_spr;
            if (auto bInfo = g_streakData.getBadgeInfo(m_data.badgeID)) {
                badgeSprite = bInfo->spriteName;
            }

            auto spr = CCSprite::create(badgeSprite.c_str());
            if (spr) {
                spr->setScale(0.30f);
              
                spr->setPosition({ width - 110.0f, height / 2 });
                this->addChild(spr, 15);

                if (contentRightEdge > width - 125.0f) {
                    contentRightEdge = width - 125.0f;  
                }
            }
        }

        if (m_data.stars > 0) {
            float starX = contentRightEdge - 15.0f;

            auto starSpr = CCSprite::create("super_star.png"_spr);
            if (!starSpr) {
                starSpr = CCSprite::createWithSpriteFrameName("GJ_starsIcon_001.png");
            }

            starSpr->setScale(0.15f);
            starSpr->setPosition({ starX, height / 2 + 8.0f });
            this->addChild(starSpr, 15);

            auto starLabel = CCLabelBMFont::create(
                fmt::format("+{}", m_data.stars).c_str(),
                "bigFont.fnt"
            );
            starLabel->setScale(0.3f);
            starLabel->setAnchorPoint({ 0.5f, 0.5f });
            starLabel->setPosition({ starX, height / 2 - 12.0f });
            this->addChild(starLabel, 15);

            contentRightEdge -= 35.0f;
        }

        float maxTextWidth = contentRightEdge - 15.0f;

        auto title = CCLabelBMFont::create(data.title.c_str(), "goldFont.fnt");
        title->setScale(0.55f);
        title->setAnchorPoint({ 0, 0.5f });
        title->setPosition({ 10.0f, height - 18.0f });

        if (title->getScaledContentSize().width > maxTextWidth) {
            title->setScale(
                title->getScale() * (maxTextWidth / title->getScaledContentSize().width)
            );
        }
        this->addChild(title, 15);

        auto desc = CCLabelBMFont::create(data.description.c_str(), "chatFont.fnt");
        desc->setScale(0.4f);
        desc->setAnchorPoint({ 0, 0.5f });
        desc->setColor({ 220, 220, 220 });
        desc->setPosition({ 10.0f, 18.0f });

        if (desc->getScaledContentSize().width > maxTextWidth) {
            desc->setScale(
                desc->getScale() * (maxTextWidth / desc->getScaledContentSize().width)
            );
        }
        this->addChild(desc, 15);

        return true;
    }

    void onSendClick(CCObject*) {
        SubmitInputPopup::create(m_data.id, m_reloadFunc)->show();
    }

     void onEditClick(CCObject*) {
     
        SubmitInputPopup::create(m_data.id, m_reloadFunc)->show();
    }

    void onResetClick(CCObject*) {
        auto am = GJAccountManager::sharedState();
        std::string url = "https://streak-servidor.onrender.com/tasks/reset";
        matjson::Value body = matjson::Value::object();
        body.set("accountID", am->m_accountID);
        body.set("taskID", m_data.id);

        std::string safeTaskID = m_data.id;
        auto safeReloadFunc = m_reloadFunc;

        auto req = web::WebRequest();
        HMACAuth::signRequest(req, am->m_accountID, body);
        req.bodyJSON(body);

        async::spawn(
            req.post(url),
            [safeTaskID, safeReloadFunc](web::WebResponse res) {
                if (res.ok()) {
                    g_streakData.setTaskStatus(safeTaskID, "");
                    if (safeReloadFunc) {
                        safeReloadFunc();
                    }
                }
            }
        );
    }

    void onClaimClick(CCObject*) {
        if (m_data.stars > 0) {
            g_streakData.addXP(m_data.stars);
            int startStars = g_streakData.superStars;
            g_streakData.superStars += m_data.stars;
            RewardNotification::show("super_star.png"_spr, startStars, m_data.stars);
        }

        if (!m_data.badgeID.empty()) {
            g_streakData.unlockBadge(m_data.badgeID);
            BadgeNotification::show(m_data.badgeID);
        }

        if (!m_data.bannerID.empty()) {
            g_streakData.unlockBanner(m_data.bannerID);
            auto bInfo = g_streakData.getBannerInfo(m_data.bannerID);
            if (bInfo) {
                BannerNotification::show(
                    m_data.bannerID,
                    bInfo->spriteName,
                    bInfo->displayName,
                    g_streakData.getCategoryName(bInfo->rarity),
                    g_streakData.getCategoryColor(bInfo->rarity)
                );
            }
        }

        g_streakData.setTaskStatus(m_data.id, "claimed");
        g_streakData.save();

        if (m_reloadFunc) {
            m_reloadFunc();
        }
        FMODAudioEngine::sharedEngine()->playEffect("buyItem01.ogg");
    }

public:
    static TaskCell* create(const TaskDef& data, float width, std::function<void()> reloadCallback) {
        auto ret = new TaskCell();
        if (ret && ret->init(data, width, reloadCallback)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

class TaskPopup : public Popup {
protected:
    ScrollLayer* m_list = nullptr;

    bool init() override {
 
        if (!Popup::init(340.f, 210.f, "geode.loader/GE_square03.png")) return false;

        this->setTitle("Special Tasks");
        float listWidth = 300.0f;
        float listHeight = 140.0f;

        auto listBg = cocos2d::extension::CCScale9Sprite::create("square02_001.png");
        listBg->setContentSize({ listWidth + 10.0f, listHeight + 10.0f });
        listBg->setOpacity(60);
        listBg->setPosition({
            m_size.width / 2,
            (m_size.height / 2) - 10.0f
            });
        m_mainLayer->addChild(listBg);

        m_list = ScrollLayer::create({ listWidth, listHeight });
        m_list->setPosition({
            (m_size.width - listWidth) / 2,
            (m_size.height - listHeight) / 2 - 10.0f
            });
        m_mainLayer->addChild(m_list);

        this->refreshList();
        return true;
    }

    void refreshList() {
        if (!m_list) return;
        m_list->m_contentLayer->removeAllChildren();

        std::vector<TaskDef> tasks = {
            {
                "task_yt",
                "Youtube",
                "Upload a video or short to YouTube about the mod",
                100,
                "youtube_badge",
                "banner_24",
                ""
            },
            {
                "task_tk",
                "Tiktok",
                "Upload a video to TikTok about the mod",
                100,
                "tiktok_badge",
                "banner_25",
                ""
            }
        };

        float cellHeight = 57.0f;
        m_list->m_contentLayer->setContentHeight(
            std::max(140.0f, tasks.size() * cellHeight)
        );
        float currentY = m_list->m_contentLayer->getContentHeight() - cellHeight;

        for (const auto& task : tasks) {
            auto cell = TaskCell::create(
                task,
                300.0f,
                [this]() {
                    this->refreshList();
                }
            );
            cell->setPosition({ 0, currentY });
            m_list->m_contentLayer->addChild(cell);
            currentY -= cellHeight;
        }
        m_list->moveToTop();
    }

public:
    static TaskPopup* create() {
        auto ret = new TaskPopup();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};