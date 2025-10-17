#pragma once

#include <Geode/Geode.hpp>
#include <ctime>
#include <string>
#include <vector>
#include <map>
#include <iomanip>
#include <sstream>
#include "FirebaseManager.h"

using namespace geode::prelude;

struct StreakData {
    int currentStreak;
    int streakPointsToday;
    int totalStreakPoints;
    bool hasNewStreak;
    std::string lastDay;
    std::string equippedBadge;
    int superStars;
    int lastRouletteIndex;
    int totalSpins;
    int starTickets;
    std::vector<int> streakCompletedLevels;
    std::map<std::string, int> streakPointsHistory;
    bool pointMission1Claimed;
    bool pointMission2Claimed;
    bool pointMission3Claimed;
    bool pointMission4Claimed;
    bool pointMission5Claimed;
    bool pointMission6Claimed;
    bool isDataLoaded;
    bool m_initialized = false;

    enum class BadgeCategory {
        COMMON, SPECIAL, EPIC, LEGENDARY, MYTHIC
    };

    struct BadgeInfo {
        int daysRequired;
        std::string spriteName;
        std::string displayName;
        BadgeCategory category;
        std::string badgeID;
        bool isFromRoulette;
    };

    std::vector<BadgeInfo> badges = {
        {5, "reward5.png"_spr, "First Steps", BadgeCategory::COMMON, "badge_5", false},
        {10, "reward10.png"_spr, "Shall We Continue?", BadgeCategory::COMMON, "badge_10", false},
        {30, "reward30.png"_spr, "We're Going Well", BadgeCategory::SPECIAL, "badge_30", false},
        {50, "reward50.png"_spr, "Half a Hundred", BadgeCategory::SPECIAL, "badge_50", false},
        {70, "reward70.png"_spr, "Progressing", BadgeCategory::EPIC, "badge_70", false},
        {100, "reward100.png"_spr, "100 Days!!!", BadgeCategory::LEGENDARY, "badge_100", false},
        {150, "reward150.png"_spr, "150 Days!!!", BadgeCategory::LEGENDARY, "badge_150", false},
        {300, "reward300.png"_spr, "300 Days!!!", BadgeCategory::LEGENDARY, "badge_300", false},
        {365, "reward1year.png"_spr, "1 Year!!!", BadgeCategory::MYTHIC, "badge_365", false},

        {0, "badge_beta.png"_spr, "Player beta?", BadgeCategory::COMMON, "beta_badge", true},
        {0, "badge_platino.png"_spr, "platino badge", BadgeCategory::COMMON, "platino_streak_badge", true},
        {0, "badge_diamante_gd.png"_spr, "GD Diamond!", BadgeCategory::COMMON, "diamante_gd_badge", true},
        {0, "badge_ncs.png"_spr, "Ncs Lover", BadgeCategory::SPECIAL, "ncs_badge", true},
        {0, "dark_badge1.png"_spr, "dark side", BadgeCategory::EPIC, "dark_streak_badge", true},
        {0, "badge_diamante_mc.png"_spr, "Minecraft Diamond!", BadgeCategory::EPIC, "diamante_mc_badge", true},
        {0, "gold_streak.png"_spr, "Gold Legend's", BadgeCategory::LEGENDARY, "gold_streak_badge", true},
        {0, "super_star.png"_spr, "First Mythic", BadgeCategory::MYTHIC, "super_star_badge", true},
        {0, "magic_flower_badge.png"_spr, "hypnotizes with its beauty", BadgeCategory::MYTHIC, "magic_flower_badge", true},
        {0, "hounter_badge.png"_spr, "looks familiar to me", BadgeCategory::SPECIAL, "hounter_badge", true },
        {0, "gd_badge.png"_spr, "GD!", BadgeCategory::COMMON, "gd_badge", true },
        {0, "adrian_badge.png"_spr, "juegajuegos", BadgeCategory::COMMON, "adrian_badge", true }
    };

    std::vector<bool> unlockedBadges;

    bool isInitialized() const {
        return m_initialized && isDataLoaded;
    }

    // Añade un método para inicializar unlockedBadges
    void initializeUnlockedBadges() {
        if (unlockedBadges.empty()) {
            unlockedBadges.assign(badges.size(), false);
        }
    }

    void parseServerResponse(const matjson::Value& data);
    void resetToDefault();
    void load();
    void save();
    int getRequiredPoints();
    int getTicketValueForRarity(BadgeCategory category);
    void unlockBadge(const std::string& badgeID);
    std::string getCurrentDate();
    void unequipBadge();
    bool isBadgeEquipped(const std::string& badgeID);
    void dailyUpdate();
    void checkRewards();
    void addPoints(int count);
    bool shouldShowAnimation();
    std::string getRachaSprite();
    std::string getCategoryName(BadgeCategory category);
    ccColor3B getCategoryColor(BadgeCategory category);
    BadgeInfo* getBadgeInfo(const std::string& badgeID);
    bool isBadgeUnlocked(const std::string& badgeID);
    void equipBadge(const std::string& badgeID);
    BadgeInfo* getEquippedBadge();
};

extern StreakData g_streakData;