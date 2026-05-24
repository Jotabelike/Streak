#include "StreakChestPopup.h"
#include "../RewardNotification.h"
#include "../FirebaseManager.h"
#include <random>

static cocos2d::ccColor3B chestGlowColorForRarity(int rarity) {
    switch (rarity) {
    case 1: return { 200, 200, 200 };
    case 2: return { 120, 220, 120 };
    case 3: return { 80, 180, 255 };
    case 4: return { 200, 120, 255 };
    case 5: return { 255, 200, 60 };
    case 6: return { 255, 60, 100 };
    }
    return { 50, 200, 255 };
}

static std::string modSpritePath(const std::string& filename) {
    return fmt::format("{}/{}", geode::Mod::get()->getID(), filename);
}

std::string StreakChestPopup::getChestFrameName(int frameIdx) const {
    int set;
    if (m_rarity <= 2)      set = 1;
    else if (m_rarity <= 5) set = 2;
    else                    set = 3;
    return fmt::format("Chest{}_{}.png", set, frameIdx);
}

static void setChestTextureFromFile(CCSprite* spr, const std::string& filename) {
    std::string fullPath = modSpritePath(filename);
    auto tex = CCTextureCache::sharedTextureCache()->addImage(fullPath.c_str(), false);
    if (tex) {
        spr->setTexture(tex);
        spr->setTextureRect(CCRect(0, 0,
            tex->getContentSize().width,
            tex->getContentSize().height));
    }
}

bool StreakChestPopup::init(int superStars, int starTickets, int gems, int rewardXP,
                            int rarity, bool testMode, std::function<void()> reloadFunc) {
    if (!Popup::init(350.f, 220.f, "GJ_square02.png")) return false;

    if (m_closeBtn) m_closeBtn->setVisible(false);

    m_rarity = std::clamp(rarity, 1, 6);
    m_testMode = testMode;
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

     

    auto cornerBR = CCSprite::createWithSpriteFrameName("rewardCorner_001.png");
    if (cornerBR) {
        cornerBR->setAnchorPoint({ 1, 0 });
        cornerBR->setPosition({ m_size.width - offset, offset });
        cornerBR->setFlipX(true);
        m_mainLayer->addChild(cornerBR, 2);
    }

    auto title = CCLabelBMFont::create("Reward Unlocked!", "goldFont.fnt");
    title->setPosition({ m_size.width / 2, m_size.height - 12.f });
    title->setScale(0.55f);
    m_mainLayer->addChild(title, 1);

    std::string closedFrame = this->getChestFrameName(1);
    m_chestSpr = CCSprite::create(modSpritePath(closedFrame).c_str());
    if (!m_chestSpr) {
        m_chestSpr = CCSprite::createWithSpriteFrameName("chest_02_02_001.png");
    }
    m_chestSpr->setAnchorPoint({ 0.5f, 0.f });
    m_chestSpr->setPosition({ m_size.width / 2, m_size.height + 150.f });
    m_chestSpr->setScale(0.63f);
    m_mainLayer->addChild(m_chestSpr, 5);

    auto fallTo = CCMoveTo::create(0.85f, { m_size.width / 2, 55.f });
    auto bounceEffect = CCEaseBounceOut::create(fallTo);

    auto soundAction = CCSequence::create(
        CCDelayTime::create(0.35f),
        CallFuncExt::create([]() {
            FMODAudioEngine::sharedEngine()->playEffect("chestLand.ogg");
            }),
        nullptr
    );

    auto startSequence = CCSequence::create(
        CCSpawn::create(bounceEffect, soundAction, nullptr),
        CallFuncExt::create([this]() {
            this->addStarCategory(m_chestSpr->getPositionY());
            }),
        CCDelayTime::create(0.5f),
        CallFuncExt::create([this]() {
            this->playOpenAnimation();
            }),
        nullptr
    );

    m_chestSpr->runAction(startSequence);

    return true;
}

