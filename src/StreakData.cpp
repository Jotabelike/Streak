#include "StreakData.h"
#include "FirebaseManager.h"
#include <Geode/utils/cocos.hpp> 
#include <sstream>
#include <iomanip>
#include <ctime>
#include <cmath>
#include <Geode/binding/GJAccountManager.hpp> 
#include <algorithm>
#include <cctype>   
#include "SystemNotification.h"
#include "RewardNotification.h"
#include <random>
#include <set>

extern void completeLevelInFirebase(int stars);

std::queue<NotificationData> SystemNotification::s_queue;
SystemNotification* SystemNotification::s_activeNotification = nullptr;

StreakData g_streakData;

void StreakData::resetToDefault() {
    equippedNameColor = "Default";
    equippedNameFont = "Default";
    equippedNameEffect = "None";
    equippedNameAnimation = "None";
    unlockedNameItems.clear();
    currentStreak = 0;
    streakPointsToday = 0;
    pinnedLevels.clear();
    totalStreakPoints = 0;
    hasNewStreak = false;
    lastDay = "";
    equippedBadge = "";
    gemRouletteHash = "";
    claimedStreakGoals.clear();
    gems = 0;
    superStars = 0;
    globalRank = 0;
    streakID = "";
    lastStreakAnimated = 0;
    needsRegistration = false;
    isBanned = false;
    banReason = "";
    starTickets = 0;
    specialRank = 0;
    lastRouletteIndex = 0;
    totalSpins = 0;
    currentXP = 0;
    gemRouletteSpinCount = 0;
    gemRouletteState.assign(7, false);
    currentLevel = 1;
    isTaskEnabled = false;
    taskStatuses.clear();
    streakCompletedLevels.clear();
    streakPointsHistory.clear();
    pointMission1Claimed = false;
    pointMission2Claimed = false;
    pointMission3Claimed = false;
    pointMission4Claimed = false;
    pointMission5Claimed = false;
    pointMission6Claimed = false;

    if (unlockedBadges.size() != badges.size()) {
        unlockedBadges.assign(badges.size(), false);
    }
    else {
        std::fill(unlockedBadges.begin(), unlockedBadges.end(), false);
    }

    equippedBanner = "";
    if (unlockedBanners.size() != banners.size()) {
        unlockedBanners.assign(banners.size(), false);
    }
    else {
        std::fill(unlockedBanners.begin(), unlockedBanners.end(), false);
    }

    completedLevelMissions.clear();
    userRole = 0;
    dailyMsgCount = 0;
    isDataLoaded = false;
    m_initialized = false;
}

void StreakData::load() {}

void StreakData::save() {
    if (!isDataLoaded && !m_initialized) return;
    updatePlayerDataInFirebase();
}

