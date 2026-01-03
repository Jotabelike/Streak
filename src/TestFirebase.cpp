#include "FirebaseManager.h"
#include <Geode/utils/web.hpp>
#include <matjson.hpp>
#include <Geode/binding/GJAccountManager.hpp>
#include "StreakData.h"
#include <Geode/loader/Event.hpp>
#include <string>
#include <vector>
#include <map>
#include <Geode/binding/GameManager.hpp>
#include "RewardNotification.h"

using namespace geode::prelude;


static EventListener<web::WebTask> s_updateListener;
static EventListener<web::WebTask> s_loadListener;
static EventListener<web::WebTask> s_completeLevelListener;

void loadPlayerDataFromServer() {
    auto am = GJAccountManager::sharedState();
    if (!am || am->m_accountID == 0) {
        g_streakData.resetToDefault();
        g_streakData.isDataLoaded = true;
        g_streakData.m_initialized = true;
        return;
    }

    int accountID = am->m_accountID;
    std::string url = fmt::format("https://streak-servidor.onrender.com/players/{}", accountID);
    log::info("Requesting data from the server...");

    s_loadListener.bind([accountID](web::WebTask::Event* e) {
        if (web::WebResponse* res = e->getValue()) {
            if (res->ok() && res->json().isOk()) {
                g_streakData.parseServerResponse(res->json().unwrap());
                g_streakData.isDataLoaded = true;
                g_streakData.m_initialized = true;
                log::info("Data received and processed.");
            }
            else if (res->code() == 404) {
                log::info("New user (404). Registration required.");
                g_streakData.resetToDefault();
                g_streakData.needsRegistration = true;
                g_streakData.isDataLoaded = true;
                g_streakData.m_initialized = true;
            }
            else {
                log::warn("Load failed (Code: {}). We maintain error state.", res->code());
                g_streakData.isDataLoaded = false;
                g_streakData.m_initialized = false;
            }
        }
        else if (e->isCancelled()) {
            log::warn("Loading canceled.");
            g_streakData.isDataLoaded = false;
            g_streakData.m_initialized = false;
        }
        });

    auto req = web::WebRequest();
    s_loadListener.setFilter(req.get(url));
}

void updatePlayerDataInFirebase() {
    auto accountManager = GJAccountManager::sharedState();
    if (!accountManager || accountManager->m_accountID == 0) {
        log::error("Save canceled: Not logged in.");
        return;
    }

    int accountID = accountManager->m_accountID;
    int userID = GameManager::sharedState()->m_playerUserID;
    matjson::Value playerData = matjson::Value::object();

    // --- DATOS BÁSICOS Y COSMÉTICOS (El cliente puede guardarlos) ---
    playerData.set("username", std::string(accountManager->m_username));
    playerData.set("accountID", accountID);
    playerData.set("userID", userID);
    playerData.set("equipped_badge_id", g_streakData.equippedBadge);
    playerData.set("equipped_banner_id", g_streakData.equippedBanner);
    playerData.set("last_streak_animated", g_streakData.lastStreakAnimated); // Estado visual local
    playerData.set("super_stars", g_streakData.superStars);
    playerData.set("star_tickets", g_streakData.starTickets);
    playerData.set("gems", g_streakData.gems);
   

    // --- ESTADOS DE MINIJUEGOS/MISIONES (Se pueden mantener si la lógica es local) ---
    playerData.set("gem_roulette_spin_count", g_streakData.gemRouletteSpinCount);
    playerData.set("gem_roulette_hash", g_streakData.gemRouletteHash);
    playerData.set("last_roulette_index", g_streakData.lastRouletteIndex);
    playerData.set("total_spins", g_streakData.totalSpins);
    playerData.set("last_day", g_streakData.lastDay); // Necesario para sincronizar fecha local

    // Estado de la ruleta de gemas
    std::vector<bool> gemStateVec = g_streakData.gemRouletteState;
    if (gemStateVec.size() < 7) gemStateVec.resize(7, false);
    playerData.set("gem_roulette_state", gemStateVec);

    // Estado de Tareas
    matjson::Value tasksJson = matjson::Value::object();
    for (auto const& [id, status] : g_streakData.taskStatuses) {
        tasksJson.set(id, status);
    }
    playerData.set("taskStatuses", tasksJson);

    // Badges desbloqueados (Solo IDs para ahorrar espacio y errores)
    std::vector<std::string> unlocked_badges_vec;
    if (g_streakData.unlockedBadges.size() == g_streakData.badges.size()) {
        for (size_t i = 0; i < g_streakData.badges.size(); ++i) {
            if (i < g_streakData.unlockedBadges.size() && g_streakData.unlockedBadges[i]) {
                unlocked_badges_vec.push_back(g_streakData.badges[i].badgeID);
            }
        }
    }
    playerData.set("unlocked_badges", unlocked_badges_vec);

    // Banners desbloqueados
    std::vector<std::string> unlocked_banners_vec;
    if (g_streakData.unlockedBanners.size() == g_streakData.banners.size()) {
        for (size_t i = 0; i < g_streakData.banners.size(); ++i) {
            if (i < g_streakData.unlockedBanners.size() && g_streakData.unlockedBanners[i]) {
                unlocked_banners_vec.push_back(g_streakData.banners[i].bannerID);
            }
        }
    }
    playerData.set("unlocked_banners", unlocked_banners_vec);

    // Misiones diarias (Checkboxes)
    matjson::Value missions_obj = matjson::Value::object();
    missions_obj.set("pm1", g_streakData.pointMission1Claimed);
    missions_obj.set("pm2", g_streakData.pointMission2Claimed);
    missions_obj.set("pm3", g_streakData.pointMission3Claimed);
    missions_obj.set("pm4", g_streakData.pointMission4Claimed);
    missions_obj.set("pm5", g_streakData.pointMission5Claimed);
    missions_obj.set("pm6", g_streakData.pointMission6Claimed);
    playerData.set("missions", missions_obj);

    // Historial (Solo lectura para el server, escritura local)
    matjson::Value history_obj = matjson::Value::object();
    for (const auto& pair : g_streakData.streakPointsHistory) {
        history_obj.set(pair.first, pair.second);
    }
    playerData.set("history", history_obj);

    // Color Mítico
    bool hasMythicEquipped = false;
    if (!g_streakData.equippedBadge.empty()) {
        if (auto* badgeInfo = g_streakData.getBadgeInfo(g_streakData.equippedBadge)) {
            if (badgeInfo->category == StreakData::BadgeCategory::MYTHIC) hasMythicEquipped = true;
        }
    }
    playerData.set("has_mythic_color", hasMythicEquipped);

    // Niveles completados
    matjson::Value completed_levels_obj = matjson::Value::object();
    for (int levelID : g_streakData.completedLevelMissions) {
        completed_levels_obj.set(std::to_string(levelID), true);
    }
    playerData.set("completedLevelMissions", completed_levels_obj);

    // --- ENVÍO AL SERVIDOR ---
    std::string url = fmt::format("https://streak-servidor.onrender.com/players/{}", accountID);

    s_updateListener.bind([](web::WebTask::Event* e) {
        if (web::WebResponse* res = e->getValue()) {
            if (!res->ok()) {
                log::error("SERVER ERROR SAVING: {}", res->code());
            }
        }
        });

    auto req = web::WebRequest();
    s_updateListener.setFilter(req.bodyJSON(playerData).post(url));
}


