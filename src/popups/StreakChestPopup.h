#pragma once
#include <Geode/ui/Popup.hpp>
#include "../StreakData.h"

using namespace geode::prelude;

class StreakChestPopup : public Popup {
protected:
    int m_superStars;
    int m_starTickets;
    int m_gems;
    int m_rewardXP;
    int m_startStars = 0;
    int m_startTickets = 0;
    int m_startGems = 0;
    int m_startXP = 0;
    int m_rarity = 1;
    bool m_testMode = false;
    std::function<void()> m_reloadFunc;

    CCSprite* m_chestSpr;

    bool init(int superStars, int starTickets, int gems, int rewardXP,
              int rarity, bool testMode, std::function<void()> reloadFunc);
    void playOpenAnimation();
    void showRewards();
    void addStarCategory(float chestY);
    std::string getChestFrameName(int frameIdx) const;
    virtual void keyBackClicked() override;
public:
    static StreakChestPopup* create(int superStars, int starTickets, int gems, int rewardXP,
                                    std::function<void()> reloadFunc);
    static StreakChestPopup* create(int superStars, int starTickets, int gems, int rewardXP,
                                    int rarity, std::function<void()> reloadFunc);

    static void rollRewardsForRarity(int rarity,
                                     int& outStars, int& outTickets, int& outGems, int& outXP);

    static StreakChestPopup* createTest(int rarity);

    static StreakChestPopup* createOpen(int superStars, int starTickets, int gems, int rewardXP,
                                        int rarity, std::function<void()> reloadFunc);
};