void StreakData::parseServerResponse(const matjson::Value& data) {
    auto safeInt = [](const matjson::Value& json, const std::string& key, int defaultVal = 0) -> int {
        if (!json.contains(key)) return defaultVal;
        auto val = json[key];
        if (val.isNumber()) return val.as<int>().unwrapOr(defaultVal);
        if (val.isString()) {
            std::string s = val.as<std::string>().unwrapOr(std::string(""));
            try { return std::stoi(s); }
            catch (...) { return defaultVal; }
        }
        return defaultVal;
        };

    currentStreak = safeInt(data, "current_streak_days", 0);
    lastStreakAnimated = safeInt(data, "last_streak_animated", 0);
    totalStreakPoints = safeInt(data, "total_streak_points", 0);
    equippedBadge = data["equipped_badge_id"].as<std::string>().unwrapOr(std::string(""));
    equippedBanner = data["equipped_banner_id"].as<std::string>().unwrapOr(std::string(""));
    superStars = safeInt(data, "super_stars", 0);
    starTickets = safeInt(data, "star_tickets", 0);
    lastRouletteIndex = safeInt(data, "last_roulette_index", 0);
    totalSpins = safeInt(data, "total_spins", 0);
    lastDay = data["lastDay"].as<std::string>().unwrapOr(std::string(""));
    if (lastDay.empty() && data.contains("last_day")) {
        lastDay = data["last_day"].as<std::string>().unwrapOr(std::string(""));
    }
    streakPointsToday = safeInt(data, "streakPointsToday", 0);
    gems = safeInt(data, "gems", 0);
    gemRouletteSpinCount = safeInt(data, "gem_roulette_spin_count", 0);
    gemRouletteHash = data["gem_roulette_hash"].as<std::string>().unwrapOr(std::string(""));
    equippedNameAnimation = data["equipped_name_animation"].as<std::string>().unwrapOr(std::string("None"));
    equippedNameColor = data["equipped_name_color"].as<std::string>().unwrapOr(std::string("Default"));
    equippedNameFont = data["equipped_name_font"].as<std::string>().unwrapOr(std::string("Default"));
    equippedNameEffect = data["equipped_name_effect"].as<std::string>().unwrapOr(std::string("None"));

    if (data.contains("gem_roulette_state")) {
        gemRouletteState = data["gem_roulette_state"].as<std::vector<bool>>().unwrapOr(std::vector<bool>(7, false));
    }
    else {
        gemRouletteState.assign(7, false);
    }

    if (data.contains("pending_season_reward")) {
        pendingSeasonRank = safeInt(data, "pending_season_reward", 0);
    }
    else {
        pendingSeasonRank = 0;
    }

    streakID = data["streakID"].as<std::string>().unwrapOr(std::string("Pending..."));
    currentXP = safeInt(data, "current_xp", 0);
    currentLevel = safeInt(data, "current_level", 1);

    int reqPoints = this->getRequiredPoints();
    if (streakPointsToday >= reqPoints && reqPoints > 0) {
        hasNewStreak = true;
    }
    else {
        hasNewStreak = false;
    }

    if (data.contains("rank")) {
        globalRank = safeInt(data, "rank", 0);
    }
    else if (data.contains("global_rank")) {
        globalRank = safeInt(data, "global_rank", 0);
    }

    if (data.contains("task_enabled")) {
        isTaskEnabled = data["task_enabled"].as<bool>().unwrapOr(false);
    }
    else {
        isTaskEnabled = false;
    }

    taskStatuses.clear();
    if (data.contains("taskStatuses")) {
        auto statusesVal = data["taskStatuses"];
        if (statusesVal.isObject()) {
            auto res = statusesVal.as<std::map<std::string, matjson::Value>>();
            if (res.isOk()) {
                for (auto const& [key, value] : res.unwrap()) {
                    taskStatuses[key] = value.as<std::string>().unwrapOr(std::string(""));
                }
            }
        }
    }

    userRole = 0;
    if (data.contains("role")) {
        if (data["role"].isString()) {
            std::string roleStr = data["role"].as<std::string>().unwrapOr(std::string(""));
            std::transform(roleStr.begin(), roleStr.end(), roleStr.begin(), [](unsigned char c) { return std::tolower(c); });
            if (roleStr == "admin" || roleStr == "administrator") userRole = 2;
            else if (roleStr == "moderator" || roleStr == "mod") userRole = 1;
        }
        else {
            userRole = safeInt(data, "role", 0);
        }
    }
    specialRank = safeInt(data, "special_rank", 0);
    dailyMsgCount = safeInt(data, "daily_msg_count", 0);
    isBanned = data["ban"].as<bool>().unwrapOr(false);
    banReason = data["ban_reason"].as<std::string>().unwrapOr(std::string("No reason provided."));

    if (unlockedBadges.size() != badges.size()) {
        unlockedBadges.assign(badges.size(), false);
    }
    else {
        std::fill(unlockedBadges.begin(), unlockedBadges.end(), false);
    }

    if (data.contains("unlocked_badges")) {
        auto badgesResult = data["unlocked_badges"].as<std::vector<matjson::Value>>();
        if (badgesResult.isOk()) {
            for (const auto& badge_id_json : badgesResult.unwrap()) {
                unlockBadge(badge_id_json.as<std::string>().unwrapOr(std::string("")));
            }
        }
    }

    if (unlockedBanners.size() != banners.size()) {
        unlockedBanners.assign(banners.size(), false);
    }
    else {
        std::fill(unlockedBanners.begin(), unlockedBanners.end(), false);
    }

    if (data.contains("unlocked_banners")) {
        auto bannersResult = data["unlocked_banners"].as<std::vector<matjson::Value>>();
        if (bannersResult.isOk()) {
            for (const auto& banner_id_json : bannersResult.unwrap()) {
                unlockBanner(banner_id_json.as<std::string>().unwrapOr(std::string("")));
            }
        }
    }


    unlockedNameItems.clear();
    if (data.contains("unlocked_name_items")) {
        auto itemsResult = data["unlocked_name_items"].as<std::vector<matjson::Value>>();
        if (itemsResult.isOk()) {
            for (const auto& val : itemsResult.unwrap()) {
                unlockedNameItems.insert(val.as<std::string>().unwrapOr(std::string("")));
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
            auto m = missionsResult.unwrap();
            if (m.count("pm1")) pointMission1Claimed = m.at("pm1").as<bool>().unwrapOr(false);
            if (m.count("pm2")) pointMission2Claimed = m.at("pm2").as<bool>().unwrapOr(false);
            if (m.count("pm3")) pointMission3Claimed = m.at("pm3").as<bool>().unwrapOr(false);
            if (m.count("pm4")) pointMission4Claimed = m.at("pm4").as<bool>().unwrapOr(false);
            if (m.count("pm5")) pointMission5Claimed = m.at("pm5").as<bool>().unwrapOr(false);
            if (m.count("pm6")) pointMission6Claimed = m.at("pm6").as<bool>().unwrapOr(false);
        }
    }

    streakPointsHistory.clear();
    if (data.contains("history")) {
        auto h = data["history"].as<std::map<std::string, matjson::Value>>();
        if (h.isOk()) {
            for (const auto& [date, val] : h.unwrap()) {
                if (val.isNumber()) streakPointsHistory[date] = val.as<int>().unwrapOr(0);
                else if (val.isString()) {
                    try { streakPointsHistory[date] = std::stoi(val.as<std::string>().unwrapOr(std::string("0"))); }
                    catch (...) { streakPointsHistory[date] = 0; }
                }
            }
        }
    }

    completedLevelMissions.clear();
    if (data.contains("completedLevelMissions")) {
        auto m = data["completedLevelMissions"].as<std::map<std::string, matjson::Value>>();
        if (m.isOk()) {
            for (const auto& [idStr, _] : m.unwrap()) {
                if (auto id = numFromString<int>(idStr)) completedLevelMissions.insert(id.unwrap());
            }
        }
    }

    claimedStreakGoals.clear();
    if (data.contains("claimed_streak_goals")) {
        auto goalsData = data["claimed_streak_goals"];
        if (goalsData.isObject()) {
            for (const auto& [idStr, _] : goalsData.as<std::map<std::string, matjson::Value>>().unwrap()) {
                try {
                    claimedStreakGoals.insert(std::stoi(idStr));
                }
                catch (...) {}
            }
        }
        else if (goalsData.isArray()) {
            for (const auto& val : goalsData.as<std::vector<matjson::Value>>().unwrap()) {
                claimedStreakGoals.insert(val.as<int>().unwrapOr(-1));
            }
        }
    }

    pinnedLevels.clear();
    if (data.contains("pinned_levels")) {
        auto pins = data["pinned_levels"];
        if (pins.isObject()) {
            auto res = pins.as<std::map<std::string, matjson::Value>>();
            if (res.isOk()) {
                for (auto const& [key, val] : res.unwrap()) {
                    pinnedLevels[key] = val.as<int>().unwrapOr(0);
                }
            }
        }
    }

    this->checkRewards();
    isDataLoaded = true;
    m_initialized = true;
}

bool StreakData::isLevelMissionClaimed(int levelID) const {
    return completedLevelMissions.count(levelID) > 0;
}

int StreakData::getRequiredPoints() {
    if (currentStreak >= 100) return 12;
    if (currentStreak >= 90)  return 11;
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
    case BadgeCategory::COMMON: return 5;
    case BadgeCategory::SPECIAL: return 20;
    case BadgeCategory::EPIC: return 50;
    case BadgeCategory::LEGENDARY: return 100;
    case BadgeCategory::MYTHIC: return 500;
    default: return 0;
    }
}

