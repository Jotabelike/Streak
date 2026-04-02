#pragma once
#include <Geode/Geode.hpp>

using namespace geode::prelude;

namespace StreakAnimations {
    
    
    static void applyPremiumHover(CCNode* node) {
        if (!node) return;   
        auto moveUp = CCEaseSineInOut::create(CCMoveBy::create(1.5f, {0.f, 6.f}));
        auto moveDown = CCEaseSineInOut::create(CCMoveBy::create(1.5f, {0.f, -6.f}));    
        auto moveSeq = CCSequence::create(moveUp, moveDown, nullptr);
        node->runAction(CCRepeatForever::create(moveSeq));   
        float baseScale = node->getScale(); 
        auto scaleUp = CCEaseSineInOut::create(CCScaleTo::create(1.5f, baseScale * 1.03f));
        auto scaleDown = CCEaseSineInOut::create(CCScaleTo::create(1.5f, baseScale));     
        auto scaleSeq = CCSequence::create(scaleUp, scaleDown, nullptr);
        node->runAction(CCRepeatForever::create(scaleSeq));
    }

    static void applySimpleHover(CCNode* node) {
        if (!node) return;
        auto moveUp = CCEaseSineInOut::create(CCMoveBy::create(1.5f, {0.f, 6.f}));
        auto moveDown = CCEaseSineInOut::create(CCMoveBy::create(1.5f, {0.f, -6.f}));
        node->runAction(CCRepeatForever::create(CCSequence::create(moveUp, moveDown, nullptr)));
    }
}