#pragma once
#include <Geode/ui/Popup.hpp>
#include <Geode/utils/general.hpp>
#include "../StreakData.h"
#include "../FirebaseManager.h"
#include <ctime>
#include "../BadgeNotification.h"
#include "../BannerNotification.h"

using namespace geode::prelude;

class DailyShopPopup : public Popup {
protected:
    CCLabelBMFont* m_gemLabel = nullptr;
    CCLabelBMFont* m_timerLabel = nullptr;
    CCNode* m_itemsContainer = nullptr;
    std::vector<StreakData::ShopItem> m_dailyItems;

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
        if (!m_itemsContainer) return;

        m_itemsContainer->removeAllChildren();

        float spacing = 110.0f;
        size_t count = m_dailyItems.size();
        float startX = -((float)(count - 1) * spacing) / 2.0f;

        for (int i = 0; i < count; i++) {
            auto cell = createItemCell(m_dailyItems[i], i);
            cell->setPosition({ startX + (i * spacing), 0 });
            m_itemsContainer->addChild(cell);
        }

        if (m_dailyItems.empty()) {
            auto emptyLabel = CCLabelBMFont::create("No items today", "bigFont.fnt");
            emptyLabel->setScale(0.5f);
            emptyLabel->setPosition({ 0, 0 });
            m_itemsContainer->addChild(emptyLabel);
        }
    }

    bool init() override {
        if (!Popup::init(360.f, 220.f, "geode.loader/GE_square03.png")) return false;
        auto size = m_mainLayer->getContentSize();
        this->setTitle("Daily Shop");

        m_dailyItems = g_streakData.getDailyShopSelection();

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

        m_itemsContainer = CCNode::create();
        m_itemsContainer->setPosition({ size.width / 2, size.height / 2 - 15.f });
        m_mainLayer->addChild(m_itemsContainer);

        this->refreshItems();

        return true;
    }

    CCNode* createItemCell(const StreakData::ShopItem& item, int index) {
        auto container = CCNode::create();

        auto bg = CCScale9Sprite::create("GJ_square02.png");
        bg->setContentSize({ 100.f, 150.f });
        bg->setOpacity(120);
        bg->setColor({ 0, 0, 0 });
        container->addChild(bg);

        std::string rarityName = g_streakData.getCategoryName(item.rarity);
        std::transform(rarityName.begin(), rarityName.end(), rarityName.begin(), ::toupper);

        auto rarityLabel = CCLabelBMFont::create(rarityName.c_str(), "goldFont.fnt");
        rarityLabel->setScale(0.4f);
        rarityLabel->setPosition({ 0, 60.f });
        rarityLabel->setColor(g_streakData.getCategoryColor(item.rarity));
        container->addChild(rarityLabel);

        auto sprite = CCSprite::create(item.sprite.c_str());
        if (!sprite) sprite = CCSprite::createWithSpriteFrameName("GJ_questionMark_001.png");

        float maxW = 75.0f;
        float scale = 0.7f;
        if (sprite->getContentSize().width > maxW) {
            scale = maxW / sprite->getContentSize().width;
        }
        sprite->setScale(scale);
        sprite->setPosition({ 0, 15.f });
        container->addChild(sprite);

        auto nameLabel = CCLabelBMFont::create(item.name.c_str(), "chatFont.fnt");
        nameLabel->setScale(0.35f);
        nameLabel->setPosition({ 0, -25.f });
        nameLabel->limitLabelWidth(90.f, 0.35f, 0.1f);
        container->addChild(nameLabel);

        bool isOwned = false;
        if (item.isBadge) isOwned = g_streakData.isBadgeUnlocked(item.id);
        else isOwned = g_streakData.isBannerUnlocked(item.id);

        if (isOwned) {
            auto ownedLabel = CCLabelBMFont::create("OWNED", "goldFont.fnt");
            ownedLabel->setScale(0.5f);
            ownedLabel->setColor({ 100, 255, 100 });
            ownedLabel->setPosition({ 0, -55.f });
            container->addChild(ownedLabel);
        }
        else {
            auto btnSprite = ButtonSprite::create(
                fmt::format("{}   ", item.price).c_str(),
                "goldFont.fnt",
                "GJ_button_01.png",
                0.8f
            );
            btnSprite->setScale(0.8f);

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
            btn->setPosition({ 0, -55.f });

            auto menu = CCMenu::createWithItem(btn);
            menu->setPosition({ 0, 0 });
            container->addChild(menu);
        }

        return container;
    }

    void onBuyItem(CCObject* sender) {
        auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
        int index = btn->getTag();
        if (index < 0 || index >= m_dailyItems.size()) return;
        auto item = m_dailyItems[index];

        if (g_streakData.gems >= item.price) {
            createQuickPopup(
                "Purchase",
                fmt::format("Buy <cy>{}</c>?", item.name),
                "No", "Yes",
                [this, item](auto, bool btn2) {
                    if (btn2) {
                        this->processPurchase(item);
                    }
                }
            );
        }
        else {
            FLAlertLayer::create("Shop", "Not enough Gems!", "OK")->show();
        }
    }

    void processPurchase(const StreakData::ShopItem& item) {
        if (g_streakData.gems >= item.price) {
            g_streakData.purchaseItem(item);

            FMODAudioEngine::sharedEngine()->playEffect("buyItem01.ogg");

            if (item.isBadge) {
                BadgeNotification::show(item.id);
            }
            else {
                std::string rarityName = g_streakData.getCategoryName(item.rarity);
                ccColor3B rareColor = g_streakData.getCategoryColor(item.rarity);
                BannerNotification::show(item.id, item.sprite, item.name, rarityName, rareColor);
            }

            if (m_gemLabel) {
                m_gemLabel->setString(std::to_string(g_streakData.gems).c_str());
            }

            this->refreshItems();
        }
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