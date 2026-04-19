#pragma once
#include <cocos2d.h>
#include <Geode/Geode.hpp>
#include <Geode/modify/CurrencyRewardLayer.hpp>

using namespace cocos2d;
using namespace geode::prelude;

// Puente de datos para sincronizarnos con EndLevelLayer
namespace StreakDataBridge {
    inline bool shouldReward = false;
    inline int pointsGained = 0;
    inline int currentTotal = 0;
    inline cocos2d::CCPoint spawnPos = cocos2d::CCPointZero;
}

class $modify(ProCurrencyRewardLayer, CurrencyRewardLayer) {
    struct Fields {
        std::unordered_map<int, CCLabelBMFont*> m_labels;
        std::unordered_map<int, CCSprite*> m_sprites;
        std::unordered_map<int, std::vector<CurrencySprite*>> m_curSprites;
        std::unordered_map<int, int> m_spriteCounts;
        std::unordered_map<int, int> m_totalCounts;
        std::vector<int> m_types;
        bool m_isExiting = false;
    };

    bool init(int p0, int p1, int p2, int p3, CurrencySpriteType p4, int p5, CurrencySpriteType p6, int p7, cocos2d::CCPoint p8, CurrencyRewardType p9, float p10, float p11) {
        // Primero dejamos que la capa nativa se inicialice con sus elementos
        if (!CurrencyRewardLayer::init(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11)) return false;

        // Si EndLevelLayer activó la bandera, nos inyectamos en esta misma capa
        if (StreakDataBridge::shouldReward) {
            int typeID = std::hash<std::string>{}("streak_point.png");
            int startAmount = StreakDataBridge::currentTotal - StreakDataBridge::pointsGained;
            if (startAmount < 0) startAmount = 0;

            // Llamamos a nuestro addObjects nativo, que calculará la posición correcta
            this->addObjects(typeID, StreakDataBridge::pointsGained, startAmount, StreakDataBridge::spawnPos, "streak_point.png"_spr);
        }

        return true;
    }

    bool isDying() {
        return !m_objects || m_objects->count() == 0;
    }

    void update(float dt) {
        CurrencyRewardLayer::update(dt);
        if (!m_objects) return;

        auto f = m_fields.self();
        for (auto type : f->m_types) {
            auto count = 0;
            for (auto sprite : f->m_curSprites.at(type)) {
                if (m_objects->containsObject(sprite)) count++;
            }

            if (f->m_spriteCounts.contains(type) && f->m_spriteCounts.at(type) != count) {
                pulseSprite(f->m_sprites.at(type));
                f->m_labels.at(type)->setString(std::to_string(f->m_totalCounts.at(type) - count).c_str());
                FMODAudioEngine::sharedEngine()->playEffect("drop_01.ogg", 0.49f, 1.f, 0.3f);
            }
            f->m_spriteCounts[type] = count;
        }


        if (m_objects->count() == 0 && !f->m_isExiting) {
            f->m_isExiting = true;

            m_mainNode->runAction(CCSequence::create(
                CCDelayTime::create(0.5f),
                CCEaseIn::create(CCMoveBy::create(0.4f, { 0, 40.f }), 2.f),
                nullptr
            ));

            for (auto node : m_mainNode->getChildrenExt()) {
                node->runAction(CCSequence::create(
                    CCDelayTime::create(0.5f),
                    CCFadeOut::create(0.4f),
                    nullptr
                ));
            }
        }
    }

    void addCurrencySprite(int type, CCPoint position, const std::string & spriteName) {
        auto f = m_fields.self();

        auto sprite = CCSprite::create(spriteName.c_str());
        if (!sprite) sprite = CCSprite::createWithSpriteFrameName(spriteName.c_str());
        if (!sprite) return;

        auto curSprite = CurrencySprite::createWithSprite(sprite);

        float baseScale = 33.0f;
        curSprite->setScale(baseScale / std::max(sprite->getContentHeight(), sprite->getContentWidth()));

        curSprite->setPosition(position);
        curSprite->m_position = f->m_sprites.at(type)->convertToWorldSpaceAR({ 0, 0 }) + ccp(0, -10 - m_rewardCount * 30);
        curSprite->m_unkFloat1 = GameToolbox::fast_rand_0_1() * 5.f * (GameToolbox::fast_rand_0_1() <= 0.5f ? -1.f : 1.f);
        curSprite->m_unkFloat2 = GameToolbox::fast_rand_0_1() * 12.f;
        curSprite->m_remaining = GameToolbox::fast_rand_0_1() * 0.2f + m_time * 0.1f + 1.4f;

        if (curSprite->m_burstSprite) {

            curSprite->m_burstSprite->setPosition(position);

            curSprite->m_burstSprite->setScale(curSprite->m_burstSprite->getScale() * 0.60f);
            curSprite->m_burstSprite->setColor({ 255, 255, 120 });
            m_orbBatchNode->addChild(curSprite->m_burstSprite);

            if (!curSprite->m_followers) {
                curSprite->m_followers = CCArray::create();
                curSprite->m_followers->retain();
            }

            curSprite->m_followers->addObject(curSprite->m_burstSprite);
            curSprite->m_propagateScaleChanges = false;
            curSprite->m_hasFollower = true;
        }

        addChild(curSprite, 100);
        m_objects->addObject(curSprite);

        auto particle = CCParticleSystemQuad::create("fireballEffect.plist", false);
        if (particle) {
            particle->setTotalParticles(30);
            particle->setStartSize(4.5f);
            particle->setEndSize(0.5f);
            particle->setStartColor({ 1.f, 1.f, 0.47f, 1.f });
            particle->setEndColor({ 1.f, 1.f, 0.47f, 0.f });
            particle->setLife(0.3f);
            particle->setAutoRemoveOnFinish(false);

            particle->setPosition(position);

            addChild(particle, -1);
            curSprite->m_particleSystem = particle;
        }

        f->m_curSprites[type].push_back(curSprite);
    }

