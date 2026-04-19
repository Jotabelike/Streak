#include "NameModifiers.h"

namespace NameModifiers {
    void applyEffect(CCLabelBMFont* label, const std::string& effectID) {
        if (!label) return;

        auto parent = label->getParent();
        if (!parent) return;

        bool isProfile = (parent->getID() == "username-menu");
        CCNode* targetLayer = parent;
        if (isProfile && parent->getParent()) {
            targetLayer = parent->getParent();
        }

        if (auto old = targetLayer->getChildByTag(8888)) {
            old->removeFromParentAndCleanup(true);
        }
        if (targetLayer != parent) {
            if (auto old = parent->getChildByTag(8888)) {
                old->removeFromParentAndCleanup(true);
            }
        }
        if (auto old = parent->getChildByTag(8889)) {
            old->removeFromParentAndCleanup(true);
        }

        CCSize labelSize = label->getContentSize();
        float scale = label->getScale();

        float halfW = (labelSize.width * scale) / 2.0f;
        float halfH = (labelSize.height * scale) / 2.0f;

        CCPoint centerInLabel = { labelSize.width / 2.0f, labelSize.height / 2.0f };
        CCPoint worldPos = label->convertToWorldSpace(centerInLabel);
        CCPoint centerPos = targetLayer->convertToNodeSpace(worldPos);

        if (effectID == "None") return;

        CCParticleSystemQuad* particles = nullptr;

        if (effectID == "Sparkle") {
            particles = CCParticleSnow::create();
            particles->setTotalParticles(60);
            particles->setLife(0.7f);
            particles->setEmissionRate(60.f / 0.7f);
            particles->setStartColor({ 0.1f, 0.8f, 1.0f, 1.0f });
            particles->setEndColor({ 0.0f, 0.2f, 0.8f, 0.0f });
            particles->setStartSize(14.f * scale);
            particles->setEndSize(4.f * scale);
            particles->setGravity({ 0.f, 8.f * scale });
            particles->setPosVar({ halfW * 0.85f, halfH * 0.7f });
            particles->setBlendAdditive(true);
            particles->setAngle(90.f);
            particles->setAngleVar(360.f);
            particles->setRadialAccel(-50.f * scale);
            particles->setTangentialAccel(50.f * scale);
        }
        else if (effectID == "Fire") {
            particles = CCParticleFire::create();
            particles->setTotalParticles(90);
            particles->setLife(0.5f);
            particles->setEmissionRate(90.f / 0.5f);
            particles->setStartSize(22.f * scale);
            particles->setEndSize(6.f * scale);
            particles->setPosVar({ halfW * 0.8f, 2.f });
            centerPos.y -= halfH * 0.5f;
        }
        else if (effectID == "Snow") {
            particles = CCParticleSnow::create();
            particles->setTotalParticles(70);
            particles->setLife(1.0f);
            particles->setEmissionRate(70.f / 1.0f);
            particles->setGravity({ 0.f, -8.f * scale });
            particles->setStartSize(12.f * scale);
            particles->setEndSize(4.f * scale);
            particles->setPosVar({ halfW * 0.85f, 0.f });
            centerPos.y += halfH * 1.2f;
        }
        else if (effectID == "Poison") {
            particles = CCParticleFire::create();
            particles->setTotalParticles(70);
            particles->setLife(0.8f);
            particles->setEmissionRate(70.f / 0.8f);
            particles->setStartColor({ 0.2f, 0.9f, 0.2f, 0.8f });
            particles->setEndColor({ 0.0f, 0.4f, 0.0f, 0.0f });
            particles->setStartSize(16.f * scale);
            particles->setEndSize(5.f * scale);
            particles->setGravity({ 0.f, 12.f * scale });
            particles->setPosVar({ halfW * 0.8f, halfH * 0.5f });
            particles->setBlendAdditive(false);
            particles->setAngle(90.f);
            particles->setAngleVar(15.f);
        }
        else if (effectID == "Stars") {
            particles = CCParticleFire::create();
            particles->setTotalParticles(130);
            particles->setLife(0.5f);
            particles->setEmissionRate(130.f / 0.5f);
            particles->setStartColor({ 1.0f, 0.8f, 0.2f, 1.0f });
            particles->setEndColor({ 1.0f, 0.2f, 0.0f, 0.0f });
            particles->setStartSize(18.f * scale);
            particles->setEndSize(5.f * scale);
            particles->setSpeed(28.f * scale);
            particles->setPosVar({ halfW * 0.85f, halfH * 0.75f });
            particles->setBlendAdditive(true);
            particles->setAngle(90.f);
            particles->setAngleVar(360.f);
        }
        else if (effectID == "Rain") {
            particles = CCParticleRain::create();
            particles->setTotalParticles(70);
            particles->setLife(0.5f);
            particles->setEmissionRate(70.f / 0.5f);
            particles->setStartSize(10.f * scale);
            particles->setEndSize(4.f * scale);
            particles->setSpeed(40.f * scale);
            particles->setPosVar({ halfW * 0.85f, 0.f });
            centerPos.y += halfH * 1.4f;
        }
        else if (effectID == "Blood") {
            particles = CCParticleRain::create();
            particles->setTotalParticles(45);
            particles->setLife(0.6f);
            particles->setEmissionRate(45.f / 0.6f);
            particles->setStartColor({ 0.8f, 0.0f, 0.0f, 1.0f });
            particles->setEndColor({ 0.4f, 0.0f, 0.0f, 0.5f });
            particles->setStartSize(14.f * scale);
            particles->setEndSize(5.f * scale);
            particles->setGravity({ 0.f, -25.f * scale });
            particles->setPosVar({ halfW * 0.8f, 0.f });
            centerPos.y += halfH;
        }
        else if (effectID == "Void") {
            particles = CCParticleSnow::create();
            particles->setTotalParticles(160);
            particles->setLife(0.7f);
            particles->setEmissionRate(160.f / 0.7f);
            particles->setStartColor({ 0.4f, 0.0f, 0.8f, 1.0f });
            particles->setEndColor({ 0.0f, 0.0f, 0.0f, 1.0f });
            particles->setStartSize(11.f * scale);
            particles->setEndSize(2.f * scale);
            particles->setPosVar({ halfW * 0.9f, halfH * 0.9f });
            particles->setSpeed(0.0f);
            particles->setSpeedVar(0.f);
            particles->setGravity({ 0.f, 0.f });
            particles->setRadialAccel(-120.f * scale);
            particles->setTangentialAccel(60.f * scale);
            particles->setBlendAdditive(true);
            particles->setAngle(90.f);
            particles->setAngleVar(360.f);
        }
        else if (effectID == "Toxic") {
            particles = CCParticleFire::create();
            particles->setTotalParticles(90);
            particles->setLife(0.9f);
            particles->setEmissionRate(90.f / 0.9f);
            particles->setStartColor({ 0.4f, 1.0f, 0.0f, 1.0f });
            particles->setEndColor({ 0.0f, 0.0f, 0.0f, 0.9f });
            particles->setStartSize(17.f * scale);
            particles->setEndSize(5.f * scale);
            particles->setAngle(-90.0f);
            particles->setAngleVar(20.0f);
            particles->setSpeed(10.f * scale);
            particles->setGravity({ 0.f, -12.f * scale });
            particles->setPosVar({ halfW * 0.8f, halfH * 0.5f });
            particles->setBlendAdditive(false);
        }
        else if (effectID == "Holy") {
            particles = CCParticleFire::create();
            particles->setTotalParticles(120);
            particles->setLife(1.2f);
            particles->setEmissionRate(120.f / 1.2f);
            particles->setStartColor({ 1.0f, 1.0f, 1.0f, 1.0f });
            particles->setEndColor({ 1.0f, 1.0f, 1.0f, 0.0f });
            particles->setStartSize(18.f * scale);
            particles->setEndSize(4.f * scale);
            particles->setSpeed(8.f * scale);
            particles->setPosVar({ halfW * 0.8f, halfH * 0.75f });
            particles->setBlendAdditive(true);
            particles->setAngle(90.f);
            particles->setAngleVar(360.f);
       
        }
        else if (effectID == "Confetti") {
            particles = CCParticleExplosion::create();
            particles->setDuration(-1);
            particles->setTotalParticles(110);
            particles->setLife(1.1f);
            particles->setEmissionRate(110.f / 1.1f);
            particles->setStartColor({ 1.0f, 0.35f, 0.75f, 1.0f });
            particles->setStartColorVar({ 0.5f, 0.5f, 0.3f, 0.0f });
            particles->setEndColor({ 0.9f, 0.9f, 0.1f, 0.0f });
            particles->setStartSize(15.f * scale);
            particles->setEndSize(6.f * scale);
            particles->setSpeed(22.f * scale);
            particles->setSpeedVar(8.f * scale);
            particles->setGravity({ 0.f, -28.f * scale });
            particles->setPosVar({ halfW * 0.85f, halfH * 0.6f });
            particles->setAngle(90.f);
            particles->setAngleVar(35.f);
            particles->setBlendAdditive(false);
        }
        else if (effectID == "Electric") {
            particles = CCParticleFire::create();
            particles->setTotalParticles(100);
            particles->setLife(0.6f);
            particles->setEmissionRate(100.f / 0.6f);
            particles->setStartColor({ 0.4f, 0.9f, 1.0f, 1.0f });
            particles->setStartColorVar({ 0.1f, 0.1f, 0.0f, 0.0f });
            particles->setEndColor({ 1.0f, 1.0f, 1.0f, 0.0f });
            particles->setStartSize(9.f * scale);
            particles->setEndSize(2.f * scale);
            particles->setSpeed(50.f * scale);
            particles->setSpeedVar(18.f * scale);
            particles->setGravity({ 0.f, -120.f * scale });
            particles->setPosVar({ halfW * 0.8f, halfH * 0.5f });
            particles->setAngle(90.f);
            particles->setAngleVar(40.f);
            particles->setBlendAdditive(true);
        }
        else if (effectID == "Ice") {
            particles = CCParticleSnow::create();
            particles->setTotalParticles(80);
            particles->setLife(1.0f);
            particles->setEmissionRate(80.f / 1.0f);
            particles->setStartColor({ 0.8f, 0.97f, 1.0f, 1.0f });
            particles->setEndColor({ 0.3f, 0.75f, 1.0f, 0.0f });
            particles->setStartSize(13.f * scale);
            particles->setEndSize(4.f * scale);
            particles->setSpeed(30.f * scale);
            particles->setSpeedVar(5.f * scale);
            particles->setRadialAccel(-60.f * scale);
            particles->setTangentialAccel(70.f * scale);
            particles->setPosVar({ halfW * 0.7f, halfH * 0.7f });
            particles->setAngle(90.f);
            particles->setAngleVar(360.f);
            particles->setBlendAdditive(true);
        }
        else if (effectID == "Galaxy") {
            particles = CCParticleFire::create();
            particles->setTotalParticles(140);
            particles->setLife(0.9f);
            particles->setEmissionRate(140.f / 0.9f);
            particles->setStartColor({ 0.55f, 0.0f, 1.0f, 1.0f });
            particles->setStartColorVar({ 0.3f, 0.15f, 0.0f, 0.0f });
            particles->setEndColor({ 0.1f, 0.0f, 0.35f, 0.0f });
            particles->setStartSize(12.f * scale);
            particles->setEndSize(3.f * scale);
            particles->setSpeed(12.f * scale);
            particles->setSpeedVar(4.f * scale);
            particles->setPosVar({ halfW * 0.8f, halfH * 0.75f });
            particles->setAngle(90.f);
            particles->setAngleVar(360.f);
            particles->setRadialAccel(-35.f * scale);
            particles->setTangentialAccel(55.f * scale);
            particles->setBlendAdditive(true);
        }
        else if (effectID == "Lava") {
            particles = CCParticleFire::create();
            particles->setTotalParticles(130);
            particles->setLife(0.7f);
            particles->setEmissionRate(130.f / 0.7f);
            particles->setStartColor({ 1.0f, 0.45f, 0.0f, 1.0f });
            particles->setStartColorVar({ 0.0f, 0.2f, 0.0f, 0.0f });
            particles->setEndColor({ 0.6f, 0.0f, 0.0f, 0.0f });
            particles->setStartSize(16.f * scale);
            particles->setEndSize(3.f * scale);
            particles->setSpeed(30.f * scale);
            particles->setSpeedVar(10.f * scale);
            particles->setPosVar({ halfW * 0.3f, halfH * 0.3f });
            particles->setAngle(90.f);
            particles->setAngleVar(360.f);
            particles->setBlendAdditive(true);
        }

        if (particles) {
            particles->setDuration(-1);
            particles->setPosition(centerPos);
            particles->setPositionType(kCCPositionTypeGrouped);
            particles->setTag(8888);
            targetLayer->addChild(particles, parent->getZOrder() - 1);
        }
    }
}