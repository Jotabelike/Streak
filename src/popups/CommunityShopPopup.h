#pragma once

#include <Geode/Geode.hpp>
#include <Geode/binding/GJAccountManager.hpp>
#include <Geode/ui/Notification.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/ScrollLayer.hpp>
#include <Geode/ui/TextInput.hpp>
#include <Geode/utils/async.hpp>
#include <Geode/utils/web.hpp>
#include <algorithm>
#include <functional>
#include <string>
#include <vector>
#include "../FirebaseManager.h"
#include "../HMACAuth.h"
#include "../StreakData.h"
#include "../utils/ScrollbarUtils.h"
#include "PurchaseConfirmPopup.h"

using namespace geode::prelude;

namespace CommunityShopUI {
    inline constexpr const char* SERVER_URL = "https://streak-servidor.onrender.com";

    struct Listing {
        std::string id;
        int sellerID = 0;
        std::string sellerName;
        std::string itemType;
        std::string itemID;
        std::string itemName;
        int itemAmount = 1;
        std::string priceCurrency;
        int priceAmount = 0;
        std::string status = "active";
        int buyerID = 0;
        std::string buyerName;
        long long createdAt = 0;
    };

    inline std::string currencyName(const std::string& type) {
        if (type == "gems") return "Gems";
        if (type == "super_stars") return "Super Stars";
        if (type == "star_tickets") return "Tickets";
        return type;
    }

    inline std::string itemCategoryName(const std::string& type) {
        if (type == "badge") return "BADGE";
        if (type == "banner") return "BANNER";
        if (type == "gems") return "GEMS";
        if (type == "super_stars") return "SUPER STARS";
        if (type == "star_tickets") return "TICKETS";
        return "ITEM";
    }

    inline const char* currencySprite(const std::string& type) {
        if (type == "gems") return "gem.png"_spr;
        if (type == "super_stars") return "super_star.png"_spr;
        return "star_tiket.png"_spr;
    }

    inline int currencyBalance(const std::string& type) {
        if (type == "gems") return g_streakData.gems;
        if (type == "super_stars") return g_streakData.superStars;
        if (type == "star_tickets") return g_streakData.starTickets;
        return 0;
    }

    inline void applyBalances(const matjson::Value& data) {
        if (!data.contains("balances")) return;
        auto balances = data["balances"];
        g_streakData.gems = balances["gems"].as<int>().unwrapOr(g_streakData.gems);
        g_streakData.superStars = balances["super_stars"].as<int>().unwrapOr(g_streakData.superStars);
        g_streakData.starTickets = balances["star_tickets"].as<int>().unwrapOr(g_streakData.starTickets);
    }

    inline void setCosmeticOwned(const std::string& type, const std::string& id, bool owned) {
        if (type == "badge") {
            if (g_streakData.unlockedBadges.size() != g_streakData.badges.size()) {
                g_streakData.unlockedBadges.resize(g_streakData.badges.size(), false);
            }
            for (size_t i = 0; i < g_streakData.badges.size(); ++i) {
                if (g_streakData.badges[i].badgeID == id) {
                    g_streakData.unlockedBadges[i] = owned;
                    break;
                }
            }
        } else if (type == "banner") {
            if (g_streakData.unlockedBanners.size() != g_streakData.banners.size()) {
                g_streakData.unlockedBanners.resize(g_streakData.banners.size(), false);
            }
            for (size_t i = 0; i < g_streakData.banners.size(); ++i) {
                if (g_streakData.banners[i].bannerID == id) {
                    g_streakData.unlockedBanners[i] = owned;
                    break;
                }
            }
        }
    }

    inline std::string itemDisplayName(const Listing& listing) {
        if (listing.itemType == "badge") {
            if (auto info = g_streakData.getBadgeInfo(listing.itemID)) return info->displayName;
        } else if (listing.itemType == "banner") {
            if (auto info = g_streakData.getBannerInfo(listing.itemID)) return info->displayName;
        } else {
            return fmt::format("{} x{}", currencyName(listing.itemType), listing.itemAmount);
        }
        return listing.itemName.empty() ? listing.itemID : listing.itemName;
    }

    inline CCNode* createItemIcon(const std::string& type, const std::string& id,
                                  float maxWidth, float maxHeight) {
        auto holder = CCNode::create();
        holder->setContentSize({ maxWidth, maxHeight });
        holder->ignoreAnchorPointForPosition(false);
        holder->setAnchorPoint({ 0.5f, 0.5f });

        CCSprite* sprite = nullptr;
        if (type == "badge") {
            if (auto info = g_streakData.getBadgeInfo(id)) sprite = CCSprite::create(info->spriteName.c_str());
        } else if (type == "banner") {
            if (auto info = g_streakData.getBannerInfo(id)) sprite = CCSprite::create(info->spriteName.c_str());
        } else {
            sprite = CCSprite::create(currencySprite(type));
        }
        if (!sprite) sprite = CCSprite::createWithSpriteFrameName("GJ_questionMark_001.png");

        float width = std::max(sprite->getContentSize().width, 1.f);
        float height = std::max(sprite->getContentSize().height, 1.f);
        sprite->setScale(std::min(maxWidth / width, maxHeight / height));
        sprite->setPosition({ maxWidth / 2.f, maxHeight / 2.f });
        holder->addChild(sprite);
        return holder;
    }

