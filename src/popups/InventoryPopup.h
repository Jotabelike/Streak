#pragma once

#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include "../StreakData.h"

using namespace geode::prelude;

class InventoryPopup : public Popup {
protected:
    bool init() {
        if (!Popup::init(400.f, 235.f, "GJ_square04.png")) return false;
        this->setTitle("Inventory", "bigFont.fnt", 0.82f, 23.f);

        auto listBackground = CCScale9Sprite::create("square02_001.png");
        listBackground->setContentSize({ 365.f, 132.f });
        listBackground->setOpacity(115);
        listBackground->setColor({ 24, 38, 58 });
        listBackground->setPosition({ m_size.width / 2.f, 105.f });
        m_mainLayer->addChild(listBackground, 1);

        static constexpr int kDiscounts[] = { 10, 25, 50, 80, 99 };
        std::vector<int> available;
        for (int discount : kDiscounts) {
            if (g_streakData.getDiscountTicketCount(discount) > 0) available.push_back(discount);
        }

        const float gap = 74.f;

        if (!available.empty()) {
            auto sectionTitle = CCLabelBMFont::create("Discount Tickets", "bigFont.fnt");
            sectionTitle->setScale(0.47f);
            sectionTitle->setPosition({ m_size.width / 2.f, m_size.height - 52.f });
            m_mainLayer->addChild(sectionTitle, 3);
        }

        float startX = m_size.width / 2.f - gap * static_cast<float>(static_cast<int>(available.size()) - 1) / 2.f;
        for (size_t i = 0; i < available.size(); ++i) {
            int discount = available[i];
            int count = g_streakData.getDiscountTicketCount(discount);

            auto cell = CCScale9Sprite::create("square02b_001.png");
            cell->setContentSize({ 64.f, 105.f });
            cell->setOpacity(105);
            cell->setColor({ 48, 70, 94 });
            cell->setPosition({ startX + gap * static_cast<float>(i), 105.f });
            m_mainLayer->addChild(cell, 2);

            auto ticket = CCSprite::create(fmt::format("discount_ticket_{}.png"_spr, discount).c_str());
            if (ticket) {
                float maxSide = std::max(ticket->getContentSize().width, ticket->getContentSize().height);
                ticket->setScale(52.f / std::max(1.f, maxSide));
                ticket->setPosition({ 32.f, 67.f });
                if (count <= 0) ticket->setColor({ 120, 120, 120 });
                cell->addChild(ticket);
            }

            auto name = CCLabelBMFont::create(fmt::format("{}% OFF", discount).c_str(), "bigFont.fnt");
            name->setScale(0.31f);
            name->setPosition({ 32.f, 32.f });
            cell->addChild(name);

            auto amount = CCLabelBMFont::create(fmt::format("x{}", count).c_str(), "goldFont.fnt");
            amount->setScale(0.48f);
            amount->setPosition({ 32.f, 12.f });
            cell->addChild(amount);
        }

        if (!available.empty()) {
            auto hint = CCLabelBMFont::create("Select a ticket while confirming a purchase", "chatFont.fnt");
            hint->setScale(0.5f);
            hint->setColor({ 190, 210, 235 });
            hint->setPosition({ m_size.width / 2.f, 25.f });
            m_mainLayer->addChild(hint, 3);
        }
        return true;
    }

public:
    static InventoryPopup* create() {
        auto ret = new InventoryPopup();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};
