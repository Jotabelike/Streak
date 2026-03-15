#pragma once
#include "StreakCommon.h"
#include "../StreakData.h"
#include "../FirebaseManager.h"
#include "QualityNode.h"
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/ScrollLayer.hpp>
#include <Geode/cocos/misc_nodes/CCClippingNode.h>
#include <Geode/binding/GameLevelManager.hpp>
#include <Geode/binding/GJGameLevel.hpp>

using namespace geode::prelude;

class AchievementsPopup : public Popup {
protected:
    geode::ScrollLayer* m_scrollLayer = nullptr;

    bool init() override {
        if (!Popup::init(340.f, 240.f, "geode.loader/GE_square03.png")) return false;

        this->setTitle("Streak Achievements");

        auto winSize = m_mainLayer->getContentSize();

        m_scrollLayer = geode::ScrollLayer::create({ 310.f, 185.f });
        m_scrollLayer->setPosition({ winSize.width / 2 - 155.f, 20.f });
        m_mainLayer->addChild(m_scrollLayer);

        populateAchievements();

        return true;
    }

    struct TaskData {
        std::string desc;
        int current;
        int required;
    };

     
    struct RealPlayerStats {
        int harderLevels = 0;
        int insaneLevels = 0;
        int easyDemons = 0;
        int mediumDemons = 0;
        int hardDemons = 0;
        int insaneDemons = 0;
        int extremeDemons = 0;
    };

    
    RealPlayerStats fetchRealPlayerStats() {
        RealPlayerStats stats;

        auto glm = GameLevelManager::sharedState();
        if (!glm || !glm->m_onlineLevels) return stats;

        for (auto [key, level] : CCDictionaryExt<gd::string, GJGameLevel*>(glm->m_onlineLevels)) {
            if (!level) continue;

            
            if (level->m_normalPercent < 100) continue;

            bool isDemon = level->m_demon.value();

            if (isDemon) {
           
                switch (level->m_demonDifficulty) {
                case 3: stats.easyDemons++; break;
                case 4: stats.mediumDemons++; break;
                case 0: stats.hardDemons++; break;
                case 5: stats.insaneDemons++; break;
                case 6: stats.extremeDemons++; break;
                default: stats.hardDemons++; break;  
                }
            }
            else {
                int stars = level->m_stars;
                if (stars == 6 || stars == 7) stats.harderLevels++;
                else if (stars == 8 || stars == 9) stats.insaneLevels++;
            }
        }

        return stats;
    }

    void populateAchievements() {
        int streakPoints = g_streakData.totalStreakPoints;
        int currentStreakDays = g_streakData.currentStreak;

  
        RealPlayerStats ps = fetchRealPlayerStats();

     
        std::vector<TaskData> specialTasks = {
            {"Complete 5 Harder levels", ps.harderLevels, 5},
            {"Collect 100 Streak Points", streakPoints, 100},
            {"Reach 10 days of streak", currentStreakDays, 10},
            {"Complete 1 Easy Demon", ps.easyDemons, 1},
            {"Complete 1 Medium Demon", ps.mediumDemons, 1}
        };

       
        std::vector<TaskData> legendaryTasks = {
            {"Complete 50 Insane levels", ps.insaneLevels, 50},
            {"Collect 2000 Streak Points", streakPoints, 2000},
            {"Complete 20 Easy Demons", ps.easyDemons, 20},
            {"Complete 10 Medium Demons", ps.mediumDemons, 10},
            {"Complete 3 Hard Demons", ps.hardDemons, 3}
        };
 
        std::vector<TaskData> mythicTasks = {
            {"Complete 100 Easy Demons", ps.easyDemons, 100},
            {"Collect 10,000 Streak Points", streakPoints, 10000},
            {"Complete 50 Medium Demons", ps.mediumDemons, 50},
            {"Complete 5 Insane Demons", ps.insaneDemons, 5},
            {"Complete 3 Extreme Demons", ps.extremeDemons, 3}
        };

        float contentHeight = 670.f;
        m_scrollLayer->m_contentLayer->setContentSize({ 310.f, contentHeight });

        float currentY = contentHeight - 21.f;

        createCategorySection(0, "achievement_1.png"_spr, specialTasks, "achievement_badge1", "banner_51", currentY);
        createCategorySection(3, "achievement_2.png"_spr, legendaryTasks, "achievement_badge2", "banner_52", currentY);
        createCategorySection(4, "achievement_3.png"_spr, mythicTasks, "achievement_badge3", "banner_53", currentY);

        m_scrollLayer->scrollToTop();
    }

