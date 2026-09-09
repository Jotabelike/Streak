#pragma once
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/ScrollLayer.hpp>
#include <Geode/ui/Notification.hpp>
#include "../StreakData.h"
#include "../FirebaseManager.h"
#include "../BadgeNotification.h"
#include "../BannerNotification.h"
#include "../RewardNotification.h"
#include "../NameModifiers.h"
#include "../utils/ScrollbarUtils.h"
#include "../utils/RoundedProgressBar.h"
#include "PurchaseConfirmPopup.h"
#include "SharedVisuals.h"
#include "StreakChestPopup.h"

using namespace geode::prelude;

// Tienda de temporada. El catalogo entero (items, precios, stock) lo define el
// servidor en settings/season_shop; aqui solo se dibuja y se manda el itemId al
// comprar. Las badges se pagan con gemas y los cofres con estrellas. El catalogo
// se presenta como un carril horizontal y puede incluir un premio mayor bloqueado
// por gasto acumulado en gemas.
class SeasonShopPopup : public Popup {
protected:
    ScrollLayer* m_scroll = nullptr;
    CCLabelBMFont* m_timerLabel = nullptr;
    CCLabelBMFont* m_gemLabel = nullptr;
    CCLabelBMFont* m_starLabel = nullptr;
    CCLabelBMFont* m_spendLabel = nullptr;
    RoundedProgressBar* m_spendBar = nullptr;
    bool m_busy = false;

    static constexpr float LIST_W = 428.f;
    static constexpr float LIST_H = 180.f;
    static constexpr float CELL_W = 104.f;
    static constexpr float FEATURED_W = 132.f;
    static constexpr float CELL_H = 82.f;
    static constexpr float FEATURED_H = 170.f;
    static constexpr float ICON_BOX = 42.f;
    static constexpr float FEATURED_ICON_BOX = 82.f;
    static constexpr float CARD_GAP = 7.f;
    // A partir de este precio la carta lleva fondo degradado animado, como los
    // tiers milestone del pase.
    static constexpr int MULTICOLOR_PRICE = 800;
    // Achatado vertical del boton de precio.
    static constexpr float BTN_SQUASH_Y = 0.72f;

    struct AnimatedGradient {
        CCLayerGradient* layer;
        float hue;
    };
    std::vector<AnimatedGradient> m_animatedGradients;

    int grandPrizeRequirement() const {
        int required = 0;
        for (const auto& item : g_streakData.seasonShop.items) {
            required = std::max(required, item.unlockSpend);
        }
        return required;
    }

    bool usesStars(const StreakData::SeasonShopItem& item) const {
        return item.currency == "stars";
    }

    int currencyBalance(const StreakData::SeasonShopItem& item) const {
        return usesStars(item) ? g_streakData.superStars : g_streakData.gems;
    }

    PurchaseCurrency purchaseCurrency(const StreakData::SeasonShopItem& item) const {
        return usesStars(item) ? PurchaseCurrency::SuperStars : PurchaseCurrency::Gems;
    }

    std::string itemSpriteName(const StreakData::SeasonShopItem& item) const {
        if (item.type == "banner") {
            if (auto info = g_streakData.getBannerInfo(item.rewardId)) return info->spriteName;
        } else if (item.type == "badge") {
            if (auto info = g_streakData.getBadgeInfo(item.rewardId)) return info->spriteName;
        } else if (item.type == "song") {
            if (auto info = g_streakData.getSongInfo(item.rewardId)) return info->iconName;
        } else if (item.type == "chest") {
            return fmt::format("{}/ChestStar{}.png", Mod::get()->getID(), std::clamp(item.amount, 1, 6));
        } else if (item.type == "tickets") {
            return "star_tiket.png"_spr;
        } else if (item.type == "stars") {
            return "super_star.png"_spr;
        } else if (item.type == "gems") {
            return "gem.png"_spr;
        } else if (item.type == "shields") {
            return "heart.png"_spr;
        }
        return "GJ_unknownBtn_001.png";
    }

