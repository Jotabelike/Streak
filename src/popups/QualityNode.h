#pragma once
#include "StreakCommon.h"
#include <Geode/Geode.hpp>
#include <cmath>

using namespace geode::prelude;

class QualityNode : public CCNode {
protected:
    CCLabelBMFont* m_label = nullptr;
    CCParticleSystemQuad* m_particles = nullptr;
    int m_currentCategory = 0;
    float m_time = 0.0f;

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
 
        m_label = CCLabelBMFont::create("Common", "bigFont.fnt");
        m_label->setScale(0.55f);
        this->addChild(m_label, 2);

      
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
            m_particles->setPosition({ 0.f, 0.f });  

            this->addChild(m_particles, 1);
        }

        this->scheduleUpdate();
        return true;
    }

    void update(float dt) override {
     
        if (m_currentCategory < 2 || !m_label) return;

        m_time += dt * 2.0f;
        auto children = m_label->getChildren();
        if (!children) return;

        int count = children->count();
        for (int i = 0; i < count; i++) {
            auto letter = static_cast<CCSprite*>(children->objectAtIndex(i));
            if (!letter) continue;
            float phase = m_time + i * 0.3f;
            float wave = sinf(phase) * 0.5f + 0.5f;
            GLubyte r = 255, g = 255, b = 255;    
            if (m_currentCategory == 2) {
                r = 255 - wave * (255 - 170);  
                g = 255 - wave * 255;          
                b = 255;                     
            }
           
            else if (m_currentCategory == 3) {
                if (wave < 0.5f) {
                    float w = wave * 2.0f;
                    r = 255;
                    g = 255;
                    b = 255 - (w * 255);  
                }
                else {
                    float w = (wave - 0.5f) * 2.0f;
                    r = 255;
                    g = 255 - (w * 105);  
                    b = 0;
                }
            }
          
            else if (m_currentCategory == 4) {
                r = wave * 255;
                g = (sinf(phase + 2.094f) * 0.5f + 0.5f) * 255;
                b = (sinf(phase + 4.188f) * 0.5f + 0.5f) * 255;

            
                if (i == count / 2 && m_particles) {
                    m_particles->setStartColor({ r / 255.f, g / 255.f, b / 255.f, 1.0f });
                }
            }

            letter->setColor({ r, g, b });
        }
    }

    void setCategory(int category, bool isHidden) {
        m_currentCategory = category;

        if (isHidden) {
            m_label->setVisible(false);
            if (m_particles) m_particles->setVisible(false);
            return;
        }

        m_label->setVisible(true);

       
        std::string text = "Common";
        if (category == 1) text = "Special";
        else if (category == 2) text = "Epic";
        else if (category == 3) text = "Legendary";
        else if (category == 4) text = "Mythic";

        m_label->setString(text.c_str());

       
        m_label->setCascadeColorEnabled(false);
        auto children = m_label->getChildren();
 
        if (category == 0) { 
            if (children) {
                for (int i = 0; i < children->count(); i++) {
                    if (auto letter = static_cast<CCSprite*>(children->objectAtIndex(i))) {
                        letter->setColor({ 255, 255, 255 });
                    }
                }
            }
        }
        else if (category == 1) {  
            if (children) {
                int count = children->count();
                for (int i = 0; i < count; i++) {
                    if (auto letter = static_cast<CCSprite*>(children->objectAtIndex(i))) {
                        float factor = (count > 1) ? static_cast<float>(i) / (count - 1) : 0.0f;
                        GLubyte r = 255 - (factor * 255);  
                        GLubyte g = 255;                 
                        GLubyte b = 255 - (factor * 255);  
                        letter->setColor({ r, g, b });
                    }
                }
            }
        }
 
        if (!m_particles) return;

        if (category == 0) {
            m_particles->setVisible(false);
            m_particles->setEmissionRate(0.f);
        }
        else {
            m_particles->setVisible(true);

        
            auto labelSize = m_label->getContentSize();
            m_particles->setPosVar({ (labelSize.width * m_label->getScale()) / 2.f, 8.f });

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
             
                endColor = { 0.8f, 0.0f, 1.0f, 0.0f };
                emissionRate = 50.f;
                startSize = 16.f;
                life = 2.2f;
            }

            m_particles->setEmissionRate(emissionRate);
            m_particles->setStartSize(startSize);
            m_particles->setStartSizeVar(startSize * 0.3f);
            m_particles->setLife(life);
            m_particles->setLifeVar(life * 0.3f);
            if (category != 4) m_particles->setStartColor(startColor);
            m_particles->setStartColorVar({ 0.1f, 0.1f, 0.1f, 0.0f });
            m_particles->setEndColor(endColor);
            m_particles->setEndColorVar({ 0.0f, 0.0f, 0.0f, 0.0f });
            m_particles->resetSystem();
        }
    }
};