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
#include <memory>
#include <functional>
#include <Geode/binding/GameManager.hpp>
#include "RewardNotification.h"
#include "HMACAuth.h"

using namespace geode::prelude;

static async::TaskHolder<web::WebResponse> s_updateListener;
static async::TaskHolder<web::WebResponse> s_loadListener;
static async::TaskHolder<web::WebResponse> s_refreshListener;
static async::TaskHolder<web::WebResponse> s_completeLevelListener;
static async::TaskHolder<web::WebResponse> s_rouletteSpinListener;
static async::TaskHolder<web::WebResponse> s_gemRouletteSpinListener;
// Cada reclamacion necesita SU PROPIO holder: TaskHolder::spawn cancela la tarea
// anterior del mismo holder y garantiza que su callback ya no se llame. Con un
// unico holder compartido, reclamar dos cosas seguidas (un tier y la cancion, o
// claim-all) mataba el callback de la primera y su boton se quedaba deshabilitado
// para siempre. Se guardan aqui para mantenerlos vivos mientras corren.
static std::vector<std::shared_ptr<async::TaskHolder<web::WebResponse>>> s_claimListeners;
static async::TaskHolder<web::WebResponse> s_wcEventListener;
static async::TaskHolder<web::WebResponse> s_wcStateListener;
static async::TaskHolder<web::WebResponse> s_pendingRewardsListener;

static const std::string SERVER_URL = "https://streak-servidor.onrender.com";

