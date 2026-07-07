#pragma once
#include <Geode/ui/Popup.hpp>
#include "../StreakData.h"
#include "../FirebaseManager.h"
#include "../BadgeNotification.h"
#include "StreakChestPopup.h"
#include <functional>

using namespace geode::prelude;

extern void wcEventVoteOnServer(const std::string& matchId, const std::string& team,
                                std::function<void(bool, matjson::Value)> callback);
extern void wcEventClaimOnServer(const std::string& matchId,
                                 std::function<void(bool, matjson::Value)> callback);
extern void wcEventFetchStateOnServer(std::function<void(bool)> callback);

// Barra unica dividida del marcador: el lado A se llena desde la izquierda y
// el B desde la derecha (degradados verticales, mismo look que
// RoundedProgressBar). setSplit() la pinta al instante; animateTo() desliza el
// punto de corte con easing y notifica cada frame para animar los porcentajes.
class WcTugBar : public CCNode {
protected:
    CCLayerGradient* m_gradA = nullptr;
    CCLayerGradient* m_gradB = nullptr;
    CCLayerColor* m_divider = nullptr;
    float m_pad = 2.f;
    float m_innerW = 0.f;
    float m_innerH = 0.f;
    float m_current = 0.5f;
    float m_from = 0.5f;
    float m_target = 0.5f;
    float m_animT = 0.f;
    float m_animDur = 0.f;
    std::function<void(float)> m_onStep;

    bool init(float w, float h, ccColor3B colA, ccColor3B colB) {
        if (!CCNode::init()) return false;
        this->setContentSize({ w, h });
        this->setAnchorPoint({ 0.5f, 0.5f });
        this->ignoreAnchorPointForPosition(false);

        float renderScale = (h < 30.f) ? (30.f / h) : 1.f;

        auto bg = cocos2d::extension::CCScale9Sprite::create("square02_small.png");
        bg->setContentSize({ w * renderScale, h * renderScale });
        bg->setScale(1.f / renderScale);
        bg->setColor({ 30, 30, 45 });
        bg->setOpacity(210);
        bg->setPosition({ w / 2.f, h / 2.f });
        this->addChild(bg, 0);

        m_innerW = w - m_pad * 2.f;
        m_innerH = h - m_pad * 2.f;

        auto stencil = cocos2d::extension::CCScale9Sprite::create("square02_small.png");
        stencil->setContentSize({ m_innerW * renderScale, m_innerH * renderScale });
        stencil->setScale(1.f / renderScale);
        stencil->setAnchorPoint({ 0.f, 0.f });
        stencil->setPosition({ m_pad, m_pad });
        auto clipper = CCClippingNode::create(stencil);
        clipper->setAlphaThreshold(0.05f);
        this->addChild(clipper, 1);

        auto darker = [](ccColor3B c) {
            return ccColor3B{
                (GLubyte)(c.r * 0.55f), (GLubyte)(c.g * 0.55f), (GLubyte)(c.b * 0.55f)
            };
        };
        auto dA = darker(colA);
        auto dB = darker(colB);

        m_gradA = CCLayerGradient::create(
            ccc4(colA.r, colA.g, colA.b, 255), ccc4(dA.r, dA.g, dA.b, 255), ccp(0, -1));
        m_gradA->ignoreAnchorPointForPosition(false);
        m_gradA->setAnchorPoint({ 0.f, 0.f });
        m_gradA->setPosition({ m_pad, m_pad });
        clipper->addChild(m_gradA);

        m_gradB = CCLayerGradient::create(
            ccc4(colB.r, colB.g, colB.b, 255), ccc4(dB.r, dB.g, dB.b, 255), ccp(0, -1));
        m_gradB->ignoreAnchorPointForPosition(false);
        m_gradB->setAnchorPoint({ 0.f, 0.f });
        clipper->addChild(m_gradB);

        m_divider = CCLayerColor::create({ 255, 255, 255, 220 });
        m_divider->setContentSize({ 1.5f, m_innerH });
        m_divider->ignoreAnchorPointForPosition(false);
        m_divider->setAnchorPoint({ 0.5f, 0.f });
        this->addChild(m_divider, 2);

        applySplit();
        return true;
    }

