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
#include <Geode/ui/ListView.hpp>
#include <functional> 
#include <map> 
#include <matjson.hpp> 
#include <Geode/ui/ScrollLayer.hpp>
#include <iomanip>
#include <sstream>
using namespace geode::prelude;

// ================== SISTEMA DE RACHAS Y RECOMPENSAS ==================
struct StreakData {
    int currentStreak = 0;
    int starsToday = 0;
    bool hasNewStreak = false;
    std::string lastDay = "";
    std::string equippedBadge = ""; 
    int superStars = 0; // star
    int lastRouletteIndex = 0;
    int totalSpins = 0;
    int starTickets = 0;
    std::vector<int> streakCompletedLevels; // Asegúrate de que esta línea exista
    std::map<std::string, int> starHistory;


    // --- NUEVAS VARIABLES PARA MISIONES DE ESTRELLAS ---
    bool starMission1Claimed = false;
    bool starMission2Claimed = false;
    bool starMission3Claimed = false;


    // Definir categorías de insignias
    enum class BadgeCategory {
        COMMON,
        SPECIAL,
        EPIC,
        LEGENDARY,
        MYTHIC
    };


    enum class LegalityStatus {
        Legal,      // Verde
        Suspicious, // Naranja
        Cheated     // Rojo
    };
    LegalityStatus legalityStatus = LegalityStatus::Legal;

    // Sistema flexible de insignias con categorías
    struct BadgeInfo {
        int daysRequired;
        std::string spriteName;
        std::string displayName;
        BadgeCategory category;
        std::string badgeID; // ID único para cada insignia
        bool isFromRoulette; // <-- NUEVO CAMPO
    };

    // Reemplaza tu vector 'badges' con este
    std::vector<BadgeInfo> badges = {
        // --- Insignias de Racha (las que ya tenías) ---
        {5, "reward5.png"_spr, "First Steps", BadgeCategory::COMMON, "badge_5", false},
        {10, "reward10.png"_spr, "Shall We Continue?", BadgeCategory::COMMON, "badge_10", false},
        {30, "reward30.png"_spr, "We're Going Well", BadgeCategory::SPECIAL, "badge_30", false},
        {50, "reward50.png"_spr, "Half a Hundred", BadgeCategory::SPECIAL, "badge_50", false},
        {70, "reward70.png"_spr, "Progressing", BadgeCategory::EPIC, "badge_70", false},
        {100, "reward100.png"_spr, "100 Days!!!", BadgeCategory::LEGENDARY, "badge_100", false},
        {150, "reward150.png"_spr, "150 Days!!!", BadgeCategory::LEGENDARY, "badge_150", false},
        {300, "reward300.png"_spr, "300 Days!!!", BadgeCategory::LEGENDARY, "badge_300", false},
        {365, "reward1year.png"_spr, "1 Year!!!", BadgeCategory::MYTHIC, "badge_365", false},

        // --- NUEVO: Insignias Exclusivas de la Ruleta ---
        {0, "badge_beta.png"_spr, "Player beta?",   BadgeCategory::COMMON,   "beta_badge",  true},
       
        
        {0, "badge_platino.png"_spr, "platino badge",  BadgeCategory::COMMON,   "platino_streak_badge",  true},
     
        {0, "badge_diamante_gd.png"_spr, "GD Diamond!",BadgeCategory::COMMON,   "diamante_gd_badge",  true},
        {0, "badge_ncs.png"_spr, "Ncs Lover",    BadgeCategory::SPECIAL,  "ncs_badge",  true},
        {0, "dark_badge1.png"_spr, "dark side",   BadgeCategory::EPIC,     "dark_streak_badge", true},
        {0, "badge_diamante_mc.png"_spr, "Minecraft Diamond!",   BadgeCategory::EPIC,     "diamante_mc_badge", true},
        {0, "gold_streak.png"_spr, "Gold Legend's", BadgeCategory::LEGENDARY,"gold_streak_badge", true},
        {0, "super_star.png"_spr, "First Mythic",  BadgeCategory::MYTHIC,   "super_star_badge", true}
    };

	//r12 - super_star.png - super_star_badge
	//r11 - gold_streak.png - gold_streak_badge
	//r10 - dark_badge1.png - dark_streak_badge
	//r9 - badge_ncs.png - ncs_badge
	//r8 - badge_diamante.png - diamante_gd_badge
	//r4 - badge_platino.png - platino_streak_badge
	//r1 - badge_beta.png - beta_badge

	//badge_diamante_mc.png - diamante_mc_badge


   

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


    int getTicketValueForRarity(BadgeCategory category) {
        switch (category) {
        case BadgeCategory::COMMON:    return 5;
        case BadgeCategory::SPECIAL:   return 20;
        case BadgeCategory::EPIC:      return 50;
        case BadgeCategory::LEGENDARY: return 100;
        case BadgeCategory::MYTHIC:    return 500;
        default:                       return 0;
        }
    }

    // Añade esta función dentro de tu struct StreakData
    void unlockBadge(const std::string& badgeID) {
        for (size_t i = 0; i < badges.size(); ++i) {
            if (badges[i].badgeID == badgeID) {
                unlockedBadges[i] = true;
                return; // Insignia encontrada y desbloqueada
            }
        }
    }

    // En la función load() de StreakData:
    void load() {
        currentStreak = geode::Mod::get()->getSavedValue<int>("streak", 0);
        starsToday = geode::Mod::get()->getSavedValue<int>("starsToday", 0);
        hasNewStreak = geode::Mod::get()->getSavedValue<bool>("hasNewStreak", false);
        lastDay = geode::Mod::get()->getSavedValue<std::string>("lastDay", "");
        equippedBadge = geode::Mod::get()->getSavedValue<std::string>("equippedBadge", "");
        superStars = geode::Mod::get()->getSavedValue<int>("superStars", 0);
        lastRouletteIndex = geode::Mod::get()->getSavedValue<int>("lastRouletteIndex", 0);
        totalSpins = geode::Mod::get()->getSavedValue<int>("totalSpins", 0);
        starTickets = geode::Mod::get()->getSavedValue<int>("starTickets", 0);

        starMission1Claimed = geode::Mod::get()->getSavedValue<bool>("starMission1Claimed", false);
        starMission2Claimed = geode::Mod::get()->getSavedValue<bool>("starMission2Claimed", false);
        starMission3Claimed = geode::Mod::get()->getSavedValue<bool>("starMission3Claimed", false);

        streakCompletedLevels = geode::Mod::get()->getSavedValue<std::vector<int>>("streakCompletedLevels", {});

        // Cargar insignias desbloqueadas
        unlockedBadges.resize(badges.size(), false);
        for (size_t i = 0; i < badges.size(); i++) {
            unlockedBadges[i] = geode::Mod::get()->getSavedValue<bool>(badges[i].badgeID, false);
        }

        starHistory = geode::Mod::get()->getSavedValue<std::map<std::string, int>>("starHistory", {});
    }

    void save() {
        geode::Mod::get()->setSavedValue<int>("streak", currentStreak);
        geode::Mod::get()->setSavedValue<int>("starsToday", starsToday);
        geode::Mod::get()->setSavedValue<bool>("hasNewStreak", hasNewStreak);
        geode::Mod::get()->setSavedValue<std::string>("lastDay", lastDay);
        geode::Mod::get()->setSavedValue<std::string>("equippedBadge", equippedBadge);
        geode::Mod::get()->setSavedValue<int>("superStars", superStars);
        geode::Mod::get()->setSavedValue<int>("lastRouletteIndex", lastRouletteIndex);
        geode::Mod::get()->setSavedValue<int>("totalSpins", totalSpins);
        geode::Mod::get()->setSavedValue<int>("starTickets", starTickets);

        geode::Mod::get()->setSavedValue<bool>("starMission1Claimed", starMission1Claimed);
        geode::Mod::get()->setSavedValue<bool>("starMission2Claimed", starMission2Claimed);
        geode::Mod::get()->setSavedValue<bool>("starMission3Claimed", starMission3Claimed);

        geode::Mod::get()->setSavedValue<std::vector<int>>("streakCompletedLevels", streakCompletedLevels);

        // Guardar insignias desbloqueadas
        for (size_t i = 0; i < badges.size(); i++) {
            geode::Mod::get()->setSavedValue<bool>(badges[i].badgeID, unlockedBadges[i]);
        }

        geode::Mod::get()->setSavedValue("starHistory", starHistory);
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
        time_t now_t = time(nullptr);
        std::string today = getCurrentDate();

        // Si es la primera vez que se ejecuta o los datos están vacíos
        if (lastDay.empty()) {
            lastDay = today;
            starsToday = 0;
            save();
            return;
        }

        // Si sigue siendo el mismo día, no hacemos nada
        if (lastDay == today) {
            return;
        }

        // --- LÓGICA CORREGIDA ---
        // Si la fecha es diferente, calculamos cuántos días han pasado.

        tm last_tm = {};
        std::stringstream ss(lastDay);
        // Usamos std::get_time para convertir el string "YYYY-MM-DD" a un objeto de tiempo
        ss >> std::get_time(&last_tm, "%Y-%m-%d");
        time_t last_t = mktime(&last_tm);

        // Calculamos la diferencia de tiempo en segundos y la convertimos a días
        double seconds_passed = difftime(now_t, last_t);
        double days_passed = seconds_passed / (60.0 * 60.0 * 24.0);

        bool streak_should_be_lost = false;

        if (days_passed >= 2.0) {
            // Si han pasado 2 días o más, la racha se pierde automáticamente.
            streak_should_be_lost = true;
        }
        else if (days_passed >= 1.0) {
            // Si ha pasado solo 1 día, comprobamos si se cumplió la meta de estrellas.
            if (starsToday < getRequiredStars()) {
                streak_should_be_lost = true;
            }
        }

        if (streak_should_be_lost) {
            currentStreak = 0;
            starHistory.clear(); // Limpiamos el historial al perder la racha
            FLAlertLayer::create("Streak Lost", "You missed a day!", "OK")->show();
        }

        // Finalmente, reiniciamos los valores para el nuevo día (hoy)
        starsToday = 0;
        lastDay = today;
        starMission1Claimed = false;
        starMission2Claimed = false;
        starMission3Claimed = false;

        save();
    }

    void checkRewards() {
        for (size_t i = 0; i < badges.size(); i++) {

            if (badges[i].isFromRoulette) {
                continue; // Saltar a la siguiente insignia
            }
            if (currentStreak >= badges[i].daysRequired && !unlockedBadges[i]) {
                unlockedBadges[i] = true;
            }
        }
        save();
    }

