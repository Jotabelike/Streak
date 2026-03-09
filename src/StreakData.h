#pragma once
#include <Geode/Geode.hpp>
#include <ctime>
#include <string>
#include <vector>
#include <map>
#include <set> 
#include <iomanip>
#include <sstream>
#include <chrono> 

using namespace geode::prelude;

struct StreakData {

    enum class BadgeCategory {
        COMMON,
        SPECIAL,
        EPIC,
        LEGENDARY,
        MYTHIC
    };

    struct ShopItem {
        std::string id;
        bool isBadge;  
        int price;
        BadgeCategory rarity;
        std::string name;
        std::string sprite;
    };

    struct MailMessage {
        std::string id;
        std::string title;
        std::string body;
        std::string type;
        long long timestamp;
        int rewardStars = 0;
        int rewardTickets = 0;
        std::string rewardBadge = "";
        bool isClaimedLocal = false;
        bool hasRewards() const {
            return rewardStars > 0 || rewardTickets > 0 || !rewardBadge.empty();
        }
    };

    struct BadgeInfo {
        int daysRequired;
        std::string spriteName;
        std::string displayName;
        BadgeCategory category;
        std::string badgeID;
        bool isFromRoulette;
        std::string creator;
    };

    struct BannerInfo {
        std::string bannerID;
        std::string spriteName;
        std::string displayName;
        BadgeCategory rarity;
        std::string creator;
    };

    std::vector<bool> gemRouletteState;
    std::map<int, std::string> userBadgeCache;
    int currentStreak;
    int streakPointsToday;
    std::string gemRouletteHash = "";
    int totalStreakPoints;
    bool hasNewStreak;
    bool isTaskEnabled;
    std::string lastDay;
    std::string equippedBadge;
    std::string equippedBanner;
    int superStars;
    int lastRouletteIndex;
    int totalSpins;
    bool needsRegistration = false;
    int lastStreakAnimated = 0;
    bool isBanned = false;
    std::string banReason = "";
    int starTickets;
    int gems;
    int gemRouletteSpinCount = 0;
    std::string streakID = "";
    std::vector<int> streakCompletedLevels;
    std::map<std::string, int> streakPointsHistory;
    std::set<int> claimedStreakGoals;
    int specialRank = 0;
    long long seasonEndTime = 0; 
    int pendingSeasonRank = 0;  
    int dailyShopSeed = 0;

    std::string equippedNameAnimation = "None";
    std::string equippedNameColor = "Default";
    std::string equippedNameFont = "Default";
    std::string equippedNameEffect = "None";

    std::set<std::string> unlockedNameItems;
    bool isNameItemUnlocked(const std::string& item);
    void unlockNameItem(const std::string& item);
    int getNameItemPrice(const std::string& item);
    
    std::chrono::steady_clock::time_point lastPointTime;

    int currentXP = 0;
    int currentLevel = 1;


    bool pointMission1Claimed;
    bool pointMission2Claimed;
    bool pointMission3Claimed;
    bool pointMission4Claimed;
    bool pointMission5Claimed;
    bool pointMission6Claimed;

    bool isDataLoaded;
    bool m_initialized = false;
    int userRole = 0;
    int dailyMsgCount = 0;
    int globalRank = 0;

    std::vector<MailMessage> inbox;
    std::set<int> completedLevelMissions;

    std::map<std::string, std::string> taskStatuses;

    std::string getTaskStatus(const std::string& taskID) {
        if (taskStatuses.count(taskID)) return taskStatuses[taskID];
        return "";
    }

    void setTaskStatus(const std::string& taskID, const std::string& status) {
        taskStatuses[taskID] = status;
    }

    std::map<std::string, int> pinnedLevels;

    int getPinnedLevel(const std::string& badgeID) {
        if (pinnedLevels.count(badgeID)) return pinnedLevels[badgeID];
        return 0;  
    }

    void setPinnedLevel(const std::string& badgeID, int levelID) {
        pinnedLevels[badgeID] = levelID;
    }

