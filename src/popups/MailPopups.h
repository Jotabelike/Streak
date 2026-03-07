#pragma once
#include "StreakCommon.h"
#include "AdminPopups.h" 
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/ScrollLayer.hpp>
#include <Geode/ui/TextInput.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/utils/async.hpp>

class MailDetailPopup : public Popup {
protected:
    StreakData::MailMessage* m_mail = nullptr;
    std::function<void()> m_onClaimCallback;
    async::TaskHolder<web::WebResponse> m_actionListener;
    CCMenuItemSpriteExtra* m_claimBtn = nullptr;

    void onEnter() override {
        Popup::onEnter();
        CCDirector::sharedDirector()->getTouchDispatcher()->addTargetedDelegate(this, -513, true);
    }

    void onExit() override {
        CCDirector::sharedDirector()->getTouchDispatcher()->removeDelegate(this);
        Popup::onExit();
    }

    bool ccTouchBegan(CCTouch* touch, CCEvent* event) override {
        return true;
    }

    void ccTouchMoved(CCTouch* touch, CCEvent* event) override {}
    void ccTouchEnded(CCTouch* touch, CCEvent* event) override {}
    void ccTouchCancelled(CCTouch* touch, CCEvent* event) override {}

    bool init(StreakData::MailMessage* mail, std::function<void()> callback) {
        if (!Popup::init(340.f, 240.f)) return false;

        m_mail = mail;
        m_onClaimCallback = callback;
        this->setTitle(m_mail->title.c_str());
        auto winSize = m_mainLayer->getContentSize();

        auto textBg = cocos2d::extension::CCScale9Sprite::create("square02_001.png");
        textBg->setContentSize({ 280.f, 90.f });
        textBg->setColor({ 0, 0, 0 }); textBg->setOpacity(60);
        textBg->setPosition({ winSize.width / 2, winSize.height / 2 + 20.f });
        m_mainLayer->addChild(textBg);

        std::string processedBody = m_mail->body;
        size_t pos = 0;
        while ((pos = processedBody.find("\\n", pos)) != std::string::npos) {
            processedBody.replace(pos, 2, "\n");
            pos += 1;
        }

        auto bodyLabel = CCLabelBMFont::create(processedBody.c_str(), "chatFont.fnt");
        bodyLabel->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
        bodyLabel->limitLabelWidth(270.f, 0.6f, 0.1f);
        bodyLabel->setPosition(textBg->getPosition());
        m_mainLayer->addChild(bodyLabel);

        if (m_mail->hasRewards()) {
            CCNode* rewardContainer = CCNode::create();
            rewardContainer->setPosition({ winSize.width / 2, winSize.height / 2 - 45.f });
            m_mainLayer->addChild(rewardContainer);

            std::vector<CCNode*> nodes;

            if (m_mail->rewardStars > 0) nodes.push_back(createRewardNode(
                "super_star.png"_spr,
                m_mail->rewardStars)
            );

            if (m_mail->rewardTickets > 0) nodes.push_back(createRewardNode(
                "star_tiket.png"_spr,
                m_mail->rewardTickets)
            );

            if (!m_mail->rewardBadge.empty()) {
                std::string spriteName = "GJ_secretChest_001.png";
                if (auto badgeInfo = g_streakData.getBadgeInfo(m_mail->rewardBadge)) spriteName = badgeInfo->spriteName;
                nodes.push_back(createRewardNode(spriteName, 0));
            }

            float spacing = 60.f;
            float startX = -((nodes.size() - 1) * spacing) / 2.f;
            for (int i = 0; i < nodes.size(); i++) {
                nodes[i]->setPosition({ startX + (i * spacing), 0 });
                rewardContainer->addChild(nodes[i]);
            }
        }

        auto buttonMenu = CCMenu::create();
        buttonMenu->setID("claim-menu");
        buttonMenu->setPosition({ winSize.width / 2, 30.f });
        m_mainLayer->addChild(buttonMenu);

        updateClaimButtonState();
        buttonMenu->addChild(m_claimBtn);
        return true;
    }