void StreakData::unlockBadge(const std::string& badgeID) {
    if (badgeID.empty()) return;
    if (unlockedBadges.size() != badges.size()) unlockedBadges.assign(badges.size(), false);
    for (size_t i = 0; i < badges.size(); ++i) {
        if (i < unlockedBadges.size() && badges[i].badgeID == badgeID) {
            unlockedBadges[i] = true;
            return;
        }
    }
}

std::string StreakData::getCurrentDate() {
    time_t t = time(nullptr);
    tm* now = localtime(&t);
    if (!now) return "";
    char buf[16];
    if (strftime(buf, sizeof(buf), "%F", now) == 0) return "";
    return std::string(buf);
}

void StreakData::unequipBadge() {
    if (!equippedBadge.empty()) {
        equippedBadge = "";
        save();
    }
}

bool StreakData::isBadgeEquipped(const std::string& badgeID) {
    return !badgeID.empty() && equippedBadge == badgeID;
}

void StreakData::dailyUpdate() {
    if (!isDataLoaded) return;
    time_t now_t = time(nullptr);
    std::string today = getCurrentDate();
    if (today.empty()) return;

    if (lastDay.empty()) {
        lastDay = today;
        streakPointsToday = 0;
        dailyMsgCount = 0;
        pointMission1Claimed = false;
        pointMission2Claimed = false;
        pointMission3Claimed = false;
        pointMission4Claimed = false;
        pointMission5Claimed = false;
        pointMission6Claimed = false;
        save();
        return;
    }

    if (lastDay == today) return;

    streakPointsToday = 0;
    dailyMsgCount = 0;
    lastDay = today;
    hasNewStreak = false;

    pointMission1Claimed = false;
    pointMission2Claimed = false;
    pointMission3Claimed = false;
    pointMission4Claimed = false;
    pointMission5Claimed = false;
    pointMission6Claimed = false;
}

