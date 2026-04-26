#include "StreakChestPopup.h"
#include "../RewardNotification.h"  

bool StreakChestPopup::init(int superStars, int starTickets, int gems, int rewardXP, std::function<void()> reloadFunc) {
    if (!Popup::init(350.f, 220.f, "GJ_square02.png")) return false;

   
    if (m_closeBtn) {
        m_closeBtn->setVisible(false);
    }

    m_superStars = superStars;
    m_starTickets = starTickets;
    m_gems = gems;
    m_rewardXP = rewardXP;  
    m_reloadFunc = reloadFunc;

    float offset = 0.0f;
    auto cornerTL = CCSprite::createWithSpriteFrameName("rewardCorner_001.png");
    if (cornerTL) {
        cornerTL->setAnchorPoint({ 0, 1 });
        cornerTL->setPosition({ offset, m_size.height - offset });
        cornerTL->setFlipY(true);
        m_mainLayer->addChild(cornerTL, 2);
    }

    auto cornerTR = CCSprite::createWithSpriteFrameName("rewardCorner_001.png");
    if (cornerTR) {
        cornerTR->setAnchorPoint({ 1, 1 });
        cornerTR->setPosition({ m_size.width - offset, m_size.height - offset });
        cornerTR->setFlipX(true);
        cornerTR->setFlipY(true);
        m_mainLayer->addChild(cornerTR, 2);
    }

    auto cornerBL = CCSprite::createWithSpriteFrameName("rewardCorner_001.png");
    if (cornerBL) {
        cornerBL->setAnchorPoint({ 0, 0 });
        cornerBL->setPosition({ offset, offset });
        m_mainLayer->addChild(cornerBL, 2);
    }

    auto cornerBR = CCSprite::createWithSpriteFrameName("rewardCorner_001.png");
    if (cornerBR) {
        cornerBR->setAnchorPoint({ 1, 0 });
        cornerBR->setPosition({ m_size.width - offset, offset });
        cornerBR->setFlipX(true);
        m_mainLayer->addChild(cornerBR, 2);
    }

    auto title = CCLabelBMFont::create("Reward Unlocked!", "goldFont.fnt");
    title->setPosition({ m_size.width / 2, m_size.height - 20.f });
    title->setScale(0.8f);
    m_mainLayer->addChild(title, 10);

    m_chestSpr = CCSprite::createWithSpriteFrameName("chest_02_02_001.png");
    m_chestSpr->setPosition({ m_size.width / 2, m_size.height + 250.f });
    m_chestSpr->setScale(0.80f);
    m_mainLayer->addChild(m_chestSpr, 5);

    auto fallTo = CCMoveTo::create(0.85f, { m_size.width / 2, m_size.height / 2 - 25.f });
    auto bounceEffect = CCEaseBounceOut::create(fallTo);

    // Secuencia paralela para reproducir el sonido en el primer impacto
    auto soundAction = CCSequence::create(
        CCDelayTime::create(0.35f), // El primer impacto del cofre ocurre aprox a los 0.35s
        CallFuncExt::create([]() {
            FMODAudioEngine::sharedEngine()->playEffect("chestLand.ogg");
            }),
        nullptr
    );

    auto startSequence = CCSequence::create(
        // CCSpawn ejecuta el rebote y el temporizador del sonido de manera simultánea
        CCSpawn::create(bounceEffect, soundAction, nullptr),
        CCDelayTime::create(0.5f),
        CallFuncExt::create([this]() {
            this->playOpenAnimation();
            }),
        nullptr
    );

    m_chestSpr->runAction(startSequence);

    return true;
}

