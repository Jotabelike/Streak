#include "FirebaseManager.h"
#include <Geode/utils/web.hpp>
#include <Geode/utils/async.hpp>
#include <matjson.hpp>
#include <Geode/binding/GJAccountManager.hpp>
#include "StreakData.h"
#include <Geode/loader/Event.hpp>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <Geode/binding/GameManager.hpp>
#include "RewardNotification.h"
#include "HMACAuth.h"

using namespace geode::prelude;

static async::TaskHolder<web::WebResponse> s_updateListener;
static async::TaskHolder<web::WebResponse> s_loadListener;
static async::TaskHolder<web::WebResponse> s_completeLevelListener;
static async::TaskHolder<web::WebResponse> s_rouletteSpinListener;
static async::TaskHolder<web::WebResponse> s_gemRouletteSpinListener;
static async::TaskHolder<web::WebResponse> s_claimListener;

static const std::string SERVER_URL = "https://streak-servidor.onrender.com";

void loadPlayerDataFromServer() {
    auto am = GJAccountManager::sharedState();
    if (!am || am->m_accountID == 0) {
        g_streakData.resetToDefault();
        g_streakData.isDataLoaded = true;
        g_streakData.m_initialized = true;
        return;
    }

    int accountID = am->m_accountID;
    std::string url = fmt::format("{}/players/{}", SERVER_URL, accountID);
    log::info("Requesting data from the server...");

    HMACAuth::clearSessionToken();

    auto req = web::WebRequest();
    HMACAuth::signGetRequest(req, accountID);

    s_loadListener.spawn(
        req.get(url),
        [accountID](web::WebResponse res) {
            if (res.ok() && res.json().isOk()) {
                auto data = res.json().unwrap();

                if (data.contains("session_token")) {
                    std::string token = data["session_token"].as<std::string>().unwrapOr(std::string(""));
                    if (!token.empty()) {
                        HMACAuth::setSessionToken(token);
                        log::info("Session token received and stored.");
                    }
                }

                g_streakData.parseServerResponse(data);
                std::string gdpsKey = fmt::format("is_gdps_player_{}", accountID);
                geode::Mod::get()->setSavedValue<bool>(gdpsKey, g_streakData.isGDPS);
                g_streakData.isDataLoaded = true;
                g_streakData.m_initialized = true;
                log::info("Data received and processed.");
            }
            else if (res.code() == 404) {
                log::info("New user (404). Registration required.");           
                if (res.json().isOk()) {
                    auto data = res.json().unwrap();
                    if (data.contains("session_token")) {
                        std::string token = data["session_token"].as<std::string>().unwrapOr("");
                        if (!token.empty()) {
                            HMACAuth::setSessionToken(token);
                            log::info("Session token saved for registration.");
                        }
                    }
                }

                g_streakData.resetToDefault();
                g_streakData.needsRegistration = true;
                g_streakData.isDataLoaded = true;
                g_streakData.m_initialized = true;
            }
            else if (res.code() == 401) {
                log::error("Authentication failed (401). HMAC mismatch or expired.");
                g_streakData.isDataLoaded = false;
                g_streakData.m_initialized = false;
            }
            else {
                log::warn("Load failed (Code: {}). Maintaining error state.", res.code());
                g_streakData.isDataLoaded = false;
                g_streakData.m_initialized = false;
            }
        }
    );
}