    // Fondo redondeado con degradado, mismo recorte que usa el pase.
    CCNode* makeRoundedGradient(float w, float h, ccColor3B a, ccColor3B b, GLubyte opacity,
                                CCLayerGradient** outGradient) {
        auto wrap = CCNode::create();
        wrap->setContentSize({ w, h });
        wrap->ignoreAnchorPointForPosition(false);
        wrap->setAnchorPoint({ 0.5f, 0.5f });

        auto stencil = cocos2d::extension::CCScale9Sprite::create("square02_small.png");
        stencil->setContentSize({ w, h });
        stencil->setAnchorPoint({ 0.5f, 0.5f });
        stencil->setPosition({ w / 2.f, h / 2.f });

        auto clipper = CCClippingNode::create(stencil);
        clipper->setAlphaThreshold(0.05f);

        auto gradient = CCLayerGradient::create(
            ccc4(a.r, a.g, a.b, opacity),
            ccc4(b.r, b.g, b.b, opacity),
            ccp(1, -1)
        );
        gradient->setContentSize({ w, h });
        clipper->addChild(gradient);

        wrap->addChild(clipper);
        if (outGradient) *outGradient = gradient;
        return wrap;
    }

    // Icono de la entrada: el cosmetico real cuando lo hay, y si no el cofre o
    // la moneda que representa.
    CCNode* createItemIcon(const StreakData::SeasonShopItem& item, float box, bool dim = false) {
        auto node = CCNode::create();
        node->setContentSize({ box, box });
        node->ignoreAnchorPointForPosition(false);
        node->setAnchorPoint({ 0.5f, 0.5f });

        CCSprite* spr = nullptr;
        if (item.type == "banner") {
            if (auto info = g_streakData.getBannerInfo(item.rewardId)) spr = CCSprite::create(info->spriteName.c_str());
        } else if (item.type == "badge") {
            if (auto info = g_streakData.getBadgeInfo(item.rewardId)) spr = CCSprite::create(info->spriteName.c_str());
        } else if (item.type == "song") {
            if (auto info = g_streakData.getSongInfo(item.rewardId)) spr = CCSprite::create(info->iconName.c_str());
        } else if (item.type == "chest") {
            // Mismos skins de star chest que usa el pase.
            std::string path = fmt::format("{}/ChestStar{}.png", Mod::get()->getID(), std::clamp(item.amount, 1, 6));
            spr = CCSprite::create(path.c_str());
            if (!spr) spr = CCSprite::createWithSpriteFrameName("chest_02_02_001.png");
        } else if (item.type == "tickets") {
            spr = CCSprite::create("star_tiket.png"_spr);
        } else if (item.type == "stars") {
            spr = CCSprite::create("super_star.png"_spr);
        } else if (item.type == "gems") {
            spr = CCSprite::create("gem.png"_spr);
        } else if (item.type == "shields") {
            spr = CCSprite::create("heart.png"_spr);
        } else if (item.type == "name_item") {
            // Preview real del color/efecto sobre un texto, como en el pase.
            auto preview = CCLabelBMFont::create("Name", "bigFont.fnt");
            if (preview) {
                NameModifiers::applyColor(preview, item.rewardId);
                float pw = std::max(preview->getContentSize().width, 1.f);
                float ph = std::max(preview->getContentSize().height, 1.f);
                preview->setScale(std::min(box / pw, box / ph));
                preview->setPosition({ box / 2.f, box / 2.f });
                if (dim) preview->setOpacity(110);
                node->addChild(preview);
                return node;
            }
        }
        if (!spr) spr = CCSprite::createWithSpriteFrameName("GJ_unknownBtn_001.png");

        float maxDim = std::max({ spr->getContentSize().width, spr->getContentSize().height, 1.f });
        spr->setScale(box / maxDim);
        spr->setPosition({ box / 2.f, box / 2.f });
        if (dim) {
            spr->setOpacity(110);
            spr->setColor({ 150, 150, 150 });
        }
        node->addChild(spr);
        return node;
    }

