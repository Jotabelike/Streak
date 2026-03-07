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

        if (auto oldParticles = targetLayer->getChildByTag(8888)) {
            oldParticles->removeFromParentAndCleanup(true);
        }
        if (targetLayer != parent) {
            if (auto oldParticles = parent->getChildByTag(8888)) {
                oldParticles->removeFromParentAndCleanup(true);
            }
        }

        CCSize labelSize = label->getContentSize();
        float scale = label->getScale();

        CCPoint centerInLabel = { labelSize.width / 2.0f, labelSize.height / 2.0f };
        CCPoint worldPos = label->convertToWorldSpace(centerInLabel);
        CCPoint centerPos = targetLayer->convertToNodeSpace(worldPos);

        if (effectID != "None") {
            CCParticleSystemQuad* particles = nullptr;

            if (effectID == "Sparkle") {
                particles = CCParticleSnow::create();
                particles->setTotalParticles(40);
                particles->setLife(0.8f);
                particles->setEmissionRate(40.f / 0.8f);
                particles->setStartColor({ 0.1f, 0.8f, 1.0f, 1.0f });
                particles->setEndColor({ 0.0f, 0.2f, 0.8f, 0.0f });
                particles->setStartSize(15.f * scale);
                particles->setGravity({ 0.f, 10.f * scale });
                particles->setPosVar({ (labelSize.width * scale) / 1.5f, (labelSize.height * scale) / 2.0f });
                particles->setBlendAdditive(true);
                particles->setAngle(90.f);
                particles->setAngleVar(360.f);
                particles->setRadialAccel(-80.f * scale);
                particles->setTangentialAccel(80.f * scale);
            }
            else if (effectID == "Fire") {
                particles = CCParticleFire::create();
                particles->setTotalParticles(60);
                particles->setLife(0.6f);
                particles->setEmissionRate(60.f / 0.6f);
                particles->setStartSize(18.f * scale);
                particles->setPosVar({ (labelSize.width * scale) / 2.2f, 2.f });
                centerPos.y -= (labelSize.height * scale) / 4.0f;
            }
            else if (effectID == "Snow") {
                particles = CCParticleSnow::create();
                particles->setTotalParticles(50);
                particles->setLife(1.0f);
                particles->setEmissionRate(50.f / 1.0f);
                particles->setGravity({ 0.f, -8.f * scale });
                particles->setStartSize(8.f * scale);
                particles->setPosVar({ (labelSize.width * scale) / 1.5f, 0.f });
                centerPos.y += (labelSize.height * scale) / 1.5f;
            }
            else if (effectID == "Poison") {
                particles = CCParticleFire::create();
                particles->setTotalParticles(45);
                particles->setLife(0.8f);
                particles->setEmissionRate(45.f / 0.8f);
                particles->setStartColor({ 0.2f, 0.9f, 0.2f, 0.8f });
                particles->setEndColor({ 0.0f, 0.4f, 0.0f, 0.0f });
                particles->setStartSize(12.f * scale);
                particles->setGravity({ 0.f, 15.f * scale });
                particles->setPosVar({ (labelSize.width * scale) / 2.0f, (labelSize.height * scale) / 3.0f });
                particles->setBlendAdditive(false);
                particles->setAngle(90.f);
                particles->setAngleVar(10.f);
            }
            else if (effectID == "Stars") {
                particles = CCParticleFire::create();
                particles->setTotalParticles(120);
                particles->setLife(0.5f);
                particles->setEmissionRate(120.f / 0.5f);
                particles->setStartColor({ 1.0f, 0.8f, 0.2f, 1.0f });
                particles->setEndColor({ 1.0f, 0.2f, 0.0f, 0.0f });
                particles->setStartSize(18.f * scale);
                particles->setEndSize(4.f * scale);
                particles->setSpeed(35.f * scale);
                particles->setPosVar({ (labelSize.width * scale) / 1.5f, (labelSize.height * scale) / 2.0f });
                particles->setBlendAdditive(true);
                particles->setAngle(90.f);
                particles->setAngleVar(360.f);
            }
            else if (effectID == "Rain") {
                particles = CCParticleRain::create();
                particles->setTotalParticles(50);
                particles->setLife(0.5f);
                particles->setEmissionRate(50.f / 0.5f);
                particles->setStartSize(8.f * scale);
                particles->setSpeed(40.f * scale);
                particles->setPosVar({ (labelSize.width * scale) / 1.2f, 0.f });
                centerPos.y += (labelSize.height * scale) / 1.0f;
            }
            else if (effectID == "Blood") {
                particles = CCParticleRain::create();
                particles->setTotalParticles(30);
                particles->setLife(0.6f);
                particles->setEmissionRate(30.f / 0.6f);
                particles->setStartColor({ 0.8f, 0.0f, 0.0f, 1.0f });
                particles->setEndColor({ 0.4f, 0.0f, 0.0f, 0.5f });
                particles->setStartSize(10.f * scale);
                particles->setGravity({ 0.f, -25.f * scale });
                particles->setPosVar({ (labelSize.width * scale) / 1.2f, 0.f });
                centerPos.y += (labelSize.height * scale) / 2.0f;
            }
            else if (effectID == "Void") {
                particles = CCParticleSnow::create();
                particles->setTotalParticles(150);
                particles->setLife(0.8f);
                particles->setEmissionRate(150.f / 0.8f);
                particles->setStartColor({ 0.4f, 0.0f, 0.8f, 1.0f });
                particles->setEndColor({ 0.0f, 0.0f, 0.0f, 1.0f });
                particles->setStartSize(8.f * scale);
                particles->setEndSize(1.f * scale);
                particles->setPosVar({ (labelSize.width * scale) * 0.8f, (labelSize.height * scale) * 0.8f });
                particles->setSpeed(0.0f);
                particles->setGravity({ 0.f, 0.f });
                particles->setRadialAccel(-150.f * scale);
                particles->setTangentialAccel(80.f * scale);
                particles->setBlendAdditive(true);
                particles->setAngle(90.f);
                particles->setAngleVar(360.f);
                particles->setSpeedVar(0.f);
            }
            else if (effectID == "Toxic") {
                particles = CCParticleFire::create();
                particles->setTotalParticles(70);
                particles->setLife(0.9f);
                particles->setEmissionRate(70.f / 0.9f);
                particles->setStartColor({ 0.4f, 1.0f, 0.0f, 1.0f });
                particles->setEndColor({ 0.0f, 0.0f, 0.0f, 0.9f });
                particles->setStartSize(12.f * scale);
                particles->setEndSize(4.f * scale);
                particles->setAngle(-90.0f);
                particles->setAngleVar(25.0f);
                particles->setSpeed(10.f * scale);
                particles->setGravity({ 0.f, -15.f * scale });
                particles->setPosVar({ (labelSize.width * scale) / 1.4f, (labelSize.height * scale) / 3.0f });
                particles->setBlendAdditive(false);
            }
            else if (effectID == "Holy") {
                particles = CCParticleFire::create();
                particles->setTotalParticles(90);
                particles->setLife(1.5f);
                particles->setEmissionRate(90.f / 1.5f);
                particles->setStartColor({ 1.0f, 1.0f, 1.0f, 1.0f });
                particles->setEndColor({ 1.0f, 1.0f, 1.0f, 0.0f });
                particles->setStartSize(15.f * scale);
                particles->setEndSize(3.f * scale);
                particles->setSpeed(8.f * scale);
                particles->setPosVar({ (labelSize.width * scale) / 1.3f, (labelSize.height * scale) / 1.5f });
                particles->setBlendAdditive(true);
                particles->setAngle(90.f);
                particles->setAngleVar(360.f);
            }
            else if (effectID == "Multy3D") {
                CCNode* textContainer = CCNode::create();
                textContainer->setPosition(centerPos);

                ccColor3B layerColors[4] = {
                    { 255, 50, 50 },
                    { 50, 255, 50 },
                    { 50, 150, 255 },
                    { 0, 0, 0 }
                };

                float offsetStep = 3.0f * scale;

                for (int i = 0; i < 4; i++) {
                    CCLabelBMFont* copyLayer = CCLabelBMFont::create(label->getString(), label->getFntFile());
                    if (copyLayer) {
                        copyLayer->setColor(layerColors[i]);
                        copyLayer->setScale(scale);
                        copyLayer->setOpacity(255);
                        copyLayer->setAnchorPoint({ 0.5f, 0.5f });

                        float currentOffset = offsetStep * (i + 1);
                        copyLayer->setPosition({ currentOffset, -currentOffset });

                        textContainer->addChild(copyLayer, -i);
                    }
                }

                textContainer->setTag(8888);
                targetLayer->addChild(textContainer, parent->getZOrder() - 1);
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
}