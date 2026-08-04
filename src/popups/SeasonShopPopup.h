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
#include "StreakChestPopup.h"

using namespace geode::prelude;

// Tienda de temporada. El catalogo entero (items, precios, stock) lo define el
// servidor en settings/season_shop; aqui solo se dibuja y se manda el itemId al
// comprar. Todo se paga con gemas. Se muestra en rejilla de 3 columnas por 3
// filas visibles; si hay mas entradas (el catalogo base trae 12) la lista
// scrollea.
class SeasonShopPopup : public Popup {
protected:
    ScrollLayer* m_scroll = nullptr;
    CCLabelBMFont* m_timerLabel = nullptr;
    CCLabelBMFont* m_gemLabel = nullptr;
    bool m_busy = false;

    // Casillas en formato carta: mas altas que anchas, con el premio ocupando
    // casi todo y solo el precio debajo.
    static constexpr int COLUMNS = 3;
    static constexpr int VISIBLE_ROWS = 3;
    static constexpr float CELL_W = 70.f;
    static constexpr float CELL_H = 84.f;
    static constexpr float ICON_BOX = 52.f;
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
        auto cell = CCNode::create();
        cell->setContentSize({ cellW, CELL_H });

        const bool owned = alreadyOwned(item);
        const bool soldOut = item.soldOut();
        const bool locked = owned || soldOut;

        // Las cartas caras llevan degradado animado; el resto, fondo plano.
        float bgW = cellW - 6.f, bgH = CELL_H - 5.f;
        if (!locked && item.price > MULTICOLOR_PRICE) {
            CCLayerGradient* gradient = nullptr;
            auto bg = makeRoundedGradient(bgW, bgH, { 90, 200, 255 }, { 200, 120, 255 }, 220, &gradient);
            bg->setPosition({ cellW / 2.f, CELL_H / 2.f });
            cell->addChild(bg);
            if (gradient) m_animatedGradients.push_back({ gradient, 0.f });
        } else {
            auto bg = CCScale9Sprite::create("square02_001.png");
            bg->setContentSize({ bgW, bgH });
            bg->setPosition({ cellW / 2.f, CELL_H / 2.f });
            bg->setColor(locked ? ccColor3B{ 0, 0, 0 } : ccColor3B{ 20, 30, 60 });
            bg->setOpacity(locked ? 130 : 170);
            cell->addChild(bg);
        }

        auto icon = createItemIcon(item, ICON_BOX, locked);
        icon->setPosition({ cellW / 2.f, CELL_H / 2.f + 10.f });
        cell->addChild(icon, 2);

        // Cuantas quedan, solo en lo que se puede comprar varias veces: sin
        // esto, tras la primera compra la carta se ve igual y parece que ya no
        // se puede volver a comprar. Los cofres se saltan esto: la rareza ya
        // manda en la carta y el contador solo mete ruido.
        if (!locked && item.stock > 1 && item.type != "chest") {
            auto left = CCLabelBMFont::create(
                fmt::format("x{}", item.stock - item.bought).c_str(), "goldFont.fnt");
            left->setScale(0.24f);
            left->setColor({ 255, 235, 160 });
            left->setPosition({ cellW / 2.f, 27.f });
            cell->addChild(left, 3);
        }

        // Linea inferior: boton de precio, o el motivo por el que no se puede.
        if (locked) {
            auto lbl = CCLabelBMFont::create(owned ? "Owned" : "Sold out", "goldFont.fnt");
            lbl->setScale(0.26f);
            lbl->setColor({ 170, 170, 170 });
            lbl->setPosition({ cellW / 2.f, 14.f });
            cell->addChild(lbl, 2);
        } else {
            // El boton es SOLO el precio, no la carta entera: un boton que
            // cubriese toda la casilla se queda con el toque y la lista deja de
            // scrollear al arrastrar sobre ella.
            const bool affordable = g_streakData.gems >= item.price;
            auto btnSpr = ButtonSprite::create(
                fmt::format("  {}", item.price).c_str(), 0, false, "bigFont.fnt",
                affordable ? "GJ_button_01.png" : "GJ_button_06.png", 0, 0.42f);

            auto gemIcon = CCSprite::create("gem.png"_spr);
            if (gemIcon) {
                gemIcon->setScale(0.13f);
                gemIcon->setPosition({ 11.f, btnSpr->getContentSize().height / 2.f });
                btnSpr->addChild(gemIcon);
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
            menu->setPosition({ cellW / 2.f, 14.f });
            cell->addChild(menu, 5);
        }

        return cell;
    }