void updatePlayerDataInFirebase() {
    auto accountManager = GJAccountManager::sharedState();
    if (!accountManager || accountManager->m_accountID == 0) {
        log::error("Save canceled: Not logged in.");
        return;
    }

    if (HMACAuth::getSessionToken().empty()) {
        log::error("Save canceled: No session token. Load data first.");
        return;
    }

    int accountID = accountManager->m_accountID;
    int userID = GameManager::sharedState()->m_playerUserID;
    matjson::Value playerData = matjson::Value::object();

 
    playerData.set("username", std::string(accountManager->m_username));
    playerData.set("accountID", accountID);
    playerData.set("userID", userID);
    playerData.set("isGDPS", g_streakData.isGDPS);
    playerData.set("equipped_badge_id", g_streakData.equippedBadge);
    playerData.set("equipped_banner_id", g_streakData.equippedBanner);
    playerData.set("equipped_name_color", g_streakData.equippedNameColor);
    playerData.set("equipped_name_font", g_streakData.equippedNameFont);
    playerData.set("equipped_name_effect", g_streakData.equippedNameEffect);
    playerData.set("equipped_name_animation", g_streakData.equippedNameAnimation);
    playerData.set("last_streak_animated", g_streakData.lastStreakAnimated);
    playerData.set("gem_roulette_spin_count", g_streakData.gemRouletteSpinCount);
    playerData.set("gem_roulette_hash", g_streakData.gemRouletteHash);
    playerData.set("last_roulette_index", g_streakData.lastRouletteIndex);
    playerData.set("total_spins", g_streakData.totalSpins);

    std::vector<bool> gemStateVec = g_streakData.gemRouletteState;
    if (gemStateVec.size() < 7) gemStateVec.resize(7, false);
    playerData.set("gem_roulette_state", gemStateVec);

    matjson::Value tasksJson = matjson::Value::object();
    for (auto const& [id, status] : g_streakData.taskStatuses) {
        tasksJson.set(id, status);
    }
    playerData.set("taskStatuses", tasksJson);

    std::vector<std::string> unlocked_badges_vec;
    if (g_streakData.unlockedBadges.size() == g_streakData.badges.size()) {
        for (size_t i = 0; i < g_streakData.badges.size(); ++i) {
            if (i < g_streakData.unlockedBadges.size() && g_streakData.unlockedBadges[i]) {
                unlocked_badges_vec.push_back(g_streakData.badges[i].badgeID);
            }
        }
    }
    playerData.set("unlocked_badges", unlocked_badges_vec);

    std::vector<std::string> unlocked_banners_vec;
    if (g_streakData.unlockedBanners.size() == g_streakData.banners.size()) {
        for (size_t i = 0; i < g_streakData.banners.size(); ++i) {
            if (i < g_streakData.unlockedBanners.size() && g_streakData.unlockedBanners[i]) {
                unlocked_banners_vec.push_back(g_streakData.banners[i].bannerID);
            }
        }
    }
    playerData.set("unlocked_banners", unlocked_banners_vec);

    matjson::Value pinnedObj = matjson::Value::object();
    for (auto const& [key, val] : g_streakData.pinnedLevels) {
        pinnedObj.set(key, val);
    }
    playerData.set("pinned_levels", pinnedObj);

    matjson::Value missions_obj = matjson::Value::object();
    missions_obj.set("pm1", g_streakData.pointMission1Claimed);
    missions_obj.set("pm2", g_streakData.pointMission2Claimed);
    missions_obj.set("pm3", g_streakData.pointMission3Claimed);
    missions_obj.set("pm4", g_streakData.pointMission4Claimed);
    missions_obj.set("pm5", g_streakData.pointMission5Claimed);
    missions_obj.set("pm6", g_streakData.pointMission6Claimed);
    missions_obj.set("pm7",  g_streakData.pointMission7Claimed);
    missions_obj.set("pm8",  g_streakData.pointMission8Claimed);
    missions_obj.set("pm9",  g_streakData.pointMission9Claimed);
    missions_obj.set("pm10", g_streakData.pointMission10Claimed);
    missions_obj.set("pm11", g_streakData.pointMission11Claimed);
    missions_obj.set("pm12", g_streakData.pointMission12Claimed);
    missions_obj.set("pm13", g_streakData.pointMission13Claimed);
    playerData.set("missions", missions_obj);

    matjson::Value weekly_missions_obj = matjson::Value::object();
    weekly_missions_obj.set("wm1",  g_streakData.weeklyMission1Claimed);
    weekly_missions_obj.set("wm2",  g_streakData.weeklyMission2Claimed);
    weekly_missions_obj.set("wm3",  g_streakData.weeklyMission3Claimed);
    weekly_missions_obj.set("wm4",  g_streakData.weeklyMission4Claimed);
    weekly_missions_obj.set("wm5",  g_streakData.weeklyMission5Claimed);
    weekly_missions_obj.set("wm6",  g_streakData.weeklyMission6Claimed);
    weekly_missions_obj.set("wm7",  g_streakData.weeklyMission7Claimed);
    weekly_missions_obj.set("wm8",  g_streakData.weeklyMission8Claimed);
    weekly_missions_obj.set("wm9",  g_streakData.weeklyMission9Claimed);
    weekly_missions_obj.set("wm10", g_streakData.weeklyMission10Claimed);
    playerData.set("weeklyMissions", weekly_missions_obj);

    playerData.set("shields_enabled", g_streakData.shieldsEnabled);

    bool hasMythicEquipped = false;
    if (!g_streakData.equippedBadge.empty()) {
        if (auto* badgeInfo = g_streakData.getBadgeInfo(g_streakData.equippedBadge)) {
            if (badgeInfo->category == StreakData::BadgeCategory::MYTHIC) hasMythicEquipped = true;
        }
    }
    playerData.set("has_mythic_color", hasMythicEquipped);

    matjson::Value completed_levels_obj = matjson::Value::object();
    for (int levelID : g_streakData.completedLevelMissions) {
        completed_levels_obj.set(std::to_string(levelID), true);
    }
    playerData.set("completedLevelMissions", completed_levels_obj);

    std::vector<std::string> gemRouletteClaimed;
    for (const auto& id : g_streakData.claimedGemRoulettePrizes) gemRouletteClaimed.push_back(id);
    playerData.set("claimed_gem_roulette_prizes", gemRouletteClaimed);

    std::vector<std::string> standardRouletteClaimed;
    for (const auto& id : g_streakData.claimedStandardRoulettePrizes) standardRouletteClaimed.push_back(id);
    playerData.set("claimed_standard_roulette_prizes", standardRouletteClaimed);

    std::vector<matjson::Value> pending_rewards_vec;
    for (const auto& p : g_streakData.pendingLevelRewards) {
        matjson::Value item = matjson::Value::object();
        item.set("level", p.level);
        item.set("stars", p.stars);
        item.set("tickets", p.tickets);
        item.set("gems", p.gems);
        item.set("shields", p.shields);
        item.set("chestRarity", p.chestRarity);
        pending_rewards_vec.push_back(item);
    }
    playerData.set("pending_level_rewards", pending_rewards_vec);

    std::vector<int> goalsArray;
    for (int index : g_streakData.claimedStreakGoals) {
        goalsArray.push_back(index);
    }
    playerData.set("claimed_streak_goals", goalsArray);

    std::vector<int> discordGoalsArray;
    for (int req : g_streakData.claimedDiscordMilestones) {
        discordGoalsArray.push_back(req);
    }
    playerData.set("claimed_discord_milestones", discordGoalsArray);

    std::vector<std::string> unlocked_names_vec;
    for (const auto& item : g_streakData.unlockedNameItems) {
        unlocked_names_vec.push_back(item);
    }
    playerData.set("unlocked_name_items", unlocked_names_vec);

    std::string url = fmt::format("{}/players/{}", SERVER_URL, accountID);

    auto req = web::WebRequest();
    HMACAuth::signRequest(req, accountID, playerData);

    s_updateListener.spawn(
        req.bodyJSON(playerData).post(url),
        [](web::WebResponse res) {
            if (!res.ok()) {
                log::error("SERVER ERROR SAVING: {}", res.code());
                if (res.code() == 401) {
                    log::error("Auth failed. Session may have expired. Reloading...");
                    HMACAuth::clearSessionToken();
                    loadPlayerDataFromServer();
                }
            }
        }
    );
}

