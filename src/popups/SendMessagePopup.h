#pragma once
#include "StreakCommon.h"
#include "MailPopups.h"
#include "AdminPopups.h" 
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/TextInput.hpp>
#include <Geode/ui/ScrollLayer.hpp>
#include <Geode/binding/TextArea.hpp>
#include <Geode/utils/async.hpp>
#include "../SystemNotification.h"
#include "../StatusSpinner.h"

using namespace geode::prelude;


class MessageCell : public CCNode {
public:
    static MessageCell* create(const matjson::Value& msgData, float width) {
        auto ret = new MessageCell();
        if (ret && ret->init(msgData, width)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    bool init(const matjson::Value& msgData, float width) {
        if (!CCNode::init()) return false;

        std::string username = msgData["username"].as<std::string>().unwrapOr("Unknown");
        std::string content = msgData["content"].as<std::string>().unwrapOr("...");
        int role = msgData["role"].as<int>().unwrapOr(0);

        size_t pos = 0;
        while ((pos = content.find("\\n", pos)) != std::string::npos) {
            content.replace(pos, 2, "\n");
            pos += 1;
        }

        ccColor3B nameColor = { 255, 255, 0 };
        if (role == 2) nameColor = { 255, 80, 80 };
        else if (role == 1) nameColor = { 255, 160, 50 };

        float contentWidth = width - 24.f;
        auto textArea = TextArea::create(
            content,
            "chatFont.fnt",
            0.6f,
            contentWidth,
            { 0, 1 },
            8.f,
            false
        );
        textArea->setColor({ 255, 255, 255 });
        textArea->setOpacity(255);
        textArea->setAnchorPoint({ 0.f, 1.f });

        auto nameLabel = CCLabelBMFont::create(username.c_str(), "goldFont.fnt");
        nameLabel->setScale(0.55f);
        nameLabel->setColor(nameColor);
        nameLabel->setAnchorPoint({ 0.0f, 0.5f });

        float paddingY = 10.f;
        float gapNameText = 4.f;
        float nameHeight = 15.f;
        float textHeight = textArea->getContentSize().height;

        float totalHeight = paddingY + nameHeight + gapNameText + textHeight + paddingY;

        this->setContentSize({ width, totalHeight });
        this->setAnchorPoint({ 0.5f, 0.5f });

        auto bg = cocos2d::extension::CCScale9Sprite::create("square02_001.png");
        bg->setContentSize({ width, totalHeight });
        bg->setPosition({ width / 2, totalHeight / 2 });
        bg->setColor({ 0, 0, 0 });
        bg->setOpacity(90);
        this->addChild(bg, 0);

        float shiftUp = 5.0f;

        float currentY = totalHeight - paddingY - (nameHeight / 2);
        nameLabel->setPosition({ 12.f, currentY + shiftUp });
        this->addChild(nameLabel, 5);

        float textTopY = totalHeight - paddingY - nameHeight - gapNameText;
        textArea->setPosition({ 12.f, textTopY + shiftUp });
        this->addChild(textArea, 5);

        return true;
    }
};


class SendMessagePopup : public Popup {
protected:
    enum class Mode {
        View,
        Write
    };

    Mode m_currentMode = Mode::View;
    CCNode* m_viewLayer = nullptr;
    CCNode* m_writeLayer = nullptr;
    CCMenuItemSpriteExtra* m_toggleBtn = nullptr;
    CCMenuItemSpriteExtra* m_banBtn = nullptr;
    CCMenuItemSpriteExtra* m_sendBtn = nullptr;
    TextInput* m_textInput = nullptr;
    StatusSpinner* m_spinner = nullptr;
    bool m_isSending = false;
    async::TaskHolder<web::WebResponse> m_sendListener;
    async::TaskHolder<web::WebResponse> m_loadListener;
    async::TaskHolder<web::WebResponse> m_mailCheckListener;
    ScrollLayer* m_scrollLayer = nullptr;
    CCSprite* m_notificationIcon = nullptr;
    long long m_newestMessageTime = 0;
    int m_activeColorTag = 0;

    bool init() {
        if (!Popup::init(360.f, 240.f, "geode.loader/GE_square03.png")) return false;
        this->setTitle("Advertisements");
        auto winSize = m_mainLayer->getContentSize();

        m_viewLayer = CCNode::create();
        m_viewLayer->setContentSize(winSize);
        m_mainLayer->addChild(m_viewLayer);

        auto scrollSize = CCSize(320.f, 150.f);
        m_scrollLayer = ScrollLayer::create(scrollSize);
        m_scrollLayer->setPosition((winSize - scrollSize) / 2 + CCPoint{ 0, 10.f });
        m_viewLayer->addChild(m_scrollLayer);

        auto bg = cocos2d::extension::CCScale9Sprite::create("square02_001.png");
        bg->setContentSize(scrollSize + CCSize(10.f, 10.f));
        bg->setColor({ 0, 0, 0 });
        bg->setOpacity(100);
        bg->setPosition(winSize / 2 + CCPoint{ 0, 10.f });
        m_viewLayer->addChild(bg, -1);

        m_spinner = StatusSpinner::create();
        m_spinner->setPosition(winSize / 2);
        m_mainLayer->addChild(m_spinner, 100);

        auto mailSprite = CCSprite::createWithSpriteFrameName("accountBtn_messages_001.png");
        if (!mailSprite) {
            mailSprite = ButtonSprite::create("Inbox");
        }
        else {
            mailSprite->setScale(0.8f);
        }

        m_notificationIcon = CCSprite::createWithSpriteFrameName("exMark_001.png");
        if (m_notificationIcon) {
            m_notificationIcon->setScale(0.5f);
            m_notificationIcon->setPosition({
                mailSprite->getContentSize().width - 5,
                mailSprite->getContentSize().height - 5
                });
            m_notificationIcon->setVisible(false);
            m_notificationIcon->runAction(CCRepeatForever::create(
                CCSequence::create(
                    CCScaleTo::create(0.5f, 0.6f),
                    CCScaleTo::create(0.5f, 0.5f),
                    nullptr
                )
            ));
            mailSprite->addChild(m_notificationIcon);
        }

        auto mailBtn = CCMenuItemSpriteExtra::create(
            mailSprite,
            this,
            menu_selector(SendMessagePopup::onOpenMailbox)
        );
        auto mailMenu = CCMenu::createWithItem(mailBtn);
        mailMenu->setPosition({ 31.f, 31.f });
        m_mainLayer->addChild(mailMenu, 100);

        m_writeLayer = CCNode::create();
        m_writeLayer->setVisible(false);
        m_mainLayer->addChild(m_writeLayer);

        m_textInput = TextInput::create(
            winSize.width - 60.f,
            "Type a message...",
            "chatFont.fnt"
        );
        m_textInput->setPosition({
            winSize.width / 2,
            winSize.height / 2 - 10.f
            });
        m_textInput->setMaxCharCount(150);
        m_textInput->setFilter(" abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.,!?'\"-()@#&<>\\/");
        m_writeLayer->addChild(m_textInput);

        auto toolsMenu = CCMenu::create();
        toolsMenu->setPosition({
            winSize.width / 2,
            winSize.height / 2 + 35.f
            });
        m_writeLayer->addChild(toolsMenu);

        struct ColorDef {
            ccColor3B color;
            int tag;
            std::string labelStr;
        };

        std::vector<ColorDef> colors = {
            {{255, 80, 80}, 1, "cr"},
            {{255, 160, 50}, 2, "co"},
            {{255, 255, 0}, 3, "cy"},
            {{80, 255, 80}, 4, "cg"},
            {{0, 4, 168}, 5, "cb"},
            {{255, 120, 255}, 6, "cp"}
        };

        float startX = -((colors.size() + 2) * 25.f) / 2.f + 12.5f;
        for (int i = 0; i < colors.size(); ++i) {
            auto spr = CCSprite::createWithSpriteFrameName("GJ_colorBtn_001.png");
            spr->setScale(0.5f);
            spr->setColor(colors[i].color);
            auto btn = CCMenuItemSpriteExtra::create(
                spr,
                this,
                menu_selector(SendMessagePopup::onColorClicked)
            );
            btn->setTag(colors[i].tag);
            btn->setPosition({ startX + i * 25.f, 0 });
            toolsMenu->addChild(btn);

            auto label = CCLabelBMFont::create(colors[i].labelStr.c_str(), "goldFont.fnt");
            label->setScale(0.4f);
            label->setPosition({ startX + i * 25.f, 15.f });
            label->setOpacity(200);
            toolsMenu->addChild(label);
        }

        float buttonsX = startX + colors.size() * 25.f + 15.f;
        auto enterBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create(
                "NL", 20, 0, 0.4f, true, "goldFont.fnt", "GJ_button_01.png", 30.f
            ),
            this,
            menu_selector(SendMessagePopup::onNewLine)
        );
        enterBtn->setPosition({ buttonsX, 0 });
        enterBtn->setScale(0.8f);
        toolsMenu->addChild(enterBtn);

        auto prevBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create(
                "Pre", 25, 0, 0.4f, true, "goldFont.fnt", "GJ_button_01.png", 30.f
            ),
            this,
            menu_selector(SendMessagePopup::onPreview)
        );
        prevBtn->setPosition({ buttonsX + 35.f, 0 });
        prevBtn->setScale(0.8f);
        toolsMenu->addChild(prevBtn);

        m_sendBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create(
                "Send", 0, 0, "goldFont.fnt", "GJ_button_01.png", 0, 0.8f
            ),
            this,
            menu_selector(SendMessagePopup::onSend)
        );

