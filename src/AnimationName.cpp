#include "NameModifiers.h"

namespace NameModifiers {
    void applyAnimation(CCLabelBMFont* label, const std::string& animID) {
        if (!label) return;

        label->stopActionByTag(9999);
        label->setRotation(0.f);

        float baseScale = label->getScale();

        for (auto letter : label->getChildrenExt<CCNode*>()) {
            if (letter) {
                letter->stopAllActions();
                letter->setRotation(0.f);
                letter->setScale(1.0f);
            }
        }

        label->setString(label->getString());

        CCAction* animAction = nullptr;

      
        if (animID == "Pulse") {
            auto scaleUp = CCScaleTo::create(0.4f, baseScale * 1.15f);
            auto scaleDown = CCScaleTo::create(0.4f, baseScale);
            animAction = CCRepeatForever::create(CCSequence::create(CCEaseInOut::create(scaleUp, 2.0f), CCEaseInOut::create(scaleDown, 2.0f), nullptr));
        }
        else if (animID == "Swing") {
            auto rotRight = CCRotateTo::create(0.8f, 4.0f);
            auto rotLeft = CCRotateTo::create(0.8f, -4.0f);
            animAction = CCRepeatForever::create(CCSequence::create(CCEaseSineInOut::create(rotRight), CCEaseSineInOut::create(rotLeft), nullptr));
        }
        else if (animID == "Glitch") {
         
            auto skew1 = CCSkewTo::create(0.03f, 15.f, 0.f);
            auto move1 = CCMoveBy::create(0.03f, { 5.f * baseScale, 0.f });
            auto sp1 = CCSpawn::create(skew1, move1, nullptr);

            auto skew2 = CCSkewTo::create(0.03f, -15.f, 0.f);
            auto move2 = CCMoveBy::create(0.03f, { -10.f * baseScale, 2.f * baseScale });
            auto sp2 = CCSpawn::create(skew2, move2, nullptr);

            auto skew3 = CCSkewTo::create(0.03f, 0.f, 0.f);
            auto move3 = CCMoveBy::create(0.03f, { 5.f * baseScale, -2.f * baseScale });
            auto sp3 = CCSpawn::create(skew3, move3, nullptr);

       
            auto skew4 = CCSkewTo::create(0.03f, 0.f, 0.f);
            auto move4 = CCMoveBy::create(0.03f, { -5.f * baseScale, 0.f });
            auto sp4 = CCSpawn::create(skew4, move4, nullptr);

            auto delay1 = CCDelayTime::create(1.5f);  
            auto delay2 = CCDelayTime::create(0.15f);  
 
            animAction = CCRepeatForever::create(CCSequence::create(
                delay1, sp1, sp2, sp3, delay2, sp1, sp4, nullptr
            ));
        }
        else if (animID == "Spin") {
            animAction = CCRepeatForever::create(CCRotateBy::create(2.0f, 360.f));
        }
        else if (animID == "Jelly") {
            auto s1 = CCScaleTo::create(0.2f, baseScale * 1.25f, baseScale * 0.75f);
            auto s2 = CCScaleTo::create(0.2f, baseScale * 0.75f, baseScale * 1.25f);
            auto s3 = CCScaleTo::create(0.2f, baseScale, baseScale);
            auto delay = CCDelayTime::create(1.0f);
            animAction = CCRepeatForever::create(CCSequence::create(s1, s2, s3, delay, nullptr));
        }
        else if (animID == "Heartbeat") {
            auto beat1 = CCScaleTo::create(0.12f, baseScale * 1.25f);
            auto beat2 = CCScaleTo::create(0.12f, baseScale * 1.0f);
            auto beat3 = CCScaleTo::create(0.12f, baseScale * 1.25f);
            auto beat4 = CCScaleTo::create(0.12f, baseScale * 1.0f);
            auto rest = CCDelayTime::create(1.0f);
            animAction = CCRepeatForever::create(CCSequence::create(beat1, beat2, beat3, beat4, rest, nullptr));
        }
        else if (animID == "Blink") {
   
            auto fadeOut = CCFadeTo::create(0.2f, 50);
            auto fadeIn = CCFadeTo::create(0.2f, 255);
            auto delay = CCDelayTime::create(0.5f);
            animAction = CCRepeatForever::create(CCSequence::create(fadeOut, fadeIn, delay, nullptr));
        }
        else if (animID == "Tremble") {
        
            auto m1 = CCMoveBy::create(0.03f, { 3.f * baseScale, 0.f });
            auto m2 = CCMoveBy::create(0.03f, { -6.f * baseScale, 0.f });
            auto m3 = CCMoveBy::create(0.03f, { 3.f * baseScale, 0.f });
            auto delay = CCDelayTime::create(0.06f);
            animAction = CCRepeatForever::create(CCSequence::create(m1, m2, m3, delay, nullptr));
        }
        else if (animID == "Wobble") {
            auto rot1 = CCRotateTo::create(0.1f, 5.f);
            auto rot2 = CCRotateTo::create(0.1f, -5.f);
            auto scale1 = CCScaleTo::create(0.1f, baseScale * 1.05f);
            auto scale2 = CCScaleTo::create(0.1f, baseScale * 0.95f);
            auto spawn1 = CCSpawn::create(rot1, scale1, nullptr);
            auto spawn2 = CCSpawn::create(rot2, scale2, nullptr);
            animAction = CCRepeatForever::create(CCSequence::create(spawn1, spawn2, nullptr));
        }

 
        else if (animID == "Bounce" || animID == "Shake" || animID == "DVD" || animID == "Float" ||
            animID == "Dynamic Jump" || animID == "Wave" || animID == "Domino" || animID == "Spiral" || animID == "Squish") {

            int totalLetters = label->getChildrenCount();
            if (totalLetters > 0) {
                int i = 0;
                for (auto letter : label->getChildrenExt<CCSprite*>()) {
                    if (!letter) continue;

                    if (animID == "Bounce") {
                        auto moveUp = CCMoveBy::create(0.3f, { 0, 4.f * baseScale });
                        auto moveDown = CCMoveBy::create(0.3f, { 0, -4.f * baseScale });
                        auto seq = CCSequence::create(CCEaseSineOut::create(moveUp), CCEaseSineIn::create(moveDown), nullptr);
                        letter->runAction(CCRepeatForever::create(seq));
                    }
                    else if (animID == "Shake") {
              
                        auto m1 = CCMoveBy::create(0.05f, { 2.f * baseScale, 2.f * baseScale });
                        auto m2 = CCMoveBy::create(0.05f, { -4.f * baseScale, -2.f * baseScale });
                        auto m3 = CCMoveBy::create(0.05f, { 2.f * baseScale, 4.f * baseScale });
                        auto m4 = CCMoveBy::create(0.05f, { -2.f * baseScale, -4.f * baseScale });
                        auto m5 = CCMoveBy::create(0.05f, { 2.f * baseScale, 0.f });
                        auto seq = CCSequence::create(m1, m2, m3, m4, m5, nullptr);
                        letter->runAction(CCRepeatForever::create(seq));
                    }
                    else if (animID == "DVD") {
                        float x = 18.f * baseScale;
                        float y = 12.f * baseScale;
                        auto m1 = CCMoveBy::create(0.8f, { x, y });
                        auto m2 = CCMoveBy::create(0.8f, { -x * 1.5f, -y * 2.f });
                        auto m3 = CCMoveBy::create(0.8f, { -x * 0.5f, y * 1.5f });
                        auto m4 = CCMoveBy::create(0.8f, { x, -y * 0.5f });
                        auto seq = CCSequence::create(m1, m2, m3, m4, nullptr);
                        letter->runAction(CCRepeatForever::create(seq));
                    }
                    else if (animID == "Float") {
                        auto moveUp = CCMoveBy::create(1.5f, { 0.f, 6.f * baseScale });
                        auto moveDown = CCMoveBy::create(1.5f, { 0.f, -6.f * baseScale });
                        auto seq = CCSequence::create(CCEaseSineInOut::create(moveUp), CCEaseSineInOut::create(moveDown), nullptr);
                        letter->runAction(CCRepeatForever::create(seq));
                    }
                    else {
                        auto delayBefore = CCDelayTime::create(i * 0.08f);
                        float remainingTime = (totalLetters - i) * 0.08f + 0.8f;
                        auto delayAfter = CCDelayTime::create(remainingTime);

                        if (animID == "Dynamic Jump") {
                            auto moveUp = CCMoveBy::create(0.15f, { 0, 15.f });
                            auto moveDown = CCMoveBy::create(0.15f, { 0, -15.f });
                            auto seq = CCSequence::create(delayBefore, CCEaseSineOut::create(moveUp), CCEaseSineIn::create(moveDown), delayAfter, nullptr);
                            letter->runAction(CCRepeatForever::create(seq));
                        }
                        else if (animID == "Wave") {
                            auto rot1 = CCRotateTo::create(0.15f, 15.f);
                            auto rot2 = CCRotateTo::create(0.15f, -15.f);
                            auto rot3 = CCRotateTo::create(0.15f, 0.f);
                            auto seq = CCSequence::create(delayBefore, rot1, rot2, rot3, delayAfter, nullptr);
                            letter->runAction(CCRepeatForever::create(seq));
                        }
                        else if (animID == "Domino") {
                            auto rot1 = CCRotateTo::create(0.15f, 60.f);
                            auto rot2 = CCRotateTo::create(0.25f, 0.f);
                            auto seq = CCSequence::create(delayBefore, CCEaseSineIn::create(rot1), CCEaseElasticOut::create(rot2, 0.6f), delayAfter, nullptr);
                            letter->runAction(CCRepeatForever::create(seq));
                        }
                        else if (animID == "Spiral") {
                            auto rot = CCRotateBy::create(0.4f, 360.f);
                            auto seq = CCSequence::create(delayBefore, CCEaseSineInOut::create(rot), delayAfter, nullptr);
                            letter->runAction(CCRepeatForever::create(seq));
                        }
                        else if (animID == "Squish") {
                            auto scale1 = CCScaleTo::create(0.15f, 1.3f, 0.7f);
                            auto scale2 = CCScaleTo::create(0.15f, 0.7f, 1.3f);
                            auto scale3 = CCScaleTo::create(0.15f, 1.0f, 1.0f);
                            auto seq = CCSequence::create(delayBefore, scale1, scale2, scale3, delayAfter, nullptr);
                            letter->runAction(CCRepeatForever::create(seq));
                        }
                    }
                    i++;
                }
            }
        }

        if (animAction) {
            animAction->setTag(9999);
            label->runAction(animAction);
        }
    }
}