void completeLevelInFirebase(int stars) {
    auto am = GJAccountManager::sharedState();
    if (!am || am->m_accountID == 0) return;

    if (HMACAuth::getSessionToken().empty()) {
        log::error("Complete level canceled: No session token.");
        return;
    }

    int accountID = am->m_accountID;
    std::string url = fmt::format("{}/players/{}/complete-level", SERVER_URL, accountID);

    matjson::Value payload = matjson::Value::object();
    payload.set("stars", stars);

    log::info("Sending completed level to the server...");

    auto req = web::WebRequest();
    HMACAuth::signRequest(req, accountID, payload);

    s_completeLevelListener.spawn(
        req.bodyJSON(payload).post(url),
        [](web::WebResponse res) {
            if (res.ok() && res.json().isOk()) {
                auto data = res.json().unwrap();

                if (data.contains("current_xp"))
                    g_streakData.currentXP = data["current_xp"].as<int>().unwrapOr(g_streakData.currentXP);
                if (data.contains("daily_shop_seed"))
                    g_streakData.dailyShopSeed = data["daily_shop_seed"].as<int>().unwrapOr(0);
                if (data.contains("current_level"))
                    g_streakData.currentLevel = data["current_level"].as<int>().unwrapOr(g_streakData.currentLevel);
                if (data.contains("super_stars"))
                    g_streakData.superStars = data["super_stars"].as<int>().unwrapOr(g_streakData.superStars);
                if (data.contains("star_tickets"))
                    g_streakData.starTickets = data["star_tickets"].as<int>().unwrapOr(g_streakData.starTickets);
                if (data.contains("current_streak_days"))
                    g_streakData.currentStreak = data["current_streak_days"].as<int>().unwrapOr(g_streakData.currentStreak);
                if (data.contains("gems"))
                    g_streakData.gems = data["gems"].as<int>().unwrapOr(g_streakData.gems);
                if (data.contains("streakPointsToday"))
                    g_streakData.streakPointsToday = data["streakPointsToday"].as<int>().unwrapOr(g_streakData.streakPointsToday);
                if (data.contains("streakPointsThisWeek"))
                    g_streakData.streakPointsThisWeek = data["streakPointsThisWeek"].as<int>().unwrapOr(g_streakData.streakPointsThisWeek);
                if (data.contains("lastWeek"))
                    g_streakData.lastWeek = data["lastWeek"].as<std::string>().unwrapOr(g_streakData.lastWeek);
                if (data.contains("streak_shields"))
                    g_streakData.streakShields = data["streak_shields"].as<int>().unwrapOr(g_streakData.streakShields);
                if (data.contains("total_streak_points"))
                    g_streakData.totalStreakPoints = data["total_streak_points"].as<int>().unwrapOr(g_streakData.totalStreakPoints);
                if (data.contains("lastDay"))
                    g_streakData.lastDay = data["lastDay"].as<std::string>().unwrapOr(std::string(""));

                int reqPoints = g_streakData.getRequiredPoints();
                if (g_streakData.streakPointsToday >= reqPoints && reqPoints > 0) {
                    g_streakData.hasNewStreak = true;
                }

                std::string today = g_streakData.getCurrentDate();
                if (!today.empty()) {
                    g_streakData.streakPointsHistory[today] = g_streakData.streakPointsToday;
                }

                if (data.contains("newRewards")) {
                    auto rewards = data["newRewards"];
                    int levelsGained = rewards["levels"].as<int>().unwrapOr(0);
                    if (levelsGained > 0) {
                        int newLevel = g_streakData.currentLevel;
                        g_streakData.handleServerLevelUp(newLevel - levelsGained, newLevel);
                    }
                }

                g_streakData.isDataLoaded = true;
                log::info("Level completed. XP: {}, Level: {}, PointsToday: {}, Streak: {}",
                    g_streakData.currentXP, g_streakData.currentLevel,
                    g_streakData.streakPointsToday, g_streakData.currentStreak);
            }
            else {
                log::error("Error completing level: {}", res.code());
                if (res.code() == 401) {
                    log::error("Session expired. Reloading data...");
                    HMACAuth::clearSessionToken();
                    loadPlayerDataFromServer();
                }
            }
        }
    );
}
 
