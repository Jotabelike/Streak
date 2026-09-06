#pragma once
#include <Geode/ui/Popup.hpp>
#include <Geode/utils/general.hpp>
#include "../StreakData.h"
#include "../FirebaseManager.h"
#include <ctime>
#include "../BadgeNotification.h"
#include "../BannerNotification.h"
#include "../RewardNotification.h"
#include "PurchaseConfirmPopup.h"

using namespace geode::prelude;
class DailyShopPopup : public Popup {
protected:
    CCLabelBMFont* m_gemLabel = nullptr;
    CCLabelBMFont* m_timerLabel = nullptr;
    CCNode* m_cosmeticContainer = nullptr;
    CCNode* m_consumableContainer = nullptr;
    std::vector<StreakData::ShopItem> m_dailyItems;
    std::vector<StreakData::ConsumableItem> m_dailyConsumables;

    void updateTimer(float dt) {
        if (!m_timerLabel) return;
        time_t now = time(nullptr);
        std::tm t = geode::localtime(now);
        int currentSeconds = (t.tm_hour * 3600) + (t.tm_min * 60) + t.tm_sec;
        int remaining = 86400 - currentSeconds;
        if (remaining < 0) remaining = 0;
        int h = remaining / 3600;
        int m = (remaining % 3600) / 60;
        int s = remaining % 60;
        m_timerLabel->setString(fmt::format("{:02d}:{:02d}:{:02d}", h, m, s).c_str());
    }

    void refreshItems() {
        float cellSpacing = 100.0f;

        if (m_cosmeticContainer) {
            m_cosmeticContainer->removeAllChildren();
            size_t count = m_dailyItems.size();
            float startX = -((float)(count - 1) * cellSpacing) / 2.0f;

            for (int i = 0; i < (int)count; i++) {
                auto cell = createItemCell(m_dailyItems[i], i);
                cell->setPosition({ startX + (i * cellSpacing), 0 });
                m_cosmeticContainer->addChild(cell);
            }

            if (m_dailyItems.empty()) {
                auto emptyLabel = CCLabelBMFont::create("No cosmetics today", "bigFont.fnt");
                emptyLabel->setScale(0.35f);
                m_cosmeticContainer->addChild(emptyLabel);
            }
        }

        if (m_consumableContainer) {
            m_consumableContainer->removeAllChildren();
            size_t count = m_dailyConsumables.size();
            float startX = -((float)(count - 1) * cellSpacing) / 2.0f;

            for (int i = 0; i < (int)count; i++) {
                auto cell = createConsumableCell(m_dailyConsumables[i], i);
                cell->setPosition({ startX + (i * cellSpacing), 0 });
                m_consumableContainer->addChild(cell);
            }

            if (m_dailyConsumables.empty()) {
                auto emptyLabel = CCLabelBMFont::create("No consumables today", "bigFont.fnt");
                emptyLabel->setScale(0.35f);
                m_consumableContainer->addChild(emptyLabel);
            }
        }
    }