    std::vector<BadgeInfo> badges = {
        //days
        {5, "reward5.png"_spr, "First Steps", BadgeCategory::COMMON, "badge_5", false, "XJotaBeLikeX"},
        {10, "reward10.png"_spr, "Shall We Continue?", BadgeCategory::COMMON, "badge_10", false, "XJotaBeLikeX"},
        {30, "reward30.png"_spr, "We're Going Well", BadgeCategory::SPECIAL, "badge_30", false, "XJotaBeLikeX"},
        {50, "reward50.png"_spr, "Half a Hundred", BadgeCategory::SPECIAL, "badge_50", false, "XJotaBeLikeX"},
        {70, "reward70.png"_spr, "Progressing", BadgeCategory::EPIC, "badge_70", false, "XJotaBeLikeX"},
        {100, "reward100.png"_spr, "100 Days!!!", BadgeCategory::LEGENDARY, "badge_100", false, "XJotaBeLikeX"},
        {150, "reward150.png"_spr, "150 Days!!!", BadgeCategory::LEGENDARY, "badge_150", false, "XJotaBeLikeX"},
        {300, "reward300.png"_spr, "300 Days!!!", BadgeCategory::LEGENDARY, "badge_300", false, "XJotaBeLikeX"},
        {365, "reward1year.png"_spr, "1 Year!!!", BadgeCategory::MYTHIC, "badge_365", false, "XJotaBeLikeX"},

        //more badges
        {0, "badge_beta.png"_spr, "Player beta?", BadgeCategory::COMMON, "beta_badge", true, "XJotaBeLikeX"},
        {0, "badge_platino.png"_spr, "platino badge", BadgeCategory::COMMON, "platino_streak_badge", true, "XJotaBeLikeX"},
        {0, "badge_diamante_gd.png"_spr, "GD Diamond!", BadgeCategory::COMMON, "diamante_gd_badge", true, "XJotaBeLikeX"},
        {0, "badge_ncs.png"_spr, "Ncs Lover", BadgeCategory::SPECIAL, "ncs_badge", true, "XJotaBeLikeX"},
        {0, "dark_badge1.png"_spr, "dark side", BadgeCategory::EPIC, "dark_streak_badge", true, "XJotaBeLikeX"},
        {0, "badge_diamante_mc.png"_spr, "Minecraft Diamond!", BadgeCategory::EPIC, "diamante_mc_badge", true, "XJotaBeLikeX"},
        {0, "gold_streak.png"_spr, "Gold Legend's", BadgeCategory::LEGENDARY, "gold_streak_badge", true, "XJotaBeLikeX"},
        {0, "super_star_badge.png"_spr, "First Mythic", BadgeCategory::MYTHIC, "super_star_badge", true, "XJotaBeLikeX"},
        {0, "magic_flower_badge.png"_spr, "hypnotizes with its beauty", BadgeCategory::MYTHIC, "magic_flower_badge", true, "XJotaBeLikeX"},
        {0, "hounter_badge.png"_spr, "looks familiar to me", BadgeCategory::SPECIAL, "hounter_badge", true, "XJotaBeLikeX"},
        {0, "gd_badge.png"_spr, "GD!", BadgeCategory::COMMON, "gd_badge", true, "XJotaBeLikeX"},
        {0, "adrian_badge.png"_spr, "juegajuegos", BadgeCategory::COMMON, "adrian_badge", true, "XJotaBeLikeX"},
        {0, "diamond_streak.png"_spr, "Diamond Legend's", BadgeCategory::EPIC, "diamond_streak_badge", true, "XJotaBeLikeX"},

        {0, "past1_badge.png"_spr, "Purple Edition", BadgeCategory::MYTHIC, "past1_badge", true, "XJotaBeLikeX"},
        {0, "past2_badge.png"_spr, "green Edition", BadgeCategory::LEGENDARY, "past2_badge", true, "XJotaBeLikeX"},
        {0, "past3_badge.png"_spr, "Red Edition", BadgeCategory::LEGENDARY, "past3_badge", true, "XJotaBeLikeX"},
        {0, "moon_badge.png"_spr, "Moon", BadgeCategory::EPIC, "moon_badge", true, "XJotaBeLikeX"},
        {0, "marshmello_badge.png"_spr, "Marshmello", BadgeCategory::SPECIAL, "marshmello_badge", true, "XJotaBeLikeX"},
        {0, "alan_walker_badge.png"_spr, "Alan Walker", BadgeCategory::LEGENDARY, "alan_walker_badge", true, "XJotaBeLikeX"},

        //misiones
        {0, "shiver_badge.png"_spr, "Shiver!", BadgeCategory::SPECIAL, "shiver_badge", true, "XJotaBeLikeX"},
        {0, "dual_badge.png"_spr, "Dual Master", BadgeCategory::LEGENDARY, "dual_badge", true, "XJotaBeLikeX"},
        {0, "ttv_badge.png"_spr, "The Towerverse?", BadgeCategory::LEGENDARY, "ttv_badge", true, "XJotaBeLikeX"},
        {0, "cos_badge.png"_spr, "Change of Scene", BadgeCategory::SPECIAL, "cos_badge", true, "XJotaBeLikeX"},
        {0, "b_badge.png"_spr, "B", BadgeCategory::SPECIAL, "b_badge", true, "XJotaBeLikeX"},
        {0, "spy_badge.png"_spr, "iSpy What?", BadgeCategory::EPIC, "spy_badge", true, "XJotaBeLikeX"},
        {0, "tsukasa_badge.png"_spr, "Tsukasitaaa", BadgeCategory::EPIC, "tsukasa_badge", true, "XJotaBeLikeX"},
        {0, "funhouse_badge.png"_spr, "you again?", BadgeCategory::LEGENDARY, "funhouse_badge", true, "XJotaBeLikeX"},
        {0, "cobre_badge.png"_spr, "idk", BadgeCategory::SPECIAL, "cobre_badge", true, "XJotaBeLikeX"},
        {0, "miku_badge.png"_spr, "Miku Miku oooeeooo", BadgeCategory::EPIC, "miku_badge", true, "XJotaBeLikeX"},
        {0, "nantendo_badge.png"_spr, "Nintendo bruh", BadgeCategory::COMMON, "nantendo_badge", true, "XJotaBeLikeX"},
        {0, "youtube_badge.png"_spr, "Youtube", BadgeCategory::COMMON, "youtube_badge", true, "XJotaBeLikeX"},
        {0, "tiktok_badge.png"_spr, "TikTok", BadgeCategory::COMMON, "tiktok_badge", true, "XJotaBeLikeX"},
        {0, "tlt.png"_spr, "The Living Tombstone", BadgeCategory::EPIC, "tlt_badge", true, "XJotaBeLikeX"},

        // shop
        {0, "Frostbite_badge.png"_spr, "Frostbite", BadgeCategory::SPECIAL, "Frostbite_badge", true, "LaFluffaroni"},
        {0, "Skybound_badge.png"_spr, "Skybound", BadgeCategory::EPIC, "Skybound_badge", true, "LaFluffaroni"},
        {0, "Steampunk_Dash_badge.png"_spr, "Steampunk", BadgeCategory::COMMON, "Steampunk_Dash_badge", true, "LaFluffaroni"},
        {0, "money_badge.png"_spr, "Money Money", BadgeCategory::SPECIAL, "money_badge", true, "LaFluffaroni"},
        {0, "Skeletal_Shenanigans_badge.png"_spr, "Skeletal Shenanigans", BadgeCategory::LEGENDARY, "Skeletal_Shenanigans_badge", true, "LaFluffaroni"},
        {0, "random_badge.png"_spr, "GD Randomizer", BadgeCategory::SPECIAL, "random_badge", true, "XJotaBeLikeX"},
        {0, "mai_badge.png"_spr, "Mai Waifu", BadgeCategory::EPIC, "mai_badge", true, "XJotaBeLikeX"},

        //moderator
        {0, "moderator_badge.png"_spr, "Moderator", BadgeCategory::LEGENDARY, "moderator_badge", true, "XJotaBeLikeX"},
        {0, "creator_badge.png"_spr, "Content creator", BadgeCategory::LEGENDARY, "creator_badge", true, "XJotaBeLikeX"},
        {0, "vip_badge.png"_spr, "V.I.P Player", BadgeCategory::MYTHIC, "vip_badge", true, "XJotaBeLikeX"},
        {0, "stellar_badge.png"_spr, "Stellar Player", BadgeCategory::MYTHIC, "stellar_badge", true, "XJotaBeLikeX"},

        //black hole
        {0, "bh_badge_1.png"_spr, "Black hole Red", BadgeCategory::EPIC, "bh_badge_1", true, "XJotaBeLikeX"},
        {0, "bh_badge_2.png"_spr, "Black hole Sky blue", BadgeCategory::SPECIAL, "bh_badge_2", true, "XJotaBeLikeX"},
        {0, "bh_badge_3.png"_spr, "Black hole purple", BadgeCategory::SPECIAL, "bh_badge_3", true, "XJotaBeLikeX"},
        {0, "bh_badge_4.png"_spr, "Black hole Blue", BadgeCategory::COMMON, "bh_badge_4", true, "XJotaBeLikeX"},
        {0, "bh_badge_5.png"_spr, "Black hole Orange", BadgeCategory::LEGENDARY, "bh_badge_5", true, "XJotaBeLikeX"},
        {0, "bh_badge_6.png"_spr, "Black hole Green", BadgeCategory::COMMON, "bh_badge_6", true, "XJotaBeLikeX"},
        {0, "bh_badge_7.png"_spr, "Black hole Ultra", BadgeCategory::MYTHIC, "bh_badge_7", true, "XJotaBeLikeX"},

        //minecraft
        {0, "mc_badge_1.png"_spr, "PVP Master", BadgeCategory::LEGENDARY, "mc_badge_1", true, "XJotaBeLikeX"},
        {0, "mc_badge_2.png"_spr, "full farming", BadgeCategory::EPIC, "mc_badge_2", true, "XJotaBeLikeX"},
        {0, "mc_badge_3.png"_spr, "break shields", BadgeCategory::COMMON, "mc_badge_3", true, "XJotaBeLikeX"},

        //events
        {0, "event_badge1.png"_spr, "Event", BadgeCategory::MYTHIC, "winter_badge", true, "XJotaBeLikeX"},
        {0, "freddy.png"_spr, "Freddy", BadgeCategory::MYTHIC, "freddy_badge", true, "XJotaBeLikeX"},
        {0, "chica.png"_spr, "chica", BadgeCategory::SPECIAL, "chica_badge", true, "XJotaBeLikeX"},
        {0, "foxi.png"_spr, "foxi", BadgeCategory::LEGENDARY, "foxi_badge", true, "XJotaBeLikeX"},
        {0, "bonnie.png"_spr, "bonnie", BadgeCategory::EPIC, "bonnie_badge", true, "XJotaBeLikeX"},

        //mastery
        {0, "cube_mastery.png"_spr, "Cube Mastery", BadgeCategory::LEGENDARY, "cube_mastery_badge", true, "XJotaBeLikeX" },
        {0, "ship_mastery.png"_spr, "Ship Mastery", BadgeCategory::LEGENDARY, "ship_mastery_badge", true, "XJotaBeLikeX" },
        {0, "ufo_mastery.png"_spr, "Ufo Mastery", BadgeCategory::LEGENDARY, "ufo_mastery_badge", true, "XJotaBeLikeX" },
        {0, "ball_mastery.png"_spr, "Ball Mastery", BadgeCategory::LEGENDARY, "ball_mastery_badge", true, "XJotaBeLikeX" },
        {0, "spider_mastery.png"_spr, "Spider Mastery", BadgeCategory::LEGENDARY, "spider_mastery_badge", true, "XJotaBeLikeX" },
        {0, "wave_mastery.png"_spr, "Wave Mastery", BadgeCategory::LEGENDARY, "wave_mastery_badge", true, "XJotaBeLikeX" },
        {0, "dual_mastery.png"_spr, "Dual Mastery", BadgeCategory::LEGENDARY, "dual_mastery_badge", true, "XJotaBeLikeX" },
        {0, "xl_mastery.png"_spr, "XL Mastery", BadgeCategory::LEGENDARY, "xl_mastery_badge", true, "XJotaBeLikeX" },
        {0, "swingcopter_mastery.png"_spr, "Swingcopter mastery Mastery", BadgeCategory::LEGENDARY, "swingcopter_mastery_badge", true, "XJotaBeLikeX" },
        {0, "memory_mastery.png"_spr, "Memory mastery Mastery", BadgeCategory::LEGENDARY, "memory_mastery_badge", true, "XJotaBeLikeX" },
        {0, "robot_mastery.png"_spr, "Robot mastery Mastery", BadgeCategory::LEGENDARY, "robot_mastery_badge", true, "XJotaBeLikeX" }
    };