void StreakData::checkRewards() {
    bool changed = false;
    if (unlockedBadges.size() != badges.size()) unlockedBadges.assign(badges.size(), false);
    for (size_t i = 0; i < badges.size(); i++) {
        if (i >= unlockedBadges.size()) continue;
        if (badges[i].isFromRoulette || unlockedBadges[i]) continue;
        if (currentStreak >= badges[i].daysRequired) {
            unlockedBadges[i] = true;
            changed = true;
        }
    }
    if (changed) save();
}

bool StreakData::shouldShowAnimation() {
    if (currentStreak > 0 && currentStreak > lastStreakAnimated) return true;
    return false;
}

std::string StreakData::getRachaSprite(int streak) {
    if (streak >= 100) return "racha11.png"_spr;
    if (streak >= 90) return "racha10.png"_spr;
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

std::string StreakData::getRachaSprite() {
    return getRachaSprite(this->currentStreak);
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
    case BadgeCategory::COMMON: return { 200, 200, 200 };
    case BadgeCategory::SPECIAL: return { 0, 170, 0 };
    case BadgeCategory::EPIC: return { 170, 0, 255 };
    case BadgeCategory::LEGENDARY: return { 255, 165, 0 };
    case BadgeCategory::MYTHIC: return { 255, 50, 50 };
    default: return { 255, 255, 255 };
    }
}