    bool init() override {
        if (!Popup::init(360.f, 280.f, "geode.loader/GE_square03.png")) return false;
        auto size = m_mainLayer->getContentSize();
        this->setTitle("Daily Shop");

        m_dailyItems = g_streakData.getDailyShopSelection();
        m_dailyConsumables = g_streakData.getDailyConsumableSelection();

        auto gemSprite = CCSprite::create("gem.png"_spr);
        gemSprite->setScale(0.2f);
        gemSprite->setPosition({ size.width - 25.f, size.height - 25.f });
        m_mainLayer->addChild(gemSprite);

        m_gemLabel = CCLabelBMFont::create(std::to_string(g_streakData.gems).c_str(), "goldFont.fnt");
        m_gemLabel->setScale(0.55f);
        m_gemLabel->setAnchorPoint({ 1.0f, 0.5f });
        m_gemLabel->setPosition({ size.width - 40.f, size.height - 25.f });
        m_mainLayer->addChild(m_gemLabel);

        auto clockIcon = CCSprite::createWithSpriteFrameName("GJ_timeIcon_001.png");
        clockIcon->setScale(0.4f);
        clockIcon->setPosition({ 20.f, 14.f });
        m_mainLayer->addChild(clockIcon);

        m_timerLabel = CCLabelBMFont::create("00:00:00", "bigFont.fnt");
        m_timerLabel->setScale(0.3f);
        m_timerLabel->setAnchorPoint({ 0.0f, 0.5f });
        m_timerLabel->setPosition({ 35.f, 14.f });
        m_timerLabel->setColor({ 200, 255, 255 });
        m_mainLayer->addChild(m_timerLabel);

        this->updateTimer(0);
        this->schedule(schedule_selector(DailyShopPopup::updateTimer), 1.0f);

        float topRowY = size.height / 2 + 50.f;
        float bottomRowY = size.height / 2 - 68.f;
        float separatorY = size.height / 2 - 8.f;

        m_cosmeticContainer = CCNode::create();
        m_cosmeticContainer->setPosition({ size.width / 2, topRowY });
        m_mainLayer->addChild(m_cosmeticContainer);

        auto separator = CCLayerColor::create({ 255, 255, 255, 40 });
        separator->setContentSize({ size.width - 40.f, 1.f });
        separator->setPosition({ 20.f, separatorY });
        m_mainLayer->addChild(separator);

        m_consumableContainer = CCNode::create();
        m_consumableContainer->setPosition({ size.width / 2, bottomRowY });
        m_mainLayer->addChild(m_consumableContainer);

        this->refreshItems();

        return true;
    }

    CCNode* createItemCell(const StreakData::ShopItem& item, int index) {
        auto container = CCNode::create();

        auto bg = CCScale9Sprite::create("GJ_square02.png");
        bg->setContentSize({ 92.f, 105.f });
        bg->setOpacity(120);
        bg->setColor({ 0, 0, 0 });
        container->addChild(bg);

        std::string rarityName = g_streakData.getCategoryName(item.rarity);
        std::transform(rarityName.begin(), rarityName.end(), rarityName.begin(), ::toupper);

        auto rarityLabel = CCLabelBMFont::create(rarityName.c_str(), "goldFont.fnt");
        rarityLabel->setScale(0.3f);
        rarityLabel->setPosition({ 0, 42.f });
        rarityLabel->setColor(g_streakData.getCategoryColor(item.rarity));
        container->addChild(rarityLabel);

        auto sprite = CCSprite::create(item.sprite.c_str());
        if (!sprite) sprite = CCSprite::createWithSpriteFrameName("GJ_questionMark_001.png");

        float maxW = 50.0f;
        float scale = 0.5f;
        if (sprite->getContentSize().width * scale > maxW) {
            scale = maxW / sprite->getContentSize().width;
        }
        sprite->setScale(scale);
        sprite->setPosition({ 0, 14.f });
        container->addChild(sprite);

        auto nameLabel = CCLabelBMFont::create(item.name.c_str(), "chatFont.fnt");
        nameLabel->setScale(0.28f);
        nameLabel->setPosition({ 0, -10.f });
        nameLabel->limitLabelWidth(85.f, 0.28f, 0.1f);
        container->addChild(nameLabel);

        bool isOwned = false;
        if (item.isBadge) isOwned = g_streakData.isBadgeUnlocked(item.id);
        else isOwned = g_streakData.isBannerUnlocked(item.id);

        if (isOwned) {
            auto ownedLabel = CCLabelBMFont::create("OWNED", "goldFont.fnt");
            ownedLabel->setScale(0.4f);
            ownedLabel->setColor({ 100, 255, 100 });
            ownedLabel->setPosition({ 0, -35.f });
            container->addChild(ownedLabel);
        }
        else {
            auto btnSprite = ButtonSprite::create(
                fmt::format("{}   ", item.price).c_str(),
                "goldFont.fnt",
                "GJ_button_01.png",
                0.8f
            );
            btnSprite->setScale(0.6f);

            auto gemIcon = CCSprite::create("gem.png"_spr);
            if (gemIcon) {
                gemIcon->setScale(0.2f);
                auto bSize = btnSprite->getContentSize();
                gemIcon->setPosition({ bSize.width - 18.f, bSize.height / 2 + 1.f });
                btnSprite->addChild(gemIcon);
            }

            auto btn = CCMenuItemSpriteExtra::create(
                btnSprite,
                this,
                menu_selector(DailyShopPopup::onBuyItem)
            );
            btn->setTag(index);
            btn->setPosition({ 0, -35.f });

            auto menu = CCMenu::createWithItem(btn);
            menu->setPosition({ 0, 0 });
            container->addChild(menu);
        }

        return container;
    }