    CCNode* createRewardNode(std::string spriteName, int amount) {
        auto node = CCNode::create();
        auto spr = CCSprite::create(spriteName.c_str());
        if (!spr) spr = CCSprite::createWithSpriteFrameName("GJ_starsIcon_001.png");
        spr->setScale(0.25f);
        node->addChild(spr);

        if (amount > 0) {
            auto lbl = CCLabelBMFont::create(fmt::format("x{}", amount).c_str(), "goldFont.fnt");
            lbl->setScale(0.4f); lbl->setPosition({ 0, -20.f });
            node->addChild(lbl);
        }
        return node;
    }

    void updateClaimButtonState() {
        if (m_claimBtn) m_claimBtn->removeFromParent();
        bool claimed = m_mail->isClaimedLocal;
        std::string txt = claimed ? "Claimed" : "Claim";
        const char* img = claimed ? "GJ_button_02.png" : "GJ_button_01.png";

        auto spr = ButtonSprite::create(txt.c_str(), 0, 0, "goldFont.fnt", img, 0, 0.8f);
        spr->setScale(0.9f);
        m_claimBtn = CCMenuItemSpriteExtra::create(
            spr,
            this,
            menu_selector(MailDetailPopup::onClaim));
        if (claimed) m_claimBtn->setEnabled(false);
    }

    void onClaim(CCObject*) {
        m_claimBtn->setEnabled(false);
        auto am = GJAccountManager::sharedState();
        matjson::Value payload = matjson::Value::object();
        payload.set("type", m_mail->type);

        std::string url = fmt::format("https://streak-servidor.onrender.com/players/{}/mail/{}/claim", am->m_accountID, m_mail->id);

        auto req = web::WebRequest();
        m_actionListener.spawn(req.bodyJSON(payload).post(url), [this](web::WebResponse res) {
            if (res.ok()) {
                FLAlertLayer::create("Success", "Rewards Claimed!", "OK")->show();
                if (!m_mail->isClaimedLocal) {
                    g_streakData.superStars += m_mail->rewardStars;
                    g_streakData.starTickets += m_mail->rewardTickets;
                    g_streakData.save();
                    m_mail->isClaimedLocal = true;
                }
                this->updateClaimButtonState();
                if (auto menu = m_mainLayer->getChildByIDRecursive("claim-menu")) menu->addChild(m_claimBtn);
                if (m_onClaimCallback) m_onClaimCallback();
            }
            else {
                FLAlertLayer::create("Error", "Failed.", "OK")->show();
                m_claimBtn->setEnabled(true);
            }
            });
    }

public:
    static MailDetailPopup* create(StreakData::MailMessage* mail, std::function<void()> callback) {
        auto ret = new MailDetailPopup();
        if (ret && ret->init(mail, callback)) {
            ret->autorelease(); return ret;
        }
        CC_SAFE_DELETE(ret); return nullptr;
    }
};


class WriteMailPopup : public Popup {
protected:
    TextInput* m_targetInput;
    TextInput* m_titleInput;
    TextInput* m_bodyInput;
    TextInput* m_starsInput;
    TextInput* m_ticketsInput;
    TextInput* m_badgeInput;
    CCNode* m_page1 = nullptr;
    CCNode* m_page2 = nullptr;
    int m_currentPage = 0;
    CCMenuItemSpriteExtra* m_leftArrow = nullptr;
    CCMenuItemSpriteExtra* m_rightArrow = nullptr;
    CCMenuItemSpriteExtra* m_sendBtn = nullptr;
    async::TaskHolder<web::WebResponse> m_sendListener;

