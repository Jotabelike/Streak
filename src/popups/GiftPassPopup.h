#pragma once
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/TextInput.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/utils/async.hpp>
#include "../StreakData.h"
#include "../HMACAuth.h"

using namespace geode::prelude;

class GiftPassPopup : public Popup {
public:
    std::function<void()> onGifted = nullptr;

protected:
    static constexpr int GIFT_PRICE = 799;
    TextInput* m_idInput = nullptr;
    CCMenuItemSpriteExtra* m_giftBtn = nullptr;
    async::TaskHolder<web::WebResponse> m_giftTask;
    bool m_busy = false;

    bool init() {
        if (!Popup::init(310.f, 225.f, "geode.loader/GE_square03.png")) return false;
        auto winSize = m_mainLayer->getContentSize();
        this->setTitle("Gift a Pass");

        auto vipSpr = CCSprite::create("vip_pass.png"_spr);
        if (vipSpr) {
            vipSpr->setScale(38.f / std::max(vipSpr->getContentSize().height, 1.f));
            vipSpr->setPosition({ winSize.width / 2.f, winSize.height - 46.f });
            m_mainLayer->addChild(vipSpr);
        }

        auto desc = CCLabelBMFont::create("Enter the player's Streak ID:", "bigFont.fnt");
        desc->setScale(0.4f);
        desc->setPosition({ winSize.width / 2.f, winSize.height / 2.f + 36.f });
        m_mainLayer->addChild(desc);

        m_idInput = TextInput::create(230.f, "STK-...", "chatFont.fnt");
        m_idInput->setPosition({ winSize.width / 2.f, winSize.height / 2.f + 10.f });
        m_idInput->setFilter("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-");
        m_idInput->setMaxCharCount(20);
        m_mainLayer->addChild(m_idInput);

        auto priceLabel = CCLabelBMFont::create("Cost: 799 gems", "goldFont.fnt");
        priceLabel->setScale(0.5f);
        priceLabel->setPosition({ winSize.width / 2.f, winSize.height / 2.f - 24.f });
        m_mainLayer->addChild(priceLabel);

        auto balLabel = CCLabelBMFont::create(
            fmt::format("You have: {} gems", g_streakData.gems).c_str(), "chatFont.fnt");
        balLabel->setScale(0.4f);
        balLabel->setColor({ 180, 180, 180 });
        balLabel->setPosition({ winSize.width / 2.f, winSize.height / 2.f - 44.f });
        m_mainLayer->addChild(balLabel);

        auto btnSpr = ButtonSprite::create("Gift");
        m_giftBtn = CCMenuItemSpriteExtra::create(btnSpr, this, menu_selector(GiftPassPopup::onGift));
        auto menu = CCMenu::createWithItem(m_giftBtn);
        menu->setPosition({ winSize.width / 2.f, 30.f });
        m_mainLayer->addChild(menu);

        auto infoSpr = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
        infoSpr->setScale(0.6f);
        auto infoBtn = CCMenuItemSpriteExtra::create(infoSpr, this, menu_selector(GiftPassPopup::onInfo));
        auto infoMenu = CCMenu::createWithItem(infoBtn);
        infoMenu->setPosition({ winSize.width - 22.f, 22.f });
        m_mainLayer->addChild(infoMenu);

        return true;
    }

    void onInfo(CCObject*) {
        FLAlertLayer::create("Gift a Pass",
            "Gift the <cy>Premium Pass</c> to another player using their <cg>Streak ID</c> for <cy>799 gems</c>.\n"
            "They instantly get premium for the current month, plus a congratulations animation on their next login.",
            "OK")->show();
    }

    void setBusy(bool busy) {
        m_busy = busy;
        if (m_giftBtn) m_giftBtn->setEnabled(!busy);
    }

    void onGift(CCObject*) {
        if (m_busy) return;
        std::string target = m_idInput ? std::string(m_idInput->getString()) : "";
        if (target.empty()) {
            FLAlertLayer::create("Error", "Enter a Streak ID.", "OK")->show();
            return;
        }
        if (g_streakData.gems < GIFT_PRICE) {
            FLAlertLayer::create("Not enough gems",
                fmt::format("You need <cy>{}</c> gems to gift a pass.", GIFT_PRICE).c_str(), "OK")->show();
            return;
        }

        int accountID = GJAccountManager::sharedState()->m_accountID;
        matjson::Value payload = matjson::Value::object();
        payload.set("targetStreakID", target);

        auto req = web::WebRequest();
        HMACAuth::signRequest(req, accountID, payload);
        req.bodyJSON(payload);

        setBusy(true);
        m_giftTask.spawn(
            req.post("https://streak-servidor.onrender.com/streak-pass/gift"),
            [this](web::WebResponse res) { this->onGiftResponse(res); }
        );
    }

    void onGiftResponse(web::WebResponse& res) {
        setBusy(false);
        if (res.ok() && res.json().isOk()) {
            auto data = res.json().unwrap();
            if (data.contains("balances")) {
                g_streakData.gems = data["balances"]["gems"].as<int>().unwrapOr(g_streakData.gems);
            }
            FMODAudioEngine::sharedEngine()->playEffect("buyItem01.ogg");
            if (onGifted) onGifted();
            FLAlertLayer::create("Gift Sent!", "The player received a Premium Pass!", "OK")->show();
            this->onClose(nullptr);
            return;
        }

        std::string msg = "Could not send the gift. Try again.";
        switch (res.code()) {
            case 404: msg = "Streak ID not found."; break;
            case 402: msg = "You don't have enough gems."; break;
            case 409: msg = "That player already has premium this month."; break;
            case 403: msg = "You can't gift a pass to this player."; break;
        }
        FLAlertLayer::create("Error", msg.c_str(), "OK")->show();
    }

public:
    static GiftPassPopup* create() {
        auto ret = new GiftPassPopup();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};