    std::vector<BannerInfo> banners = {
            {"banner_1", "banner1.png"_spr, "Mm hi?", BadgeCategory::COMMON, "XJotaBeLikeX"},
            {"banner_2", "banner2.png"_spr, "Everyone!", BadgeCategory::SPECIAL, "XJotaBeLikeX"},
            {"banner_3", "banner3.png"_spr, "Never Die", BadgeCategory::EPIC, "XJotaBeLikeX"},
            {"banner_4", "banner4.png"_spr, "Damn", BadgeCategory::SPECIAL, "XJotaBeLikeX"},
            {"banner_5", "banner5.png"_spr, "Unleashed?", BadgeCategory::MYTHIC, "XJotaBeLikeX"},
            {"banner_6", "banner6.png"_spr, "Mikumikumiku", BadgeCategory::COMMON, "XJotaBeLikeX"},
            {"banner_7", "banner7.png"_spr, "So cute", BadgeCategory::LEGENDARY, "XJotaBeLikeX"},
            {"banner_8", "banner8.png"_spr, "this is...", BadgeCategory::EPIC, "XJotaBeLikeX"},
            {"banner_9", "banner9.png"_spr, "Memories", BadgeCategory::EPIC, "XJotaBeLikeX"},
            {"banner_10", "banner10.png"_spr, "peace", BadgeCategory::LEGENDARY, "XJotaBeLikeX"},
            {"banner_11", "banner11.png"_spr, "Awww", BadgeCategory::SPECIAL, "XJotaBeLikeX"},
            {"banner_12", "banner12.png"_spr, "Ok?..", BadgeCategory::COMMON, "XJotaBeLikeX"},
            {"banner_13", "banner13.png"_spr, "I am the Bloodlust", BadgeCategory::EPIC, "LaFluffaroni"},
            {"banner_14", "banner14.png"_spr, "red prison", BadgeCategory::COMMON, "LaFluffaroni"},
            {"banner_15", "banner15.png"_spr, "I always come back", BadgeCategory::MYTHIC, "LaFluffaroni"},
            {"banner_16", "banner16.png"_spr, "Miku v2", BadgeCategory::SPECIAL, "XJotaBeLikeX"},
            {"banner_17", "banner17.png"_spr, "Where am I?", BadgeCategory::LEGENDARY, "XJotaBeLikeX"},
            {"banner_18", "banner18.png"_spr, "there we go", BadgeCategory::EPIC, "XJotaBeLikeX"},
            {"banner_19", "banner19.png"_spr, "Horizon", BadgeCategory::LEGENDARY, "XJotaBeLikeX"},
            {"banner_20", "banner20.png"_spr, "Miku v3", BadgeCategory::EPIC, "XJotaBeLikeX"},
            {"banner_21", "banner21.png"_spr, "Can you hear me?", BadgeCategory::MYTHIC, "XJotaBeLikeX"},
            {"banner_22", "banner22.png"_spr, "This is familiar...", BadgeCategory::MYTHIC, "XJotaBeLikeX"},
            {"banner_23", "banner23.png"_spr, "this is life", BadgeCategory::MYTHIC, "XJotaBeLikeX"},
            {"banner_24", "banner24.png"_spr, "Youtube", BadgeCategory::LEGENDARY, "XJotaBeLikeX"},
            {"banner_25", "banner25.png"_spr, "Tiktok", BadgeCategory::LEGENDARY, "XJotaBeLikeX"},
            {"banner_26", "banner26.png"_spr, "Sky", BadgeCategory::COMMON, "XJotaBeLikeX"},
            {"banner_27", "banner27.png"_spr, "Over The Moon For You", BadgeCategory::LEGENDARY, "XJotaBeLikeX"},
            {"banner_28", "banner28.png"_spr, "garden", BadgeCategory::COMMON, "XJotaBeLikeX"},
            {"banner_29", "banner29.png"_spr, "Pink sakura", BadgeCategory::EPIC, "XJotaBeLikeX"},
            {"banner_30", "banner30.png"_spr, "Reanimate", BadgeCategory::EPIC, "XJotaBeLikeX"},
            {"banner_31", "banner31.png"_spr, "Pink sunset", BadgeCategory::EPIC, "XJotaBeLikeX"},
            {"banner_32", "banner32.png"_spr, "You and Me", BadgeCategory::LEGENDARY, "XJotaBeLikeX"},
            {"banner_33", "banner33.png"_spr, "Under the moonlight", BadgeCategory::MYTHIC, "XJotaBeLikeX"},
            {"banner_34", "banner34.png"_spr, "What lies beyond?", BadgeCategory::LEGENDARY, "XJotaBeLikeX"},
            {"banner_35", "banner35.png"_spr, "I'm still here", BadgeCategory::COMMON, "XJotaBeLikeX"},
            {"banner_36", "banner36.png"_spr, "Blue heart", BadgeCategory::EPIC, "XJotaBeLikeX"},
            {"banner_37", "banner37.png"_spr, "The Big City", BadgeCategory::SPECIAL, "XJotaBeLikeX"},
            {"banner_38", "banner38.png"_spr, "Tidal Wave", BadgeCategory::EPIC, "XJotaBeLikeX"},
            {"banner_39", "banner39.png"_spr, "Emerald Realm", BadgeCategory::LEGENDARY, "XJotaBeLikeX"},
            {"banner_40", "banner40.png"_spr, "Stellar", BadgeCategory::MYTHIC, "XJotaBeLikeX"},
            {"banner_41", "banner41.png"_spr, "V.I.P", BadgeCategory::MYTHIC, "XJotaBeLikeX"},
            {"banner_42", "banner42.png"_spr, "pink flowers", BadgeCategory::EPIC, "XJotaBeLikeX"},
            {"banner_43", "banner43.png"_spr, "light party", BadgeCategory::SPECIAL, "XJotaBeLikeX"},
            {"banner_44", "banner44.png"_spr, "magic night", BadgeCategory::MYTHIC, "XJotaBeLikeX"},
            {"banner_45", "banner45.png"_spr, "Emerald Realm II", BadgeCategory::MYTHIC, "XJotaBeLikeX"},
            {"banner_46", "banner46.png"_spr, "FNAF 2", BadgeCategory::EPIC, "XJotaBeLikeX"},
            {"banner_47", "banner47.png"_spr, "FNAF 2 Room", BadgeCategory::SPECIAL, "XJotaBeLikeX"},
            {"banner_48", "banner48.png"_spr, "what's up", BadgeCategory::COMMON, "XJotaBeLikeX"},
            {"banner_49", "banner49.png"_spr, "mountains", BadgeCategory::SPECIAL, "XJotaBeLikeX"},
            {"banner_50", "banner50.png"_spr, "Oshi No Ko", BadgeCategory::EPIC, "XJotaBeLikeX"}

            
    };