    bool init() {
        if (!Popup::init(360.f, 260.f)) return false;
        this->setTitle("Compose Mail");
        auto winSize = m_mainLayer->getContentSize();
        m_page1 = CCNode::create();
        m_page1->setContentSize(winSize);
        m_mainLayer->addChild(m_page1);
        m_page2 = CCNode::create();
        m_page2->setContentSize(winSize);
        m_page2->setVisible(false);
        m_mainLayer->addChild(m_page2);

        m_targetInput = TextInput::create(
            140.f,
            "Target ID / 'global'",
            "chatFont.fnt"
        );

        m_targetInput->setPosition({
            winSize.width / 2,
            winSize.height - 60.f
            }
        );
        m_targetInput->setFilter("abcdefghijklmnopqrstuvwxyz0123456789");
        m_page1->addChild(m_targetInput);

        auto targetLabel = CCLabelBMFont::create(
            "Recipient:",
            "goldFont.fnt"
        );

        targetLabel->setScale(0.6f);
        targetLabel->setPosition({
            winSize.width / 2,
            winSize.height - 40.f
            }
        );

        m_page1->addChild(targetLabel);
        m_titleInput = TextInput::create(
            220.f,
            "Subject",
            "chatFont.fnt"
        );

        m_titleInput->setPosition({
            winSize.width / 2,
            winSize.height - 100.f }
            );

        m_titleInput->setMaxCharCount(30);
        m_page1->addChild(m_titleInput);

        auto subjectLabel = CCLabelBMFont::create(
            "Subject:",
            "goldFont.fnt"
        );

        subjectLabel->setScale(0.6f);
        subjectLabel->setPosition({
            winSize.width / 2,
            winSize.height - 80.f
            }
        );

        m_page1->addChild(subjectLabel);
        auto bodyLabel = CCLabelBMFont::create(
            "Message:",
            "goldFont.fnt"
        );

        bodyLabel->setScale(0.6f);
        bodyLabel->setPosition({
            winSize.width / 2,
            winSize.height - 130.f
            }
        );
        m_page1->addChild(bodyLabel);

        m_bodyInput = TextInput::create(
            280.f,
            "Type message here...",
            "chatFont.fnt"
        );

        m_bodyInput->setPosition({
            winSize.width / 2,
            winSize.height / 2 - 40.f }
            );

        m_bodyInput->setMaxCharCount(200);
        m_page1->addChild(m_bodyInput);

        auto rewardsTitle = CCLabelBMFont::create(
            "Attach Rewards",
            "goldFont.fnt"
        );

        rewardsTitle->setPosition({
            winSize.width / 2,
            winSize.height - 45.f
            }
        );

        m_page2->addChild(rewardsTitle);
        float rowY = winSize.height / 2 + 10.f;

        auto starSpr = CCSprite::create(
            "super_star.png"_spr
        );

        starSpr->setScale(0.2f);
        starSpr->setPosition({
            winSize.width / 2 - 80.f,
            rowY + 30.f
            });

        m_page2->addChild(starSpr);

        m_starsInput = TextInput::create(
            80.f,
            "0",
            "chatFont.fnt"
        );

        m_starsInput->setPosition({
            winSize.width / 2 - 80.f,
            rowY }
            );

        m_starsInput->setFilter("0123456789");
        m_page2->addChild(m_starsInput);

        auto tickSpr = CCSprite::create("star_tiket.png"_spr);
        tickSpr->setScale(0.25f);
        tickSpr->setPosition({
            winSize.width / 2 + 80.f,
            rowY + 30.f
            });

        m_page2->addChild(tickSpr);

        m_ticketsInput = TextInput::create(
            80.f,
            "0",
            "chatFont.fnt"
        );

        m_ticketsInput->setPosition({
            winSize.width / 2 + 80.f,
            rowY }
            );

        m_ticketsInput->setFilter("0123456789");
        m_page2->addChild(m_ticketsInput);

        auto badgeLabel = CCLabelBMFont::create("Badge ID (Optional):", "chatFont.fnt");
        badgeLabel->setScale(0.6f);
        badgeLabel->setPosition({
            winSize.width / 2,
            rowY - 45.f
            });

        m_page2->addChild(badgeLabel);

        m_badgeInput = TextInput::create(
            180.f,
            "badge",
            "chatFont.fnt"
        );

        m_badgeInput->setPosition({
            winSize.width / 2 - 30.f,
            rowY - 70.f }
            );

        m_page2->addChild(m_badgeInput);

        auto selectBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create(
            "Select",
            0,
            0,
            "goldFont.fnt",
            "GJ_button_05.png",
            0, 0.6f),
            this,
            menu_selector(WriteMailPopup::onSelectBadge
            )
        );