    std::string itemLabel(const StreakData::SeasonShopItem& item) const {
        if (!item.name.empty()) return item.name;
        if (item.type == "banner") {
            if (auto info = g_streakData.getBannerInfo(item.rewardId)) return info->displayName;
        } else if (item.type == "badge") {
            if (auto info = g_streakData.getBadgeInfo(item.rewardId)) return info->displayName;
        } else if (item.type == "song") {
            if (auto info = g_streakData.getSongInfo(item.rewardId)) return info->displayName;
        } else if (item.type == "chest") {
            return fmt::format("Chest {}*", item.amount);
        } else if (item.type == "shields") {
            return fmt::format("Shields x{}", item.amount);
        } else if (item.type == "tickets" || item.type == "stars" || item.type == "gems") {
            return fmt::format("x{}", item.amount);
        }
        return item.rewardId.empty() ? item.itemId : item.rewardId;
    }

    // Un cosmetico que el jugador ya posee no se puede volver a comprar (el
    // servidor tambien lo rechaza con 409).
    bool alreadyOwned(const StreakData::SeasonShopItem& item) const {
        if (item.type == "banner")    return g_streakData.isBannerUnlocked(item.rewardId);
        if (item.type == "badge")     return g_streakData.isBadgeUnlocked(item.rewardId);
        if (item.type == "song")      return g_streakData.isSongUnlocked(item.rewardId);
        if (item.type == "name_item") return g_streakData.isNameItemUnlocked(item.rewardId);
        return false;
    }

    void refreshBalances() {
        if (m_gemLabel) m_gemLabel->setString(fmt::format("{}", g_streakData.gems).c_str());
        if (m_starLabel) m_starLabel->setString(fmt::format("{}", g_streakData.superStars).c_str());
        const int required = grandPrizeRequirement();
        const int spent = std::max(0, g_streakData.seasonShop.gemsSpent);
        if (m_spendLabel) {
            m_spendLabel->setVisible(required > 0);
            m_spendLabel->setString(required > 0
                ? fmt::format("{}/{} GEMS", std::min(spent, required), required).c_str()
                : "");
        }
        if (m_spendBar) {
            m_spendBar->setVisible(required > 0);
            m_spendBar->setProgress(required > 0 ? static_cast<float>(spent) / static_cast<float>(required) : 0.f);
        }
    }

    void updateTimer(float) {
        if (!m_timerLabel) return;
        long long endsAt = g_streakData.seasonShop.endsAt;
        if (endsAt <= 0) { m_timerLabel->setString(""); return; }
        long long remaining = (endsAt - g_streakData.getServerNowMs()) / 1000;
        if (remaining <= 0) { m_timerLabel->setString("CLOSED"); return; }
        long long days = remaining / 86400;
        long long hours = (remaining % 86400) / 3600;
        long long mins = (remaining % 3600) / 60;
        if (days > 0)       m_timerLabel->setString(fmt::format("{}d {}h", days, hours).c_str());
        else if (hours > 0) m_timerLabel->setString(fmt::format("{}h {}m", hours, mins).c_str());
        else                m_timerLabel->setString(fmt::format("{}m", mins).c_str());
    }