        auto clearBtn = CCMenuItemSpriteExtra::create(
            CCSprite::createWithSpriteFrameName("GJ_trashBtn_001.png"),
            this,
            menu_selector(SendMessagePopup::onClearText)
        );
        clearBtn->setScale(0.7f);

        auto sendMenu = CCMenu::create();
        sendMenu->addChild(clearBtn);
        sendMenu->addChild(m_sendBtn);
        sendMenu->alignItemsHorizontallyWithPadding(10.f);
        sendMenu->setPosition({ winSize.width / 2, 40.f });
        m_writeLayer->addChild(sendMenu);

        if (g_streakData.userRole >= 1) {
            auto editSpr = CCSprite::createWithSpriteFrameName("GJ_chatBtn_001.png");
            editSpr->setScale(0.8f);
            m_toggleBtn = CCMenuItemSpriteExtra::create(
                editSpr,
                this,
                menu_selector(SendMessagePopup::onToggleMode)
            );

            auto hammerSpr = CCSprite::createWithSpriteFrameName("GJ_reportBtn_001.png");
            hammerSpr->setScale(0.8f);
            m_banBtn = CCMenuItemSpriteExtra::create(
                hammerSpr,
                this,
                menu_selector(SendMessagePopup::onOpenBanTool)
            );

            auto modMenu = CCMenu::create();
            modMenu->addChild(m_banBtn);
            modMenu->addChild(m_toggleBtn);
            modMenu->alignItemsHorizontallyWithPadding(8.f);
            modMenu->setPosition({ winSize.width / 2 + 135.f, winSize.height / 2 - 90.f });
            m_mainLayer->addChild(modMenu, 10);
        }