void StreakChestPopup::addStarCategory(float /*chestY*/) {
    int rarity = std::max(1, std::min(6, m_rarity));

    struct Slot { int row; int col; int rowCount; };
    std::vector<Slot> slots;
    switch (rarity) {
    case 1: slots = { {0,0,1} }; break;
    case 2: slots = { {0,0,2}, {0,1,2} }; break;
    case 3: slots = { {0,0,3}, {0,1,3}, {0,2,3} }; break;
    case 4: slots = { {0,0,2}, {0,1,2}, {1,0,2}, {1,1,2} }; break;
    case 5: slots = { {0,0,2}, {0,1,2}, {1,0,3}, {1,1,3}, {1,2,3} }; break;
    case 6: slots = { {0,0,3}, {0,1,3}, {0,2,3}, {1,0,3}, {1,1,3}, {1,2,3} }; break;
    }

    const float starScale = 0.156f;
    const float starGap = -15.f;
    const float rowGap = -15.f;
    const float marginX = 4.f;
    const float marginY = 14.f;

    std::string starPath = modSpritePath("star_category.png");
    auto sample = CCSprite::create(starPath.c_str());
    float starW = sample ? sample->getContentSize().width  * starScale : 60.f;
    float starH = sample ? sample->getContentSize().height * starScale : 60.f;

    int delayIdx = 0;
    for (auto& s : slots) {
        auto star = CCSprite::create(starPath.c_str());
        if (!star) continue;
        star->setScale(starScale);
        float rowW = s.rowCount * starW + (s.rowCount - 1) * starGap;
        float x = marginX + s.col * (starW + starGap) + starW / 2.f;
        float y = marginY + s.row * (starH + rowGap) + starH / 2.f;
        star->setPosition({ x, y });
        m_mainLayer->addChild(star, 6);

        star->setOpacity(0);
        star->setScale(starScale * 0.3f);
        star->runAction(CCSequence::create(
            CCDelayTime::create(0.05f * delayIdx),
            CCSpawn::create(
                CCFadeIn::create(0.18f),
                CCEaseElasticOut::create(CCScaleTo::create(0.35f, starScale), 0.5f),
                nullptr),
            nullptr));
        delayIdx++;
        (void)rowW;
    }
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
        setChestTextureFromFile(m_chestSpr, this->getChestFrameName(2));
        });

    auto delay2 = CCDelayTime::create(0.15f);

    auto frame3 = CallFuncExt::create([this]() {
        setChestTextureFromFile(m_chestSpr, this->getChestFrameName(4));

        const float lipW = m_chestSpr->getContentSize().width;
        const float lipH = m_chestSpr->getContentSize().height;

        auto topSpr = CCSprite::create(modSpritePath(this->getChestFrameName(3)).c_str());
        float cavityH = 0.f;
        if (topSpr) {
            topSpr->setAnchorPoint({ 0.5f, 0.0f });
            topSpr->setPosition({ lipW / 2, lipH - 8.f });
            topSpr->setZOrder(-3);
            m_chestSpr->addChild(topSpr);
            cavityH = topSpr->getContentSize().height;
        }

        FMODAudioEngine::sharedEngine()->playEffect("chestOpen01.ogg");

        ccColor3B glow = chestGlowColorForRarity(m_rarity);

        auto clippingNode = CCClippingNode::create();
        clippingNode->setAlphaThreshold(0.05f);

        auto stencil = CCSprite::createWithSpriteFrameName("block005b_05_001.png");
        if (!stencil) stencil = CCSprite::create("square02_small.png");

        stencil->setScale(10.0f);
        stencil->setAnchorPoint({ 0.5f, 0.0f });
        stencil->setPosition({ lipW / 2, lipH - 4.f });

        clippingNode->setStencil(stencil);
        clippingNode->setZOrder(-2);
        m_chestSpr->addChild(clippingNode);

        float glowY = lipH + (cavityH > 0.f ? cavityH * 0.10f : 12.f);

        auto glowSpr = CCSprite::createWithSpriteFrameName("chest_02_04_glow_001.png");
        if (glowSpr) {
            glowSpr->setPosition({ lipW / 2, glowY - 10.f });
            glowSpr->setScale(1.4f * 0.9f * 0.85f * 0.95f);
            glowSpr->setColor(glow);
            glowSpr->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });
            clippingNode->addChild(glowSpr, 1);
        }

        auto shine = CCSprite::createWithSpriteFrameName("shineBurst_001.png");
        if (shine) {
            shine->setPosition({ lipW / 2, glowY - 10.f });
            shine->setScale(6.0f);
            shine->setColor(glow);
            shine->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });
            shine->runAction(CCRepeatForever::create(CCRotateBy::create(4.0f, 360.f)));
            clippingNode->addChild(shine, 0);
        }

        auto shine2 = CCSprite::createWithSpriteFrameName("shineBurst_001.png");
        if (shine2) {
            shine2->setPosition({ lipW / 2, glowY - 10.f });
            shine2->setScale(4.4f);
            shine2->setColor(glow);
            shine2->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });
            shine2->runAction(CCRepeatForever::create(CCRotateBy::create(5.0f, -360.f)));
            clippingNode->addChild(shine2, 0);
        }

        auto burstParticles = new CCParticleSystemQuad();
        if (burstParticles && burstParticles->initWithFile("chestOpen.plist", false)) {
            burstParticles->autorelease();
            burstParticles->setPosition({ lipW / 2, lipH + 4.f });
            m_chestSpr->addChild(burstParticles, -1);
        }
        else { CC_SAFE_DELETE(burstParticles); }

        auto ambientParticles = new CCParticleSystemQuad();
        if (ambientParticles && ambientParticles->initWithFile("chestOpened.plist", false)) {
            ambientParticles->autorelease();
            ambientParticles->setPosition({ lipW / 2, lipH + 4.f });
            m_chestSpr->addChild(ambientParticles, -1);
        }
        else { CC_SAFE_DELETE(ambientParticles); }

        m_startStars = g_streakData.superStars;
        m_startTickets = g_streakData.starTickets;
        m_startGems = g_streakData.gems;
        m_startXP = g_streakData.currentXP;

        if (m_testMode) {
            this->runAction(CCSequence::create(
                CCDelayTime::create(0.4f),
                CallFuncExt::create([this]() { this->showRewards(); }),
                nullptr
            ));
        }
        else {
            matjson::Value chestPayload = matjson::Value::object();
            chestPayload.set("superStars", m_superStars);
            chestPayload.set("starTickets", m_starTickets);
            chestPayload.set("gems", m_gems);
            chestPayload.set("xp", m_rewardXP);
            chestPayload.set("context", std::string("chest"));
            chestPayload.set("rarity", m_rarity);

            claimOnServer("/chest/claim", chestPayload, [this](bool ok) {
                this->runAction(CCSequence::create(
                    CCDelayTime::create(0.4f),
                    CallFuncExt::create([this]() { this->showRewards(); }),
                    nullptr
                ));
                });
        }
        });

    m_chestSpr->runAction(CCSequence::create(shake, shake, delay1, frame2, delay2, frame3, nullptr));
}

