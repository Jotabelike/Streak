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
            auto fadeOut = CCFadeTo::create(0.05f, 100);
            auto fadeIn = CCFadeTo::create(0.05f, 255);
            auto delay = CCDelayTime::create(1.5f);
            animAction = CCRepeatForever::create(CCSequence::create(fadeOut, fadeIn, fadeOut, fadeIn, delay, nullptr));
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

       
        else if (animID == "Bounce" || animID == "Shake" || animID == "DVD" || animID == "Float" ||
            animID == "Dynamic jump" || animID == "Wave" || animID == "Domino") {

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

                        if (animID == "Dynamic jump") {
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
                            auto rot1 = CCRotateTo::create(0.15f, 15.f);
                            auto rot2 = CCRotateTo::create(0.15f, -8.f);
                            auto rot3 = CCRotateTo::create(0.15f, 0.f);
                            auto seq = CCSequence::create(delayBefore, rot1, rot2, rot3, delayAfter, nullptr);
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