    CCNode* buildCell(const StreakData::SeasonShopItem& item, int index, float cellW) {
        const float cellH = item.featured ? FEATURED_H : CELL_H;
        auto cell = CCNode::create();
        cell->setContentSize({ cellW, cellH });

        const bool owned = alreadyOwned(item);
        const bool soldOut = item.soldOut();
        const bool requirementLocked = item.unlockSpend > 0 &&
            g_streakData.seasonShop.gemsSpent < item.unlockSpend;
        const bool locked = owned || soldOut || requirementLocked;

        float bgW = cellW - 6.f, bgH = cellH - 5.f;
        if (item.featured || (!locked && item.price > MULTICOLOR_PRICE)) {
            CCLayerGradient* gradient = nullptr;
            auto bg = makeRoundedGradient(bgW, bgH,
                item.featured ? ccColor3B{ 0, 205, 230 } : ccColor3B{ 90, 200, 255 },
                item.featured ? ccColor3B{ 15, 15, 20 } : ccColor3B{ 200, 120, 255 },
                requirementLocked ? 145 : 220, &gradient);
            bg->setPosition({ cellW / 2.f, cellH / 2.f });
            cell->addChild(bg);
            if (gradient) m_animatedGradients.push_back({ gradient, 0.f });
        } else {
            auto bg = CCScale9Sprite::create("square02_001.png");
            bg->setContentSize({ bgW, bgH });
            bg->setPosition({ cellW / 2.f, cellH / 2.f });
            bg->setColor(locked ? ccColor3B{ 0, 0, 0 } : ccColor3B{ 20, 30, 60 });
            bg->setOpacity(locked ? 130 : 170);
            cell->addChild(bg);
        }

        if (item.featured) {
            auto featured = CCLabelBMFont::create("GRAND PRIZE", "goldFont.fnt");
            featured->setScale(0.34f);
            featured->setColor({ 100, 255, 255 });
            featured->setPosition({ cellW / 2.f, cellH - 13.f });
            cell->addChild(featured, 4);

            m_spendLabel = CCLabelBMFont::create("", "goldFont.fnt");
            m_spendLabel->setScale(0.24f);
            m_spendLabel->setColor({ 110, 245, 255 });
            m_spendLabel->setPosition({ cellW / 2.f, 47.f });
            cell->addChild(m_spendLabel, 5);

            m_spendBar = RoundedProgressBar::create(cellW - 22.f, 8.f);
            m_spendBar->setGradientColors({ 0, 225, 240 }, { 8, 8, 12 });
            m_spendBar->setBackgroundColor({ 18, 25, 36 });
            m_spendBar->setPosition({ cellW / 2.f, 36.f });
            cell->addChild(m_spendBar, 5);
        }

        const int shownUnit = item.stock > 0 ? std::min(item.bought + 1, item.stock) : item.bought + 1;
        const std::string unitText = item.stock > 0
            ? fmt::format("{}/{}", shownUnit, item.stock)
            : fmt::format("{}", shownUnit);
        auto unit = CCLabelBMFont::create(unitText.c_str(), "goldFont.fnt");
        unit->setScale(item.featured ? 0.31f : 0.27f);
        unit->setColor({ 255, 238, 170 });
        unit->setPosition({ cellW / 2.f, cellH - (item.featured ? 30.f : 10.f) });
        cell->addChild(unit, 4);

        const float iconBox = item.featured ? FEATURED_ICON_BOX : ICON_BOX;
        auto icon = createItemIcon(item, iconBox, locked && !owned);
        icon->setPosition({ cellW / 2.f, item.featured ? 94.f : 41.f });
        cell->addChild(icon, 2);

        if (locked) {
            std::string status = owned ? "Owned" : (soldOut ? "Sold out" :
                fmt::format("Spend {} more", item.unlockSpend - g_streakData.seasonShop.gemsSpent));
            auto lbl = CCLabelBMFont::create(status.c_str(), "goldFont.fnt");
            lbl->setScale(requirementLocked ? 0.27f : 0.3f);
            lbl->setColor(requirementLocked ? ccColor3B{ 100, 245, 255 } : ccColor3B{ 170, 170, 170 });
            lbl->limitLabelWidth(cellW - 8.f, requirementLocked ? 0.27f : 0.3f, 0.18f);
            lbl->setPosition({ cellW / 2.f, item.featured ? 18.f : 11.f });
            cell->addChild(lbl, 2);
        } else {
            const bool affordable = currencyBalance(item) >= item.price;
            auto btnSpr = ButtonSprite::create(
                fmt::format("  {}", item.price).c_str(), 0, false, "bigFont.fnt",
                affordable ? "GJ_button_01.png" : "GJ_button_06.png", 0, 0.42f);

            auto currencyIcon = CCSprite::create(usesStars(item) ? "super_star.png"_spr : "gem.png"_spr);
            if (currencyIcon) {
                currencyIcon->setScale(usesStars(item) ? 0.135f : 0.13f);
                currencyIcon->setPosition({ 11.f, btnSpr->getContentSize().height / 2.f });
                btnSpr->addChild(currencyIcon);
            }
            float maxBtnW = cellW - 10.f;
            if (btnSpr->getScaledContentSize().width > maxBtnW) {
                btnSpr->setScale(btnSpr->getScale() * maxBtnW / btnSpr->getScaledContentSize().width);
            }
            // Boton mas plano: se achata SOLO la textura de fondo. Al ser un
            // 9-slice se le baja el alto de su content size, asi las esquinas no
            // se deforman y ni el numero ni la gema se ven aplastados.
            if (auto bg = btnSpr->m_BGSprite) {
                auto bgSize = bg->getContentSize();
                bg->setContentSize({ bgSize.width, bgSize.height * BTN_SQUASH_Y });
                bg->setPosition({ btnSpr->getContentSize().width / 2.f, btnSpr->getContentSize().height / 2.f });
            }

            auto btn = CCMenuItemSpriteExtra::create(btnSpr, this, menu_selector(SeasonShopPopup::onBuy));
            btn->setTag(index);
            auto menu = CCMenu::createWithItem(btn);
            menu->setPosition({ cellW / 2.f, item.featured ? 18.f : 11.f });
            cell->addChild(menu, 5);
        }

        return cell;
    }