    inline Listing parseListing(const matjson::Value& value) {
        Listing listing;
        listing.id = value["id"].as<std::string>().unwrapOr(std::string(""));
        listing.sellerID = value["seller_id"].as<int>().unwrapOr(0);
        listing.sellerName = value["seller_name"].as<std::string>().unwrapOr(std::string("Player"));
        listing.itemType = value["item_type"].as<std::string>().unwrapOr(std::string(""));
        listing.itemID = value["item_id"].as<std::string>().unwrapOr(std::string(""));
        listing.itemName = value["item_name"].as<std::string>().unwrapOr(std::string(""));
        listing.itemAmount = value["item_amount"].as<int>().unwrapOr(1);
        listing.priceCurrency = value["price_currency"].as<std::string>().unwrapOr(std::string("gems"));
        listing.priceAmount = value["price_amount"].as<int>().unwrapOr(0);
        listing.status = value["status"].as<std::string>().unwrapOr(std::string("active"));
        listing.buyerID = value["buyer_id"].as<int>().unwrapOr(0);
        listing.buyerName = value["buyer_name"].as<std::string>().unwrapOr(std::string(""));
        listing.createdAt = value["created_at"].as<long long>().unwrapOr(0LL);
        return listing;
    }

    inline std::vector<Listing> parseListings(const matjson::Value& value) {
        std::vector<Listing> result;
        if (!value.isArray()) return result;
        auto array = value.as<std::vector<matjson::Value>>();
        if (!array.isOk()) return result;
        for (const auto& raw : array.unwrap()) {
            auto listing = parseListing(raw);
            if (!listing.id.empty()) result.push_back(std::move(listing));
        }
        return result;
    }

    inline std::string errorMessage(const matjson::Value& data, const std::string& fallback) {
        if (data.contains("error")) {
            auto error = data["error"].as<std::string>().unwrapOr(std::string(""));
            if (!error.empty()) return error;
        }
        return fallback;
    }

    inline bool isBlockedRoleBadge(const std::string& id) {
        return id == "moderator_badge" || id == "creator_badge" ||
               id == "vip_badge" || id == "stellar_badge";
    }
}

class CommunitySellPopup : public Popup {
protected:
    struct SellOption {
        std::string id;
        std::string name;
    };

    std::function<void()> m_onPublished;
    std::vector<SellOption> m_options;
    int m_typeIndex = 0;
    int m_optionIndex = 0;
    int m_priceCurrencyIndex = 0;
    bool m_busy = false;

    CCLabelBMFont* m_typeLabel = nullptr;
    CCLabelBMFont* m_itemLabel = nullptr;
    CCLabelBMFont* m_priceCurrencyLabel = nullptr;
    CCLabelBMFont* m_balanceLabel = nullptr;
    CCLabelBMFont* m_amountCaption = nullptr;
    CCNode* m_preview = nullptr;
    TextInput* m_amountInput = nullptr;
    TextInput* m_priceInput = nullptr;
    CCMenuItemSpriteExtra* m_itemPrev = nullptr;
    CCMenuItemSpriteExtra* m_itemNext = nullptr;
    CCMenuItemSpriteExtra* m_publishBtn = nullptr;

    std::string itemType() const {
        switch (m_typeIndex) {
            case 0: return "badge";
            case 1: return "banner";
            case 2: return "gems";
            case 3: return "super_stars";
            default: return "star_tickets";
        }
    }

    std::string itemTypeName() const {
        switch (m_typeIndex) {
            case 0: return "Badge";
            case 1: return "Banner";
            case 2: return "Gems";
            case 3: return "Super Stars";
            default: return "Tickets";
        }
    }

    std::string priceCurrency() const {
        if (m_priceCurrencyIndex == 0) return "gems";
        if (m_priceCurrencyIndex == 1) return "super_stars";
        return "star_tickets";
    }

    void chooseCompatiblePriceCurrency() {
        auto type = itemType();
        if (type == "badge" || type == "banner") return;
        for (int i = 0; i < 3 && priceCurrency() == type; ++i) {
            m_priceCurrencyIndex = (m_priceCurrencyIndex + 1) % 3;
        }
    }

    void rebuildOptions() {
        m_options.clear();
        m_optionIndex = 0;
        if (m_typeIndex == 0) {
            for (size_t i = 0; i < g_streakData.badges.size(); ++i) {
                const auto& badge = g_streakData.badges[i];
                if (i >= g_streakData.unlockedBadges.size() || !g_streakData.unlockedBadges[i]) continue;
                if (badge.badgeID == g_streakData.equippedBadge) continue;
                if (CommunityShopUI::isBlockedRoleBadge(badge.badgeID)) continue;
                m_options.push_back({ badge.badgeID, badge.displayName });
            }
        } else if (m_typeIndex == 1) {
            for (size_t i = 0; i < g_streakData.banners.size(); ++i) {
                const auto& banner = g_streakData.banners[i];
                if (i >= g_streakData.unlockedBanners.size() || !g_streakData.unlockedBanners[i]) continue;
                if (banner.bannerID == g_streakData.equippedBanner) continue;
                m_options.push_back({ banner.bannerID, banner.displayName });
            }
        }
    }

    void refreshView() {
        chooseCompatiblePriceCurrency();
        auto type = itemType();
        bool cosmetic = type == "badge" || type == "banner";

        if (m_typeLabel) m_typeLabel->setString(itemTypeName().c_str());
        if (m_priceCurrencyLabel) {
            m_priceCurrencyLabel->setString(CommunityShopUI::currencyName(priceCurrency()).c_str());
        }
        if (m_amountInput) m_amountInput->setVisible(!cosmetic);
        if (m_amountCaption) m_amountCaption->setVisible(!cosmetic);
        if (m_itemPrev) m_itemPrev->setVisible(cosmetic);
        if (m_itemNext) m_itemNext->setVisible(cosmetic);

        if (m_preview) {
            m_preview->removeAllChildren();
            CCNode* icon = nullptr;
            if (cosmetic && !m_options.empty()) {
                const auto& option = m_options[m_optionIndex];
                icon = CommunityShopUI::createItemIcon(type, option.id, 78.f, 58.f);
                if (m_itemLabel) {
                    m_itemLabel->setString(option.name.c_str());
                    m_itemLabel->limitLabelWidth(230.f, 0.42f, 0.18f);
                }
            } else if (cosmetic) {
                if (m_itemLabel) m_itemLabel->setString("No unequipped items available");
            } else {
                icon = CommunityShopUI::createItemIcon(type, "", 62.f, 52.f);
                if (m_itemLabel) m_itemLabel->setString(itemTypeName().c_str());
            }
            if (icon) {
                icon->setPosition({ 0.f, 0.f });
                m_preview->addChild(icon);
            }
        }

        if (m_balanceLabel) {
            m_balanceLabel->setString(fmt::format("Available: {}", CommunityShopUI::currencyBalance(type)).c_str());
            m_balanceLabel->setVisible(!cosmetic);
        }
        bool canPublish = !cosmetic || !m_options.empty();
        if (m_publishBtn) m_publishBtn->setEnabled(canPublish && !m_busy);
    }