StreakData::BadgeInfo* StreakData::getBadgeInfo(const std::string& badgeID) {
    if (badgeID.empty()) return nullptr;
    for (auto& badge : badges) {
        if (badge.badgeID == badgeID) return &badge;
    }
    return nullptr;
}

bool StreakData::isBadgeUnlocked(const std::string& badgeID) {
    if (badgeID.empty()) return false;
    if (unlockedBadges.size() != badges.size()) return false;
    for (size_t i = 0; i < badges.size(); ++i) {
        if (i < unlockedBadges.size() && badges[i].badgeID == badgeID) return unlockedBadges[i];
    }
    return false;
}

void StreakData::equipBadge(const std::string& badgeID) {
    if (badgeID.empty()) return;
    if (isBadgeUnlocked(badgeID)) {
        if (equippedBadge != badgeID) {
            equippedBadge = badgeID;
            save();
        }
    }
}

StreakData::BadgeInfo* StreakData::getEquippedBadge() {
    return getBadgeInfo(equippedBadge);
}

int StreakData::getXPForCurrentStreak() {
    int bonus = (currentStreak / 10) * 15;
    return 25 + bonus;
}

int StreakData::getXPRequiredForNextLevel() {
    return currentLevel * 100;
}

float StreakData::getXPPercentage() {
    int req = getXPRequiredForNextLevel();
    if (req == 0) return 0.0f;
    return std::clamp((float)currentXP / (float)req, 0.0f, 1.0f);
}

void StreakData::unlockBanner(const std::string& bannerID) {
    if (bannerID.empty()) return;
    if (unlockedBanners.size() != banners.size()) unlockedBanners.assign(banners.size(), false);
    for (size_t i = 0; i < banners.size(); ++i) {
        if (i < unlockedBanners.size() && banners[i].bannerID == bannerID) {
            unlockedBanners[i] = true;
            return;
        }
    }
}

bool StreakData::isBannerUnlocked(const std::string& bannerID) {
    if (bannerID.empty()) return false;
    if (unlockedBanners.size() != banners.size()) return false;
    for (size_t i = 0; i < banners.size(); ++i) {
        if (i < unlockedBanners.size() && banners[i].bannerID == bannerID) return unlockedBanners[i];
    }
    return false;
}

StreakData::BannerInfo* StreakData::getBannerInfo(const std::string& bannerID) {
    if (bannerID.empty()) return nullptr;
    for (auto& banner : banners) {
        if (banner.bannerID == bannerID) return &banner;
    }
    return nullptr;
}

void StreakData::equipBanner(const std::string& bannerID) {
    if (bannerID.empty()) return;
    if (isBannerUnlocked(bannerID)) {
        if (equippedBanner != bannerID) {
            equippedBanner = bannerID;
            save();
        }
    }
}

void StreakData::unequipBanner() {
    if (!equippedBanner.empty()) {
        equippedBanner = "";
        save();
    }
}

StreakData::BannerInfo* StreakData::getEquippedBanner() {
    return getBannerInfo(equippedBanner);
}

bool StreakData::isStreakGoalClaimed(int index) const {
    return claimedStreakGoals.count(index) > 0;
}

void StreakData::setStreakGoalClaimed(int index) {
    claimedStreakGoals.insert(index);
}

int StreakData::getPriceForRarity(BadgeCategory rarity) {
    switch (rarity) {
    case BadgeCategory::COMMON: return 10;
    case BadgeCategory::SPECIAL: return 25;
    case BadgeCategory::EPIC: return 50;
    case BadgeCategory::LEGENDARY: return 100;
    default: return 9999;
    }
}

