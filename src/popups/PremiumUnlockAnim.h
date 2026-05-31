#pragma once
#include <Geode/Geode.hpp>

using namespace geode::prelude;

class PremiumUnlockAnim : public CCLayer {
protected:
    bool init() override {
        if (!CCLayer::init()) return false;

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        this->setContentSize(winSize);
        this->setAnchorPoint({ 0.f, 0.f });
        this->setPosition({ 0.f, 0.f });

        auto dimmer = CCLayerColor::create({ 0, 0, 0, 0 }, winSize.width, winSize.height);
        dimmer->setPosition({ 0.f, 0.f });
        this->addChild(dimmer, 0);
        dimmer->runAction(CCFadeTo::create(0.35f, 200));

        auto flash = CCLayerColor::create({ 255, 255, 255, 255 }, winSize.width, winSize.height);
        flash->setPosition({ 0.f, 0.f });
        this->addChild(flash, 1);
        flash->runAction(CCFadeTo::create(0.25f, 0));

        auto center = CCNode::create();
        center->setPosition({ winSize.width / 2.f, winSize.height / 2.f });
        this->addChild(center, 2);

        auto burstPlist = new CCParticleSystemQuad();
        if (burstPlist && burstPlist->initWithFile("chestOpen.plist", false)) {
            burstPlist->autorelease();
            burstPlist->setPosition({ 0.f, 0.f });
            burstPlist->setScale(1.6f);
            center->addChild(burstPlist, 0);
        } else {
            CC_SAFE_DELETE(burstPlist);
        }

        auto ambient = new CCParticleSystemQuad();
        if (ambient && ambient->initWithFile("chestOpened.plist", false)) {
            ambient->autorelease();
            ambient->setPosition({ 0.f, 20.f });
            ambient->setScale(1.2f);
            center->addChild(ambient, 0);
        } else {
            CC_SAFE_DELETE(ambient);
        }

        auto vipSpr = CCSprite::create("vip_pass.png"_spr);
        if (!vipSpr) vipSpr = CCSprite::createWithSpriteFrameName("GJ_button_01.png");
        if (vipSpr) {
            vipSpr->setPosition({ 0.f, 30.f });
            vipSpr->setScale(0.f);
            vipSpr->setRotation(-15.f);
            center->addChild(vipSpr, 2);

            vipSpr->runAction(CCSequence::create(
                CCDelayTime::create(0.15f),
                CCSpawn::create(
                    CCEaseElasticOut::create(CCScaleTo::create(0.9f, 1.4f), 0.6f),
                    CCEaseSineOut::create(CCRotateTo::create(0.7f, 0.f)),
                    nullptr
                ),
                CCDelayTime::create(1.2f),
                CCSpawn::create(
                    CCEaseSineIn::create(CCScaleTo::create(0.4f, 0.0f)),
                    CCFadeTo::create(0.4f, 0),
                    nullptr
                ),
                nullptr
            ));

            vipSpr->runAction(CCRepeatForever::create(CCSequence::create(
                CCEaseSineInOut::create(CCRotateBy::create(2.5f, 6.f)),
                CCEaseSineInOut::create(CCRotateBy::create(2.5f, -6.f)),
                nullptr
            )));
        }

        auto title = CCLabelBMFont::create("PREMIUM UNLOCKED!", "goldFont.fnt");
        title->setScale(0.0f);
        title->setPosition({ 0.f, -70.f });
        center->addChild(title, 3);

        title->runAction(CCSequence::create(
            CCDelayTime::create(0.45f),
            CCEaseBackOut::create(CCScaleTo::create(0.55f, 0.85f)),
            CCDelayTime::create(1.1f),
            CCSpawn::create(
                CCScaleTo::create(0.4f, 0.0f),
                CCFadeTo::create(0.4f, 0),
                nullptr
            ),
            nullptr
        ));

        auto subtitle = CCLabelBMFont::create("Enjoy the VIP track and premium rewards!", "bigFont.fnt");
        subtitle->setScale(0.0f);
        subtitle->setPosition({ 0.f, -100.f });
        subtitle->setColor({ 255, 230, 180 });
        center->addChild(subtitle, 3);

        subtitle->runAction(CCSequence::create(
            CCDelayTime::create(0.7f),
            CCEaseBackOut::create(CCScaleTo::create(0.5f, 0.42f)),
            CCDelayTime::create(0.9f),
            CCSpawn::create(
                CCScaleTo::create(0.4f, 0.0f),
                CCFadeTo::create(0.4f, 0),
                nullptr
            ),
            nullptr
        ));

        FMODAudioEngine::sharedEngine()->playEffect("chest07.ogg");

        this->runAction(CCSequence::create(
            CCDelayTime::create(2.4f),
            CallFuncExt::create([this, dimmer]() {
                dimmer->runAction(CCFadeTo::create(0.3f, 0));
            }),
            CCDelayTime::create(0.35f),
            CCRemoveSelf::create(),
            nullptr
        ));

        return true;
    }

public:
    static void show() {
        auto scene = CCDirector::sharedDirector()->getRunningScene();
        if (!scene) return;
        auto anim = new PremiumUnlockAnim();
        if (anim && anim->init()) {
            anim->autorelease();
            scene->addChild(anim, 99999);
        } else {
            CC_SAFE_DELETE(anim);
        }
    }
};
