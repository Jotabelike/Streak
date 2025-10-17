#include "StreakData.h"
#include "FirebaseManager.h" 
StreakData g_streakData;

void StreakData::resetToDefault() {
    currentStreak = 0;
    streakPointsToday = 0;
    totalStreakPoints = 0;
    hasNewStreak = false;
    lastDay = "";
    equippedBadge = "";
    superStars = 0;
    starTickets = 0;
    lastRouletteIndex = 0;
    totalSpins = 0;
    streakCompletedLevels.clear();
    streakPointsHistory.clear();
    pointMission1Claimed = false;
    pointMission2Claimed = false;
    pointMission3Claimed = false;
    pointMission4Claimed = false;
    pointMission5Claimed = false;
    pointMission6Claimed = false;

    // Inicializar unlockedBadges
    initializeUnlockedBadges();

    isDataLoaded = false;
}

void StreakData::load() {
    // Implementación existente
}

void StreakData::save() {
    updatePlayerDataInFirebase();
}

void StreakData::parseServerResponse(const matjson::Value& data) {
    currentStreak = data["current_streak_days"].as<int>().unwrapOr(0);
    totalStreakPoints = data["total_streak_points"].as<int>().unwrapOr(0);
    equippedBadge = data["equipped_badge_id"].as<std::string>().unwrapOr("");
    superStars = data["super_stars"].as<int>().unwrapOr(0);
    starTickets = data["star_tickets"].as<int>().unwrapOr(0);
    lastRouletteIndex = data["last_roulette_index"].as<int>().unwrapOr(0);
    totalSpins = data["total_spins"].as<int>().unwrapOr(0);
    lastDay = data["last_day"].as<std::string>().unwrapOr("");
    streakPointsToday = data["streakPointsToday"].as<int>().unwrapOr(0);

    // Inicializar unlockedBadges antes de procesar
    initializeUnlockedBadges();

    // Procesar insignias desbloqueadas
    if (data.contains("unlocked_badges")) {
        auto badgesResult = data["unlocked_badges"].as<std::vector<matjson::Value>>();
        if (badgesResult.isOk()) {
            for (const auto& badge_id_json : badgesResult.unwrap()) {
                std::string badgeID = badge_id_json.as<std::string>().unwrapOr("");
                if (!badgeID.empty()) {
                    unlockBadge(badgeID);
                }
            }
        }
    }

    pointMission1Claimed = false;
    pointMission2Claimed = false;
    pointMission3Claimed = false;
    pointMission4Claimed = false;
    pointMission5Claimed = false;
    pointMission6Claimed = false;
    if (data.contains("missions")) {
        auto missionsResult = data["missions"].as<std::map<std::string, matjson::Value>>();
        if (missionsResult.isOk()) {
            auto missions = missionsResult.unwrap();
            if (missions.count("pm1")) pointMission1Claimed = missions.at("pm1").as<bool>().unwrapOr(false);
            if (missions.count("pm2")) pointMission2Claimed = missions.at("pm2").as<bool>().unwrapOr(false);
            if (missions.count("pm3")) pointMission3Claimed = missions.at("pm3").as<bool>().unwrapOr(false);
            if (missions.count("pm4")) pointMission4Claimed = missions.at("pm4").as<bool>().unwrapOr(false);
            if (missions.count("pm5")) pointMission5Claimed = missions.at("pm5").as<bool>().unwrapOr(false);
            if (missions.count("pm6")) pointMission6Claimed = missions.at("pm6").as<bool>().unwrapOr(false);
        }
    }

    streakPointsHistory.clear();
    if (data.contains("history")) {
        auto historyResult = data["history"].as<std::map<std::string, matjson::Value>>();
        if (historyResult.isOk()) {
            auto historyMap = historyResult.unwrap();
            for (const auto& [date, pointsValue] : historyMap) {
                streakPointsHistory[date] = pointsValue.as<int>().unwrapOr(0);
            }
        }
        else {
            log::warn("No se pudo leer 'history' como un objeto desde el servidor.");
        }
    }
}

int StreakData::getRequiredPoints() {
    if (currentStreak >= 80) return 10;
    if (currentStreak >= 70) return 9;
    if (currentStreak >= 60) return 8;
    if (currentStreak >= 50) return 7;
    if (currentStreak >= 40) return 6;
    if (currentStreak >= 30) return 5;
    if (currentStreak >= 20) return 4;
    if (currentStreak >= 10) return 3;
    if (currentStreak >= 1)  return 2;
    return 2;
}

int StreakData::getTicketValueForRarity(BadgeCategory category) {
    switch (category) {
    case BadgeCategory::COMMON:   return 5;
    case BadgeCategory::SPECIAL:  return 20;
    case BadgeCategory::EPIC:     return 50;
    case BadgeCategory::LEGENDARY: return 100;
    case BadgeCategory::MYTHIC:   return 500;
    default:                      return 0;
    }
}

void StreakData::unlockBadge(const std::string& badgeID) {
    // Asegurarse de que unlockedBadges tenga el tamaño correcto
    if (unlockedBadges.empty() || unlockedBadges.size() != badges.size()) {
        unlockedBadges.assign(badges.size(), false);
    }

    for (size_t i = 0; i < badges.size(); ++i) {
        if (badges[i].badgeID == badgeID) {
            if (i < unlockedBadges.size()) {
                unlockedBadges[i] = true;
                log::info("Badge unlocked: {} at index {}", badgeID, i);
            }
            else {
                log::error("Index out of bounds when unlocking badge: {}", badgeID);
            }
            return;
        }
    }
    log::warn("Badge ID not found: {}", badgeID);
}