    void rebuildList(bool keepPosition = false) {
        if (!m_scroll) return;
        float preservedX = m_scroll->m_contentLayer->getPositionX();
        m_scroll->m_contentLayer->removeAllChildren();
        m_spendLabel = nullptr;
        m_spendBar = nullptr;
        m_animatedGradients.clear();

        const auto& items = g_streakData.seasonShop.items;
        int featuredCount = 0;
        int regularCount = 0;
        for (const auto& item : items) {
            if (item.featured) ++featuredCount;
            else ++regularCount;
        }
        const int regularColumns = (regularCount + 1) / 2;
        float totalWidth = 10.f +
            featuredCount * (FEATURED_W + CARD_GAP) +
            regularColumns * (CELL_W + CARD_GAP);
        totalWidth = std::max(m_scroll->getContentSize().width, totalWidth + 3.f);
        m_scroll->m_contentLayer->setContentSize({ totalWidth, LIST_H });

        float featuredX = 7.f;
        for (int i = 0; i < (int)items.size(); ++i) {
            if (!items[i].featured) continue;
            auto cell = buildCell(items[i], i, FEATURED_W);
            cell->setPosition({ featuredX, (LIST_H - FEATURED_H) / 2.f });
            m_scroll->m_contentLayer->addChild(cell);
            featuredX += FEATURED_W + CARD_GAP;
        }

        const float regularStartX = featuredX;
        int regularIndex = 0;
        for (int i = 0; i < (int)items.size(); ++i) {
            if (items[i].featured) continue;
            const int column = regularIndex / 2;
            const int row = regularIndex % 2;
            auto cell = buildCell(items[i], i, CELL_W);
            cell->setPosition({
                regularStartX + column * (CELL_W + CARD_GAP),
                row == 0 ? LIST_H - CELL_H - 4.f : 4.f
            });
            m_scroll->m_contentLayer->addChild(cell);
            ++regularIndex;
        }

        if (items.empty()) {
            auto empty = CCLabelBMFont::create("The shop is empty right now.", "bigFont.fnt");
            empty->setScale(0.4f);
            empty->setColor({ 190, 190, 190 });
            empty->setPosition({ LIST_W / 2.f, LIST_H / 2.f });
            m_scroll->m_contentLayer->addChild(empty);
        }

        if (keepPosition) {
            float minX = std::min(0.f, m_scroll->getContentSize().width - totalWidth);
            m_scroll->m_contentLayer->setPositionX(std::clamp(preservedX, minX, 0.f));
        } else {
            m_scroll->m_contentLayer->setPositionX(0.f);
        }
        refreshBalances();
    }