void StreakChestPopup::showRewards() {
    FMODAudioEngine::sharedEngine()->playEffect("chest07.ogg");

    CCPoint spawnPos = m_chestSpr->convertToWorldSpaceAR(CCPointZero);

    if (m_superStars > 0) RewardNotification::show("super_star.png"_spr,
        m_startStars, m_superStars, spawnPos);
    if (m_starTickets > 0) RewardNotification::show("star_tiket.png"_spr,
        m_startTickets, m_starTickets, spawnPos);
    if (m_gems > 0) RewardNotification::show("gem.png"_spr,
        m_startGems, m_gems, spawnPos);
    if (m_rewardXP > 0) RewardNotification::show("xp.png"_spr,
        m_startXP, m_rewardXP, spawnPos);

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

    if (m_reloadFunc) m_reloadFunc();
}

void StreakChestPopup::rollRewardsForRarity(int rarity,
                                            int& outStars, int& outTickets, int& outGems, int& outXP) {
    outStars = outTickets = outGems = outXP = 0;
    rarity = std::clamp(rarity, 1, 6);

    static std::mt19937 gen(std::random_device{}());

    struct Range { int lo, hi; };
    Range ranges[6][4] = {
        {{1, 3},     {58, 173},     {1, 2},    {20, 60}},
        {{3, 8},     {110, 330},    {2, 5},    {50, 150}},
        {{6, 16},    {300, 720},    {4, 11},   {170, 425}},
        {{19, 43},   {1260, 3080},  {15, 29},  {490, 980}},
        {{47, 95},   {2860, 5850},  {31, 61},  {1120, 2240}},
        {{95, 180},  {5400, 9900},  {62, 115}, {2300, 4200}}
    };

    int idx = rarity - 1;
    auto& r = ranges[idx];

    int minPick = 1, maxPick = 1;
    switch (rarity) {
    case 1: minPick = 1; maxPick = 1; break;
    case 2: minPick = 1; maxPick = 2; break;
    case 3: minPick = 2; maxPick = 3; break;
    case 4: minPick = 3; maxPick = 3; break;
    case 5: minPick = 3; maxPick = 4; break;
    case 6: minPick = 4; maxPick = 4; break;
    }

    int picks = std::uniform_int_distribution<>(minPick, maxPick)(gen);
    std::vector<int> indices = { 0, 1, 2, 3 };
    std::shuffle(indices.begin(), indices.end(), gen);
    indices.resize(picks);

    auto rollIn = [&](int lo, int hi) {
        return std::uniform_int_distribution<>(lo, hi)(gen);
    };

    for (int i : indices) {
        int v = rollIn(r[i].lo, r[i].hi);
        switch (i) {
        case 0: outStars   = v; break;
        case 1: outTickets = v; break;
        case 2: outGems    = v; break;
        case 3: outXP      = v; break;
        }
    }

    if (rarity == 4 && std::uniform_int_distribution<>(0, 99)(gen) < 35) {
        outStars   = std::max(outStars,   rollIn(33, 57));
        outGems    = std::max(outGems,    rollIn(21, 37));
        outTickets = std::max(outTickets, rollIn(2100, 3920));
    }
    if (rarity == 5 && std::uniform_int_distribution<>(0, 99)(gen) < 55) {
        outStars   = std::max(outStars,   rollIn(74, 137));
        outGems    = std::max(outGems,    rollIn(44, 77));
        outTickets = std::max(outTickets, rollIn(3900, 6500));
        outXP      = std::max(outXP,      rollIn(1760, 2800));
    }
    if (rarity == 6 && std::uniform_int_distribution<>(0, 99)(gen) < 70) {
        outStars   = std::max(outStars,   rollIn(150, 260));
        outGems    = std::max(outGems,    rollIn(90, 160));
        outTickets = std::max(outTickets, rollIn(7800, 12500));
        outXP      = std::max(outXP,      rollIn(3500, 4900));
    }
}