static void applyServerBalances(const matjson::Value& data) {
    int previousLevel = g_streakData.currentLevel;

    if (data.contains("balances")) {
        auto bal = data["balances"];
        g_streakData.superStars = bal["super_stars"].as<int>().unwrapOr(g_streakData.superStars);
        g_streakData.starTickets = bal["star_tickets"].as<int>().unwrapOr(g_streakData.starTickets);
        g_streakData.gems = bal["gems"].as<int>().unwrapOr(g_streakData.gems);
        g_streakData.fragments = bal["fragments"].as<int>().unwrapOr(g_streakData.fragments);
        g_streakData.currentXP = bal["current_xp"].as<int>().unwrapOr(g_streakData.currentXP);
        g_streakData.currentLevel = bal["current_level"].as<int>().unwrapOr(g_streakData.currentLevel);
    }
    if (data.contains("totalSpins"))
        g_streakData.totalSpins = data["totalSpins"].as<int>().unwrapOr(g_streakData.totalSpins);

    g_streakData.handleServerLevelUp(previousLevel, g_streakData.currentLevel);
}
 
static void applyServerUnlocks(const matjson::Value& data) {
    if (data.contains("unlocked_badges")) {
        auto badges = data["unlocked_badges"].as<std::vector<matjson::Value>>();
        if (badges.isOk()) {
            for (auto& b : badges.unwrap()) {
                g_streakData.unlockBadge(b.as<std::string>().unwrapOr(std::string("")));
            }
        }
    }
    if (data.contains("unlocked_banners")) {
        auto banners = data["unlocked_banners"].as<std::vector<matjson::Value>>();
        if (banners.isOk()) {
            for (auto& b : banners.unwrap()) {
                g_streakData.unlockBanner(b.as<std::string>().unwrapOr(std::string("")));
            }
        }
    }
}

