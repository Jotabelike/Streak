#include <Geode/Geode.hpp>
#include <Geode/modify/GameStatsManager.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/ProfilePage.hpp>
#include <Geode/modify/CommentCell.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/GJAccountManager.hpp>
#include <Geode/binding/GJUserScore.hpp>
#include <Geode/binding/GJComment.hpp>
#include <Geode/loader/Log.hpp>
#include <ctime>
#include <string_view>
#include <vector>
#include <Geode/fmod/fmod.hpp>
#include <Geode/cocos/particle_nodes/CCParticleSystemQuad.h>
#include <Geode/cocos/extensions/cocos-ext.h>
#include <cmath> 
#include <Geode/modify/PauseLayer.hpp>


using namespace geode::prelude;
using namespace std::literals;
using namespace cocos2d;

// ================== SISTEMA DE RACHAS Y RECOMPENSAS ==================
struct StreakData {
    int currentStreak = 0;
    int starsToday = 0;
    bool hasNewStreak = false;
    std::string lastDay = "";
    std::string equippedBadge = ""; 


    // Definir categorías de insignias
    enum class BadgeCategory {
        COMMON,
        SPECIAL,
        EPIC,
        LEGENDARY,
        MYTHIC
    };

    // Sistema flexible de insignias con categorías
    struct BadgeInfo {
        int daysRequired;
        std::string spriteName;
        std::string displayName;
        BadgeCategory category;
        std::string badgeID; // ID único para cada insignia
    };

    std::vector<BadgeInfo> badges = {
        {5, "reward5.png"_spr, "First Steps", BadgeCategory::COMMON, "badge_5"},
        {10, "reward10.png"_spr, "Shall We Continue?", BadgeCategory::COMMON, "badge_10"},
        {30, "reward30.png"_spr, "We're Going Well", BadgeCategory::SPECIAL, "badge_30"},
        {50, "reward50.png"_spr, "Half a Hundred", BadgeCategory::SPECIAL, "badge_50"},
        {70, "reward70.png"_spr, "Progressing", BadgeCategory::EPIC, "badge_70"},
        {100, "reward100.png"_spr, "100 Days!!!", BadgeCategory::LEGENDARY, "badge_100"},
        {150, "reward150.png"_spr, "150 Days!!!", BadgeCategory::LEGENDARY, "badge_150"},
        {300, "reward300.png"_spr, "300 Days!!!", BadgeCategory::LEGENDARY, "badge_300"},
        {365, "reward1year.png"_spr, "1 Year!!!", BadgeCategory::MYTHIC, "badge_365"}
    };

    std::vector<bool> unlockedBadges;

    int getRequiredStars() {
        if (currentStreak >= 81) return 21;
        else if (currentStreak >= 71) return 19;
        else if (currentStreak >= 61) return 17;
        else if (currentStreak >= 51) return 15;
        else if (currentStreak >= 41) return 13;
        else if (currentStreak >= 31) return 11;
        else if (currentStreak >= 21) return 9;
        else if (currentStreak >= 11) return 7;
        else if (currentStreak >= 1) return 5;
        else return 5;
    }

    void load() {
        currentStreak = Mod::get()->getSavedValue<int>("streak", 0);
        starsToday = Mod::get()->getSavedValue<int>("starsToday", 0);
        hasNewStreak = Mod::get()->getSavedValue<bool>("hasNewStreak", false);
        lastDay = Mod::get()->getSavedValue<std::string>("lastDay", "");
        equippedBadge = Mod::get()->getSavedValue<std::string>("equippedBadge", "");

        // Cargar insignias desbloqueadas automáticamente según la configuración
        unlockedBadges.resize(badges.size(), false);
        for (size_t i = 0; i < badges.size(); i++) {
            unlockedBadges[i] = Mod::get()->getSavedValue<bool>(
                badges[i].badgeID,
                false
            );
        }

        dailyUpdate();
        checkRewards(); // Verificar recompensas al cargar
    }

    void save() {
        Mod::get()->setSavedValue<int>("streak", currentStreak);
        Mod::get()->setSavedValue<int>("starsToday", starsToday);
        Mod::get()->setSavedValue<bool>("hasNewStreak", hasNewStreak);
        Mod::get()->setSavedValue<std::string>("lastDay", lastDay);
        Mod::get()->setSavedValue<std::string>("equippedBadge", equippedBadge);

        // Guardar insignias desbloqueadas
        for (size_t i = 0; i < badges.size(); i++) {
            Mod::get()->setSavedValue<bool>(
                badges[i].badgeID,
                unlockedBadges[i]
            );
        }
    }

    std::string getCurrentDate() {
        time_t t = time(nullptr);
        tm* now = localtime(&t);
        char buf[16];
        strftime(buf, sizeof(buf), "%Y-%m-%d", now);
        return std::string(buf);
    }

    void unequipBadge() {
        equippedBadge = "";
        save();
    }

    bool isBadgeEquipped(const std::string& badgeID) {
        return equippedBadge == badgeID;
    }


    void dailyUpdate() {
        std::string today = getCurrentDate();
        if (lastDay != today && !lastDay.empty()) {
            // Guardar el valor ANTES de resetear
            int yesterdayStars = starsToday;
            int requiredStars = getRequiredStars();

            // Solo ahora resetear para el nuevo día
            starsToday = 0;
            lastDay = today;

            // Verificar si no se cumplió el requerimiento del día anterior
            if (yesterdayStars < requiredStars) {
                currentStreak = 0;
                // Opcional: mostrar mensaje de racha perdida
                FLAlertLayer::create("Streak Lost", "You didn't get enough stars yesterday!", "OK")->show();
            }

            save();
        }
        else if (lastDay.empty()) {
            // Primer uso del mod
            lastDay = today;
            starsToday = 0;
            save();
        }
    }

    void checkRewards() {
        for (size_t i = 0; i < badges.size(); i++) {
            if (currentStreak >= badges[i].daysRequired && !unlockedBadges[i]) {
                unlockedBadges[i] = true;
            }
        }
        save();
    }

    void addStars(int count) {
        load();
        dailyUpdate();

        // Obtener los requisitos ANTES de cualquier cambio
        int currentRequired = getRequiredStars();
        bool alreadyGotRacha = (starsToday >= currentRequired);
        starsToday += count;

        if (!alreadyGotRacha && starsToday >= currentRequired) {
            // Guardar el requerimiento actual antes de incrementar
            int oldRequired = currentRequired;

            currentStreak++;
            hasNewStreak = true;

            // Obtener el NUEVO requerimiento después de incrementar la racha
            int newRequired = getRequiredStars();

            // Si los requisitos aumentan (cambio de categoría de racha)
            if (newRequired > oldRequired) {
                // Mantener el progreso del día anterior completado
                starsToday = oldRequired;
            }

            checkRewards();
        }

        save();
    }

    bool shouldShowAnimation() {
        if (hasNewStreak) {
            hasNewStreak = false;
            save();
            return true;
        }
        return false;
    }

    std::string getRachaSprite() {
        int streak = currentStreak;
        if (streak >= 81) return "racha9.png"_spr;
        else if (streak >= 71) return "racha8.png"_spr;
        else if (streak >= 61) return "racha7.png"_spr;
        else if (streak >= 51) return "racha6.png"_spr;
        else if (streak >= 41) return "racha5.png"_spr;
        else if (streak >= 31) return "racha4.png"_spr;
        else if (streak >= 21) return "racha3.png"_spr;
        else if (streak >= 11) return "racha2.png"_spr;
        else if (streak >= 1)  return "racha1.png"_spr;
        else return "racha0.png"_spr;
    }

    // Función para añadir una nueva insignia fácilmente con categoría
    void addBadge(int days, const std::string& sprite, const std::string& name, BadgeCategory category, const std::string& id) {
        badges.push_back({ days, sprite, name, category, id });
        unlockedBadges.push_back(false);
    }

    // Función para obtener el nombre de la categoría
    std::string getCategoryName(BadgeCategory category) {
        switch (category) {
        case BadgeCategory::COMMON: return "Common";
        case BadgeCategory::SPECIAL: return "Special";
        case BadgeCategory::EPIC: return "Epic";
        case BadgeCategory::LEGENDARY: return "Legendary";
        case BadgeCategory::MYTHIC: return "Mythic";
        default: return "Unknown";
        }
    }

    // Función para obtener el color de la categoría
    ccColor3B getCategoryColor(BadgeCategory category) {
        switch (category) {
        case BadgeCategory::COMMON: return ccc3(200, 200, 200); // Gris
        case BadgeCategory::SPECIAL: return ccc3(100, 200, 255); // Azul claro
        case BadgeCategory::EPIC: return ccc3(170, 0, 255); // Púrpura
        case BadgeCategory::LEGENDARY: return ccc3(255, 165, 0); // Naranja
        case BadgeCategory::MYTHIC: return ccc3(255, 50, 50); // Rojo
        default: return ccc3(255, 255, 255);
        }
    }

