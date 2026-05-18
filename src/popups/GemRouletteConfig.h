#pragma once
#include <Geode/Geode.hpp>
#include "StreakCommon.h"
#include <vector>

using namespace geode::prelude;

namespace GemRouletteConfig {

    static const std::vector<int> SPIN_COSTS = {
        9, 19, 49, 199, 249, 399, 459
    };

    static int getCostForStep(int step) {
        if (step < 0) return SPIN_COSTS[0];
        if (step >= SPIN_COSTS.size()) return SPIN_COSTS.back();
        return SPIN_COSTS[step];
    }

    static std::vector<RoulettePrize> getPrizes() {
        return {
            
            { RewardType::Banner, "banner_58", 1, "", "RamRem", 5, StreakData::BadgeCategory::MYTHIC }, 
            { RewardType::Badge, "badge_cherry8", 1, "", "Tsukasa", 15, StreakData::BadgeCategory::LEGENDARY },
            { RewardType::Badge, "badge_cherry5", 1, "", "Ram", 30, StreakData::BadgeCategory::EPIC }, 
            { RewardType::SuperStar, "super_star50", 50, "super_star.png"_spr, "50 Super Stras", 45, StreakData::BadgeCategory::SPECIAL },
            { RewardType::StarTicket, "star_ticket7k", 7000, "star_tiket.png"_spr, "7000 Tickets", 45, StreakData::BadgeCategory::SPECIAL },
            { RewardType::SuperStar, "super_star5", 5, "super_star.png"_spr, "5 Super Stars", 60, StreakData::BadgeCategory::COMMON },
            { RewardType::StarTicket, "star_ticket1500", 1500, "star_tiket.png"_spr, "1500 Star tickets", 60, StreakData::BadgeCategory::COMMON } 
        };
    }
}