    void onBuy(CCObject* sender) {
        if (m_busy) return;
        int index = static_cast<CCNode*>(sender)->getTag();
        auto& items = g_streakData.seasonShop.items;
        if (index < 0 || index >= (int)items.size()) return;
        const auto item = items[index];

        if (item.unlockSpend > 0 && g_streakData.seasonShop.gemsSpent < item.unlockSpend) {
            FLAlertLayer::create("Grand Prize",
                fmt::format("Spend <cy>{}</c> more gems in this Season Shop to unlock it.",
                    item.unlockSpend - g_streakData.seasonShop.gemsSpent).c_str(), "OK")->show();
            return;
        }

        auto popup = PurchaseConfirmPopup::create(
            itemSpriteName(item), itemLabel(item), item.price, purchaseCurrency(item),
            [this, item, keepAlive = Ref<CCNode>(this)](int discountPercent) {
                int finalPrice = discountedPurchasePrice(item.price, discountPercent);
                if (currencyBalance(item) < finalPrice) {
                    FLAlertLayer::create("Season Shop",
                        usesStars(item) ? "You don't have enough <cy>stars</c> for this."
                                        : "You don't have enough <cl>gems</c> for this.",
                        "OK")->show();
                    return;
                }
                this->requestPurchase(item, discountPercent);
            });
        if (popup) popup->show();
    }

    void requestPurchase(const StreakData::SeasonShopItem& item, int discountPercent) {
        if (m_busy) return;
        m_busy = true;

        matjson::Value payload = matjson::Value::object();
        payload.set("item_id", item.itemId);
        payload.set("discount_percent", discountPercent);

        claimOnServerEx("/season-shop/purchase", payload,
            [this, item, keepAlive = Ref<CCNode>(this)](bool ok, int code, const matjson::Value& data) {
                m_busy = false;
                if (!ok) {
                    std::string msg = "Could not complete the purchase. Try again.";
                    if (code == 402) msg = usesStars(item)
                        ? "You don't have enough stars for this."
                        : "You don't have enough gems for this.";
                    else if (code == 409) msg = "You already own this, or it's sold out.";
                    else if (code == 423) {
                        int spent = data["gems_spent"].as<int>().unwrapOr(g_streakData.seasonShop.gemsSpent);
                        int required = data["required_spend"].as<int>().unwrapOr(item.unlockSpend);
                        msg = fmt::format("Spend {} more gems in this Season Shop to unlock it.",
                            std::max(0, required - spent));
                    }
                    else if (code == 403 || code == 404) msg = "This offer is no longer available.";
                    FLAlertLayer::create("Season Shop", msg.c_str(), "OK")->show();
                    rebuildList(true);
                    return;
                }
                applyPurchase(item, data);
            });
    }