    int positiveInput(TextInput* input) const {
        if (!input) return 0;
        try {
            long long value = std::stoll(std::string(input->getString()));
            if (value <= 0 || value > 2000000000LL) return 0;
            return static_cast<int>(value);
        } catch (...) {
            return 0;
        }
    }

    void onPrevType(CCObject*) {
        if (m_busy) return;
        m_typeIndex = (m_typeIndex + 4) % 5;
        rebuildOptions();
        refreshView();
    }

    void onNextType(CCObject*) {
        if (m_busy) return;
        m_typeIndex = (m_typeIndex + 1) % 5;
        rebuildOptions();
        refreshView();
    }

    void onPrevItem(CCObject*) {
        if (m_busy || m_options.empty()) return;
        m_optionIndex = (m_optionIndex + static_cast<int>(m_options.size()) - 1) % static_cast<int>(m_options.size());
        refreshView();
    }

    void onNextItem(CCObject*) {
        if (m_busy || m_options.empty()) return;
        m_optionIndex = (m_optionIndex + 1) % static_cast<int>(m_options.size());
        refreshView();
    }

    void onPrevCurrency(CCObject*) {
        if (m_busy) return;
        m_priceCurrencyIndex = (m_priceCurrencyIndex + 2) % 3;
        chooseCompatiblePriceCurrency();
        refreshView();
    }

    void onNextCurrency(CCObject*) {
        if (m_busy) return;
        m_priceCurrencyIndex = (m_priceCurrencyIndex + 1) % 3;
        chooseCompatiblePriceCurrency();
        refreshView();
    }

    void onPublish(CCObject*) {
        if (m_busy) return;
        auto type = itemType();
        bool cosmetic = type == "badge" || type == "banner";
        if (cosmetic && m_options.empty()) return;

        int amount = cosmetic ? 1 : positiveInput(m_amountInput);
        int price = positiveInput(m_priceInput);
        if (amount <= 0) {
            Notification::create("Enter a valid amount", NotificationIcon::Error)->show();
            return;
        }
        if (price <= 0) {
            Notification::create("Enter a valid price", NotificationIcon::Error)->show();
            return;
        }
        if (!cosmetic && amount > CommunityShopUI::currencyBalance(type)) {
            Notification::create("You do not have enough to list", NotificationIcon::Error)->show();
            return;
        }

        matjson::Value payload = matjson::Value::object();
        payload.set("item_type", type);
        payload.set("item_amount", amount);
        payload.set("price_currency", priceCurrency());
        payload.set("price_amount", price);
        if (cosmetic) {
            const auto& option = m_options[m_optionIndex];
            payload.set("item_id", option.id);
            payload.set("item_name", option.name);
        }

        m_busy = true;
        if (m_publishBtn) m_publishBtn->setEnabled(false);
        claimOnServerEx("/community-shop/list", payload,
            [this, type, cosmetic, keepAlive = Ref<CCNode>(this)](bool ok, int, const matjson::Value& data) {
                m_busy = false;
                if (!ok) {
                    if (m_publishBtn) m_publishBtn->setEnabled(true);
                    FLAlertLayer::create(
                        "Community Shop",
                        CommunityShopUI::errorMessage(data, "Could not publish the listing.").c_str(),
                        "OK"
                    )->show();
                    return;
                }

                CommunityShopUI::applyBalances(data);
                if (cosmetic && !m_options.empty()) {
                    CommunityShopUI::setCosmeticOwned(type, m_options[m_optionIndex].id, false);
                }
                if (m_onPublished) m_onPublished();
                Notification::create("Listing published!", NotificationIcon::Success)->show();
                this->onClose(nullptr);
            });
    }