void completeLevelInFirebase(int stars) {
    auto am = GJAccountManager::sharedState();
    if (!am || am->m_accountID == 0) return;

    int accountID = am->m_accountID;
    std::string url = fmt::format("https://streak-servidor.onrender.com/players/{}/complete-level", accountID);

    matjson::Value payload = matjson::Value::object();
    payload.set("stars", stars);
    
    // CORRECCIÓN 1: Enviar fecha local del cliente para evitar conflictos de zona horaria (10 -> 1 bug)
    payload.set("clientDate", g_streakData.getCurrentDate());

    log::info("Sending completed level to the server...");

    s_completeLevelListener.bind([](web::WebTask::Event* e) {
        if (web::WebResponse* res = e->getValue()) {
            if (res->ok() && res->json().isOk()) {
                auto data = res->json().unwrap();

                // Sincronización Básica
                if (data.contains("current_xp")) g_streakData.currentXP = data["current_xp"].as<int>().unwrapOr(g_streakData.currentXP);
                if (data.contains("current_level")) g_streakData.currentLevel = data["current_level"].as<int>().unwrapOr(g_streakData.currentLevel);
                if (data.contains("super_stars")) g_streakData.superStars = data["super_stars"].as<int>().unwrapOr(g_streakData.superStars);
                if (data.contains("star_tickets")) g_streakData.starTickets = data["star_tickets"].as<int>().unwrapOr(g_streakData.starTickets);
                if (data.contains("current_streak_days")) g_streakData.currentStreak = data["current_streak_days"].as<int>().unwrapOr(g_streakData.currentStreak);
                if (data.contains("gems")) g_streakData.gems = data["gems"].as<int>().unwrapOr(g_streakData.gems);
                // CORRECCIÓN 2: Sincronización visual obligatoria (Barra de progreso y Racha)
                // Esto permite que la barra se actualice correctamente sin llamar a save() extra.
                if (data.contains("streakPointsToday")) g_streakData.streakPointsToday = data["streakPointsToday"].as<int>().unwrapOr(g_streakData.streakPointsToday);
                if (data.contains("total_streak_points")) g_streakData.totalStreakPoints = data["total_streak_points"].as<int>().unwrapOr(g_streakData.totalStreakPoints);
                if (data.contains("lastDay")) g_streakData.lastDay = data["lastDay"].as<std::string>().unwrapOr(g_streakData.lastDay);

                // Manejo de Recompensas
                if (data.contains("newRewards")) {
                    auto rewards = data["newRewards"];
                    int starsGiven = rewards["stars"].as<int>().unwrapOr(0);
                    int ticketsGiven = rewards["tickets"].as<int>().unwrapOr(0);
                    int levelsGained = rewards["levels"].as<int>().unwrapOr(0);

                    if (levelsGained > 0) {
                        FMODAudioEngine::sharedEngine()->playEffect("magic_explode_01.ogg");

                        Notification::create(
                            fmt::format("LEVEL UP!\nWelcome to Level {}", g_streakData.currentLevel),
                            NotificationIcon::Success
                        )->show();

                        if (starsGiven > 0) RewardNotification::show("super_star.png"_spr, g_streakData.superStars - starsGiven, starsGiven);
                        if (ticketsGiven > 0) RewardNotification::show("star_tiket.png"_spr, g_streakData.starTickets - ticketsGiven, ticketsGiven);
                    }
                }

                g_streakData.isDataLoaded = true;
                log::info("Level completed. XP: {}, Level: {}, PointsToday: {}", g_streakData.currentXP, g_streakData.currentLevel, g_streakData.streakPointsToday);
            }
            else {
                log::error("Error completing level: {}", res->code());
            }
        }
    });

    auto req = web::WebRequest();
    s_completeLevelListener.setFilter(req.bodyJSON(payload).post(url));
}