    CCNode* createConsumableCell(const StreakData::ConsumableItem& item, int index) {
        auto container = CCNode::create();

        auto bg = CCScale9Sprite::create("GJ_square02.png");
        bg->setContentSize({ 92.f, 105.f });
        bg->setOpacity(120);
        bg->setColor({ 0, 0, 0 });
        container->addChild(bg);

        std::string rarityName = g_streakData.getCategoryName(item.rarity);
        std::transform(rarityName.begin(), rarityName.end(), rarityName.begin(), ::toupper);

        auto rarityLabel = CCLabelBMFont::create(rarityName.c_str(), "goldFont.fnt");
        rarityLabel->setScale(0.3f);
        rarityLabel->setPosition({ 0, 42.f });
        rarityLabel->setColor(g_streakData.getCategoryColor(item.rarity));
        container->addChild(rarityLabel);

        std::string sprName = item.isTickets ? "star_tiket.png" : "super_star.png";
        auto sprite = CCSprite::create(fmt::format("{}"_spr, sprName).c_str());
        if (!sprite) sprite = CCSprite::createWithSpriteFrameName("GJ_questionMark_001.png");

        float maxW = 40.0f;
        float sc = 0.4f;
        if (sprite->getContentSize().width * sc > maxW) {
            sc = maxW / sprite->getContentSize().width;
        }
        sprite->setScale(sc);
        sprite->setPosition({ 0, 18.f });
        container->addChild(sprite);

        std::string amountStr = fmt::format("x{}", item.amount);
        auto amountLabel = CCLabelBMFont::create(amountStr.c_str(), "bigFont.fnt");
        amountLabel->setScale(0.3f);
        amountLabel->setPosition({ 0, -2.f });
        amountLabel->setColor({ 255, 255, 255 });
        container->addChild(amountLabel);

        std::string displayName = item.isTickets ? "Star Tickets" : "Super Stars";
        auto nameLabel = CCLabelBMFont::create(displayName.c_str(), "chatFont.fnt");
        nameLabel->setScale(0.25f);
        nameLabel->setPosition({ 0, -14.f });
        nameLabel->setColor({ 200, 200, 200 });
        container->addChild(nameLabel);

        auto btnSprite = ButtonSprite::create(
            fmt::format("{}   ", item.price).c_str(),
            "goldFont.fnt",
            "GJ_button_01.png",
            0.8f
        );
        btnSprite->setScale(0.6f);

        auto gemIcon = CCSprite::create("gem.png"_spr);
        if (gemIcon) {
            gemIcon->setScale(0.2f);
            auto bSize = btnSprite->getContentSize();
            gemIcon->setPosition({ bSize.width - 18.f, bSize.height / 2 + 1.f });
            btnSprite->addChild(gemIcon);
        }

        auto btn = CCMenuItemSpriteExtra::create(
            btnSprite,
            this,
            menu_selector(DailyShopPopup::onBuyConsumable)
        );
        btn->setTag(index);
        btn->setPosition({ 0, -35.f });

        auto menu = CCMenu::createWithItem(btn);
        menu->setPosition({ 0, 0 });
        container->addChild(menu);

        return container;
    }

