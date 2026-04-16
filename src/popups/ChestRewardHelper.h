#pragma once
#include <Geode/Geode.hpp>
#include "../StreakData.h"
#include "StreakChestPopup.h"  
#include <random>

using namespace geode::prelude;

class ChestRewardHelper {
public:
    static void openRandomChest(std::function<void()> reloadFunc) {   
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> gemDist(1, 10);
        std::uniform_int_distribution<> starDist(5, 20);
        std::uniform_int_distribution<> ticketDist(200, 1000);
        int gems = gemDist(gen);
        int stars = starDist(gen);
        int tickets = ticketDist(gen);
        int xp = 0;       
        auto chestPopup = StreakChestPopup::create(stars, tickets, gems, xp, reloadFunc);
        if (chestPopup) {
            chestPopup->show();
        }
    }
};