    bool init(std::function<void()> onPublished) {
        if (!Popup::init(390.f, 300.f, "geode.loader/GE_square03.png")) return false;
        m_onPublished = std::move(onPublished);
        this->setTitle("Create Listing");
        auto size = m_mainLayer->getContentSize();
        float centerX = size.width / 2.f;

        auto controls = CCMenu::create();
        controls->setPosition({ 0.f, 0.f });
        m_mainLayer->addChild(controls, 10);

        auto arrowLeft = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        arrowLeft->setScale(0.55f);
        auto typePrev = CCMenuItemSpriteExtra::create(arrowLeft, this, menu_selector(CommunitySellPopup::onPrevType));
        typePrev->setPosition({ 58.f, 252.f });
        controls->addChild(typePrev);

        auto arrowRight = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        arrowRight->setFlipX(true);
        arrowRight->setScale(0.55f);
        auto typeNext = CCMenuItemSpriteExtra::create(arrowRight, this, menu_selector(CommunitySellPopup::onNextType));
        typeNext->setPosition({ size.width - 58.f, 252.f });
        controls->addChild(typeNext);

        m_typeLabel = CCLabelBMFont::create("Badge", "goldFont.fnt");
        m_typeLabel->setScale(0.65f);
        m_typeLabel->setPosition({ centerX, 252.f });
        m_mainLayer->addChild(m_typeLabel, 3);

        auto itemLeftSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        itemLeftSpr->setScale(0.45f);
        m_itemPrev = CCMenuItemSpriteExtra::create(itemLeftSpr, this, menu_selector(CommunitySellPopup::onPrevItem));
        m_itemPrev->setPosition({ 85.f, 186.f });
        controls->addChild(m_itemPrev);

        auto itemRightSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        itemRightSpr->setFlipX(true);
        itemRightSpr->setScale(0.45f);
        m_itemNext = CCMenuItemSpriteExtra::create(itemRightSpr, this, menu_selector(CommunitySellPopup::onNextItem));
        m_itemNext->setPosition({ size.width - 85.f, 186.f });
        controls->addChild(m_itemNext);

        m_preview = CCNode::create();
        m_preview->setPosition({ centerX, 188.f });
        m_mainLayer->addChild(m_preview, 4);

        m_itemLabel = CCLabelBMFont::create("", "bigFont.fnt");
        m_itemLabel->setScale(0.42f);
        m_itemLabel->setPosition({ centerX, 145.f });
        m_mainLayer->addChild(m_itemLabel, 4);

        m_amountCaption = CCLabelBMFont::create("Amount", "goldFont.fnt");
        m_amountCaption->setScale(0.42f);
        m_amountCaption->setAnchorPoint({ 1.f, 0.5f });
        m_amountCaption->setPosition({ centerX - 20.f, 116.f });
        m_mainLayer->addChild(m_amountCaption, 4);

        m_amountInput = TextInput::create(100.f, "Amount", "bigFont.fnt");
        m_amountInput->setFilter("0123456789");
        m_amountInput->setMaxCharCount(10);
        m_amountInput->setPosition({ centerX + 52.f, 116.f });
        m_mainLayer->addChild(m_amountInput, 4);

        m_balanceLabel = CCLabelBMFont::create("", "chatFont.fnt");
        m_balanceLabel->setScale(0.45f);
        m_balanceLabel->setColor({ 190, 220, 255 });
        m_balanceLabel->setPosition({ centerX, 91.f });
        m_mainLayer->addChild(m_balanceLabel, 4);

        auto priceCaption = CCLabelBMFont::create("Price", "goldFont.fnt");
        priceCaption->setScale(0.45f);
        priceCaption->setPosition({ 55.f, 65.f });
        m_mainLayer->addChild(priceCaption, 4);

        m_priceInput = TextInput::create(90.f, "Price", "bigFont.fnt");
        m_priceInput->setFilter("0123456789");
        m_priceInput->setMaxCharCount(10);
        m_priceInput->setPosition({ 125.f, 65.f });
        m_mainLayer->addChild(m_priceInput, 4);

        auto currencyPrevSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        currencyPrevSpr->setScale(0.35f);
        auto currencyPrev = CCMenuItemSpriteExtra::create(currencyPrevSpr, this, menu_selector(CommunitySellPopup::onPrevCurrency));
        currencyPrev->setPosition({ 210.f, 65.f });
        controls->addChild(currencyPrev);

        m_priceCurrencyLabel = CCLabelBMFont::create("Gems", "bigFont.fnt");
        m_priceCurrencyLabel->setScale(0.35f);
        m_priceCurrencyLabel->setPosition({ 278.f, 65.f });
        m_mainLayer->addChild(m_priceCurrencyLabel, 4);

        auto currencyNextSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        currencyNextSpr->setFlipX(true);
        currencyNextSpr->setScale(0.35f);
        auto currencyNext = CCMenuItemSpriteExtra::create(currencyNextSpr, this, menu_selector(CommunitySellPopup::onNextCurrency));
        currencyNext->setPosition({ 350.f, 65.f });
        controls->addChild(currencyNext);

        auto publishSprite = ButtonSprite::create("Publish", 0, false, "goldFont.fnt", "GJ_button_01.png", 0, 0.72f);
        m_publishBtn = CCMenuItemSpriteExtra::create(publishSprite, this, menu_selector(CommunitySellPopup::onPublish));
        m_publishBtn->setPosition({ centerX, 27.f });
        controls->addChild(m_publishBtn);

        rebuildOptions();
        refreshView();
        return true;
    }

public:
    static CommunitySellPopup* create(std::function<void()> onPublished) {
        auto ret = new CommunitySellPopup();
        if (ret && ret->init(std::move(onPublished))) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

class CommunityPriceEditPopup : public Popup {
protected:
    CommunityShopUI::Listing m_listing;
    std::function<void()> m_onUpdated;
    TextInput* m_priceInput = nullptr;
    CCMenuItemSpriteExtra* m_saveBtn = nullptr;
    bool m_busy = false;

    int priceInput() const {
        if (!m_priceInput) return 0;
        try {
            long long value = std::stoll(std::string(m_priceInput->getString()));
            if (value <= 0 || value > 2000000000LL) return 0;
            return static_cast<int>(value);
        } catch (...) {
            return 0;
        }
    }

    void onSave(CCObject*) {
        if (m_busy) return;
        int newPrice = priceInput();
        if (newPrice <= 0) {
            Notification::create("Enter a valid price", NotificationIcon::Error)->show();
            return;
        }
        if (newPrice == m_listing.priceAmount) {
            this->onClose(nullptr);
            return;
        }

        matjson::Value payload = matjson::Value::object();
        payload.set("listing_id", m_listing.id);
        payload.set("price_amount", newPrice);

        m_busy = true;
        if (m_saveBtn) m_saveBtn->setEnabled(false);
        claimOnServerEx("/community-shop/edit-price", payload,
            [this, keepAlive = Ref<CCNode>(this)](bool ok, int, const matjson::Value& data) {
                m_busy = false;
                if (!ok) {
                    if (m_saveBtn) m_saveBtn->setEnabled(true);
                    FLAlertLayer::create(
                        "Community Shop",
                        CommunityShopUI::errorMessage(data, "Could not update the price.").c_str(),
                        "OK"
                    )->show();
                    return;
                }

                if (m_onUpdated) m_onUpdated();
                Notification::create("Price updated!", NotificationIcon::Success)->show();
                this->onClose(nullptr);
            }
        );
    }

    bool init(const CommunityShopUI::Listing& listing, std::function<void()> onUpdated) {
        if (!Popup::init(310.f, 175.f, "geode.loader/GE_square03.png")) return false;
        m_listing = listing;
        m_onUpdated = std::move(onUpdated);
        this->setTitle("Edit Price");

        auto itemName = CCLabelBMFont::create(CommunityShopUI::itemDisplayName(listing).c_str(), "bigFont.fnt");
        itemName->setScale(0.4f);
        itemName->limitLabelWidth(245.f, 0.4f, 0.25f);
        itemName->setPosition({ 155.f, 128.f });
        m_mainLayer->addChild(itemName, 3);

        auto currencyIcon = CCSprite::create(CommunityShopUI::currencySprite(listing.priceCurrency));
        if (currencyIcon) {
            float maxDim = std::max({
                currencyIcon->getContentSize().width,
                currencyIcon->getContentSize().height,
                1.f
            });
            currencyIcon->setScale(22.f / maxDim);
            currencyIcon->setPosition({ 73.f, 88.f });
            m_mainLayer->addChild(currencyIcon, 4);
        }

        m_priceInput = TextInput::create(135.f, "New price", "bigFont.fnt");
        m_priceInput->setFilter("0123456789");
        m_priceInput->setMaxCharCount(10);
        m_priceInput->setString(std::to_string(listing.priceAmount));
        m_priceInput->setPosition({ 165.f, 88.f });
        m_mainLayer->addChild(m_priceInput, 4);

        auto currencyName = CCLabelBMFont::create(
            CommunityShopUI::currencyName(listing.priceCurrency).c_str(), "bigFont.fnt"
        );
        currencyName->setScale(0.27f);
        currencyName->setColor({ 180, 220, 255 });
        currencyName->setPosition({ 155.f, 61.f });
        m_mainLayer->addChild(currencyName, 3);

        auto menu = CCMenu::create();
        menu->setPosition({ 0.f, 0.f });
        m_mainLayer->addChild(menu, 6);

        auto saveSprite = ButtonSprite::create(
            "Save Price", 0, false, "goldFont.fnt", "GJ_button_01.png", 0, 0.65f
        );
        m_saveBtn = CCMenuItemSpriteExtra::create(
            saveSprite, this, menu_selector(CommunityPriceEditPopup::onSave)
        );
        m_saveBtn->setPosition({ 155.f, 28.f });
        menu->addChild(m_saveBtn);
        return true;
    }

public:
    static CommunityPriceEditPopup* create(
        const CommunityShopUI::Listing& listing,
        std::function<void()> onUpdated
    ) {
        auto ret = new CommunityPriceEditPopup();
        if (ret && ret->init(listing, std::move(onUpdated))) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

class CommunityShopPopup : public Popup {
protected:
    static constexpr float CELL_HEIGHT = 76.f;
    std::vector<CommunityShopUI::Listing> m_marketListings;
    std::vector<CommunityShopUI::Listing> m_myListings;
    bool m_showMine = false;
    bool m_busy = false;

    ScrollLayer* m_scroll = nullptr;
    CCLabelBMFont* m_statusLabel = nullptr;
    CCLabelBMFont* m_gemsLabel = nullptr;
    CCLabelBMFont* m_starsLabel = nullptr;
    CCLabelBMFont* m_ticketsLabel = nullptr;
    CCMenuItemSpriteExtra* m_marketTab = nullptr;
    CCMenuItemSpriteExtra* m_mineTab = nullptr;
    CCMenuItemSpriteExtra* m_refreshBtn = nullptr;
    async::TaskHolder<web::WebResponse> m_loadTask;

    void addBalance(float centerX, float y, const char* spriteName, int value, CCLabelBMFont*& outLabel) {
        auto icon = CCSprite::create(spriteName);
        if (icon) {
            float maxDim = std::max({ icon->getContentSize().width, icon->getContentSize().height, 1.f });
            icon->setScale(19.f / maxDim);
            icon->setPosition({ centerX - 27.f, y });
            m_mainLayer->addChild(icon, 5);
        }
        outLabel = CCLabelBMFont::create(std::to_string(value).c_str(), "goldFont.fnt");
        outLabel->setScale(0.42f);
        outLabel->setAnchorPoint({ 0.5f, 0.5f });
        outLabel->setPosition({ centerX + 10.f, y });
        outLabel->limitLabelWidth(62.f, 0.42f, 0.24f);
        m_mainLayer->addChild(outLabel, 5);
    }

    void refreshBalances() {
        if (m_gemsLabel) m_gemsLabel->setString(std::to_string(g_streakData.gems).c_str());
        if (m_starsLabel) m_starsLabel->setString(std::to_string(g_streakData.superStars).c_str());
        if (m_ticketsLabel) m_ticketsLabel->setString(std::to_string(g_streakData.starTickets).c_str());
    }

    ButtonSprite* createPriceSprite(const CommunityShopUI::Listing& listing, const char* background) {
        auto sprite = ButtonSprite::create(
            "          ", 0, false, "goldFont.fnt", background, 25.f, 0.56f
        );
        auto size = sprite->getContentSize();

        auto icon = CCSprite::create(CommunityShopUI::currencySprite(listing.priceCurrency));
        if (icon) {
            float maxDim = std::max({ icon->getContentSize().width, icon->getContentSize().height, 1.f });
            icon->setScale(16.f / maxDim);
            icon->setPosition({ 16.f, size.height / 2.f });
            sprite->addChild(icon, 2);
        }

        auto amount = CCLabelBMFont::create(std::to_string(listing.priceAmount).c_str(), "goldFont.fnt");
        amount->setScale(0.46f);
        amount->setPosition({ size.width / 2.f + 8.f, size.height / 2.f });
        amount->limitLabelWidth(size.width - 32.f, 0.46f, 0.25f);
        sprite->addChild(amount, 2);
        return sprite;
    }

    CCNode* buildCell(const CommunityShopUI::Listing& listing, int index, float width) {
        auto cell = CCNode::create();
        cell->setContentSize({ width, CELL_HEIGHT });

        auto bg = CCScale9Sprite::create("square02_001.png");
        bg->setContentSize({ width - 8.f, CELL_HEIGHT - 6.f });
        bg->setPosition({ width / 2.f, CELL_HEIGHT / 2.f });
        bg->setColor({ 32, 48, 78 });
        bg->setOpacity(118);
        cell->addChild(bg);

        auto icon = CommunityShopUI::createItemIcon(listing.itemType, listing.itemID, 48.f, 38.f);
        icon->setPosition({ 35.f, 46.f });
        cell->addChild(icon, 3);

        auto category = CCLabelBMFont::create(
            CommunityShopUI::itemCategoryName(listing.itemType).c_str(), "bigFont.fnt"
        );
        category->setScale(0.22f);
        category->setColor({ 145, 205, 255 });
        category->setPosition({ 35.f, 14.f });
        category->limitLabelWidth(57.f, 0.22f, 0.15f);
        cell->addChild(category, 3);

        auto name = CCLabelBMFont::create(CommunityShopUI::itemDisplayName(listing).c_str(), "bigFont.fnt");
        name->setScale(0.43f);
        name->setAnchorPoint({ 0.f, 0.5f });
        name->setPosition({ 66.f, 54.f });
        name->limitLabelWidth(185.f, 0.43f, 0.24f);
        cell->addChild(name, 3);

        std::string detail;
        if (m_showMine) {
            if (listing.status == "sold") detail = listing.buyerName.empty() ? "Sold" : "Sold to " + listing.buyerName;
            else if (listing.status == "cancelled") detail = "Cancelled";
            else detail = "Active listing";
        } else {
            detail = "by " + (listing.sellerName.empty() ? std::string("Player") : listing.sellerName);
        }
        auto detailLabel = CCLabelBMFont::create(detail.c_str(), "bigFont.fnt");
        detailLabel->setScale(0.29f);
        detailLabel->setAnchorPoint({ 0.f, 0.5f });
        detailLabel->setColor(listing.status == "active" ? ccColor3B{ 180, 215, 255 } : ccColor3B{ 165, 165, 165 });
        detailLabel->setPosition({ 66.f, 29.f });
        detailLabel->limitLabelWidth(185.f, 0.29f, 0.2f);
        cell->addChild(detailLabel, 3);

        auto actionMenu = CCMenu::create();
        actionMenu->setPosition({ 0.f, 0.f });
        cell->addChild(actionMenu, 6);

        if (m_showMine) {
            auto priceSprite = createPriceSprite(listing, "GJ_button_04.png");
            priceSprite->setPosition({ width - 67.f, 53.f });
            cell->addChild(priceSprite, 4);

            if (listing.status == "active") {
                auto editSprite = ButtonSprite::create(
                    "Edit", 0, false, "goldFont.fnt", "GJ_button_02.png", 0, 0.46f
                );
                auto editButton = CCMenuItemSpriteExtra::create(
                    editSprite, this, menu_selector(CommunityShopPopup::onEditPrice)
                );
                editButton->setTag(index);
                editButton->setPosition({ width - 99.f, 22.f });
                actionMenu->addChild(editButton);

                auto cancelSprite = ButtonSprite::create(
                    "Cancel", 0, false, "goldFont.fnt", "GJ_button_06.png", 0, 0.46f
                );
                auto cancelButton = CCMenuItemSpriteExtra::create(
                    cancelSprite, this, menu_selector(CommunityShopPopup::onCancel)
                );
                cancelButton->setTag(index);
                cancelButton->setPosition({ width - 43.f, 22.f });
                actionMenu->addChild(cancelButton);
            } else {
                auto status = CCLabelBMFont::create(listing.status == "sold" ? "SOLD" : "CANCELLED", "goldFont.fnt");
                status->setScale(0.36f);
                status->setColor(listing.status == "sold" ? ccColor3B{ 110, 255, 150 } : ccColor3B{ 170, 170, 170 });
                status->setPosition({ width - 67.f, 21.f });
                cell->addChild(status, 4);
            }
        } else {
            auto am = GJAccountManager::sharedState();
            int accountID = am ? am->m_accountID : 0;
            if (listing.sellerID == accountID) {
                auto priceSprite = createPriceSprite(listing, "GJ_button_04.png");
                priceSprite->setPosition({ width - 67.f, 53.f });
                cell->addChild(priceSprite, 4);

                auto yours = CCLabelBMFont::create("YOURS", "goldFont.fnt");
                yours->setScale(0.39f);
                yours->setColor({ 150, 210, 255 });
                yours->setPosition({ width - 67.f, 21.f });
                cell->addChild(yours, 4);
            } else {
                bool affordable = CommunityShopUI::currencyBalance(listing.priceCurrency) >= listing.priceAmount;
                auto priceSprite = createPriceSprite(
                    listing, affordable ? "GJ_button_01.png" : "GJ_button_06.png"
                );
                auto button = CCMenuItemSpriteExtra::create(priceSprite, this, menu_selector(CommunityShopPopup::onBuy));
                button->setTag(index);
                button->setPosition({ width - 67.f, CELL_HEIGHT / 2.f });
                actionMenu->addChild(button);
            }
        }

        return cell;
    }

    void rebuildList() {
        if (!m_scroll) return;
        m_scroll->m_contentLayer->removeAllChildren();
        const auto& listings = m_showMine ? m_myListings : m_marketListings;
        float width = m_scroll->getContentSize().width;
        float viewHeight = m_scroll->getContentSize().height;
        float totalHeight = std::max(viewHeight, CELL_HEIGHT * static_cast<float>(listings.size()));
        m_scroll->m_contentLayer->setContentSize({ width, totalHeight });

        for (int i = 0; i < static_cast<int>(listings.size()); ++i) {
            auto cell = buildCell(listings[i], i, width);
            cell->setPosition({ 0.f, totalHeight - CELL_HEIGHT * static_cast<float>(i + 1) });
            m_scroll->m_contentLayer->addChild(cell);
        }

        if (listings.empty()) {
            auto empty = CCLabelBMFont::create(
                m_busy ? "Loading marketplace..." : (m_showMine ? "You have no listings yet." : "No active listings right now."),
                "bigFont.fnt"
            );
            empty->setScale(0.4f);
            empty->setColor({ 185, 195, 220 });
            empty->setPosition({ width / 2.f, totalHeight / 2.f });
            m_scroll->m_contentLayer->addChild(empty);
        }
        m_scroll->moveToTop();
        refreshBalances();
        if (m_marketTab) m_marketTab->setEnabled(m_showMine);
        if (m_mineTab) m_mineTab->setEnabled(!m_showMine);
    }

    void loadMarketplace() {
        if (m_busy) return;
        auto am = GJAccountManager::sharedState();
        if (!am || am->m_accountID == 0 || HMACAuth::getSessionToken().empty()) {
            if (m_statusLabel) m_statusLabel->setString("Sign in to use the marketplace");
            return;
        }

        m_busy = true;
        if (m_refreshBtn) m_refreshBtn->setEnabled(false);
        rebuildList();

        int accountID = am->m_accountID;
        auto request = web::WebRequest();
        HMACAuth::signGetRequest(request, accountID);
        m_loadTask.spawn(
            request.get(fmt::format("{}/community-shop/{}", CommunityShopUI::SERVER_URL, accountID)),
            [this, keepAlive = Ref<CCNode>(this)](web::WebResponse response) {
                m_busy = false;
                if (m_refreshBtn) m_refreshBtn->setEnabled(true);
                if (!response.ok() || !response.json().isOk()) {
                    if (response.code() == 401) {
                        HMACAuth::clearSessionToken();
                        refreshPlayerDataFromServer([](bool) {});
                    }
                    if (m_statusLabel) m_statusLabel->setString("Could not load marketplace");
                    rebuildList();
                    return;
                }

                auto data = response.json().unwrap();
                m_marketListings = CommunityShopUI::parseListings(data["listings"]);
                m_myListings = CommunityShopUI::parseListings(data["my_listings"]);
                CommunityShopUI::applyBalances(data);
                if (m_statusLabel) {
                    m_statusLabel->setString(fmt::format("{} active offers", m_marketListings.size()).c_str());
                }
                rebuildList();
            }
        );
    }

    void onMarketTab(CCObject*) {
        m_showMine = false;
        rebuildList();
    }

    void onMineTab(CCObject*) {
        m_showMine = true;
        rebuildList();
    }

    void onRefresh(CCObject*) {
        loadMarketplace();
    }

    void onSell(CCObject*) {
        Ref<CommunityShopPopup> self = this;
        if (auto popup = CommunitySellPopup::create([self]() {
            if (self && self->isRunning()) self->loadMarketplace();
        })) popup->show();
    }

    void onBuy(CCObject* sender) {
        if (m_busy) return;
        int index = static_cast<CCNode*>(sender)->getTag();
        if (index < 0 || index >= static_cast<int>(m_marketListings.size())) return;
        auto listing = m_marketListings[index];

        std::string spriteName = CommunityShopUI::currencySprite(listing.itemType);
        if (listing.itemType == "badge") {
            if (auto info = g_streakData.getBadgeInfo(listing.itemID)) spriteName = info->spriteName;
        } else if (listing.itemType == "banner") {
            if (auto info = g_streakData.getBannerInfo(listing.itemID)) spriteName = info->spriteName;
        }

        PurchaseCurrency currency = PurchaseCurrency::StarTickets;
        if (listing.priceCurrency == "gems") currency = PurchaseCurrency::Gems;
        else if (listing.priceCurrency == "super_stars") currency = PurchaseCurrency::SuperStars;

        auto popup = PurchaseConfirmPopup::create(
            spriteName, CommunityShopUI::itemDisplayName(listing), listing.priceAmount, currency,
            [this, listing, keepAlive = Ref<CCNode>(this)](int discountPercent) {
                int finalPrice = discountedPurchasePrice(listing.priceAmount, discountPercent);
                if (CommunityShopUI::currencyBalance(listing.priceCurrency) < finalPrice) {
                    FLAlertLayer::create(
                        "Community Shop",
                        fmt::format("You do not have enough <cy>{}</c>.",
                            CommunityShopUI::currencyName(listing.priceCurrency)).c_str(),
                        "OK"
                    )->show();
                    return;
                }
                requestBuy(listing, discountPercent);
            });
        if (popup) popup->show();
    }

    void requestBuy(const CommunityShopUI::Listing& listing, int discountPercent) {
        if (m_busy) return;
        m_busy = true;
        matjson::Value payload = matjson::Value::object();
        payload.set("listing_id", listing.id);
        payload.set("discount_percent", discountPercent);
        claimOnServerEx("/community-shop/buy", payload,
            [this, listing, keepAlive = Ref<CCNode>(this)](bool ok, int, const matjson::Value& data) {
                m_busy = false;
                if (!ok) {
                    FLAlertLayer::create(
                        "Community Shop",
                        CommunityShopUI::errorMessage(data, "Could not complete the purchase.").c_str(),
                        "OK"
                    )->show();
                    loadMarketplace();
                    return;
                }
                CommunityShopUI::applyBalances(data);
                if (listing.itemType == "badge" || listing.itemType == "banner") {
                    CommunityShopUI::setCosmeticOwned(listing.itemType, listing.itemID, true);
                }
                Notification::create("Purchase completed!", NotificationIcon::Success)->show();
                loadMarketplace();
            }
        );
    }

    void onCancel(CCObject* sender) {
        if (m_busy) return;
        int index = static_cast<CCNode*>(sender)->getTag();
        if (index < 0 || index >= static_cast<int>(m_myListings.size())) return;
        auto listing = m_myListings[index];
        createQuickPopup(
            "Cancel Listing",
            fmt::format("Return <cy>{}</c> to your inventory?", CommunityShopUI::itemDisplayName(listing)),
            "Keep", "Cancel listing",
            [this, listing, keepAlive = Ref<CCNode>(this)](auto, bool confirmed) {
                if (confirmed) requestCancel(listing);
            }
        );
    }

    void onEditPrice(CCObject* sender) {
        if (m_busy) return;
        int index = static_cast<CCNode*>(sender)->getTag();
        if (index < 0 || index >= static_cast<int>(m_myListings.size())) return;
        const auto listing = m_myListings[index];
        if (listing.status != "active") return;

        Ref<CommunityShopPopup> self = this;
        if (auto popup = CommunityPriceEditPopup::create(listing, [self]() {
            if (self && self->isRunning()) self->loadMarketplace();
        })) popup->show();
    }

    void requestCancel(const CommunityShopUI::Listing& listing) {
        if (m_busy) return;
        m_busy = true;
        matjson::Value payload = matjson::Value::object();
        payload.set("listing_id", listing.id);
        claimOnServerEx("/community-shop/cancel", payload,
            [this, listing, keepAlive = Ref<CCNode>(this)](bool ok, int, const matjson::Value& data) {
                m_busy = false;
                if (!ok) {
                    FLAlertLayer::create(
                        "Community Shop",
                        CommunityShopUI::errorMessage(data, "Could not cancel the listing.").c_str(),
                        "OK"
                    )->show();
                    loadMarketplace();
                    return;
                }
                CommunityShopUI::applyBalances(data);
                if (listing.itemType == "badge" || listing.itemType == "banner") {
                    CommunityShopUI::setCosmeticOwned(listing.itemType, listing.itemID, true);
                }
                Notification::create("Listing cancelled", NotificationIcon::Success)->show();
                loadMarketplace();
            }
        );
    }

    bool init() override {
        if (!Popup::init(430.f, 310.f, "geode.loader/GE_square03.png")) return false;
        this->setTitle("Community Shop");

        addBalance(105.f, 265.f, "gem.png"_spr, g_streakData.gems, m_gemsLabel);
        addBalance(215.f, 265.f, "super_star.png"_spr, g_streakData.superStars, m_starsLabel);
        addBalance(325.f, 265.f, "star_tiket.png"_spr, g_streakData.starTickets, m_ticketsLabel);

        auto topMenu = CCMenu::create();
        topMenu->setPosition({ 0.f, 0.f });
        m_mainLayer->addChild(topMenu, 10);

        auto marketSprite = ButtonSprite::create("Market", 0, false, "goldFont.fnt", "GJ_button_01.png", 0, 0.5f);
        m_marketTab = CCMenuItemSpriteExtra::create(marketSprite, this, menu_selector(CommunityShopPopup::onMarketTab));
        m_marketTab->setPosition({ 78.f, 230.f });
        topMenu->addChild(m_marketTab);

        auto mineSprite = ButtonSprite::create("My Listings", 0, false, "goldFont.fnt", "GJ_button_01.png", 0, 0.5f);
        m_mineTab = CCMenuItemSpriteExtra::create(mineSprite, this, menu_selector(CommunityShopPopup::onMineTab));
        m_mineTab->setPosition({ 180.f, 230.f });
        topMenu->addChild(m_mineTab);

        auto sellSprite = ButtonSprite::create("Sell", 0, false, "goldFont.fnt", "GJ_button_02.png", 0, 0.52f);
        auto sellBtn = CCMenuItemSpriteExtra::create(sellSprite, this, menu_selector(CommunityShopPopup::onSell));
        sellBtn->setPosition({ 325.f, 230.f });
        topMenu->addChild(sellBtn);

        auto refreshSprite = CCSprite::createWithSpriteFrameName("GJ_updateBtn_001.png");
        refreshSprite->setScale(0.55f);
        m_refreshBtn = CCMenuItemSpriteExtra::create(refreshSprite, this, menu_selector(CommunityShopPopup::onRefresh));
        m_refreshBtn->setPosition({ 397.f, 230.f });
        topMenu->addChild(m_refreshBtn);

        m_statusLabel = CCLabelBMFont::create("Loading marketplace...", "chatFont.fnt");
        m_statusLabel->setScale(0.52f);
        m_statusLabel->setColor({ 190, 215, 255 });
        m_statusLabel->setAnchorPoint({ 1.f, 0.5f });
        m_statusLabel->setPosition({ 408.f, 205.f });
        m_mainLayer->addChild(m_statusLabel, 4);

        auto listBackground = CCScale9Sprite::create("square02_001.png");
        listBackground->setContentSize({ 398.f, 192.f });
        listBackground->setPosition({ 213.f, 107.f });
        listBackground->setColor({ 10, 17, 34 });
        listBackground->setOpacity(145);
        m_mainLayer->addChild(listBackground, 3);

        m_scroll = ScrollLayer::create({ 390.f, 184.f });
        m_scroll->setPosition({ 18.f, 15.f });
        m_mainLayer->addChild(m_scroll, 4);
        addScrollbar(m_scroll, 6.f, m_mainLayer);

        rebuildList();
        loadMarketplace();
        return true;
    }

public:
    static CommunityShopPopup* create() {
        auto ret = new CommunityShopPopup();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};