    // Obtener información de una insignia por su ID
    BadgeInfo* getBadgeInfo(const std::string& badgeID) {
        for (auto& badge : badges) {
            if (badge.badgeID == badgeID) {
                return &badge;
            }
        }
        return nullptr;
    }

    // Verificar si una insignia está desbloqueada
    bool isBadgeUnlocked(const std::string& badgeID) {
        for (size_t i = 0; i < badges.size(); i++) {
            if (badges[i].badgeID == badgeID) {
                return unlockedBadges[i];
            }
        }
        return false;
    }

    // Equipar una insignia
    void equipBadge(const std::string& badgeID) {
        if (isBadgeUnlocked(badgeID)) {
            equippedBadge = badgeID;
            save();
        }
    }

    // Obtener la insignia equipada actualmente
    BadgeInfo* getEquippedBadge() {
        if (equippedBadge.empty()) return nullptr;
        return getBadgeInfo(equippedBadge);
    }
};

StreakData g_streakData;

// ============= HOOK PARA CONTAR ESTRELLAS =============
class $modify(MyGameStatsManager, GameStatsManager) {
    void incrementStat(char const* p0, int p1) {
        if (std::string_view(p0) == "6"sv) {
            g_streakData.addStars(p1);
        }
        GameStatsManager::incrementStat(p0, p1);
    }
};

// ============= POPUP PARA EQUIPAR/DESEQUIPAR INSIGNIAS =============

class EquipBadgePopup : public Popup<std::string> {
protected:
    std::string m_badgeID;
    bool m_isCurrentlyEquipped;

    bool setup(std::string badgeID) override {
        m_badgeID = badgeID;
        auto winSize = m_mainLayer->getContentSize();

        auto badgeInfo = g_streakData.getBadgeInfo(badgeID);
        if (!badgeInfo) return false;

        // Verificar si esta insignia está actualmente equipada
        auto equippedBadge = g_streakData.getEquippedBadge();
        m_isCurrentlyEquipped = (equippedBadge && equippedBadge->badgeID == badgeID);

        this->setTitle(m_isCurrentlyEquipped ? "Badge Equipped" : "Equip Badge");

        // Mostrar la insignia
        auto badgeSprite = CCSprite::create(badgeInfo->spriteName.c_str());
        if (badgeSprite) {
            badgeSprite->setScale(0.3f);
            badgeSprite->setPosition({ winSize.width / 2, winSize.height / 2 + 20 });
            m_mainLayer->addChild(badgeSprite);
        }

        // Mostrar nombre de la insignia
        auto nameLabel = CCLabelBMFont::create(badgeInfo->displayName.c_str(), "goldFont.fnt");
        nameLabel->setScale(0.6f);
        nameLabel->setPosition({ winSize.width / 2, winSize.height / 2 - 20 });
        m_mainLayer->addChild(nameLabel);

        // Mostrar categoría de la insignia
        auto categoryLabel = CCLabelBMFont::create(
            g_streakData.getCategoryName(badgeInfo->category).c_str(),
            "bigFont.fnt"
        );
        categoryLabel->setScale(0.4f);
        categoryLabel->setPosition({ winSize.width / 2, winSize.height / 2 - 40 });
        categoryLabel->setColor(g_streakData.getCategoryColor(badgeInfo->category));
        m_mainLayer->addChild(categoryLabel);

        // Botón principal (Equipar/Desequipar)
        auto mainBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create(m_isCurrentlyEquipped ? "Unequip" : "Equip"),
            this,
            menu_selector(EquipBadgePopup::onToggleEquip)
        );

        auto menu = CCMenu::create();
        menu->addChild(mainBtn);
        menu->setPosition(winSize.width / 2, winSize.height / 2 - 70);
        m_mainLayer->addChild(menu);

        return true;
    }

    void onToggleEquip(CCObject*) {
        if (m_isCurrentlyEquipped) {
            g_streakData.unequipBadge();
            FLAlertLayer::create("Success", "Badge unequipped!", "OK")->show();
        }
        else {
            g_streakData.equipBadge(m_badgeID);
            FLAlertLayer::create("Success", "Badge equipped!", "OK")->show();
        }
        this->onClose(nullptr);
    }

