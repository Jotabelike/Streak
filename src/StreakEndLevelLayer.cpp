#include <Geode/Geode.hpp>
#include <Geode/modify/EndLevelLayer.hpp>
#include "RewardNotification.h"
#include "StreakData.h"
#include "StreakProgressBar.h"

using namespace geode::prelude;

namespace {
    void resetScaleAction(CCNode* node, float delay, float scale) {
        if (!node) return;
        node->stopAllActions();
        node->setScale(3.f * scale);
        node->runAction(CCSequence::create(
            CCHide::create(),
            CCDelayTime::create(delay),
            CCShow::create(),
            CCEaseBounceOut::create(CCScaleTo::create(0.3f, scale)),
            nullptr
        ));
    }

    void runFadeInAction(CCNode* node, float delay) {
        if (!node) return;
        node->runAction(CCSequence::create(
            CCDelayTime::create(delay),
            CCFadeIn::create(0.3f),
            nullptr
        ));
    }

    void runContainerAction(CCNode* container, CCNode* lbl, CCNode* icon, float delay, float scale) {
        resetScaleAction(container, delay, scale);
        runFadeInAction(lbl, delay);
        runFadeInAction(icon, delay);
    }
}

class $modify(StreakEndLevelLayer, EndLevelLayer) {
    struct Fields {
        int m_pointsGained = 0;
        CCNode* m_streakContainer = nullptr;
        CCLabelBMFont* m_lbl = nullptr;
        CCSprite* m_icon = nullptr;
    };

    void addStreakRewardLayer() {
        auto f = m_fields.self();
        if (!f->m_streakContainer) return;
        auto winSize = CCDirector::get()->getWinSize();
        CCPoint spawnPos = f->m_streakContainer->convertToWorldSpaceAR({ 0, 0 });
        int startAmount = StreakDataBridge::currentTotal - f->m_pointsGained;
        if (startAmount < 0) startAmount = 0;
        RewardNotification::show("streak_point.png"_spr, startAmount, f->m_pointsGained, spawnPos);
    }

