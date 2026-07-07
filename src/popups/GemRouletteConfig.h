#pragma once
#include <Geode/Geode.hpp>
#include "StreakCommon.h"
#include "../StreakData.h"
#include <vector>

using namespace geode::prelude;

namespace GemRouletteConfig {

    // Fallback si aun no llego la config del servidor (settings/roulette_config).
    static const std::vector<int> SPIN_COSTS = {
        9, 19, 49, 199, 249, 399, 459
    };

    static int getCostForStep(int step) {
        const auto& costs = g_streakData.serverGemSpinCosts.size() == 7
            ? g_streakData.serverGemSpinCosts : SPIN_COSTS;
        if (step < 0) return costs[0];
        if (step >= (int)costs.size()) return costs.back();
        return costs[step];
    }

    // Convierte las definiciones crudas del servidor a RoulettePrize.
    // Para badges/banners el sprite sale del catalogo local; para monedas se
    // usa el sprite que manda el servidor (expandido al namespace del mod).
    static std::vector<RoulettePrize> fromServerDefs(const std::vector<StreakData::RoulettePrizeDef>& defs) {
        std::vector<RoulettePrize> prizes;
        prizes.reserve(defs.size());
        for (const auto& def : defs) {
            RoulettePrize p;
            p.type = static_cast<RewardType>(def.type);
            p.id = def.id;
            p.quantity = def.quantity;
            p.spriteName = def.sprite.empty()
                ? std::string("")
                : std::string(Mod::get()->expandSpriteName(def.sprite));
            p.displayName = def.name;
            p.probabilityWeight = def.weight;
            p.category = static_cast<StreakData::BadgeCategory>(def.category);
            prizes.push_back(p);
        }
        return prizes;
    }

    static std::vector<RoulettePrize> getPrizes() {
        // La ruleta de gemas requiere exactamente 7 premios (layout y estado
        // claimed son fijos); si el servidor mando otra cosa, fallback local.
        if (g_streakData.serverGemRoulette.size() == 7) {
            return fromServerDefs(g_streakData.serverGemRoulette);
        }
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