    // El servidor ya cobro la moneda correspondiente y registro la compra; aqui
    // se sincronizan los saldos y se entrega lo comprado.
    // en local y se entrega lo comprado.
    void applyPurchase(const StreakData::SeasonShopItem& item, const matjson::Value& data) {
        const int fallbackSpent = g_streakData.seasonShop.gemsSpent +
            (usesStars(item) ? 0 : data["purchase"]["final_price"].as<int>().unwrapOr(item.price));
        g_streakData.seasonShop.gemsSpent = data["gems_spent"].as<int>().unwrapOr(fallbackSpent);
        if (data.contains("balances")) {
            auto balances = data["balances"];
            g_streakData.gems = balances["gems"].as<int>().unwrapOr(g_streakData.gems);
            g_streakData.superStars = balances["super_stars"].as<int>().unwrapOr(g_streakData.superStars);
            g_streakData.starTickets = balances["star_tickets"].as<int>().unwrapOr(g_streakData.starTickets);
            g_streakData.streakShields = balances["streak_shields"].as<int>().unwrapOr(g_streakData.streakShields);
        }
        for (auto& it : g_streakData.seasonShop.items) {
            if (it.itemId == item.itemId) {
                it.bought = data["bought"].as<int>().unwrapOr(it.bought + 1);
                break;
            }
        }

        if (item.type == "chest") {
            // Star chest normal: animacion, skin por rareza y contenido por
            // /chest/claim, igual que los cofres del pase.
            int rarity = std::clamp(data["chest_rarity"].as<int>().unwrapOr(item.amount), 1, 6);
            int stars = 0, tickets = 0, gems = 0, xp = 0;
            StreakChestPopup::rollRewardsForRarity(rarity, stars, tickets, gems, xp);
            auto reload = [this]() { this->rebuildList(true); };
            if (auto popup = StreakChestPopup::create(stars, tickets, gems, xp, rarity, reload)) {
                popup->show();
            }
            rebuildList(true);
            return;
        }

        if (item.type == "banner") {
            g_streakData.unlockBanner(item.rewardId);
            if (auto info = g_streakData.getBannerInfo(item.rewardId)) {
                BannerNotification::show(item.rewardId, info->spriteName, info->displayName,
                    "SHOP", { 120, 200, 255 });
            }
        } else if (item.type == "badge") {
            g_streakData.unlockBadge(item.rewardId);
            if (auto info = g_streakData.getBadgeInfo(item.rewardId);
                info && StreakData::usesMythicPresentation(info->category)) {
                auto animation = MythicAnimationLayer::create(*info, [id = item.rewardId]() {
                    BadgeNotification::show(id);
                });
                CCDirector::sharedDirector()->getRunningScene()->addChild(animation, 99999);
            } else {
                BadgeNotification::show(item.rewardId);
            }
        } else if (item.type == "song") {
            g_streakData.unlockSong(item.rewardId);
            if (auto info = g_streakData.getSongInfo(item.rewardId)) {
                BannerNotification::show(item.rewardId, info->iconName, info->displayName,
                    "MUSIC", { 255, 200, 80 }, "SONG UNLOCKED!");
            }
        } else if (item.type == "name_item") {
            g_streakData.unlockNameItem(item.rewardId);
            FLAlertLayer::create("Name Item",
                fmt::format("Unlocked <cy>{}</c>! Equip it from your name customization.", item.rewardId).c_str(),
                "OK")->show();
        } else {
            const char* spr = "gem.png"_spr;
            if (item.type == "tickets") spr = "star_tiket.png"_spr;
            else if (item.type == "stars") spr = "super_star.png"_spr;
            else if (item.type == "shields") spr = "heart.png"_spr;
            int shownAmount = item.amount;
            if (item.type == "shields" &&
                data.contains("shield_conversion") && !data["shield_conversion"].isNull()) {
                int converted = data["shield_conversion"]["converted_shields"].as<int>().unwrapOr(0);
                shownAmount = std::max(0, item.amount - converted);
            }
            if (shownAmount > 0) RewardNotification::show(spr, 0, shownAmount);
        }

        FMODAudioEngine::sharedEngine()->playEffect("secretKey.wav");
        rebuildList(true);
    }

    void update(float dt) override {
        if (m_scroll && m_scroll->m_contentLayer) {
            float contentW = m_scroll->m_contentLayer->getContentSize().width;
            float viewW = m_scroll->getContentSize().width;
            float minX = std::min(0.f, viewW - contentW);
            float currentX = m_scroll->m_contentLayer->getPositionX();
            if (currentX > 0.f) m_scroll->m_contentLayer->setPositionX(0.f);
            else if (currentX < minX) m_scroll->m_contentLayer->setPositionX(minX);
        }
        for (auto& ag : m_animatedGradients) {
            if (!ag.layer) continue;
            ag.hue += dt * 0.35f;
            if (ag.hue > 1.f) ag.hue -= 1.f;
            float h2 = ag.hue + 0.18f;
            if (h2 > 1.f) h2 -= 1.f;
            ag.layer->setStartColor(HSVtoRGB(ag.hue, 0.55f, 1.f));
            ag.layer->setEndColor(HSVtoRGB(h2, 0.65f, 1.f));
        }
    }

