#pragma once
#include "../StreakData.h"
#include <Geode/Geode.hpp>

using namespace geode::prelude;


enum class RewardType {
    Badge,
    SuperStar,
    StarTicket,
    Banner
};

enum class LevelRewardType {
    None,
    Badge, 
    SuperStars,
    StarTickets,
    Banner
};

struct RoulettePrize {
    RewardType type;
    std::string id;
    int quantity;
    std::string spriteName;
    std::string displayName;
    int probabilityWeight;
    StreakData::BadgeCategory category;
};

struct GenericPrizeResult {
    RewardType type;
    std::string id;
    int quantity;
    std::string displayName;
    std::string spriteName;
    StreakData::BadgeCategory category;
    bool isNew = false;
    int ticketsFromDuplicate = 0;
};

struct LevelMission {
    int levelID;
    std::string levelName;
    std::string rewardBadgeID;
    LevelRewardType secondaryRewardType;
    int secondaryRewardQuantity;
    std::string secondaryRewardSprite;
    std::string secondaryRewardDisplayName;
};


inline ccColor3B getBrightQualityColor(StreakData::BadgeCategory category) {
    switch (category) {
    case StreakData::BadgeCategory::COMMON:   return ccc3(220, 220, 220);
    case StreakData::BadgeCategory::SPECIAL:  return ccc3(0, 255, 80);
    case StreakData::BadgeCategory::EPIC:     return ccc3(255, 0, 255);
    case StreakData::BadgeCategory::LEGENDARY:return ccc3(255, 200, 0);
    case StreakData::BadgeCategory::MYTHIC:   return ccc3(255, 60, 60);
    default:                                  return ccc3(255, 255, 255);
    }
}


inline std::vector<LevelMission> g_levelMissions = {
    {44062068,  "Future Funk",         "dual_badge",    LevelRewardType::StarTickets,  7500,     "star_tiket.png"_spr,    "+7500 Tickets"},
    {34085027,  "B",                   "b_badge",       LevelRewardType::None,          0,     "",                      ""},
    {125070012, "The Towerverse II",   "ttv_badge",     LevelRewardType::SuperStars,    40,     "super_star.png"_spr,    "+40 Super Stars"},
    {90475473,  "Change of Scene",     "cos_badge",     LevelRewardType::None,          0,     "",                      ""},
    {89886591,  "iSpyWithMyLittleEye", "spy_badge",     LevelRewardType::StarTickets,  3000,      "star_tiket.png"_spr,   "+3000 Tickets"},
    {56210242,  "Shiver",              "shiver_badge",  LevelRewardType::SuperStars,    35,      "super_star.png"_spr,   "+35 Super Stars"},
    {58270823,  "Nantendo",            "nantendo_badge",LevelRewardType::StarTickets,  8300,     "star_tiket.png"_spr,   "+8300 Star tickets"},
    {126094460, "Funhouse",            "funhouse_badge",LevelRewardType::SuperStars,    20,     "super_star.png"_spr,   "+20 Super Stars"},
    {114832829, "Miku Party",          "miku_badge",    LevelRewardType::SuperStars,    20,     "super_star.png"_spr,   "+20 Super Stars"},
    {96978035,  "Oxidize",             "cobre_badge",   LevelRewardType::SuperStars,   60,     "super_star.png"_spr,   "+60 Super Stars"},
    {103032383, "Koi no uta",          "tsukasa_badge", LevelRewardType::StarTickets,   7000,    "star_tiket.png"_spr,   "+7000 Star Tickets"},
    {113469866, "Skybound",            "Skybound_badge",LevelRewardType::SuperStars,     10,      "super_star.png"_spr,        "+10 Super Stars"},
    {118509879, "Skeletal Shenanigans","Skeletal_Shenanigans_badge", LevelRewardType::SuperStars, 75, "super_star.png"_spr, "+75 Super Stars"},
    {112690334, "Frostbite",           "Frostbite_badge", LevelRewardType::SuperStars,   15,      "super_star.png"_spr,        "+15 Star Tickets" },
	{126765939, "GD Randomizer",      "random_badge", LevelRewardType::StarTickets, 4000, "star_tiket.png"_spr, "+4000 Star Tickets" },
    {38637027, "Wild","tlt_badge", LevelRewardType::StarTickets, 8000, "star_tiket.png"_spr, "+8000 Star Tickets" },
    {109508844, "Stargaze","ncs_badge", LevelRewardType::SuperStars, 40, "super_star.png"_spr, "+40 Super Stars" }
  


};