    void onBuyItem(CCObject* sender) {
        auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
        int index = btn->getTag();
        if (index < 0 || index >= (int)m_dailyItems.size()) return;
        auto item = m_dailyItems[index];

        PurchaseConfirmPopup::create(
            item.sprite,
            item.name,
            item.price,
            PurchaseCurrency::Gems,
            [this, item](int discountPercent) {
                this->processPurchase(item, discountPercent);
            }
        )->show();
    }

    void processPurchase(const StreakData::ShopItem& item, int discountPercent) {
        int finalPrice = discountedPurchasePrice(item.price, discountPercent);
        if (g_streakData.gems < finalPrice) {
            FLAlertLayer::create("Shop", "Not enough Gems!", "OK")->show();
            return;
        }

        matjson::Value payload = matjson::Value::object();
        payload.set("itemID", item.id);
        payload.set("price", item.price);
        payload.set("isBadge", item.isBadge);
        payload.set("discount_percent", discountPercent);

        claimOnServer("/daily-shop/purchase", payload, [this, item, keepAlive = Ref<CCNode>(this)](bool ok) {
            if (!ok) {
                FLAlertLayer::create("Error", "Purchase failed. Try again.", "OK")->show();
                return;
            }
            if (item.isBadge) g_streakData.unlockBadge(item.id);
            else g_streakData.unlockBanner(item.id);

            FMODAudioEngine::sharedEngine()->playEffect("buyItem01.ogg");

            if (item.isBadge) {
                BadgeNotification::show(item.id);
            } else {
                std::string rarityName = g_streakData.getCategoryName(item.rarity);
                ccColor3B rareColor = g_streakData.getCategoryColor(item.rarity);
                BannerNotification::show(item.id, item.sprite, item.name, rarityName, rareColor);
            }

            if (m_gemLabel) m_gemLabel->setString(std::to_string(g_streakData.gems).c_str());
            this->refreshItems();
        });
    }

    void onBuyConsumable(CCObject* sender) {
        auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
        int index = btn->getTag();
        if (index < 0 || index >= (int)m_dailyConsumables.size()) return;
        auto item = m_dailyConsumables[index];

        std::string typeName = item.isTickets ? "Star Tickets" : "Super Stars";
        std::string sprName = item.isTickets ? "star_tiket.png" : "super_star.png";
        std::string displayName = fmt::format("x{} {}", item.amount, typeName);

        PurchaseConfirmPopup::create(
            fmt::format("{}"_spr, sprName),
            displayName,
            item.price,
            PurchaseCurrency::Gems,
            [this, item](int discountPercent) {
                this->processConsumablePurchase(item, discountPercent);
            }
        )->show();
    }

    void processConsumablePurchase(const StreakData::ConsumableItem& item, int discountPercent) {
        int finalPrice = discountedPurchasePrice(item.price, discountPercent);
        if (g_streakData.gems < finalPrice) {
            FLAlertLayer::create("Shop", "Not enough Gems!", "OK")->show();
            return;
        }

        int startAmount = item.isTickets ? g_streakData.starTickets : g_streakData.superStars;

        matjson::Value payload = matjson::Value::object();
        payload.set("price", item.price);
        payload.set("amount", item.amount);
        payload.set("isTickets", item.isTickets);
        payload.set("discount_percent", discountPercent);

        claimOnServer("/daily-shop/purchase-consumable", payload, [this, item, startAmount, keepAlive = Ref<CCNode>(this)](bool ok) {
            if (!ok) {
                FLAlertLayer::create("Error", "Purchase failed. Try again.", "OK")->show();
                return;
            }
            FMODAudioEngine::sharedEngine()->playEffect("buyItem01.ogg");
            std::string sprName = item.isTickets ? "star_tiket.png" : "super_star.png";
            auto winSize = CCDirector::sharedDirector()->getWinSize();
            RewardNotification::show(fmt::format("{}"_spr, sprName), startAmount, item.amount, winSize / 2);
            if (m_gemLabel) m_gemLabel->setString(std::to_string(g_streakData.gems).c_str());
            this->refreshItems();
        });
    }

public:
    static DailyShopPopup* create() {
        auto ret = new DailyShopPopup();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};