    bool init() override {
        if (!Popup::init(460.f, 292.f, "geode.loader/GE_square03.png")) return false;
        auto winSize = m_mainLayer->getContentSize();

        const auto& shop = g_streakData.seasonShop;
        this->setTitle(shop.title.empty() ? "Season Shop" : shop.title.c_str());
        // El popup es estrecho: el titulo se encoge para no chocar con las
        // gemas ni con el contador de cierre.
        if (m_title) {
            float maxTitleW = winSize.width - 240.f;
            float titleW = m_title->getScaledContentSize().width;
            if (titleW > maxTitleW) m_title->setScale(m_title->getScale() * maxTitleW / titleW);
        }

        float headerY = winSize.height - 19.f;
        if (auto gem = CCSprite::create("gem.png"_spr)) {
            gem->setScale(0.17f);
            gem->setPosition({ 18.f, headerY });
            m_mainLayer->addChild(gem, 5);
        }
        m_gemLabel = CCLabelBMFont::create("0", "bigFont.fnt");
        m_gemLabel->setAnchorPoint({ 0.f, 0.5f });
        m_gemLabel->setScale(0.28f);
        m_gemLabel->setPosition({ 27.f, headerY });
        m_mainLayer->addChild(m_gemLabel, 5);

        if (auto star = CCSprite::create("super_star.png"_spr)) {
            star->setScale(0.16f);
            star->setPosition({ 76.f, headerY });
            m_mainLayer->addChild(star, 5);
        }
        m_starLabel = CCLabelBMFont::create("0", "bigFont.fnt");
        m_starLabel->setAnchorPoint({ 0.f, 0.5f });
        m_starLabel->setScale(0.28f);
        m_starLabel->setPosition({ 87.f, headerY });
        m_mainLayer->addChild(m_starLabel, 5);

        m_timerLabel = CCLabelBMFont::create("", "goldFont.fnt");
        m_timerLabel->setAnchorPoint({ 1.f, 0.5f });
        m_timerLabel->setScale(0.32f);
        m_timerLabel->setPosition({ winSize.width - 16.f, headerY });
        m_mainLayer->addChild(m_timerLabel, 5);

        m_scroll = ScrollLayer::create({ LIST_W, LIST_H }, true, false);
        m_scroll->setPosition({ (winSize.width - LIST_W) / 2.f, 42.f });
        m_mainLayer->addChild(m_scroll, 4);

        auto dragHint = CCLabelBMFont::create("Drag sideways to browse", "goldFont.fnt");
        dragHint->setScale(0.24f);
        dragHint->setColor({ 170, 185, 205 });
        dragHint->setPosition({ winSize.width / 2.f, 22.f });
        m_mainLayer->addChild(dragHint, 5);

        rebuildList();
        updateTimer(0.f);
        this->schedule(schedule_selector(SeasonShopPopup::updateTimer), 1.0f);
        this->scheduleUpdate();

        // El catalogo y el stock son del servidor: se repiden al abrir para no
        // quedarse con lo que se cargo al entrar al menu (si el admin cambia la
        // tienda, la sesion abierta seguiria viendo la version vieja).
        Ref<SeasonShopPopup> self = this;
        refreshPlayerDataFromServer([self](bool ok) {
            if (!ok || !self->isRunning()) return;
            self->rebuildList();
            self->updateTimer(0.f);
        });
        return true;
    }

public:
    static SeasonShopPopup* create() {
        auto ret = new SeasonShopPopup();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};
