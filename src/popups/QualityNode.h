#pragma once
#include "StreakCommon.h"
#include <Geode/Geode.hpp>

using namespace geode::prelude;

class QualityNode : public CCNode {
protected:
    CCSprite* m_sprite = nullptr;
    CCParticleSystemQuad* m_particles = nullptr;
    int m_currentCategory = 0;

 
    float m_colorTransitionTime = 0.0f;
    int m_colorIndex = 0;
    std::vector<ccColor3B> m_mythicColors;
    ccColor3B m_currentColor;
    ccColor3B m_targetColor;

public:
    static QualityNode* create() {
        auto ret = new QualityNode();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    bool init() {
        if (!CCNode::init()) return false;

     
        m_mythicColors = {
            ccc3(255, 200, 200),
            ccc3(255, 225, 190),
            ccc3(255, 255, 200),
            ccc3(200, 255, 200),
            ccc3(200, 240, 255),
            ccc3(220, 200, 255),
            ccc3(255, 200, 255)
        };
        m_currentColor = m_mythicColors[0];
        m_targetColor = m_mythicColors[1];
 
        m_sprite = CCSprite::create("common.png"_spr);
        if (!m_sprite) m_sprite = CCSprite::createWithSpriteFrameName("GJ_button_01.png");
        m_sprite->setScale(0.8f);
        this->addChild(m_sprite, 2);

 
        m_particles = static_cast<CCParticleSystemQuad*>(CCParticleFire::create());
        if (m_particles) {
         
            m_particles->setTotalParticles(80);
            m_particles->setDuration(-1);
            m_particles->setEmissionRate(0.f);

            m_particles->setLife(2.0f);
            m_particles->setLifeVar(0.5f);

            m_particles->setStartSize(14.0f);
            m_particles->setStartSizeVar(3.0f);
            m_particles->setEndSize(2.0f);
            m_particles->setEndSizeVar(1.0f);

            m_particles->setSpeed(4.f);
            m_particles->setSpeedVar(2.f);
            m_particles->setGravity({ 0.f, 2.f });

            m_particles->setAngle(90.f);
            m_particles->setAngleVar(360.f);
            m_particles->setStartSpin(0.f);
            m_particles->setStartSpinVar(180.f);
            m_particles->setEndSpin(0.f);
            m_particles->setEndSpinVar(180.f);

            m_particles->setBlendAdditive(true);

            auto spriteSize = m_sprite->getContentSize();
            m_particles->setPosition({ spriteSize.width / 2.f, spriteSize.height / 2.f });
            m_particles->setPosVar({ 40.f, 15.f });
 
            m_sprite->addChild(m_particles, 1);
        }

        this->scheduleUpdate();
        return true;
    }

    void update(float dt) override {
        if (m_currentCategory == 4 && m_particles) {
            m_colorTransitionTime += dt;
            if (m_colorTransitionTime >= 1.0f) {
                m_colorTransitionTime = 0.0f;
                m_colorIndex = (m_colorIndex + 1) % m_mythicColors.size();
                m_currentColor = m_targetColor;
                m_targetColor = m_mythicColors[(m_colorIndex + 1) % m_mythicColors.size()];
            }
            float progress = m_colorTransitionTime;

            ccColor3B topColor = {
                static_cast<GLubyte>(m_currentColor.r + (m_targetColor.r - m_currentColor.r) * progress),
                static_cast<GLubyte>(m_currentColor.g + (m_targetColor.g - m_currentColor.g) * progress),
                static_cast<GLubyte>(m_currentColor.b + (m_targetColor.b - m_currentColor.b) * progress)
            };

            m_sprite->setColor(topColor);
            m_particles->setStartColor({ topColor.r / 255.f, topColor.g / 255.f, topColor.b / 255.f, 1.0f });
        }
    }

    void setCategory(int category, bool isHidden) {
        m_currentCategory = category;

        if (isHidden) {
            m_sprite->setVisible(false);
            if (m_particles) m_particles->setVisible(false);
            return;
        }

        m_sprite->setVisible(true);
        std::string sprName = "common.png"_spr;
        if (category == 0) sprName = "common.png"_spr;
        else if (category == 1) sprName = "special.png"_spr;
        else if (category == 2) sprName = "epic.png"_spr;
        else if (category == 3) sprName = "legendary.png"_spr;
        else if (category == 4) sprName = "mythic.png"_spr;

        if (auto newSprite = CCSprite::create(sprName.c_str())) {
            if (auto tex = newSprite->getTexture()) {
                m_sprite->setTexture(tex);
                m_sprite->setTextureRect(newSprite->getTextureRect());
            }
        }

        if (category != 4) {
            m_sprite->setColor({ 255, 255, 255 });
        }

        if (!m_particles) return;

        if (category == 0) {
            m_particles->setVisible(false);
            m_particles->setEmissionRate(0.f);
        }
        else {
            m_particles->setVisible(true);

            ccColor4F startColor = { 1.f, 1.f, 1.f, 1.f };
            ccColor4F endColor = { 1.f, 1.f, 1.f, 0.f };

           
            float emissionRate = 35.f;
            float startSize = 14.f;
            float life = 2.0f;

            if (category == 1) {  
                startColor = { 0.0f, 1.0f, 0.47f, 1.0f };
                endColor = { 0.0f, 0.39f, 1.0f, 0.0f };
                emissionRate = 15.f;  
                startSize = 10.f;
            }
            else if (category == 2) {  
                startColor = { 0.7f, 0.0f, 1.0f, 1.0f };
                endColor = { 0.0f, 0.39f, 1.0f, 0.0f };
                emissionRate = 25.f;  
                startSize = 12.f;
            }
            else if (category == 3) {  
                startColor = { 1.0f, 0.8f, 0.2f, 1.0f };
                endColor = { 1.0f, 0.2f, 0.0f, 0.0f };
                emissionRate = 35.f;  
                startSize = 14.f;
            }
            else if (category == 4) {  
                startColor = { 1.0f, 0.2f, 0.5f, 1.0f };
                endColor = { 0.8f, 0.0f, 1.0f, 0.0f };
                emissionRate = 50.f;  
                startSize = 16.f;
                life = 2.2f;

                m_colorTransitionTime = 0.0f;
                m_colorIndex = 0;
                m_currentColor = m_mythicColors[0];
                m_targetColor = m_mythicColors[1];
            }

            m_particles->setEmissionRate(emissionRate);
            m_particles->setStartSize(startSize);
            m_particles->setStartSizeVar(startSize * 0.3f);
            m_particles->setLife(life);
            m_particles->setLifeVar(life * 0.3f);
            m_particles->setStartColor(startColor);
            m_particles->setStartColorVar({ 0.1f, 0.1f, 0.1f, 0.0f });
            m_particles->setEndColor(endColor);
            m_particles->setEndColorVar({ 0.0f, 0.0f, 0.0f, 0.0f });
            m_particles->resetSystem();
        }
    }
};