std::string StreakData::getCurrentDate() {
    time_t t = time(nullptr);
    tm* now = localtime(&t);
    char buf[16];
    strftime(buf, sizeof(buf), "%Y-%m-%d", now);
    return std::string(buf);
}

void StreakData::unequipBadge() {
    equippedBadge = "";
    save();
}

bool StreakData::isBadgeEquipped(const std::string& badgeID) {
    return equippedBadge == badgeID;
}

void StreakData::dailyUpdate() {
    time_t now_t = time(nullptr);
    std::string today = getCurrentDate();
    if (lastDay.empty()) {
        lastDay = today;
        streakPointsToday = 0;
        return;
    }
    if (lastDay == today) return;

    tm last_tm = {};
    std::stringstream ss(lastDay);
    ss >> std::get_time(&last_tm, "%Y-%m-%d");
    time_t last_t = mktime(&last_tm);
    double seconds_passed = difftime(now_t, last_t);
    double days_passed = floor(seconds_passed / (60.0 * 60.0 * 24.0));

    bool streak_should_be_lost = false;
    bool showAlert = false;

    if (days_passed >= 2.0) {
        streak_should_be_lost = true;
    }
    else if (days_passed >= 1.0) {
        if (streakPointsToday < getRequiredPoints()) {
            streak_should_be_lost = true;
        }
    }

    if (streak_should_be_lost) {
        if (currentStreak > 0) {
            currentStreak = 0;
            streakPointsHistory.clear();
            totalStreakPoints = 0;
            showAlert = true;
            updatePlayerDataInFirebase();
        }
    }

    if (days_passed >= 1.0) {
        streakPointsToday = 0;
        lastDay = today;

        pointMission1Claimed = false;
        pointMission2Claimed = false;
        pointMission3Claimed = false;
        pointMission4Claimed = false;
        pointMission5Claimed = false;
        pointMission6Claimed = false;

        if (!streak_should_be_lost) {
            updatePlayerDataInFirebase();
        }
    }

    if (showAlert) {
        FLAlertLayer::create("Streak Lost", "You missed a day!", "OK")->show();
    }
}

void StreakData::checkRewards() {
    bool changed = false;
    for (size_t i = 0; i < badges.size(); i++) {
        if (badges[i].isFromRoulette) continue;
        if (i < unlockedBadges.size() && currentStreak >= badges[i].daysRequired && !unlockedBadges[i]) {
            unlockedBadges[i] = true;
            changed = true;
        }
    }
    if (changed) {
        save();
    }
}

void StreakData::addPoints(int count) {
    dailyUpdate();
    int currentRequired = getRequiredPoints();
    bool alreadyGotRacha = (streakPointsToday >= currentRequired);
    streakPointsToday += count;
    totalStreakPoints += count;
    std::string today = getCurrentDate();
    streakPointsHistory[today] = streakPointsToday;

    if (!alreadyGotRacha && streakPointsToday >= currentRequired) {
        currentStreak++;
        hasNewStreak = true;
        checkRewards();
    }
    save();
}

bool StreakData::shouldShowAnimation() {
    if (hasNewStreak) {
        hasNewStreak = false;
        return true;
    }
    return false;
}

std::string StreakData::getRachaSprite() {
    int streak = currentStreak;
    if (streak >= 80) return "racha9.png"_spr;
    if (streak >= 70) return "racha8.png"_spr;
    if (streak >= 60) return "racha7.png"_spr;
    if (streak >= 50) return "racha6.png"_spr;
    if (streak >= 40) return "racha5.png"_spr;
    if (streak >= 30) return "racha4.png"_spr;
    if (streak >= 20) return "racha3.png"_spr;
    if (streak >= 10) return "racha2.png"_spr;
    if (streak >= 1)  return "racha1.png"_spr;
    return "racha0.png"_spr;
}

std::string StreakData::getCategoryName(BadgeCategory category) {
    switch (category) {
    case BadgeCategory::COMMON: return "Common";
    case BadgeCategory::SPECIAL: return "Special";
    case BadgeCategory::EPIC: return "Epic";
    case BadgeCategory::LEGENDARY: return "Legendary";
    case BadgeCategory::MYTHIC: return "Mythic";
    default: return "Unknown";
    }
}

ccColor3B StreakData::getCategoryColor(BadgeCategory category) {
    switch (category) {
    case BadgeCategory::COMMON: return ccc3(200, 200, 200);
    case BadgeCategory::SPECIAL: return ccc3(0, 170, 0);
    case BadgeCategory::EPIC: return ccc3(170, 0, 255);
    case BadgeCategory::LEGENDARY: return ccc3(255, 165, 0);
    case BadgeCategory::MYTHIC: return ccc3(255, 50, 50);
    default: return ccc3(255, 255, 255);
    }
}

StreakData::BadgeInfo* StreakData::getBadgeInfo(const std::string& badgeID) {
    for (auto& badge : badges) {
        if (badge.badgeID == badgeID) return &badge;
    }
    return nullptr;
}

bool StreakData::isBadgeUnlocked(const std::string& badgeID) {
    for (size_t i = 0; i < badges.size(); i++) {
        if (badges[i].badgeID == badgeID) {
            if (!unlockedBadges.empty() && i < unlockedBadges.size()) return unlockedBadges[i];
        }
    }
    return false;
}

void StreakData::equipBadge(const std::string& badgeID) {
    if (isBadgeUnlocked(badgeID)) {
        equippedBadge = badgeID;
        save();
    }
}

StreakData::BadgeInfo* StreakData::getEquippedBadge() {
    if (equippedBadge.empty()) return nullptr;
    return getBadgeInfo(equippedBadge);
}