void StreakChestPopup::playOpenAnimation() {
    auto shake = CCSequence::create(
        CCMoveBy::create(0.04f, { -4.f, 0.f }),
        CCMoveBy::create(0.08f, { 8.f, 0.f }),
        CCMoveBy::create(0.04f, { -4.f, 0.f }),
        nullptr
    );

    auto delay1 = CCDelayTime::create(0.1f);

    auto frame2 = CallFuncExt::create([this]() {
        auto frame = CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName("chest_02_03_001.png");
        if (frame) m_chestSpr->setDisplayFrame(frame);
        });

    auto delay2 = CCDelayTime::create(0.15f);

    auto frame3 = CallFuncExt::create([this]() {
        auto frameBottom = CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName("chest_02_04_001.png");
        if (frameBottom) m_chestSpr->setDisplayFrame(frameBottom);

        auto topSpr = CCSprite::createWithSpriteFrameName("chest_02_04_back_001.png");
        if (topSpr) {
            topSpr->setPosition({ m_chestSpr->getContentSize().width / 2, m_chestSpr->getContentSize().height / 2 + 3.f });
            topSpr->setZOrder(-3);
            m_chestSpr->addChild(topSpr);
        }

        FMODAudioEngine::sharedEngine()->playEffect("chestOpen01.ogg");

        auto clippingNode = CCClippingNode::create();
        clippingNode->setAlphaThreshold(0.05f);

        auto stencil = CCSprite::createWithSpriteFrameName("block005b_05_001.png");
        if (!stencil) stencil = CCSprite::create("square02_small.png");

        stencil->setScale(10.0f);
        stencil->setAnchorPoint({ 0.5f, 0.0f });
        stencil->setPosition({ m_chestSpr->getContentSize().width / 2, m_chestSpr->getContentSize().height / 2 - 2.f });

        clippingNode->setStencil(stencil);
        clippingNode->setZOrder(-2);
        m_chestSpr->addChild(clippingNode);

        auto glowSpr = CCSprite::createWithSpriteFrameName("chest_02_04_glow_001.png");
        if (glowSpr) {
            glowSpr->setPosition({ m_chestSpr->getContentSize().width / 2, m_chestSpr->getContentSize().height / 2 - 3.f });
            glowSpr->setColor({ 50, 200, 255 });
            glowSpr->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });
            clippingNode->addChild(glowSpr, 1);
        }

        auto shine = CCSprite::createWithSpriteFrameName("shineBurst_001.png");
        if (shine) {
            shine->setPosition({ m_chestSpr->getContentSize().width / 2, m_chestSpr->getContentSize().height / 2 - 18.f });
            shine->setScale(5.0f);
            shine->setColor({ 50, 200, 255 });
            shine->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });

            auto rot = CCRotateBy::create(4.0f, 360.f);
            shine->runAction(CCRepeatForever::create(rot));

            clippingNode->addChild(shine, 0);
        }

        auto shine2 = CCSprite::createWithSpriteFrameName("shineBurst_001.png");
        if (shine2) {
            shine2->setPosition({ m_chestSpr->getContentSize().width / 2, m_chestSpr->getContentSize().height / 2 - 18.f });
            shine2->setScale(3.8f);
            shine2->setColor({ 50, 200, 255 });
            shine2->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });

            auto rot2 = CCRotateBy::create(5.0f, -360.f);
            shine2->runAction(CCRepeatForever::create(rot2));

            clippingNode->addChild(shine2, 0);
        }

        auto burstParticles = new CCParticleSystemQuad();
        if (burstParticles && burstParticles->initWithFile("chestOpen.plist", false)) {
            burstParticles->autorelease();
            burstParticles->setPosition({ m_chestSpr->getContentSize().width / 2, m_chestSpr->getContentSize().height / 2 - 2.f });
            m_chestSpr->addChild(burstParticles, -1);
        }
        else {
            CC_SAFE_DELETE(burstParticles);
        }

        auto ambientParticles = new CCParticleSystemQuad();
        if (ambientParticles && ambientParticles->initWithFile("chestOpened.plist", false)) {
            ambientParticles->autorelease();
            ambientParticles->setPosition({ m_chestSpr->getContentSize().width / 2, m_chestSpr->getContentSize().height / 2 - 2.f });
            m_chestSpr->addChild(ambientParticles, -1);
        }
        else {
            CC_SAFE_DELETE(ambientParticles);
        }

       
        if (m_superStars > 0) g_streakData.superStars += m_superStars;
        if (m_starTickets > 0) g_streakData.starTickets += m_starTickets;
        if (m_gems > 0) g_streakData.gems += m_gems;
        if (m_rewardXP > 0) g_streakData.addXP(m_rewardXP);
        g_streakData.save();

        this->runAction(CCSequence::create(
            CCDelayTime::create(0.4f),
            CallFuncExt::create([this]() {
                this->showRewards();
                }),
            nullptr
        ));
        });

    m_chestSpr->runAction(CCSequence::create(shake, shake, delay1, frame2, delay2, frame3, nullptr));
}