int getRarityWeight(StreakData::BadgeCategory category) {
    switch (category) {
    case StreakData::BadgeCategory::COMMON:    return 150;
    case StreakData::BadgeCategory::SPECIAL:   return 80;
    case StreakData::BadgeCategory::EPIC:      return 30;
    case StreakData::BadgeCategory::LEGENDARY: return 5;
    case StreakData::BadgeCategory::MYTHIC:    return 0;
    default: return 0;
    }
}

std::vector<StreakData::ShopItem> StreakData::getDailyShopSelection() {
    std::vector<ShopItem> selection;


    unsigned int seed = 0;
    if (this->dailyShopSeed != 0) {
        seed = static_cast<unsigned int>(this->dailyShopSeed);
    }
    else {

        std::string dateStr = getCurrentDate();
        if (dateStr.empty()) return selection;
        seed = static_cast<unsigned int>(std::stoi(dateStr.substr(0, 4) + dateStr.substr(5, 2) + dateStr.substr(8, 2)));
    }


    std::mt19937 gen(seed);


    std::set<std::string> excludedIDs = {
        "ncs_badge", "super_star_badge", "beta_badge", "magic_flower_badge",
        "diamond_streak_badge", "past1_badge", "marshmello_badge", "alan_walker_badge",
        "shiver_badge", "dual_badge", "ttv_badge", "tsukasa_badge", "funhouse_badge",
        "miku_badge", "nantendo_badge", "youtube_badge", "tiktok_badge",
        "Skeletal_Shenanigans_badge", "bh_badge_7", "winter_badge", "freddy_badge",
        "chica_badge","bonnie_badge","foxy_badge",
        "banner_5", "banner_15", "banner_21", "banner_22", "banner_23",
        "banner_33", "banner_40", "banner_41", "banner_44", "banner_45",
        "banner_19", "banner_26", "banner_32", "banner_16", "banner_29",
        "banner_42", "banner_51", "banner_52", "banner_53",
        "badge_5", "badge_10", "badge_30", "badge_50", "badge_70",
        "badge_100", "badge_150", "badge_300", "badge_365",
        "moderator_badge", "creator_badge", "vip_badge", "stellar_badge",
        "cube_mastery_badge", "ship_mastery_badge", "ufo_mastery_badge",
        "ball_mastery_badge", "spider_mastery_badge", "wave_mastery_badge",
        "robot_mastery_badge", "memory_mastery_badge", "swingcopter_mastery_badge",
        "xl_mastery_badge", "dual_mastery_badge", "achievement_badge3", "archievement_badge2",
         "archievement_badge1"
    };

    std::vector<ShopItem> candidates;

    auto addCandidate = [&](const std::string& id,
        bool isBadge,
        BadgeCategory cat,
        const std::string& name,
        const std::string& spr, int daysReq) {
            if (cat == BadgeCategory::MYTHIC) return;
            if (excludedIDs.count(id)) return;
            if (daysReq > 0) return;
            candidates.push_back({ id, isBadge, getPriceForRarity(cat), cat, name, spr });
        };

    for (const auto& b : badges) addCandidate(b.badgeID, true, b.category, b.displayName, b.spriteName, b.daysRequired);
    for (const auto& b : banners) addCandidate(b.bannerID, false, b.rarity, b.displayName, b.spriteName, 0);


    if (!candidates.empty()) {
        for (int i = candidates.size() - 1; i > 0; i--) {
            std::uniform_int_distribution<> dist(0, i);
            int j = dist(gen);
            std::swap(candidates[i], candidates[j]);
        }
    }

    for (int i = 0; i < 3; i++) {
        if (candidates.empty()) break;

        int totalWeight = 0;
        for (const auto& item : candidates) totalWeight += getRarityWeight(item.rarity);

        if (totalWeight == 0) break;

        std::uniform_int_distribution<> dist(0, totalWeight - 1);
        int randomValue = dist(gen);

        int selectedIndex = -1;
        int currentWeight = 0;

        for (int j = 0; j < candidates.size(); j++) {
            currentWeight += getRarityWeight(candidates[j].rarity);
            if (randomValue < currentWeight) {
                selectedIndex = j;
                break;
            }
        }

        if (selectedIndex != -1) {
            selection.push_back(candidates[selectedIndex]);
            candidates.erase(candidates.begin() + selectedIndex);
        }
    }

    return selection;
}

