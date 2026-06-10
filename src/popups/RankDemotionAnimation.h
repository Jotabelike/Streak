#pragma once
#include <Geode/Geode.hpp>
#include "../StreakData.h"
#include "../NameModifiers.h"
#include "../utils/RoundedProgressBar.h"

using namespace geode::prelude;

class RankDemotionAnimation : public CCLayer {
protected:
    int m_oldIdx = 0;
    int m_newIdx = 0;
    CCLayer* m_content = nullptr;
    CCLayerColor* m_bg = nullptr;
    CCPoint m_impactPos;

    RoundedProgressBar* m_bar = nullptr;
    CCMenu* m_okMenu = nullptr;
    CCMenuItemSpriteExtra* m_okBtn = nullptr;
    float m_barTarget = 0.f;
    float m_barProgress = 0.f;
    bool m_dismissing = false;

    static constexpr float IMPACT = 2.05f;
    static constexpr float CRASH_DUR = 0.4f;
    static constexpr float BADGE_SCALE = 0.62f;
    static constexpr float IMPACT_SFX_LEAD = 1.4f;
    static constexpr float BAR_FILL_DUR = 0.9f;

    bool initWithRanks(int oldIdx, int newIdx) {
        if (!CCLayer::init()) return false;
        m_oldIdx = oldIdx;
        m_newIdx = newIdx;

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        CCPoint center = winSize / 2;

        float badgeY = winSize.height * 0.56f;
        float nameY = winSize.height * 0.23f + 30.f;
        float barY = nameY - 26.f;
        float okY = winSize.height * 0.08f;
        m_impactPos = ccp(center.x, badgeY);

        this->setTouchEnabled(true);

        FMODAudioEngine::sharedEngine()->playEffect("achievement_01.ogg");

        auto bgLayer = CCLayerColor::create({ 0, 0, 0, 0 });
        this->addChild(bgLayer, 0);
        bgLayer->runAction(CCFadeTo::create(0.5f, 225));
        m_bg = bgLayer;

        auto content = CCLayer::create();
        content->ignoreAnchorPointForPosition(false);
        content->setAnchorPoint({ 0.5f, 0.5f });
        content->setContentSize(winSize);
        content->setPosition(center);
        this->addChild(content, 1);
        m_content = content;

        auto flash = CCLayerColor::create({ 255, 255, 255, 0 });
        this->addChild(flash, 2);
        flash->runAction(CCSequence::create(
            CCDelayTime::create(IMPACT),
            CCFadeTo::create(0.05f, 190),
            CCFadeTo::create(0.30f, 0),
            CCRemoveSelf::create(),
            nullptr
        ));

        auto title = CCLabelBMFont::create("New Season", "bigFont.fnt");
        title->setScale(0.0f);
        title->setPosition({ center.x, winSize.height - 28.f });
        content->addChild(title, 5);
        title->runAction(CCSequence::create(
            CCDelayTime::create(0.2f),
            CCEaseBackOut::create(CCScaleTo::create(0.45f, 0.9f)),
            nullptr
        ));

        const char* subText = "Your rank changed";
        ccColor3B subColor = { 255, 220, 120 };
        if (newIdx > oldIdx) { subText = "Your rank went up!"; subColor = { 150, 255, 150 }; }
        else if (newIdx < oldIdx) { subText = "Your rank dropped"; subColor = { 255, 120, 120 }; }
        auto subtitle = CCLabelBMFont::create(subText, "goldFont.fnt");
        subtitle->setScale(0.0f);
        subtitle->setColor(subColor);
        subtitle->setPosition({ center.x, winSize.height - 52.f });
        content->addChild(subtitle, 5);
        subtitle->runAction(CCSequence::create(
            CCDelayTime::create(IMPACT),
            CCEaseBackOut::create(CCScaleTo::create(0.4f, 0.55f)),
            nullptr
        ));

        if (auto glow = CCSprite::createWithSpriteFrameName("shineBurst_001.png")) {
            glow->setPosition({ center.x, badgeY });
            glow->setColor({ 255, 220, 120 });
            glow->setOpacity(0);
            glow->setScale(3.0f);
            glow->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });
            content->addChild(glow, 0);
            glow->runAction(CCRepeatForever::create(CCRotateBy::create(8.0f, 360.f)));
            glow->runAction(CCSequence::create(
                CCDelayTime::create(0.3f),
                CCFadeTo::create(0.6f, 110),
                nullptr
            ));
        }

        auto oldBadge = CCSprite::create(StreakData::getRankSpriteForIndex(oldIdx).c_str());
        if (oldBadge) {
            oldBadge->setScale(0.0f);
            oldBadge->setPosition({ center.x, badgeY });
            content->addChild(oldBadge, 2);
            oldBadge->runAction(CCSequence::create(
                CCDelayTime::create(0.3f),
                CCEaseBackOut::create(CCScaleTo::create(0.5f, BADGE_SCALE)),
                nullptr
            ));
            oldBadge->runAction(CCSequence::create(
                CCDelayTime::create(IMPACT),
                CCSpawn::create(
                    CCEaseBackIn::create(CCScaleTo::create(0.3f, 0.0f)),
                    CCFadeOut::create(0.3f),
                    nullptr
                ),
                CCRemoveSelf::create(),
                nullptr
            ));
        }

        auto oldName = CCLabelBMFont::create(StreakData::getRankNameForIndex(oldIdx).c_str(), "bigFont.fnt");
        oldName->setScale(0.0f);
        oldName->setPosition({ center.x, nameY });
        content->addChild(oldName, 3);
        NameModifiers::applyColor(oldName, StreakData::getRankColorStyleForIndex(oldIdx));
        oldName->runAction(CCSequence::create(
            CCDelayTime::create(0.6f),
            CCEaseBackOut::create(CCScaleTo::create(0.4f, 0.6f)),
            nullptr
        ));
        oldName->runAction(CCSequence::create(
            CCDelayTime::create(IMPACT),
            CCFadeOut::create(0.2f),
            CCRemoveSelf::create(),
            nullptr
        ));

        if (auto ring = CCSprite::createWithSpriteFrameName("shineBurst_001.png")) {
            ring->setPosition({ center.x, badgeY });
            ring->setScale(0.0f);
            ring->setOpacity(0);
            ring->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });
            content->addChild(ring, 1);
            ring->runAction(CCSequence::create(
                CCDelayTime::create(IMPACT),
                CCSpawn::create(
                    CCEaseSineOut::create(CCScaleTo::create(0.6f, 9.0f)),
                    CCSequence::create(
                        CCFadeTo::create(0.05f, 220),
                        CCFadeOut::create(0.55f),
                        nullptr
                    ),
                    nullptr
                ),
                CCRemoveSelf::create(),
                nullptr
            ));
        }

        auto newBadge = CCSprite::create(StreakData::getRankSpriteForIndex(newIdx).c_str());
        if (newBadge) {
            newBadge->setScale(6.0f);
            newBadge->setOpacity(0);
            newBadge->setVisible(false);
            newBadge->setPosition({ center.x, badgeY });
            content->addChild(newBadge, 4);
            newBadge->runAction(CCSequence::create(
                CCDelayTime::create(IMPACT - CRASH_DUR),
                CCShow::create(),
                CCSpawn::create(
                    CCEaseIn::create(CCScaleTo::create(CRASH_DUR, BADGE_SCALE), 3.0f),
                    CCFadeTo::create(CRASH_DUR * 0.5f, 255),
                    nullptr
                ),
                CCEaseBackOut::create(CCScaleTo::create(0.18f, BADGE_SCALE)),
                nullptr
            ));
            newBadge->runAction(CCSequence::create(
                CCDelayTime::create(IMPACT + 0.4f),
                CCRepeatForever::create(CCSequence::create(
                    CCEaseSineInOut::create(CCScaleTo::create(1.4f, BADGE_SCALE + 0.03f)),
                    CCEaseSineInOut::create(CCScaleTo::create(1.4f, BADGE_SCALE)),
                    nullptr
                )),
                nullptr
            ));
        }

        auto newName = CCLabelBMFont::create(StreakData::getRankNameForIndex(newIdx).c_str(), "bigFont.fnt");
        newName->setScale(0.0f);
        newName->setPosition({ center.x, nameY });
        content->addChild(newName, 5);
        NameModifiers::applyColor(newName, StreakData::getRankColorStyleForIndex(newIdx));
        newName->runAction(CCSequence::create(
            CCDelayTime::create(IMPACT + 0.05f),
            CCEaseBackOut::create(CCScaleTo::create(0.45f, 0.7f)),
            nullptr
        ));

        std::string barText;
        {
            int tokens = g_streakData.streakTokens;
            int curThr = StreakData::getRankThreshold(newIdx);
            int nextThr = StreakData::getNextRankThreshold(curThr);
            if (nextThr < 0) {
                m_barTarget = 1.0f;
                barText = "MAX RANK";
            } else {
                int span = std::max(1, nextThr - curThr);
                m_barTarget = (float)(tokens - curThr) / (float)span;
                barText = fmt::format("{} / {}", tokens, nextThr);
            }
            m_barTarget = std::max(0.0f, std::min(1.0f, m_barTarget));
        }

        m_bar = RoundedProgressBar::create(200.f, 18.f);
        m_bar->setPosition({ center.x, barY });
        m_bar->setGradientColors({ 255, 200, 60 }, { 255, 240, 150 });
        m_bar->setProgress(0.f);
        m_bar->setScale(0.f);
        content->addChild(m_bar, 5);
        m_bar->runAction(CCSequence::create(
            CCDelayTime::create(IMPACT + 0.1f),
            CCEaseBackOut::create(CCScaleTo::create(0.35f, 1.0f)),
            nullptr
        ));

        auto barLabel = CCLabelBMFont::create(barText.c_str(), "bigFont.fnt");
        barLabel->setScale(0.4f);
        barLabel->setPosition(m_bar->getContentSize() / 2.f);
        m_bar->addChild(barLabel, 10);

        content->runAction(CCSequence::create(
            CCDelayTime::create(IMPACT),
            CCMoveBy::create(0.04f, { 14.f, -8.f }),
            CCMoveBy::create(0.04f, { -22.f, 14.f }),
            CCMoveBy::create(0.04f, { 16.f, -10.f }),
            CCMoveBy::create(0.04f, { -10.f, 6.f }),
            CCMoveBy::create(0.04f, { 6.f, -2.f }),
            nullptr
        ));

        this->runAction(CCSequence::create(
            CCDelayTime::create(std::max(0.f, IMPACT - IMPACT_SFX_LEAD)),
            CCCallFunc::create(this, callfunc_selector(RankDemotionAnimation::playImpactSfx)),
            nullptr
        ));

        content->runAction(CCSequence::create(
            CCDelayTime::create(IMPACT),
            CCCallFunc::create(this, callfunc_selector(RankDemotionAnimation::onImpact)),
            nullptr
        ));

        this->runAction(CCSequence::create(
            CCDelayTime::create(IMPACT + 0.5f),
            CCCallFunc::create(this, callfunc_selector(RankDemotionAnimation::startBarFill)),
            nullptr
        ));

        m_okBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("OK"), this,
            menu_selector(RankDemotionAnimation::onClose)
        );
        m_okBtn->setEnabled(false);
        m_okMenu = CCMenu::createWithItem(m_okBtn);
        m_okMenu->setPosition({ center.x, okY });
        m_okMenu->setVisible(false);
        this->addChild(m_okMenu, 5);
        m_okMenu->setTouchPriority(-514);
        m_okMenu->runAction(CCSequence::create(
            CCDelayTime::create(IMPACT + 0.5f + BAR_FILL_DUR + 0.6f),
            CCCallFunc::create(this, callfunc_selector(RankDemotionAnimation::showOk)),
            nullptr
        ));

        return true;
    }

    void showOk() {
        if (!m_okMenu) return;
        m_okMenu->setVisible(true);
        if (m_okBtn) {
            m_okBtn->setEnabled(true);
            m_okBtn->setCascadeOpacityEnabled(true);
            m_okBtn->setOpacity(0);
            m_okBtn->runAction(CCFadeIn::create(0.4f));
        }
    }

    void onEnter() override {
        CCLayer::onEnter();
        CCDirector::sharedDirector()->getTouchDispatcher()->addTargetedDelegate(this, -513, true);
    }

    void onExit() override {
        CCDirector::sharedDirector()->getTouchDispatcher()->removeDelegate(this);
        CCLayer::onExit();
    }

    bool ccTouchBegan(CCTouch*, CCEvent*) override { return true; }
    void ccTouchMoved(CCTouch*, CCEvent*) override {}
    void ccTouchEnded(CCTouch*, CCEvent*) override {}
    void ccTouchCancelled(CCTouch*, CCEvent*) override {}

    void playImpactSfx() {
        FMODAudioEngine::sharedEngine()->playEffect("sfx_rank_impact.mp3"_spr);
    }

    void startBarFill() {
        FMODAudioEngine::sharedEngine()->playEffect("sfx_rank_bar.mp3"_spr);
        m_barProgress = 0.f;
        this->schedule(schedule_selector(RankDemotionAnimation::tickBar));
    }

    void tickBar(float dt) {
        if (!m_bar) return;
        float step = (m_barTarget / BAR_FILL_DUR) * dt;
        m_barProgress += step;
        if (m_barProgress >= m_barTarget) {
            m_barProgress = m_barTarget;
            this->unschedule(schedule_selector(RankDemotionAnimation::tickBar));
        }
        m_bar->setProgress(m_barProgress);
    }

    void onImpact() {
        if (!m_content) return;

        if (auto burst = CCParticleExplosion::create()) {
            burst->setPosition(m_impactPos);
            burst->setAutoRemoveOnFinish(true);
            burst->setTotalParticles(160);
            burst->setDuration(0.18f);
            burst->setLife(0.7f);
            burst->setLifeVar(0.4f);
            burst->setSpeed(260.f);
            burst->setSpeedVar(120.f);
            burst->setStartSize(16.f);
            burst->setStartSizeVar(6.f);
            burst->setEndSize(0.f);
            burst->setStartColor({ 1.f, 0.85f, 0.35f, 1.f });
            burst->setStartColorVar({ 0.1f, 0.1f, 0.1f, 0.f });
            burst->setEndColor({ 1.f, 0.4f, 0.1f, 0.f });
            burst->setBlendAdditive(true);
            m_content->addChild(burst, 7);
        }

        if (auto sparks = CCParticleExplosion::create()) {
            sparks->setPosition(m_impactPos);
            sparks->setAutoRemoveOnFinish(true);
            sparks->setTotalParticles(80);
            sparks->setDuration(0.12f);
            sparks->setLife(1.0f);
            sparks->setLifeVar(0.5f);
            sparks->setSpeed(420.f);
            sparks->setSpeedVar(160.f);
            sparks->setStartSize(7.f);
            sparks->setStartSizeVar(3.f);
            sparks->setEndSize(0.f);
            sparks->setGravity({ 0.f, -380.f });
            sparks->setStartColor({ 1.f, 1.f, 1.f, 1.f });
            sparks->setEndColor({ 0.8f, 0.85f, 1.f, 0.f });
            sparks->setBlendAdditive(true);
            m_content->addChild(sparks, 8);
        }
    }

    void onClose(CCObject*) {
        if (m_dismissing) return;
        m_dismissing = true;
        this->removeFromParent();
    }

public:
    static RankDemotionAnimation* create(int oldIdx, int newIdx) {
        auto ret = new RankDemotionAnimation();
        if (ret && ret->initWithRanks(oldIdx, newIdx)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    static void show(int oldIdx, int newIdx) {
        auto scene = CCDirector::sharedDirector()->getRunningScene();
        if (!scene) return;
        if (auto anim = RankDemotionAnimation::create(oldIdx, newIdx)) {
            scene->addChild(anim, 99999);
        }
    }
};
