#include <Geode/Geode.hpp>
#include <Geode/modify/GameManager.hpp>
#include <Geode/binding/GJAccountManager.hpp>
#include "StreakData.h"
#include "FirebaseManager.h" 
#include "HMACAuth.h"

using namespace geode::prelude;

static int g_lastAccountID = -1;

class $modify(AccountWatcher, GameManager) {
    void update(float dt) {
        GameManager::update(dt);

        auto am = GJAccountManager::sharedState();
        if (!am) return;

        int currentID = am->m_accountID;

        if (g_lastAccountID == -1) {
            g_lastAccountID = currentID;
            if (currentID != 0 && !g_streakData.isInitialized()) {
                g_streakData.isDataLoaded = false;
                loadPlayerDataFromServer();
            }
            return;
        }

        if (currentID != g_lastAccountID) {
            log::info("Account change detected ({} -> {})", g_lastAccountID, currentID);      
            HMACAuth::clearSessionToken();

            if (currentID == 0) {
                log::info("Logging out. Resetting data.");
                g_streakData.resetToDefault();
                g_streakData.isDataLoaded = true;
            }
            else {
                log::info("Login (ID: {}). Loading data...", currentID);
                g_streakData.resetToDefault();
                g_streakData.isDataLoaded = false;
                loadPlayerDataFromServer();
            }

            g_lastAccountID = currentID;
        }
    }
};