    void applySplit() {
        float wa = m_innerW * std::clamp(m_current, 0.f, 1.f);
        float wb = m_innerW - wa;
        m_gradA->setContentSize({ std::max(wa, 0.f), m_innerH });
        m_gradA->setVisible(wa > 1.f);
        m_gradB->setContentSize({ std::max(wb, 0.f), m_innerH });
        m_gradB->setPosition({ m_pad + wa, m_pad });
        m_gradB->setVisible(wb > 1.f);
        m_divider->setPosition({ m_pad + wa, m_pad });
    }

public:
    static WcTugBar* create(float w, float h, ccColor3B colA, ccColor3B colB) {
        auto ret = new WcTugBar();
        if (ret && ret->init(w, h, colA, colB)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    void setSplit(float pa) {
        m_current = std::clamp(pa, 0.f, 1.f);
        applySplit();
    }

    void animateTo(float pa, float dur, std::function<void(float)> onStep) {
        m_from = m_current;
        m_target = std::clamp(pa, 0.f, 1.f);
        m_animDur = std::max(dur, 0.05f);
        m_animT = 0.f;
        m_onStep = onStep;
        this->scheduleUpdate();
    }

    void update(float dt) override {
        if (m_animDur <= 0.f) { this->unscheduleUpdate(); return; }
        m_animT += dt;
        float t = std::min(m_animT / m_animDur, 1.f);
        float eased = 1.f - powf(1.f - t, 3.f); // ease-out cubic
        m_current = m_from + (m_target - m_from) * eased;
        applySplit();
        if (m_onStep) m_onStep(m_current);
        if (t >= 1.f) {
            m_animDur = 0.f;
            this->unscheduleUpdate();
        }
    }
};

// Evento Mundial 2026: marcador de prediccion de partidos (hasta 3 a la vez).
// Cada partido es una tarjeta compacta: banderas + marcador (score, "-" hasta
// que el admin escribe el resultado) y UNA barra dividida donde cada lado se
// llena segun los votos de su equipo (solo porcentajes, sin conteos). El
// jugador vota una vez por partido; al acertar reclama un cofre (contenido
// acreditado via /chest/claim por StreakChestPopup, igual que el pass).
class WcEventPopup : public Popup {
protected:
    static constexpr float POPUP_W = 340.f;
    static constexpr float POPUP_H = 250.f;

    CCNode* m_contentNode = nullptr;
    bool m_busy = false;
    // match_id cuyo resultado se acaba de votar: en el proximo rebuild su
    // barra se anima de 50/50 al valor real en vez de aparecer de golpe.
    std::string m_animateMatchId;

    // Refresco en vivo: referencias a las barras visibles para animarlas
    // cuando llegan conteos nuevos, y una clave del estado estructural para
    // decidir cuando hace falta reconstruir la UI completa.
    struct BarRef {
        std::string matchId;
        WcTugBar* bar;
        CCLabelBMFont* lblA;
        CCLabelBMFont* lblB;
    };
    std::vector<BarRef> m_barRefs;
    std::string m_uiKey;

    static std::string computeUiKey() {
        const auto& wc = g_streakData.wcEvent;
        std::string key = wc.active ? "1" : "0";
        for (const auto& m : wc.matches) {
            key += fmt::format("|{}:{}:{}:{}:{}:{}",
                m.matchId, m.status, m.score, m.winner, m.myVote, m.claimed ? 1 : 0);
        }
        return key;
    }

    static constexpr const char* WC_BADGE_ID = "wc_2026";
    static constexpr int WC_BADGE_GOAL = 4;

    static ccColor3B teamColorA() { return { 90, 170, 255 }; }
    static ccColor3B teamColorB() { return { 255, 110, 110 }; }

    // Color representativo de cada pais (por archivo de bandera): se usa en el
    // nombre, la barra y el porcentaje. Fallback: azul equipo A / rojo equipo B.
    static ccColor3B countryColor(const std::string& sprite, bool isTeamA) {
        static const std::map<std::string, ccColor3B> colors = {
            { "colombia.png",   { 255, 209, 0 } },   // amarillo
            { "argentina.png",  { 117, 170, 219 } }, // celeste
            { "spain.png",      { 220, 40, 50 } },   // rojo
            { "francia.png",    { 60, 100, 200 } },  // azul
            { "inglaterra.png", { 240, 240, 245 } }, // blanco
            { "belgica.png",    { 255, 190, 40 } },  // dorado
            { "marruecos.png",  { 30, 150, 90 } },   // verde
            { "noruega.png",    { 200, 30, 60 } },   // rojo oscuro
            { "suiza.png",      { 230, 50, 40 } },   // rojo
            { "egipto.png",     { 210, 170, 60 } }   // dorado faraonico
        };
        auto it = colors.find(sprite);
        if (it != colors.end()) return it->second;
        return isTeamA ? teamColorA() : teamColorB();
    }

    CCSprite* createFlag(const std::string& file, float maxSize) {
        CCSprite* spr = nullptr;
        if (!file.empty()) {
            auto path = fmt::format("{}/{}", Mod::get()->getID(), file);
            spr = CCSprite::create(path.c_str());
        }
        if (!spr) spr = CCSprite::createWithSpriteFrameName("GJ_unknownBtn_001.png");
        if (spr) {
            float scale = maxSize / std::max({ spr->getContentSize().width, spr->getContentSize().height, 1.f });
            spr->setScale(scale);
        }
        return spr;
    }

    CCSprite* createChestIcon(int rarity, float size) {
        std::string path = fmt::format("{}/ChestStar{}.png", Mod::get()->getID(), std::clamp(rarity, 1, 6));
        auto chest = CCSprite::create(path.c_str());
        if (!chest) chest = CCSprite::createWithSpriteFrameName("chest_02_02_001.png");
        if (chest) {
            float scale = size / std::max(chest->getContentSize().height, 1.f);
            chest->setScale(scale);
        }
        return chest;
    }

    bool init() override {
        if (!Popup::init(POPUP_W, POPUP_H, "GJ_square02.png")) return false;
        this->setTitle("World Cup 2026");

        // Fondo degradado tipo estadio (recortado al marco del popup)
        auto bgSize = m_bgSprite->getContentSize();
        auto stencil = cocos2d::extension::CCScale9Sprite::create("GJ_square02.png");
        stencil->setContentSize(bgSize);
        stencil->setPosition(bgSize / 2);
        auto clipper = CCClippingNode::create(stencil);
        clipper->setAlphaThreshold(0.05f);
        clipper->setPosition({ 0, 0 });
        auto gradient = CCLayerGradient::create(
            ccc4(10, 45, 25, 160), ccc4(15, 25, 70, 160), ccp(0, -1)
        );
        gradient->setContentSize(bgSize);
        clipper->addChild(gradient);
        m_bgSprite->addChild(clipper, 1);

        rebuild();

        // Refresco en vivo: consulta el estado cada 8s mientras el popup este
        // abierto para que las barras reflejen los votos de otros jugadores.
        this->schedule(schedule_selector(WcEventPopup::onPollTick), 8.f);
        return true;
    }

    void onPollTick(float) {
        if (m_busy) return;
        wcEventFetchStateOnServer([this, keepAlive = Ref<CCNode>(this)](bool ok) {
            if (!ok || m_busy) return;
            std::string newKey = computeUiKey();
            if (newKey != m_uiKey) {
                // Cambio estructural (partido nuevo, cierre, resultado...):
                // reconstruir la tarjeta completa.
                this->rebuild();
                return;
            }
            // Solo cambiaron los conteos: deslizar las barras al valor nuevo.
            const auto& matches = g_streakData.wcEvent.matches;
            for (const auto& ref : m_barRefs) {
                for (const auto& m : matches) {
                    if (m.matchId != ref.matchId) continue;
                    int total = m.votesA + m.votesB;
                    float pa = total > 0 ? (float)m.votesA / total : 0.5f;
                    auto lblA = ref.lblA;
                    auto lblB = ref.lblB;
                    ref.bar->animateTo(pa, 0.6f, [lblA, lblB](float cur) {
                        int a = (int)std::round(cur * 100.f);
                        lblA->setString(fmt::format("{}%", a).c_str());
                        lblB->setString(fmt::format("{}%", 100 - a).c_str());
                    });
                    break;
                }
            }
        });
    }

    void rebuild() {
        if (m_contentNode) m_contentNode->removeFromParent();
        m_contentNode = CCNode::create();
        m_mainLayer->addChild(m_contentNode, 2);
        m_barRefs.clear();
        m_uiKey = computeUiKey();

        auto winSize = m_mainLayer->getContentSize();
        const auto& wc = g_streakData.wcEvent;

        if (!wc.active || wc.matches.empty()) {
            auto lbl = CCLabelBMFont::create("No matches right now.\nCome back later!", "bigFont.fnt");
            lbl->setAlignment(kCCTextAlignmentCenter);
            lbl->setScale(0.5f);
            lbl->setPosition({ winSize.width / 2, winSize.height / 2 });
            m_contentNode->addChild(lbl);
            return;
        }

        buildBadgeCorner(winSize);

        int count = (int)wc.matches.size();
        float top = winSize.height - 42.f;
        float bottom = 12.f;
        float pitch = (top - bottom) / count;

        for (int i = 0; i < count; i++) {
            float cy = top - pitch * i - pitch / 2.f;
            buildMatchCard(wc.matches[i], i, cy, winSize);
        }
    }

    // Esquina superior derecha: badge wc_2026 con contador de predicciones
    // acertadas (N/4) y boton de info.
    void buildBadgeCorner(CCSize winSize) {
        float y = winSize.height - 24.f;
        bool unlocked = g_streakData.isBadgeUnlocked(WC_BADGE_ID);
        int shown = std::min(g_streakData.wcEvent.correctPredictions, WC_BADGE_GOAL);

        CCSprite* badgeSpr = nullptr;
        if (auto* info = g_streakData.getBadgeInfo(WC_BADGE_ID)) {
            badgeSpr = CCSprite::create(info->spriteName.c_str());
        }
        if (!badgeSpr) badgeSpr = CCSprite::createWithSpriteFrameName("GJ_unknownBtn_001.png");
        float scale = 24.f / std::max(badgeSpr->getContentSize().height, 1.f);
        badgeSpr->setScale(scale);
        if (!unlocked) badgeSpr->setColor({ 110, 110, 110 });
        badgeSpr->setPosition({ winSize.width - 84.f, y });
        m_contentNode->addChild(badgeSpr, 5);

        auto counterLbl = CCLabelBMFont::create(
            fmt::format("{}/{}", shown, WC_BADGE_GOAL).c_str(), "goldFont.fnt");
        counterLbl->setScale(0.42f);
        counterLbl->setAnchorPoint({ 0.f, 0.5f });
        counterLbl->setPosition({ winSize.width - 68.f, y });
        if (unlocked) counterLbl->setColor({ 120, 255, 140 });
        m_contentNode->addChild(counterLbl, 5);

        auto infoIcon = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
        infoIcon->setScale(0.55f);
        auto infoBtn = CCMenuItemSpriteExtra::create(
            infoIcon, this, menu_selector(WcEventPopup::onBadgeInfo));
        auto infoMenu = CCMenu::createWithItem(infoBtn);
        infoMenu->setPosition({ winSize.width - 24.f, y });
        m_contentNode->addChild(infoMenu, 5);
    }

    void onBadgeInfo(CCObject*) {
        int shown = std::min(g_streakData.wcEvent.correctPredictions, WC_BADGE_GOAL);
        bool unlocked = g_streakData.isBadgeUnlocked(WC_BADGE_ID);
        std::string text = fmt::format(
            "Predict match winners and claim your rewards!\n"
            "Get <cy>{} correct predictions</c> to unlock the exclusive "
            "<cg>Perfect Predictor</c> badge!\n\n"
            "Correct predictions: <cy>{}/{}</c>{}",
            WC_BADGE_GOAL, shown, WC_BADGE_GOAL,
            unlocked ? "\n<cg>Badge unlocked!</c>" : ""
        );
        FLAlertLayer::create("WC 2026 Badge", text, "OK")->show();
    }

    void buildMatchCard(const StreakData::WcMatch& match, int index, float cy, CCSize winSize) {
        const bool voted = !match.myVote.empty();
        const bool finished = (match.status == "finished") && !match.winner.empty();
        const bool revealed = voted || match.status != "open";
        const bool isDraw = finished && match.winner == "draw";

        float cx = winSize.width / 2;
        float flagX = 52.f;
        float rowTop = cy + 18.f;
        float rowBar = cy - 12.f;
        float rowAction = cy - 32.f;

        auto colA = countryColor(match.spriteA, true);
        auto colB = countryColor(match.spriteB, false);

        // Fondo de la tarjeta completa, de bandera a bandera
        auto cardBg = cocos2d::extension::CCScale9Sprite::create("square02b_001.png");
        cardBg->setContentSize({ winSize.width - 40.f, 86.f });
        cardBg->setColor({ 10, 22, 38 });
        cardBg->setOpacity(150);
        cardBg->setPosition({ cx, cy - 2.f });
        m_contentNode->addChild(cardBg, -1);

        // ===== Fila superior: banderas, nombres y marcador =====
        auto flagA = createFlag(match.spriteA, 44.f);
        flagA->setPosition({ flagX, rowTop });
        m_contentNode->addChild(flagA);

        auto flagB = createFlag(match.spriteB, 44.f);
        flagB->setPosition({ winSize.width - flagX, rowTop });
        m_contentNode->addChild(flagB);

        auto nameA = CCLabelBMFont::create(match.teamA.c_str(), "bigFont.fnt");
        nameA->setScale(0.26f);
        nameA->setAnchorPoint({ 0.f, 0.5f });
        nameA->setColor(colA);
        nameA->setPosition({ flagX + 28.f, rowTop });
        m_contentNode->addChild(nameA);

        auto nameB = CCLabelBMFont::create(match.teamB.c_str(), "bigFont.fnt");
        nameB->setScale(0.26f);
        nameB->setAnchorPoint({ 1.f, 0.5f });
        nameB->setColor(colB);
        nameB->setPosition({ winSize.width - flagX - 28.f, rowTop });
        m_contentNode->addChild(nameB);

        // Marcador (score): "-" hasta que el admin escribe el resultado.
        // Panel negro redondeado detras para que luzca como marcador de estadio.
        auto scorePanel = cocos2d::extension::CCScale9Sprite::create("square02b_001.png");
        scorePanel->setContentSize({ 74.f, 34.f });
        scorePanel->setColor({ 0, 125, 145 });
        scorePanel->setOpacity(210);
        scorePanel->setPosition({ cx, rowTop - 1.f });
        m_contentNode->addChild(scorePanel);

        auto scoreLbl = CCLabelBMFont::create(match.score.c_str(), "goldFont.fnt");
        scoreLbl->setScale(0.5f);
        scoreLbl->setPosition({ cx, rowTop + 4.f });
        m_contentNode->addChild(scoreLbl);

        std::string statusText;
        ccColor3B statusColor;
        if (finished) { statusText = "FULL TIME"; statusColor = { 255, 180, 60 }; }
        else if (match.status == "closed") { statusText = "LIVE"; statusColor = { 255, 230, 100 }; }
        else { statusText = "OPEN"; statusColor = { 120, 255, 140 }; }
        auto statusLbl = CCLabelBMFont::create(statusText.c_str(), "bigFont.fnt");
        statusLbl->setScale(0.2f);
        statusLbl->setColor(statusColor);
        statusLbl->setPosition({ cx, rowTop - 9.f });
        m_contentNode->addChild(statusLbl);

        // Ganador: nombre en dorado, bandera perdedora atenuada
        if (finished && !isDraw) {
            auto winnerName = (match.winner == "a") ? nameA : nameB;
            auto loserFlag = (match.winner == "a") ? flagB : flagA;
            winnerName->setColor({ 255, 215, 80 });
            loserFlag->setColor({ 110, 110, 110 });
        }
        else if (isDraw) {
            nameA->setColor({ 255, 215, 80 });
            nameB->setColor({ 255, 215, 80 });
        }

        // ===== Fila media: barra de votos o botones de voto =====
        if (!revealed) {
            // Botones de voto solo-texto, uno debajo de cada equipo.
            auto menu = CCMenu::create();
            menu->setPosition({ 0, 0 });
            m_contentNode->addChild(menu);

            auto voteLblA = CCLabelBMFont::create("VOTE", "goldFont.fnt");
            voteLblA->setScale(0.55f);
            auto voteBtnA = CCMenuItemSpriteExtra::create(
                voteLblA, this, menu_selector(WcEventPopup::onVote));
            voteBtnA->setTag(index * 2);
            voteBtnA->setPosition({ flagX + 12.f, rowBar });
            menu->addChild(voteBtnA);

            auto voteLblB = CCLabelBMFont::create("VOTE", "goldFont.fnt");
            voteLblB->setScale(0.55f);
            auto voteBtnB = CCMenuItemSpriteExtra::create(
                voteLblB, this, menu_selector(WcEventPopup::onVote));
            voteBtnB->setTag(index * 2 + 1);
            voteBtnB->setPosition({ winSize.width - flagX - 12.f, rowBar });
            menu->addChild(voteBtnB);

            // Pulso sutil para invitar al toque
            for (auto* btn : { voteBtnA, voteBtnB }) {
                btn->runAction(CCRepeatForever::create(CCSequence::create(
                    CCEaseSineInOut::create(CCScaleTo::create(0.6f, 1.1f)),
                    CCEaseSineInOut::create(CCScaleTo::create(0.6f, 1.0f)),
                    nullptr
                )));
            }
        }
        else {
            int total = match.votesA + match.votesB;
            float pa = total > 0 ? (float)match.votesA / total : 0.5f;
            int pctA = (int)std::round(pa * 100.f);
            int pctB = total > 0 ? 100 - pctA : 50;

            float barW = 190.f;
            auto bar = WcTugBar::create(barW, 14.f, colA, colB);
            bar->setPosition({ cx, rowBar });
            m_contentNode->addChild(bar);

            auto pctLblA = CCLabelBMFont::create(fmt::format("{}%", pctA).c_str(), "bigFont.fnt");
            pctLblA->setScale(0.28f);
            pctLblA->setColor(colA);
            pctLblA->setAnchorPoint({ 1.f, 0.5f });
            pctLblA->setPosition({ cx - barW / 2.f - 6.f, rowBar });
            m_contentNode->addChild(pctLblA);

            auto pctLblB = CCLabelBMFont::create(fmt::format("{}%", pctB).c_str(), "bigFont.fnt");
            pctLblB->setScale(0.28f);
            pctLblB->setColor(colB);
            pctLblB->setAnchorPoint({ 0.f, 0.5f });
            pctLblB->setPosition({ cx + barW / 2.f + 6.f, rowBar });
            m_contentNode->addChild(pctLblB);

            m_barRefs.push_back({ match.matchId, bar, pctLblA, pctLblB });

            if (match.matchId == m_animateMatchId) {
                // Recien voto: la barra arranca con MI equipo en 0% y se llena
                // hasta el valor real (mi voto garantiza que mi lado nunca es
                // 0, asi que siempre hay movimiento visible), con los
                // porcentajes contando en vivo.
                m_animateMatchId.clear();
                float start = (match.myVote == "a") ? 0.f : 1.f;
                bar->setSplit(start);
                int startA = (int)std::round(start * 100.f);
                pctLblA->setString(fmt::format("{}%", startA).c_str());
                pctLblB->setString(fmt::format("{}%", 100 - startA).c_str());
                bar->animateTo(pa, 1.1f, [pctLblA, pctLblB](float cur) {
                    int a = (int)std::round(cur * 100.f);
                    pctLblA->setString(fmt::format("{}%", a).c_str());
                    pctLblB->setString(fmt::format("{}%", 100 - a).c_str());
                });
            }
            else {
                bar->setSplit(pa);
            }
        }

        // Estrella marcando mi eleccion, como insignia junto a la bandera
        if (voted) {
            auto pick = CCSprite::createWithSpriteFrameName("GJ_starsIcon_001.png");
            if (pick) {
                pick->setScale(0.45f);
                float px = (match.myVote == "a")
                    ? flagX - 22.f
                    : winSize.width - flagX + 22.f;
                pick->setPosition({ px, rowTop + 15.f });
                m_contentNode->addChild(pick, 5);
            }
        }

        // ===== Fila inferior: premio / claim / resultado =====
        if (finished) {
            const bool won = voted && (isDraw || match.myVote == match.winner);
            if (won && !match.claimed) {
                auto menu = CCMenu::create();
                menu->setPosition({ 0, 0 });
                m_contentNode->addChild(menu);

                auto claimBtn = CCMenuItemSpriteExtra::create(
                    ButtonSprite::create("Claim", 0, 0, "goldFont.fnt", "GJ_button_01.png", 0, 0.5f),
                    this, menu_selector(WcEventPopup::onClaim));
                claimBtn->setTag(index);
                claimBtn->setPosition({ cx, rowAction });
                menu->addChild(claimBtn);

                auto chest = createChestIcon(match.chestRarity, 20.f);
                if (chest) {
                    chest->setPosition({ cx - 48.f, rowAction });
                    m_contentNode->addChild(chest);
                }
            }
            else if (won && match.claimed) {
                auto check = CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
                check->setScale(0.4f);
                check->setPosition({ cx - 45.f, rowAction });
                m_contentNode->addChild(check);
                auto lbl = CCLabelBMFont::create("Claimed!", "bigFont.fnt");
                lbl->setScale(0.28f);
                lbl->setColor({ 120, 255, 140 });
                lbl->setPosition({ cx + 5.f, rowAction });
                m_contentNode->addChild(lbl);
            }
            else if (voted) {
                auto lbl = CCLabelBMFont::create("Better luck next match!", "bigFont.fnt");
                lbl->setScale(0.26f);
                lbl->setColor({ 255, 140, 140 });
                lbl->setPosition({ cx, rowAction });
                m_contentNode->addChild(lbl);
            }
            else {
                auto lbl = CCLabelBMFont::create("You didn't vote this match.", "bigFont.fnt");
                lbl->setScale(0.24f);
                lbl->setOpacity(170);
                lbl->setPosition({ cx, rowAction });
                m_contentNode->addChild(lbl);
            }
        }
        else {
            // Premio en juego
            auto lbl = CCLabelBMFont::create("Prize:", "bigFont.fnt");
            lbl->setScale(0.26f);
            lbl->setOpacity(200);
            lbl->setPosition({ cx - 18.f, rowAction });
            m_contentNode->addChild(lbl);
            auto chest = createChestIcon(match.chestRarity, 20.f);
            if (chest) {
                chest->setPosition({ cx + 16.f, rowAction });
                m_contentNode->addChild(chest);
            }
        }
    }

    void onVote(CCObject* sender) {
        if (m_busy) return;
        int tag = static_cast<CCNode*>(sender)->getTag();
        int index = tag / 2;
        bool isA = (tag % 2 == 0);
        const auto& matches = g_streakData.wcEvent.matches;
        if (index < 0 || index >= (int)matches.size()) return;
        const auto& match = matches[index];
        std::string matchId = match.matchId;
        std::string team = isA ? "a" : "b";
        std::string teamName = isA ? match.teamA : match.teamB;

        geode::createQuickPopup(
            "Confirm Vote",
            fmt::format("Vote for <cy>{}</c>?\nYou can only vote <cr>once</c> per match!", teamName),
            "Cancel", "Vote",
            [this, matchId, team, keepAlive = Ref<CCNode>(this)](FLAlertLayer*, bool btn2) {
                if (!btn2 || m_busy) return;
                m_busy = true;
                wcEventVoteOnServer(matchId, team, [this, matchId, keepAlive](bool ok, matjson::Value) {
                    m_busy = false;
                    if (!ok) {
                        FLAlertLayer::create("Error", "Could not send your vote. Try again.", "OK")->show();
                        return;
                    }
                    FMODAudioEngine::sharedEngine()->playEffect("dummyDestroy.ogg");
                    m_animateMatchId = matchId;
                    this->rebuild();
                });
            }
        );
    }

    void onClaim(CCObject* sender) {
        if (m_busy) return;
        int index = static_cast<CCNode*>(sender)->getTag();
        const auto& matches = g_streakData.wcEvent.matches;
        if (index < 0 || index >= (int)matches.size()) return;
        std::string matchId = matches[index].matchId;
        int fallbackRarity = matches[index].chestRarity;

        m_busy = true;
        wcEventClaimOnServer(matchId, [this, fallbackRarity, keepAlive = Ref<CCNode>(this)](bool ok, matjson::Value data) {
            m_busy = false;
            if (!ok) {
                FLAlertLayer::create("Error", "Could not claim the reward. Try again.", "OK")->show();
                return;
            }
            int rarity = data["chest_rarity"].as<int>().unwrapOr(fallbackRarity);
            bool badgeUnlocked = data["badge_unlocked"].as<bool>().unwrapOr(false);
            if (badgeUnlocked) {
                g_streakData.unlockBadge(WC_BADGE_ID);
                BadgeNotification::show(WC_BADGE_ID);
            }
            this->rebuild();

            int stars = 0, tickets = 0, gems = 0, xp = 0;
            StreakChestPopup::rollRewardsForRarity(rarity, stars, tickets, gems, xp);
            if (auto popup = StreakChestPopup::create(stars, tickets, gems, xp, rarity, nullptr)) {
                popup->show();
            }
        });
    }

public:
    static WcEventPopup* create() {
        auto ret = new WcEventPopup();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};
