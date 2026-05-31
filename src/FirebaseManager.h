#pragma once
#include <functional>
#include <string>
#include <matjson.hpp>
void updatePlayerDataInFirebase();
void loadPlayerDataFromServer();
void completeLevelInFirebase(int stars);
void claimOnServer(const std::string& endpoint, const matjson::Value& payload, std::function<void(bool)> callback);
void refreshPendingLevelRewardsFromServer(std::function<void(bool)> callback);