        this->loadMessages();
        this->checkForMail();

        return true;
    }

    void onOpenMailbox(CCObject*) {
        auto popup = MailboxPopup::create();
        popup->m_onCloseCallback = [this]() {
            this->checkForMail();
            };
        popup->show();
    }

    void checkForMail() {
        auto am = GJAccountManager::sharedState();
        auto req = web::WebRequest();
        m_mailCheckListener.spawn(
            req.get(fmt::format("https://streak-servidor.onrender.com/players/{}/mail", am->m_accountID)),
            [this](web::WebResponse res) {
                if (res.ok() && res.json().isOk()) {
                    auto data = res.json().unwrap();
                    if (data.isArray() && !data.as<std::vector<matjson::Value>>().unwrap().empty() && m_notificationIcon) {
                        m_notificationIcon->setVisible(true);
                    }
                }
            }
        );
    }

    void onOpenBanTool(CCObject*) {
        BanUserPopup::create()->show();
    }

    void onToggleMode(CCObject*) {
        m_currentMode = (m_currentMode == Mode::View) ? Mode::Write : Mode::View;
        m_viewLayer->setVisible(m_currentMode == Mode::View);
        m_writeLayer->setVisible(m_currentMode == Mode::Write);
        this->setTitle(m_currentMode == Mode::View ? "Advertisements" : "Write Message");

        if (m_currentMode == Mode::View) {
            this->loadMessages();
            if (m_banBtn) m_banBtn->setVisible(true);
        }
        else {
            m_activeColorTag = 0;
            if (m_banBtn) m_banBtn->setVisible(false);
        }
    }

    void onClearText(CCObject*) {
        if (m_textInput) m_textInput->setString("");
    }

    void insertText(std::string text) {
        if (m_textInput) m_textInput->setString(m_textInput->getString() + text);
    }

    void onColorClicked(CCObject* sender) {
        int tagIdx = sender->getTag();
        if (m_activeColorTag == tagIdx) {
            insertText("</c>");
            m_activeColorTag = 0;
        }
        else {
            if (m_activeColorTag != 0) insertText("</c>");
            std::string tag = "";
            switch (tagIdx) {
            case 1: tag = "<cr>"; break;
            case 2: tag = "<co>"; break;
            case 3: tag = "<cy>"; break;
            case 4: tag = "<cg>"; break;
            case 5: tag = "<cb>"; break;
            case 6: tag = "<cp>"; break;
            }
            insertText(tag);
            m_activeColorTag = tagIdx;
        }
    }

    void onNewLine(CCObject*) {
        insertText("\\n");
    }

    void onPreview(CCObject*) {
        std::string text = m_textInput->getString();
        if (text.empty()) text = " ";
        std::string processed = text;
        size_t pos = 0;
        while ((pos = processed.find("\\n", pos)) != std::string::npos) {
            processed.replace(pos, 2, "\n");
            pos += 1;
        }
        FLAlertLayer::create("Preview", processed, "OK")->show();
    }

    void loadMessages() {
        m_scrollLayer->setVisible(false);
        m_spinner->setLoading("Loading...");
        auto req = web::WebRequest();
        m_loadListener.spawn(req.get("https://streak-servidor.onrender.com/messages"), [this](web::WebResponse res) {
            this->onLoadResponse(std::move(res));
            });
    }

    void onLoadResponse(web::WebResponse res) {
        if (res.ok() && res.json().isOk()) {
            m_spinner->hide();
            m_scrollLayer->setVisible(true);

            auto data = res.json().unwrap();
            if (data.isArray()) {
                this->populateMessages(data.as<std::vector<matjson::Value>>().unwrap());
            }
        }
        else {
            m_spinner->setError("Failed to load");
        }
    }

    void populateMessages(const std::vector<matjson::Value>& messages) {
        m_scrollLayer->m_contentLayer->removeAllChildren();
        auto sortedMessages = messages;
        std::reverse(sortedMessages.begin(), sortedMessages.end());

        float scrollWidth = m_scrollLayer->getContentSize().width;
        float cellWidth = 300.f;
        float gapBetweenCells = 6.f;
        float topPadding = 10.f;
        float bottomPadding = 10.f;

        std::vector<MessageCell*> cells;
        float totalContentHeight = topPadding;

        for (const auto& msgData : sortedMessages) {
            long long timestamp = msgData["timestamp"].as<long long>().unwrapOr(0);
            if (timestamp > m_newestMessageTime) m_newestMessageTime = timestamp;

            auto cell = MessageCell::create(msgData, cellWidth);
            cells.push_back(cell);

            totalContentHeight += cell->getContentSize().height + gapBetweenCells;
        }

        if (!cells.empty()) totalContentHeight = totalContentHeight - gapBetweenCells + bottomPadding;

        float finalHeight = std::max(m_scrollLayer->getContentSize().height, totalContentHeight);
        m_scrollLayer->m_contentLayer->setContentSize({ scrollWidth, finalHeight });

        float currentYCursor = finalHeight - topPadding;

        for (auto* cell : cells) {
            float cellH = cell->getContentSize().height;

            cell->setPosition({ scrollWidth / 2.f, currentYCursor - (cellH / 2.f) });

            m_scrollLayer->m_contentLayer->addChild(cell);

            currentYCursor -= (cellH + gapBetweenCells);
        }

        m_scrollLayer->moveToTop();

        if (m_newestMessageTime > 0) {
            Mod::get()->setSavedValue<double>("streak_last_chat_time", (double)m_newestMessageTime);
        }
    }

    void onSend(CCObject*) {
        if (m_isSending) return;
        if (g_streakData.userRole == 1 && g_streakData.dailyMsgCount >= 3) {
            FLAlertLayer::create("Limit Reached", "Daily limit reached (3/3).", "OK")->show();
            return;
        }

        std::string msg = m_textInput->getString();
        if (msg.length() < 1) {
            FLAlertLayer::create("Error", "Message empty.", "OK")->show();
            return;
        }

        m_isSending = true;
        if (m_sendBtn) m_sendBtn->setEnabled(false);

        auto am = GJAccountManager::sharedState();
        matjson::Value payload = matjson::Value::object();
        payload.set("accountID", am->m_accountID);
        payload.set("username", std::string(am->m_username));
        payload.set("content", msg);

        auto req = web::WebRequest();
        m_sendListener.spawn(
            req.bodyJSON(payload).post("https://streak-servidor.onrender.com/messages"),
            [this](web::WebResponse res) { this->onWebResponse(std::move(res)); }
        );
    }

    void onWebResponse(web::WebResponse res) {
        m_isSending = false;
        if (m_sendBtn) m_sendBtn->setEnabled(true);

        if (res.ok()) {
            if (g_streakData.userRole == 1) {
                g_streakData.dailyMsgCount++;
                g_streakData.save();
            }
            m_textInput->setString("");
            this->onToggleMode(nullptr);
        }
        else {
            FLAlertLayer::create("Error", fmt::format("Failed: {}", res.code()), "OK")->show();
        }
    }

public:
    static SendMessagePopup* create() {
        auto ret = new SendMessagePopup();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};