    void addObjects(int type, int count, int prevCount, CCPoint position, const std::string & spriteName) {
        auto f = m_fields.self();
        if (f->m_curSprites.contains(type)) return;

        f->m_types.push_back(type);
        f->m_totalCounts[type] = count + prevCount;

        auto circleWave = CCCircleWave::create(10.f, 110.f, 0.3f, false, true);
        circleWave->setPosition(position);
        circleWave->m_color.r = 255;
        circleWave->m_color.g = 255;
        circleWave->m_color.b = 120;
        addChild(circleWave, 9);

        auto label = CCLabelBMFont::create(std::to_string(prevCount).c_str(), "bigFont.fnt");
        label->setAnchorPoint({ 1, 0.5f });
        label->setScale(0.6f);

        float lowest = 0;
        for (auto node : m_mainNode->getChildrenExt()) {
            if (node->getPositionY() < lowest) lowest = node->getPositionY();
        }
        label->setPositionY(lowest - 35);
        m_mainNode->addChild(label);
        f->m_labels[type] = label;

        auto sprite = CCSprite::create(spriteName.c_str());
        if (!sprite) sprite = CCSprite::createWithSpriteFrameName(spriteName.c_str());
        if (sprite) {
            sprite->setScale(22.0f / std::max(sprite->getContentHeight(), sprite->getContentWidth()));
            sprite->setPosition({ 18, label->getPositionY() });
            m_mainNode->addChild(sprite);
            f->m_sprites[type] = sprite;
        }

        FMODAudioEngine::sharedEngine()->playEffect("magicExplosion.ogg", 1.35f, 1.f, 0.1f);

        int visualCount = std::min(count, 30);
        for (int i = 0; i < visualCount; i++) {
            addCurrencySprite(type, position, spriteName);
        }
    }

    void addMoreObjects(int type, int count, CCPoint position, const std::string & spriteName) {
        auto f = m_fields.self();
        if (!f->m_curSprites.contains(type)) return;

        f->m_totalCounts[type] += count;

        auto circleWave = CCCircleWave::create(10.f, 110.f, 0.3f, false, true);
        circleWave->setPosition(position);
        circleWave->m_color.r = 255;
        circleWave->m_color.g = 255;
        circleWave->m_color.b = 120;
        addChild(circleWave, 9);

        FMODAudioEngine::sharedEngine()->playEffect("magicExplosion.ogg", 1.35f, 1.f, 0.1f);

        int visualCount = std::min(count, 30);
        for (int i = 0; i < visualCount; i++) {
            addCurrencySprite(type, position, spriteName);
        }
    }
};

class RewardNotification {
public:
    static void show(const std::string& spriteName,
        int startAmount,
        int addedAmount,
        CCPoint spawnPosition = CCPointZero) {
        auto overlay = OverlayManager::get();
        if (!overlay) return;

        if (spawnPosition == CCPointZero) {
            spawnPosition = CCDirector::sharedDirector()->getWinSize() / 2;
        }

        int const LAYER_TAG = 849302;
        auto activeLayer = overlay->getChildByTag(LAYER_TAG);

        if (activeLayer && static_cast<ProCurrencyRewardLayer*>(activeLayer)->isDying()) {
            activeLayer->setTag(0);
            activeLayer = nullptr;
        }

        if (!activeLayer) {
            auto newLayer = CurrencyRewardLayer::create(0, 0, 0, 0, CurrencySpriteType::Icon, 0,
                CurrencySpriteType::Icon, 0,
                spawnPosition,
                CurrencyRewardType::Default, 0.f, 0.9f);
            if (auto bg = typeinfo_cast<CCLayerColor*>(newLayer->getChildren()->objectAtIndex(0))) bg->setOpacity(0);
            newLayer->setTag(LAYER_TAG);
            overlay->addChild(newLayer, 105000);
            activeLayer = newLayer;
        }
        else {
            overlay->reorderChild(activeLayer, 105000);
        }

        auto modLayer = static_cast<ProCurrencyRewardLayer*>(activeLayer);
        int typeID = std::hash<std::string>{}(spriteName);

        if (modLayer->m_fields->m_curSprites.contains(typeID)) {
            modLayer->addMoreObjects(typeID, addedAmount, spawnPosition, spriteName);
        }
        else {
            modLayer->addObjects(typeID, addedAmount, startAmount, spawnPosition, spriteName);
        }
    }
};