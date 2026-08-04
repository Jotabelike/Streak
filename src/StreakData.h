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

inline constexpr int STREAK_MENU_MUSIC_CHANNEL = 7777;
inline constexpr int STREAK_SONG_PREVIEW_CHANNEL = 7778;
inline constexpr const char* STREAK_MENU_MUSIC_VOLUME_KEY = "streak_menu_music_volume";

struct DiscordMilestone {
    int requirement;
    int tickets;
    int stars;
    int gems;
    std::string spriteName;
    bool isChest; 
};


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
        bool excludeFromShop = false;
    };

    struct BannerInfo {
        std::string bannerID;
        std::string spriteName;
        std::string displayName;
        BadgeCategory rarity;
        std::string creator;
        bool excludeFromShop = false;
    };

    struct SongInfo {
        std::string songID;
        std::string fileName;
        std::string iconName;
        std::string displayName;
        std::string description;
        std::string creator;
    };

    struct ConsumableItem {
        bool isTickets;
        int amount;
        int price;
        BadgeCategory rarity;
    };

    struct NameCosmeticsCache {
        std::string color = "Default";
        std::string font = "Default";
        std::string effect = "None";
        std::string animation = "None";
    };

    // Definicion cruda de un premio de ruleta recibida del servidor
    // (settings/roulette_config). `type` sigue el orden de RewardType
    // (Badge=0, SuperStar=1, StarTicket=2, Banner=3) y `category` el de
    // BadgeCategory. `sprite` viene sin expandir ("_spr" se aplica al usarlo).
    struct RoulettePrizeDef {
        int type = 0;
        std::string id;
        int quantity = 1;
        std::string sprite;
        std::string name;
        int weight = 1;
        int category = 0;
    };
    std::vector<RoulettePrizeDef> serverStandardRoulette;
    std::vector<RoulettePrizeDef> serverGemRoulette;
    std::vector<int> serverGemSpinCosts;

    // Evento Mundial 2026 (prediccion de partidos). Estado recibido del
    // servidor en GET /players; los sprites vienen sin expandir (son archivos
    // de resources/wc_event, "_spr" se aplica al usarlos).
    struct WcMatch {
        std::string matchId;
        std::string teamA;
        std::string teamB;
        std::string spriteA;
        std::string spriteB;
        std::string score;    // "-" mientras se juega; luego el resultado real
        std::string status;   // "open" | "closed" | "finished"
        std::string winner;   // "a" | "b" | "draw" (solo con status finished)
        int chestRarity = 3;
        int votesA = 0;
        int votesB = 0;
        std::string myVote;   // "a" | "b" | "" si no ha votado
        bool claimed = false;
    };
    struct WcEventState {
        bool active = false;
        std::vector<WcMatch> matches;
        int correctPredictions = 0; // claims acumulados; con 4 se gana wc_2026
    };
    WcEventState wcEvent;
    // Parsea { active, matches: [...] } (de GET /players o /wc-event/state).
    static void parseWcEvent(const matjson::Value& wc, WcEventState& out);

    // Tienda de temporada (evento activable). El catalogo entero lo define el
    // servidor en settings/season_shop; el cliente solo lo dibuja y manda el
    // itemId al comprar.
    struct SeasonShopItem {
        std::string itemId;    // id de la entrada de tienda (lo que se compra)
        std::string type;      // banner|badge|name_item|song|tickets|stars|gems|shields|chest
        std::string rewardId;  // id del cosmetico (vacio en consumibles/cofres)
        std::string name;      // etiqueta opcional
        int amount = 0;        // cantidad del consumible / rareza del cofre
        int price = 0;
        std::string currency;  // gems | tickets | stars
        int stock = 0;         // compras maximas por jugador (0 = ilimitado)
        int bought = 0;        // veces que YA lo compro este jugador

        bool isCosmetic() const {
            return type == "banner" || type == "badge" || type == "name_item" || type == "song";
        }
        bool soldOut() const { return stock > 0 && bought >= stock; }
    };
    struct SeasonShopState {
        bool active = false;
        std::string shopId;
        std::string title;
        long long endsAt = 0;  // ms epoch; 0 = sin fecha de cierre
        std::vector<SeasonShopItem> items;
    };
    SeasonShopState seasonShop;
    // Parsea { active, items: [...], my_buys: {...} } (de GET /players o
    // /season-shop/state).
    static void parseSeasonShop(const matjson::Value& shop, SeasonShopState& out);

    std::vector<ConsumableItem> getDailyConsumableSelection();
    std::vector<DiscordMilestone> m_discordMilestones;
    std::vector<bool> gemRouletteState;
    std::map<int, std::string> userBadgeCache;
    int currentStreak;
    int streakPointsToday;
    std::string gemRouletteHash = "";
    int totalStreakPoints;
    bool hasNewStreak;
    bool isDiscordGoalEnabled;
    int discordCount = 0;
    int discordGoalMax = 1000;
    std::set<int> claimedDiscordMilestones;
    bool isTaskEnabled;
    std::string lastDay;
    std::string equippedBadge;
    std::string equippedBanner;
    std::string equippedSong;
    int superStars;
    int lastRouletteIndex;
    int totalSpins;
    bool needsRegistration = false;
    bool isGDPS = false;
    int lastStreakAnimated = 0;
    bool isBanned = false;
    std::string banReason = "";
    int starTickets;
    int gems;
    int fragments = 0;
    int gemRouletteSpinCount = 0;
    std::string streakID = "";
    std::vector<int> streakCompletedLevels;
    std::map<std::string, int> streakPointsHistory;
    std::set<int> claimedStreakGoals;
    int specialRank = 0;
    long long seasonEndTime = 0;
    // serverNow - localNow, calculado al cargar los datos del jugador.
    long long serverTimeOffsetMs = 0;
    // Meta de gold tickets para el premio de completar el pase. La manda el
    // server (pass_complete_goal); este valor solo es el respaldo si no viene.
    int passCompleteGoal = 2500;
    int pendingSeasonRank = 0;
    int dailyShopSeed = 0;

    std::string equippedNameAnimation = "None";
    std::string equippedNameColor = "Default";
    std::string equippedNameFont = "Default";
    std::string equippedNameEffect = "None";

    std::set<std::string> unlockedNameItems;
    std::set<std::string> claimedGemRoulettePrizes;
    std::set<std::string> claimedStandardRoulettePrizes;
    bool isNameItemUnlocked(const std::string& item);
    void unlockNameItem(const std::string& item);
    int getNameItemPrice(const std::string& item);
    static bool isEventOnlyNameItem(const std::string& item);

    std::chrono::steady_clock::time_point lastPointTime;

    int currentXP = 0;
    int currentLevel = 1;

    struct PendingLevelReward {
        int level;
        int stars;
        int tickets;
        int gems;
        int shields = 0;
        int chestRarity = 0;
    };
    std::vector<PendingLevelReward> pendingLevelRewards;
    bool hasNewMessages = false;


    bool pointMission1Claimed;
    bool pointMission2Claimed;
    bool pointMission3Claimed;
    bool pointMission4Claimed;
    bool pointMission5Claimed;
    bool pointMission6Claimed;
    bool pointMission7Claimed;
    bool pointMission8Claimed;
    bool pointMission9Claimed;
    bool pointMission10Claimed;
    bool pointMission11Claimed;
    bool pointMission12Claimed;
    bool pointMission13Claimed;

    int streakShields = 0;
    bool shieldsEnabled = false;

    int streakPointsThisWeek = 0;
    std::string lastWeek = "";

    int streakPointsThisMonth = 0;
    std::string lastMonth = "";
    std::string premiumPassMonth = "";
    std::set<int> claimedFreePassTiers;
    std::set<int> claimedPaidPassTiers;
    bool passCompleteRewardClaimed = false;
    std::string pendingPassGiftFrom = "";

    bool pendingRankAnim = false;
    int pendingRankAnimOld = 0;
    int pendingRankAnimNew = 0;

    std::string activePassID = "";
    bool passEnabled = true;
    int passPrice = 1999;

    int goldTickets = 0;
    int streakTokens = 0;

    struct PassRewardDef {
        std::string type;
        int amount = 0;
        std::string itemID;
    };
    std::vector<PassRewardDef> freePassRewards;
    std::vector<PassRewardDef> paidPassRewards;

    struct PassMissionDef {
        std::string id;
        int target = 0;
        int reward = 0;
    };
    std::vector<PassMissionDef> passDailyMissions;
    std::vector<PassMissionDef> passWeeklyMissions;
    std::vector<PassMissionDef> passSeasonMissions;

    int passDailyLevels = 0;
    int passWeeklyLevels = 0;
    int passSeasonLevels = 0;

    std::set<std::string> claimedPassDailyMissions;
    std::set<std::string> claimedPassWeeklyMissions;
    std::set<std::string> claimedPassSeasonMissions;

    bool weeklyMission1Claimed = false;
    bool weeklyMission2Claimed = false;
    bool weeklyMission3Claimed = false;
    bool weeklyMission4Claimed = false;
    bool weeklyMission5Claimed = false;
    bool weeklyMission6Claimed = false;
    bool weeklyMission7Claimed = false;
    bool weeklyMission8Claimed = false;
    bool weeklyMission9Claimed = false;
    bool weeklyMission10Claimed = false;
    bool weeklyMission11Claimed = false;

    bool isDataLoaded;
    bool m_initialized = false;
    int loadedAccountID = 0;
    int userRole = 0;
    int dailyMsgCount = 0;
    int globalRank = 0;

    std::vector<MailMessage> inbox;
    std::set<int> completedLevelMissions;

    std::map<int, std::string> userBannerCache;

    void cacheUserBanner(int accountID, const std::string& bannerID) {
        userBannerCache[accountID] = bannerID;
    }

    std::string getCachedBanner(int accountID) {
        if (userBannerCache.count(accountID)) {
            return userBannerCache[accountID];
        }
        return ""; 
    }

    std::map<std::string, std::string> taskStatuses;

    std::string getTaskStatus(const std::string& taskID) {
        if (taskStatuses.count(taskID)) return taskStatuses[taskID];
        return "";
    }

    std::map<int, NameCosmeticsCache> userNameCosmeticsCache;

    NameCosmeticsCache getCachedNameCosmetics(int accountID) {
        if (userNameCosmeticsCache.count(accountID)) {
            return userNameCosmeticsCache[accountID];
        }
        return { "Default", "Default", "None", "None" };
    }

    void cacheUserNameCosmetics(int accountID, const std::string& color, const std::string& font, const std::string& effect, const std::string& anim) {
        userNameCosmeticsCache[accountID] = { color, font, effect, anim };
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
        {0, "badge_beta.png"_spr, "Player beta?", BadgeCategory::COMMON, "beta_badge", true, "XJotaBeLikeX", true},
        {0, "badge_platino.png"_spr, "platino badge", BadgeCategory::COMMON, "platino_streak_badge", true, "XJotaBeLikeX"},
        {0, "badge_diamante_gd.png"_spr, "GD Diamond!", BadgeCategory::COMMON, "diamante_gd_badge", true, "XJotaBeLikeX"},
        {0, "badge_ncs.png"_spr, "Ncs Lover", BadgeCategory::SPECIAL, "ncs_badge", true, "XJotaBeLikeX", true},
        {0, "dark_badge1.png"_spr, "dark side", BadgeCategory::EPIC, "dark_streak_badge", true, "XJotaBeLikeX"},
        {0, "badge_diamante_mc.png"_spr, "Minecraft Diamond!", BadgeCategory::EPIC, "diamante_mc_badge", true, "XJotaBeLikeX"},
        {0, "gold_streak.png"_spr, "Gold Legend's", BadgeCategory::LEGENDARY, "gold_streak_badge", true, "XJotaBeLikeX"},
        {0, "super_star_badge.png"_spr, "First Mythic", BadgeCategory::MYTHIC, "super_star_badge", true, "XJotaBeLikeX"},
        {0, "magic_flower_badge.png"_spr, "hypnotizes with its beauty", BadgeCategory::MYTHIC, "magic_flower_badge", true, "XJotaBeLikeX"},
        {0, "hounter_badge.png"_spr, "looks familiar to me", BadgeCategory::SPECIAL, "hounter_badge", true, "XJotaBeLikeX"},
        {0, "gd_badge.png"_spr, "GD!", BadgeCategory::COMMON, "gd_badge", true, "XJotaBeLikeX"},
        {0, "adrian_badge.png"_spr, "juegajuegos", BadgeCategory::COMMON, "adrian_badge", true, "XJotaBeLikeX"},
        {0, "diamond_streak.png"_spr, "Diamond Legend's", BadgeCategory::EPIC, "diamond_streak_badge", true, "XJotaBeLikeX", true},

        {0, "past1_badge.png"_spr, "Purple Edition", BadgeCategory::MYTHIC, "past1_badge", true, "XJotaBeLikeX"},
        {0, "past2_badge.png"_spr, "green Edition", BadgeCategory::LEGENDARY, "past2_badge", true, "XJotaBeLikeX"},
        {0, "past3_badge.png"_spr, "Red Edition", BadgeCategory::LEGENDARY, "past3_badge", true, "XJotaBeLikeX"},
        {0, "moon_badge.png"_spr, "Moon", BadgeCategory::EPIC, "moon_badge", true, "XJotaBeLikeX"},
        {0, "marshmello_badge.png"_spr, "Marshmello", BadgeCategory::SPECIAL, "marshmello_badge", true, "XJotaBeLikeX", true},
        {0, "alan_walker_badge.png"_spr, "Alan Walker", BadgeCategory::LEGENDARY, "alan_walker_badge", true, "XJotaBeLikeX", true},
        {0, "teto_badge.png"_spr, "Red Miku", BadgeCategory::EPIC, "teto_badge", true, "idk", true },
        {0, "Bagde_destello1.png"_spr, "silver flash", BadgeCategory::EPIC, "bagde_destello1", true, "Cherry'wCode" },
        {0, "Bagde_destello2.png"_spr, "meow meow", BadgeCategory::SPECIAL, "bagde_destello2", true, "Cherry'wCode" },
        {0, "Bagde_mc1.png"_spr, "Minecraft Again", BadgeCategory::EPIC, "badge_mc1", true, "Cherry'wCode", true },
        {0, "Bagde_mc2.png"_spr, "Minecraft Again again", BadgeCategory::SPECIAL, "badge_mc2", true, "Cherry'wCode", true },
        {0, "Bagde_pixel1.png"_spr, "pixel eye", BadgeCategory::LEGENDARY, "badge_pixel1", true, "Cherry'wCode", true },
        {0, "Bagde_pixel2.png"_spr, "sakura flower", BadgeCategory::MYTHIC, "badge_pixel2", true, "Cherry'wCode" },
        {0, "BagdeCherry.png"_spr, "What can I say...", BadgeCategory::LEGENDARY, "badge_cherry", true, "Cherry'wCode", true },
        {0, "BagdeCherry_03.png"_spr, "Mahiru", BadgeCategory::EPIC, "badge_cherry3", true, "Cherry'wCode", true },
        {0, "BagdeCherry_04.png"_spr, "Mahiru v2", BadgeCategory::LEGENDARY, "badge_cherry4", true, "Cherry'wCode", true },
        {0, "BagdeCherry_05.png"_spr, "Rem But pink...", BadgeCategory::EPIC, "badge_cherry5", true, "Cherry'wCode", true },
        {0, "BagdeCherry_06.png"_spr, "Rem", BadgeCategory::EPIC, "badge_cherry6", true, "Cherry'wCode", true},
        {0, "BagdeCherry_08.png"_spr, "Tsukasa vlove", BadgeCategory::LEGENDARY, "badge_cherry8", true, "Cherry'wCode", true },
        {0, "BagdeCherry_09.png"_spr, "What was he going to do? ", BadgeCategory::SPECIAL, "badge_cherry9", true, "Cherry'wCode", true },
        {0, "BagdeCherry_10.png"_spr, "I don't want to get up", BadgeCategory::LEGENDARY, "badge_cherry10", true, "Cherry'wCode", true },
        {0, "omega_badge.png"_spr, "Omega", BadgeCategory::COMMON, "omega_badge", true, "XJotaBeLikeX"},
       

        //misiones
        {0, "shiver_badge.png"_spr, "Shiver!", BadgeCategory::SPECIAL, "shiver_badge", true, "XJotaBeLikeX", true},
        {0, "dual_badge.png"_spr, "Dual Master", BadgeCategory::LEGENDARY, "dual_badge", true, "XJotaBeLikeX", true},
        {0, "ttv_badge.png"_spr, "The Towerverse?", BadgeCategory::LEGENDARY, "ttv_badge", true, "XJotaBeLikeX", true},
        {0, "cos_badge.png"_spr, "Change of Scene", BadgeCategory::SPECIAL, "cos_badge", true, "XJotaBeLikeX"},
        {0, "b_badge.png"_spr, "B", BadgeCategory::SPECIAL, "b_badge", true, "XJotaBeLikeX"},
        {0, "spy_badge.png"_spr, "iSpy What?", BadgeCategory::EPIC, "spy_badge", true, "XJotaBeLikeX"},
        {0, "tsukasa_badge.png"_spr, "Tsukasitaaa", BadgeCategory::EPIC, "tsukasa_badge", true, "XJotaBeLikeX", true},
        {0, "funhouse_badge.png"_spr, "you again?", BadgeCategory::LEGENDARY, "funhouse_badge", true, "XJotaBeLikeX", true},
        {0, "cobre_badge.png"_spr, "idk", BadgeCategory::SPECIAL, "cobre_badge", true, "XJotaBeLikeX"},
        {0, "miku_badge.png"_spr, "Miku Miku oooeeooo", BadgeCategory::EPIC, "miku_badge", true, "XJotaBeLikeX", true},
        {0, "nantendo_badge.png"_spr, "Nintendo bruh", BadgeCategory::COMMON, "nantendo_badge", true, "XJotaBeLikeX", true},
        {0, "youtube_badge.png"_spr, "Youtube", BadgeCategory::COMMON, "youtube_badge", true, "XJotaBeLikeX", true},
        {0, "tiktok_badge.png"_spr, "TikTok", BadgeCategory::COMMON, "tiktok_badge", true, "XJotaBeLikeX", true},
        {0, "tlt.png"_spr, "The Living Tombstone", BadgeCategory::EPIC, "tlt_badge", true, "XJotaBeLikeX"},

        // shop
        {0, "Frostbite_badge.png"_spr, "Frostbite", BadgeCategory::SPECIAL, "Frostbite_badge", true, "LaFluffaroni"},
        {0, "Skybound_badge.png"_spr, "Skybound", BadgeCategory::EPIC, "Skybound_badge", true, "LaFluffaroni"},
        {0, "Steampunk_Dash_badge.png"_spr, "Steampunk", BadgeCategory::COMMON, "Steampunk_Dash_badge", true, "LaFluffaroni"},
        {0, "money_badge.png"_spr, "Money Money", BadgeCategory::SPECIAL, "money_badge", true, "LaFluffaroni"},
        {0, "Skeletal_Shenanigans_badge.png"_spr, "Skeletal Shenanigans", BadgeCategory::LEGENDARY, "Skeletal_Shenanigans_badge", true, "LaFluffaroni", true},
        {0, "random_badge.png"_spr, "GD Randomizer", BadgeCategory::SPECIAL, "random_badge", true, "XJotaBeLikeX"},
        {0, "mai_badge.png"_spr, "Mai Waifu", BadgeCategory::EPIC, "mai_badge", true, "XJotaBeLikeX"},

        //moderator
        {0, "moderator_badge.png"_spr, "Moderator", BadgeCategory::LEGENDARY, "moderator_badge", true, "XJotaBeLikeX", true},
        {0, "creator_badge.png"_spr, "Content creator", BadgeCategory::LEGENDARY, "creator_badge", true, "XJotaBeLikeX", true},
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
        {0, "Allay_Egg.png"_spr, "Allay Egg", BadgeCategory::MYTHIC, "allay_egg", true, "Minecraft"},
        {0, "Bat_Egg.png"_spr, "Bat Egg", BadgeCategory::MYTHIC, "bat_egg", true, "Minecraft"},
        {0, "Bee_Egg.png"_spr, "Bee Egg", BadgeCategory::EPIC, "bee_egg", true, "Minecraft"},
        {0, "Iron_Golem_Egg.png"_spr, "Iron Golem Egg", BadgeCategory::MYTHIC, "iron_golem_egg", true, "Minecraft"},
        {0, "Panda_Egg.png"_spr, "Panda Egg", BadgeCategory::LEGENDARY, "panda_egg", true, "Minecraft"},
        {0, "Sniffer_Egg.png"_spr, "Sniffer Egg", BadgeCategory::SPECIAL, "sniffer_egg", true, "Minecraft"},
        {0, "Snow_Golem_Egg.png"_spr, "Snow Golem Egg", BadgeCategory::LEGENDARY, "snow_golem_egg", true, "Minecraft"},
        {0, "The_Warden_Egg.png"_spr, "The Warden Egg", BadgeCategory::MYTHIC, "the_warden_egg", true, "Minecraft"},
        {0, "Witch_Egg.png"_spr, "Witch Egg", BadgeCategory::COMMON, "witch_egg", true, "Minecraft"},


        //events
        {0, "event_badge1.png"_spr, "Event", BadgeCategory::MYTHIC, "winter_badge", true, "XJotaBeLikeX"},
        {0, "freddy.png"_spr, "Freddy", BadgeCategory::MYTHIC, "freddy_badge", true, "XJotaBeLikeX"},
        {0, "chica.png"_spr, "chica", BadgeCategory::SPECIAL, "chica_badge", true, "XJotaBeLikeX", true},
        {0, "foxi.png"_spr, "foxi", BadgeCategory::LEGENDARY, "foxi_badge", true, "XJotaBeLikeX"},
        {0, "bonnie.png"_spr, "bonnie", BadgeCategory::EPIC, "bonnie_badge", true, "XJotaBeLikeX", true},
        {0, "limbo_badge.png"_spr, "Limbo", BadgeCategory::EPIC, "limbo_badge", true, "XJotaBeLikeX", true},

        //mastery
        {0, "cube_mastery.png"_spr, "Cube Mastery", BadgeCategory::LEGENDARY, "cube_mastery_badge", true, "XJotaBeLikeX", true },
        {0, "ship_mastery.png"_spr, "Ship Mastery", BadgeCategory::LEGENDARY, "ship_mastery_badge", true, "XJotaBeLikeX", true },
        {0, "ufo_mastery.png"_spr, "Ufo Mastery", BadgeCategory::LEGENDARY, "ufo_mastery_badge", true, "XJotaBeLikeX", true },
        {0, "ball_mastery.png"_spr, "Ball Mastery", BadgeCategory::LEGENDARY, "ball_mastery_badge", true, "XJotaBeLikeX", true },
        {0, "spider_mastery.png"_spr, "Spider Mastery", BadgeCategory::LEGENDARY, "spider_mastery_badge", true, "XJotaBeLikeX", true },
        {0, "wave_mastery.png"_spr, "Wave Mastery", BadgeCategory::LEGENDARY, "wave_mastery_badge", true, "XJotaBeLikeX", true },
        {0, "dual_mastery.png"_spr, "Dual Mastery", BadgeCategory::LEGENDARY, "dual_mastery_badge", true, "XJotaBeLikeX", true },
        {0, "xl_mastery.png"_spr, "XL Mastery", BadgeCategory::LEGENDARY, "xl_mastery_badge", true, "XJotaBeLikeX", true },
        {0, "swingcopter_mastery.png"_spr, "Swingcopter mastery Mastery", BadgeCategory::LEGENDARY, "swingcopter_mastery_badge", true, "XJotaBeLikeX", true },
        {0, "memory_mastery.png"_spr, "Memory mastery Mastery", BadgeCategory::LEGENDARY, "memory_mastery_badge", true, "XJotaBeLikeX", true },
        {0, "robot_mastery.png"_spr, "Robot mastery Mastery", BadgeCategory::LEGENDARY, "robot_mastery_badge", true, "XJotaBeLikeX", true },

        //achievements
       {0, "achievement_1.png"_spr, "Achievement Badge", BadgeCategory::SPECIAL, "achievement_badge1", true, "XJotaBeLikeX", true},
       {0, "achievement_2.png"_spr, "Achievement Badge", BadgeCategory::LEGENDARY, "achievement_badge2", true, "XJotaBeLikeX", true },
       {0, "achievement_3.png"_spr, "Achievement Badge", BadgeCategory::MYTHIC, "achievement_badge3", true, "XJotaBeLikeX" },

       //banderas
       { 0, "col.png"_spr, "Colombia", BadgeCategory::COMMON, "colombia_badge", true, "XJotaBeLikeX" },
       { 0, "usa.png"_spr, "United States", BadgeCategory::COMMON, "usa_badge", true, "XJotaBeLikeX" },
       { 0, "ven.png"_spr, "Venezuela", BadgeCategory::COMMON, "venezuela_badge", true, "XJotaBeLikeX" },
       { 0, "par.png"_spr, "Paraguay", BadgeCategory::COMMON, "paraguay_badge", true, "XJotaBeLikeX" },
       { 0, "phi.png"_spr, "Philippines", BadgeCategory::COMMON, "filipinas_badge", true, "XJotaBeLikeX" },
       { 0, "mex.png"_spr, "Mexico", BadgeCategory::COMMON, "mexico_badge", true, "XJotaBeLikeX" },
       { 0, "esp.png"_spr, "Spain", BadgeCategory::COMMON, "spain_badge", true, "XJotaBeLikeX" },
       { 0, "arg.png"_spr, "Argentina", BadgeCategory::COMMON, "arg_badge", true, "XJotaBeLikeX" },
       { 0, "nzl.png"_spr, "New Zealand", BadgeCategory::COMMON, "nzl_badge", true, "XJotaBeLikeX" },
       { 0, "bra.png"_spr, "brazil", BadgeCategory::COMMON, "bra_badge", true, "XJotaBeLikeX" },
       { 0, "eng.png"_spr, "England", BadgeCategory::COMMON, "eng_badge", true, "XJotaBeLikeX" },
       { 0, "per.png"_spr, "Peru", BadgeCategory::COMMON, "peru_badge", true, "XJotaBeLikeX" },
       { 0, "fra.png"_spr, "Francia", BadgeCategory::COMMON, "fra_badge", true, "XJotaBeLikeX" },
       { 0, "rus.png"_spr, "Russia", BadgeCategory::COMMON, "rus_badge", true, "XJotaBeLikeX" },

       //pass
       { 0, "wc_badge.png"_spr, "World Cup 2026", BadgeCategory::MYTHIC, "wc_badge", true, "XJotaBeLikeX" }

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
            {"banner_16", "banner16.png"_spr, "Miku v2", BadgeCategory::SPECIAL, "XJotaBeLikeX", true},
            {"banner_17", "banner17.png"_spr, "Where am I?", BadgeCategory::LEGENDARY, "XJotaBeLikeX"},
            {"banner_18", "banner18.png"_spr, "there we go", BadgeCategory::EPIC, "XJotaBeLikeX"},
            {"banner_19", "banner19.png"_spr, "Horizon", BadgeCategory::LEGENDARY, "XJotaBeLikeX", true},
            {"banner_20", "banner20.png"_spr, "Miku v3", BadgeCategory::EPIC, "XJotaBeLikeX"},
            {"banner_21", "banner21.png"_spr, "Can you hear me?", BadgeCategory::MYTHIC, "XJotaBeLikeX"},
            {"banner_22", "banner22.png"_spr, "This is familiar...", BadgeCategory::MYTHIC, "XJotaBeLikeX"},
            {"banner_23", "banner23.png"_spr, "this is life", BadgeCategory::MYTHIC, "XJotaBeLikeX"},
            {"banner_24", "banner24.png"_spr, "Youtube", BadgeCategory::LEGENDARY, "XJotaBeLikeX"},
            {"banner_25", "banner25.png"_spr, "Tiktok", BadgeCategory::LEGENDARY, "XJotaBeLikeX"},
            {"banner_26", "banner26.png"_spr, "Sky", BadgeCategory::COMMON, "XJotaBeLikeX", true},
            {"banner_27", "banner27.png"_spr, "Over The Moon For You", BadgeCategory::LEGENDARY, "XJotaBeLikeX"},
            {"banner_28", "banner28.png"_spr, "garden", BadgeCategory::COMMON, "XJotaBeLikeX"},
            {"banner_29", "banner29.png"_spr, "Pink sakura", BadgeCategory::EPIC, "XJotaBeLikeX", true},
            {"banner_30", "banner30.png"_spr, "Reanimate", BadgeCategory::EPIC, "XJotaBeLikeX"},
            {"banner_31", "banner31.png"_spr, "Pink sunset", BadgeCategory::EPIC, "XJotaBeLikeX"},
            {"banner_32", "banner32.png"_spr, "You and Me", BadgeCategory::LEGENDARY, "XJotaBeLikeX", true},
            {"banner_33", "banner33.png"_spr, "Under the moonlight", BadgeCategory::MYTHIC, "XJotaBeLikeX"},
            {"banner_34", "banner34.png"_spr, "What lies beyond?", BadgeCategory::LEGENDARY, "XJotaBeLikeX"},
            {"banner_35", "banner35.png"_spr, "I'm still here", BadgeCategory::COMMON, "XJotaBeLikeX"},
            {"banner_36", "banner36.png"_spr, "Blue heart", BadgeCategory::EPIC, "XJotaBeLikeX"},
            {"banner_37", "banner37.png"_spr, "The Big City", BadgeCategory::SPECIAL, "XJotaBeLikeX"},
            {"banner_38", "banner38.png"_spr, "Tidal Wave", BadgeCategory::EPIC, "XJotaBeLikeX"},
            {"banner_39", "banner39.png"_spr, "Emerald Realm", BadgeCategory::LEGENDARY, "XJotaBeLikeX"},
            {"banner_40", "banner40.png"_spr, "Stellar", BadgeCategory::MYTHIC, "XJotaBeLikeX"},
            {"banner_41", "banner41.png"_spr, "V.I.P", BadgeCategory::MYTHIC, "XJotaBeLikeX"},
            {"banner_42", "banner42.png"_spr, "pink flowers", BadgeCategory::EPIC, "XJotaBeLikeX", true},
            {"banner_43", "banner43.png"_spr, "light party", BadgeCategory::SPECIAL, "XJotaBeLikeX"},
            {"banner_44", "banner44.png"_spr, "magic night", BadgeCategory::MYTHIC, "XJotaBeLikeX"},
            {"banner_45", "banner45.png"_spr, "Emerald Realm II", BadgeCategory::MYTHIC, "XJotaBeLikeX"},
            {"banner_46", "banner46.png"_spr, "FNAF 2", BadgeCategory::EPIC, "XJotaBeLikeX"},
            {"banner_47", "banner47.png"_spr, "FNAF 2 Room", BadgeCategory::SPECIAL, "XJotaBeLikeX"},
            {"banner_48", "banner48.png"_spr, "what's up", BadgeCategory::COMMON, "XJotaBeLikeX"},
            {"banner_49", "banner49.png"_spr, "mountains", BadgeCategory::SPECIAL, "XJotaBeLikeX"},
            {"banner_50", "banner50.png"_spr, "Oshi No Ko", BadgeCategory::EPIC, "XJotaBeLikeX"},
            {"banner_51", "banner51.png"_spr, "Archievement 1", BadgeCategory::COMMON, "XJotaBeLikeX", true},
            {"banner_52", "banner52.png"_spr, "Archievement 2", BadgeCategory::LEGENDARY, "XJotaBeLikeX", true},
            {"banner_53", "banner53.png"_spr, "Archievement 3", BadgeCategory::MYTHIC, "XJotaBeLikeX"},
            {"banner_54", "banner54.png"_spr, "Space", BadgeCategory::MYTHIC, "XJotaBeLikeX"},
            {"banner_55", "banner55.png"_spr, "Dark", BadgeCategory::LEGENDARY, ""},
            {"banner_56", "banner56.png"_spr, "Pink Again", BadgeCategory::SPECIAL, ""},
            {"banner_57", "banner57.png"_spr, "what is this?", BadgeCategory::COMMON, ""},
            {"banner_58", "banner58.png"_spr, "RamRem", BadgeCategory::MYTHIC, ""},
            {"banner_59", "banner59.png"_spr, "Cristal Banner", BadgeCategory::LEGENDARY, "XJotaBeLikeX", true },
            {"banner_60", "banner60.png"_spr, "Cristal Banner II", BadgeCategory::LEGENDARY, "XJotaBeLikeX", true },
            {"banner_61", "banner61.png"_spr, "Limbo", BadgeCategory::EPIC, "XJotaBeLikeX", true},
            {"banner_62", "banner62.png"_spr, "Car", BadgeCategory::MYTHIC, "XJotaBeLikeX"},
            {"banner_63", "banner63.png"_spr, "red and yellow", BadgeCategory::LEGENDARY, ""},
            {"banner_64", "banner64.png"_spr, "ok fine", BadgeCategory::MYTHIC, ""},
            {"banner_65", "banner65.png"_spr, "Mc v....idk", BadgeCategory::COMMON, ""},
            {"banner_66", "banner66.png"_spr, "Dungeons", BadgeCategory::EPIC, ""},
            {"banner_67", "banner67.png"_spr, "Pass", BadgeCategory::MYTHIC, "XJotaBeLikeX"},


            {"banner_68", "banner68.png"_spr, "Canada Wc", BadgeCategory::LEGENDARY, "XJotaBeLikeX"},
            {"banner_69", "banner69.png"_spr, "USA Wc", BadgeCategory::LEGENDARY, "XJotaBeLikeX"},
            {"banner_70", "banner70.png"_spr, "Mexico Wc", BadgeCategory::LEGENDARY, "XJotaBeLikeX"},
            {"banner_71", "banner71.png"_spr, "We are 2026", BadgeCategory::MYTHIC, "XJotaBeLikeX"}
           
        };


    std::vector<SongInfo> songs = {
        {"song_1", "s1.mp3"_spr, "s1.png"_spr, "Streak Theme", "The original Streak! menu theme.", "Suno AI"},
        {"song_2", "s2.mp3"_spr, "s2.png"_spr, "DNA (FIFA World Cup 2026)", "WC 2026 theme.", "FIFA"},
        {"song_3", "s3.mp3"_spr, "s3.png"_spr, "otherside", "minecraft Season", "Minecraft"}
    };

    std::vector<bool> unlockedBadges;
    std::vector<bool> unlockedBanners;
    std::vector<bool> unlockedSongs;


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
    std::string getCurrentWeek();
    std::string getCurrentMonth();
    bool isPremiumPassActive();
    bool isFreePassTierClaimed(int tier) const;
    bool isPaidPassTierClaimed(int tier) const;
    void setFreePassTierClaimed(int tier);
    void setPaidPassTierClaimed(int tier);
    bool isPassActive() const;
    const std::vector<PassMissionDef>& getPassMissions(const std::string& scope) const;
    int getPassMissionProgress(const std::string& scope) const;
    bool isPassMissionClaimed(const std::string& scope, const std::string& id) const;
    void markPassMissionClaimed(const std::string& scope, const std::string& id);
    long long getSeasonEndTime() const { return seasonEndTime; }
    // Hora actual corregida con el desfase del server (ms desde epoch).
    long long getServerNowMs() const {
        return static_cast<long long>(std::time(nullptr)) * 1000LL + serverTimeOffsetMs;
    }
    int getPassCompleteGoal() const { return passCompleteGoal > 0 ? passCompleteGoal : 2500; }
    void unequipBadge();
    bool isBadgeEquipped(const std::string& badgeID);
    void dailyUpdate();
    void checkRewards();
    void addPoints(int count);
    bool shouldShowAnimation();
    std::string getRachaSprite();
    std::string getRachaSprite(int streak);

    // Rank system (streak tokens). Mirror of server.js RANK_THRESHOLDS / getRankIndexForTokens.
    static constexpr int RANK_COUNT = 12;
    static int getRankIndexForTokens(int tokens);
    static int getRankThreshold(int rankIndex);   // cumulative tokens needed for rankIndex (0..11)
    static std::string getRankSprite(int tokens);
    static std::string getRankSpriteForIndex(int rankIndex);
    static std::string getRankName(int tokens);
    static std::string getRankNameForIndex(int rankIndex);
    // Animated name-color style for the rank tier (see NameModifiers::applyColor).
    static std::string getRankColorStyle(int tokens);
    static std::string getRankColorStyleForIndex(int rankIndex);
    // Streak tokens granted when reaching a given streak day (mirror of server).
    static int getStreakTokensForDay(int day);
    // Next rank threshold above the player's current rank, or -1 if already max rank.
    static int getNextRankThreshold(int tokens);
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
    void unlockSong(const std::string& songID);
    bool isSongUnlocked(const std::string& songID);
    SongInfo* getSongInfo(const std::string& songID);
    void equipSong(const std::string& songID);
    void unequipSong();
    SongInfo* getEquippedSong();
    std::string getEquippedSongFile();
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
        int shields = 0;
        int chestRarity = 0;
    };
    LevelRewards getRewardsForLevel(int level);

    bool hasPendingLevelRewards() const { return !pendingLevelRewards.empty(); }
    void queuePendingLevelReward(int level);
    bool isLevelRewardPending(int level) const;
    bool claimPendingLevelReward(int level);
    bool hasPendingDailyMissions() const;
    bool hasPendingLevelMissions() const;
    void handleServerLevelUp(int previousLevel, int newLevel);
};

extern StreakData g_streakData;