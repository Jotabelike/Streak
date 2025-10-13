#include <Geode/modify/MenuLayer.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/loader/Event.hpp>
#include <Geode/binding/GJAccountManager.hpp>
#include <matjson.hpp>
#include "StreakData.h"

using namespace geode::prelude;


void updatePlayerDataInFirebase() {
    log::info("Iniciando actualización de datos en Firebase...");
    auto accountManager = GJAccountManager::sharedState();


    if (accountManager->m_accountID == 0) {
        log::warn("El jugador no ha iniciado sesión. Se cancela la actualización de datos.");
        return;
    }


    g_streakData.load();
    std::string equippedBadgeId = "";
    bool hasMythicEquipped = false;
    if (auto* equippedBadge = g_streakData.getEquippedBadge()) {
        equippedBadgeId = equippedBadge->badgeID;
        if (equippedBadge->category == StreakData::BadgeCategory::MYTHIC) {
            hasMythicEquipped = true;
        }
    }


    matjson::Value playerData = matjson::Value::object();
    playerData.set("username", accountManager->m_username);
    playerData.set("accountID", accountManager->m_accountID);
    playerData.set("equipped_badge_id", equippedBadgeId);
    playerData.set("total_streak_points", g_streakData.totalStreakPoints);
    playerData.set("current_streak", g_streakData.currentStreak);
    playerData.set("has_mythic_color", hasMythicEquipped);

    std::string url = fmt::format(
        "https://streak-44a83-default-rtdb.firebaseio.com/players/{}.json",
        accountManager->m_accountID
    );


    static EventListener<web::WebTask> s_updateListener;
    s_updateListener.setFilter(web::WebRequest().bodyJSON(playerData).put(url));
}


class $modify(MyFirebaseLayer, MenuLayer) {
    bool init() {
        
        if (!MenuLayer::init()) {
            return false;
        }


        updatePlayerDataInFirebase();

        return true;
    }
};