    void addStars(int count) {
        load();
        dailyUpdate();

        int currentRequired = getRequiredStars();
        bool alreadyGotRacha = (starsToday >= currentRequired);

        starsToday += count;

        // La lógica del historial ahora es solo esto, sin "DailyData"
        std::string today = getCurrentDate();
        starHistory[today] = starsToday;

        if (!alreadyGotRacha && starsToday >= currentRequired) {
            currentStreak++;
            hasNewStreak = true;

            int oldRequired = currentRequired;
            int newRequired = getRequiredStars();
            if (newRequired > oldRequired) {
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
        case BadgeCategory::SPECIAL: return ccc3(0, 170, 0); // Verde
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



class $modify(MyGameStatsManager, GameStatsManager) {
    void incrementStat(char const* key, int amount) {
        // CORRECCIÓN: Se cambió "6"sv por std::string("6")
        if (std::string(key) == "6") {
            // El resto de tu lógica para sumar estrellas
            if (amount > 0 && amount <= 15) {
                g_streakData.addStars(amount);
            }
        }

        GameStatsManager::incrementStat(key, amount);
    }
};


// NUEVA función con colores más brillantes solo para la ruleta

ccColor3B getBrightQualityColor(StreakData::BadgeCategory category) {
    switch (category) {
    case StreakData::BadgeCategory::COMMON:   return ccc3(220, 220, 220);
    case StreakData::BadgeCategory::SPECIAL:  return ccc3(0, 255, 80);
    case StreakData::BadgeCategory::EPIC:     return ccc3(255, 0, 255);
    case StreakData::BadgeCategory::LEGENDARY:return ccc3(255, 200, 0);
    case StreakData::BadgeCategory::MYTHIC:   return ccc3(255, 60, 60);
    default:                                  return ccc3(255, 255, 255);
    }
}

// Estructura para definir los premios de la ruleta con su probabilidad
// Estructura para definir los premios de la ruleta con su probabilidad
enum class RewardType {
    Badge,
    SuperStar,
    StarTicket
};

struct RoulettePrize {
    RewardType type;
    std::string id; // ID de la insignia o un identificador para el item
    int quantity;   // Cantidad de items a entregar (para insignias será 1)
    std::string spriteName;
    std::string displayName;
    int probabilityWeight;
    StreakData::BadgeCategory category; // Para el color del fondo de la casilla
};

struct GenericPrizeResult {
    RewardType type; // <--- Ahora esto funcionará
    std::string id;
    int quantity;
    std::string displayName;
    std::string spriteName;
    StreakData::BadgeCategory category;
    bool isNew = false;
    int ticketsFromDuplicate = 0;
};



// HistoryCell hereda de CCLayer de nuevo, es solo un contenedor transparente
class HistoryCell : public cocos2d::CCLayer {
protected:
    bool init(const std::string& date, int stars) {
        if (!cocos2d::CCLayer::init()) return false;

        this->setContentSize({ 280.f, 25.f });
        float cellHeight = this->getContentSize().height;

        auto dateLabel = cocos2d::CCLabelBMFont::create(date.c_str(), "goldFont.fnt");
        dateLabel->setScale(0.5f);
        dateLabel->setAnchorPoint({ 0.0f, 0.5f });
        dateLabel->setPosition({ 10.f, cellHeight / 2 });
        this->addChild(dateLabel);

        auto starLabel = cocos2d::CCLabelBMFont::create(std::to_string(stars).c_str(), "bigFont.fnt");
        starLabel->setScale(0.4f);
        starLabel->setAnchorPoint({ 1.0f, 0.5f });
        starLabel->setPosition({ this->getContentSize().width - 10.f, cellHeight / 2 });
        this->addChild(starLabel);

        auto starIcon = cocos2d::CCSprite::createWithSpriteFrameName("GJ_starsIcon_001.png");
        starIcon->setScale(0.5f);
        starIcon->setPosition({ starLabel->getPositionX() - starLabel->getScaledContentSize().width - 5.f, cellHeight / 2 });
        this->addChild(starIcon);

        return true;
    }

public:
    static HistoryCell* create(const std::string& date, int stars) {
        auto ret = new HistoryCell();
        if (ret && ret->init(date, stars)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

class HistoryPopup : public Popup<> {
protected:
    // Variables para manejar las páginas
    std::vector<std::pair<std::string, int>> m_historyEntries;
    int m_currentPage = 0;
    int m_totalPages = 0;
    const int m_itemsPerPage = 8;

    // Nodos de la UI
    CCLayer* m_pageContainer;
    CCMenuItemSpriteExtra* m_leftArrow;
    CCMenuItemSpriteExtra* m_rightArrow;

    bool setup() override {
        this->setTitle("Star History");
        g_streakData.load();

        auto listSize = CCSize{ 280.f, 200.f };
        auto popupCenter = m_mainLayer->getContentSize() / 2;

        auto listBg = cocos2d::extension::CCScale9Sprite::create("square02_001.png");
        listBg->setContentSize(listSize);
        listBg->setColor({ 0, 0, 0 });
        listBg->setOpacity(100);
        listBg->setPosition(popupCenter);
        m_mainLayer->addChild(listBg);

        m_historyEntries = std::vector<std::pair<std::string, int>>(g_streakData.starHistory.begin(), g_streakData.starHistory.end());

        // --- CAMBIO 1: Ordenar del más ANTIGUO al más RECIENTE ---
        std::sort(m_historyEntries.begin(), m_historyEntries.end(), [](const auto& a, const auto& b) {
            return a.first < b.first; // Se usa '<' en lugar de '>'
            });

        // --- CAMBIO 2: Se eliminaron los datos de prueba ---

        m_totalPages = static_cast<int>(ceil(static_cast<float>(m_historyEntries.size()) / m_itemsPerPage));

        // --- CAMBIO 3: Empezar en la última página (la más reciente) ---
        m_currentPage = (m_totalPages > 0) ? (m_totalPages - 1) : 0;

        m_pageContainer = CCLayer::create();
        m_pageContainer->setPosition(popupCenter - listSize / 2);
        m_mainLayer->addChild(m_pageContainer);

        // Flechas de Navegación
        auto leftSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
        m_leftArrow = CCMenuItemSpriteExtra::create(leftSpr, this, menu_selector(HistoryPopup::onPrevPage));

        auto rightSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
        rightSpr->setFlipX(true);
        m_rightArrow = CCMenuItemSpriteExtra::create(rightSpr, this, menu_selector(HistoryPopup::onNextPage));

        auto navMenu = CCMenu::create();
        navMenu->addChild(m_leftArrow);
        navMenu->addChild(m_rightArrow);
        navMenu->alignItemsHorizontallyWithPadding(listSize.width + 25.f);
        navMenu->setPosition(popupCenter);
        m_mainLayer->addChild(navMenu);

        this->updatePage();

        return true;
    }

    void updatePage() {
        if (m_totalPages > 0) {
            this->setTitle(fmt::format("Star History - Week {}", m_currentPage + 1).c_str());
        }
        else {
            this->setTitle("Star History");
        }

        m_pageContainer->removeAllChildren();
        int startIndex = m_currentPage * m_itemsPerPage;

        for (int i = 0; i < m_itemsPerPage; ++i) {
            int entryIndex = startIndex + i;
            if (entryIndex < m_historyEntries.size()) {
                const auto& [date, stars] = m_historyEntries[entryIndex];
                auto cell = HistoryCell::create(date, stars);

                cell->setPosition({ 0, 200.f - (i + 1) * 25.f });
                m_pageContainer->addChild(cell);
            }
        }

        m_leftArrow->setVisible(m_currentPage > 0);
        m_rightArrow->setVisible(m_currentPage < m_totalPages - 1);
    }

    void onPrevPage(CCObject*) {
        if (m_currentPage > 0) {
            m_currentPage--;
            this->updatePage();
        }
    }

    void onNextPage(CCObject*) {
        if (m_currentPage < m_totalPages - 1) {
            m_currentPage++;
            this->updatePage();
        }
    }

public:
    static HistoryPopup* create() {
        auto ret = new HistoryPopup();
        if (ret && ret->initAnchored(340.f, 250.f)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

// ============= CAPA PARA LA ANIMACIÓN DE TICKETS (VERSIÓN COMPATIBLE) =============
class TicketAnimationLayer : public CCLayer {
protected:
    int m_ticketsWon;
    CCLabelBMFont* m_counterLabel;
    cocos2d::extension::CCScale9Sprite* m_counterBG;
    int m_initialTickets;
    float m_ticketsPerParticle;
    float m_ticketAccumulator = 0.f;

    bool init(int ticketsWon) {
        if (!CCLayer::init()) return false;
        m_ticketsWon = ticketsWon;

        if (m_ticketsWon <= 0) {
            this->removeFromParent();
            return true;
        }

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        m_initialTickets = g_streakData.starTickets - m_ticketsWon;

        m_counterBG = cocos2d::extension::CCScale9Sprite::create("square02_001.png");
        m_counterBG->setColor({ 0, 0, 0 });
        m_counterBG->setOpacity(120);
        this->addChild(m_counterBG);

        auto ticketSprite = CCSprite::create("star_tiket.png"_spr);
        ticketSprite->setScale(0.35f);
        m_counterBG->addChild(ticketSprite);

        m_counterLabel = CCLabelBMFont::create(std::to_string(m_initialTickets).c_str(), "goldFont.fnt");
        m_counterLabel->setScale(0.6f);
        m_counterBG->addChild(m_counterLabel);

        m_counterBG->setContentSize({ m_counterLabel->getScaledContentSize().width + ticketSprite->getScaledContentSize().width + 25.f, 40.f });
        ticketSprite->setPosition({ m_counterBG->getContentSize().width - ticketSprite->getScaledContentSize().width / 2 - 10.f, m_counterBG->getContentSize().height / 2 });
        m_counterLabel->setPosition({ m_counterLabel->getScaledContentSize().width / 2 + 10.f, m_counterBG->getContentSize().height / 2 });
        m_counterBG->setPosition(winSize.width - m_counterBG->getContentSize().width / 2 - 5.f, winSize.height - m_counterBG->getContentSize().height / 2 - 5.f);

        this->runAnimation(m_counterBG->getPosition());
        return true;
    }

    void runAnimation(CCPoint counterPos) {
        auto winSize = CCDirector::sharedDirector()->getWinSize();
        int particleCount = std::min(m_ticketsWon, 30);
        m_ticketsPerParticle = static_cast<float>(m_ticketsWon) / particleCount;
        float duration = 0.5f;
        float delayPerParticle = 0.05f;

        for (int i = 0; i < particleCount; ++i) {
            auto particle = CCSprite::create("star_tiket.png"_spr);
            particle->setPosition(winSize / 2);
            particle->setScale(0.4f);
            particle->setRotation((rand() % 40) - 20);
            this->addChild(particle);

            ccBezierConfig bezier;
            bezier.endPosition = counterPos;
            bezier.controlPoint_1 = ccp(winSize.width / 2 + (rand() % 200) - 100, winSize.height / 2 + (rand() % 150) - 75);
            bezier.controlPoint_2 = counterPos + ccp((rand() % 100) - 50, (rand() % 100) - 50);

            auto bezierTo = CCBezierTo::create(duration, bezier);
            auto scaleTo = CCScaleTo::create(duration, 0.1f);
            auto fadeOut = CCFadeOut::create(duration * 0.8f);

            particle->runAction(CCSequence::create(
                CCDelayTime::create(i * delayPerParticle),
                CCSpawn::create(CCEaseSineIn::create(bezierTo), scaleTo, CCSequence::create(CCDelayTime::create(duration * 0.2f), fadeOut, nullptr), nullptr),
                CCCallFuncN::create(this, callfuncN_selector(TicketAnimationLayer::onParticleHit)),
                CCRemoveSelf::create(),
                nullptr
            ));
        }

        this->runAction(CCSequence::create(
            CCDelayTime::create(particleCount * delayPerParticle + duration),
            CCCallFunc::create(this, callfunc_selector(TicketAnimationLayer::onAnimationEnd)),
            nullptr
        ));
    }

    void onParticleHit(CCNode* sender) {
        FMODAudioEngine::sharedEngine()->playEffect("coin.mp3"_spr);
        m_ticketAccumulator += m_ticketsPerParticle;
        m_counterLabel->setString(std::to_string(m_initialTickets + static_cast<int>(m_ticketAccumulator)).c_str());
        m_counterLabel->runAction(CCSequence::create(
            CCScaleTo::create(0.1f, 0.8f),
            CCScaleTo::create(0.1f, 0.6f),
            nullptr
        ));
    }

    // --- NUEVA FUNCIÓN ---
    // Esta función contiene la lógica de la animación de salida
    void runSlideOffAnimation() {
        auto slideOff = CCEaseSineIn::create(
            CCMoveBy::create(0.4f, { m_counterBG->getContentSize().width + 10.f, 0 })
        );
        m_counterBG->runAction(slideOff);
    }

    void onAnimationEnd() {
        m_counterLabel->setString(std::to_string(g_streakData.starTickets).c_str());

        // --- LÓGICA CORREGIDA ---
        // Ahora llamamos a nuestra nueva función usando el método clásico
        this->runAction(CCSequence::create(
            CCDelayTime::create(0.5f),
            CCCallFunc::create(this, callfunc_selector(TicketAnimationLayer::runSlideOffAnimation)),
            CCDelayTime::create(0.5f),
            CCRemoveSelf::create(),
            nullptr
        ));
    }

public:
    static TicketAnimationLayer* create(int ticketsWon) {
        auto ret = new TicketAnimationLayer();
        if (ret && ret->init(ticketsWon)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

// ============= POPUP DE PREMIO GENÉRICO (CON ÍCONO DE TICKET EN DUPLICADOS) =============
class GenericPrizePopup : public Popup<GenericPrizeResult, std::function<void()>> {
protected:
    std::function<void()> m_onCloseCallback = nullptr;

    void onClose(CCObject* sender) override {
        if (m_onCloseCallback) {
            m_onCloseCallback();
        }
        Popup::onClose(sender);
    }

    bool setup(GenericPrizeResult prize, std::function<void()> onCloseCallback) override {
        m_onCloseCallback = onCloseCallback;
        auto winSize = m_mainLayer->getContentSize();

        this->setTitle("You Won a Prize!");

        auto nameLabel = CCLabelBMFont::create(prize.displayName.c_str(), "goldFont.fnt");
        nameLabel->setScale(0.7f);
        m_mainLayer->addChild(nameLabel);

        auto categoryLabel = CCLabelBMFont::create(g_streakData.getCategoryName(prize.category).c_str(), "bigFont.fnt");
        categoryLabel->setColor(g_streakData.getCategoryColor(prize.category));
        categoryLabel->setScale(0.5f);
        m_mainLayer->addChild(categoryLabel);

        if (prize.type == RewardType::Badge) {
            if (!prize.isNew) {
                this->setTitle("Duplicate Badge!");

                // --- CORRECCIÓN AQUÍ ---
                // Nodo para agrupar el texto y el ícono del ticket
                auto rewardNode = CCNode::create();
                rewardNode->setPosition({ winSize.width / 2, winSize.height / 2 + 55.f });
                m_mainLayer->addChild(rewardNode);
                

                // Etiqueta con la cantidad
                auto amountLabel = CCLabelBMFont::create(fmt::format("+{}", prize.ticketsFromDuplicate).c_str(), "goldFont.fnt");
                amountLabel->setScale(0.6f);
                rewardNode->addChild(amountLabel);

                // Ícono del ticket
                auto ticketSprite = CCSprite::create("star_tiket.png"_spr);
                ticketSprite->setScale(0.3f);
                rewardNode->addChild(ticketSprite);

                // Alinear texto e ícono
                float totalWidth = amountLabel->getScaledContentSize().width + ticketSprite->getScaledContentSize().width + 5.f;
                amountLabel->setPosition({ -totalWidth / 2 + amountLabel->getScaledContentSize().width / 2, 0 });
                ticketSprite->setPosition({ totalWidth / 2 - ticketSprite->getScaledContentSize().width / 2, 0 });

            }
            else {
                this->setTitle("You Won a Badge!");
            }
            auto badgeSprite = CCSprite::create(prize.spriteName.c_str());
            badgeSprite->setPosition({ winSize.width / 2, winSize.height / 2 + 20.f });
            badgeSprite->setScale(0.3f);
            m_mainLayer->addChild(badgeSprite);
            nameLabel->setPosition({ winSize.width / 2, winSize.height / 2 - 35.f });
            categoryLabel->setPosition({ winSize.width / 2, winSize.height / 2 - 55.f });

        }
        else { // Para Tickets o Super Estrellas
            auto itemSprite = CCSprite::create(prize.spriteName.c_str());
            itemSprite->setPosition({ winSize.width / 2, winSize.height / 2 + 30.f });
            itemSprite->setScale(0.6f);
            m_mainLayer->addChild(itemSprite);
            nameLabel->setPosition({ winSize.width / 2, winSize.height / 2 - 25.f });
            categoryLabel->setPosition({ winSize.width / 2, winSize.height / 2 - 45.f });
        }

        auto okBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("OK"), this, menu_selector(GenericPrizePopup::onClose));
        auto menu = CCMenu::createWithItem(okBtn);
        menu->setPosition({ winSize.width / 2, 40.f });
        m_mainLayer->addChild(menu);

        return true;
    }

public:
    static GenericPrizePopup* create(GenericPrizeResult prize, std::function<void()> onCloseCallback) {
        auto ret = new GenericPrizePopup();
        if (ret && ret->initAnchored(280.f, 240.f, prize, onCloseCallback)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};


// ============= ANIMACIÓN DE PREMIO MÍTICO (CON BOTÓN OK FUNCIONAL) =============
class MythicAnimationLayer : public CCLayer {
public:
    CCMenu* m_okMenu;
    CCLabelBMFont* m_mythicLabel;
    std::vector<ccColor3B> m_mythicColors;
    int m_colorIndex = 0;
    float m_colorTransitionTime = 0.0f;
    ccColor3B m_currentColor;
    ccColor3B m_targetColor;
    std::function<void()> m_onCompletionCallback;

    virtual void onEnter() {
        CCLayer::onEnter();
        CCDirector::sharedDirector()->getTouchDispatcher()->addTargetedDelegate(this, -513, true);
    }

    virtual void onExit() {
        CCDirector::sharedDirector()->getTouchDispatcher()->removeDelegate(this);
        CCLayer::onExit();
    }

    // --- FUNCIÓN ccTouchBegan CORREGIDA CON ÁREA DE CLIC MÁS GRANDE ---
    virtual bool ccTouchBegan(CCTouch* touch, CCEvent* event) {
        // Convertimos la ubicación del toque a las coordenadas de esta capa
        auto location = this->convertTouchToNodeSpace(touch);

        if (m_okMenu) {
            // Obtenemos el área rectangular del menú
            CCRect menuBox = m_okMenu->boundingBox();

            // --- CAMBIO AQUÍ ---
            // Creamos un área de clic más grande, añadiendo un margen de 15 píxeles por cada lado.
            CCRect largerClickArea = CCRect(
                menuBox.origin.x - 15,
                menuBox.origin.y - 15,
                menuBox.size.width + 30,
                menuBox.size.height + 30
            );

            // Verificamos si el toque está dentro de esta nueva área más grande
            if (largerClickArea.containsPoint(location)) {
                // Si el toque fue en el área del botón, retornamos 'false' para que el menú pueda procesarlo.
                return false;
            }
        }

        // Si el toque fue en cualquier otro lugar, lo absorbemos para bloquear los botones de fondo.
        return true;
    }

    // Estas funciones son necesarias aunque estén vacías
    virtual void ccTouchMoved(CCTouch* touch, CCEvent* event) {}
    virtual void ccTouchEnded(CCTouch* touch, CCEvent* event) {}
    virtual void ccTouchCancelled(CCTouch* touch, CCEvent* event) {}


    void playCustomSound() {
        FMODAudioEngine::sharedEngine()->playEffect("mythic_badge.mp3"_spr);
    }

    void onMythicClose(CCObject*) {
        if (m_onCompletionCallback) {
            m_onCompletionCallback();
        }
        this->removeFromParent();
    }

    static MythicAnimationLayer* create(StreakData::BadgeInfo badge, std::function<void()> onCompletion = nullptr) {
        auto ret = new MythicAnimationLayer();
        if (ret && ret->init(badge, onCompletion)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    bool init(StreakData::BadgeInfo badge, std::function<void()> onCompletion) {
        if (!CCLayer::init()) return false;

        m_onCompletionCallback = onCompletion;
        auto winSize = CCDirector::sharedDirector()->getWinSize();

        m_mythicColors = {
            ccc3(255, 0, 0), ccc3(255, 165, 0), ccc3(255, 255, 0), ccc3(0, 255, 0),
            ccc3(0, 0, 255), ccc3(75, 0, 130), ccc3(238, 130, 238)
        };
        m_currentColor = m_mythicColors[0];
        m_targetColor = m_mythicColors[1];

        auto background = CCLayerColor::create({ 0, 0, 0, 0 });
        background->runAction(CCFadeTo::create(0.5f, 180));
        this->addChild(background);

        auto congratsLabel = CCLabelBMFont::create("Congratulations!", "goldFont.fnt");
        congratsLabel->setColor({ 255, 80, 80 });
        congratsLabel->setPosition(winSize / 2);
        congratsLabel->setScale(0.f);
        this->addChild(congratsLabel);

        auto badgeNode = CCNode::create();
        badgeNode->setPosition(winSize / 2);
        badgeNode->setScale(0.f);
        this->addChild(badgeNode);

        auto shineSprite = CCSprite::createWithSpriteFrameName("shineBurst_001.png");
        shineSprite->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });
        shineSprite->setOpacity(150);
        shineSprite->setScale(3.f);
        badgeNode->addChild(shineSprite);

        auto badgeSprite = CCSprite::create(badge.spriteName.c_str());
        badgeSprite->setScale(0.4f);
        badgeNode->addChild(badgeSprite, 2);

        m_mythicLabel = CCLabelBMFont::create("Mythic", "goldFont.fnt");
        m_mythicLabel->setColor({ 255, 80, 80 });
        m_mythicLabel->setAnchorPoint({ 0, 0.5f });
        m_mythicLabel->setPosition({ -200.f, winSize.height - 40.f });
        m_mythicLabel->setOpacity(0);
        m_mythicLabel->setScale(0.7f);
        this->addChild(m_mythicLabel);

        auto nameLabel = CCLabelBMFont::create(badge.displayName.c_str(), "goldFont.fnt");
        nameLabel->setAnchorPoint({ 0, 0.5f });
        nameLabel->setPosition({ -200.f, winSize.height - 65.f });
        nameLabel->setOpacity(0);
        nameLabel->setScale(0.5f);
        this->addChild(nameLabel);

        auto okBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("OK"), this, menu_selector(MythicAnimationLayer::onMythicClose));
        m_okMenu = CCMenu::createWithItem(okBtn);
        m_okMenu->setPosition({ winSize.width / 2, 60.f });
        this->addChild(m_okMenu);

        FMODAudioEngine::sharedEngine()->playEffect("achievement.mp3"_spr);
        congratsLabel->runAction(CCSequence::create(
            CCEaseBackOut::create(CCScaleTo::create(0.4f, 1.2f)),
            CCDelayTime::create(0.7f),
            CCEaseBackIn::create(CCScaleTo::create(0.4f, 0.f)),
            nullptr
        ));
        badgeNode->runAction(CCSequence::create(
            CCDelayTime::create(1.5f),
            CCCallFunc::create(this, callfunc_selector(MythicAnimationLayer::playCustomSound)),
            CCEaseBackOut::create(CCScaleTo::create(0.5f, 1.0f)),
            nullptr
        ));
        shineSprite->runAction(CCRepeatForever::create(CCRotateBy::create(4.0f, 360.f)));
        auto floatUp = CCMoveBy::create(1.5f, { 0, 5.f });
        badgeNode->runAction(CCSequence::create(
            CCDelayTime::create(2.0f),
            CCRepeatForever::create(CCSequence::create(
                CCEaseSineInOut::create(floatUp),
                CCEaseSineInOut::create(floatUp->reverse()),
                nullptr
            )),
            nullptr
        ));
        auto cornerTextMove1 = CCEaseSineOut::create(CCMoveTo::create(0.5f, { 20.f, winSize.height - 40.f }));
        m_mythicLabel->runAction(CCSequence::create(
            CCDelayTime::create(2.5f),
            CCSpawn::create(CCFadeIn::create(0.5f), cornerTextMove1, nullptr),
            nullptr
        ));
        auto cornerTextMove2 = CCEaseSineOut::create(CCMoveTo::create(0.5f, { 20.f, winSize.height - 65.f }));
        nameLabel->runAction(CCSequence::create(
            CCDelayTime::create(2.5f),
            CCSpawn::create(CCFadeIn::create(0.5f), cornerTextMove2, nullptr),
            nullptr
        ));

        this->scheduleUpdate();
        return true;
    }

    void update(float dt) override {
        m_colorTransitionTime += dt;
        float transitionDuration = 1.0f;
        if (m_colorTransitionTime >= transitionDuration) {
            m_colorTransitionTime = 0.0f;
            m_colorIndex = (m_colorIndex + 1) % m_mythicColors.size();
            m_currentColor = m_targetColor;
            m_targetColor = m_mythicColors[(m_colorIndex + 1) % m_mythicColors.size()];
        }
        float progress = m_colorTransitionTime / transitionDuration;
        ccColor3B interpolatedColor = {
            static_cast<GLubyte>(m_currentColor.r + (m_targetColor.r - m_currentColor.r) * progress),
            static_cast<GLubyte>(m_currentColor.g + (m_targetColor.g - m_currentColor.g) * progress),
            static_cast<GLubyte>(m_currentColor.b + (m_targetColor.b - m_currentColor.b) * progress)
        };
        m_mythicLabel->setColor(interpolatedColor);
    }
};



class MultiPrizePopup : public Popup<std::vector<GenericPrizeResult>, std::function<void()>> {
protected:
    std::function<void()> m_onCloseCallback;

    void onClose(CCObject* sender) override {
        if (m_onCloseCallback) {
            m_onCloseCallback();
        }
        Popup::onClose(sender);
    }

    bool setup(std::vector<GenericPrizeResult> prizes, std::function<void()> onCloseCallback) override {
        this->setTitle("Congratulations!");
        m_onCloseCallback = onCloseCallback;
        auto winSize = m_mainLayer->getContentSize();

        auto prizesContainer = CCNode::create();
        m_mainLayer->addChild(prizesContainer);
        const int cols = 5;
        const float itemWidth = 70.f;
        const float itemHeight = 75.f;
        const CCPoint startPos = {
            (winSize.width - (cols - 1) * itemWidth) / 2.f,
            winSize.height / 2.f + 45.f
        };

        for (size_t i = 0; i < prizes.size(); ++i) {
            auto& result = prizes[i];
            int row = i / cols;
            int col = i % cols;
            auto itemNode = CCNode::create();
            itemNode->setPosition({ startPos.x + col * itemWidth, startPos.y - row * itemHeight });
            prizesContainer->addChild(itemNode);

            auto itemSprite = CCSprite::create(result.spriteName.c_str());
            itemSprite->setScale(0.28f);
            itemNode->addChild(itemSprite);

            auto categoryLabel = CCLabelBMFont::create(g_streakData.getCategoryName(result.category).c_str(), "goldFont.fnt");
            categoryLabel->setColor(getBrightQualityColor(result.category));
            categoryLabel->setScale(0.4f);
            categoryLabel->setPosition({ 0, -25.f });
            itemNode->addChild(categoryLabel);

            // --- LÓGICA CORREGIDA Y SIMPLIFICADA ---
            if (result.type == RewardType::Badge) {
                if (result.isNew) {
                    // Si es una insignia NUEVA, muestra la etiqueta "NEW!"
                    auto newLabel = CCLabelBMFont::create("NEW!", "bigFont.fnt");
                    newLabel->setColor(getBrightQualityColor(result.category));
                    newLabel->setScale(0.4f);
                    newLabel->setPosition({ 18.f, 18.f });
                    itemNode->addChild(newLabel);
                    newLabel->runAction(CCRepeatForever::create(CCSequence::create(
                        CCScaleTo::create(0.5f, 0.45f), CCScaleTo::create(0.5f, 0.4f), nullptr
                    )));
                }
                else {
                    // Si es una insignia DUPLICADA, muestra los tickets ganados con su ícono
                    auto ticketNode = CCNode::create();
                    ticketNode->setPosition({ 20.f, 18.f });
                    itemNode->addChild(ticketNode);

                    auto ticketSprite = CCSprite::create("star_tiket2.png"_spr);
                    ticketSprite->setScale(0.18f);
                    ticketSprite->setPosition({ -5.f, 0 });
                    ticketNode->addChild(ticketSprite);

                    auto amountLabel = CCLabelBMFont::create(fmt::format("+{}", result.ticketsFromDuplicate).c_str(), "goldFont.fnt");
                    amountLabel->setScale(0.4f);
                    amountLabel->setAnchorPoint({ 0, 0.5f });
                    amountLabel->setPosition({ 5.f, 0 });
                    ticketNode->addChild(amountLabel);
                }
            }
            else {
                // Si es un ITEM (Ticket o Super Estrella), muestra solo la cantidad
                auto amountLabel = CCLabelBMFont::create(fmt::format("x{}", result.quantity).c_str(), "goldFont.fnt");
                amountLabel->setScale(0.5f);
                amountLabel->setPosition({ 18.f, -18.f }); // Posicionado en la esquina inferior derecha
                itemNode->addChild(amountLabel);
            }
        }

        auto okBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("OK"), this, menu_selector(MultiPrizePopup::onClose));
        auto menu = CCMenu::createWithItem(okBtn);
        menu->setPosition({ winSize.width / 2, 30.f });
        m_mainLayer->addChild(menu);

        return true;
    }

public:
    static MultiPrizePopup* create(std::vector<GenericPrizeResult> prizes, std::function<void()> onCloseCallback = nullptr) {
        auto ret = new MultiPrizePopup();
        if (ret && ret->initAnchored(380.f, 230.f, prizes, onCloseCallback)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};


//r12 - super_star.png - super_star_badge
//r11 - gold_streak.png - gold_streak_badge
//r10 - dark_badge1.png - dark_streak_badge
//r9 - badge_ncs.png - ncs_badge
//r8 - badge_diamante.png - diamante_gd_badge
//r4 - badge_platino.png - platino_streak_badge
//r1 - badge_beta.png - beta_badge

//badge_diamante_mc.png - diamante_mc_badge

// ============= POPUP DE LA TINDA GENERAL (CORREGIDO) =============
class ShopPopup : public Popup<> {
protected:
    CCLabelBMFont* m_ticketCounterLabel = nullptr;
    CCMenu* m_itemMenu = nullptr;

    // Guardamos los ítems de la tienda para acceder a ellos fácilmente
    std::map<std::string, int> m_shopItems;

    bool setup() override {
        this->setTitle("Streak Shop");
        auto winSize = m_mainLayer->getContentSize();
        g_streakData.load();

        // --- PRECIOS MANUALES ---
        m_shopItems = {
            {"beta_badge", 25},
            {"diamante_mc_badge", 550},
            {"platino_streak_badge", 30},
            {"diamante_gd_badge", 40},
            {"ncs_badge", 150},
            {"dark_streak_badge", 500},
            {"gold_streak_badge", 1200},
            {"super_star_badge", 5000}
        };

        auto background = cocos2d::extension::CCScale9Sprite::create("square02_001.png");
        background->setColor({ 0, 0, 0 });
        background->setOpacity(120);
        background->setContentSize({ 340.f, 220.f });
        background->setPosition(winSize / 2);
        m_mainLayer->addChild(background);

        auto ticketNode = CCNode::create();
        m_mainLayer->addChild(ticketNode);
        m_ticketCounterLabel = CCLabelBMFont::create(std::to_string(g_streakData.starTickets).c_str(), "goldFont.fnt");
        m_ticketCounterLabel->setScale(0.5f);
        ticketNode->addChild(m_ticketCounterLabel);
        auto ticketSprite = CCSprite::create("star_tiket.png"_spr);
        ticketSprite->setScale(0.25f);
        ticketNode->addChild(ticketSprite);
        ticketNode->setContentSize({
            m_ticketCounterLabel->getScaledContentSize().width + ticketSprite->getScaledContentSize().width + 5.f,
            ticketSprite->getScaledContentSize().height
            });
        m_ticketCounterLabel->setPosition({ -ticketSprite->getScaledContentSize().width / 2, 0 });
        ticketSprite->setPosition({ m_ticketCounterLabel->getScaledContentSize().width / 2 + 5.f, 0 });
        ticketNode->setPosition({ m_size.width - ticketNode->getContentSize().width / 2 - 15.f, m_size.height - 25.f });

        m_itemMenu = CCMenu::create();
        m_itemMenu->setContentSize(background->getContentSize());
        m_itemMenu->ignoreAnchorPointForPosition(false);
        m_itemMenu->setPosition(background->getPosition());
        m_mainLayer->addChild(m_itemMenu);

        updateItems();

        return true;
    }

    void updateItems() {
        m_itemMenu->removeAllChildren();
        g_streakData.load();

        std::vector<std::pair<std::string, int>> sortedItems(m_shopItems.begin(), m_shopItems.end());

        std::sort(sortedItems.begin(), sortedItems.end(), [this](const auto& a, const auto& b) {
            auto* badgeA = g_streakData.getBadgeInfo(a.first);
            auto* badgeB = g_streakData.getBadgeInfo(b.first);
            if (badgeA && badgeB) {
                return static_cast<int>(badgeA->category) < static_cast<int>(badgeB->category);
            }
            return false;
            });

        const int cols = 4;
        const float itemSize = 60.f;
        const float spacingX = 80.f;
        const float spacingY = 65.f;
        const CCPoint startPos = {
            m_itemMenu->getContentSize().width / 2 - (spacingX * 1.5f),
            m_itemMenu->getContentSize().height - 45.f
        };

        for (size_t i = 0; i < sortedItems.size(); ++i) {
            int row = i / cols;
            int col = i % cols;

            std::string badgeID = sortedItems[i].first;
            int price = sortedItems[i].second;
            auto* badge = g_streakData.getBadgeInfo(badgeID);

            if (!badge) continue;

            auto itemNode = CCNode::create();
            itemNode->setContentSize({ itemSize, itemSize });

            auto badgeSprite = CCSprite::create(badge->spriteName.c_str());
            badgeSprite->setScale(0.22f);
            badgeSprite->setPosition({ itemSize / 2, itemSize / 2 + 5.f });
            itemNode->addChild(badgeSprite);

            auto priceLabel = CCLabelBMFont::create(std::to_string(price).c_str(), "goldFont.fnt");
            priceLabel->setScale(0.35f);
            itemNode->addChild(priceLabel);

            auto priceTicketIcon = CCSprite::create("star_tiket.png"_spr);
            priceTicketIcon->setScale(0.15f);
            itemNode->addChild(priceTicketIcon);

            float totalWidth = priceLabel->getScaledContentSize().width + priceTicketIcon->getScaledContentSize().width + 3.f;
            priceLabel->setPosition({ itemSize / 2 - totalWidth / 2 + priceLabel->getScaledContentSize().width / 2, 8.f });
            priceTicketIcon->setPosition({ itemSize / 2 + totalWidth / 2 - priceTicketIcon->getScaledContentSize().width / 2, 8.f });

            CCMenuItemSpriteExtra* buyBtn = CCMenuItemSpriteExtra::create(
                itemNode, this, menu_selector(ShopPopup::onBuyItem)
            );
            buyBtn->setUserObject(CCString::create(badge->badgeID));
            buyBtn->setPosition({ startPos.x + col * spacingX, startPos.y - row * spacingY });
            m_itemMenu->addChild(buyBtn);

            if (g_streakData.isBadgeUnlocked(badge->badgeID)) {
                buyBtn->setEnabled(false);
                for (auto* child : CCArrayExt<CCNode*>(itemNode->getChildren())) {
                    if (auto* rgbaNode = dynamic_cast<CCRGBAProtocol*>(child)) {
                        rgbaNode->setOpacity(100);
                    }
                }
                auto checkmark = CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
                checkmark->setScale(0.4f);
                checkmark->setPosition(badgeSprite->getPosition());
                itemNode->addChild(checkmark, 2);
            }
        }
    }

    void onBuyItem(CCObject* sender) {
        std::string badgeID = static_cast<CCString*>(static_cast<CCNode*>(sender)->getUserObject())->getCString();
        auto badgeInfo = g_streakData.getBadgeInfo(badgeID);
        if (!badgeInfo) return;

        int price = m_shopItems[badgeID];

        if (g_streakData.starTickets < price) {
            FLAlertLayer::create("Not Enough Tickets", "You don't have enough tickets to buy this item.", "OK")->show();
            return;
        }

        createQuickPopup(
            "Confirm Purchase",
            fmt::format("Do you want to buy <cg>{}</c> for <cr>{}</c> tickets?", badgeInfo->displayName, price),
            "Cancel", "Buy",
            [this, badgeID, price](FLAlertLayer*, bool btn2) {
                if (btn2) {
                    g_streakData.load();
                    g_streakData.starTickets -= price;
                    g_streakData.unlockBadge(badgeID);
                    g_streakData.save();

                    // --- SONIDO DE COMPRA AÑADIDO AQUÍ ---
                    FMODAudioEngine::sharedEngine()->playEffect("buy_obj.mp3"_spr);
                    // ------------------------------------

                    FLAlertLayer::create("Purchase Successful", "You have unlocked a new badge!", "OK")->show();
                    m_ticketCounterLabel->setString(std::to_string(g_streakData.starTickets).c_str());
                    updateItems();
                }
            }
        );
    }

public:
    static ShopPopup* create() {
        auto ret = new ShopPopup();
        if (ret && ret->initAnchored(420.f, 280.f)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};



 class ShopPopup; // Declaración anticipada para la tienda

 // ============= POPUP DE LA RULETA (COMPLETO Y ACTUALIZADO) =============
 class RoulettePopup : public Popup<> {
 protected:
     CCNode* m_rouletteNode;
     CCSprite* m_selectorSprite;
     CCMenuItemSpriteExtra* m_spinBtn;
     CCMenuItemSpriteExtra* m_spin10Btn;
     bool m_isSpinning = false;
     std::vector<CCNode*> m_orderedSlots;
     std::vector<RoulettePrize> m_roulettePrizes;
     int m_currentSelectorIndex = 0;
     int m_totalSteps = 0;
     std::vector<CCSprite*> m_mythicSprites;
     std::vector<ccColor3B> m_mythicColors;
     int m_colorIndex = 0;
     float m_colorTransitionTime = 0.0f;
     ccColor3B m_currentColor;
     ccColor3B m_targetColor;
     float m_slotSize = 0.f;
     std::vector<GenericPrizeResult> m_multiSpinResults;
     std::vector<StreakData::BadgeInfo> m_pendingMythics;

     // Funciones auxiliares para reemplazar CCTargetedAction
     void runPopAnimation(CCObject* sender) {
         if (auto node = dynamic_cast<CCNode*>(sender)) {
             auto popAction = CCSequence::create(
                 CCScaleTo::create(0.025f, 1.1f),
                 CCScaleTo::create(0.025f, 1.0f),
                 nullptr
             );
             node->runAction(popAction);
         }
     }

     void runSlotAnimation(CCObject* sender) {
         if (auto node = dynamic_cast<CCNode*>(sender)) {
             auto popAction = CCSequence::create(
                 CCScaleTo::create(0.02f, 1.1f),
                 CCScaleTo::create(0.02f, 1.0f),
                 nullptr
             );
             node->runAction(popAction);
         }
     }

     bool setup() override {
         this->setTitle("Roulette");
         auto winSize = m_mainLayer->getContentSize();
         g_streakData.load();
         m_currentSelectorIndex = g_streakData.lastRouletteIndex;
         m_mythicColors = {
             ccc3(255, 0, 0), ccc3(255, 165, 0), ccc3(255, 255, 0), ccc3(0, 255, 0),
             ccc3(0, 0, 255), ccc3(75, 0, 130), ccc3(238, 130, 238)
         };
         m_currentColor = m_mythicColors[0];
         m_targetColor = m_mythicColors[1];
         m_rouletteNode = CCNode::create();
         m_rouletteNode->setPosition({ winSize.width / 2, winSize.height / 2 + 10.f });
         m_mainLayer->addChild(m_rouletteNode);
         m_roulettePrizes = {
             { RewardType::Badge, "super_star_badge", 1, "", "First Mythic", 1, StreakData::BadgeCategory::MYTHIC },
             { RewardType::Badge, "gold_streak_badge", 1, "", "Gold Legend's", 3, StreakData::BadgeCategory::LEGENDARY },
             { RewardType::Badge, "dark_streak_badge", 1, "", "dark side", 5, StreakData::BadgeCategory::EPIC },
             { RewardType::Badge, "ncs_badge", 1, "", "NCS lover", 10, StreakData::BadgeCategory::SPECIAL },
             { RewardType::Badge, "beta_badge", 1, "", "Player beta?", 70, StreakData::BadgeCategory::COMMON },
             { RewardType::Badge, "platino_streak_badge", 1, "", "platino badge", 70, StreakData::BadgeCategory::COMMON },
             { RewardType::StarTicket, "star_tiket_1", 1, "star_tiket.png"_spr, "1 star tiket", 70, StreakData::BadgeCategory::COMMON },
             { RewardType::StarTicket, "star_tiket_3", 3, "star_tiket.png"_spr, "3 star tiket", 70, StreakData::BadgeCategory::COMMON },
             { RewardType::StarTicket, "star_ticket_15", 15, "star_tiket.png"_spr, "15 Tickets", 70, StreakData::BadgeCategory::COMMON },
             { RewardType::StarTicket, "star_ticket_30", 30, "star_tiket.png"_spr, "30 Tickets", 10, StreakData::BadgeCategory::SPECIAL },
             { RewardType::StarTicket, "star_ticket_60", 60, "star_tiket.png"_spr, "60 Tickets", 5, StreakData::BadgeCategory::EPIC },
             { RewardType::StarTicket, "star_ticket_100", 100, "star_tiket.png"_spr, "100 Tickets", 3, StreakData::BadgeCategory::LEGENDARY }
         };

         const int gridSize = 4;
         m_slotSize = 38.f;
         const float spacing = 5.f;
         const float totalSize = (gridSize * m_slotSize) + ((gridSize - 1) * spacing);
         const float startPos = -totalSize / 2.f + m_slotSize / 2.f;
         std::vector<CCPoint> slotPositions;
         for (int i = 0; i < gridSize; ++i) slotPositions.push_back({ startPos + i * (m_slotSize + spacing), startPos + (gridSize - 1) * (m_slotSize + spacing) });
         for (int i = gridSize - 2; i > 0; --i) slotPositions.push_back({ startPos + (gridSize - 1) * (m_slotSize + spacing), startPos + i * (m_slotSize + spacing) });
         for (int i = gridSize - 1; i >= 0; --i) slotPositions.push_back({ startPos + i * (m_slotSize + spacing), startPos });
         for (int i = 1; i < gridSize - 1; ++i) slotPositions.push_back({ startPos, startPos + i * (m_slotSize + spacing) });
         for (size_t i = 0; i < slotPositions.size(); ++i) {
             if (i >= m_roulettePrizes.size()) continue;
             auto& currentPrize = m_roulettePrizes[i];
             auto slotContainer = CCNode::create();
             slotContainer->setPosition(slotPositions[i]);
             m_rouletteNode->addChild(slotContainer);
             m_orderedSlots.push_back(slotContainer);
             auto slotBG = cocos2d::extension::CCScale9Sprite::create("square02_001.png");
             slotBG->setContentSize({ m_slotSize, m_slotSize });
             slotBG->setColor({ 0, 0, 0 });
             slotBG->setOpacity(150);
             slotContainer->addChild(slotBG);
             auto qualitySprite = CCSprite::create(getQualitySpriteName(currentPrize.category).c_str());
             qualitySprite->setScale((m_slotSize - 4.f) / qualitySprite->getContentSize().width);
             slotContainer->addChild(qualitySprite, 1);
             if (currentPrize.category == StreakData::BadgeCategory::MYTHIC) m_mythicSprites.push_back(qualitySprite);
             if (currentPrize.type == RewardType::Badge) {
                 auto* badgeInfo = g_streakData.getBadgeInfo(currentPrize.id);
                 if (badgeInfo) {
                     auto rewardIcon = CCSprite::create(badgeInfo->spriteName.c_str());
                     rewardIcon->setScale(0.15f);
                     slotContainer->addChild(rewardIcon, 2);
                     if (g_streakData.isBadgeUnlocked(badgeInfo->badgeID)) {
                         auto claimedIcon = CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
                         claimedIcon->setScale(0.5f);
                         claimedIcon->setPosition({ m_slotSize / 2 - 5, -m_slotSize / 2 + 5 });
                         claimedIcon->setTag(199);
                         slotContainer->addChild(claimedIcon, 3);
                     }
                 }
             }
             else {
                 auto rewardIcon = CCSprite::create(currentPrize.spriteName.c_str());
                 rewardIcon->setScale(0.2f);
                 slotContainer->addChild(rewardIcon, 2);
                 auto quantityLabel = CCLabelBMFont::create(CCString::createWithFormat("x%d", currentPrize.quantity)->getCString(), "goldFont.fnt");
                 quantityLabel->setScale(0.3f);
                 quantityLabel->setPosition(0, -m_slotSize / 2 + 5);
                 slotContainer->addChild(quantityLabel, 3);
             }
         }
         m_selectorSprite = CCSprite::create("casilla_selector.png"_spr);
         m_selectorSprite->setScale(m_slotSize / m_selectorSprite->getContentSize().width);
         if (!m_orderedSlots.empty()) m_selectorSprite->setPosition(m_orderedSlots[m_currentSelectorIndex]->getPosition());
         m_rouletteNode->addChild(m_selectorSprite, 4);

         // Botones y contadores
         m_spinBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("Spin"), this, menu_selector(RoulettePopup::onSpin));
         m_spin10Btn = CCMenuItemSpriteExtra::create(ButtonSprite::create("x10"), this, menu_selector(RoulettePopup::onSpinMultiple));
         auto spinMenu = CCMenu::create();
         spinMenu->addChild(m_spinBtn);
         spinMenu->addChild(m_spin10Btn);
         spinMenu->alignItemsHorizontallyWithPadding(10.f);
         spinMenu->setPosition({ winSize.width / 2, 30.f });
         m_mainLayer->addChild(spinMenu);

         auto shopSprite = CCSprite::create("shop_btn.png"_spr);
         shopSprite->setScale(0.7f);
         auto shopBtn = CCMenuItemSpriteExtra::create(shopSprite, this, menu_selector(RoulettePopup::onOpenShop));
         auto shopMenu = CCMenu::createWithItem(shopBtn);
         shopMenu->setPosition({ winSize.width - 35.f, 30.f });
         m_mainLayer->addChild(shopMenu);

         auto superStarCounterNode = CCNode::create();
         auto haveAmountLabel = CCLabelBMFont::create(std::to_string(g_streakData.superStars).c_str(), "goldFont.fnt");
         haveAmountLabel->setScale(0.4f);
         haveAmountLabel->setAnchorPoint({ 1.f, 0.5f });
         haveAmountLabel->setID("roulette-super-star-label");
         superStarCounterNode->addChild(haveAmountLabel);
         auto haveStar = CCSprite::create("super_star.png"_spr);
         haveStar->setScale(0.12f);
         superStarCounterNode->addChild(haveStar);
         haveAmountLabel->setPosition({ -haveStar->getScaledContentSize().width / 2 - 2, 0 });
         haveStar->setPosition({ haveAmountLabel->getScaledContentSize().width / 2, 0 });
         superStarCounterNode->setPosition({ winSize.width - 35.f, winSize.height - 25.f });
         m_mainLayer->addChild(superStarCounterNode);

         auto infoIcon = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
         infoIcon->setScale(0.8f);
         auto infoBtn = CCMenuItemSpriteExtra::create(infoIcon, this, menu_selector(RoulettePopup::onShowProbabilities));
         auto infoMenu = CCMenu::createWithItem(infoBtn);
         infoMenu->setPosition({ 25.f, winSize.height - 25.f });
         m_mainLayer->addChild(infoMenu);

         this->scheduleUpdate();
         return true;
     }

     void update(float dt) override {
         if (m_mythicSprites.empty()) return;
         m_colorTransitionTime += dt;
         float transitionDuration = 1.0f;
         if (m_colorTransitionTime >= transitionDuration) {
             m_colorTransitionTime = 0.0f;
             m_colorIndex = (m_colorIndex + 1) % m_mythicColors.size();
             m_currentColor = m_targetColor;
             m_targetColor = m_mythicColors[(m_colorIndex + 1) % m_mythicColors.size()];
         }
         float progress = m_colorTransitionTime / transitionDuration;
         ccColor3B interpolatedColor = {
             static_cast<GLubyte>(m_currentColor.r + (m_targetColor.r - m_currentColor.r) * progress),
             static_cast<GLubyte>(m_currentColor.g + (m_targetColor.g - m_currentColor.g) * progress),
             static_cast<GLubyte>(m_currentColor.b + (m_targetColor.b - m_currentColor.b) * progress)
         };
         for (auto* sprite : m_mythicSprites) sprite->setColor(interpolatedColor);
     }

     void onSpin(CCObject*) {
         g_streakData.load();
         if (g_streakData.superStars < 1) {
             FLAlertLayer::create("Not Enough Super Stars", "You need at least <cy>1 Super Star</c> to spin.", "OK")->show();
             return;
         }
         if (m_isSpinning) return;
         m_isSpinning = true;
         m_spinBtn->setEnabled(false);
         m_spin10Btn->setEnabled(false);
         g_streakData.superStars -= 1;
         g_streakData.totalSpins += 1;
         g_streakData.save();
         auto starCountLabel = static_cast<CCLabelBMFont*>(m_mainLayer->getChildByIDRecursive("roulette-super-star-label"));
         if (starCountLabel) starCountLabel->setString(std::to_string(g_streakData.superStars).c_str());
         auto spinCountLabel = static_cast<CCLabelBMFont*>(m_mainLayer->getChildByIDRecursive("roulette-spin-count-label"));
         if (spinCountLabel) spinCountLabel->setString(CCString::createWithFormat("Spins: %d", g_streakData.totalSpins)->getCString());
         int totalWeight = 0;
         for (const auto& prize : m_roulettePrizes) totalWeight += prize.probabilityWeight;
         int randomValue = rand() % totalWeight;
         int winningIndex = 0;
         for (size_t i = 0; i < m_roulettePrizes.size(); ++i) {
             if (randomValue < m_roulettePrizes[i].probabilityWeight) {
                 winningIndex = i;
                 break;
             }
             randomValue -= m_roulettePrizes[i].probabilityWeight;
         }
         int totalSlots = m_orderedSlots.size();
         int laps = 5;
         int distance = (winningIndex - m_currentSelectorIndex + totalSlots) % totalSlots;
         m_totalSteps = (laps * totalSlots) + distance;
         auto actions = CCArray::create();
         for (int i = 1; i <= m_totalSteps; ++i) {
             int stepIndex = (m_currentSelectorIndex + i) % totalSlots;
             auto targetSlot = m_orderedSlots[stepIndex];
             float duration = 0.05f;
             if (i < 5) duration = 0.2f - (i * 0.03f);
             else if (i > m_totalSteps - 10) duration = 0.05f + ((i - (m_totalSteps - 10)) * 0.04f);
             auto moveAction = CCEaseSineInOut::create(CCMoveTo::create(duration, targetSlot->getPosition()));
             auto soundAction = CCCallFunc::create(this, callfunc_selector(RoulettePopup::playTickSound));
             auto slotCall = CCCallFuncO::create(this, callfuncO_selector(RoulettePopup::runPopAnimation), targetSlot);
             actions->addObject(CCSpawn::create(moveAction, soundAction, slotCall, nullptr));
         }
         actions->addObject(CCCallFunc::create(this, callfunc_selector(RoulettePopup::onSpinEnd)));
         m_selectorSprite->runAction(CCSequence::create(actions));
     }

     void onSpinMultiple(CCObject*) {
         g_streakData.load();
         if (g_streakData.superStars < 10) {
             FLAlertLayer::create("Not Enough Super Stars", "You need at least <cy>10 Super Stars</c> to spin x10.", "OK")->show();
             return;
         }
         if (m_isSpinning) return;
         m_isSpinning = true;
         m_spinBtn->setEnabled(false);
         m_spin10Btn->setEnabled(false);
         g_streakData.superStars -= 10;
         g_streakData.totalSpins += 10;
         std::vector<GenericPrizeResult> allPrizes;
         int totalTicketsWon = 0;
         m_pendingMythics.clear();
         std::vector<int> winningIndices;
         int totalWeight = 0;
         for (const auto& prize : m_roulettePrizes) totalWeight += prize.probabilityWeight;
         for (int i = 0; i < 10; ++i) {
             int randomValue = rand() % totalWeight;
             int winningIndex = 0;
             for (size_t j = 0; j < m_roulettePrizes.size(); ++j) {
                 if (randomValue < m_roulettePrizes[j].probabilityWeight) {
                     winningIndex = j;
                     break;
                 }
                 randomValue -= m_roulettePrizes[j].probabilityWeight;
             }
             winningIndices.push_back(winningIndex);
             auto& prize = m_roulettePrizes[winningIndex];
             GenericPrizeResult result;
             result.type = prize.type;
             result.id = prize.id;
             result.quantity = prize.quantity;
             result.displayName = prize.displayName;
             result.category = prize.category;
             switch (prize.type) {
             case RewardType::Badge: {
                 auto* badgeInfo = g_streakData.getBadgeInfo(prize.id);
                 if (badgeInfo) {
                     result.spriteName = badgeInfo->spriteName;
                     result.isNew = !g_streakData.isBadgeUnlocked(prize.id);
                     if (result.isNew) {
                         g_streakData.unlockBadge(prize.id);
                         if (badgeInfo->category == StreakData::BadgeCategory::MYTHIC) {
                             m_pendingMythics.push_back(*badgeInfo);
                         }
                     }
                     else {
                         int tickets = g_streakData.getTicketValueForRarity(badgeInfo->category);
                         g_streakData.starTickets += tickets;
                         totalTicketsWon += tickets;
                         result.ticketsFromDuplicate = tickets;
                     }
                 }
                 break;
             }
             case RewardType::SuperStar: {
                 g_streakData.superStars += prize.quantity;
                 result.spriteName = prize.spriteName;
                 break;
             }
             case RewardType::StarTicket: {
                 g_streakData.starTickets += prize.quantity;
                 totalTicketsWon += prize.quantity;
                 result.spriteName = prize.spriteName;
                 break;
             }
             }
             allPrizes.push_back(result);
         }
         m_multiSpinResults = allPrizes;
         g_streakData.save();
         if (auto starCountLabel = static_cast<CCLabelBMFont*>(m_mainLayer->getChildByIDRecursive("roulette-super-star-label"))) {
             starCountLabel->setString(std::to_string(g_streakData.superStars).c_str());
         }
         if (auto spinCountLabel = static_cast<CCLabelBMFont*>(m_mainLayer->getChildByIDRecursive("roulette-spin-count-label"))) {
             spinCountLabel->setString(CCString::createWithFormat("Spins: %d", g_streakData.totalSpins)->getCString());
         }
         auto actions = CCArray::create();
         int totalSlots = m_orderedSlots.size();
         int lastIndex = m_currentSelectorIndex;
         for (int prizeIndex : winningIndices) {
             int distance = (prizeIndex - lastIndex + totalSlots) % totalSlots;
             if (distance == 0) distance = totalSlots;
             for (int i = 1; i <= distance; ++i) {
                 int stepIndex = (lastIndex + i) % totalSlots;
                 auto targetSlot = m_orderedSlots[stepIndex];
                 float moveDuration = 0.04f;
                 auto moveAction = CCMoveTo::create(moveDuration, targetSlot->getPosition());
                 auto soundAction = CCCallFunc::create(this, callfunc_selector(RoulettePopup::playTickSound));
                 auto slotCall = CCCallFuncO::create(this, callfuncO_selector(RoulettePopup::runSlotAnimation), targetSlot);
                 actions->addObject(CCSpawn::create(moveAction, soundAction, slotCall, nullptr));
             }
             actions->addObject(CCDelayTime::create(0.25f));
             actions->addObject(CCCallFuncO::create(this, callfuncO_selector(RoulettePopup::flashWinningSlot), CCInteger::create(prizeIndex)));
             lastIndex = prizeIndex;
         }
         m_currentSelectorIndex = lastIndex;
         actions->addObject(CCCallFunc::create(this, callfunc_selector(RoulettePopup::onMultiSpinEnd)));
         m_selectorSprite->runAction(CCSequence::create(actions));
     }

     void flashWinningSlot(CCObject* pSender) {
         int slotIndex = static_cast<CCInteger*>(pSender)->getValue();
         if (static_cast<size_t>(slotIndex) >= m_orderedSlots.size()) return;
         auto targetSlot = m_orderedSlots[slotIndex];
         auto glow = CCSprite::create("cuadro.png"_spr);
         if (!glow) return;
         glow->setPosition(targetSlot->getContentSize() / 2);
         glow->setScale(m_slotSize / glow->getContentSize().width);
         glow->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });
         glow->setColor({ 255, 255, 150 });
         glow->setOpacity(0);
         glow->runAction(CCSequence::create(
             CCFadeTo::create(0.2f, 200),
             CCFadeTo::create(0.3f, 0),
             CCRemoveSelf::create(),
             nullptr
         ));
         targetSlot->addChild(glow, 5);
     }

     void processMythicQueue() {
         if (!m_pendingMythics.empty()) {
             auto mythicBadge = m_pendingMythics.front();
             m_pendingMythics.erase(m_pendingMythics.begin());
             auto animLayer = MythicAnimationLayer::create(mythicBadge, [this]() {
                 this->processMythicQueue();
                 });
             CCDirector::sharedDirector()->getRunningScene()->addChild(animLayer, 400);
         }
         else {
             MultiPrizePopup::create(m_multiSpinResults, [this]() {
                 if (m_spinBtn && m_spin10Btn) {
                     m_spinBtn->setEnabled(true);
                     m_spin10Btn->setEnabled(true);
                 }
                 })->show();
         }
     }

     void onMultiSpinEnd() {
         m_isSpinning = false;
         updateAllCheckmarks();
         g_streakData.lastRouletteIndex = m_currentSelectorIndex;
         g_streakData.save();
         m_pendingMythics.clear();
         int totalTicketsWonInSpin = 0;
         for (const auto& result : m_multiSpinResults) {
             if (result.isNew && result.type == RewardType::Badge && result.category == StreakData::BadgeCategory::MYTHIC) {
                 auto* badgeInfo = g_streakData.getBadgeInfo(result.id);
                 if (badgeInfo) {
                     m_pendingMythics.push_back(*badgeInfo);
                 }
             }
             if (result.type == RewardType::StarTicket) {
                 totalTicketsWonInSpin += result.quantity;
             }
             else if (result.type == RewardType::Badge && !result.isNew) {
                 totalTicketsWonInSpin += result.ticketsFromDuplicate;
             }
         }
         if (!m_pendingMythics.empty()) {
             processMythicQueue();
         }
         else {
             MultiPrizePopup::create(m_multiSpinResults, [this, totalTicketsWonInSpin]() {
                 m_spinBtn->setEnabled(true);
                 m_spin10Btn->setEnabled(true);
                 if (totalTicketsWonInSpin > 0) {
                     CCDirector::sharedDirector()->getRunningScene()->addChild(TicketAnimationLayer::create(totalTicketsWonInSpin), 1000);
                 }
                 })->show();
         }
     }

     void updateAllCheckmarks() {
         for (size_t i = 0; i < m_orderedSlots.size(); ++i) {
             if (i >= m_roulettePrizes.size()) continue;
             auto& prize = m_roulettePrizes[i];
             if (prize.type != RewardType::Badge) continue;
             auto slotNode = m_orderedSlots[i];
             if (g_streakData.isBadgeUnlocked(prize.id) && !slotNode->getChildByTag(199)) {
                 auto claimedIcon = CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
                 claimedIcon->setScale(0.5f);
                 claimedIcon->setPosition({ m_slotSize / 2 - 5, -m_slotSize / 2 + 5 });
                 claimedIcon->setTag(199);
                 slotNode->addChild(claimedIcon, 3);
             }
         }
     }

     void playTickSound() {
         FMODAudioEngine::sharedEngine()->playEffect("ruleta_sfx.mp3"_spr);
     }

     void onSpinEnd() {
         m_isSpinning = false;
         m_spinBtn->setEnabled(true);
         m_spin10Btn->setEnabled(true);
         int winningIndex = (m_currentSelectorIndex + m_totalSteps) % m_orderedSlots.size();
         m_currentSelectorIndex = winningIndex;
         g_streakData.lastRouletteIndex = m_currentSelectorIndex;
         int ticketsWon = 0;
         if (static_cast<size_t>(m_currentSelectorIndex) < m_roulettePrizes.size()) {
             auto& prize = m_roulettePrizes[m_currentSelectorIndex];
             GenericPrizeResult result;
             result.type = prize.type;
             result.id = prize.id;
             result.quantity = prize.quantity;
             result.displayName = prize.displayName;
             result.category = prize.category;
             switch (prize.type) {
             case RewardType::Badge: {
                 auto* badgeInfo = g_streakData.getBadgeInfo(prize.id);
                 if (badgeInfo) {
                     result.spriteName = badgeInfo->spriteName;
                     result.isNew = !g_streakData.isBadgeUnlocked(prize.id);
                     if (result.isNew) {
                         g_streakData.unlockBadge(prize.id);
                     }
                     else {
                         ticketsWon = g_streakData.getTicketValueForRarity(badgeInfo->category);
                         g_streakData.starTickets += ticketsWon;
                         result.ticketsFromDuplicate = ticketsWon;
                     }
                 }
                 break;
             }
             case RewardType::SuperStar: {
                 g_streakData.superStars += prize.quantity;
                 result.spriteName = prize.spriteName;
                 break;
             }
             case RewardType::StarTicket: {
                 ticketsWon = prize.quantity;
                 g_streakData.starTickets += ticketsWon;
                 result.spriteName = prize.spriteName;
                 break;
             }
             }
             GenericPrizePopup::create(result, [ticketsWon]() {
                 if (ticketsWon > 0) {
                     CCDirector::sharedDirector()->getRunningScene()->addChild(TicketAnimationLayer::create(ticketsWon), 1000);
                 }
                 })->show();
         }
         g_streakData.save();
         updateAllCheckmarks();
     }

     void onOpenShop(CCObject*) {
         ShopPopup::create()->show();
     }

     void onShowProbabilities(CCObject*) {
         std::map<StreakData::BadgeCategory, int> categoryWeights;
         int totalWeight = 0;
         for (const auto& prize : m_roulettePrizes) {
             categoryWeights[prize.category] += prize.probabilityWeight;
             totalWeight += prize.probabilityWeight;
         }

         std::string message = "";
         auto getColorForCategory = [](StreakData::BadgeCategory cat) -> std::string {
             switch (cat) {
             case StreakData::BadgeCategory::MYTHIC:    return "<cr>";
             case StreakData::BadgeCategory::LEGENDARY: return "<co>";
             case StreakData::BadgeCategory::EPIC:      return "<cp>";
             case StreakData::BadgeCategory::SPECIAL:   return "<cg>";
             default:                                   return "<cy>";
             }
             };

         for (const auto& [category, weight] : categoryWeights) {
             float percentage = (static_cast<float>(weight) / totalWeight) * 100.f;
             message += fmt::format("{}{}:</c> {:.2f}%\n",
                 getColorForCategory(category),
                 g_streakData.getCategoryName(category),
                 percentage
             );
         }

         message += fmt::format("\n<cp>Total Spins:</c> {}", g_streakData.totalSpins);
         FLAlertLayer::create("Probabilities", message, "OK")->show();
     }

     std::string getQualitySpriteName(StreakData::BadgeCategory category) {
         switch (category) {
         case StreakData::BadgeCategory::SPECIAL:   return "casilla_especial.png"_spr;
         case StreakData::BadgeCategory::EPIC:      return "casilla_epica.png"_spr;
         case StreakData::BadgeCategory::LEGENDARY: return "casilla_legendaria.png"_spr;
         case StreakData::BadgeCategory::MYTHIC:    return "casilla_mitica.png"_spr;
         default:                                   return "casilla_comun.png"_spr;
         }
     }

 public:
     static RoulettePopup* create() {
         auto ret = new RoulettePopup();
         if (ret && ret->initAnchored(260.f, 260.f)) {
             ret->autorelease();
             return ret;
         }
         CC_SAFE_DELETE(ret);
         return nullptr;
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

// ============= POPUP DE PROGRESO MÚLTIPLE (CORREGIDO) =============
class DayProgressPopup : public Popup<> {
protected:
    // Guardaremos una lista filtrada solo con las insignias de racha
    std::vector<StreakData::BadgeInfo> m_streakBadges;

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

        // --- LÓGICA DE FILTRADO ---
        // Llenamos nuestra lista solo con las insignias que no son de la ruleta
        g_streakData.load();
        for (const auto& badge : g_streakData.badges) {
            if (!badge.isFromRoulette) {
                m_streakBadges.push_back(badge);
            }
        }

        // El resto del setup no cambia...
        auto leftArrow = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        leftArrow->setScale(0.8f);
        auto leftBtn = CCMenuItemSpriteExtra::create(leftArrow, this, menu_selector(DayProgressPopup::onPreviousGoal));
        leftBtn->setPosition({ -winSize.width / 2 + 30, 0 });

        auto rightArrow = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        rightArrow->setScale(0.8f);
        rightArrow->setFlipX(true);
        auto rightBtn = CCMenuItemSpriteExtra::create(rightArrow, this, menu_selector(DayProgressPopup::onNextGoal));
        rightBtn->setPosition({ winSize.width / 2 - 30, 0 });

        auto arrowMenu = CCMenu::create();
        arrowMenu->addChild(leftBtn);
        arrowMenu->addChild(rightBtn);
        arrowMenu->setPosition({ winSize.width / 2, winSize.height / 2 });
        m_mainLayer->addChild(arrowMenu);

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
        if (m_streakBadges.empty()) return; // Prevenir crasheo si no hay insignias de racha

        auto winSize = m_mainLayer->getContentSize();
        g_streakData.load();

        // Ahora usamos nuestra lista filtrada
        if (m_currentGoalIndex >= m_streakBadges.size()) {
            m_currentGoalIndex = 0;
        }

        auto& badge = m_streakBadges[m_currentGoalIndex];

        int currentDays;
        if (g_streakData.isBadgeUnlocked(badge.badgeID)) {
            currentDays = badge.daysRequired;
        }
        else {
            currentDays = std::min(g_streakData.currentStreak, badge.daysRequired);
        }

        float percent = 0.f;
        if (badge.daysRequired > 0) { // Evitar división por cero
            percent = currentDays / static_cast<float>(badge.daysRequired);
        }

        m_titleLabel->setString(CCString::createWithFormat("Progress to %d Days", badge.daysRequired)->getCString());

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

        if (m_rewardSprite) {
            m_rewardSprite->removeFromParent();
            m_rewardSprite = nullptr;
        }

        m_rewardSprite = CCSprite::create(badge.spriteName.c_str());
        if (m_rewardSprite) {
            m_rewardSprite->setScale(0.25f);
            m_rewardSprite->setPosition({ winSize.width / 2, winSize.height / 2 + 30 });
            m_mainLayer->addChild(m_rewardSprite, 5);
        }

        m_dayText->setString(
            CCString::createWithFormat("Day %d / %d", currentDays, badge.daysRequired)->getCString()
        );
    }

    void onNextGoal(CCObject*) {
        if (m_streakBadges.empty()) return;
        m_currentGoalIndex = (m_currentGoalIndex + 1) % m_streakBadges.size();
        updateDisplay();
    }

    void onPreviousGoal(CCObject*) {
        if (m_streakBadges.empty()) return;
        m_currentGoalIndex = (m_currentGoalIndex - 1 + m_streakBadges.size()) % m_streakBadges.size();
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



// ============= POPUP DE MISIONES =============
class MissionsPopup : public Popup<> {
protected:
    CCNode* m_counterNode;
    // Crea el nodo visual para una misión con el diseño final
    CCNode* createStarMissionNode(int missionID) {
        std::string title;
        int targetStars, reward;
        bool isClaimed;

        switch (missionID) {
        case 0: title = "Star Collector"; targetStars = 15; reward = 2; isClaimed = g_streakData.starMission1Claimed; break;
        case 1: title = "Star Gatherer"; targetStars = 30; reward = 3; isClaimed = g_streakData.starMission2Claimed; break;
        case 2: title = "Star Master"; targetStars = 45; reward = 5; isClaimed = g_streakData.starMission3Claimed; break;
        default: return CCNode::create();
        }
        bool isComplete = g_streakData.starsToday >= targetStars;

        auto container = cocos2d::extension::CCScale9Sprite::create("GJ_square01.png");
        container->setContentSize({ 250.f, 45.f });

        auto missionIcon = CCSprite::createWithSpriteFrameName("GJ_starsIcon_001.png");
        missionIcon->setScale(0.7f);
        missionIcon->setPosition({ 20.f, 28.f });
        container->addChild(missionIcon);

        auto descLabel = CCLabelBMFont::create(CCString::createWithFormat("Collect %d Stars", targetStars)->getCString(), "goldFont.fnt");
        descLabel->setScale(0.45f);
        descLabel->setAnchorPoint({ 0, 0.5f });
        descLabel->setPosition({ 40.f, 28.f });
        container->addChild(descLabel);

        float barWidth = 120.f;
        float barHeight = 8.f;
        CCPoint barPosition = { descLabel->getPositionX(), descLabel->getPositionY() - 20.f };

        auto barBg = CCLayerColor::create({ 0, 0, 0, 120 });
        barBg->setContentSize({ barWidth, barHeight });
        barBg->setPosition(barPosition);
        container->addChild(barBg);

        float progressPercent = std::min(1.f, static_cast<float>(g_streakData.starsToday) / targetStars);
        if (progressPercent > 0.f) {
            auto barFill = CCLayerColor::create({ 120, 255, 120, 255 });
            barFill->setContentSize({ barWidth * progressPercent, barHeight });
            barFill->setPosition(barPosition);
            container->addChild(barFill);
        }

        auto progressLabel = CCLabelBMFont::create(CCString::createWithFormat("%d/%d", std::min(g_streakData.starsToday, targetStars), targetStars)->getCString(), "bigFont.fnt");
        progressLabel->setScale(0.4f);
        progressLabel->setPosition(barPosition + CCPoint(barWidth / 2, barHeight / 2));
        container->addChild(progressLabel);

        if (isClaimed) {
            container->setOpacity(100);
            auto claimedLabel = CCLabelBMFont::create("CLAIMED", "goldFont.fnt");
            claimedLabel->setScale(0.7f);
            claimedLabel->setPosition(container->getContentSize() / 2);
            container->addChild(claimedLabel);
        }
        else if (isComplete) {
            auto claimBtnSprite = ButtonSprite::create("Claim");
            claimBtnSprite->setScale(0.7f);
            auto claimBtn = CCMenuItemSpriteExtra::create(claimBtnSprite, this, menu_selector(MissionsPopup::onClaimReward));
            claimBtn->setTag(missionID);
            auto menu = CCMenu::createWithItem(claimBtn);
            menu->setPosition({ 215.f, 22.5f });
            container->addChild(menu);
        }
        else {
            auto rewardSprite = CCSprite::create("super_star.png"_spr);
            rewardSprite->setScale(0.2f);
            rewardSprite->setPosition({ 205.f, 22.5f });
            container->addChild(rewardSprite);

            auto rewardLabel = CCLabelBMFont::create(CCString::createWithFormat("x%d", reward)->getCString(), "bigFont.fnt");
            rewardLabel->setScale(0.5f);
            rewardLabel->setAnchorPoint({ 0, 0.5f });
            rewardLabel->setPosition({ 218.f, 22.5f });
            container->addChild(rewardLabel);
        }

        return container;
    }

    bool setup() override {
        this->setTitle("Missions");
        auto winSize = m_mainLayer->getContentSize();
        g_streakData.load();

        auto background = cocos2d::extension::CCScale9Sprite::create("square02_001.png");
        background->setColor({ 0, 0, 0 });
        background->setOpacity(120);
        background->setContentSize({ 280.f, 160.f });
        background->setPosition({ winSize.width / 2, winSize.height / 2 - 15.f });
        m_mainLayer->addChild(background);

        auto menu = CCMenu::create();
        menu->setContentSize(background->getContentSize());
        menu->setPosition(background->getPosition());
        menu->setID("missions-menu");
        m_mainLayer->addChild(menu);

        if (!g_streakData.starMission1Claimed) {
            auto mission = createStarMissionNode(0);
            mission->setTag(0);
            menu->addChild(mission);
        }
        if (!g_streakData.starMission2Claimed) {
            auto mission = createStarMissionNode(1);
            mission->setTag(1);
            menu->addChild(mission);
        }
        if (!g_streakData.starMission3Claimed) {
            auto mission = createStarMissionNode(2);
            mission->setTag(2);
            menu->addChild(mission);
        }

        if (menu->getChildrenCount() == 0) {
            auto label = CCLabelBMFont::create("No more missions today.\nReturn tomorrow!", "goldFont.fnt");
            label->setPosition(menu->getPosition());
            label->setScale(0.7f);
            m_mainLayer->addChild(label);
        }
        else {
            menu->alignItemsVerticallyWithPadding(3.f);
        }

        // --- Contador de súper estrellas (AÑADIMOS IDs) ---
       // --- Contador de súper estrellas (SIN FONDO Y AJUSTADO) ---
        auto starSprite = CCSprite::create("super_star.png"_spr);
        starSprite->setScale(0.18f);
        starSprite->setID("super-star-icon");
        starSprite->setAnchorPoint({ 0.f, 0.5f }); // Ancla a la izquierda
        m_mainLayer->addChild(starSprite);

        auto countLabel = CCLabelBMFont::create(std::to_string(g_streakData.superStars).c_str(), "goldFont.fnt");
        countLabel->setScale(0.5f);
        countLabel->setID("super-star-label");
        countLabel->setAnchorPoint({ 0.f, 0.5f }); // Ancla a la izquierda
        m_mainLayer->addChild(countLabel);

        // Posicionar el contador en la esquina superior izquierda
        CCPoint const counterPos = { 25.f, winSize.height - 25.f };
        starSprite->setPosition(counterPos);
        countLabel->setPosition(starSprite->getPosition() + CCPoint{ starSprite->getScaledContentSize().width + 5.f, 0 }); // 5px de espacio

        return true;
    }

    // --- LÓGICA DE ANIMACIÓN SIN RECARGAR EL POPUP ---
    void onClaimReward(CCObject* sender) {
        int missionID = sender->getTag();
        g_streakData.load();

        switch (missionID) {
        case 0: g_streakData.superStars += 2; g_streakData.starMission1Claimed = true; break;
        case 1: g_streakData.superStars += 3; g_streakData.starMission2Claimed = true; break;
        case 2: g_streakData.superStars += 5; g_streakData.starMission3Claimed = true; break;
        }
        g_streakData.save();

        // --- LÓGICA DE ACTUALIZACIÓN DEL CONTADOR CORREGIDA ---
        auto countLabel = static_cast<CCLabelBMFont*>(this->m_mainLayer->getChildByIDRecursive("super-star-label"));
        auto starSprite = static_cast<CCSprite*>(this->m_mainLayer->getChildByIDRecursive("super-star-icon"));

        // Ya no busca el fondo "bg"
        if (countLabel && starSprite) {
            // 1. Actualizamos el texto del contador
            countLabel->setString(std::to_string(g_streakData.superStars).c_str());

            // 2. Reposicionamos la etiqueta a la derecha del icono para que se ajuste si el número cambia
            countLabel->setPosition(starSprite->getPosition() + CCPoint{ starSprite->getScaledContentSize().width + 5.f, 0 });
        }
        // --------------------------------------------------------

        // La animación de las misiones se queda igual
        auto menu = static_cast<CCMenu*>(m_mainLayer->getChildByID("missions-menu"));
        if (!menu) return;

        menu->setTouchEnabled(false);

        auto claimedNode = static_cast<CCNode*>(menu->getChildByTag(missionID));
        if (!claimedNode) return;

        auto slideAction = CCEaseSineIn::create(CCMoveBy::create(0.3f, { 400, 0 }));
        auto fadeAction = CCFadeOut::create(0.3f);
        auto sequence = CCSequence::create(
            CCSpawn::create(slideAction, fadeAction, nullptr),
            CCCallFuncN::create(this, callfuncN_selector(MissionsPopup::onClaimAnimationFinished)),
            nullptr
        );
        claimedNode->runAction(sequence);

        float slotHeight = claimedNode->getContentSize().height + 3.f;
        CCObject* child;
        CCARRAY_FOREACH(menu->getChildren(), child) {
            auto node = static_cast<CCNode*>(child);
            if (node->getTag() > missionID) {
                node->runAction(CCEaseSineInOut::create(CCMoveBy::create(0.3f, { 0, slotHeight })));
            }
        }

        this->runAction(CCSequence::create(
            CCDelayTime::create(0.4f),
            CCCallFunc::create(this, callfunc_selector(MissionsPopup::checkIfMissionsAreDone)),
            nullptr
        ));
    }

    void onClaimAnimationFinished(CCNode* sender) {
        sender->removeFromParentAndCleanup(true);
    }

    void checkIfMissionsAreDone() {
        auto menu = static_cast<CCMenu*>(m_mainLayer->getChildByID("missions-menu"));
        if (menu && menu->getChildrenCount() == 0) {
            auto label = CCLabelBMFont::create("No more missions today.\nReturn tomorrow!", "goldFont.fnt");
            label->setPosition(menu->getPosition());
            label->setScale(0.f);
            m_mainLayer->addChild(label);
            label->runAction(CCEaseBackOut::create(CCScaleTo::create(0.4f, 0.7f)));
        }
        else if (menu) {
            menu->setTouchEnabled(true);
        }
    }

public:
    static MissionsPopup* create() {
        auto ret = new MissionsPopup();
        if (ret && ret->initAnchored(320.f, 240.f)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};



// ============= POPUP DE RECOMPENSAS (AJUSTE FINAL DE TAMAÑO) =============
class RewardsPopup : public Popup<> {
protected:
    int m_currentCategory = 0;
    int m_currentPage = 0;
    int m_totalPages = 0;

    CCLabelBMFont* m_categoryLabel = nullptr;
    CCNode* m_badgeContainer = nullptr;
    CCMenuItemSpriteExtra* m_pageLeftArrow = nullptr;
    CCMenuItemSpriteExtra* m_pageRightArrow = nullptr;
    CCLabelBMFont* m_counterText = nullptr;

    // Variables para la animación del título mítico
    int m_colorIndex = 0;
    float m_colorTransitionTime = 0.0f;
    std::vector<ccColor3B> m_mythicColors;
    ccColor3B m_currentColor;
    ccColor3B m_targetColor;

    bool setup() override {
        this->setTitle("Badges Collection");
        auto winSize = m_mainLayer->getContentSize();
        g_streakData.load();

        m_mythicColors = {
            ccc3(255, 0, 0), ccc3(255, 165, 0), ccc3(255, 255, 0),
            ccc3(0, 255, 0), ccc3(0, 0, 255), ccc3(75, 0, 130),
            ccc3(238, 130, 238), ccc3(255, 105, 180), ccc3(255, 215, 0),
            ccc3(192, 192, 192)
        };
        m_colorIndex = 0;
        m_currentColor = m_mythicColors[0];
        m_targetColor = m_mythicColors[1];
        m_colorTransitionTime = 0.0f;

        // Fondo negro
        auto background = cocos2d::extension::CCScale9Sprite::create("square02_001.png");
        background->setColor({ 0, 0, 0 });
        background->setOpacity(120);
        background->setContentSize({ 320.f, 150.f });
        background->setPosition({ winSize.width / 2, winSize.height / 2 - 15.f }); // Un poco más abajo
        m_mainLayer->addChild(background);

        // --- Flechas de categoría (más pegadas al texto) ---
        auto catLeftArrow = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        auto catLeftBtn = CCMenuItemSpriteExtra::create(catLeftArrow, this, menu_selector(RewardsPopup::onPreviousCategory));
        catLeftBtn->setPosition(-110.f, 0); // Más cerca del centro

        auto catRightArrow = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        catRightArrow->setFlipX(true);
        auto catRightBtn = CCMenuItemSpriteExtra::create(catRightArrow, this, menu_selector(RewardsPopup::onNextCategory));
        catRightBtn->setPosition(110.f, 0); // Más cerca del centro

        auto catArrowMenu = CCMenu::create();
        catArrowMenu->addChild(catLeftBtn);
        catArrowMenu->addChild(catRightBtn);
        catArrowMenu->setPosition({ winSize.width / 2, winSize.height - 40.f });
        m_mainLayer->addChild(catArrowMenu);

        // Etiqueta de la categoría
        m_categoryLabel = CCLabelBMFont::create("", "goldFont.fnt");
        m_categoryLabel->setScale(0.7f);
        m_categoryLabel->setPosition({ winSize.width / 2, winSize.height - 40.f });
        m_mainLayer->addChild(m_categoryLabel);

        // --- Flechas de página (centradas con el fondo) ---
        auto pageLeftArrowSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
        m_pageLeftArrow = CCMenuItemSpriteExtra::create(pageLeftArrowSprite, this, menu_selector(RewardsPopup::onPreviousBadgePage));
        m_pageLeftArrow->setPosition(-background->getContentSize().width / 2 - 5.f, 0); // A la izquierda del fondo

        auto pageRightArrowSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
        pageRightArrowSprite->setFlipX(true);
        m_pageRightArrow = CCMenuItemSpriteExtra::create(pageRightArrowSprite, this, menu_selector(RewardsPopup::onNextBadgePage));
        m_pageRightArrow->setPosition(background->getContentSize().width / 2 + 5.f, 0); // A la derecha del fondo

        auto pageArrowMenu = CCMenu::create();
        pageArrowMenu->addChild(m_pageLeftArrow);
        pageArrowMenu->addChild(m_pageRightArrow);
        pageArrowMenu->setPosition(background->getPosition()); // Centrado verticalmente con el fondo
        m_mainLayer->addChild(pageArrowMenu);

        m_badgeContainer = CCNode::create();
        m_mainLayer->addChild(m_badgeContainer);

        // --- Contador de "Unlocked" (más abajo) ---
        m_counterText = CCLabelBMFont::create("", "bigFont.fnt");
        m_counterText->setScale(0.5f);
        m_counterText->setPosition({ winSize.width / 2, 32.f }); // Más abajo
        m_mainLayer->addChild(m_counterText);

        this->scheduleUpdate();
        updateCategoryDisplay();

        return true;
    }

    void update(float dt) override {
        StreakData::BadgeCategory currentCat = static_cast<StreakData::BadgeCategory>(m_currentCategory);
        if (currentCat != StreakData::BadgeCategory::MYTHIC || !m_categoryLabel) {
            m_categoryLabel->setColor(g_streakData.getCategoryColor(currentCat));
            return;
        }

        m_colorTransitionTime += dt;
        float transitionDuration = 1.0f;
        if (m_colorTransitionTime >= transitionDuration) {
            m_colorTransitionTime = 0.0f;
            m_colorIndex = (m_colorIndex + 1) % m_mythicColors.size();
            m_currentColor = m_targetColor;
            m_targetColor = m_mythicColors[(m_colorIndex + 1) % m_mythicColors.size()];
        }
        float progress = m_colorTransitionTime / transitionDuration;
        ccColor3B interpolatedColor = {
            static_cast<GLubyte>(m_currentColor.r + (m_targetColor.r - m_currentColor.r) * progress),
            static_cast<GLubyte>(m_currentColor.g + (m_targetColor.g - m_currentColor.g) * progress),
            static_cast<GLubyte>(m_currentColor.b + (m_targetColor.b - m_currentColor.b) * progress)
        };
        m_categoryLabel->setColor(interpolatedColor);
    }

    void updateCategoryDisplay() {
        m_badgeContainer->removeAllChildren();

        StreakData::BadgeCategory currentCat = static_cast<StreakData::BadgeCategory>(m_currentCategory);
        std::vector<StreakData::BadgeInfo> categoryBadges;
        for (auto& badge : g_streakData.badges) {
            if (badge.category == currentCat) {
                categoryBadges.push_back(badge);
            }
        }

        std::string categoryName = g_streakData.getCategoryName(currentCat);
        m_categoryLabel->setString(categoryName.c_str());

        const int badgesPerPage = 6;
        m_totalPages = static_cast<int>(ceil(static_cast<float>(categoryBadges.size()) / badgesPerPage));

        if (m_currentPage >= m_totalPages && m_totalPages > 0) {
            m_currentPage = m_totalPages - 1;
        }

        int startIndex = m_currentPage * badgesPerPage;
        int endIndex = std::min(static_cast<int>(categoryBadges.size()), startIndex + badgesPerPage);

        const int badgesPerRow = 3;
        const float horizontalSpacing = 65.f;
        const float verticalSpacing = 65.f;
        auto winSize = m_mainLayer->getContentSize();
        // Posición inicial ajustada para el popup más pequeño
        const CCPoint startPosition = { winSize.width / 2 - horizontalSpacing, winSize.height / 2 + 10.f };

        for (int i = startIndex; i < endIndex; ++i) {
            int indexOnPage = i - startIndex;
            int row = indexOnPage / badgesPerRow;
            int col = indexOnPage % badgesPerRow;
            auto& badge = categoryBadges[i];

            auto badgeSprite = CCSprite::create(badge.spriteName.c_str());
            badgeSprite->setScale(0.3f);

            bool unlocked = g_streakData.isBadgeUnlocked(badge.badgeID);
            if (!unlocked) {
                badgeSprite->setColor({ 100, 100, 100 });
            }

            CCMenuItemSpriteExtra* badgeBtn;
            if (unlocked) {
                badgeBtn = CCMenuItemSpriteExtra::create(badgeSprite, this, menu_selector(RewardsPopup::onBadgeClick));
                badgeBtn->setUserObject(CCString::create(badge.badgeID));
            }
            else {
                badgeBtn = CCMenuItemSpriteExtra::create(badgeSprite, this, nullptr);
                badgeBtn->setEnabled(false);
            }

            CCPoint position = { startPosition.x + col * horizontalSpacing, startPosition.y - row * verticalSpacing };

            auto badgeMenu = CCMenu::createWithItem(badgeBtn);
            badgeMenu->setPosition(position);
            m_badgeContainer->addChild(badgeMenu);

            if (!badge.isFromRoulette) {
                auto daysLabel = CCLabelBMFont::create(CCString::createWithFormat("%d days", badge.daysRequired)->getCString(), "goldFont.fnt");
                daysLabel->setScale(0.35f);
                daysLabel->setPosition(position - CCPoint(0, 30.f));
                m_badgeContainer->addChild(daysLabel);
            }

            if (!unlocked) {
                auto lockIcon = CCSprite::createWithSpriteFrameName("GJ_lock_001.png");
                lockIcon->setPosition(position);
                m_badgeContainer->addChild(lockIcon, 5);
            }
        }

        m_pageLeftArrow->setVisible(m_currentPage > 0);
        m_pageRightArrow->setVisible(m_currentPage < m_totalPages - 1);

        int unlockedCount = 0;
        for (auto& badge : categoryBadges) {
            if (g_streakData.isBadgeUnlocked(badge.badgeID)) {
                unlockedCount++;
            }
        }
        m_counterText->setString(CCString::createWithFormat("Unlocked: %d/%d", unlockedCount, categoryBadges.size())->getCString());
    }

    void onBadgeClick(CCObject* sender) {
        auto badgeID = static_cast<CCString*>(static_cast<CCNode*>(sender)->getUserObject())->getCString();
        EquipBadgePopup::create(badgeID)->show();
    }

    void onNextBadgePage(CCObject*) {
        if (m_currentPage < m_totalPages - 1) {
            m_currentPage++;
            updateCategoryDisplay();
        }
    }

    void onPreviousBadgePage(CCObject*) {
        if (m_currentPage > 0) {
            m_currentPage--;
            updateCategoryDisplay();
        }
    }

    void onNextCategory(CCObject*) {
        m_currentCategory = (m_currentCategory + 1) % 5;
        m_currentPage = 0;
        updateCategoryDisplay();
    }

    void onPreviousCategory(CCObject*) {
        m_currentCategory = (m_currentCategory - 1 + 5) % 5;
        m_currentPage = 0;
        updateCategoryDisplay();
    }

    void onClose(CCObject* sender) override {
        this->unscheduleUpdate();
        Popup::onClose(sender);
    }

public:
    static RewardsPopup* create() {
        auto ret = new RewardsPopup();
        if (ret && ret->initAnchored(360.f, 250.f)) { // Popup más compacto
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
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





class InfoPopup : public Popup<> {
protected:
    bool setup() override {
        g_streakData.dailyUpdate();


        this->setTitle("My Streak");
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
        float percent = 0.f;
        if (requiredStars > 0) {
            percent = std::min(static_cast<float>(g_streakData.starsToday) / requiredStars, 1.0f);
        }

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

        auto missionsIcon = CCSprite::create("super_star_btn.png"_spr);
        if (missionsIcon) {
            missionsIcon->setScale(0.7f);
            auto missionsBtn = CCMenuItemSpriteExtra::create(missionsIcon, this, menu_selector(InfoPopup::onOpenMissions));
            auto missionsMenu = CCMenu::create();
            missionsMenu->addChild(missionsBtn);
            missionsMenu->setPosition({ 22, centerY });
            m_mainLayer->addChild(missionsMenu, 10);
        }

        auto rouletteIcon = CCSprite::create("boton_ruleta.png"_spr);
        if (rouletteIcon) {
            rouletteIcon->setScale(0.7f);
            auto rouletteBtn = CCMenuItemSpriteExtra::create(rouletteIcon, this, menu_selector(InfoPopup::onOpenRoulette));
            auto rouletteMenu = CCMenu::create();
            rouletteMenu->addChild(rouletteBtn);
            rouletteMenu->setPosition({ 22, centerY - 37 });
            m_mainLayer->addChild(rouletteMenu, 10);
        }

        auto infoIcon = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
        infoIcon->setScale(0.6f);
        auto infoBtn = CCMenuItemSpriteExtra::create(infoIcon, this, menu_selector(InfoPopup::onInfo));
        auto menu = CCMenu::create();
        menu->setPosition({ winSize.width - 20, winSize.height - 20 });
        menu->addChild(infoBtn);
        m_mainLayer->addChild(menu, 10);

        // --- NUEVO BOTÓN DE HISTORIAL ---
        auto historyIcon = CCSprite::create("historial_btn.png"_spr);
        historyIcon->setScale(0.7f);
        auto historyBtn = CCMenuItemSpriteExtra::create(historyIcon, this, menu_selector(InfoPopup::onOpenHistory));
        auto historyMenu = CCMenu::create();
        historyMenu->addChild(historyBtn);
        historyMenu->setPosition({ 20, 20 });
        m_mainLayer->addChild(historyMenu, 10);
        // -------------------------------

        if (g_streakData.shouldShowAnimation()) {
            this->showStreakAnimation(g_streakData.currentStreak);
        }
        return true;
    }

    void onOpenHistory(CCObject*) {
        HistoryPopup::create()->show();
    }

    void onOpenStats(CCObject*) { DayProgressPopup::create()->show(); }
    void onOpenRewards(CCObject*) { RewardsPopup::create()->show(); }
    void onRachaClick(CCObject*) { AllRachasPopup::create()->show(); }
    void onOpenMissions(CCObject*) { MissionsPopup::create()->show(); }

    void onOpenRoulette(CCObject*) {
        g_streakData.load();
        if (g_streakData.currentStreak < 1) { // Puedes ajustar este requerimiento si quieres
            FLAlertLayer::create("Roulette Locked", "You need a streak of at least <cg>1 day</c> to use the roulette.", "OK")->show();
            return;
        }
        RoulettePopup::create()->show();
    }

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

        // --- MODIFICACIÓN PARA MÓVILES ---
        // Solo creamos la estela de partículas en plataformas que no son móviles
#if !defined(GEODE_IS_MOBILE)
        auto trail = ManualParticleEmitter::create();
        trail->setPosition(rachaSprite->getPosition());
        trail->setTag(5);
        animLayer->addChild(trail, 1);
#endif

        float entranceDuration = 0.8f;
        auto entranceMove = CCEaseElasticOut::create(CCMoveTo::create(entranceDuration, { winSize.width / 2, winSize.height / 2 }), 0.5f);
        auto entranceScale = CCEaseBackOut::create(CCScaleTo::create(entranceDuration, 1.2f));
        auto entranceRotate = CCEaseExponentialOut::create(CCRotateTo::create(entranceDuration, 0.f));

        auto auraMove = CCMoveTo::create(entranceDuration, { winSize.width / 2, winSize.height / 2 });

        // --- MODIFICACIÓN PARA MÓVILES ---
#if !defined(GEODE_IS_MOBILE)
        auto trailMove = CCMoveTo::create(entranceDuration, { winSize.width / 2, winSize.height / 2 });
#endif

        auto shineMove = CCMoveTo::create(entranceDuration, { winSize.width / 2, winSize.height / 2 });

        aura->runAction(auraMove);

        // --- MODIFICACIÓN PARA MÓVILES ---
#if !defined(GEODE_IS_MOBILE)
        if (auto trail = static_cast<ManualParticleEmitter*>(animLayer->getChildByTag(5))) {
            trail->runAction(CCSequence::create(
                trailMove,
                CCCallFunc::create(trail, callfunc_selector(ManualParticleEmitter::stopEmitting)),
                nullptr
            ));
        }
#endif

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

        // --- MODIFICACIÓN PARA MÓVILES ---
        // La explosión de partículas solo ocurre en PC
#if !defined(GEODE_IS_MOBILE)
        animLayer->runAction(CCSequence::create(
            CCDelayTime::create(1.0f),
            CCCallFunc::create(this, callfunc_selector(InfoPopup::spawnStarBurst)),
            CCDelayTime::create(0.5f),
            CCCallFunc::create(this, callfunc_selector(InfoPopup::spawnStarBurst)),
            CCDelayTime::create(0.5f),
            CCCallFunc::create(this, callfunc_selector(InfoPopup::spawnStarBurst)),
            CCDelayTime::create(2.6f),
            CCCallFunc::create(this, callfunc_selector(InfoPopup::onAnimationExit)),
            nullptr
        ));
#else
// En móvil, la animación es más corta porque no hay partículas
        animLayer->runAction(CCSequence::create(
            CCDelayTime::create(3.0f), // Tiempo de espera para poder ver la animación
            CCCallFunc::create(this, callfunc_selector(InfoPopup::onAnimationExit)),
            nullptr
        ));
#endif
    }

    // --- MODIFICACIÓN PARA MÓVILES ---
    // Esta función ahora solo se compilará en plataformas que no son móviles
#if !defined(GEODE_IS_MOBILE)
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
#endif

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

        // --- MODIFICACIÓN PARA MÓVILES ---
#if !defined(GEODE_IS_MOBILE)
        if (auto trail = static_cast<ManualParticleEmitter*>(animLayer->getChildByTag(5))) {
            trail->stopEmitting();
        }
#endif

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

class $modify(MyMenuLayer, MenuLayer) {
    struct Fields {
        CCSprite* m_alertSprite = nullptr;
    };

    bool init() {
        if (!MenuLayer::init()) return false;

        g_streakData.load();
        g_streakData.dailyUpdate();

        // --- LÓGICA DE REINICIO CORREGIDA ---
        int totalStars = GameStatsManager::sharedState()->getStat("6");

        if (totalStars == 0 && g_streakData.currentStreak > 0) {
            // CORRECCIÓN: Ahora reiniciamos AMBAS cosas
            g_streakData.currentStreak = 0;
            g_streakData.starsToday = 0; // <-- AÑADIDO: Resetea las estrellas del día
            g_streakData.save();

            // Y nos aseguramos de que el aviso correcto aparezca
            FLAlertLayer::create("Streak Reset", "Your stats were reset, so your streak has been reset too.", "OK")->show();
        }
        // ------------------------------------

        // El resto del código para crear el botón de racha no cambia
        auto menu = this->getChildByID("bottom-menu");
        if (!menu) return false;

        auto spriteName = g_streakData.getRachaSprite();
        auto icon = CCSprite::create(spriteName.c_str());
        icon->setScale(0.5f);

        auto circle = CircleButtonSprite::create(icon, CircleBaseColor::Green, CircleBaseSize::Medium);
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

        auto btn = CCMenuItemSpriteExtra::create(circle, this, menu_selector(MyMenuLayer::onOpenPopup));
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

        auto winSize = cocos2d::CCDirector::sharedDirector()->getWinSize();

        m_barWidth = 160.0f;
        m_barHeight = 15.0f;

        m_barContainer = CCNode::create();
        m_barContainer->setPosition(30, winSize.height - 280);
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
        m_starLabel->setPosition({ m_barFg->getContentSize().width - 5, m_barHeight / 2 });
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

            // ANIMACIÓN ORIGINAL RESTAURADA - Usando curva de Bezier manual
            ccBezierConfig bezier;
            bezier.endPosition = endPos;
            float explosionRadius = 150.f;
            float randomAngle = (float)(rand() % 360);
            bezier.controlPoint_1 = center + CCPoint((explosionRadius + (rand() % 50)) * cos(CC_DEGREES_TO_RADIANS(randomAngle)), (explosionRadius + (rand() % 50)) * sin(CC_DEGREES_TO_RADIANS(randomAngle)));
            bezier.controlPoint_2 = endPos + CCPoint(0, 100);

            // Para Windows, necesitamos una implementación alternativa de Bezier
            // ya que CCBezierTo/CCBezierBy pueden no estar disponibles
            std::vector<CCPoint> bezierPoints;
            int segments = 20;
            for (int j = 0; j <= segments; j++) {
                float t = (float)j / segments;
                float u = 1 - t;
                float tt = t * t;
                float uu = u * u;
                float ut2 = 2 * u * t;

                CCPoint point = center * uu + bezier.controlPoint_1 * ut2 + bezier.endPosition * tt;
                bezierPoints.push_back(point);
            }

            CCArray* bezierActions = CCArray::create();
            for (size_t j = 1; j < bezierPoints.size(); j++) {
                bezierActions->addObject(CCMoveTo::create(1.0f / segments, bezierPoints[j]));
            }

            auto rotateAction = CCRotateBy::create(1.0f, 360 + (rand() % 180));
            auto scaleAction = CCScaleTo::create(1.0f, 0.4f);

            // SOLUCIÓN COMPATIBLE: Usar CCCallFuncO para pasar el índice como objeto
            auto starIndexObj = CCInteger::create(i + 1);
            starParticle->runAction(CCSequence::create(
                CCDelayTime::create(i * delayPerStar),
                CCSpawn::create(
                    CCSequence::create(bezierActions),
                    rotateAction,
                    scaleAction,
                    nullptr
                ),
                CCCallFuncO::create(this, callfuncO_selector(StreakProgressBar::onStarHitBar), starIndexObj),
                CCRemoveSelf::create(),
                nullptr
            ));
        }
    }

    // Función callback que recibe el índice como objeto
    void onStarHitBar(CCObject* starIndexObj) {
        CCInteger* indexInt = static_cast<CCInteger*>(starIndexObj);
        int starIndex = indexInt->getValue();

        int currentTotalStars = m_starsBefore + starIndex;

        m_targetPercent = std::min(1.f, static_cast<float>(currentTotalStars) / m_starsRequired);
        m_targetStarsDisplay = static_cast<float>(currentTotalStars);

        auto popUp = CCEaseSineOut::create(CCScaleTo::create(0.1f, 1.0f, 1.2f));
        auto popDown = CCEaseSineIn::create(CCScaleTo::create(0.1f, 1.0f, 1.0f));
        m_barFg->runAction(CCSequence::create(popUp, popDown, nullptr));

        // CORRECCIÓN: Calcular la posición correcta del flash
        // La posición X es el ancho actual de la barra (donde está el progreso)
        // La posición Y es la mitad de la altura de la barra
        float currentBarWidth = m_barWidth * m_currentPercent;
        CCPoint flashPosition = m_barContainer->getPosition() +
            CCPoint(3 + currentBarWidth, 3 + m_barHeight / 2);

        auto flash = CCSprite::createWithSpriteFrameName("GJ_bigStar_001.png");
        flash->setPosition(flashPosition);  // Posición corregida
        flash->setScale(0.1f);
        flash->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });
        flash->runAction(CCSequence::create(
            CCSpawn::create(
                CCScaleTo::create(0.3f, 1.0f),
                CCFadeOut::create(0.3f),
                nullptr
            ),
            CCRemoveSelf::create(),
            nullptr
        ));
        this->addChild(flash, 20);  // Añadir al layer principal, no al contenedor de la barra

        FMODAudioEngine::sharedEngine()->playEffect("coin.mp3"_spr);
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

            int starsNow = g_streakData.starsToday;
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

        if (geode::Mod::get()->getSettingValue<bool>("show-in-pause")) {
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