        auto selectMenu = CCMenu::createWithItem(selectBtn);
        selectMenu->setPosition({
            winSize.width / 2 + 95.f,
            rowY - 70.f
            }
        );

        m_page2->addChild(selectMenu);

        m_sendBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create(
            "Send Mail",
            0,
            0,
            "goldFont.fnt",
            "GJ_button_01.png",
            0,
            0.8f
        ),
            this,
            menu_selector(WriteMailPopup::onSend
            )
        );

        auto sendMenu = CCMenu::createWithItem(m_sendBtn);
        sendMenu->setPosition({
            winSize.width / 2,
            40.f
            });

        m_page2->addChild(sendMenu);

        auto leftSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
        m_leftArrow = CCMenuItemSpriteExtra::create(leftSpr, this, menu_selector(WriteMailPopup::onPrevPage));
        m_leftArrow->setPosition({
            -winSize.width / 2 + 25.f,
            0
            });

        auto rightSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
        rightSpr->setFlipX(true);
        m_rightArrow = CCMenuItemSpriteExtra::create(rightSpr, this, menu_selector(WriteMailPopup::onNextPage));
        m_rightArrow->setPosition({
            winSize.width / 2 - 25.f,
            0
            });
        auto navMenu = CCMenu::create();
        navMenu->addChild(m_leftArrow);
        navMenu->addChild(m_rightArrow);
        navMenu->setPosition(
            winSize.width / 2,
            winSize.height / 2
        );
        m_mainLayer->addChild(navMenu);

        this->updatePageVisibility();

        return true;
    }

    void onSelectBadge(CCObject*) {
        BadgeSelectorPopup::create([this](std::string id) {
            if (m_badgeInput) m_badgeInput->setString(id);
            }
        )->show();
    }

    void onNextPage(CCObject*) {
        if (m_currentPage == 0) {
            m_currentPage = 1; updatePageVisibility();
        }
    }

    void onPrevPage(CCObject*) {
        if (m_currentPage == 1) {
            m_currentPage = 0; updatePageVisibility();
        }
    }

    void updatePageVisibility() {
        m_page1->setVisible(m_currentPage == 0);
        m_page2->setVisible(m_currentPage == 1);
        m_leftArrow->setVisible(m_currentPage == 1);
        m_rightArrow->setVisible(m_currentPage == 0);
        this->setTitle(m_currentPage == 0 ? "Compose Mail" : "Add Rewards");
    }

    void onSend(CCObject*) {
        std::string target = m_targetInput->getString();
        std::string title = m_titleInput->getString();
        std::string body = m_bodyInput->getString();

        if (target.empty() || title.empty() || body.empty()) {
            FLAlertLayer::create("Error",
                "Fill Page 1.",
                "OK")->show();
            return;
        }

        m_sendBtn->setEnabled(false);
        m_sendBtn->setNormalImage(ButtonSprite::create(
            "Sending...",
            0,
            0,
            "goldFont.fnt",
            "GJ_button_01.png",
            0,
            0.8f)
        );

        matjson::Value rewards = matjson::Value::object();
        int stars = m_starsInput->getString().empty() ? 0 : std::stoi(m_starsInput->getString());

        if (stars > 0) rewards.set("super_stars", stars);
        int tickets = m_ticketsInput->getString().empty() ? 0 : std::stoi(m_ticketsInput->getString());

        if (tickets > 0) rewards.set("star_tickets", tickets);
        if (!std::string(
            m_badgeInput->getString()).empty()) rewards.set(
                "badge",
                m_badgeInput->getString()
            );

        matjson::Value payload = matjson::Value::object();

        payload.set(
            "adminID",
            GJAccountManager::sharedState()->m_accountID
        );

        payload.set(
            "targetID",
            target
        );

        payload.set(
            "title",
            title
        );

        payload.set(
            "body",
            body
        );

        payload.set(
            "rewards",
            rewards
        );

        auto req = web::WebRequest();
        m_sendListener.spawn(req.bodyJSON(payload).post("https://streak-servidor.onrender.com/send-mail"), [this](web::WebResponse res) {
            this->onResponse(std::move(res));
            });
    }

    void onResponse(web::WebResponse res) {
        if (res.ok()) {
            FLAlertLayer::create(
                "Success",
                "Mail Sent!",
                "OK")->show();
            this->onClose(nullptr);
        }
        else {
            m_sendBtn->setEnabled(true);
            m_sendBtn->setNormalImage(ButtonSprite::create(
                "Send Mail",
                0,
                0,
                "goldFont.fnt",
                "GJ_button_01.png",
                0,
                0.8f)
            );

            std::string err = "Failed.";
            if (res.json().isOk()) err = res.json().unwrap()["error"].as<std::string>().unwrapOr(err);
            FLAlertLayer::create("Error", err.c_str(), "OK")->show();
        }
    }

