#pragma once

#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include "../StreakData.h"

using namespace geode::prelude;

enum class PurchaseCurrency {
    Gems,
    StarTickets,
    SuperStars
};

inline int discountedPurchasePrice(int basePrice, int discountPercent) {
    if (basePrice <= 0) return 0;
    int pct = std::clamp(discountPercent, 0, 99);
    auto numerator = static_cast<long long>(basePrice) * (100 - pct) + 99;
    return std::max(1, static_cast<int>(numerator / 100));
}

inline const char* purchaseCurrencySprite(PurchaseCurrency currency) {
    switch (currency) {
        case PurchaseCurrency::StarTickets: return "star_tiket.png"_spr;
        case PurchaseCurrency::SuperStars:  return "super_star.png"_spr;
        default:                            return "gem.png"_spr;
    }
}

class DiscountTicketPickerPopup : public Popup {
protected:
    std::function<void(int)> m_onSelect;

    bool init(std::function<void(int)> onSelect) {
        if (!Popup::init(390.f, 205.f, "GJ_square04.png")) return false;
        m_onSelect = std::move(onSelect);
        this->setTitle("Discount Tickets", "bigFont.fnt", 0.72f, 22.f);

        std::vector<int> available;
        static constexpr int kDiscounts[] = { 10, 25, 50, 80, 99 };
        for (int discount : kDiscounts) {
            if (g_streakData.getDiscountTicketCount(discount) > 0) available.push_back(discount);
        }

        auto hint = CCLabelBMFont::create(
            available.empty() ? "No discount tickets available" : "Choose one ticket for this purchase",
            "goldFont.fnt"
        );
        hint->setScale(0.42f);
        hint->setPosition({ m_size.width / 2.f, m_size.height - 50.f });
        m_mainLayer->addChild(hint);

        auto ticketMenu = CCMenu::create();
        ticketMenu->setPosition({ 0.f, 0.f });
        m_mainLayer->addChild(ticketMenu);

        const float gap = 70.f;
        float startX = m_size.width / 2.f - gap * static_cast<float>(static_cast<int>(available.size()) - 1) / 2.f;
        for (size_t i = 0; i < available.size(); ++i) {
            int discount = available[i];
            int count = g_streakData.getDiscountTicketCount(discount);

            auto cell = CCScale9Sprite::create("square02b_001.png");
            cell->setContentSize({ 61.f, 94.f });
            cell->setOpacity(count > 0 ? 125 : 70);
            cell->setColor(count > 0 ? ccColor3B{ 44, 67, 92 } : ccColor3B{ 75, 75, 75 });

            auto ticket = CCSprite::create(fmt::format("discount_ticket_{}.png"_spr, discount).c_str());
            if (ticket) {
                float maxSide = std::max(ticket->getContentSize().width, ticket->getContentSize().height);
                ticket->setScale(48.f / std::max(1.f, maxSide));
                ticket->setPosition({ 30.5f, 58.f });
                if (count <= 0) ticket->setColor({ 120, 120, 120 });
                cell->addChild(ticket);
            }

            auto percent = CCLabelBMFont::create(fmt::format("{}% OFF", discount).c_str(), "bigFont.fnt");
            percent->setScale(0.32f);
            percent->setPosition({ 30.5f, 27.f });
            cell->addChild(percent);

            auto amount = CCLabelBMFont::create(fmt::format("x{}", count).c_str(), "goldFont.fnt");
            amount->setScale(0.42f);
            amount->setPosition({ 30.5f, 10.f });
            cell->addChild(amount);

            auto button = CCMenuItemSpriteExtra::create(cell, this, menu_selector(DiscountTicketPickerPopup::onTicket));
            button->setTag(discount);
            button->setPosition({ startX + gap * static_cast<float>(i), 103.f });
            ticketMenu->addChild(button);
        }

        auto noneSprite = ButtonSprite::create("No Ticket", 0, false, "goldFont.fnt", "GJ_button_06.png", 0, 0.65f);
        auto noneButton = CCMenuItemSpriteExtra::create(noneSprite, this, menu_selector(DiscountTicketPickerPopup::onTicket));
        noneButton->setTag(0);
        noneButton->setPosition({ m_size.width / 2.f, 28.f });
        ticketMenu->addChild(noneButton);
        return true;
    }