void claimOnServer(const std::string& endpoint, const matjson::Value& payload, std::function<void(bool)> callback) {
    auto am = GJAccountManager::sharedState();
    if (!am || am->m_accountID == 0) { callback(false); return; }
    if (HMACAuth::getSessionToken().empty()) { callback(false); return; }

    int accountID = am->m_accountID;
    std::string url = fmt::format("{}{}", SERVER_URL, endpoint);

    auto req = web::WebRequest();
    HMACAuth::signRequest(req, accountID, payload);

    log::info("Claiming on server: {}", endpoint);

    s_claimListener.spawn(
        req.bodyJSON(payload).post(url),
        [callback](web::WebResponse res) {
            if (res.ok() && res.json().isOk()) {
                auto data = res.json().unwrap();
                applyServerBalances(data);
                log::info("Claim OK. Stars: {}, Tickets: {}, Gems: {}",
                    g_streakData.superStars, g_streakData.starTickets, g_streakData.gems);
                callback(true);
            } else {
                log::error("Claim failed: {}", res.code());
                if (res.code() == 401) {
                    HMACAuth::clearSessionToken();
                    loadPlayerDataFromServer();
                }
                callback(false);
            }
        }
    );
}

void spinStandardRouletteOnServer(int spins, std::function<void(bool, matjson::Value)> callback) {
    auto am = GJAccountManager::sharedState();
    if (!am || am->m_accountID == 0) { callback(false, matjson::Value()); return; }
    if (HMACAuth::getSessionToken().empty()) { callback(false, matjson::Value()); return; }

    int accountID = am->m_accountID;
    std::string url = fmt::format("{}/roulette/spin", SERVER_URL);

    matjson::Value payload = matjson::Value::object();
    payload.set("spins", spins);

    auto req = web::WebRequest();
    HMACAuth::signRequest(req, accountID, payload);

    log::info("Sending standard roulette spin (x{}) to server...", spins);

    s_rouletteSpinListener.spawn(
        req.bodyJSON(payload).post(url),
        [callback](web::WebResponse res) {
            if (res.ok() && res.json().isOk()) {
                auto data = res.json().unwrap();
                applyServerBalances(data);
                applyServerUnlocks(data);
                log::info("Standard roulette spin OK. Stars: {}, Tickets: {}",
                    g_streakData.superStars, g_streakData.starTickets);
                callback(true, data);
            }
            else {
                log::error("Standard roulette spin failed: {}", res.code());
                if (res.code() == 401) {
                    HMACAuth::clearSessionToken();
                    loadPlayerDataFromServer();
                }
                callback(false, matjson::Value());
            }
        }
    );
}

void spinGemRouletteOnServer(std::function<void(bool, matjson::Value)> callback) {
    auto am = GJAccountManager::sharedState();
    if (!am || am->m_accountID == 0) { callback(false, matjson::Value()); return; }
    if (HMACAuth::getSessionToken().empty()) { callback(false, matjson::Value()); return; }

    int accountID = am->m_accountID;
    std::string url = fmt::format("{}/gem-roulette/spin", SERVER_URL);

    matjson::Value payload = matjson::Value::object();

    auto req = web::WebRequest();
    HMACAuth::signRequest(req, accountID, payload);

    log::info("Sending gem roulette spin to server...");

    s_gemRouletteSpinListener.spawn(
        req.bodyJSON(payload).post(url),
        [callback](web::WebResponse res) {
            if (res.ok() && res.json().isOk()) {
                auto data = res.json().unwrap();
                applyServerBalances(data);
                applyServerUnlocks(data);

            
                if (data.contains("gemRouletteState")) {
                    g_streakData.gemRouletteState =
                        data["gemRouletteState"].as<std::vector<bool>>().unwrapOr(g_streakData.gemRouletteState);
                }
                if (data.contains("gemRouletteSpinCount")) {
                    g_streakData.gemRouletteSpinCount =
                        data["gemRouletteSpinCount"].as<int>().unwrapOr(g_streakData.gemRouletteSpinCount);
                }
                if (data.contains("claimed_gem_roulette_prizes")) {
                    auto arr = data["claimed_gem_roulette_prizes"].as<std::vector<matjson::Value>>();
                    if (arr.isOk()) {
                        g_streakData.claimedGemRoulettePrizes.clear();
                        for (const auto& item : arr.unwrap()) {
                            std::string s = item.as<std::string>().unwrapOr(std::string(""));
                            if (!s.empty()) g_streakData.claimedGemRoulettePrizes.insert(s);
                        }
                    }
                }

                log::info("Gem roulette spin OK. Gems: {}, SpinCount: {}",
                    g_streakData.gems, g_streakData.gemRouletteSpinCount);
                callback(true, data);
            }
            else {
                log::error("Gem roulette spin failed: {}", res.code());
                if (res.code() == 401) {
                    HMACAuth::clearSessionToken();
                    loadPlayerDataFromServer();
                }
                callback(false, matjson::Value());
            }
        }
    );
}