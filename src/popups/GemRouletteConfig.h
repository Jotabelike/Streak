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
            
            { RewardType::Banner, "banner_5", 1, "", "KOCMOC", 5, StreakData::BadgeCategory::MYTHIC }, 
            { RewardType::Badge, "past3_badge", 1, "", "Red flower", 15, StreakData::BadgeCategory::LEGENDARY },
            { RewardType::Badge, "moon_badge", 1, "", "Moon", 30, StreakData::BadgeCategory::EPIC }, 
            { RewardType::SuperStar, "super_star40", 40, "super_star.png"_spr, "40 Super Stras", 45, StreakData::BadgeCategory::SPECIAL },
            { RewardType::StarTicket, "star_ticket5k", 5000, "star_tiket.png"_spr, "5000 Tickets", 45, StreakData::BadgeCategory::SPECIAL },
            { RewardType::SuperStar, "super_star10", 10, "super_star.png"_spr, "10 Super Stars", 60, StreakData::BadgeCategory::COMMON },
            { RewardType::StarTicket, "star_ticket1200", 1200, "star_tiket.png"_spr, "1200 Star tickets", 60, StreakData::BadgeCategory::COMMON } 
        };
    }
}