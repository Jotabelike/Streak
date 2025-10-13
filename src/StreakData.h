#pragma once

#include <Geode/Geode.hpp>
#include <ctime>
#include <string>
#include <vector>
#include <map>
#include <iomanip>
#include <sstream>
#include <sstream>
#include "FirebaseManager.h"
using namespace geode::prelude;


struct StreakData {
    int currentStreak = 0;
    int streakPointsToday = 0;
    int totalStreakPoints = 0;
    bool hasNewStreak = false;
    std::string lastDay = "";
    std::string equippedBadge = "";
    int superStars = 0;
    int lastRouletteIndex = 0;
    int totalSpins = 0;
    int starTickets = 0;
    std::vector<int> streakCompletedLevels;
    std::map<std::string, int> streakPointsHistory;
    bool pointMission1Claimed = false;
    bool pointMission2Claimed = false;
    bool pointMission3Claimed = false;
    bool pointMission4Claimed = false;
    bool pointMission5Claimed = false;
    bool pointMission6Claimed = false;


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
        {0, "super_star.png"_spr, "First Mythic", BadgeCategory::MYTHIC, "super_star_badge", true}
    };

    std::vector<bool> unlockedBadges;
    int getRequiredPoints() {
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

    int getTicketValueForRarity(BadgeCategory category) {
        switch (category) {
        case BadgeCategory::COMMON:   return 5;
        case BadgeCategory::SPECIAL:  return 20;
        case BadgeCategory::EPIC:     return 50;
        case BadgeCategory::LEGENDARY: return 100;
        case BadgeCategory::MYTHIC:   return 500;
        default:                      return 0;
        }
    }

    void unlockBadge(const std::string& badgeID) {
        for (size_t i = 0; i < badges.size(); ++i) {
            if (badges[i].badgeID == badgeID) {
                if (i < unlockedBadges.size()) unlockedBadges[i] = true;
                return;
            }
        }
    }

    void load() {
        totalStreakPoints = geode::Mod::get()->getSavedValue<int>("totalStreakPoints", 0);
        currentStreak = geode::Mod::get()->getSavedValue<int>("streak", 0);
        streakPointsToday = geode::Mod::get()->getSavedValue<int>("streakPointsToday", 0);
        hasNewStreak = geode::Mod::get()->getSavedValue<bool>("hasNewStreak", false);
        lastDay = geode::Mod::get()->getSavedValue<std::string>("lastDay", "");
        equippedBadge = geode::Mod::get()->getSavedValue<std::string>("equippedBadge", "");
        superStars = geode::Mod::get()->getSavedValue<int>("superStars", 0);
        lastRouletteIndex = geode::Mod::get()->getSavedValue<int>("lastRouletteIndex", 0);
        totalSpins = geode::Mod::get()->getSavedValue<int>("totalSpins", 0);
        starTickets = geode::Mod::get()->getSavedValue<int>("starTickets", 0);
        streakCompletedLevels = geode::Mod::get()->getSavedValue<std::vector<int>>("streakCompletedLevels", {});

       
        pointMission1Claimed = geode::Mod::get()->getSavedValue<bool>("pointMission1Claimed", false);
        pointMission2Claimed = geode::Mod::get()->getSavedValue<bool>("pointMission2Claimed", false);
        pointMission3Claimed = geode::Mod::get()->getSavedValue<bool>("pointMission3Claimed", false);
        pointMission4Claimed = geode::Mod::get()->getSavedValue<bool>("pointMission4Claimed", false);
        pointMission5Claimed = geode::Mod::get()->getSavedValue<bool>("pointMission5Claimed", false);
        pointMission6Claimed = geode::Mod::get()->getSavedValue<bool>("pointMission6Claimed", false);

        unlockedBadges.resize(badges.size(), false);
        for (size_t i = 0; i < badges.size(); i++) {
            unlockedBadges[i] = geode::Mod::get()->getSavedValue<bool>(badges[i].badgeID, false);
        }
        streakPointsHistory = geode::Mod::get()->getSavedValue<std::map<std::string, int>>("streakPointsHistory", {});
    }
    

    void save() {
        geode::Mod::get()->setSavedValue<int>("streak", currentStreak);
        geode::Mod::get()->setSavedValue<int>("streakPointsToday", streakPointsToday);
        geode::Mod::get()->setSavedValue<int>("totalStreakPoints", totalStreakPoints);
        geode::Mod::get()->setSavedValue<bool>("hasNewStreak", hasNewStreak);
        geode::Mod::get()->setSavedValue<std::string>("lastDay", lastDay);
        geode::Mod::get()->setSavedValue<std::string>("equippedBadge", equippedBadge); 
        geode::Mod::get()->setSavedValue<int>("superStars", superStars);
        geode::Mod::get()->setSavedValue<int>("lastRouletteIndex", lastRouletteIndex);
        geode::Mod::get()->setSavedValue<int>("totalSpins", totalSpins);
        geode::Mod::get()->setSavedValue<int>("starTickets", starTickets);

		
        geode::Mod::get()->setSavedValue<bool>("pointMission1Claimed", pointMission1Claimed);
        geode::Mod::get()->setSavedValue<bool>("pointMission2Claimed", pointMission2Claimed);
        geode::Mod::get()->setSavedValue<bool>("pointMission3Claimed", pointMission3Claimed);
        geode::Mod::get()->setSavedValue<bool>("pointMission4Claimed", pointMission4Claimed);
        geode::Mod::get()->setSavedValue<bool>("pointMission5Claimed", pointMission5Claimed);
        geode::Mod::get()->setSavedValue<bool>("pointMission6Claimed", pointMission6Claimed);

        geode::Mod::get()->setSavedValue<std::vector<int>>("streakCompletedLevels", streakCompletedLevels);
        for (size_t i = 0; i < badges.size(); i++) {
            geode::Mod::get()->setSavedValue<bool>(badges[i].badgeID, unlockedBadges[i]);
        }
        geode::Mod::get()->setSavedValue("streakPointsHistory", streakPointsHistory);
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
        if (lastDay.empty()) {
            lastDay = today;
            streakPointsToday = 0;
            save();
            return;
        }
        if (lastDay == today) return;

        tm last_tm = {};
        std::stringstream ss(lastDay);
        ss >> std::get_time(&last_tm, "%Y-%m-%d");
        time_t last_t = mktime(&last_tm);
        double seconds_passed = difftime(now_t, last_t);
        double days_passed = seconds_passed / (60.0 * 60.0 * 24.0);

        bool streak_should_be_lost = false;
        if (days_passed >= 2.0) {
            streak_should_be_lost = true;
        }
        else if (days_passed >= 1.0) {
            if (streakPointsToday < getRequiredPoints()) {
                streak_should_be_lost = true;
            }
        }

        if (streak_should_be_lost) {
            currentStreak = 0;
            streakPointsHistory.clear();
            totalStreakPoints = 0;

            FLAlertLayer::create("Streak Lost", "You missed a day!", "OK")->show();
        }

        
        streakPointsToday = 0;
        lastDay = today;

        pointMission1Claimed = false;
        pointMission2Claimed = false;
        pointMission3Claimed = false;
        pointMission4Claimed = false;
        pointMission5Claimed = false;
        pointMission6Claimed = false;

        save();
    }

    void checkRewards() {
        for (size_t i = 0; i < badges.size(); i++) {
            if (badges[i].isFromRoulette) continue;
            if (currentStreak >= badges[i].daysRequired && i < unlockedBadges.size() && !unlockedBadges[i]) {
                unlockedBadges[i] = true;
            }
        }
        save();
    }

    void addPoints(int count) {
        load();
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
            int oldRequired = currentRequired;
            int newRequired = getRequiredPoints();
            if (newRequired > oldRequired) {
                streakPointsToday = oldRequired;
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

    ccColor3B getCategoryColor(BadgeCategory category) {
        switch (category) {
        case BadgeCategory::COMMON: return ccc3(200, 200, 200);
        case BadgeCategory::SPECIAL: return ccc3(0, 170, 0);
        case BadgeCategory::EPIC: return ccc3(170, 0, 255);
        case BadgeCategory::LEGENDARY: return ccc3(255, 165, 0);
        case BadgeCategory::MYTHIC: return ccc3(255, 50, 50);
        default: return ccc3(255, 255, 255);
        }
    }

    BadgeInfo* getBadgeInfo(const std::string& badgeID) {
        for (auto& badge : badges) {
            if (badge.badgeID == badgeID) return &badge;
        }
        return nullptr;
    }

    bool isBadgeUnlocked(const std::string& badgeID) {
        for (size_t i = 0; i < badges.size(); i++) {
            if (badges[i].badgeID == badgeID) {
                if (i < unlockedBadges.size()) return unlockedBadges[i];
            }
        }
        return false;
    }

    void equipBadge(const std::string& badgeID) {
        if (isBadgeUnlocked(badgeID)) {
            equippedBadge = badgeID;
            save();
        }
    }

    BadgeInfo* getEquippedBadge() {
        if (equippedBadge.empty()) return nullptr;
        return getBadgeInfo(equippedBadge);
    }
};

extern StreakData g_streakData;