public:
    static EquipBadgePopup* create(std::string badgeID) {
        auto ret = new EquipBadgePopup();
        if (ret && ret->initAnchored(250.f, 200.f, badgeID)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

// ============= POPUP QUE MUESTRA TODAS LAS RACHAS =============
class AllRachasPopup : public Popup<> {
protected:
    bool setup() override {
        this->setTitle("All Streaks");
        auto winSize = m_mainLayer->getContentSize();

        float startX = 50.f;
        float y = winSize.height / 2 + 20.f;
        float spacing = 50.f;

        std::vector<std::tuple<std::string, int, int>> rachas = {
            { "racha1.png"_spr, 1, 5 },
            { "racha2.png"_spr, 11, 7 },
            { "racha3.png"_spr, 21, 9 },
            { "racha4.png"_spr, 31, 11 },
            { "racha5.png"_spr, 41, 13 },
            { "racha6.png"_spr, 51, 15 },
            { "racha7.png"_spr, 61, 17 },
            { "racha8.png"_spr, 71, 19 },
            { "racha9.png"_spr, 81, 21 }
        };

        int i = 0;
        for (auto& [sprite, day, requiredStars] : rachas) {
            auto spr = CCSprite::create(sprite.c_str());
            spr->setScale(0.22f);
            spr->setPosition({ startX + i * spacing, y });
            m_mainLayer->addChild(spr);

            auto label = CCLabelBMFont::create(
                CCString::createWithFormat("Day %d", day)->getCString(),
                "goldFont.fnt"
            );
            label->setScale(0.35f);
            label->setPosition({ startX + i * spacing, y - 40 });
            m_mainLayer->addChild(label);

            auto starIcon = CCSprite::createWithSpriteFrameName("GJ_starsIcon_001.png");
            starIcon->setScale(0.4f);
            starIcon->setPosition({ startX + i * spacing - 4, y - 60 });
            m_mainLayer->addChild(starIcon);

            auto starsLabel = CCLabelBMFont::create(
                CCString::createWithFormat("%d", requiredStars)->getCString(),
                "bigFont.fnt"
            );
            starsLabel->setScale(0.3f);
            starsLabel->setPosition({ startX + i * spacing + 6, y - 60 });
            m_mainLayer->addChild(starsLabel);

            i++;
        }

        return true;
    }

public:
    static AllRachasPopup* create() {
        auto ret = new AllRachasPopup();
        if (ret && ret->initAnchored(500.f, 155.f)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

// ============= POPUP DE PROGRESO MÚLTIPLE =============
class DayProgressPopup : public Popup<> {
protected:
    int m_currentGoalIndex = 0;
    CCLabelBMFont* m_titleLabel = nullptr;
    CCLayerColor* m_barBg = nullptr;
    CCLayerGradient* m_barFg = nullptr;
    CCLayerColor* m_border = nullptr;
    CCLayerColor* m_outer = nullptr;
    CCSprite* m_rewardSprite = nullptr;
    CCLabelBMFont* m_dayText = nullptr;

    bool setup() override {
        auto winSize = m_mainLayer->getContentSize();

        // Botón izquierdo
        auto leftArrow = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        leftArrow->setScale(0.8f);
        auto leftBtn = CCMenuItemSpriteExtra::create(
            leftArrow, this, menu_selector(DayProgressPopup::onPreviousGoal)
        );
        leftBtn->setPosition({ -winSize.width / 2 + 30, 0 });

        // Botón derecho
        auto rightArrow = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        rightArrow->setScale(0.8f);
        rightArrow->setFlipX(true);
        auto rightBtn = CCMenuItemSpriteExtra::create(
            rightArrow, this, menu_selector(DayProgressPopup::onNextGoal)
        );
        rightBtn->setPosition({ winSize.width / 2 - 30, 0 });

        auto arrowMenu = CCMenu::create();
        arrowMenu->addChild(leftBtn);
        arrowMenu->addChild(rightBtn);
        arrowMenu->setPosition({ winSize.width / 2, winSize.height / 2 });
        m_mainLayer->addChild(arrowMenu);

        // Crear elementos de la barra
        m_barBg = CCLayerColor::create(ccc4(45, 45, 45, 255), 180.0f, 16.0f);
        m_barFg = CCLayerGradient::create(ccc4(250, 225, 60, 255), ccc4(255, 165, 0, 255));
        m_border = CCLayerColor::create(ccc4(255, 255, 255, 255), 182.0f, 18.0f);
        m_outer = CCLayerColor::create(ccc4(0, 0, 0, 255), 186.0f, 22.0f);

        m_barBg->setVisible(false);
        m_barFg->setVisible(false);
        m_border->setVisible(false);
        m_outer->setVisible(false);

        m_mainLayer->addChild(m_barBg, 1);
        m_mainLayer->addChild(m_barFg, 2);
        m_mainLayer->addChild(m_border, 4);
        m_mainLayer->addChild(m_outer, 0);

        m_titleLabel = CCLabelBMFont::create("", "goldFont.fnt");
        m_titleLabel->setScale(0.6f);
        m_titleLabel->setPosition({ winSize.width / 2, winSize.height / 2 + 60 });
        m_mainLayer->addChild(m_titleLabel, 5);

        m_dayText = CCLabelBMFont::create("", "goldFont.fnt");
        m_dayText->setScale(0.5f);
        m_dayText->setPosition({ winSize.width / 2, winSize.height / 2 - 35 });
        m_mainLayer->addChild(m_dayText, 5);

        updateDisplay();

        return true;
    }

    void updateDisplay() {
        auto winSize = m_mainLayer->getContentSize();
        g_streakData.load();

        // Usar el sistema automático de insignias
        if (m_currentGoalIndex >= g_streakData.badges.size()) {
            m_currentGoalIndex = 0;
        }

        auto& badge = g_streakData.badges[m_currentGoalIndex];

        int currentDays;
        // Verificamos si la insignia ya fue desbloqueada
        if (g_streakData.isBadgeUnlocked(badge.badgeID)) {
            // Si ya se desbloqueó, siempre mostramos la barra como completa
            currentDays = badge.daysRequired;
        }
        else {
            // Si no, mostramos el progreso actual real
            currentDays = std::min(g_streakData.currentStreak, badge.daysRequired);
        }

        float percent = currentDays / static_cast<float>(badge.daysRequired);

        m_titleLabel->setString(CCString::createWithFormat("Progress to %d Days", badge.daysRequired)->getCString());

        // Barra de progreso
        float barWidth = 180.0f;
        float barHeight = 16.0f;

        m_barBg->setContentSize({ barWidth, barHeight });
        m_barBg->setPosition({ winSize.width / 2 - barWidth / 2, winSize.height / 2 - 10 });
        m_barBg->setVisible(true);

        m_barFg->setContentSize({ barWidth * percent, barHeight });
        m_barFg->setPosition({ winSize.width / 2 - barWidth / 2, winSize.height / 2 - 10 });
        m_barFg->setVisible(true);

        m_border->setContentSize({ barWidth + 2, barHeight + 2 });
        m_border->setPosition({ winSize.width / 2 - barWidth / 2 - 1, winSize.height / 2 - 11 });
        m_border->setVisible(true);
        m_border->setOpacity(120);

        m_outer->setContentSize({ barWidth + 6, barHeight + 6 });
        m_outer->setPosition({ winSize.width / 2 - barWidth / 2 - 3, winSize.height / 2 - 13 });
        m_outer->setVisible(true);
        m_outer->setOpacity(70);

        // Eliminar sprite de recompensa anterior
        if (m_rewardSprite) {
            m_rewardSprite->removeFromParent();
            m_rewardSprite = nullptr;
        }

        // Insignia centrada encima de la barra
        m_rewardSprite = CCSprite::create(badge.spriteName.c_str());
        if (m_rewardSprite) {
            m_rewardSprite->setScale(0.25f);
            m_rewardSprite->setPosition({
                winSize.width / 2,
                winSize.height / 2 + 30
                });
            m_mainLayer->addChild(m_rewardSprite, 5);
        }

        // Texto de días
        m_dayText->setString(
            CCString::createWithFormat("Day %d / %d", currentDays, badge.daysRequired)->getCString()
        );
    }

    void onNextGoal(CCObject*) {
        m_currentGoalIndex = (m_currentGoalIndex + 1) % g_streakData.badges.size();
        updateDisplay();
    }

    void onPreviousGoal(CCObject*) {
        m_currentGoalIndex = (m_currentGoalIndex - 1 + g_streakData.badges.size()) % g_streakData.badges.size();
        updateDisplay();
    }

public:
    static DayProgressPopup* create() {
        auto ret = new DayProgressPopup();
        if (ret && ret->initAnchored(300.f, 180.f)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

// ============= POPUP DE RECOMPENSAS (INSIGNIAS) CON CATEGORÍAS =============
class RewardsPopup : public Popup<> {
protected:
    int m_currentCategory = 0;
    CCLabelBMFont* m_categoryLabel = nullptr;
    CCLayerColor* m_darkPanel = nullptr;
    CCNode* m_badgeContainer = nullptr;
    int m_colorIndex = 0;
    float m_colorTransitionTime = 0.0f;
    std::vector<ccColor3B> m_mythicColors;
    ccColor3B m_currentColor;
    ccColor3B m_targetColor;

    bool setup() override {
        this->setTitle("Awards Collection");
        auto winSize = m_mainLayer->getContentSize();

        g_streakData.load();

        // Inicializar colores para Mythic
        m_mythicColors = {
            ccc3(255, 0, 0),     // Rojo
            ccc3(255, 165, 0),   // Naranja
            ccc3(255, 255, 0),   // Amarillo
            ccc3(0, 255, 0),     // Verde
            ccc3(0, 0, 255),     // Azul
            ccc3(75, 0, 130),    // Índigo
            ccc3(238, 130, 238), // Violeta
            ccc3(255, 105, 180), // Rosa
            ccc3(255, 215, 0),   // Oro
            ccc3(192, 192, 192)  // Plata
        };
        m_colorIndex = 0;
        m_currentColor = m_mythicColors[0];
        m_targetColor = m_mythicColors[1];
        m_colorTransitionTime = 0.0f;

        // Crear panel oscuro para mejor contraste
        m_darkPanel = CCLayerColor::create(ccc4(30, 30, 30, 200), winSize.width - 40, 140);
        m_darkPanel->setPosition({ 20, winSize.height / 2 - 50 });
        m_darkPanel->setZOrder(-1);
        m_mainLayer->addChild(m_darkPanel);

        // Botón izquierdo
        auto leftArrow = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        leftArrow->setScale(0.8f);
        auto leftBtn = CCMenuItemSpriteExtra::create(
            leftArrow, this, menu_selector(RewardsPopup::onPreviousCategory)
        );
        leftBtn->setPosition({ -winSize.width / 2 + 30, 60 });

        // Botón derecho
        auto rightArrow = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        rightArrow->setScale(0.8f);
        rightArrow->setFlipX(true);
        auto rightBtn = CCMenuItemSpriteExtra::create(
            rightArrow, this, menu_selector(RewardsPopup::onNextCategory)
        );
        rightBtn->setPosition({ winSize.width / 2 - 30, 60 });

        auto arrowMenu = CCMenu::create();
        arrowMenu->addChild(leftBtn);
        arrowMenu->addChild(rightBtn);
        arrowMenu->setPosition({ winSize.width / 2, winSize.height / 2 });
        m_mainLayer->addChild(arrowMenu);

        // Etiqueta de categoría
        m_categoryLabel = CCLabelBMFont::create("", "goldFont.fnt");
        m_categoryLabel->setScale(0.6f);
        m_categoryLabel->setPosition({ winSize.width / 2, winSize.height / 2 + 60 });
        m_mainLayer->addChild(m_categoryLabel);

        // Contenedor para insignias
        m_badgeContainer = CCNode::create();
        m_badgeContainer->setPosition(0, 0);
        m_mainLayer->addChild(m_badgeContainer);

        // Mostrar la categoría actual
        updateCategoryDisplay();

        // Programar actualización de colores
        this->schedule(schedule_selector(RewardsPopup::updateColorEffect), 0.016f); // ~60 FPS

        return true;
    }

    void updateColorEffect(float dt) {
        StreakData::BadgeCategory currentCat = static_cast<StreakData::BadgeCategory>(m_currentCategory);

        if (currentCat == StreakData::BadgeCategory::MYTHIC && m_categoryLabel) {
            m_colorTransitionTime += dt;

            // Transición suave entre colores
            float transitionDuration = 1.0f;

            if (m_colorTransitionTime >= transitionDuration) {
                m_colorTransitionTime = 0.0f;
                m_colorIndex = (m_colorIndex + 1) % m_mythicColors.size();
                m_currentColor = m_targetColor;
                m_targetColor = m_mythicColors[(m_colorIndex + 1) % m_mythicColors.size()];
            }

            // Interpolación lineal entre colores
            float progress = m_colorTransitionTime / transitionDuration;
            ccColor3B interpolatedColor = ccc3(
                m_currentColor.r + (m_targetColor.r - m_currentColor.r) * progress,
                m_currentColor.g + (m_targetColor.g - m_currentColor.g) * progress,
                m_currentColor.b + (m_targetColor.b - m_currentColor.b) * progress
            );

            m_categoryLabel->setColor(interpolatedColor);
        }
    }

    void updateCategoryDisplay() {
        // Limpiar contenedor de insignias
        m_badgeContainer->removeAllChildren();

        // Obtener todas las insignias de la categoría actual
        StreakData::BadgeCategory currentCat = static_cast<StreakData::BadgeCategory>(m_currentCategory);
        std::vector<StreakData::BadgeInfo> categoryBadges;

        for (auto& badge : g_streakData.badges) {
            if (badge.category == currentCat) {
                categoryBadges.push_back(badge);
            }
        }

        // Actualizar etiqueta de categoría
        std::string categoryName = g_streakData.getCategoryName(currentCat);
        m_categoryLabel->setString(categoryName.c_str());

        // Detener cualquier animación previa y resetear escala
        m_categoryLabel->stopAllActions();
        m_categoryLabel->setScale(0.6f);

        // Reiniciar transición de colores para Mythic
        if (currentCat == StreakData::BadgeCategory::MYTHIC) {
            m_colorIndex = 0;
            m_colorTransitionTime = 0.0f;
            m_currentColor = m_mythicColors[0];
            m_targetColor = m_mythicColors[1];
            m_categoryLabel->setColor(m_currentColor);
        }
        else {
            // Color normal para otras categorías
            m_categoryLabel->setColor(g_streakData.getCategoryColor(currentCat));
        }

        // Posicionar insignias
        auto winSize = m_mainLayer->getContentSize();
        float startX = winSize.width / 2 - (categoryBadges.size() * 45.f / 2) + 22.5f;
        float y = winSize.height / 2;
        float spacing = 45.f;

        for (size_t i = 0; i < categoryBadges.size(); i++) {
            auto& badge = categoryBadges[i];
            bool unlocked = g_streakData.isBadgeUnlocked(badge.badgeID);
            bool equipped = (g_streakData.getEquippedBadge() && g_streakData.getEquippedBadge()->badgeID == badge.badgeID);

            // Sprite de la insignia
            auto badgeSprite = CCSprite::create(badge.spriteName.c_str());
            if (badgeSprite) {
                badgeSprite->setScale(0.25f);

                if (!unlocked) {
                    badgeSprite->setColor(ccc3(100, 100, 100));
                }
                else if (equipped) {
                    // Resaltar insignia equipada
                    auto glow = CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
                    glow->setScale(1.8f);
                    glow->setPosition({
                        badgeSprite->getContentSize().width / 2,
                        badgeSprite->getContentSize().height / 2 - 100
                        });
                    glow->setColor(g_streakData.getCategoryColor(currentCat));
                    glow->setOpacity(150);
                    badgeSprite->addChild(glow);
                }

                // Hacer la insignia clickeable si está desbloqueada
                if (unlocked) {
                    auto badgeBtn = CCMenuItemSpriteExtra::create(
                        badgeSprite,
                        this,
                        menu_selector(RewardsPopup::onBadgeClick)
                    );
                    badgeBtn->setTag(i); // Guardar índice para identificarla
                    badgeBtn->setUserObject(CCString::create(badge.badgeID));

                    auto badgeMenu = CCMenu::create();
                    badgeMenu->addChild(badgeBtn);
                    badgeMenu->setPosition({ startX + i * spacing, y });
                    m_badgeContainer->addChild(badgeMenu);
                }
                else {
                    badgeSprite->setPosition({ startX + i * spacing, y });
                    m_badgeContainer->addChild(badgeSprite);
                }
            }

            // Texto de días requeridos
            auto daysLabel = CCLabelBMFont::create(
                CCString::createWithFormat("%d days", badge.daysRequired)->getCString(),
                "goldFont.fnt"
            );
            daysLabel->setScale(0.3f);
            daysLabel->setPosition({ startX + i * spacing, y - 35 });

            if (!unlocked) {
                daysLabel->setColor(ccc3(150, 150, 150));
            }
            else {
                daysLabel->setColor(g_streakData.getCategoryColor(currentCat));
            }

            m_badgeContainer->addChild(daysLabel);

            // Icono de candado para no desbloqueadas
            if (!unlocked) {
                auto lockIcon = CCSprite::createWithSpriteFrameName("GJ_lock_001.png");
                lockIcon->setScale(0.4f);
                lockIcon->setPosition({ startX + i * spacing, y });
                m_badgeContainer->addChild(lockIcon, 5);
            }
        }

        // Contador de insignias para esta categoría
        int unlockedCount = 0;
        int totalInCategory = categoryBadges.size();

        for (auto& badge : categoryBadges) {
            if (g_streakData.isBadgeUnlocked(badge.badgeID)) {
                unlockedCount++;
            }
        }

        auto counterText = CCLabelBMFont::create(
            CCString::createWithFormat("Unlocked: %d/%d", unlockedCount, totalInCategory)->getCString(),
            "bigFont.fnt"
        );
        counterText->setScale(0.4f);
        counterText->setPosition({ winSize.width / 2, winSize.height / 2 - 60 });
        m_badgeContainer->addChild(counterText);
    }

    void onBadgeClick(CCObject* sender) {
        auto menuItem = static_cast<CCMenuItemSpriteExtra*>(sender);
        auto badgeID = static_cast<CCString*>(menuItem->getUserObject())->getCString();

        // Mostrar popup para equipar la insignia
        EquipBadgePopup::create(badgeID)->show();
    }

    void onNextCategory(CCObject*) {
        // Reiniciar para la nueva categoría
        m_currentCategory = (m_currentCategory + 1) % 5;
        updateCategoryDisplay();
    }

    void onPreviousCategory(CCObject*) {
        // Reiniciar para la nueva categoría
        m_currentCategory = (m_currentCategory - 1 + 5) % 5;
        updateCategoryDisplay();
    }

public:
    static RewardsPopup* create() {
        auto ret = new RewardsPopup();
        if (ret && ret->initAnchored(300.f, 200.f)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    void onClose(CCObject* sender) override {
        this->unschedule(schedule_selector(RewardsPopup::updateColorEffect));
        Popup::onClose(sender);
    }
};




// ====================================================================
// ================ CLASE PARA PARTÍCULAS MANUALES ====================
// ====================================================================

class ManualParticleEmitter : public cocos2d::CCNode {
protected:
    float m_spawnTimer = 0.0f;
    float m_spawnRate = 0.005f;
    bool m_emitting = true;

public:
    void update(float dt) override {
        if (!m_emitting) {
            if (this->getChildrenCount() == 0) {
                this->unscheduleUpdate();
                return;
            }
            return;
        }

        m_spawnTimer += dt;
        if (m_spawnTimer >= m_spawnRate) {
            m_spawnTimer = 0.0f;
            spawnParticle();
        }
    }

    void spawnParticle() {
        auto particle = CCSprite::create("cuadro.png"_spr);
        if (!particle) return;

        particle->setPosition({ 0, 0 });
        particle->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });

        std::vector<ccColor3B> colors = { {255, 255, 255}, {255, 215, 0}, {255, 165, 0}, {255, 255, 150} };
        particle->setColor(colors[rand() % colors.size()]);
        particle->setOpacity(200);
        particle->setScale(0.05f + (static_cast<float>(rand() % 10) / 100.0f));

        float angle = static_cast<float>(rand() % 360);
        float speed = 30.0f + (rand() % 20);
        float lifespan = 0.3f + (static_cast<float>(rand() % 20) / 100.0f);

        auto move = CCEaseExponentialOut::create(
            CCMoveBy::create(lifespan, {
                speed * cos(CC_DEGREES_TO_RADIANS(angle)),
                speed * sin(CC_DEGREES_TO_RADIANS(angle))
                })
        );

        auto fadeOut = CCFadeOut::create(lifespan);
        auto scaleDown = CCScaleTo::create(lifespan, 0.0f);
        auto rotate = CCRotateBy::create(lifespan, (rand() % 180) - 90);

        particle->runAction(CCSequence::create(
            CCSpawn::create(move, fadeOut, scaleDown, rotate, nullptr),
            CCRemoveSelf::create(),
            nullptr
        ));

        this->addChild(particle);
    }

    void stopEmitting() {
        m_emitting = false;
    }

    static ManualParticleEmitter* create() {
        auto ret = new ManualParticleEmitter();
        if (ret && ret->init()) {
            ret->autorelease();
            ret->scheduleUpdate();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

// Esta función auxiliar no cambia
CCAction* createShakeAction(float duration, float strength) {
    auto shake = CCArray::create();
    int steps = static_cast<int>(duration * 30);
    for (int i = 0; i < steps; ++i) {
        float dx = (rand() % 2 == 0 ? 1 : -1) * (static_cast<float>(rand() % 100) / 100.f) * strength;
        float dy = (rand() % 2 == 0 ? 1 : -1) * (static_cast<float>(rand() % 100) / 100.f) * strength;
        shake->addObject(CCMoveBy::create(duration / steps, ccp(dx, dy)));
    }
    shake->addObject(CCMoveTo::create(0.f, ccp(0, 0)));
    return CCSequence::create(shake);
}

// =========== POPUP PRINCIPAL (VERSIÓN FINAL) ==============
class InfoPopup : public Popup<> {
protected:
    // La función setup no cambia, incluye todo el contenido del popup
    bool setup() override {
        g_streakData.load();
        g_streakData.dailyUpdate();

        this->setTitle("Current Streak");
        auto winSize = m_mainLayer->getContentSize();
        float centerY = winSize.height / 2 + 25;

        auto spriteName = g_streakData.getRachaSprite();
        auto rachaSprite = CCSprite::create(spriteName.c_str());
        rachaSprite->setScale(0.4f);

        auto rachaBtn = CCMenuItemSpriteExtra::create(
            rachaSprite, this, menu_selector(InfoPopup::onRachaClick)
        );

        auto menuRacha = CCMenu::create();
        menuRacha->addChild(rachaBtn);
        menuRacha->setPosition({ winSize.width / 2, centerY });
        m_mainLayer->addChild(menuRacha, 3);

        auto floatUp = CCMoveBy::create(1.5f, { 0, 8 });
        auto floatDown = floatUp->reverse();
        auto seq = CCSequence::create(floatUp, floatDown, nullptr);
        auto repeat = CCRepeatForever::create(seq);
        rachaSprite->runAction(repeat);

        auto streakLabel = CCLabelBMFont::create(
            ("Daily streak: " + std::to_string(g_streakData.currentStreak)).c_str(),
            "goldFont.fnt"
        );
        streakLabel->setScale(0.55f);
        streakLabel->setPosition({ winSize.width / 2, centerY - 60 });
        m_mainLayer->addChild(streakLabel);

        float barWidth = 140.0f;
        float barHeight = 16.0f;
        int requiredStars = g_streakData.getRequiredStars();
        float percent = std::min(g_streakData.starsToday / static_cast<float>(requiredStars), 1.0f);

        auto barBg = CCLayerColor::create(ccc4(45, 45, 45, 255), barWidth, barHeight);
        barBg->setPosition({ winSize.width / 2 - barWidth / 2, centerY - 90 });
        m_mainLayer->addChild(barBg, 1);

        auto barFg = CCLayerGradient::create(ccc4(250, 225, 60, 255), ccc4(255, 165, 0, 255));
        barFg->setContentSize({ barWidth * percent, barHeight });
        barFg->setPosition({ winSize.width / 2 - barWidth / 2, centerY - 90 });
        m_mainLayer->addChild(barFg, 2);

        auto border = CCLayerColor::create(ccc4(255, 255, 255, 255), barWidth + 2, barHeight + 2);
        border->setPosition({ winSize.width / 2 - barWidth / 2 - 1, centerY - 91 });
        border->setZOrder(4);
        border->setOpacity(120);
        m_mainLayer->addChild(border);

        auto outer = CCLayerColor::create(ccc4(0, 0, 0, 255), barWidth + 6, barHeight + 6);
        outer->setPosition({ winSize.width / 2 - barWidth / 2 - 3, centerY - 93 });
        outer->setZOrder(0);
        outer->setOpacity(70);
        m_mainLayer->addChild(outer);

        auto starIcon = CCSprite::createWithSpriteFrameName("GJ_starsIcon_001.png");
        starIcon->setScale(0.45f);
        starIcon->setPosition({ winSize.width / 2 - 25, centerY - 82 });
        m_mainLayer->addChild(starIcon, 5);

        auto barText = CCLabelBMFont::create(
            (std::to_string(g_streakData.starsToday) + " / " + std::to_string(requiredStars)).c_str(),
            "bigFont.fnt"
        );
        barText->setScale(0.45f);
        barText->setPosition({ winSize.width / 2 + 15, centerY - 82 });
        m_mainLayer->addChild(barText, 5);

        std::string indicatorSpriteName = (g_streakData.starsToday >= requiredStars) ? g_streakData.getRachaSprite() : "racha0.png"_spr;
        auto rachaIndicator = CCSprite::create(indicatorSpriteName.c_str());
        rachaIndicator->setScale(0.14f);
        rachaIndicator->setPosition({ winSize.width / 2 + barWidth / 2 + 20, centerY - 82 });
        m_mainLayer->addChild(rachaIndicator, 5);

        auto statsIcon = CCSprite::create("BtnStats.png"_spr);
        if (statsIcon) {
            statsIcon->setScale(0.7f);
            auto statsBtn = CCMenuItemSpriteExtra::create(statsIcon, this, menu_selector(InfoPopup::onOpenStats));
            auto statsMenu = CCMenu::create();
            statsMenu->addChild(statsBtn);
            statsMenu->setPosition({ winSize.width - 22, centerY });
            m_mainLayer->addChild(statsMenu, 10);
        }

        auto rewardsIcon = CCSprite::create("RewardsBtn.png"_spr);
        if (rewardsIcon) {
            rewardsIcon->setScale(0.7f);
            auto rewardsBtn = CCMenuItemSpriteExtra::create(rewardsIcon, this, menu_selector(InfoPopup::onOpenRewards));
            auto rewardsMenu = CCMenu::create();
            rewardsMenu->addChild(rewardsBtn);
            rewardsMenu->setPosition({ winSize.width - 22, centerY - 37 });
            m_mainLayer->addChild(rewardsMenu, 10);
        }

        auto infoIcon = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
        infoIcon->setScale(0.6f);
        auto infoBtn = CCMenuItemSpriteExtra::create(infoIcon, this, menu_selector(InfoPopup::onInfo));
        auto menu = CCMenu::create();
        menu->setPosition({ winSize.width - 20, winSize.height - 20 });
        menu->addChild(infoBtn);
        m_mainLayer->addChild(menu, 10);

        if (g_streakData.shouldShowAnimation()) {
            this->showStreakAnimation(g_streakData.currentStreak);
        }
        return true;
    }

    void onOpenStats(CCObject*) { DayProgressPopup::create()->show(); }
    void onOpenRewards(CCObject*) { RewardsPopup::create()->show(); }
    void onRachaClick(CCObject*) { AllRachasPopup::create()->show(); }

    void onInfo(CCObject*) {
        FLAlertLayer::create(
            "About Streak!",
            "Collect 5+ stars every day to increase your streak!\n"
            "If you miss a day (less than required), your streak resets.\n\n"
            "Icons change depending on how many days you keep your streak.\n"
            "Earn special awards for maintaining your streak!",
            "OK"
        )->show();
    }

	//animacion de racha

    void showStreakAnimation(int streakLevel) {
        auto winSize = CCDirector::sharedDirector()->getWinSize();
        auto animLayer = CCLayer::create();
        animLayer->setZOrder(1000);
        animLayer->setTag(111);
        this->addChild(animLayer);

        auto bg = CCLayerColor::create(ccc4(0, 0, 0, 0));
        bg->runAction(CCFadeTo::create(0.3f, 180));
        animLayer->addChild(bg, 0);

        auto shineBurst = CCSprite::createWithSpriteFrameName("shineBurst_001.png");
        shineBurst->setPosition({ winSize.width / 2, winSize.height + 100.f });
        shineBurst->setScale(0.1f);
        shineBurst->setOpacity(0);
        shineBurst->setTag(6);
        shineBurst->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });
        shineBurst->setColor({ 255, 240, 180 });
        animLayer->addChild(shineBurst, 2);

        auto rachaSprite = CCSprite::create(g_streakData.getRachaSprite().c_str());
        rachaSprite->setPosition({ winSize.width / 2, winSize.height + 100.f });
        rachaSprite->setScale(0.1f);
        rachaSprite->setRotation(-360.f);
        rachaSprite->setTag(1);
        animLayer->addChild(rachaSprite, 3);

        auto aura = CCSprite::createWithSpriteFrameName("GJ_bigStar_001.png");
        aura->setColor({ 255, 200, 100 });
        aura->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });
        aura->setScale(0.0f);
        aura->setOpacity(0);
        aura->setTag(4);
        aura->setPosition(rachaSprite->getPosition());
        animLayer->addChild(aura, 1);

        auto newStreakSprite = CCSprite::create("NewStreak.png"_spr);
        newStreakSprite->setPosition({ winSize.width / 2, winSize.height / 2 + 80.f });
        newStreakSprite->setScale(0.f);
        newStreakSprite->setTag(2);
        animLayer->addChild(newStreakSprite, 4);

        auto daysLabel = CCLabelBMFont::create(CCString::createWithFormat("Day %d", streakLevel)->getCString(), "goldFont.fnt");
        daysLabel->setPosition({ winSize.width / 2, winSize.height / 2 - 80.f });
        daysLabel->setScale(0.5f);
        daysLabel->setOpacity(0);
        daysLabel->setTag(3);
        animLayer->addChild(daysLabel, 5);

        auto trail = ManualParticleEmitter::create();
        trail->setPosition(rachaSprite->getPosition());
        trail->setTag(5);
        animLayer->addChild(trail, 1);

        float entranceDuration = 0.8f;
        auto entranceMove = CCEaseElasticOut::create(CCMoveTo::create(entranceDuration, { winSize.width / 2, winSize.height / 2 }), 0.5f);
        auto entranceScale = CCEaseBackOut::create(CCScaleTo::create(entranceDuration, 1.2f));
        auto entranceRotate = CCEaseExponentialOut::create(CCRotateTo::create(entranceDuration, 0.f));

        auto auraMove = CCMoveTo::create(entranceDuration, { winSize.width / 2, winSize.height / 2 });
        auto trailMove = CCMoveTo::create(entranceDuration, { winSize.width / 2, winSize.height / 2 });
        auto shineMove = CCMoveTo::create(entranceDuration, { winSize.width / 2, winSize.height / 2 });

        aura->runAction(auraMove);

        trail->runAction(CCSequence::create(
            trailMove,
            CCCallFunc::create(trail, callfunc_selector(ManualParticleEmitter::stopEmitting)),
            nullptr
        ));

        shineBurst->runAction(CCSequence::create(
            CCSpawn::create(
                shineMove,
                CCFadeIn::create(entranceDuration * 0.5f),
                CCScaleTo::create(entranceDuration, 6.2f),
                nullptr
            ),
            CCCallFunc::create(this, callfunc_selector(InfoPopup::startShineRotation)),
            nullptr
        ));

        FMODAudioEngine::sharedEngine()->playEffect("achievement.mp3"_spr, 1.0f, 1.0f, 0.6f);

        rachaSprite->runAction(CCSequence::create(
            CCSpawn::create(entranceMove, entranceScale, entranceRotate, nullptr),
            CCCallFunc::create(this, callfunc_selector(InfoPopup::onAnimationImpact)),
            nullptr
        ));
    }

    void startShineRotation() {
        auto animLayer = this->getChildByTag(111);
        if (!animLayer) return;

        if (auto shineBurst = static_cast<CCSprite*>(animLayer->getChildByTag(6))) {
            auto rotate = CCRotateBy::create(8.0f, 360);
            auto repeatRotate = CCRepeatForever::create(rotate);
            shineBurst->runAction(repeatRotate);
            shineBurst->setOpacity(150);
        }
    }

    void onAnimationImpact() {
        auto winSize = CCDirector::sharedDirector()->getWinSize();
        auto animLayer = this->getChildByTag(111);
        if (!animLayer) return;

        animLayer->runAction(createShakeAction(0.2f, 5.0f));

        auto shockwave = CCSprite::createWithSpriteFrameName("d_practiceIcon_001.png");
        shockwave->setPosition({ winSize.width / 2, winSize.height / 2 });
        shockwave->setColor({ 255, 225, 150 });
        shockwave->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });
        shockwave->setScale(0.2f);
        shockwave->runAction(CCSequence::create(
            CCSpawn::create(
                CCScaleTo::create(0.5f, 5.0f),
                CCFadeOut::create(0.5f),
                nullptr
            ),
            CCRemoveSelf::create(),
            nullptr
        ));
        animLayer->addChild(shockwave, 1);

        auto flash = CCSprite::createWithSpriteFrameName("GJ_bigStar_001.png");
        flash->setPosition({ winSize.width / 2, winSize.height / 2 });
        flash->setScale(4.0f);
        flash->setColor({ 255, 255, 150 });
        flash->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });
        flash->runAction(CCSequence::create(CCFadeIn::create(0.1f), CCFadeOut::create(0.3f), CCRemoveSelf::create(), nullptr));
        animLayer->addChild(flash, 3);

        FMODAudioEngine::sharedEngine()->playEffect("mcsfx.mp3"_spr);

        if (auto rachaSprite = static_cast<CCSprite*>(animLayer->getChildByTag(1))) {
            rachaSprite->runAction(CCEaseBackOut::create(CCScaleTo::create(0.3f, 1.0f)));

            auto breatheIn = CCScaleTo::create(1.5f, 1.05f);
            auto breatheOut = CCScaleTo::create(1.5f, 1.0f);
            rachaSprite->runAction(CCRepeatForever::create(CCSequence::create(breatheIn, breatheOut, nullptr)));
        }

        if (auto aura = static_cast<CCSprite*>(animLayer->getChildByTag(4))) {
            aura->runAction(CCSequence::create(
                CCSpawn::create(
                    CCFadeTo::create(0.4f, 150),
                    CCScaleTo::create(0.4f, 1.5f),
                    nullptr
                ),
                CCFadeOut::create(0.3f),
                nullptr
            ));
        }

        if (auto newStreakSprite = static_cast<CCSprite*>(animLayer->getChildByTag(2))) {
            newStreakSprite->runAction(CCSequence::create(CCDelayTime::create(0.2f), CCEaseBackOut::create(CCScaleTo::create(0.4f, 1.0f)), nullptr));
        }
        if (auto daysLabel = static_cast<CCLabelBMFont*>(animLayer->getChildByTag(3))) {
            daysLabel->runAction(CCSequence::create(CCDelayTime::create(0.4f), CCSpawn::create(CCFadeIn::create(0.5f), CCEaseBackOut::create(CCScaleTo::create(0.5f, 1.0f)), nullptr), nullptr));
        }

        animLayer->runAction(CCSequence::create(
            CCDelayTime::create(1.0f),
            CCCallFunc::create(this, callfunc_selector(InfoPopup::spawnStarBurst)),
            CCDelayTime::create(0.5f),
            CCCallFunc::create(this, callfunc_selector(InfoPopup::spawnStarBurst)),
            CCDelayTime::create(0.5f),
            CCCallFunc::create(this, callfunc_selector(InfoPopup::spawnStarBurst)),
			// duracion de la animacion 6.5 segundos aprox
            CCDelayTime::create(2.6f),
            CCCallFunc::create(this, callfunc_selector(InfoPopup::onAnimationExit)),
            nullptr
        ));
    }

    void spawnStarBurst() {
        auto winSize = CCDirector::sharedDirector()->getWinSize();
        auto animLayer = static_cast<CCLayer*>(this->getChildByTag(111));
        if (!animLayer) return;

        std::vector<ccColor3B> colors = {
            {255, 255, 255}, {255, 215, 0}, {255, 165, 0}, {255, 255, 150}
        };

        int numStars = 25 + (rand() % 10);
        float burstStrength = 80.f + (rand() % 50);
        float burstDuration = 1.0f + (rand() % 5 / 10.f);

        for (int i = 0; i < numStars; ++i) {
            auto particle = CCSprite::create("cuadro.png"_spr);
            particle->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });
            particle->setColor(colors[rand() % colors.size()]);

            particle->setPosition({ winSize.width / 2, winSize.height / 2 });
            particle->setScale(0.1f + (rand() % 3 / 10.f));
            particle->setRotation(rand() % 90);

            float angle = (static_cast<float>(i) / numStars) * 360.f + (rand() % 20 - 10);
            float distance = burstStrength * (0.8f + (rand() % 5 / 10.f));
            CCPoint dest = ccp(winSize.width / 2 + distance * cos(CC_DEGREES_TO_RADIANS(angle)), winSize.height / 2 + distance * sin(CC_DEGREES_TO_RADIANS(angle)));

            particle->runAction(CCSequence::create(
                CCSpawn::create(
                    CCEaseExponentialOut::create(CCMoveTo::create(burstDuration, dest)),
                    CCFadeOut::create(burstDuration),
                    CCScaleTo::create(burstDuration, 0.0f),
                    CCRotateBy::create(burstDuration, (rand() % 180) * (rand() % 2 == 0 ? 1 : -1)),
                    nullptr
                ),
                CCRemoveSelf::create(),
                nullptr
            ));
            animLayer->addChild(particle, 1);
        }
    }

    void onAnimationExit() {
        auto animLayer = this->getChildByTag(111);
        if (!animLayer) return;

        if (auto bg = animLayer->getChildByTag(0)) {
            bg->runAction(CCFadeOut::create(1.0f));
        }

        if (auto rachaSprite = static_cast<CCSprite*>(animLayer->getChildByTag(1))) {
            rachaSprite->stopAllActions();
          
            rachaSprite->runAction(CCSpawn::create(
                CCScaleTo::create(0.8f, 0.0f),
                CCFadeOut::create(0.8f),
                nullptr)
            );
        }

        if (auto newStreakSprite = static_cast<CCSprite*>(animLayer->getChildByTag(2))) {
            newStreakSprite->runAction(CCFadeOut::create(0.5f));
        }
        if (auto daysLabel = static_cast<CCLabelBMFont*>(animLayer->getChildByTag(3))) {
            daysLabel->runAction(CCFadeOut::create(0.5f));
        }
        if (auto aura = static_cast<CCSprite*>(animLayer->getChildByTag(4))) {
            aura->runAction(CCFadeOut::create(0.5f));
        }
        if (auto trail = static_cast<ManualParticleEmitter*>(animLayer->getChildByTag(5))) {
            trail->stopEmitting();
        }
        if (auto shineBurst = static_cast<CCSprite*>(animLayer->getChildByTag(6))) {
            shineBurst->stopAllActions();
            shineBurst->runAction(CCFadeOut::create(0.5f));
        }

        animLayer->runAction(CCSequence::create(
            CCDelayTime::create(1.0f),
            CCRemoveSelf::create(),
            nullptr
        ));
    }

public:
    static InfoPopup* create() {
        auto ret = new InfoPopup();
        if (ret && ret->initAnchored(260.f, 220.f)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};



// ============ BOTÓN EN MENÚ PRINCIPAL ==================
class $modify(MyMenuLayer, MenuLayer) {
    struct Fields {
        CCSprite* m_alertSprite = nullptr;
    };

    bool init() {
        if (!MenuLayer::init()) return false;

        g_streakData.load();
        g_streakData.dailyUpdate();

        auto menu = this->getChildByID("bottom-menu");
        if (!menu) return false;

        auto spriteName = g_streakData.getRachaSprite();
        auto icon = CCSprite::create(spriteName.c_str());
        icon->setScale(0.5f);

        auto circle = CircleButtonSprite::create(
            icon, CircleBaseColor::Green, CircleBaseSize::Medium
        );

        int requiredStars = g_streakData.getRequiredStars();
        bool streakInactive = (g_streakData.starsToday < requiredStars);

        m_fields->m_alertSprite = CCSprite::createWithSpriteFrameName("exMark_001.png");
        m_fields->m_alertSprite->setScale(0.4f);
        m_fields->m_alertSprite->setPosition({ circle->getContentSize().width - 12, circle->getContentSize().height - 12 });
        m_fields->m_alertSprite->setZOrder(10);
        m_fields->m_alertSprite->setVisible(streakInactive);
        circle->addChild(m_fields->m_alertSprite);

        if (streakInactive) {
            auto blink = CCBlink::create(2.0f, 3);
            auto repeat = CCRepeatForever::create(blink);
            m_fields->m_alertSprite->runAction(repeat);
        }

        auto btn = CCMenuItemSpriteExtra::create(
            circle, this, menu_selector(MyMenuLayer::onOpenPopup)
        );
        btn->setPositionY(btn->getPositionY() + 5);
        menu->addChild(btn);
        menu->updateLayout();

        return true;
    }

    void onOpenPopup(CCObject*) {
        InfoPopup::create()->show();
    }
};

// ============= MODIFICACIONES PARA MOSTRAR INSIGNIAS EN PERFIL =============

class $modify(MyProfilePage, ProfilePage) {
    struct Fields {
        CCMenuItemSpriteExtra* badgeButton = nullptr;
        StreakData::BadgeInfo* equippedBadgeInfo = nullptr;
    };

    void onBadgeInfoClick(CCObject*) {
        if (m_fields->equippedBadgeInfo) {
            auto badge = m_fields->equippedBadgeInfo;
            std::string title = badge->displayName;
            std::string category = g_streakData.getCategoryName(badge->category);

            // Usamos fmt::format para dar color al texto del mensaje
            std::string message = fmt::format(
                "<cy>{}</c>\n\n<cg>Unlocked at {} days</c>",
                category,
                badge->daysRequired
            );

            FLAlertLayer::create(title.c_str(), message, "OK")->show();
        }
    }

    void loadPageFromUserInfo(GJUserScore * a2) {
        ProfilePage::loadPageFromUserInfo(a2);

        // Solo mostrar en el perfil propio
        if (a2->m_accountID == GJAccountManager::get()->m_accountID) {
            auto layer = m_mainLayer;
            CCMenu* username_menu = static_cast<CCMenu*>(layer->getChildByIDRecursive("username-menu"));

            if (username_menu) {
                // Eliminar insignia anterior si existe
                if (m_fields->badgeButton) {
                    m_fields->badgeButton->removeFromParent();
                    m_fields->badgeButton = nullptr;
                    m_fields->equippedBadgeInfo = nullptr;
                }

                // Obtener insignia equipada
                auto equippedBadge = g_streakData.getEquippedBadge();
                if (equippedBadge) {
                    // Guardar la información de la insignia para el popup
                    m_fields->equippedBadgeInfo = equippedBadge;

                    // Crear sprite de la insignia
                    auto badgeSprite = CCSprite::create(equippedBadge->spriteName.c_str());
                    if (badgeSprite) {
                        badgeSprite->setScale(0.2f);

                        // Crear el botón que llamará a onBadgeInfoClick
                        m_fields->badgeButton = CCMenuItemSpriteExtra::create(
                            badgeSprite,
                            this,
                            menu_selector(MyProfilePage::onBadgeInfoClick)
                        );
                        m_fields->badgeButton->setID("streak-badge");

                        // Posicionar la insignia al lado del nombre de usuario
                        username_menu->addChild(m_fields->badgeButton);
                        username_menu->updateLayout();
                    }
                }
            }
        }
    }
};

class $modify(MyCommentCell, CommentCell) {
    struct Fields {
        CCMenuItemSpriteExtra* badgeButton = nullptr;
        StreakData::BadgeInfo* equippedBadgeInfo = nullptr;
    };

    void onBadgeInfoClick(CCObject*) {
        if (m_fields->equippedBadgeInfo) {
            auto badge = m_fields->equippedBadgeInfo;
            std::string title = badge->displayName;
            std::string category = g_streakData.getCategoryName(badge->category);

            // Usamos fmt::format para dar color al texto del mensaje
            std::string message = fmt::format(
                "<cy>{}</c>\n\n<cg>Unlocked at {} days</c>",
                category,
                badge->daysRequired
            );

            FLAlertLayer::create(title.c_str(), message, "OK")->show();
        }
    }

    void loadFromComment(GJComment * p0) {
        CommentCell::loadFromComment(p0);

        // Solo mostrar en comentarios del usuario actual
        if (p0->m_accountID == GJAccountManager::get()->m_accountID) {
            auto layer = m_mainLayer;
            CCMenu* username_menu = static_cast<CCMenu*>(layer->getChildByIDRecursive("username-menu"));

            if (username_menu) {
                // Eliminar insignia anterior si existe
                if (m_fields->badgeButton) {
                    m_fields->badgeButton->removeFromParent();
                    m_fields->badgeButton = nullptr;
                    m_fields->equippedBadgeInfo = nullptr;
                }

                // Obtener insignia equipada
                auto equippedBadge = g_streakData.getEquippedBadge();
                if (equippedBadge) {
                    // Guardar la información de la insignia para el popup
                    m_fields->equippedBadgeInfo = equippedBadge;

                    // Crear sprite de la insignia
                    auto badgeSprite = CCSprite::create(equippedBadge->spriteName.c_str());
                    if (badgeSprite) {
                        badgeSprite->setScale(0.15f);

                        // Crear el botón que llamará a onBadgeInfoClick
                        m_fields->badgeButton = CCMenuItemSpriteExtra::create(
                            badgeSprite,
                            this,
                            menu_selector(MyCommentCell::onBadgeInfoClick)
                        );
                        m_fields->badgeButton->setID("streak-badge");

                        // Posicionar la insignia al lado del nombre de usuario
                        username_menu->addChild(m_fields->badgeButton);
                        username_menu->updateLayout();
                    }
                }
            }
        }
    }
};





class StreakProgressBar : public cocos2d::CCLayerColor {
protected:
    int m_starsGained;
    int m_starsBefore;
    int m_starsRequired;
    cocos2d::CCLabelBMFont* m_starLabel;
    cocos2d::CCNode* m_barContainer;
    cocos2d::CCLayer* m_barFg;

    float m_barWidth;
    float m_barHeight;
    float m_currentPercent;
    float m_targetPercent;
    float m_currentStarsDisplay;
    float m_targetStarsDisplay;

    bool init(int starsGained, int starsBefore, int starsRequired) {
        if (!CCLayerColor::initWithColor({ 0, 0, 0, 0 })) return false;

        m_starsGained = starsGained;
        m_starsBefore = starsBefore;
        m_starsRequired = starsRequired;

        m_currentPercent = std::min(1.f, static_cast<float>(m_starsBefore) / m_starsRequired);
        m_targetPercent = m_currentPercent;
        m_currentStarsDisplay = static_cast<float>(m_starsBefore);
        m_targetStarsDisplay = m_currentStarsDisplay;

        auto visibleSize = cocos2d::CCDirector::sharedDirector()->getVisibleSize();
        auto origin = cocos2d::CCDirector::sharedDirector()->getVisibleOrigin();

        m_barWidth = 160.0f;
        m_barHeight = 15.0f;

        m_barContainer = CCNode::create();
        m_barContainer->setPosition(origin + CCPoint(80, visibleSize.height - 180));
        this->addChild(m_barContainer);

        auto streakIcon = CCSprite::create(g_streakData.getRachaSprite().c_str());
        streakIcon->setScale(0.2f);
        streakIcon->setRotation(-15.f);
        streakIcon->setPosition({ -5, (m_barHeight + 6) / 2 });
        m_barContainer->addChild(streakIcon, 10);

        auto barBg = cocos2d::extension::CCScale9Sprite::create("GJ_button_01.png");
        barBg->setContentSize({ m_barWidth + 6, m_barHeight + 6 });
        barBg->setColor({ 0, 0, 0 });
        barBg->setOpacity(120);
        barBg->setAnchorPoint({ 0, 0 });
        barBg->setPosition({ 0, 0 });
        m_barContainer->addChild(barBg);

        auto stencil = cocos2d::extension::CCScale9Sprite::create("GJ_button_01.png");
        stencil->setContentSize({ m_barWidth, m_barHeight });
        stencil->setAnchorPoint({ 0, 0 });
        stencil->setPosition({ 3, 3 });

        auto clipper = CCClippingNode::create();
        clipper->setStencil(stencil);
        barBg->addChild(clipper);

        m_barFg = CCLayerGradient::create({ 255, 225, 60, 255 }, { 255, 165, 0, 255 });
        m_barFg->setContentSize({ m_barWidth * m_currentPercent, m_barHeight });
        m_barFg->setAnchorPoint({ 0, 0 });
        m_barFg->setPosition({ 0, 0 });
        clipper->addChild(m_barFg);

        m_starLabel = CCLabelBMFont::create(
            CCString::createWithFormat("%d/%d", m_starsBefore, m_starsRequired)->getCString(), "bigFont.fnt"
        );
        m_starLabel->setAnchorPoint({ 1, 0.5f });
        m_starLabel->setScale(0.4f);
        m_starLabel->setPosition({ m_barWidth - 5, m_barHeight / 2 });
        m_barFg->addChild(m_starLabel, 5);

        this->runAnimations();
        this->scheduleUpdate();

        return true;
    }

    void update(float dt) override {
        float smoothingFactor = 8.0f;
        m_currentPercent = m_currentPercent + (m_targetPercent - m_currentPercent) * dt * smoothingFactor;
        m_currentStarsDisplay = m_currentStarsDisplay + (m_targetStarsDisplay - m_currentStarsDisplay) * dt * smoothingFactor;

        m_barFg->setContentSize({ m_barWidth * m_currentPercent, m_barHeight });
        m_starLabel->setString(CCString::createWithFormat("%d/%d", static_cast<int>(round(m_currentStarsDisplay)), m_starsRequired)->getCString());
        m_starLabel->setPosition({ m_barFg->getContentSize().width - 5, m_barHeight / 2 });
    }

    void runAnimations() {
        CCPoint onScreenPos = m_barContainer->getPosition();
        CCPoint offScreenPos = m_barContainer->getPosition() + CCPoint(-250, 0);

        m_barContainer->setPosition(offScreenPos);
        m_barContainer->runAction(CCSequence::create(
            CCEaseSineOut::create(CCMoveTo::create(0.4f, onScreenPos)),
            CCCallFunc::create(this, callfunc_selector(StreakProgressBar::spawnStarParticles)),
            CCDelayTime::create(2.5f + m_starsGained * 0.15f),
            CCEaseSineIn::create(CCMoveTo::create(0.4f, offScreenPos)),
            CCCallFunc::create(this, callfunc_selector(StreakProgressBar::stopUpdateLoop)),
            CCRemoveSelf::create(),
            nullptr
        ));
    }

    void stopUpdateLoop() {
        this->unscheduleUpdate();
    }

    void spawnStarParticles() {
        auto winSize = CCDirector::sharedDirector()->getWinSize();
        CCPoint center = winSize / 2;
        float delayPerStar = 0.1f;

        for (int i = 0; i < m_starsGained; ++i) {
            auto starParticle = CCSprite::createWithSpriteFrameName("GJ_starsIcon_001.png");
            starParticle->setScale(0.6f);
            starParticle->setPosition(center);
            this->addChild(starParticle, 10);

            CCPoint endPos = m_barContainer->getPosition() + CCPoint(3 + m_barWidth * (std::min(1.f, (float)(m_starsBefore + i + 1) / m_starsRequired)), 3 + m_barHeight / 2);

            ccBezierConfig bezier;
            bezier.endPosition = this->convertToNodeSpace(endPos);
            float explosionRadius = 150.f;
            float randomAngle = (float)(rand() % 360);
            bezier.controlPoint_1 = this->convertToNodeSpace(center + CCPoint((explosionRadius + (rand() % 50)) * cos(CC_DEGREES_TO_RADIANS(randomAngle)), (explosionRadius + (rand() % 50)) * sin(CC_DEGREES_TO_RADIANS(randomAngle))));
            bezier.controlPoint_2 = this->convertToNodeSpace(endPos) + CCPoint(0, 100);

            auto bezierAction = CCBezierTo::create(1.0f, bezier);
            auto rotateAction = CCRotateBy::create(1.0f, 360 + (rand() % 180));
            auto scaleAction = CCScaleTo::create(1.0f, 0.4f);

            starParticle->runAction(CCSequence::create(
                CCDelayTime::create(i * delayPerStar),
                CCSpawn::create(CCEaseSineOut::create(bezierAction), rotateAction, scaleAction, nullptr),
                CCCallFuncND::create(this, callfuncND_selector(StreakProgressBar::onStarHitBar), (void*)(size_t)(i + 1)),
                CCRemoveSelf::create(),
                nullptr
            ));
        }
    }

    void onStarHitBar(CCNode* sender, void* data) {
        int starIndex = (int)(size_t)(data);
        int currentTotalStars = m_starsBefore + starIndex;

        m_targetPercent = std::min(1.f, static_cast<float>(currentTotalStars) / m_starsRequired);
        m_targetStarsDisplay = static_cast<float>(currentTotalStars);

        auto popUp = CCEaseSineOut::create(CCScaleTo::create(0.1f, 1.0f, 1.2f));
        auto popDown = CCEaseSineIn::create(CCScaleTo::create(0.1f, 1.0f, 1.0f));
        m_barFg->runAction(CCSequence::create(popUp, popDown, nullptr));

        auto flash = CCSprite::createWithSpriteFrameName("GJ_bigStar_001.png");
        flash->setPosition(m_barContainer->convertToNodeSpace(sender->getPosition()));
        flash->setScale(0.1f);
        flash->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });
        flash->runAction(CCSequence::create(CCSpawn::create(CCScaleTo::create(0.3f, 1.0f), CCFadeOut::create(0.3f), nullptr), CCRemoveSelf::create(), nullptr));
        m_barContainer->addChild(flash, 20);

        FMODAudioEngine::sharedEngine()->playEffect("collect_1.mp3"_spr);
    }

public:
    static StreakProgressBar* create(int starsGained, int starsBefore, int starsRequired) {
        auto ret = new StreakProgressBar();
        if (ret && ret->init(starsGained, starsBefore, starsRequired)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

class $modify(MyPlayLayer, PlayLayer) {
    void levelComplete() {
        PlayLayer::levelComplete();
        int starsGained = this->m_level->m_stars;

        if (starsGained > 0) {
            g_streakData.load();

            // EL TRUCO ESTÁ AQUÍ:
            // Leemos el total de estrellas actual (que ya fue incrementado por tu GameStatsManager)
            int starsNow = g_streakData.starsToday;
            // Y le restamos las estrellas que acabamos de ganar para saber cuántas había ANTES
            int starsBefore = starsNow - starsGained;

            int starsRequired = g_streakData.getRequiredStars();

            auto progressBar = StreakProgressBar::create(starsGained, starsBefore, starsRequired);
            CCDirector::sharedDirector()->getRunningScene()->addChild(progressBar, 100);
        }
    }
};



// racha en el menu de pausa

class $modify(MyPauseLayer, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();

        // Leemos si la opción está activada
        if (Mod::get()->getSettingValue<bool>("show-in-pause")) {
            auto winSize = cocos2d::CCDirector::sharedDirector()->getWinSize();

            // Leemos las posiciones X e Y desde los ajustes
            float posX = Mod::get()->getSettingValue<double>("pause-pos-x");
            float posY = Mod::get()->getSettingValue<double>("pause-pos-y");

            g_streakData.load();
            int starsToday = g_streakData.starsToday;
            int requiredStars = g_streakData.getRequiredStars();
            int streakDays = g_streakData.currentStreak;

            auto streakNode = CCNode::create();

            // 1. Icono de racha
            auto streakIcon = CCSprite::create(g_streakData.getRachaSprite().c_str());
            streakIcon->setScale(0.2f);
            streakNode->addChild(streakIcon);

            // 2. Texto con el número de días, debajo del icono
            auto daysLabel = CCLabelBMFont::create(
                CCString::createWithFormat("Day %d", streakDays)->getCString(),
                "goldFont.fnt"
            );
            daysLabel->setScale(0.35f);
            daysLabel->setPosition({ 0, -22 });
            streakNode->addChild(daysLabel);

            // 3. Contenedor para el contador de estrellas
            auto starCounterNode = CCNode::create();
            starCounterNode->setPosition({ 0, -37 }); // Debajo del texto de días
            streakNode->addChild(starCounterNode);

            // 3a. Texto del contador
            auto starLabel = CCLabelBMFont::create(
                CCString::createWithFormat("%d / %d", starsToday, requiredStars)->getCString(),
                "bigFont.fnt"
            );
            starLabel->setScale(0.35f);
            starCounterNode->addChild(starLabel);

            // 3b. Icono de estrella
            auto starIcon = CCSprite::createWithSpriteFrameName("GJ_starsIcon_001.png");
            starIcon->setScale(0.5f);
            starCounterNode->addChild(starIcon);

            // Alineamos el contador y el icono para que estén centrados
            starCounterNode->setContentSize({ starLabel->getScaledContentSize().width + starIcon->getScaledContentSize().width + 5, starLabel->getScaledContentSize().height });
            starLabel->setPosition({ -starIcon->getScaledContentSize().width / 2, 0 });
            starIcon->setPosition({ starLabel->getScaledContentSize().width / 2 + 5, 0 });

            // Posicionamos el nodo completo usando los valores de los ajustes
            streakNode->setPosition({ winSize.width * posX, winSize.height * posY });
            this->addChild(streakNode);
        }
    }
};