StreakChestPopup* StreakChestPopup::create(int superStars, int starTickets, int gems, int rewardXP,
                                           std::function<void()> reloadFunc) {
    return StreakChestPopup::create(superStars, starTickets, gems, rewardXP, 1, reloadFunc);
}

StreakChestPopup* StreakChestPopup::create(int superStars, int starTickets, int gems, int rewardXP,
                                           int rarity, std::function<void()> reloadFunc) {
    auto ret = new StreakChestPopup();
    if (ret && ret->init(superStars, starTickets, gems, rewardXP, rarity, false, reloadFunc)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

StreakChestPopup* StreakChestPopup::createOpen(int superStars, int starTickets, int gems, int rewardXP,
                                               int rarity, std::function<void()> reloadFunc) {
    auto ret = new StreakChestPopup();
    if (ret && ret->init(superStars, starTickets, gems, rewardXP, rarity, true, reloadFunc)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

StreakChestPopup* StreakChestPopup::createTest(int rarity) {
    int stars = 0, tickets = 0, gems = 0, xp = 0;
    StreakChestPopup::rollRewardsForRarity(rarity, stars, tickets, gems, xp);

    auto ret = new StreakChestPopup();
    if (ret && ret->init(stars, tickets, gems, xp, rarity, true, nullptr)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

void StreakChestPopup::keyBackClicked() {
}