    void createCategorySection(int qualityCategory, 
        const std::string& iconName,
        const std::vector<TaskData>& tasks,
        const std::string& rewardId, 
        const std::string& bannerId, 
        float& currentY) {

        bool allComplete = true;
        for (const auto& task : tasks) {
            if (task.current < task.required) {
                allComplete = false;
                break;
            }
        }

        
        bool isUnlocked = g_streakData.isBadgeUnlocked(rewardId);
        bool isBannerUnlocked = g_streakData.isBannerUnlocked(bannerId);

        if (allComplete && (!isUnlocked || !isBannerUnlocked)) {
            g_streakData.unlockBadge(rewardId);
            g_streakData.unlockBanner(bannerId);
            g_streakData.save();
            updatePlayerDataInFirebase();
            isUnlocked = true;
        }

        
        for (const auto& task : tasks) {
            const float cellW = 310.f;
            const float cellH = 38.f;

            auto cellNode = CCNode::create();
            cellNode->setContentSize({ cellW, cellH });
            cellNode->setPosition({ 155.f - (cellW / 2.f), currentY - (cellH / 2.f) });
            m_scrollLayer->m_contentLayer->addChild(cellNode);

            auto cellBg = cocos2d::extension::CCScale9Sprite::create(
                "square02_001.png",
                CCRect(0, 0, 80, 80),
                CCRect(10, 10, 60, 60)
            );
            cellBg->setContentSize({ cellW, cellH });
            cellBg->setColor({ 0, 0, 0 });
            cellBg->setOpacity(120);
            cellBg->setPosition({ cellW / 2.f, cellH / 2.f });
            cellNode->addChild(cellBg);

            auto stencil = cocos2d::extension::CCScale9Sprite::create("square02b_001.png");
            stencil->setContentSize({ cellW, cellH });
            stencil->setPosition({ cellW / 2.f, cellH / 2.f });

            auto clipNode = CCClippingNode::create(stencil);
            clipNode->setAlphaThreshold(0.05f);
            clipNode->setContentSize({ cellW, cellH });
            clipNode->setPosition({ 0.f, 0.f });
            cellNode->addChild(clipNode);

          
            auto bannerInfo = g_streakData.getBannerInfo(bannerId);
            if (bannerInfo) {
                auto bannerSpr = CCSprite::create(bannerInfo->spriteName.c_str());
                if (bannerSpr) {
                    bannerSpr->setPosition({ cellW / 2.f, cellH / 2.f });

                    auto bSize = bannerSpr->getContentSize();
                    bannerSpr->setScaleX(cellW / bSize.width);
                    bannerSpr->setScaleY(cellH / bSize.height);

                  
                    clipNode->addChild(bannerSpr);
                }
            }

       
            int safeCurrent = std::min(task.current, task.required);
            float progress = (task.required > 0) ? static_cast<float>(safeCurrent) / task.required : 0.f;
            float barWidth = cellW * progress;

            if (barWidth > 0.f) {
                ccColor4B startColor;
                ccColor4B endColor;
 
                switch (qualityCategory) {
                case 3: // Legendary
                    startColor = ccc4(255, 90, 0, 220);   
                    endColor = ccc4(255, 240, 150, 220);  
                    break;
                case 4: // Mythic
                    startColor = ccc4(180, 100, 255, 220); 
                    endColor = ccc4(200, 240, 255, 220);  
                    break;
                default: // Common / Special / Epic (0, 1, 2)
                    startColor = ccc4(10, 60, 230, 220);    
                    endColor = ccc4(40, 180, 255, 220);   
                    break;
                }

                auto progressBar = CCLayerGradient::create(startColor, endColor, ccp(1.f, 0.f));
                progressBar->setContentSize({ barWidth, 6.f });
                progressBar->setPosition({ 0.f, 0.f });
                clipNode->addChild(progressBar);
            }

            auto icon = CCSprite::create(iconName.c_str());
            if (!icon) icon = CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
            icon->setPosition({ 22.f, cellH / 2.f });
            icon->setScale(0.28f);
            cellNode->addChild(icon);

            if (isUnlocked) {
                auto check = CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
                check->setPosition({ 22.f, cellH / 2.f });
                check->setScale(0.28f);
                cellNode->addChild(check);
            }

            auto desc = CCLabelBMFont::create(task.desc.c_str(), "bigFont.fnt");
            desc->setAnchorPoint({ 0.f, 0.5f });
            desc->setPosition({ 45.f, (cellH / 2.f) + 6.f });
            desc->setScale(0.35f);
            cellNode->addChild(desc);

            auto qNode = QualityNode::create();
            qNode->setCategory(qualityCategory, false);
            qNode->setPosition({ 63.f, (cellH / 2.f) - 6.f });
            qNode->setScale(0.45f);
            cellNode->addChild(qNode);

            std::string progStr = fmt::format("{}/{}", safeCurrent, task.required);
            auto prog = CCLabelBMFont::create(progStr.c_str(), "goldFont.fnt");
            prog->setAnchorPoint({ 1.f, 0.5f });
            prog->setPosition({ cellW - 15.f, cellH / 2.f });
            prog->setScale(0.35f);

            if (safeCurrent < task.required) {
                prog->setColor({ 200, 200, 200 });
            }
            else {
                prog->setColor({ 100, 255, 100 });
            }
            cellNode->addChild(prog);

            currentY -= 43.f;
        }
    }


    void onClose(CCObject* sender) override {
        Popup::onClose(sender);
    }

public:
    static AchievementsPopup* create() {
        auto ret = new AchievementsPopup();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};