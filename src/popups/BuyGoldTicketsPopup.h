#pragma once
#include <Geode/ui/Popup.hpp>
#include "../StreakData.h"
#include "../FirebaseManager.h"

using namespace geode::prelude;

class BuyGoldTicketsPopup : public Popup {
public:
    std::function<void()> onPurchased = nullptr;

protected:
    static constexpr int STEP = 5;            // tickets per step
    static constexpr int GEMS_PER_STEP = 10;  // gems for each STEP tickets
    static constexpr int MIN_QTY = 5;
    static constexpr int MAX_QTY = 200;

    int m_qty = 5;
    CCLabelBMFont* m_qtyLabel = nullptr;
    CCLabelBMFont* m_priceLabel = nullptr;
    CCLabelBMFont* m_balanceLabel = nullptr;
    CCMenuItemSpriteExtra* m_buyBtn = nullptr;

    int priceFor(int qty) const { return (qty / STEP) * GEMS_PER_STEP; }

    bool init() {
        if (!Popup::init(290.f, 210.f, "geode.loader/GE_square03.png")) return false;
        auto winSize = m_mainLayer->getContentSize();
        this->setTitle("Buy Gold Tickets");

        auto goldIcon = CCSprite::create("gold_ticket.png"_spr);
        if (goldIcon) {
            goldIcon->setScale(0.22f);
            goldIcon->setPosition({ winSize.width / 2.f, winSize.height - 56.f });
            m_mainLayer->addChild(goldIcon);
        }

        m_qtyLabel = CCLabelBMFont::create("5", "bigFont.fnt");
        m_qtyLabel->setScale(0.8f);
        m_qtyLabel->setPosition({ winSize.width / 2.f, winSize.height / 2.f + 6.f });
        m_mainLayer->addChild(m_qtyLabel);

        auto minusSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
        auto minusBtn = CCMenuItemSpriteExtra::create(
            minusSpr, this, menu_selector(BuyGoldTicketsPopup::onMinus));
        minusBtn->setPosition({ winSize.width / 2.f - 60.f, winSize.height / 2.f + 6.f });

        auto plusSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
        plusSpr->setFlipX(true);
        auto plusBtn = CCMenuItemSpriteExtra::create(
            plusSpr, this, menu_selector(BuyGoldTicketsPopup::onPlus));
        plusBtn->setPosition({ winSize.width / 2.f + 60.f, winSize.height / 2.f + 6.f });

        auto stepMenu = CCMenu::create();
        stepMenu->addChild(minusBtn);
        stepMenu->addChild(plusBtn);
        stepMenu->setPosition({ 0, 0 });
        m_mainLayer->addChild(stepMenu);

        m_priceLabel = CCLabelBMFont::create("", "goldFont.fnt");
        m_priceLabel->setScale(0.55f);
        m_priceLabel->setPosition({ winSize.width / 2.f, winSize.height / 2.f - 28.f });
        m_mainLayer->addChild(m_priceLabel);

        m_balanceLabel = CCLabelBMFont::create("", "chatFont.fnt");
        m_balanceLabel->setScale(0.4f);
        m_balanceLabel->setColor({ 180, 180, 180 });
        m_balanceLabel->setPosition({ winSize.width / 2.f, winSize.height / 2.f - 48.f });
        m_mainLayer->addChild(m_balanceLabel);

        auto buySpr = ButtonSprite::create("Buy");
        m_buyBtn = CCMenuItemSpriteExtra::create(
            buySpr, this, menu_selector(BuyGoldTicketsPopup::onBuy));
        auto buyMenu = CCMenu::createWithItem(m_buyBtn);
        buyMenu->setPosition({ winSize.width / 2.f, 32.f });
        m_mainLayer->addChild(buyMenu);

        updateLabels();
        return true;
    }

    void updateLabels() {
        if (m_qtyLabel) m_qtyLabel->setString(fmt::format("{}", m_qty).c_str());
        if (m_priceLabel) m_priceLabel->setString(fmt::format("{} gems", priceFor(m_qty)).c_str());
        if (m_balanceLabel) m_balanceLabel->setString(fmt::format("You have: {} gems", g_streakData.gems).c_str());
    }

    void onMinus(CCObject*) { m_qty = std::max(MIN_QTY, m_qty - STEP); updateLabels(); }
    void onPlus(CCObject*) { m_qty = std::min(MAX_QTY, m_qty + STEP); updateLabels(); }

    void onBuy(CCObject*) {
        int price = priceFor(m_qty);
        if (g_streakData.gems < price) {
            FLAlertLayer::create("Not enough gems",
                fmt::format("You need <cy>{}</c> gems for {} gold tickets.", price, m_qty).c_str(),
                "OK")->show();
            return;
        }
        if (m_buyBtn) m_buyBtn->setEnabled(false);

        matjson::Value payload = matjson::Value::object();
        payload.set("quantity", m_qty);
        claimOnServer("/gold-tickets/purchase", payload,
            [this, keepAlive = Ref<CCNode>(this)](bool ok) {
                if (m_buyBtn) m_buyBtn->setEnabled(true);
                if (!ok) {
                    FLAlertLayer::create("Error", "Purchase failed. Try again.", "OK")->show();
                    return;
                }
                FMODAudioEngine::sharedEngine()->playEffect("buyItem01.ogg");
                updateLabels();
                if (onPurchased) onPurchased();
                FLAlertLayer::create("Success", "Gold tickets added to your pass!", "OK")->show();
            });
    }

public:
    static BuyGoldTicketsPopup* create() {
        auto ret = new BuyGoldTicketsPopup();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};