std::vector<StreakData::ConsumableItem> StreakData::getDailyConsumableSelection() {
    std::vector<ConsumableItem> selection;

    unsigned int seed = 0;
    if (this->dailyShopSeed != 0) {
        seed = static_cast<unsigned int>(this->dailyShopSeed);
    }
    else {
        std::string dateStr = getCurrentDate();
        if (dateStr.empty()) return selection;
        seed = static_cast<unsigned int>(std::stoi(dateStr.substr(0, 4) + dateStr.substr(5, 2) + dateStr.substr(8, 2)));
    }

    // Offset seed so consumables differ from cosmetics
    std::mt19937 gen(seed + 77777);

    // ---- All consumable options by rarity ----
    std::vector<ConsumableItem> pool = {
        // Common Star Tickets
        { true, 100,  5, BadgeCategory::COMMON },
        { true, 120,  6, BadgeCategory::COMMON },
        { true, 200,  7, BadgeCategory::COMMON },
        // Special Star Tickets
        { true, 350, 12, BadgeCategory::SPECIAL },
        { true, 500, 15, BadgeCategory::SPECIAL },
        { true, 600, 18, BadgeCategory::SPECIAL },
        // Epic Star Tickets
        { true, 800,  25, BadgeCategory::EPIC },
        { true, 1000, 32, BadgeCategory::EPIC },
        { true, 1500, 40, BadgeCategory::EPIC },
        // Legendary Star Tickets
        { true, 3000, 60, BadgeCategory::LEGENDARY },
        { true, 5000, 80, BadgeCategory::LEGENDARY },

        // Common Super Stars
        { false, 5,  10, BadgeCategory::COMMON },
        { false, 10, 15, BadgeCategory::COMMON },
        { false, 12, 20, BadgeCategory::COMMON },
        // Special Super Stars
        { false, 20, 25, BadgeCategory::SPECIAL },
        { false, 30, 30, BadgeCategory::SPECIAL },
        { false, 40, 35, BadgeCategory::SPECIAL },
        // Epic Super Stars
        { false, 60,  45, BadgeCategory::EPIC },
        { false, 80,  55, BadgeCategory::EPIC },
        { false, 100, 65, BadgeCategory::EPIC },
        // Legendary Super Stars
        { false, 150, 80,  BadgeCategory::LEGENDARY },
        { false, 200, 100, BadgeCategory::LEGENDARY },
        { false, 300, 120, BadgeCategory::LEGENDARY },
    };

    // Shuffle pool
    for (int i = pool.size() - 1; i > 0; i--) {
        std::uniform_int_distribution<> dist(0, i);
        int j = dist(gen);
        std::swap(pool[i], pool[j]);
    }

    // Weighted selection of 3 consumables
    for (int i = 0; i < 3; i++) {
        if (pool.empty()) break;

        int totalWeight = 0;
        for (const auto& item : pool) totalWeight += getRarityWeight(item.rarity);
        if (totalWeight == 0) break;

        std::uniform_int_distribution<> dist(0, totalWeight - 1);
        int randomValue = dist(gen);

        int selectedIndex = -1;
        int currentWeight = 0;

        for (int j = 0; j < (int)pool.size(); j++) {
            currentWeight += getRarityWeight(pool[j].rarity);
            if (randomValue < currentWeight) {
                selectedIndex = j;
                break;
            }
        }

        if (selectedIndex != -1) {
            selection.push_back(pool[selectedIndex]);
            pool.erase(pool.begin() + selectedIndex);
        }
    }

    return selection;
}

void StreakData::purchaseItem(const ShopItem& item) {
    if (gems < item.price) return;
    gems -= item.price;
    if (item.isBadge) {
        unlockBadge(item.id);
    }
    else {
        unlockBanner(item.id);
    }
    save();
}