    std::vector<bool> unlockedBadges;
    std::vector<bool> unlockedBanners;


    bool isInitialized() const {
        return m_initialized && isDataLoaded;
    }

    void cacheUserBadge(int accountID, const std::string& badgeID) {
        userBadgeCache[accountID] = badgeID;
    }

    std::string getCachedBadge(int accountID) {
        if (userBadgeCache.count(accountID)) {
            return userBadgeCache[accountID];
        }
        return "";
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
    std::string getRachaSprite(int streak);
    std::string getCategoryName(BadgeCategory category);
    ccColor3B getCategoryColor(BadgeCategory category);
    BadgeInfo* getBadgeInfo(const std::string& badgeID);
    bool isBadgeUnlocked(const std::string& badgeID);
    void equipBadge(const std::string& badgeID);
    BadgeInfo* getEquippedBadge();
    bool isLevelMissionClaimed(int levelID) const;
    void unlockBanner(const std::string& bannerID);
    bool isBannerUnlocked(const std::string& bannerID);
    BannerInfo* getBannerInfo(const std::string& bannerID);
    void equipBanner(const std::string& bannerID);
    void unequipBanner();
    BannerInfo* getEquippedBanner();
    int getXPForCurrentStreak();
    int getXPRequiredForNextLevel();
    float getXPPercentage();
    void addXP(int amount);
    bool isStreakGoalClaimed(int index) const;
    void setStreakGoalClaimed(int index);
    std::vector<ShopItem> getDailyShopSelection();
    int getPriceForRarity(BadgeCategory rarity);
    void purchaseItem(const ShopItem& item);
    struct LevelRewards {
        int stars;
        int tickets;
        int gems;
    };
    LevelRewards getRewardsForLevel(int level);
};

extern StreakData g_streakData;