    // keepPosition solo al redibujar tras una compra; al abrir (o al llegar los
    // datos del servidor) la lista debe empezar arriba. Ojo: en ScrollLayer el
    // 0 es el FONDO de la lista y el tope es -contentH + viewH, asi que
    // conservar la posicion inicial dejaba la tienda abierta por abajo.
    void rebuildList(bool keepPosition = false) {
        if (!m_scroll) return;
        float preservedY = m_scroll->m_contentLayer->getPositionY();
        m_scroll->m_contentLayer->removeAllChildren();
        m_animatedGradients.clear();

        const auto& items = g_streakData.seasonShop.items;
        float width = m_scroll->getContentSize().width;
        float cellW = width / (float)COLUMNS;
        int rows = std::max(1, ((int)items.size() + COLUMNS - 1) / COLUMNS);
        float totalHeight = std::max(m_scroll->getContentSize().height, CELL_H * (float)rows);
        m_scroll->m_contentLayer->setContentSize({ width, totalHeight });

        for (int i = 0; i < (int)items.size(); ++i) {
            int row = i / COLUMNS;
            int col = i % COLUMNS;
            auto cell = buildCell(items[i], i, cellW);
            cell->setPosition({ cellW * (float)col, totalHeight - CELL_H * (float)(row + 1) });
            m_scroll->m_contentLayer->addChild(cell);
        }

        if (items.empty()) {
            auto empty = CCLabelBMFont::create("The shop is empty right now.", "bigFont.fnt");
            empty->setScale(0.4f);
            empty->setColor({ 190, 190, 190 });
            empty->setPosition({ width / 2.f, totalHeight / 2.f });
            m_scroll->m_contentLayer->addChild(empty);
        }

        if (keepPosition) {
            float topY = m_scroll->getContentSize().height - totalHeight; // <= 0
            m_scroll->m_contentLayer->setPositionY(std::clamp(preservedY, topY, 0.f));
        } else {
            m_scroll->moveToTop();
        }
        refreshBalances();
    }

    void onBuy(CCObject* sender) {
        if (m_busy) return;
        int index = static_cast<CCNode*>(sender)->getTag();
        auto& items = g_streakData.seasonShop.items;
        if (index < 0 || index >= (int)items.size()) return;
        const auto item = items[index];

        if (g_streakData.gems < item.price) {
            FLAlertLayer::create("Season Shop", "You don't have enough <cl>gems</c> for this.", "OK")->show();
            return;
        }

        createQuickPopup("Confirm Purchase",
            fmt::format("Buy <cy>{}</c> for <cl>{}</c> gems?", itemLabel(item), item.price),
            "Cancel", "Buy",
            [this, item, keepAlive = Ref<CCNode>(this)](auto, bool confirmed) {
                if (confirmed) this->requestPurchase(item);
            });
    }

    void requestPurchase(const StreakData::SeasonShopItem& item) {
        if (m_busy) return;
        m_busy = true;

        matjson::Value payload = matjson::Value::object();
        payload.set("item_id", item.itemId);

        claimOnServerEx("/season-shop/purchase", payload,
            [this, item, keepAlive = Ref<CCNode>(this)](bool ok, int code, const matjson::Value& data) {
                m_busy = false;
                if (!ok) {
                    const char* msg = "Could not complete the purchase. Try again.";
                    if (code == 402) msg = "You don't have enough gems for this.";
                    else if (code == 409) msg = "You already own this, or it's sold out.";
                    else if (code == 403 || code == 404) msg = "This offer is no longer available.";
                    FLAlertLayer::create("Season Shop", msg, "OK")->show();
                    rebuildList(true);
                    return;
                }
                applyPurchase(item, data);
            });
    }

    // El servidor ya cobro las gemas y registro la compra; aqui solo se refleja
    // en local y se entrega lo comprado.
    void applyPurchase(const StreakData::SeasonShopItem& item, const matjson::Value& data) {
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
            BadgeNotification::show(item.rewardId);
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
            RewardNotification::show(spr, 0, item.amount);
        }

        FMODAudioEngine::sharedEngine()->playEffect("secretKey.wav");
        rebuildList(true);
    }

    void update(float dt) override {
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
        // 3 columnas x 3 filas visibles. El ancho extra deja sitio al scrollbar
        // en el margen derecho sin comerse las cartas.
        float listW = COLUMNS * CELL_W;
        float listH = VISIBLE_ROWS * CELL_H;
        if (!Popup::init(listW + 42.f, listH + 46.f, "geode.loader/GE_square03.png")) return false;
        auto winSize = m_mainLayer->getContentSize();

        const auto& shop = g_streakData.seasonShop;
        this->setTitle(shop.title.empty() ? "Season Shop" : shop.title.c_str());
        // El popup es estrecho: el titulo se encoge para no chocar con las
        // gemas ni con el contador de cierre.
        if (m_title) {
            float maxTitleW = winSize.width - 96.f;
            float titleW = m_title->getScaledContentSize().width;
            if (titleW > maxTitleW) m_title->setScale(m_title->getScale() * maxTitleW / titleW);
        }

        // Gemas a la izquierda y cierre a la derecha, en la linea del titulo.
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

        m_timerLabel = CCLabelBMFont::create("", "goldFont.fnt");
        m_timerLabel->setAnchorPoint({ 1.f, 0.5f });
        m_timerLabel->setScale(0.32f);
        m_timerLabel->setPosition({ winSize.width - 16.f, headerY });
        m_mainLayer->addChild(m_timerLabel, 5);

        m_scroll = ScrollLayer::create({ listW, listH });
        // Desplazada a la izquierda del centro: el hueco de la derecha es del
        // scrollbar.
        m_scroll->setPosition({ (winSize.width - listW) / 2.f - 7.f, 8.f });
        m_mainLayer->addChild(m_scroll, 4);
        addScrollbar(m_scroll, 6.f, m_mainLayer);

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
