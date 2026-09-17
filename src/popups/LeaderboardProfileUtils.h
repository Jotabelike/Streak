#pragma once

#include "ProfileCardPopup.h"
#include <matjson.hpp>

namespace LeaderboardProfileUtils {
    inline int accountID(const matjson::Value& player) {
        if (player["accountID"].isNumber()) {
            return player["accountID"].as<int>().unwrapOr(0);
        }
        if (player["accountID"].isString()) {
            try {
                return std::stoi(player["accountID"].as<std::string>().unwrapOr("0"));
            }
            catch (...) { }
        }
        return 0;
    }

    inline void show(const matjson::Value& player) {
        ProfileData data;
        data.accountID = accountID(player);
        if (data.accountID <= 0) return;

        data.username = player["username"].as<std::string>().unwrapOr("Player");
        data.currentStreak = player["current_streak_days"].as<int>().unwrapOr(0);
        data.totalSP = player["total_streak_points"].as<int>().unwrapOr(0);
        data.badgeID = player["equipped_badge_id"].as<std::string>().unwrapOr("");
        data.level = player["current_level"].as<int>().unwrapOr(1);
        data.streakTokens = player["streak_tokens"].as<int>().unwrapOr(0);
        data.bannerID = player["equipped_banner_id"].as<std::string>().unwrapOr("");
        data.nameColor = player["equipped_name_color"].as<std::string>().unwrapOr("Default");
        data.nameFont = player["equipped_name_font"].as<std::string>().unwrapOr("Default");
        data.nameEffect = player["equipped_name_effect"].as<std::string>().unwrapOr("None");
        data.nameAnimation = player["equipped_name_animation"].as<std::string>().unwrapOr("None");
        data.isPartialData = true;
        data.showGDProfileButton = true;

        if (auto popup = ProfileCardPopup::create(data)) popup->show();
    }
}