void StreakData::addPoints(int count) {
    if (!isDataLoaded) return;
    if (count <= 0) return;

    static auto s_lastPointTime = std::chrono::steady_clock::time_point();
    static int s_lastPointAmount = -1;
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - s_lastPointTime).count();
    if (elapsed < 500) return;
    if (elapsed < 2000 && count == s_lastPointAmount) return;

    s_lastPointTime = now;
    s_lastPointAmount = count;
    this->lastPointTime = now;

    dailyUpdate();

    streakPointsToday += count;
    totalStreakPoints += count;

    std::string today = getCurrentDate();
    if (!today.empty()) streakPointsHistory[today] = streakPointsToday;

    int starsToSend = 1;
    if (count >= 6) starsToSend = 10;
    else if (count >= 5) starsToSend = 9;
    else if (count >= 4) starsToSend = 7;
    else if (count >= 3) starsToSend = 5;
    else if (count >= 2) starsToSend = 4;
    else starsToSend = 1;

    completeLevelInFirebase(starsToSend);
   
}

void StreakData::addXP(int amount) {
    if (amount <= 0) return;

    int preLevel = currentLevel;
    currentXP += amount;

    while (currentXP >= getXPRequiredForNextLevel()) {
        currentXP -= getXPRequiredForNextLevel();
        currentLevel++;
    }

    int levelsGained = currentLevel - preLevel;

    if (levelsGained > 0) {
        int totalStarsGained = 0;
        int totalTicketsGained = 0;
        int totalGemsGained = 0;

        for (int i = 1; i <= levelsGained; i++) {
            auto rewards = getRewardsForLevel(preLevel + i);
            totalStarsGained += rewards.stars;
            totalTicketsGained += rewards.tickets;
            totalGemsGained += rewards.gems;
        }

        int startGems = this->gems;
        int startStars = this->superStars;
        int startTickets = this->starTickets;

        this->superStars += totalStarsGained;
        this->starTickets += totalTicketsGained;
        this->gems += totalGemsGained;
        this->save();

        SystemNotification::show(
            "LEVEL UP!",
            fmt::format("Welcome to Level {}", currentLevel),
            "xp.png"_spr,
            0.3f
        );


        auto winSize = CCDirector::sharedDirector()->getWinSize();
        CCPoint spawnPos = winSize / 2;

        if (totalGemsGained > 0) RewardNotification::show("gem.png"_spr, startGems, totalGemsGained, spawnPos);
        if (totalStarsGained > 0) RewardNotification::show("super_star.png"_spr, startStars, totalStarsGained, spawnPos);
        if (totalTicketsGained > 0) RewardNotification::show("star_tiket.png"_spr, startTickets, totalTicketsGained, spawnPos);
    }
    else {
        this->save();
    }
}

StreakData::LevelRewards StreakData::getRewardsForLevel(int level) {
    int r_tickets = 0;
    int r_stars = 0;
    int r_gems = 0;
    if (level < 10) r_tickets = 40;
    else if (level < 20) r_tickets = 48;
    else {
        int tier = (level / 10) - 1;
        r_tickets = 48 * static_cast<int>(std::pow(2, tier));
    }
    if (level < 10) r_stars = 5;
    else if (level < 20) r_stars = 20;
    else {
        int tier = (level / 10) - 1;
        r_stars = 20 + (tier * 20);
    }
    r_gems = ((level - 1) / 10) + 1;
    return { r_stars, r_tickets, r_gems };
}

bool StreakData::isNameItemUnlocked(const std::string& item) {
    if (item == "Default" || item == "None") return true;
    return unlockedNameItems.count(item) > 0;
}

void StreakData::unlockNameItem(const std::string& item) {
    unlockedNameItems.insert(item);
    save();
}

int StreakData::getNameItemPrice(const std::string& item) {

    return 100;
}