    void showLayer(bool p0) {
        EndLevelLayer::showLayer(p0);
        auto f = m_fields.self();
        f->m_pointsGained = StreakDataBridge::pointsGained;
        int animMode = geode::Mod::get()->getSavedValue<int>("end_level_anim_mode", 2);     
        if (f->m_pointsGained <= 0 || animMode == 0) return;   
        if (animMode == 1) {
            int startPoints = g_streakData.streakPointsToday - f->m_pointsGained;
            if (startPoints < 0) startPoints = 0;
            int reqPoints = g_streakData.getRequiredPoints();
            auto bar = StreakProgressBar::create(startPoints, f->m_pointsGained, reqPoints);
            bar->setID("streak-progress-bar"_spr);
            auto winSize = CCDirector::get()->getWinSize();
            bar->setPosition({ winSize.width / 2, winSize.height - 110.f });  
            bar->setScale(0.f);
            m_mainLayer->addChild(bar, 10);    
            bar->runAction(CCSequence::create(
                CCDelayTime::create(0.5f),
                CCEaseBounceOut::create(CCScaleTo::create(0.5f, 1.0f)),
                CCCallFunc::create(bar, callfunc_selector(StreakProgressBar::animate)),
                nullptr
            ));
            return;  
        }

      
        auto starContainer = m_mainLayer->getChildByID("star-container");
        auto orbContainer = m_mainLayer->getChildByID("orb-container");
        auto diamondContainer = m_mainLayer->getChildByID("diamond-container");
        if (!starContainer) {
            starContainer = m_mainLayer->getChildByID("moon-container");
        }

       
        f->m_icon = CCSprite::create("streak_point.png"_spr);
        if (!f->m_icon) f->m_icon = CCSprite::createWithSpriteFrameName("starSmall_001.png");        
        f->m_icon->setScale(0.17f);
        f->m_icon->setAnchorPoint({ 1.f, 0.5f });   
        f->m_icon->setPositionX(-1.f);
        f->m_icon->setOpacity(0);    
        f->m_lbl = CCLabelBMFont::create(fmt::format("+{}", f->m_pointsGained).c_str(), "bigFont.fnt");
        f->m_lbl->setScale(0.625f);
        f->m_lbl->setAnchorPoint({ 0.f, 0.5f });
        f->m_lbl->setPositionX(1.f);
        f->m_lbl->setOpacity(0);
        f->m_streakContainer = CCNode::create();
        f->m_streakContainer->setID("streak-container"_spr);
        auto winSize = CCDirector::get()->getWinSize();      
        f->m_streakContainer->setPosition({ winSize.width / 2 - 120.f, winSize.height / 2 + 22.f });
        f->m_streakContainer->addChild(f->m_icon);
        f->m_streakContainer->addChild(f->m_lbl);
        m_mainLayer->addChild(f->m_streakContainer, 10);

        float delay = 0.f;
        if (m_coinsToAnimate && !GameManager::get()->getGameVariable("0168")) {
            delay = m_coinsToAnimate->count() * 0.35f + 0.7f;
        }

        auto runFinished = [&](CCNode* node, SEL_CallFunc func) {
            node->runAction(CCSequence::create(
                CCDelayTime::create(delay + 0.2f),
                CCCallFunc::create(this, func),
                nullptr
            ));
            };

      
        if (!starContainer && !orbContainer && !diamondContainer) {
            runContainerAction(f->m_streakContainer, f->m_lbl, f->m_icon, delay, 1.f);
            f->m_streakContainer->runAction(CCSequence::create(
                CCDelayTime::create(delay + 0.2f),
                CCCallFunc::create(this, callfunc_selector(StreakEndLevelLayer::addStreakRewardLayer)),
                nullptr
            ));
        }
        else {
            int count = (starContainer != nullptr) + (orbContainer != nullptr) + (diamondContainer != nullptr);
            float scale = count == 3 ? 0.67f : (count == 2 ? 0.77f : 1.f);

       
            if (starContainer) starContainer->setScale(scale);
            if (orbContainer) orbContainer->setScale(scale);
            if (diamondContainer) diamondContainer->setScale(scale);
            auto top = starContainer ? starContainer : (orbContainer ? orbContainer : diamondContainer);
            auto prev = top;

       
            for (auto node : { orbContainer, diamondContainer }) {
                if (node && node != top) {
                    node->setPositionY(prev->getPositionY() - 38.f * scale);
                    prev = node;
                }
            }

            for (auto node : { starContainer, orbContainer, diamondContainer }) {
                if (node) resetScaleAction(node, delay, scale);
            }

         
            runContainerAction(f->m_streakContainer, f->m_lbl, f->m_icon, delay, 1.f);  
            if (starContainer) runFinished(starContainer, callfunc_selector(EndLevelLayer::starEnterFinished));
            if (orbContainer) runFinished(orbContainer, callfunc_selector(EndLevelLayer::currencyEnterFinished));
            if (diamondContainer) runFinished(diamondContainer, callfunc_selector(EndLevelLayer::diamondEnterFinished));
        }
    }

    void playEndEffect() {
        auto f = m_fields.self();

        if (f->m_pointsGained <= 0) {
            return EndLevelLayer::playEndEffect();
        }

        StreakDataBridge::shouldReward = true;
        StreakDataBridge::currentTotal = g_streakData.totalStreakPoints;
        auto winSize = CCDirector::get()->getWinSize();
        StreakDataBridge::spawnPos = f->m_streakContainer
            ? f->m_streakContainer->convertToWorldSpaceAR({ 0, 0 })
            : ccp(winSize.width / 2, winSize.height / 2);

        EndLevelLayer::playEndEffect();
        StreakDataBridge::shouldReward = false;
    }
};