public:
    static WriteMailPopup* create() {
        auto ret = new WriteMailPopup();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};


class MailboxPopup : public Popup {
public:
    std::function<void()> m_onCloseCallback;
    void onClose(CCObject* sender) override {
        if (m_onCloseCallback) m_onCloseCallback();
        Popup::onClose(sender);
    }

protected:
    ScrollLayer* m_list;
    async::TaskHolder<web::WebResponse> m_loadListener;
    CCLabelBMFont* m_loadingLabel = nullptr;
    std::vector<StreakData::MailMessage> m_messages;

    bool init() {
        if (!Popup::init(340.f, 220.f, "geode.loader/GE_square01.png")) return false;
        this->setTitle("Inbox");
        auto winSize = m_mainLayer->getContentSize();
        auto listSize = CCSize(300.f, 160.f);
        m_list = ScrollLayer::create(listSize);
        m_list->setPosition((winSize - listSize) / 2);
        auto bg = cocos2d::extension::CCScale9Sprite::create("square02_001.png");
        bg->setContentSize(listSize);
        bg->setColor({ 0, 0, 0 });
        bg->setOpacity(80);
        bg->setPosition(winSize / 2);
        m_mainLayer->addChild(bg);
        m_mainLayer->addChild(m_list);
        m_loadingLabel = CCLabelBMFont::create("Loading...", "bigFont.fnt");
        m_loadingLabel->setPosition(winSize / 2);
        m_loadingLabel->setScale(0.6f);
        m_mainLayer->addChild(m_loadingLabel, 100);

        auto refreshBtn = CCMenuItemSpriteExtra::create(
            CCSprite::createWithSpriteFrameName(
                "GJ_updateBtn_001.png"
            ),
            this,
            menu_selector(MailboxPopup::onRefresh)
        );

        auto refreshMenu = CCMenu::createWithItem(refreshBtn);
        refreshMenu->setPosition({ 25.f, 25.f });
        m_mainLayer->addChild(refreshMenu, 10);

        if (g_streakData.userRole == 2) {
            auto composeBtn = CCMenuItemSpriteExtra::create(
                CCSprite::createWithSpriteFrameName(
                    "GJ_editBtn_001.png"),
                this,
                menu_selector(MailboxPopup::onCompose)
            );

            auto menu = CCMenu::createWithItem(composeBtn);
            menu->setPosition({
                winSize.width - 25.f,
                winSize.height - 25.f
                }
            );
            m_mainLayer->addChild(menu, 10);
        }
        this->loadMail();
        return true;
    }

    void onRefresh(CCObject*) {
        m_list->m_contentLayer->removeAllChildren();
        if (m_loadingLabel) m_loadingLabel->setVisible(true);
        this->loadMail();
    }