void StreakChestPopup::showRewards() {
    FMODAudioEngine::sharedEngine()->playEffect("chest07.ogg");

    CCPoint spawnPos = m_chestSpr->convertToWorldSpaceAR(CCPointZero);

   
    if (m_superStars > 0) RewardNotification::show("super_star.png"_spr,
        g_streakData.superStars - m_superStars,
        m_superStars, spawnPos
    );

    if (m_starTickets > 0) RewardNotification::show("star_tiket.png"_spr,
        g_streakData.starTickets - m_starTickets,
        m_starTickets, spawnPos
    );

    if (m_gems > 0) RewardNotification::show("gem.png"_spr,
        g_streakData.gems - m_gems, m_gems,
        spawnPos
    );

    if (m_rewardXP > 0) RewardNotification::show("xp.png"_spr,
        g_streakData.currentXP - m_rewardXP, m_rewardXP,
        spawnPos
    );

    std::vector<std::pair<std::string, int>> rewards;
    if (m_superStars > 0) rewards.push_back({ "super_star.png"_spr, m_superStars });
    if (m_starTickets > 0) rewards.push_back({ "star_tiket.png"_spr, m_starTickets });
    if (m_gems > 0) rewards.push_back({ "gem.png"_spr, m_gems });
    if (m_rewardXP > 0) rewards.push_back({ "xp.png"_spr, m_rewardXP });  

    float gap = 75.0f;
    float startX = -((rewards.size() - 1) * gap) / 2.0f;
    float targetY = m_size.height / 2 + 35.0f;

    for (size_t i = 0; i < rewards.size(); i++) {
        auto node = CCNode::create();

        auto icon = CCSprite::create(rewards[i].first.c_str());
        if (!icon) icon = CCSprite::createWithSpriteFrameName("GJ_starsIcon_001.png");
        icon->setScale(0.25f);
        icon->setAnchorPoint({ 0.5f, 0.5f });
        icon->setPosition({ 0.f, 10.f });
        node->addChild(icon);

        auto label = CCLabelBMFont::create(fmt::format("+{}", rewards[i].second).c_str(), "bigFont.fnt");
        label->setScale(0.28f);
        label->setAnchorPoint({ 0.5f, 0.5f });
        label->setPosition({ 0.f, -10.f });
        node->addChild(label);

        node->setPosition({ m_size.width / 2, m_size.height / 2 - 10.f });
        node->setScale(0.f);
        m_mainLayer->addChild(node, 15);

     
        float arcOffset = 0.0f;
        if (rewards.size() == 3 && i == 1) arcOffset = 15.0f;
        else if (rewards.size() == 4 && (i == 1 || i == 2)) arcOffset = 15.0f;

        CCPoint finalPos = { startX + (i * gap) + (m_size.width / 2), targetY + arcOffset };

        auto moveAction = CCEaseOut::create(CCMoveTo::create(0.4f, finalPos), 2.0f);
        auto scaleAction = CCEaseElasticOut::create(CCScaleTo::create(0.5f, 1.0f), 0.6f);

        node->runAction(CCSpawn::create(moveAction, scaleAction, nullptr));
    }

    auto menu = CCMenu::create();
    menu->setPosition({ 0, 0 });
    m_mainLayer->addChild(menu, 10);

    auto btnSpr = CCSprite::createWithSpriteFrameName("GJ_rewardBtn_001.png");
    if (!btnSpr) btnSpr = CCSprite::create("GJ_rewardBtn_001.png");

    btnSpr->setScale(0.8f);

    auto btn = CCMenuItemSpriteExtra::create(btnSpr, this, menu_selector(StreakChestPopup::onClose));
    btn->setPosition({ m_size.width / 2, 25.f });
    btn->setScale(0.f);

    btn->runAction(CCEaseElasticOut::create(CCScaleTo::create(0.5f, 1.0f), 0.6f));
    menu->addChild(btn);
}

StreakChestPopup* StreakChestPopup::create(int superStars,
    int starTickets,
    int gems,
    int rewardXP,
    std::function<void()> reloadFunc) {
    auto ret = new StreakChestPopup();
    if (ret && ret->init(superStars, starTickets, gems, rewardXP, reloadFunc)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

void StreakChestPopup::keyBackClicked() {
}