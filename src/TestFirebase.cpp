#include "FirebaseManager.h" 
#include <Geode/utils/web.hpp>
#include <matjson.hpp>
#include <Geode/binding/GJAccountManager.hpp>
#include "StreakData.h"

void updatePlayerDataInFirebase() {
    log::info("--- STARTING DATA UPDATE ---");
    auto accountManager = GJAccountManager::sharedState();

    if (accountManager->m_accountID == 0) {
        log::warn("Player is not logged in. Update canceled.");
        return;
    }

    int accountID = accountManager->m_accountID;
    matjson::Value playerData = matjson::Value::object();

    playerData.set("username", std::string(accountManager->m_username));
    playerData.set("accountID", accountID);
    playerData.set("current_streak_days", g_streakData.currentStreak);
    playerData.set("total_streak_points", g_streakData.totalStreakPoints);
    playerData.set("equipped_badge_id", g_streakData.equippedBadge);
    playerData.set("super_stars", g_streakData.superStars);
    playerData.set("star_tickets", g_streakData.starTickets);
    playerData.set("last_roulette_index", g_streakData.lastRouletteIndex);
    playerData.set("total_spins", g_streakData.totalSpins);
    playerData.set("last_day", g_streakData.lastDay);
    playerData.set("streakPointsToday", g_streakData.streakPointsToday);

    // Asegurar que unlockedBadges esté inicializado
    g_streakData.initializeUnlockedBadges();

    std::vector<std::string> unlocked_badges_vec;
    for (size_t i = 0; i < g_streakData.badges.size(); ++i) {
        if (i < g_streakData.unlockedBadges.size() && g_streakData.unlockedBadges[i]) {
            unlocked_badges_vec.push_back(g_streakData.badges[i].badgeID);
            log::info("Including badge in update: {}", g_streakData.badges[i].badgeID);
        }
    }
    playerData.set("unlocked_badges", unlocked_badges_vec);

    log::info("Unlocked badges count: {}", unlocked_badges_vec.size());

    matjson::Value missions_obj = matjson::Value::object();
    missions_obj.set("pm1", g_streakData.pointMission1Claimed);
    missions_obj.set("pm2", g_streakData.pointMission2Claimed);
    missions_obj.set("pm3", g_streakData.pointMission3Claimed);
    missions_obj.set("pm4", g_streakData.pointMission4Claimed);
    missions_obj.set("pm5", g_streakData.pointMission5Claimed);
    missions_obj.set("pm6", g_streakData.pointMission6Claimed);
    playerData.set("missions", missions_obj);

    matjson::Value history_obj = matjson::Value::object();
    for (const auto& [date, points] : g_streakData.streakPointsHistory) {
        history_obj.set(date, points);
    }
    playerData.set("history", history_obj);

    bool hasMythicEquipped = false;
    if (auto* equippedBadge = g_streakData.getEquippedBadge()) {
        if (equippedBadge->category == StreakData::BadgeCategory::MYTHIC) {
            hasMythicEquipped = true;
        }
    }
    playerData.set("has_mythic_color", hasMythicEquipped);

    log::info("Data packaged for sending: {}", playerData.dump());

    std::string url = fmt::format(
        "https://streak-servidor.onrender.com/players/{}",
        accountID
    );

    static EventListener<web::WebTask> s_updateListener;
    s_updateListener.bind([](web::WebTask::Event* e) {
        if (web::WebResponse* res = e->getValue()) {
            if (res->ok()) {
                log::info("The data was updated successfully.");
            }
            else {
                log::error("Error communicating with the server. Code: {}", res->code());
                log::error("Server response: {}", res->string().unwrapOr("Unanswered."));
            }
        }
        else if (e->isCancelled()) {
            log::warn("The request to the server was cancelled.");
        }
        });

    auto req = web::WebRequest();
    s_updateListener.setFilter(req.bodyJSON(playerData).post(url));
}