void loadPlayerDataFromServer() {
    auto am = GJAccountManager::sharedState();
    if (!am || am->m_accountID == 0) {
        g_streakData.resetToDefault();
        g_streakData.isDataLoaded = true;
        g_streakData.m_initialized = true;
        g_streakData.loadedAccountID = 0;
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
                showShieldConversionAlert(data);
                std::string gdpsKey = fmt::format("is_gdps_player_{}", accountID);
                geode::Mod::get()->setSavedValue<bool>(gdpsKey, g_streakData.isGDPS);
                g_streakData.isDataLoaded = true;
                g_streakData.m_initialized = true;
                g_streakData.loadedAccountID = accountID;
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

void refreshPlayerDataFromServer(std::function<void(bool)> callback) {
    auto am = GJAccountManager::sharedState();
    if (!am || am->m_accountID == 0) { if (callback) callback(false); return; }

    int accountID = am->m_accountID;
    std::string url = fmt::format("{}/players/{}", SERVER_URL, accountID);

    HMACAuth::clearSessionToken();

    auto req = web::WebRequest();
    HMACAuth::signGetRequest(req, accountID);

    s_refreshListener.spawn(
        req.get(url),
        [callback, accountID](web::WebResponse res) {
            if (res.ok() && res.json().isOk()) {
                auto data = res.json().unwrap();

                if (data.contains("session_token")) {
                    std::string token = data["session_token"].as<std::string>().unwrapOr(std::string(""));
                    if (!token.empty()) HMACAuth::setSessionToken(token);
                }

                g_streakData.parseServerResponse(data);
                showShieldConversionAlert(data);
                std::string gdpsKey = fmt::format("is_gdps_player_{}", accountID);
                geode::Mod::get()->setSavedValue<bool>(gdpsKey, g_streakData.isGDPS);
                g_streakData.isDataLoaded = true;
                g_streakData.m_initialized = true;
                g_streakData.loadedAccountID = accountID;
                if (callback) callback(true);
            }
            else {
                log::warn("refreshPlayerDataFromServer failed (Code: {})", res.code());
                if (callback) callback(false);
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
    playerData.set("equipped_song_id", g_streakData.equippedSong);
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

    std::vector<std::string> unlocked_songs_vec;
    if (g_streakData.unlockedSongs.size() == g_streakData.songs.size()) {
        for (size_t i = 0; i < g_streakData.songs.size(); ++i) {
            if (i < g_streakData.unlockedSongs.size() && g_streakData.unlockedSongs[i]) {
                unlocked_songs_vec.push_back(g_streakData.songs[i].songID);
            }
        }
    }
    playerData.set("unlocked_songs", unlocked_songs_vec);

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
    weekly_missions_obj.set("wm11", g_streakData.weeklyMission11Claimed);
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

    playerData.set("streakPointsThisMonth", g_streakData.streakPointsThisMonth);
    playerData.set("lastMonth", g_streakData.lastMonth);
    playerData.set("premium_pass_month", g_streakData.premiumPassMonth);

    std::vector<int> freePassTiersArr;
    for (int t : g_streakData.claimedFreePassTiers) freePassTiersArr.push_back(t);
    playerData.set("claimed_free_pass_tiers", freePassTiersArr);

    std::vector<int> paidPassTiersArr;
    for (int t : g_streakData.claimedPaidPassTiers) paidPassTiersArr.push_back(t);
    playerData.set("claimed_paid_pass_tiers", paidPassTiersArr);

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
                showShieldConversionAlert(data);

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
                if (data.contains("streakPointsThisMonth"))
                    g_streakData.streakPointsThisMonth = data["streakPointsThisMonth"].as<int>().unwrapOr(g_streakData.streakPointsThisMonth);
                if (data.contains("lastMonth"))
                    g_streakData.lastMonth = data["lastMonth"].as<std::string>().unwrapOr(g_streakData.lastMonth);
                if (data.contains("streak_shields"))
                    g_streakData.streakShields = std::clamp(
                        data["streak_shields"].as<int>().unwrapOr(g_streakData.streakShields),
                        0, STREAK_MAX_SHIELDS
                    );
                if (data.contains("total_streak_points"))
                    g_streakData.totalStreakPoints = data["total_streak_points"].as<int>().unwrapOr(g_streakData.totalStreakPoints);
                if (data.contains("streak_tokens"))
                    g_streakData.streakTokens = data["streak_tokens"].as<int>().unwrapOr(g_streakData.streakTokens);
                if (data.contains("lastDay"))
                    g_streakData.lastDay = data["lastDay"].as<std::string>().unwrapOr(std::string(""));
                if (data.contains("pass_daily_levels"))
                    g_streakData.passDailyLevels = data["pass_daily_levels"].as<int>().unwrapOr(g_streakData.passDailyLevels);
                if (data.contains("pass_weekly_levels"))
                    g_streakData.passWeeklyLevels = data["pass_weekly_levels"].as<int>().unwrapOr(g_streakData.passWeeklyLevels);
                if (data.contains("pass_season_levels"))
                    g_streakData.passSeasonLevels = data["pass_season_levels"].as<int>().unwrapOr(g_streakData.passSeasonLevels);

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

                if (data.contains("pending_level_rewards")) {
                    auto arr = data["pending_level_rewards"].as<std::vector<matjson::Value>>();
                    if (arr.isOk()) {
                        g_streakData.pendingLevelRewards.clear();
                        for (const auto& item : arr.unwrap()) {
                            int lvl = item["level"].as<int>().unwrapOr(0);
                            if (lvl <= 0) continue;
                            auto r = g_streakData.getRewardsForLevel(lvl);
                            StreakData::PendingLevelReward p;
                            p.level       = lvl;
                            p.stars       = item["stars"].as<int>().unwrapOr(r.stars);
                            p.tickets     = item["tickets"].as<int>().unwrapOr(r.tickets);
                            p.gems        = item["gems"].as<int>().unwrapOr(r.gems);
                            p.shields     = item["shields"].as<int>().unwrapOr(r.shields);
                            p.chestRarity = item["chestRarity"].as<int>().unwrapOr(r.chestRarity);
                            g_streakData.pendingLevelRewards.push_back(p);
                        }
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
 
void showShieldConversionAlert(int convertedShields, int gemsAwarded) {
    if (convertedShields <= 0 || gemsAwarded <= 0) return;

    std::string body = fmt::format(
        "Maximum shields reached!\n\nYour <cr>{}</c> extra shield{} {} been "
        "converted into <cg>{}</c> gems. Each extra shield is worth <cg>{}</c> gems.",
        convertedShields,
        convertedShields == 1 ? "" : "s",
        convertedShields == 1 ? "has" : "have",
        gemsAwarded,
        STREAK_SHIELD_OVERFLOW_GEMS
    );
    FLAlertLayer::create("Maximum Shields Reached", body.c_str(), "OK")->show();
}

void showShieldConversionAlert(const matjson::Value& data) {
    if (!data.contains("shield_conversion") || data["shield_conversion"].isNull()) return;

    auto conversion = data["shield_conversion"];
    showShieldConversionAlert(
        conversion["converted_shields"].as<int>().unwrapOr(0),
        conversion["gems_awarded"].as<int>().unwrapOr(0)
    );
}

static void applyServerBalances(const matjson::Value& data) {
    int previousLevel = g_streakData.currentLevel;

    if (data.contains("balances")) {
        auto bal = data["balances"];
        g_streakData.superStars = bal["super_stars"].as<int>().unwrapOr(g_streakData.superStars);
        g_streakData.starTickets = bal["star_tickets"].as<int>().unwrapOr(g_streakData.starTickets);
        g_streakData.gems = bal["gems"].as<int>().unwrapOr(g_streakData.gems);
        g_streakData.streakShields = std::clamp(
            bal["streak_shields"].as<int>().unwrapOr(g_streakData.streakShields),
            0, STREAK_MAX_SHIELDS
        );
        g_streakData.fragments = bal["fragments"].as<int>().unwrapOr(g_streakData.fragments);
        g_streakData.currentXP = bal["current_xp"].as<int>().unwrapOr(g_streakData.currentXP);
        g_streakData.currentLevel = bal["current_level"].as<int>().unwrapOr(g_streakData.currentLevel);
        g_streakData.goldTickets = bal["gold_tickets"].as<int>().unwrapOr(g_streakData.goldTickets);
    }
    if (data.contains("totalSpins"))
        g_streakData.totalSpins = data["totalSpins"].as<int>().unwrapOr(g_streakData.totalSpins);

    if (data.contains("discount_tickets")) {
        auto tickets = data["discount_tickets"];
        g_streakData.setDiscountTicketCount(10, tickets["10"].as<int>().unwrapOr(g_streakData.getDiscountTicketCount(10)));
        g_streakData.setDiscountTicketCount(25, tickets["25"].as<int>().unwrapOr(g_streakData.getDiscountTicketCount(25)));
        g_streakData.setDiscountTicketCount(50, tickets["50"].as<int>().unwrapOr(g_streakData.getDiscountTicketCount(50)));
        g_streakData.setDiscountTicketCount(80, tickets["80"].as<int>().unwrapOr(g_streakData.getDiscountTicketCount(80)));
        g_streakData.setDiscountTicketCount(99, tickets["99"].as<int>().unwrapOr(g_streakData.getDiscountTicketCount(99)));
    }

    showShieldConversionAlert(data);

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

// Devuelve un holder nuevo para esta peticion, soltando antes los que ya
// terminaron. Solo se llama al lanzar una peticion (nunca desde un callback),
// asi que ningun holder se destruye mientras su propio callback corre.
static std::shared_ptr<async::TaskHolder<web::WebResponse>> acquireClaimHolder() {
    std::erase_if(s_claimListeners, [](const auto& h) { return !h->isPending(); });
    auto holder = std::make_shared<async::TaskHolder<web::WebResponse>>();
    s_claimListeners.push_back(holder);
    return holder;
}

void claimOnServerEx(const std::string& endpoint, const matjson::Value& payload,
    std::function<void(bool, int, const matjson::Value&)> callback) {
    static const matjson::Value kEmpty = matjson::Value::object();
    auto am = GJAccountManager::sharedState();
    if (!am || am->m_accountID == 0) { callback(false, 0, kEmpty); return; }
    if (HMACAuth::getSessionToken().empty()) { callback(false, 0, kEmpty); return; }

    int accountID = am->m_accountID;
    std::string url = fmt::format("{}{}", SERVER_URL, endpoint);

    auto req = web::WebRequest();
    HMACAuth::signRequest(req, accountID, payload);

    log::info("Claiming on server: {}", endpoint);

    auto holder = acquireClaimHolder();
    holder->spawn(
        req.bodyJSON(payload).post(url),
        [callback](web::WebResponse res) {
            auto json = res.json();
            if (res.ok() && json.isOk()) {
                auto data = json.unwrap();
                applyServerBalances(data);
                log::info("Claim OK. Stars: {}, Tickets: {}, Gems: {}",
                    g_streakData.superStars, g_streakData.starTickets, g_streakData.gems);
                callback(true, res.code(), data);
            } else {
                log::error("Claim failed: {}", res.code());
                if (res.code() == 401) {
                    HMACAuth::clearSessionToken();
                    loadPlayerDataFromServer();
                }
                matjson::Value body = matjson::Value::object();
                if (json.isOk()) body = json.unwrap();
                callback(false, res.code(), body);
            }
        }
    );
}

void claimOnServer(const std::string& endpoint, const matjson::Value& payload, std::function<void(bool)> callback) {
    claimOnServerEx(endpoint, payload, [callback](bool ok, int, const matjson::Value&) { callback(ok); });
}

void refreshPendingLevelRewardsFromServer(std::function<void(bool)> callback) {
    auto am = GJAccountManager::sharedState();
    if (!am || am->m_accountID == 0) { if (callback) callback(false); return; }
    if (HMACAuth::getSessionToken().empty()) { if (callback) callback(false); return; }

    int accountID = am->m_accountID;
    std::string url = fmt::format("{}/players/{}/pending-level-rewards", SERVER_URL, accountID);

    auto req = web::WebRequest();
    HMACAuth::signGetRequest(req, accountID);

    s_pendingRewardsListener.spawn(
        req.get(url),
        [callback](web::WebResponse res) {
            if (!res.ok() || !res.json().isOk()) {
                log::error("pending-level-rewards fetch failed: {}", res.code());
                if (res.code() == 401) {
                    HMACAuth::clearSessionToken();
                    loadPlayerDataFromServer();
                }
                if (callback) callback(false);
                return;
            }
            auto data = res.json().unwrap();
            g_streakData.pendingLevelRewards.clear();
            if (data.contains("list")) {
                auto arr = data["list"].as<std::vector<matjson::Value>>();
                if (arr.isOk()) {
                    for (const auto& item : arr.unwrap()) {
                        int lvl = item["level"].as<int>().unwrapOr(0);
                        if (lvl <= 0) continue;
                        auto r = g_streakData.getRewardsForLevel(lvl);
                        StreakData::PendingLevelReward p;
                        p.level       = lvl;
                        p.stars       = item["stars"].as<int>().unwrapOr(r.stars);
                        p.tickets     = item["tickets"].as<int>().unwrapOr(r.tickets);
                        p.gems        = item["gems"].as<int>().unwrapOr(r.gems);
                        p.shields     = item["shields"].as<int>().unwrapOr(r.shields);
                        p.chestRarity = item["chestRarity"].as<int>().unwrapOr(r.chestRarity);
                        g_streakData.pendingLevelRewards.push_back(p);
                    }
                }
            }
            log::info("pending-level-rewards refreshed: {} entries", (int)g_streakData.pendingLevelRewards.size());
            if (callback) callback(true);
        }
    );
}

// =====================
// EVENTO MUNDIAL 2026
// =====================
static StreakData::WcMatch* findWcMatch(const std::string& matchId) {
    for (auto& m : g_streakData.wcEvent.matches) {
        if (m.matchId == matchId) return &m;
    }
    return nullptr;
}

// Refresco en vivo del estado del evento (conteos/status/score/winner) sin
// tocar my_vote/claimed, que solo cambian por acciones del propio jugador.
void wcEventFetchStateOnServer(std::function<void(bool)> callback) {
    auto am = GJAccountManager::sharedState();
    if (!am || am->m_accountID == 0) { if (callback) callback(false); return; }
    if (HMACAuth::getSessionToken().empty()) { if (callback) callback(false); return; }

    int accountID = am->m_accountID;
    std::string url = fmt::format("{}/wc-event/state/{}", SERVER_URL, accountID);

    auto req = web::WebRequest();
    HMACAuth::signGetRequest(req, accountID);

    s_wcStateListener.spawn(
        req.get(url),
        [callback](web::WebResponse res) {
            if (!res.ok() || !res.json().isOk()) {
                if (callback) callback(false);
                return;
            }
            auto data = res.json().unwrap();

            auto& wc = g_streakData.wcEvent;
            std::map<std::string, std::pair<std::string, bool>> mine;
            for (const auto& m : wc.matches) mine[m.matchId] = { m.myVote, m.claimed };

            StreakData::WcEventState fresh;
            StreakData::parseWcEvent(data, fresh);
            fresh.correctPredictions = wc.correctPredictions; // el state no lo trae
            for (auto& m : fresh.matches) {
                auto it = mine.find(m.matchId);
                if (it != mine.end()) {
                    m.myVote = it->second.first;
                    m.claimed = it->second.second;
                }
            }
            g_streakData.wcEvent = fresh;
            if (callback) callback(true);
        }
    );
}

void wcEventVoteOnServer(const std::string& matchId, const std::string& team,
                         std::function<void(bool, matjson::Value)> callback) {
    auto am = GJAccountManager::sharedState();
    if (!am || am->m_accountID == 0) { callback(false, matjson::Value()); return; }
    if (HMACAuth::getSessionToken().empty()) { callback(false, matjson::Value()); return; }

    int accountID = am->m_accountID;
    std::string url = fmt::format("{}/wc-event/vote", SERVER_URL);

    matjson::Value payload = matjson::Value::object();
    payload.set("match_id", matchId);
    payload.set("team", team);

    auto req = web::WebRequest();
    HMACAuth::signRequest(req, accountID, payload);

    log::info("Sending WC event vote ({} -> {}) to server...", matchId, team);

    s_wcEventListener.spawn(
        req.bodyJSON(payload).post(url),
        [callback, matchId](web::WebResponse res) {
            if (res.ok() && res.json().isOk()) {
                auto data = res.json().unwrap();
                if (auto* m = findWcMatch(matchId)) {
                    m->myVote = data["my_vote"].as<std::string>().unwrapOr(m->myVote);
                    m->votesA = data["votes_a"].as<int>().unwrapOr(m->votesA);
                    m->votesB = data["votes_b"].as<int>().unwrapOr(m->votesB);
                }
                callback(true, data);
            }
            else {
                log::error("WC event vote failed: {}", res.code());
                if (res.code() == 401) {
                    HMACAuth::clearSessionToken();
                    loadPlayerDataFromServer();
                }
                callback(false, matjson::Value());
            }
        }
    );
}

void wcEventClaimOnServer(const std::string& matchId, std::function<void(bool, matjson::Value)> callback) {
    auto am = GJAccountManager::sharedState();
    if (!am || am->m_accountID == 0) { callback(false, matjson::Value()); return; }
    if (HMACAuth::getSessionToken().empty()) { callback(false, matjson::Value()); return; }

    int accountID = am->m_accountID;
    std::string url = fmt::format("{}/wc-event/claim", SERVER_URL);

    matjson::Value payload = matjson::Value::object();
    payload.set("match_id", matchId);

    auto req = web::WebRequest();
    HMACAuth::signRequest(req, accountID, payload);

    log::info("Claiming WC event reward on server ({})...", matchId);

    s_wcEventListener.spawn(
        req.bodyJSON(payload).post(url),
        [callback, matchId](web::WebResponse res) {
            if (res.ok() && res.json().isOk()) {
                auto data = res.json().unwrap();
                if (auto* m = findWcMatch(matchId)) m->claimed = true;
                g_streakData.wcEvent.correctPredictions =
                    data["correct_predictions"].as<int>().unwrapOr(g_streakData.wcEvent.correctPredictions);
                callback(true, data);
            }
            else {
                log::error("WC event claim failed: {}", res.code());
                if (res.code() == 401) {
                    HMACAuth::clearSessionToken();
                    loadPlayerDataFromServer();
                }
                callback(false, matjson::Value());
            }
        }
    );
}

void spinStandardRouletteOnServer(int spins, int discountPercent, std::function<void(bool, matjson::Value)> callback) {
    auto am = GJAccountManager::sharedState();
    if (!am || am->m_accountID == 0) { callback(false, matjson::Value()); return; }
    if (HMACAuth::getSessionToken().empty()) { callback(false, matjson::Value()); return; }

    int accountID = am->m_accountID;
    std::string url = fmt::format("{}/roulette/spin", SERVER_URL);

    matjson::Value payload = matjson::Value::object();
    payload.set("spins", spins);
    payload.set("discount_percent", discountPercent);

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

void spinGemRouletteOnServer(int discountPercent, std::function<void(bool, matjson::Value)> callback) {
    auto am = GJAccountManager::sharedState();
    if (!am || am->m_accountID == 0) { callback(false, matjson::Value()); return; }
    if (HMACAuth::getSessionToken().empty()) { callback(false, matjson::Value()); return; }

    int accountID = am->m_accountID;
    std::string url = fmt::format("{}/gem-roulette/spin", SERVER_URL);

    matjson::Value payload = matjson::Value::object();
    payload.set("discount_percent", discountPercent);

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
