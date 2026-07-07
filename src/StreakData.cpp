#include "StreakData.h"
#include "FirebaseManager.h"
#include <Geode/utils/cocos.hpp>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <cmath>
#include <Geode/binding/GJAccountManager.hpp>
#include <Geode/binding/GameLevelManager.hpp>
#include <algorithm>
#include <cctype>
#include "SystemNotification.h"
#include "RewardNotification.h"
#include "popups/StreakCommon.h"
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
    isGDPS = false;
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
    fragments = 0;
    specialRank = 0;
    lastRouletteIndex = 0;
    totalSpins = 0;
    currentXP = 0;
    gemRouletteSpinCount = 0;
    gemRouletteState.assign(7, false);
    currentLevel = 1;
    isDiscordGoalEnabled = false;
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
    pointMission7Claimed = false;
    pointMission8Claimed = false;
    pointMission9Claimed = false;
    pointMission10Claimed = false;
    pointMission11Claimed = false;
    pointMission12Claimed = false;
    pointMission13Claimed = false;

    streakShields = 0;
    shieldsEnabled = false;
    streakPointsThisWeek = 0;
    lastWeek = "";

    streakPointsThisMonth = 0;
    lastMonth = "";
    premiumPassMonth = "";
    claimedFreePassTiers.clear();
    claimedPaidPassTiers.clear();
    passCompleteRewardClaimed = false;
    pendingPassGiftFrom = "";
    pendingRankAnim = false;
    pendingRankAnimOld = 0;
    pendingRankAnimNew = 0;

    goldTickets = 0;
    streakTokens = 0;
    passDailyLevels = 0;
    passWeeklyLevels = 0;
    passSeasonLevels = 0;
    passDailyMissions.clear();
    passWeeklyMissions.clear();
    passSeasonMissions.clear();
    claimedPassDailyMissions.clear();
    claimedPassWeeklyMissions.clear();
    claimedPassSeasonMissions.clear();

    weeklyMission1Claimed = false;
    weeklyMission2Claimed = false;
    weeklyMission3Claimed = false;
    weeklyMission4Claimed = false;
    weeklyMission5Claimed = false;
    weeklyMission6Claimed = false;
    weeklyMission7Claimed = false;
    weeklyMission8Claimed = false;
    weeklyMission9Claimed = false;
    weeklyMission10Claimed = false;
    weeklyMission11Claimed = false;


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

    equippedSong = "";
    if (unlockedSongs.size() != songs.size()) {
        unlockedSongs.assign(songs.size(), false);
    }
    else {
        std::fill(unlockedSongs.begin(), unlockedSongs.end(), false);
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

void StreakData::parseWcEvent(const matjson::Value& wc, WcEventState& out) {
    out.active = wc["active"].as<bool>().unwrapOr(false);
    out.correctPredictions = wc["correct_predictions"].as<int>().unwrapOr(0);
    out.matches.clear();
    auto arr = wc["matches"].as<std::vector<matjson::Value>>();
    if (arr.isOk()) {
        for (auto& m : arr.unwrap()) {
            WcMatch match;
            match.matchId = m["match_id"].as<std::string>().unwrapOr("");
            if (match.matchId.empty()) continue;
            match.teamA = m["team_a"].as<std::string>().unwrapOr("");
            match.teamB = m["team_b"].as<std::string>().unwrapOr("");
            match.spriteA = m["sprite_a"].as<std::string>().unwrapOr("");
            match.spriteB = m["sprite_b"].as<std::string>().unwrapOr("");
            match.score = m["score"].as<std::string>().unwrapOr("-");
            match.status = m["status"].as<std::string>().unwrapOr("open");
            match.winner = m["winner"].as<std::string>().unwrapOr("");
            match.chestRarity = m["chest_rarity"].as<int>().unwrapOr(3);
            match.votesA = m["votes_a"].as<int>().unwrapOr(0);
            match.votesB = m["votes_b"].as<int>().unwrapOr(0);
            match.myVote = m["my_vote"].as<std::string>().unwrapOr("");
            match.claimed = m["claimed"].as<bool>().unwrapOr(false);
            out.matches.push_back(match);
        }
    }
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
    equippedSong = data["equipped_song_id"].as<std::string>().unwrapOr(std::string(""));
    superStars = safeInt(data, "super_stars", 0);
    starTickets = safeInt(data, "star_tickets", 0);
    lastRouletteIndex = safeInt(data, "last_roulette_index", 0);
    discordCount = safeInt(data, "discord_count", 0);
    discordGoalMax = safeInt(data, "discord_goal_max", 1000);
    totalSpins = safeInt(data, "total_spins", 0);
    
    if (data.contains("claimed_discord_milestones")) {
        auto goalsResult = data["claimed_discord_milestones"].as<std::vector<int>>();
        if (goalsResult.isOk()) {
            for (int g : goalsResult.unwrap()) {
                claimedDiscordMilestones.insert(g);
            }
        }
    }
    lastDay = data["lastDay"].as<std::string>().unwrapOr(std::string(""));
    if (lastDay.empty() && data.contains("last_day")) {
        lastDay = data["last_day"].as<std::string>().unwrapOr(std::string(""));
    }
    streakPointsToday = safeInt(data, "streakPointsToday", 0);
    streakPointsThisWeek = safeInt(data, "streakPointsThisWeek", 0);
    lastWeek = data["lastWeek"].as<std::string>().unwrapOr(std::string(""));
    streakPointsThisMonth = safeInt(data, "streakPointsThisMonth", 0);
    lastMonth = data["lastMonth"].as<std::string>().unwrapOr(std::string(""));
    premiumPassMonth = data["premium_pass_month"].as<std::string>().unwrapOr(std::string(""));
    activePassID = data["active_pass_id"].as<std::string>().unwrapOr(std::string(""));
    passEnabled = data["pass_enabled"].as<bool>().unwrapOr(true);
    passPrice = data["pass_price"].as<int>().unwrapOr(1999);
    if (passPrice < 0) passPrice = 0;
    if (data.contains("season_end_time")) {
        auto v = data["season_end_time"];
        if (v.isNumber()) seasonEndTime = v.as<long long>().unwrapOr(seasonEndTime);
    }

    freePassRewards.clear();
    paidPassRewards.clear();
    if (data.contains("pass_rewards")) {
        auto pr = data["pass_rewards"];
        auto parseList = [&](const matjson::Value& list, std::vector<PassRewardDef>& out) {
            if (!list.isArray()) return;
            auto vec = list.as<std::vector<matjson::Value>>();
            if (!vec.isOk()) return;
            for (const auto& item : vec.unwrap()) {
                PassRewardDef d;
                d.type = item["type"].as<std::string>().unwrapOr(std::string(""));
                d.amount = item["amount"].as<int>().unwrapOr(0);
                d.itemID = item["item_id"].as<std::string>().unwrapOr(std::string(""));
                out.push_back(d);
            }
        };
        if (pr.contains("free")) parseList(pr["free"], freePassRewards);
        if (pr.contains("paid")) parseList(pr["paid"], paidPassRewards);
    }

    claimedFreePassTiers.clear();
    if (data.contains("claimed_free_pass_tiers")) {
        auto v = data["claimed_free_pass_tiers"];
        if (v.isArray()) {
            for (const auto& val : v.as<std::vector<matjson::Value>>().unwrap()) {
                claimedFreePassTiers.insert(val.as<int>().unwrapOr(-1));
            }
        } else if (v.isObject()) {
            for (const auto& [k, _] : v.as<std::map<std::string, matjson::Value>>().unwrap()) {
                try { claimedFreePassTiers.insert(std::stoi(k)); } catch (...) {}
            }
        }
    }
    claimedPaidPassTiers.clear();
    if (data.contains("claimed_paid_pass_tiers")) {
        auto v = data["claimed_paid_pass_tiers"];
        if (v.isArray()) {
            for (const auto& val : v.as<std::vector<matjson::Value>>().unwrap()) {
                claimedPaidPassTiers.insert(val.as<int>().unwrapOr(-1));
            }
        } else if (v.isObject()) {
            for (const auto& [k, _] : v.as<std::map<std::string, matjson::Value>>().unwrap()) {
                try { claimedPaidPassTiers.insert(std::stoi(k)); } catch (...) {}
            }
        }
    }
    passCompleteRewardClaimed = data["pass_complete_reward_claimed"].as<bool>().unwrapOr(false);
    pendingPassGiftFrom = "";
    if (data.contains("pending_pass_gift")) {
        auto gift = data["pending_pass_gift"];
        if (gift.isObject()) {
            pendingPassGiftFrom = gift["from"].as<std::string>().unwrapOr(std::string("A player"));
        }
    }

    pendingRankAnim = false;
    pendingRankAnimOld = 0;
    pendingRankAnimNew = 0;
    if (data.contains("pending_rank_anim")) {
        auto anim = data["pending_rank_anim"];
        if (anim.isObject()) {
            pendingRankAnim = true;
            pendingRankAnimOld = anim["old"].as<int>().unwrapOr(0);
            pendingRankAnimNew = anim["new"].as<int>().unwrapOr(0);
        }
    }
    goldTickets = safeInt(data, "gold_tickets", 0);
    streakTokens = safeInt(data, "streak_tokens", 0);
    passDailyLevels = safeInt(data, "pass_daily_levels", 0);
    passWeeklyLevels = safeInt(data, "pass_weekly_levels", 0);
    passSeasonLevels = safeInt(data, "pass_season_levels", 0);

    auto parseMissionDefs = [&](const matjson::Value& list, std::vector<PassMissionDef>& out) {
        out.clear();
        if (!list.isArray()) return;
        auto vec = list.as<std::vector<matjson::Value>>();
        if (!vec.isOk()) return;
        for (const auto& item : vec.unwrap()) {
            PassMissionDef d;
            d.id = item["id"].as<std::string>().unwrapOr(std::string(""));
            d.target = item["target"].as<int>().unwrapOr(0);
            d.reward = item["reward"].as<int>().unwrapOr(0);
            if (!d.id.empty() && d.target > 0) out.push_back(d);
        }
    };
    passDailyMissions.clear();
    passWeeklyMissions.clear();
    passSeasonMissions.clear();
    if (data.contains("pass_missions")) {
        auto pm = data["pass_missions"];
        if (pm.contains("daily"))  parseMissionDefs(pm["daily"], passDailyMissions);
        if (pm.contains("weekly")) parseMissionDefs(pm["weekly"], passWeeklyMissions);
        if (pm.contains("season")) parseMissionDefs(pm["season"], passSeasonMissions);
    }

    auto parseClaimedMissions = [&](const matjson::Value& v, std::set<std::string>& out) {
        out.clear();
        if (v.isArray()) {
            for (const auto& val : v.as<std::vector<matjson::Value>>().unwrap()) {
                out.insert(val.as<std::string>().unwrapOr(std::string("")));
            }
        } else if (v.isObject()) {
            for (const auto& [k, _] : v.as<std::map<std::string, matjson::Value>>().unwrap()) {
                out.insert(k);
            }
        }
        out.erase("");
    };
    if (data.contains("claimed_pass_daily_missions"))  parseClaimedMissions(data["claimed_pass_daily_missions"], claimedPassDailyMissions);
    else claimedPassDailyMissions.clear();
    if (data.contains("claimed_pass_weekly_missions")) parseClaimedMissions(data["claimed_pass_weekly_missions"], claimedPassWeeklyMissions);
    else claimedPassWeeklyMissions.clear();
    if (data.contains("claimed_pass_season_missions")) parseClaimedMissions(data["claimed_pass_season_missions"], claimedPassSeasonMissions);
    else claimedPassSeasonMissions.clear();

    streakShields = safeInt(data, "streak_shields", 0);
    shieldsEnabled = data["shields_enabled"].as<bool>().unwrapOr(false);
    gems = safeInt(data, "gems", 0);
    fragments = safeInt(data, "fragments", 0);
    gemRouletteSpinCount = safeInt(data, "gem_roulette_spin_count", 0);
    isGDPS = data["isGDPS"].as<bool>().unwrapOr(false);
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

    if (data.contains("discord_goal_enabled")) {
        isDiscordGoalEnabled = data["discord_goal_enabled"].as<bool>().unwrapOr(false);
    }
    else {
        isDiscordGoalEnabled = false;
    }

    m_discordMilestones.clear();
    if (data.contains("discord_milestones")) {
        auto list = data["discord_milestones"].as<std::vector<matjson::Value>>();
        if (list.isOk()) {
            for (auto& item : list.unwrap()) {
                m_discordMilestones.push_back({
                    item["req"].as<int>().unwrapOr(0),
                    item["tickets"].as<int>().unwrapOr(0),
                    item["stars"].as<int>().unwrapOr(0),
                    item["gems"].as<int>().unwrapOr(0),
                    item["spr"].as<std::string>().unwrapOr(""),
                    item["isChest"].as<bool>().unwrapOr(false)  
                    });
            }
        }
    }

    if (data.contains("roulette_config")) {
        auto parsePrizeList = [](const matjson::Value& listVal, std::vector<RoulettePrizeDef>& out) {
            out.clear();
            auto list = listVal.as<std::vector<matjson::Value>>();
            if (!list.isOk()) return;
            for (auto& item : list.unwrap()) {
                RoulettePrizeDef def;
                std::string type = item["type"].as<std::string>().unwrapOr("");
                if (type == "badge") def.type = 0;
                else if (type == "super_star") def.type = 1;
                else if (type == "star_ticket") def.type = 2;
                else if (type == "banner") def.type = 3;
                else continue;
                def.id = item["id"].as<std::string>().unwrapOr("");
                if (def.id.empty()) continue;
                def.quantity = item["quantity"].as<int>().unwrapOr(1);
                def.sprite = item["sprite"].as<std::string>().unwrapOr("");
                def.name = item["name"].as<std::string>().unwrapOr("");
                def.weight = item["weight"].as<int>().unwrapOr(1);
                std::string cat = item["category"].as<std::string>().unwrapOr("common");
                if (cat == "special") def.category = 1;
                else if (cat == "epic") def.category = 2;
                else if (cat == "legendary") def.category = 3;
                else if (cat == "mythic") def.category = 4;
                else def.category = 0;
                out.push_back(def);
            }
        };
        auto cfg = data["roulette_config"];
        parsePrizeList(cfg["standard"], serverStandardRoulette);
        parsePrizeList(cfg["gem"], serverGemRoulette);
        serverGemSpinCosts = cfg["gem_costs"].as<std::vector<int>>().unwrapOr(std::vector<int>{});
    }

    wcEvent = WcEventState{};
    if (data.contains("wc_event")) {
        parseWcEvent(data["wc_event"], wcEvent);
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

    if (unlockedSongs.size() != songs.size()) {
        unlockedSongs.assign(songs.size(), false);
    }
    else {
        std::fill(unlockedSongs.begin(), unlockedSongs.end(), false);
    }

    if (data.contains("unlocked_songs")) {
        auto songsResult = data["unlocked_songs"].as<std::vector<matjson::Value>>();
        if (songsResult.isOk()) {
            for (const auto& song_id_json : songsResult.unwrap()) {
                unlockSong(song_id_json.as<std::string>().unwrapOr(std::string("")));
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
    pointMission7Claimed = false;
    pointMission8Claimed = false;
    pointMission9Claimed = false;
    pointMission10Claimed = false;
    pointMission11Claimed = false;
    pointMission12Claimed = false;
    pointMission13Claimed = false;

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
            if (m.count("pm7"))  pointMission7Claimed  = m.at("pm7").as<bool>().unwrapOr(false);
            if (m.count("pm8"))  pointMission8Claimed  = m.at("pm8").as<bool>().unwrapOr(false);
            if (m.count("pm9"))  pointMission9Claimed  = m.at("pm9").as<bool>().unwrapOr(false);
            if (m.count("pm10")) pointMission10Claimed = m.at("pm10").as<bool>().unwrapOr(false);
            if (m.count("pm11")) pointMission11Claimed = m.at("pm11").as<bool>().unwrapOr(false);
            if (m.count("pm12")) pointMission12Claimed = m.at("pm12").as<bool>().unwrapOr(false);
            if (m.count("pm13")) pointMission13Claimed = m.at("pm13").as<bool>().unwrapOr(false);
        }
    }

    weeklyMission1Claimed = false;
    weeklyMission2Claimed = false;
    weeklyMission3Claimed = false;
    weeklyMission4Claimed = false;
    weeklyMission5Claimed = false;
    weeklyMission6Claimed = false;
    weeklyMission7Claimed = false;
    weeklyMission8Claimed = false;
    weeklyMission9Claimed = false;
    weeklyMission10Claimed = false;
    weeklyMission11Claimed = false;

    if (data.contains("weeklyMissions")) {
        auto wmResult = data["weeklyMissions"].as<std::map<std::string, matjson::Value>>();
        if (wmResult.isOk()) {
            auto wm = wmResult.unwrap();
            if (wm.count("wm1"))  weeklyMission1Claimed  = wm.at("wm1").as<bool>().unwrapOr(false);
            if (wm.count("wm2"))  weeklyMission2Claimed  = wm.at("wm2").as<bool>().unwrapOr(false);
            if (wm.count("wm3"))  weeklyMission3Claimed  = wm.at("wm3").as<bool>().unwrapOr(false);
            if (wm.count("wm4"))  weeklyMission4Claimed  = wm.at("wm4").as<bool>().unwrapOr(false);
            if (wm.count("wm5"))  weeklyMission5Claimed  = wm.at("wm5").as<bool>().unwrapOr(false);
            if (wm.count("wm6"))  weeklyMission6Claimed  = wm.at("wm6").as<bool>().unwrapOr(false);
            if (wm.count("wm7"))  weeklyMission7Claimed  = wm.at("wm7").as<bool>().unwrapOr(false);
            if (wm.count("wm8"))  weeklyMission8Claimed  = wm.at("wm8").as<bool>().unwrapOr(false);
            if (wm.count("wm9"))  weeklyMission9Claimed  = wm.at("wm9").as<bool>().unwrapOr(false);
            if (wm.count("wm10")) weeklyMission10Claimed = wm.at("wm10").as<bool>().unwrapOr(false);
            if (wm.count("wm11")) weeklyMission11Claimed = wm.at("wm11").as<bool>().unwrapOr(false);
        }
    }

    streakPointsHistory.clear();
    if (data.contains("history")) {
        auto parseLeaf = [&](const std::string& date, const matjson::Value& val) {
            if (val.isNumber()) streakPointsHistory[date] = val.as<int>().unwrapOr(0);
            else if (val.isString()) {
                try { streakPointsHistory[date] = std::stoi(val.as<std::string>().unwrapOr(std::string("0"))); }
                catch (...) { streakPointsHistory[date] = 0; }
            }
        };

        auto h = data["history"].as<std::map<std::string, matjson::Value>>();
        if (h.isOk()) {
            for (const auto& [key, val] : h.unwrap()) {
                if (val.isObject()) {
                    auto inner = val.as<std::map<std::string, matjson::Value>>();
                    if (inner.isOk()) {
                        for (const auto& [date, dayVal] : inner.unwrap()) {
                            parseLeaf(date, dayVal);
                        }
                    }
                } else {
                    parseLeaf(key, val);
                }
            }
        }
    }

    claimedGemRoulettePrizes.clear();
    if (data.contains("claimed_gem_roulette_prizes")) {
        auto arr = data["claimed_gem_roulette_prizes"].as<std::vector<matjson::Value>>();
        if (arr.isOk()) {
            for (const auto& item : arr.unwrap()) {
                std::string s = item.as<std::string>().unwrapOr(std::string(""));
                if (!s.empty()) claimedGemRoulettePrizes.insert(s);
            }
        }
    }

    claimedStandardRoulettePrizes.clear();
    if (data.contains("claimed_standard_roulette_prizes")) {
        auto arr = data["claimed_standard_roulette_prizes"].as<std::vector<matjson::Value>>();
        if (arr.isOk()) {
            for (const auto& item : arr.unwrap()) {
                std::string s = item.as<std::string>().unwrapOr(std::string(""));
                if (!s.empty()) claimedStandardRoulettePrizes.insert(s);
            }
        }
    }

    pendingLevelRewards.clear();
    if (data.contains("pending_level_rewards")) {
        auto arr = data["pending_level_rewards"].as<std::vector<matjson::Value>>();
        if (arr.isOk()) {
            for (const auto& item : arr.unwrap()) {
                int lvl = item["level"].as<int>().unwrapOr(0);
                if (lvl <= 0) continue;
                auto r = getRewardsForLevel(lvl);
                PendingLevelReward p;
                p.level = lvl;
                p.stars       = item["stars"].as<int>().unwrapOr(r.stars);
                p.tickets     = item["tickets"].as<int>().unwrapOr(r.tickets);
                p.gems        = item["gems"].as<int>().unwrapOr(r.gems);
                p.shields     = item["shields"].as<int>().unwrapOr(r.shields);
                p.chestRarity = item["chestRarity"].as<int>().unwrapOr(r.chestRarity);
                pendingLevelRewards.push_back(p);
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

    bool isModOrAdmin = (userRole >= 1);
    for (size_t i = 0; i < badges.size(); ++i) {
        if (badges[i].badgeID == "moderator_badge") {
            if (i < unlockedBadges.size()) {
                unlockedBadges[i] = isModOrAdmin;
            }
            break;
        }
    }

 
    if (!isModOrAdmin && equippedBadge == "moderator_badge") {
        equippedBadge = "";
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
    t -= 5 * 3600;
    tm* now = gmtime(&t);
    if (!now) return "";
    char buf[16];
    if (strftime(buf, sizeof(buf), "%F", now) == 0) return "";
    return std::string(buf);
}

std::string StreakData::getCurrentMonth() {
    time_t t = time(nullptr);
    t -= 5 * 3600;
    tm* now = gmtime(&t);
    if (!now) return "";
    char buf[16];
    if (strftime(buf, sizeof(buf), "%Y-%m", now) == 0) return "";
    return std::string(buf);
}

bool StreakData::isPremiumPassActive() {
    if (premiumPassMonth.empty()) return false;
    return premiumPassMonth == getCurrentMonth();
}

bool StreakData::isPassActive() const {
    if (!passEnabled) return false;
    if (activePassID.empty()) return false;
    return true;
}

const std::vector<StreakData::PassMissionDef>& StreakData::getPassMissions(const std::string& scope) const {
    if (scope == "weekly") return passWeeklyMissions;
    if (scope == "season") return passSeasonMissions;
    return passDailyMissions;
}

int StreakData::getPassMissionProgress(const std::string& scope) const {
    if (scope == "weekly") return passWeeklyLevels;
    if (scope == "season") return passSeasonLevels;
    return passDailyLevels;
}

bool StreakData::isPassMissionClaimed(const std::string& scope, const std::string& id) const {
    if (scope == "weekly") return claimedPassWeeklyMissions.count(id) > 0;
    if (scope == "season") return claimedPassSeasonMissions.count(id) > 0;
    return claimedPassDailyMissions.count(id) > 0;
}

void StreakData::markPassMissionClaimed(const std::string& scope, const std::string& id) {
    if (scope == "weekly") claimedPassWeeklyMissions.insert(id);
    else if (scope == "season") claimedPassSeasonMissions.insert(id);
    else claimedPassDailyMissions.insert(id);
}

bool StreakData::isFreePassTierClaimed(int tier) const {
    return claimedFreePassTiers.count(tier) > 0;
}

bool StreakData::isPaidPassTierClaimed(int tier) const {
    return claimedPaidPassTiers.count(tier) > 0;
}

void StreakData::setFreePassTierClaimed(int tier) {
    claimedFreePassTiers.insert(tier);
}

void StreakData::setPaidPassTierClaimed(int tier) {
    claimedPaidPassTiers.insert(tier);
}

std::string StreakData::getCurrentWeek() {
    time_t t = time(nullptr);
    t -= 5 * 3600;
    tm* now = gmtime(&t);
    if (!now) return "";

    int weekday = now->tm_wday;
    int daysSinceMonday = (weekday == 0) ? 6 : (weekday - 1);

    time_t monday = t - (time_t)daysSinceMonday * 86400;
    tm* mondayTm = gmtime(&monday);
    if (!mondayTm) return "";

    char buf[16];
    if (strftime(buf, sizeof(buf), "%F", mondayTm) == 0) return "";
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
    std::string currentWeek = getCurrentWeek();
    std::string currentMonth = getCurrentMonth();
    if (today.empty()) return;

    auto resetWeeklyIfNeeded = [&]() {
        if (currentWeek.empty()) return;
        if (lastWeek == currentWeek) return;
        lastWeek = currentWeek;
        streakPointsThisWeek = 0;
        weeklyMission1Claimed = false;
        weeklyMission2Claimed = false;
        weeklyMission3Claimed = false;
        weeklyMission4Claimed = false;
        weeklyMission5Claimed = false;
        weeklyMission6Claimed = false;
        weeklyMission7Claimed = false;
        weeklyMission8Claimed = false;
        weeklyMission9Claimed = false;
        weeklyMission10Claimed = false;
        weeklyMission11Claimed = false;
    };

    auto resetMonthlyIfNeeded = [&]() {
        if (currentMonth.empty()) return;
        if (lastMonth == currentMonth) return;
        lastMonth = currentMonth;
        streakPointsThisMonth = 0;
    };

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
        resetWeeklyIfNeeded();
        resetMonthlyIfNeeded();
        save();
        return;
    }

    if (lastDay == today) {
        bool needSave = false;
        if (lastWeek != currentWeek) {
            resetWeeklyIfNeeded();
            needSave = true;
        }
        if (lastMonth != currentMonth) {
            resetMonthlyIfNeeded();
            needSave = true;
        }
        if (needSave) save();
        return;
    }

   
    if (!lastDay.empty() && lastDay != today) {
        
        std::tm lastTm = {};
        std::istringstream lastSS(lastDay);
        lastSS >> std::get_time(&lastTm, "%Y-%m-%d");

        std::tm todayTm = {};
        std::istringstream todaySS(today);
        todaySS >> std::get_time(&todayTm, "%Y-%m-%d");

        if (!lastSS.fail() && !todaySS.fail()) {
            std::time_t lastTime = std::mktime(&lastTm);
            std::time_t todayTime = std::mktime(&todayTm);

            if (lastTime != -1 && todayTime != -1) {
                double diffSeconds = std::difftime(todayTime, lastTime);
                int diffDays = static_cast<int>(diffSeconds / 86400.0);

                bool wouldBreak = false;
                int daysLost = 0;
                if (diffDays > 1) {
                    wouldBreak = true;
                    daysLost = diffDays - 1;
                }
                else if (diffDays == 1) {
                    int reqPoints = getRequiredPoints();
                    if (streakPointsToday < reqPoints && reqPoints > 0) {
                        wouldBreak = true;
                        daysLost = 1;
                    }
                }

                if (wouldBreak) {
                    if (shieldsEnabled && streakShields >= daysLost && daysLost > 0) {
                        streakShields -= daysLost;
                        log::info("Streak saved by shields! Consumed {} (left: {})", daysLost, streakShields);
                    } else {
                        log::info("Streak broken! Last day: {}, Today: {}, Gap: {} days", lastDay, today, diffDays);
                        currentStreak = 0;
                        hasNewStreak = false;
                        streakPointsHistory.clear();
                    }
                }
            }
        }
    }

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
    pointMission7Claimed = false;
    pointMission8Claimed = false;
    pointMission9Claimed = false;
    pointMission10Claimed = false;
    pointMission11Claimed = false;
    pointMission12Claimed = false;
    pointMission13Claimed = false;

    resetWeeklyIfNeeded();
    resetMonthlyIfNeeded();
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

// Cumulative streak-token thresholds for each of the 12 sub-ranks.
// MUST stay in sync with RANK_THRESHOLDS in server.js.
static const int s_rankThresholds[StreakData::RANK_COUNT] = {
    0,     // Bronze I
    400,   // Bronze II
    900,   // Bronze III
    1600,  // Platinum I
    2500,  // Platinum II
    3600,  // Platinum III
    5000,  // Gold I
    6800,  // Gold II
    9000,  // Gold III
    12000, // Diamond I
    16000, // Diamond II
    21000  // Diamond III
};

static const char* s_rankSprites[StreakData::RANK_COUNT] = {
    "bronze.png", "bronze2.png", "bronze3.png",
    "platinum.png", "platinum2.png", "platinum3.png",
    "gold.png", "gold2.png", "gold3.png",
    "diamond.png", "diamond2.png", "diamond3.png"
};

static const char* s_rankNames[StreakData::RANK_COUNT] = {
    "Bronze I", "Bronze II", "Bronze III",
    "Platinum I", "Platinum II", "Platinum III",
    "Gold I", "Gold II", "Gold III",
    "Diamond I", "Diamond II", "Diamond III"
};

int StreakData::getRankIndexForTokens(int tokens) {
    int idx = 0;
    for (int i = 0; i < RANK_COUNT; ++i) {
        if (tokens >= s_rankThresholds[i]) idx = i;
        else break;
    }
    return idx;
}

int StreakData::getRankThreshold(int rankIndex) {
    if (rankIndex < 0) rankIndex = 0;
    if (rankIndex >= RANK_COUNT) rankIndex = RANK_COUNT - 1;
    return s_rankThresholds[rankIndex];
}

std::string StreakData::getRankSpriteForIndex(int rankIndex) {
    if (rankIndex < 0) rankIndex = 0;
    if (rankIndex >= RANK_COUNT) rankIndex = RANK_COUNT - 1;
    return fmt::format("{}/{}", Mod::get()->getID(), s_rankSprites[rankIndex]);
}

std::string StreakData::getRankSprite(int tokens) {
    return getRankSpriteForIndex(getRankIndexForTokens(tokens));
}

std::string StreakData::getRankNameForIndex(int rankIndex) {
    if (rankIndex < 0) rankIndex = 0;
    if (rankIndex >= RANK_COUNT) rankIndex = RANK_COUNT - 1;
    return s_rankNames[rankIndex];
}

std::string StreakData::getRankName(int tokens) {
    return getRankNameForIndex(getRankIndexForTokens(tokens));
}

int StreakData::getNextRankThreshold(int tokens) {
    int idx = getRankIndexForTokens(tokens);
    if (idx >= RANK_COUNT - 1) return -1; // already max rank
    return s_rankThresholds[idx + 1];
}

std::string StreakData::getRankColorStyleForIndex(int rankIndex) {
    if (rankIndex <= 2) return "Bronze Wave";   // Bronze
    if (rankIndex <= 5) return "Platinum Wave"; // Platinum
    if (rankIndex <= 8) return "Gold Wave";     // Gold
    return "Diamond Wave";                      // Diamond
}

std::string StreakData::getRankColorStyle(int tokens) {
    return getRankColorStyleForIndex(getRankIndexForTokens(tokens));
}

int StreakData::getStreakTokensForDay(int day) {
    if (day < 0) day = 0;
    int tier = day / 10;
    if (tier > 10) tier = 10; // cap at day 100 -> 550 tokens
    return 50 * (tier + 1);
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

void StreakData::unlockSong(const std::string& songID) {
    if (songID.empty()) return;
    if (unlockedSongs.size() != songs.size()) unlockedSongs.assign(songs.size(), false);
    for (size_t i = 0; i < songs.size(); ++i) {
        if (i < unlockedSongs.size() && songs[i].songID == songID) {
            unlockedSongs[i] = true;
            return;
        }
    }
}

bool StreakData::isSongUnlocked(const std::string& songID) {
    if (songID.empty()) return false;
    if (unlockedSongs.size() != songs.size()) return false;
    for (size_t i = 0; i < songs.size(); ++i) {
        if (i < unlockedSongs.size() && songs[i].songID == songID) return unlockedSongs[i];
    }
    return false;
}

StreakData::SongInfo* StreakData::getSongInfo(const std::string& songID) {
    if (songID.empty()) return nullptr;
    for (auto& song : songs) {
        if (song.songID == songID) return &song;
    }
    return nullptr;
}

void StreakData::equipSong(const std::string& songID) {
    if (songID.empty()) return;
    if (isSongUnlocked(songID)) {
        if (equippedSong != songID) {
            equippedSong = songID;
            save();
        }
    }
}

void StreakData::unequipSong() {
    if (!equippedSong.empty()) {
        equippedSong = "";
        save();
    }
}

StreakData::SongInfo* StreakData::getEquippedSong() {
    return getSongInfo(equippedSong);
}

std::string StreakData::getEquippedSongFile() {
    auto info = getSongInfo(equippedSong);
    if (info && isSongUnlocked(equippedSong)) return info->fileName;
    // Default menu theme when no song is equipped: song_2 (DNA - WC 2026).
    return std::string("s2.mp3"_spr);
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


    std::vector<ShopItem> candidates;

    auto addCandidate = [&](const std::string& id,
        bool isBadge,
        BadgeCategory cat,
        const std::string& name,
        const std::string& spr, int daysReq,
        bool excludeFromShop) {
            if (cat == BadgeCategory::MYTHIC) return;
            if (excludeFromShop) return;
            if (daysReq > 0) return;
            candidates.push_back({ id, isBadge, getPriceForRarity(cat), cat, name, spr });
        };

    for (const auto& b : badges) addCandidate(b.badgeID, true, b.category, b.displayName, b.spriteName, b.daysRequired, b.excludeFromShop);
    for (const auto& b : banners) addCandidate(b.bannerID, false, b.rarity, b.displayName, b.spriteName, 0, b.excludeFromShop);


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


    std::mt19937 gen(seed + 77777);

    //All consumable options by rarity 
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

    
    for (int i = pool.size() - 1; i > 0; i--) {
        std::uniform_int_distribution<> dist(0, i);
        int j = dist(gen);
        std::swap(pool[i], pool[j]);
    }

    
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
    streakPointsThisWeek += count;
    streakPointsThisMonth += count;

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
        for (int i = 1; i <= levelsGained; i++) {
            queuePendingLevelReward(preLevel + i);
        }

        this->save();

        SystemNotification::show(
            "LEVEL UP!",
            fmt::format("{} reward(s) ready to claim", levelsGained),
            "xp.png"_spr,
            0.3f
        );
    }
    else {
        this->save();
    }
}

void StreakData::queuePendingLevelReward(int level) {
    if (isLevelRewardPending(level)) return;
    auto r = getRewardsForLevel(level);
    PendingLevelReward p;
    p.level = level;
    p.stars = r.stars;
    p.tickets = r.tickets;
    p.gems = r.gems;
    p.shields = r.shields;
    p.chestRarity = r.chestRarity;
    pendingLevelRewards.push_back(p);
}

bool StreakData::isLevelRewardPending(int level) const {
    for (const auto& p : pendingLevelRewards) {
        if (p.level == level) return true;
    }
    return false;
}

bool StreakData::claimPendingLevelReward(int level) {
    for (auto it = pendingLevelRewards.begin(); it != pendingLevelRewards.end(); ++it) {
        if (it->level == level) {
            this->superStars += it->stars;
            this->starTickets += it->tickets;
            this->gems += it->gems;
            pendingLevelRewards.erase(it);
            this->save();
            return true;
        }
    }
    return false;
}

void StreakData::handleServerLevelUp(int previousLevel, int newLevel) {
    if (newLevel <= previousLevel) return;
    int levelsGained = newLevel - previousLevel;
    for (int lvl = previousLevel + 1; lvl <= newLevel; lvl++) {
        queuePendingLevelReward(lvl);
    }
    SystemNotification::show(
        "LEVEL UP!",
        fmt::format("{} reward(s) ready to claim", levelsGained),
        "xp.png"_spr,
        0.3f
    );
}

bool StreakData::hasPendingDailyMissions() const {
    int p = streakPointsToday;
    if (p >= 5   && !pointMission1Claimed)  return true;
    if (p >= 10  && !pointMission2Claimed)  return true;
    if (p >= 15  && !pointMission3Claimed)  return true;
    if (p >= 20  && !pointMission4Claimed)  return true;
    if (p >= 25  && !pointMission5Claimed)  return true;
    if (p >= 30  && !pointMission6Claimed)  return true;
    if (p >= 35  && !pointMission7Claimed)  return true;
    if (p >= 45  && !pointMission8Claimed)  return true;
    if (p >= 55  && !pointMission9Claimed)  return true;
    if (p >= 60  && !pointMission10Claimed) return true;
    if (p >= 75  && !pointMission11Claimed) return true;
    if (p >= 90  && !pointMission12Claimed) return true;
    if (p >= 100 && !pointMission13Claimed) return true;

    int w = streakPointsThisWeek;
    if (w >= 300  && !weeklyMission1Claimed)  return true;
    if (w >= 500  && !weeklyMission2Claimed)  return true;
    if (w >= 700  && !weeklyMission3Claimed)  return true;
    if (w >= 900  && !weeklyMission4Claimed)  return true;
    if (w >= 1100 && !weeklyMission5Claimed)  return true;
    if (w >= 1300 && !weeklyMission6Claimed)  return true;
    if (w >= 1500 && !weeklyMission7Claimed)  return true;
    if (w >= 1700 && !weeklyMission8Claimed)  return true;
    if (w >= 1900 && !weeklyMission9Claimed)  return true;
    if (w >= 2100 && !weeklyMission10Claimed) return true;
    if (w >= 3000 && !weeklyMission11Claimed) return true;
    return false;
}

bool StreakData::hasPendingLevelMissions() const {
    for (const auto& mission : g_levelMissions) {
        if (isLevelMissionClaimed(mission.levelID)) continue;
        auto level = GameLevelManager::sharedState()->getSavedLevel(mission.levelID);
        if (level && level->m_normalPercent >= 100) return true;
    }
    return false;
}

StreakData::LevelRewards StreakData::getRewardsForLevel(int level) {
    LevelRewards rewards;
    rewards.stars = 0;
    rewards.tickets = 0;
    rewards.gems = 0;
    rewards.shields = 0;
    rewards.chestRarity = 0;

    
    if (level > 0 && level % 10 == 0) {
        rewards.chestRarity = (level == 80 || level == 90 || level == 100) ? 5 : 4;
        return rewards;
    }

    if (level < 10) rewards.stars = 5;
    else if (level < 20) rewards.stars = 20;
    else {
        int tier = (level / 10) - 1;
        rewards.stars = 20 + (tier * 20);
    }
    rewards.gems = ((level - 1) / 10) + 1;
    rewards.shields = 1;
    return rewards;
}

bool StreakData::isNameItemUnlocked(const std::string& item) {
    if (item == "Default" || item == "None") return true;
    return unlockedNameItems.count(item) > 0;
}

void StreakData::unlockNameItem(const std::string& item) {
    unlockedNameItems.insert(item);
    save();
}

bool StreakData::isEventOnlyNameItem(const std::string& item) {
    static const std::set<std::string> kEventOnly = {
        "Galaxy Wave"
    };
    return kEventOnly.count(item) > 0;
}

int StreakData::getNameItemPrice(const std::string& item) {

    if (item == "Default" || item == "None") return 0;
    if (isEventOnlyNameItem(item)) return 0;
    if (item.find("Wave") != std::string::npos ||
        item == "Synthwave" ||
        item.find("Blink") != std::string::npos ||
        item == "Rainbow") {
        return 800;
    }
 
    if (item.find("Static") != std::string::npos) {
        return 350;
    }
 
    if (item.find("Font") != std::string::npos || item == "Chat" || item == "Gold" || item == "Pusab") {
        return 150;
    }
 
    std::set<std::string> basicColors = {
        "Black", "Blue", "Brown", "Cyan", "Gold", "Green", "Lime", "Magenta",
        "Maroon", "Mint", "Navy", "Orange", "Peach", "Pink", "Purple", "Red",
        "Silver", "Teal", "Yellow"
    };
    if (basicColors.count(item)) {
        return 100;
    }
 
    return 250;
}