    void onTicket(CCObject* sender) {
        int discount = static_cast<CCNode*>(sender)->getTag();
        auto callback = m_onSelect;
        this->onClose(nullptr);
        if (callback) callback(discount);
    }

public:
    static DiscountTicketPickerPopup* create(std::function<void(int)> onSelect) {
        auto ret = new DiscountTicketPickerPopup();
        if (ret && ret->init(std::move(onSelect))) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

class PurchaseConfirmPopup : public Popup {
protected:
    int m_basePrice = 0;
    int m_discountPercent = 0;
    PurchaseCurrency m_currency = PurchaseCurrency::Gems;
    std::function<void(int)> m_onBuy;
    CCLabelBMFont* m_priceLabel = nullptr;
    CCLabelBMFont* m_basePriceLabel = nullptr;
    CCSprite* m_discountSprite = nullptr;

    bool init(const std::string& spriteName, const std::string& displayName, int price,
              PurchaseCurrency currency, std::function<void(int)> onBuy) {
        if (!Popup::init(280.f, 220.f, "GJ_square04.png")) return false;
        m_basePrice = std::max(0, price);
        m_currency = currency;
        m_onBuy = std::move(onBuy);
        this->setTitle("Confirm Purchase", "bigFont.fnt", 0.78f, 23.f);

        auto sprite = CCSprite::create(spriteName.c_str());
        if (sprite) {
            float maxSide = std::max(sprite->getContentSize().width, sprite->getContentSize().height);
            sprite->setScale(68.f / std::max(1.f, maxSide));
            sprite->setPosition({ m_size.width / 2.f, 145.f });
            m_mainLayer->addChild(sprite);
        }

        auto nameLabel = CCLabelBMFont::create(displayName.c_str(), "bigFont.fnt");
        nameLabel->setScale(0.48f);
        nameLabel->limitLabelWidth(205.f, 0.48f, 0.2f);
        nameLabel->setPosition({ m_size.width / 2.f, 103.f });
        m_mainLayer->addChild(nameLabel);

        auto priceBg = CCScale9Sprite::create("square02b_001.png");
        priceBg->setContentSize({ 130.f, 38.f });
        priceBg->setOpacity(120);
        priceBg->setColor({ 35, 55, 75 });
        priceBg->setPosition({ m_size.width / 2.f, 72.f });
        m_mainLayer->addChild(priceBg);

        auto currencyIcon = CCSprite::create(purchaseCurrencySprite(currency));
        if (currencyIcon) {
            float maxSide = std::max(currencyIcon->getContentSize().width, currencyIcon->getContentSize().height);
            currencyIcon->setScale(23.f / std::max(1.f, maxSide));
            currencyIcon->setPosition({ 40.f, 19.f });
            priceBg->addChild(currencyIcon);
        }

        m_priceLabel = CCLabelBMFont::create(std::to_string(m_basePrice).c_str(), "bigFont.fnt");
        m_priceLabel->setScale(0.55f);
        m_priceLabel->setAnchorPoint({ 0.f, 0.5f });
        m_priceLabel->setPosition({ 58.f, 20.f });
        priceBg->addChild(m_priceLabel);

        m_basePriceLabel = CCLabelBMFont::create("", "goldFont.fnt");
        m_basePriceLabel->setScale(0.31f);
        m_basePriceLabel->setPosition({ m_size.width / 2.f, 48.f });
        m_mainLayer->addChild(m_basePriceLabel);

        auto menu = CCMenu::create();
        menu->setPosition({ m_size.width / 2.f, 25.f });
        m_mainLayer->addChild(menu);

        auto cancelSprite = ButtonSprite::create("Cancel", 0, false, "goldFont.fnt", "GJ_button_06.png", 0, 0.7f);
        auto cancelButton = CCMenuItemSpriteExtra::create(cancelSprite, this, menu_selector(PurchaseConfirmPopup::onClose));
        cancelButton->setPosition({ -68.f, 0.f });
        menu->addChild(cancelButton);

        auto buySprite = ButtonSprite::create("Buy", 0, false, "goldFont.fnt", "GJ_button_01.png", 0, 0.7f);
        auto buyButton = CCMenuItemSpriteExtra::create(buySprite, this, menu_selector(PurchaseConfirmPopup::onConfirmBuy));
        buyButton->setPosition({ 68.f, 0.f });
        menu->addChild(buyButton);

        auto discountMenu = CCMenu::create();
        discountMenu->setPosition({ 0.f, 0.f });
        m_mainLayer->addChild(discountMenu, 20);

        auto discountSprite = CCSprite::create("discount_ticket_btn.png"_spr);
        if (discountSprite) {
            m_discountSprite = discountSprite;
            float maxSide = std::max(discountSprite->getContentSize().width, discountSprite->getContentSize().height);
            discountSprite->setScale(55.f / std::max(1.f, maxSide));
            auto discountButton = CCMenuItemSpriteExtra::create(
                discountSprite, this, menu_selector(PurchaseConfirmPopup::onChooseDiscount));
            discountButton->setPosition({ m_size.width + 24.f, m_size.height - 24.f });
            discountMenu->addChild(discountButton);
        }

        updatePrice();
        return true;
    }

    void updatePrice() {
        int finalPrice = discountedPurchasePrice(m_basePrice, m_discountPercent);
        if (m_priceLabel) m_priceLabel->setString(std::to_string(finalPrice).c_str());

        if (m_discountSprite) {
            std::string selectedSprite = m_discountPercent > 0
                ? fmt::format("discount_ticket_{}.png"_spr, m_discountPercent)
                : std::string("discount_ticket_btn.png"_spr);
            if (auto sprite = CCSprite::create(selectedSprite.c_str())) {
                m_discountSprite->setTexture(sprite->getTexture());
            }
        }

        if (m_basePriceLabel) {
            m_basePriceLabel->setString(m_discountPercent > 0
                ? fmt::format("Base {}  -  {}% off", m_basePrice, m_discountPercent).c_str()
                : "");
        }
    }

    void onChooseDiscount(CCObject*) {
        auto picker = DiscountTicketPickerPopup::create(
            [this, keepAlive = Ref<CCNode>(this)](int discount) {
                m_discountPercent = discount;
                updatePrice();
            });
        if (picker) picker->show();
    }

    void onConfirmBuy(CCObject* sender) {
        auto callback = m_onBuy;
        int discount = m_discountPercent;
        this->onClose(sender);
        if (callback) callback(discount);
    }

public:
    static PurchaseConfirmPopup* create(const std::string& spriteName, const std::string& displayName,
                                        int price, PurchaseCurrency currency,
                                        std::function<void(int)> onBuy) {
        auto ret = new PurchaseConfirmPopup();
        if (ret && ret->init(spriteName, displayName, price, currency, std::move(onBuy))) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};