    void loadMail() {
        auto am = GJAccountManager::sharedState();
        std::string url = fmt::format("https://streak-servidor.onrender.com/players/{}/mail", am->m_accountID);

        auto req = web::WebRequest();
        m_loadListener.spawn(req.get(url), [this](web::WebResponse res) {
            if (m_loadingLabel) m_loadingLabel->setVisible(false);
            if (res.ok() && res.json().isOk()) {
                m_messages.clear();
                auto data = res.json().unwrap();

                if (data.isArray()) {
                    auto jsonArray = data.as<std::vector<matjson::Value>>().unwrap();
                    for (const auto& m : jsonArray) {
                        StreakData::MailMessage msg;
                        msg.id = m["id"].as<std::string>().unwrapOr("");
                        msg.title = m["title"].as<std::string>().unwrapOr("No Title");
                        msg.body = m["body"].as<std::string>().unwrapOr("");
                        msg.type = m["type"].as<std::string>().unwrapOr("personal");
                        msg.timestamp = m["timestamp"].as<long long>().unwrapOr(0);
                        if (m.contains("rewards")) {
                            auto r = m["rewards"];
                            msg.rewardStars = r["super_stars"].as<int>().unwrapOr(0);
                            msg.rewardTickets = r["star_tickets"].as<int>().unwrapOr(0);
                            msg.rewardBadge = r["badge"].as<std::string>().unwrapOr("");
                        }
                        if (m.contains("claimed")) msg.isClaimedLocal = m["claimed"].as<bool>().unwrapOr(false);
                        m_messages.push_back(msg);
                    }
                }
                this->refreshList();
            }
            else {
                auto errLbl = CCLabelBMFont::create("Error loading", "goldFont.fnt");
                errLbl->setScale(0.6f);
                errLbl->setPosition(m_list->getContentSize() / 2);
                m_list->m_contentLayer->addChild(errLbl);
            }
            });
    }

    void refreshList() {
        m_list->m_contentLayer->removeAllChildren();
        if (m_messages.empty()) {
            auto lbl = CCLabelBMFont::create(
                "Inbox Empty", "goldFont.fnt"
            );

            lbl->setScale(0.7f);
            lbl->setPosition(
                m_list->getContentSize() / 2
            );

            m_list->m_contentLayer->addChild(lbl);
            return;
        }

        float itemHeight = 45.f;
        float totalHeight = std::max(160.f, (float)m_messages.size() * itemHeight);
        m_list->m_contentLayer->setContentSize({ 300.f, totalHeight });
        float yPos = totalHeight - (itemHeight / 2) - 5.f;
        for (int i = 0; i < m_messages.size(); ++i) {

            const auto& msg = m_messages[i];
            auto cellMenu = CCMenu::create(); cellMenu->setPosition(0, 0);
            auto btnSpr = cocos2d::extension::CCScale9Sprite::create("GJ_square02.png");
            btnSpr->setContentSize({ 290.f, 40.f });

            if (msg.isClaimedLocal) {
                btnSpr->setColor({ 150, 150, 150 }
                );

                btnSpr->setOpacity(150);
            }
            auto titleLbl = CCLabelBMFont::create(msg.title.c_str(), "goldFont.fnt");
            titleLbl->setAnchorPoint({ 0, 0.5f });
            titleLbl->setPosition({ 10.f, 25.f });
            titleLbl->setScale(0.55f);
            titleLbl->limitLabelWidth(160.f, 0.55f, 0.1f);
            if (msg.isClaimedLocal) titleLbl->setColor({ 200,200,200 });
            btnSpr->addChild(titleLbl);
            std::string typeTxt = (msg.type == "global") ? "Global" : "Personal";

            auto subLbl = CCLabelBMFont::create(typeTxt.c_str(), "chatFont.fnt");
            subLbl->setAnchorPoint({ 0, 0.5f }); subLbl->setPosition({ 10.f, 12.f });
            subLbl->setScale(0.4f); subLbl->setColor({ 180, 180, 180 });
            btnSpr->addChild(subLbl);
            auto btn = CCMenuItemSpriteExtra::create(btnSpr, this, menu_selector(MailboxPopup::onOpenMail));
            btn->setTag(i);
            btn->setPosition({ 150.f, yPos });
            cellMenu->addChild(btn);
            m_list->m_contentLayer->addChild(cellMenu);
            yPos -= itemHeight;
        }
    }

    void onOpenMail(CCObject* sender) {
        int index = sender->getTag();
        if (index >= 0 && index < m_messages.size()) {
            MailDetailPopup::create(&m_messages[index],
                [this]() {
                    this->refreshList();
                }
            )->show();
        }
    }
    void onCompose(CCObject*) {
        WriteMailPopup::create()->show();
    }

public:
    static MailboxPopup* create() {
        auto ret = new MailboxPopup();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};