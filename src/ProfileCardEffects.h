#pragma once

#include <Geode/Geode.hpp>
#include <array>
#include <cmath>

using namespace geode::prelude;

namespace ProfileCardEffects {
    struct EffectInfo {
        const char* id;
        const char* name;
        const char* description;
    };

    inline constexpr std::array<EffectInfo, 40> EFFECTS = {{
        { "AuroraPulse", "Aurora Pulse", "A brilliant frame floods the card with color" },
        { "StarShower", "Star Shower", "A bright shower of stars crosses the profile" },
        { "CometOrbit", "Comet Orbit", "Colorful comets complete one orbit" },
        { "LightningFlash", "Lightning Flash", "Electric bolts flash across the card" },
        { "PixelSweep", "Pixel Sweep", "A colorful pixel wave scans the profile" },
        { "SolarBurst", "Solar Burst", "A radiant explosion expands from the center" },
        { "FrostReveal", "Frost Reveal", "Icy crystals reveal the profile" },
        { "HeartBurst", "Heart Burst", "A joyful burst of hearts rises upward" },
        { "MatrixRain", "Matrix Rain", "Digital green trails fall through the card" },
        { "CrownRise", "Crown Rise", "Golden lights rise into a crown" },
        { "Shockwave", "Shockwave", "Three luminous waves expand over the profile" },
        { "NeonImpact", "Neon Impact", "A powerful cyan impact shakes the whole card" },
        { "ThunderCrash", "Thunder Crash", "A wall of lightning crashes across the profile" },
        { "PlasmaRift", "Plasma Rift", "A violent plasma tear splits the card open" },
        { "NovaBlast", "Nova Blast", "A blazing nova detonates across the profile" },
        { "CrimsonPulse", "Crimson Pulse", "Heavy crimson waves strike every edge" },
        { "PrismBreak", "Prism Break", "Prismatic energy slices through the full card" },
        { "VoidCollapse", "Void Collapse", "Dark energy collapses and erupts from the center" },
        { "ElectricSurge", "Electric Surge", "Electric waves overload every edge" },
        { "OceanBreaker", "Ocean Breaker", "Deep blue pressure crashes through the card" },
        { "ToxicOverdrive", "Toxic Overdrive", "Acid energy pulses at maximum power" },
        { "GoldenCataclysm", "Golden Cataclysm", "Golden shock fronts consume the profile" },
        { "MagmaCrash", "Magma Crash", "Molten impact waves hammer the card" },
        { "ArcticImpact", "Arctic Impact", "Frozen rays explode from the center" },
        { "CyberAssault", "Cyber Assault", "Digital beams launch in every direction" },
        { "PhantomBurst", "Phantom Burst", "Spectral energy tears out of the void" },
        { "EmeraldQuake", "Emerald Quake", "Emerald force fractures the profile" },
        { "RoseDetonation", "Rose Detonation", "A fierce rose-colored blast erupts" },
        { "LaserGrid", "Laser Grid", "A high-energy grid scans the entire card" },
        { "SpectrumStorm", "Spectrum Storm", "A storm of color sweeps across the profile" },
        { "ShadowBlitz", "Shadow Blitz", "Dark slashes strike from alternating sides" },
        { "DivineFlash", "Divine Flash", "Brilliant light crosses the full profile" },
        { "QuantumBreak", "Quantum Break", "Reality splits into cyan and violet bands" },
        { "CosmicSlam", "Cosmic Slam", "Cosmic fragments slam outward from the center" },
        { "InfernoRing", "Inferno Ring", "Burning rings engulf the profile" },
        { "GlacialRupture", "Glacial Rupture", "Icy shards rupture every corner" },
        { "StarlightImpact", "Starlight Impact", "Concentrated starlight floods the card" },
        { "MeteorBarrage", "Meteor Barrage", "A heavy meteor barrage crosses the profile" },
        { "FireSpark", "Fire Sparks", "Hot sparks surge from the bottom edge" },
        { "GalaxySpiral", "Galaxy Spiral", "Cosmic lights spiral out from the center" }
    }};

    inline const EffectInfo* getInfo(const std::string& id) {
        for (auto const& effect : EFFECTS) {
            if (id == effect.id) return &effect;
        }
        return nullptr;
    }

    inline void play(CCNode* node, CCActionInterval* action, bool loopPreview) {
        if (loopPreview) node->runAction(CCRepeatForever::create(action));
        else node->runAction(action);
    }

    inline CCSprite* starSprite(ccColor3B color, float scale) {
        auto sprite = CCSprite::createWithSpriteFrameName("GJ_starsIcon_001.png");
        if (!sprite) return nullptr;
        sprite->setColor(color);
        sprite->setScale(scale);
        return sprite;
    }

    inline CCNode* createEffect(const std::string& id, CCSize size, bool loopPreview = true) {
        auto root = CCNodeRGBA::create();
        root->setContentSize(size);
        root->setCascadeOpacityEnabled(true);
        float unit = std::max(0.2f, std::min(size.width / 270.f, size.height / 160.f));
        // Keep previews and the real profile card visually identical. Preview
        // mode only controls repetition; it must not change particle, border,
        // or beam thickness relative to the available area.
        float visual = unit * 1.65f;
        CCPoint center{ size.width / 2.f, size.height / 2.f };

        if (id == "AuroraPulse") {
            float insetX = std::max(2.f, 7.f * unit);
            float insetY = std::max(2.f, 7.f * unit);
            float thickness = std::max(2.f, 5.5f * visual);
            float innerW = size.width - insetX * 2.f;
            float innerH = size.height - insetY * 2.f;

            auto addEdge = [&](CCPoint position, CCSize edgeSize, float delay) {
                auto glow = CCLayerColor::create({ 55, 120, 255, 45 },
                    edgeSize.width + thickness * 1.2f, edgeSize.height + thickness * 1.2f);
                glow->setPosition({ position.x - thickness * 0.6f, position.y - thickness * 0.6f });
                play(glow, CCSequence::create(
                    CCFadeTo::create(0.45f + delay, 155),
                    CCFadeOut::create(0.75f),
                    nullptr
                ), loopPreview);
                root->addChild(glow);

                auto edge = CCLayerColor::create({ 45, 205, 255, 100 }, edgeSize.width, edgeSize.height);
                edge->setPosition(position);
                play(edge, CCSequence::create(
                    CCDelayTime::create(delay),
                    CCSpawn::create(CCFadeTo::create(0.38f, 255), CCTintTo::create(0.38f, 75, 235, 255), nullptr),
                    CCSpawn::create(CCFadeTo::create(0.42f, 125), CCTintTo::create(0.42f, 225, 65, 255), nullptr),
                    CCSpawn::create(CCFadeTo::create(0.38f, 245), CCTintTo::create(0.38f, 45, 185, 255), nullptr),
                    CCSpawn::create(CCFadeOut::create(0.5f), CCTintTo::create(0.5f, 90, 70, 255), nullptr),
                    nullptr
                ), loopPreview);
                root->addChild(edge, 2);
            };

            addEdge({ insetX, insetY }, { innerW, thickness }, 0.f);
            addEdge({ insetX, size.height - insetY - thickness }, { innerW, thickness }, 0.08f);
            addEdge({ insetX, insetY }, { thickness, innerH }, 0.16f);
            addEdge({ size.width - insetX - thickness, insetY }, { thickness, innerH }, 0.24f);

            std::array<CCPoint, 4> corners = {{
                { insetX, insetY }, { size.width - insetX, insetY },
                { insetX, size.height - insetY }, { size.width - insetX, size.height - insetY }
            }};
            for (int i = 0; i < 4; ++i) {
                auto flare = starSprite(i % 2 == 0 ? ccColor3B{ 85, 235, 255 } : ccColor3B{ 235, 95, 255 }, 0.18f * visual);
                if (!flare) continue;
                flare->setPosition(corners[i]);
                flare->setOpacity(70);
                play(flare, CCSequence::create(
                    CCDelayTime::create(i * 0.08f),
                    CCSpawn::create(CCScaleTo::create(0.34f, 0.42f * visual), CCFadeTo::create(0.34f, 255), nullptr),
                    CCSpawn::create(CCScaleTo::create(0.5f, 0.12f * visual), CCFadeOut::create(0.5f), nullptr),
                    nullptr
                ), loopPreview);
                flare->runAction(CCRotateBy::create(1.3f, 230.f));
                root->addChild(flare, 4);
            }
        }
        else if (id == "StarShower" || id == "FrostReveal") {
            int count = id == "StarShower" ? 15 : 18;
            for (int i = 0; i < count; ++i) {
                bool frost = id == "FrostReveal";
                auto star = starSprite(
                    frost ? (i % 2 ? ccColor3B{ 205, 250, 255 } : ccColor3B{ 90, 195, 255 })
                          : (i % 2 ? ccColor3B{ 255, 235, 100 } : ccColor3B{ 115, 220, 255 }),
                    (frost ? 0.2f : 0.24f) * visual + (i % 3) * 0.035f * visual
                );
                if (!star) continue;
                float x = size.width * (0.035f + 0.067f * static_cast<float>(i % 15));
                CCPoint start{ x, size.height * (0.9f + 0.035f * static_cast<float>(i % 3)) };
                CCPoint end{ x + (frost ? ((i % 2) ? 18.f : -18.f) : -22.f) * unit,
                    size.height * (0.06f + 0.045f * static_cast<float>(i % 3)) };
                float duration = (frost ? 1.25f : 0.82f) + 0.08f * static_cast<float>(i % 4);
                star->setPosition(start);
                star->setOpacity(0);
                play(star, CCSequence::create(
                    CCMoveTo::create(0.01f, start),
                    CCFadeTo::create(0.01f, 0),
                    CCDelayTime::create((frost ? 0.07f : 0.055f) * static_cast<float>(i)),
                    CCSpawn::create(
                        CCEaseSineIn::create(CCMoveTo::create(duration, end)),
                        CCRotateBy::create(duration, frost ? 170.f : 330.f),
                        CCSequence::create(CCFadeTo::create(duration * 0.18f, 255), CCDelayTime::create(duration * 0.5f), CCFadeOut::create(duration * 0.32f), nullptr),
                        nullptr
                    ),
                    nullptr
                ), loopPreview);
                root->addChild(star, 2);
            }
        }
        else if (id == "CometOrbit") {
            float left = size.width * 0.05f;
            float right = size.width * 0.95f;
            float bottom = size.height * 0.06f;
            float top = size.height * 0.94f;
            std::array<CCPoint, 4> corners = {{ { left, bottom }, { left, top }, { right, top }, { right, bottom } }};
            for (int i = 0; i < 7; ++i) {
                int start = i % 4;
                auto comet = starSprite(i % 3 == 0 ? ccColor3B{ 70, 235, 255 } :
                    i % 3 == 1 ? ccColor3B{ 225, 95, 255 } : ccColor3B{ 255, 225, 85 },
                    (0.24f + (i % 2) * 0.055f) * visual);
                if (!comet) continue;
                comet->setPosition(corners[start]);
                comet->setOpacity(0);
                int p1 = (start + 1) % 4;
                int p2 = (start + 2) % 4;
                int p3 = (start + 3) % 4;
                play(comet, CCSequence::create(
                    CCMoveTo::create(0.01f, corners[start]),
                    CCFadeTo::create(0.01f, 0),
                    CCDelayTime::create(0.08f * i),
                    CCFadeTo::create(0.15f, 255),
                    CCEaseSineInOut::create(CCMoveTo::create(0.38f, corners[p1])),
                    CCEaseSineInOut::create(CCMoveTo::create(0.62f, corners[p2])),
                    CCEaseSineInOut::create(CCMoveTo::create(0.38f, corners[p3])),
                    CCSpawn::create(CCEaseSineInOut::create(CCMoveTo::create(0.62f, corners[start])), CCFadeOut::create(0.62f), nullptr),
                    nullptr
                ), loopPreview);
                comet->runAction(CCRotateBy::create(2.2f, 720.f));
                root->addChild(comet, 3);
            }
        }
        else if (id == "LightningFlash") {
            for (int i = 0; i < 8; ++i) {
                float height = size.height * (0.32f + 0.08f * (i % 4));
                auto bolt = CCLayerColor::create({
                    static_cast<GLubyte>(i % 2 ? 170 : 80),
                    static_cast<GLubyte>(225),
                    static_cast<GLubyte>(255),
                    static_cast<GLubyte>(0)
                }, std::max(2.f, 4.5f * visual), height);
                bolt->setAnchorPoint({ 0.5f, 0.5f });
                bolt->setPosition({ size.width * (0.08f + 0.12f * i), size.height * (0.2f + 0.08f * (i % 3)) });
                bolt->setRotation(i % 2 ? 24.f : -24.f);
                play(bolt, CCSequence::create(
                    CCFadeTo::create(0.01f, 0),
                    CCDelayTime::create(0.055f * i),
                    CCFadeTo::create(0.055f, 255),
                    CCFadeOut::create(0.13f),
                    CCDelayTime::create(0.08f),
                    CCFadeTo::create(0.035f, 230),
                    CCFadeOut::create(0.16f),
                    nullptr
                ), loopPreview);
                root->addChild(bolt, 4);
            }
        }
        else if (id == "PixelSweep") {
            for (int i = 0; i < 24; ++i) {
                float pixel = std::max(3.f, 10.f * visual);
                auto block = CCLayerColor::create({
                    static_cast<GLubyte>(i % 3 == 0 ? 65 : i % 3 == 1 ? 220 : 255),
                    static_cast<GLubyte>(i % 3 == 0 ? 220 : i % 3 == 1 ? 80 : 210),
                    static_cast<GLubyte>(i % 3 == 0 ? 255 : i % 3 == 1 ? 255 : 85), 0
                }, pixel, pixel);
                float row = static_cast<float>(i % 6);
                CCPoint start{ -pixel, size.height * (0.14f + row * 0.14f) };
                CCPoint end{ size.width + pixel, start.y + ((i / 6) % 2 ? 10.f : -10.f) * unit };
                block->setPosition(start);
                play(block, CCSequence::create(
                    CCMoveTo::create(0.01f, start), CCFadeTo::create(0.01f, 0),
                    CCDelayTime::create(0.035f * i),
                    CCSpawn::create(CCMoveTo::create(0.78f, end), CCSequence::create(CCFadeIn::create(0.12f), CCDelayTime::create(0.42f), CCFadeOut::create(0.24f), nullptr), nullptr),
                    nullptr
                ), loopPreview);
                root->addChild(block, 3);
            }
        }
        else if (id == "SolarBurst" || id == "GalaxySpiral") {
            int count = id == "SolarBurst" ? 16 : 20;
            bool galaxy = id == "GalaxySpiral";
            for (int i = 0; i < count; ++i) {
                float angle = 6.2831853f * static_cast<float>(i) / static_cast<float>(count);
                float radius = (galaxy ? 0.48f : 0.58f) * std::min(size.width, size.height);
                CCPoint end{ center.x + std::cos(angle) * radius * 1.55f, center.y + std::sin(angle) * radius };
                auto ray = starSprite(galaxy ? (i % 2 ? ccColor3B{ 205, 95, 255 } : ccColor3B{ 75, 210, 255 })
                                             : (i % 2 ? ccColor3B{ 255, 155, 45 } : ccColor3B{ 255, 245, 110 }),
                    0.14f * visual);
                if (!ray) continue;
                ray->setPosition(center);
                ray->setOpacity(0);
                CCActionInterval* movement = nullptr;
                if (galaxy) {
                    ccBezierConfig curve;
                    curve.controlPoint_1 = CCPoint{
                        center.x + std::cos(angle + 1.5f) * radius,
                        center.y + std::sin(angle + 1.5f) * radius
                    };
                    curve.controlPoint_2 = CCPoint{
                        center.x + std::cos(angle + 2.7f) * radius,
                        center.y + std::sin(angle + 2.7f) * radius
                    };
                    curve.endPosition = end;
                    movement = CCBezierTo::create(1.25f, curve);
                }
                else movement = CCEaseExponentialOut::create(CCMoveTo::create(0.78f, end));
                play(ray, CCSequence::create(
                    CCMoveTo::create(0.01f, center), CCScaleTo::create(0.01f, 0.08f * visual), CCFadeTo::create(0.01f, 0),
                    CCDelayTime::create(0.025f * i),
                    CCSpawn::create(movement, CCScaleTo::create(galaxy ? 1.25f : 0.78f, (galaxy ? 0.34f : 0.4f) * visual),
                        CCSequence::create(CCFadeIn::create(0.12f), CCDelayTime::create(galaxy ? 0.78f : 0.38f), CCFadeOut::create(galaxy ? 0.35f : 0.28f), nullptr), nullptr),
                    nullptr
                ), loopPreview);
                root->addChild(ray, 4);
            }
        }
        else if (id == "HeartBurst") {
            for (int i = 0; i < 12; ++i) {
                auto heart = CCSprite::createWithSpriteFrameName("GJ_likesIcon_001.png");
                if (!heart) heart = starSprite({ 255, 100, 175 }, 0.2f * visual);
                if (!heart) continue;
                float scale = (0.24f + (i % 3) * 0.04f) * visual;
                CCPoint start{ center.x + (i % 2 ? 12.f : -12.f) * unit, size.height * 0.12f };
                CCPoint end{ size.width * (0.08f + 0.075f * i), size.height * (0.7f + 0.055f * (i % 4)) };
                heart->setColor(i % 2 ? ccColor3B{ 255, 90, 165 } : ccColor3B{ 255, 150, 205 });
                heart->setPosition(start);
                heart->setScale(scale * 0.45f);
                heart->setOpacity(0);
                play(heart, CCSequence::create(
                    CCMoveTo::create(0.01f, start), CCScaleTo::create(0.01f, scale * 0.45f), CCFadeTo::create(0.01f, 0),
                    CCDelayTime::create(0.055f * i),
                    CCSpawn::create(CCEaseBackOut::create(CCMoveTo::create(0.82f, end)), CCScaleTo::create(0.82f, scale),
                        CCSequence::create(CCFadeIn::create(0.12f), CCDelayTime::create(0.42f), CCFadeOut::create(0.28f), nullptr), nullptr),
                    nullptr
                ), loopPreview);
                root->addChild(heart, 4);
            }
        }
        else if (id == "MatrixRain") {
            for (int i = 0; i < 18; ++i) {
                float width = std::max(2.f, 4.f * visual);
                float height = std::max(8.f, (22.f + (i % 4) * 8.f) * visual);
                auto trail = CCLayerColor::create({ 60, 255, 105, 0 }, width, height);
                float x = size.width * (0.03f + 0.055f * i);
                CCPoint start{ x, size.height + height };
                CCPoint end{ x, -height };
                trail->setPosition(start);
                play(trail, CCSequence::create(
                    CCMoveTo::create(0.01f, start), CCFadeTo::create(0.01f, 0),
                    CCDelayTime::create(0.045f * (i % 9)),
                    CCSpawn::create(CCMoveTo::create(0.85f + 0.08f * (i % 3), end),
                        CCSequence::create(CCFadeIn::create(0.1f), CCDelayTime::create(0.5f), CCFadeOut::create(0.25f), nullptr), nullptr),
                    nullptr
                ), loopPreview);
                root->addChild(trail, 3);
            }
        }
        else if (id == "CrownRise") {
            std::array<CCPoint, 9> crown = {{
                { -68.f, 0.f }, { -50.f, 30.f }, { -28.f, 8.f }, { -13.f, 42.f },
                { 0.f, 12.f }, { 13.f, 42.f }, { 28.f, 8.f }, { 50.f, 30.f }, { 68.f, 0.f }
            }};
            for (int i = 0; i < static_cast<int>(crown.size()); ++i) {
                auto light = starSprite(i == 4 ? ccColor3B{ 255, 245, 125 } : ccColor3B{ 255, 185, 55 },
                    (i == 4 ? 0.4f : 0.28f) * visual);
                if (!light) continue;
                CCPoint start{ center.x, size.height * 0.08f };
                CCPoint end{ center.x + crown[i].x * unit, size.height * 0.58f + crown[i].y * unit };
                light->setPosition(start);
                light->setOpacity(0);
                play(light, CCSequence::create(
                    CCMoveTo::create(0.01f, start), CCFadeTo::create(0.01f, 0),
                    CCDelayTime::create(0.05f * i),
                    CCSpawn::create(CCEaseBackOut::create(CCMoveTo::create(0.62f, end)), CCFadeIn::create(0.24f), CCRotateBy::create(0.62f, 180.f), nullptr),
                    CCSpawn::create(CCScaleBy::create(0.22f, 1.45f), CCFadeTo::create(0.22f, 255), nullptr),
                    CCSpawn::create(CCScaleBy::create(0.36f, 0.69f), CCFadeOut::create(0.36f), nullptr),
                    nullptr
                ), loopPreview);
                root->addChild(light, 4);
            }
        }
        else if (id == "Shockwave") {
            for (int wave = 0; wave < 3; ++wave) {
                auto frame = CCLayerColor::create({ 0, 0, 0, 0 }, size.width, size.height);
                frame->setPosition(center);
                frame->setAnchorPoint({ 0.5f, 0.5f });
                frame->ignoreAnchorPointForPosition(false);
                frame->setCascadeOpacityEnabled(true);
                frame->setScale(0.08f);
                frame->setOpacity(0);
                float thickness = std::max(2.f, 6.f * visual);
                auto addLine = [&](CCPoint pos, CCSize lineSize) {
                    auto line = CCLayerColor::create({ static_cast<GLubyte>(70 + wave * 65), 205, 255, 230 }, lineSize.width, lineSize.height);
                    line->setPosition(pos);
                    frame->addChild(line);
                };
                addLine({ 0.f, 0.f }, { size.width, thickness });
                addLine({ 0.f, size.height - thickness }, { size.width, thickness });
                addLine({ 0.f, 0.f }, { thickness, size.height });
                addLine({ size.width - thickness, 0.f }, { thickness, size.height });
                play(frame, CCSequence::create(
                    CCScaleTo::create(0.01f, 0.08f), CCFadeTo::create(0.01f, 0),
                    CCDelayTime::create(0.2f * wave),
                    CCSpawn::create(CCEaseExponentialOut::create(CCScaleTo::create(0.72f, 1.f)),
                        CCSequence::create(CCFadeIn::create(0.12f), CCFadeOut::create(0.6f), nullptr), nullptr),
                    nullptr
                ), loopPreview);
                root->addChild(frame, 3);
            }
        }
        else if (id == "NeonImpact") {
            auto flash = CCLayerColor::create({ 35, 205, 255, 0 }, size.width, size.height);
            play(flash, CCSequence::create(
                CCFadeTo::create(0.01f, 0),
                CCDelayTime::create(0.08f),
                CCFadeTo::create(0.06f, 180),
                CCFadeOut::create(0.42f),
                CCDelayTime::create(0.8f),
                nullptr
            ), loopPreview);
            root->addChild(flash, 1);

            for (int wave = 0; wave < 5; ++wave) {
                auto frame = CCLayerColor::create({ 0, 0, 0, 0 }, size.width, size.height);
                frame->setAnchorPoint({ 0.5f, 0.5f });
                frame->ignoreAnchorPointForPosition(false);
                frame->setPosition(center);
                frame->setCascadeOpacityEnabled(true);
                float thickness = std::max(2.f, (7.f - wave * 0.65f) * visual);
                ccColor3B color = wave % 2 ? ccColor3B{ 70, 120, 255 } : ccColor3B{ 45, 245, 255 };
                auto addLine = [&](CCPoint pos, CCSize lineSize) {
                    auto line = CCLayerColor::create({ color.r, color.g, color.b, 245 }, lineSize.width, lineSize.height);
                    line->setPosition(pos);
                    frame->addChild(line);
                };
                addLine({ 0.f, 0.f }, { size.width, thickness });
                addLine({ 0.f, size.height - thickness }, { size.width, thickness });
                addLine({ 0.f, 0.f }, { thickness, size.height });
                addLine({ size.width - thickness, 0.f }, { thickness, size.height });
                play(frame, CCSequence::create(
                    CCScaleTo::create(0.01f, 0.04f), CCFadeTo::create(0.01f, 0),
                    CCDelayTime::create(0.09f * wave),
                    CCSpawn::create(
                        CCEaseExponentialOut::create(CCScaleTo::create(0.55f, 1.02f)),
                        CCSequence::create(CCFadeTo::create(0.07f, 255), CCFadeOut::create(0.48f), nullptr),
                        nullptr
                    ),
                    CCDelayTime::create(0.55f),
                    nullptr
                ), loopPreview);
                root->addChild(frame, 4);
            }

            auto horizontal = CCLayerColor::create({ 175, 250, 255, 0 }, size.width, std::max(4.f, 8.f * visual));
            horizontal->setAnchorPoint({ 0.5f, 0.5f });
            horizontal->ignoreAnchorPointForPosition(false);
            horizontal->setPosition(center);
            play(horizontal, CCSequence::create(
                CCScaleTo::create(0.01f, 0.02f, 1.f), CCFadeTo::create(0.01f, 0),
                CCDelayTime::create(0.1f),
                CCSpawn::create(CCEaseExponentialOut::create(CCScaleTo::create(0.28f, 1.f, 1.f)), CCFadeTo::create(0.08f, 255), nullptr),
                CCFadeOut::create(0.32f),
                CCDelayTime::create(0.72f),
                nullptr
            ), loopPreview);
            root->addChild(horizontal, 6);
        }
        else if (id == "ThunderCrash") {
            auto stormFlash = CCLayerColor::create({ 150, 215, 255, 0 }, size.width, size.height);
            play(stormFlash, CCSequence::create(
                CCFadeTo::create(0.01f, 0), CCDelayTime::create(0.05f),
                CCFadeTo::create(0.04f, 190), CCFadeOut::create(0.12f),
                CCDelayTime::create(0.11f), CCFadeTo::create(0.035f, 135), CCFadeOut::create(0.35f),
                CCDelayTime::create(0.65f), nullptr
            ), loopPreview);
            root->addChild(stormFlash, 1);

            for (int i = 0; i < 11; ++i) {
                float thickness = std::max(2.f, (3.5f + (i % 3) * 1.4f) * visual);
                auto bolt = CCLayerColor::create({
                    static_cast<GLubyte>(i % 2 ? 125 : 225),
                    static_cast<GLubyte>(225),
                    static_cast<GLubyte>(255),
                    static_cast<GLubyte>(0)
                }, thickness, size.height * 1.35f);
                bolt->setAnchorPoint({ 0.5f, 0.5f });
                bolt->ignoreAnchorPointForPosition(false);
                bolt->setPosition({ size.width * (0.04f + 0.092f * i), center.y });
                bolt->setRotation(i % 2 ? 13.f : -13.f);
                float delay = 0.035f * std::abs(i - 5);
                play(bolt, CCSequence::create(
                    CCFadeTo::create(0.01f, 0), CCDelayTime::create(delay),
                    CCFadeTo::create(0.035f, 255), CCFadeOut::create(0.14f),
                    CCDelayTime::create(0.08f), CCFadeTo::create(0.025f, 230), CCFadeOut::create(0.2f),
                    CCDelayTime::create(0.6f), nullptr
                ), loopPreview);
                root->addChild(bolt, 5);
            }
        }
        else if (id == "PlasmaRift") {
            auto plasmaFlash = CCLayerColor::create({ 165, 45, 255, 0 }, size.width, size.height);
            play(plasmaFlash, CCSequence::create(
                CCFadeTo::create(0.01f, 0), CCDelayTime::create(0.12f),
                CCFadeTo::create(0.08f, 150), CCFadeOut::create(0.5f),
                CCDelayTime::create(0.7f), nullptr
            ), loopPreview);
            root->addChild(plasmaFlash, 1);

            float diagonal = std::sqrt(size.width * size.width + size.height * size.height) * 1.25f;
            for (int i = 0; i < 7; ++i) {
                float thickness = std::max(3.f, (4.f + (i % 3) * 2.f) * visual);
                ccColor4B color = i % 2 ? ccColor4B{ 60, 230, 255, 0 } : ccColor4B{ 230, 65, 255, 0 };
                auto slash = CCLayerColor::create(color, diagonal, thickness);
                slash->setAnchorPoint({ 0.5f, 0.5f });
                slash->ignoreAnchorPointForPosition(false);
                slash->setPosition({ center.x, center.y + (i - 3) * 10.f * unit });
                slash->setRotation(i % 2 ? 27.f : -27.f);
                play(slash, CCSequence::create(
                    CCScaleTo::create(0.01f, 0.02f, 1.f), CCFadeTo::create(0.01f, 0),
                    CCDelayTime::create(0.055f * i),
                    CCSpawn::create(CCEaseBackOut::create(CCScaleTo::create(0.28f, 1.f, 1.f)), CCFadeTo::create(0.08f, 245), nullptr),
                    CCSpawn::create(CCScaleTo::create(0.32f, 1.18f, 0.35f), CCFadeOut::create(0.32f), nullptr),
                    CCDelayTime::create(0.6f), nullptr
                ), loopPreview);
                root->addChild(slash, 4);
            }
        }
        else if (id == "NovaBlast") {
            auto novaFlash = CCLayerColor::create({ 255, 125, 20, 0 }, size.width, size.height);
            play(novaFlash, CCSequence::create(
                CCFadeTo::create(0.01f, 0), CCDelayTime::create(0.08f),
                CCFadeTo::create(0.07f, 190), CCTintTo::create(0.16f, 255, 220, 85),
                CCFadeOut::create(0.45f), CCDelayTime::create(0.6f), nullptr
            ), loopPreview);
            root->addChild(novaFlash, 1);

            float rayLength = std::max(size.width, size.height) * 0.78f;
            for (int i = 0; i < 20; ++i) {
                float angle = 360.f * static_cast<float>(i) / 20.f;
                float thickness = std::max(2.f, (2.2f + (i % 3)) * visual);
                auto ray = CCLayerColor::create({
                    static_cast<GLubyte>(255),
                    static_cast<GLubyte>(i % 2 ? 235 : 135),
                    static_cast<GLubyte>(i % 2 ? 105 : 35),
                    static_cast<GLubyte>(0)
                }, rayLength, thickness);
                ray->setAnchorPoint({ 0.f, 0.5f });
                ray->ignoreAnchorPointForPosition(false);
                ray->setPosition(center);
                ray->setRotation(angle);
                play(ray, CCSequence::create(
                    CCScaleTo::create(0.01f, 0.01f, 1.f), CCFadeTo::create(0.01f, 0),
                    CCDelayTime::create(0.012f * i),
                    CCSpawn::create(CCEaseExponentialOut::create(CCScaleTo::create(0.34f, 1.f, 1.f)), CCFadeTo::create(0.06f, 255), nullptr),
                    CCSpawn::create(CCScaleTo::create(0.38f, 1.2f, 0.1f), CCFadeOut::create(0.38f), nullptr),
                    CCDelayTime::create(0.55f), nullptr
                ), loopPreview);
                root->addChild(ray, 5);
            }
        }
        else if (id == "CrimsonPulse") {
            auto redWash = CCLayerColor::create({ 255, 25, 55, 0 }, size.width, size.height);
            play(redWash, CCSequence::create(
                CCFadeTo::create(0.01f, 0), CCFadeTo::create(0.08f, 125),
                CCFadeOut::create(0.22f), CCDelayTime::create(0.1f),
                CCFadeTo::create(0.06f, 95), CCFadeOut::create(0.35f),
                CCDelayTime::create(0.65f), nullptr
            ), loopPreview);
            root->addChild(redWash, 1);

            for (int wave = 0; wave < 5; ++wave) {
                auto frame = CCLayerColor::create({ 0, 0, 0, 0 }, size.width, size.height);
                frame->setAnchorPoint({ 0.5f, 0.5f });
                frame->ignoreAnchorPointForPosition(false);
                frame->setPosition(center);
                frame->setCascadeOpacityEnabled(true);
                float thickness = std::max(2.f, (4.3f - wave * 0.32f) * visual);
                auto addLine = [&](CCPoint pos, CCSize lineSize) {
                    auto line = CCLayerColor::create({ 255, static_cast<GLubyte>(35 + wave * 20), 75, 245 }, lineSize.width, lineSize.height);
                    line->setPosition(pos);
                    frame->addChild(line);
                };
                addLine({ 0.f, 0.f }, { size.width, thickness });
                addLine({ 0.f, size.height - thickness }, { size.width, thickness });
                addLine({ 0.f, 0.f }, { thickness, size.height });
                addLine({ size.width - thickness, 0.f }, { thickness, size.height });
                play(frame, CCSequence::create(
                    CCScaleTo::create(0.01f, 0.97f), CCFadeTo::create(0.01f, 0),
                    CCDelayTime::create(0.1f * wave),
                    CCSpawn::create(CCEaseSineInOut::create(CCScaleTo::create(0.3f, 0.72f)), CCFadeTo::create(0.08f, 235), nullptr),
                    CCSpawn::create(CCEaseExponentialOut::create(CCScaleTo::create(0.42f, 0.97f)), CCFadeOut::create(0.42f), nullptr),
                    CCDelayTime::create(0.5f), nullptr
                ), loopPreview);
                root->addChild(frame, 4);
            }
        }
        else if (id == "PrismBreak") {
            constexpr std::array<ccColor3B, 6> colors = {{
                { 255, 55, 105 }, { 255, 145, 45 }, { 255, 235, 70 },
                { 65, 240, 180 }, { 65, 170, 255 }, { 195, 75, 255 }
            }};
            float bandWidth = size.width * 0.2f;
            for (int i = 0; i < 12; ++i) {
                auto color = colors[i % colors.size()];
                auto band = CCLayerColor::create({ color.r, color.g, color.b, 0 }, bandWidth, size.height * 1.35f);
                band->setAnchorPoint({ 0.5f, 0.5f });
                band->ignoreAnchorPointForPosition(false);
                CCPoint start{ -bandWidth, center.y };
                CCPoint end{ size.width + bandWidth, center.y };
                band->setPosition(start);
                band->setRotation(i % 2 ? 10.f : -10.f);
                play(band, CCSequence::create(
                    CCMoveTo::create(0.01f, start), CCFadeTo::create(0.01f, 0),
                    CCDelayTime::create(0.045f * i),
                    CCSpawn::create(CCEaseSineInOut::create(CCMoveTo::create(0.62f, end)),
                        CCSequence::create(CCFadeTo::create(0.08f, 185), CCDelayTime::create(0.3f), CCFadeOut::create(0.24f), nullptr), nullptr),
                    CCDelayTime::create(0.45f), nullptr
                ), loopPreview);
                root->addChild(band, 3);
            }
        }
        else if (id == "VoidCollapse") {
            auto voidWash = CCLayerColor::create({ 35, 0, 65, 0 }, size.width, size.height);
            play(voidWash, CCSequence::create(
                CCFadeTo::create(0.01f, 0), CCFadeTo::create(0.25f, 190),
                CCDelayTime::create(0.18f), CCTintTo::create(0.05f, 65, 210, 255),
                CCFadeTo::create(0.05f, 220), CCFadeOut::create(0.5f),
                CCDelayTime::create(0.55f), nullptr
            ), loopPreview);
            root->addChild(voidWash, 1);

            for (int wave = 0; wave < 4; ++wave) {
                auto frame = CCLayerColor::create({ 0, 0, 0, 0 }, size.width, size.height);
                frame->setAnchorPoint({ 0.5f, 0.5f });
                frame->ignoreAnchorPointForPosition(false);
                frame->setPosition(center);
                frame->setCascadeOpacityEnabled(true);
                float thickness = std::max(2.f, (5.f + wave) * visual);
                ccColor3B color = wave % 2 ? ccColor3B{ 75, 220, 255 } : ccColor3B{ 180, 55, 255 };
                auto addLine = [&](CCPoint pos, CCSize lineSize) {
                    auto line = CCLayerColor::create({ color.r, color.g, color.b, 245 }, lineSize.width, lineSize.height);
                    line->setPosition(pos);
                    frame->addChild(line);
                };
                addLine({ 0.f, 0.f }, { size.width, thickness });
                addLine({ 0.f, size.height - thickness }, { size.width, thickness });
                addLine({ 0.f, 0.f }, { thickness, size.height });
                addLine({ size.width - thickness, 0.f }, { thickness, size.height });
                play(frame, CCSequence::create(
                    CCScaleTo::create(0.01f, 1.f), CCFadeTo::create(0.01f, 235),
                    CCDelayTime::create(0.07f * wave),
                    CCSpawn::create(CCEaseExponentialIn::create(CCScaleTo::create(0.42f, 0.08f)), CCFadeTo::create(0.42f, 255), nullptr),
                    CCSpawn::create(CCEaseExponentialOut::create(CCScaleTo::create(0.5f, 1.05f)), CCFadeOut::create(0.5f), nullptr),
                    CCDelayTime::create(0.38f), nullptr
                ), loopPreview);
                root->addChild(frame, 5);
            }
        }
        else if (id == "ElectricSurge" || id == "OceanBreaker" ||
                 id == "ToxicOverdrive" || id == "GoldenCataclysm" ||
                 id == "MagmaCrash") {
            ccColor3B primary{ 65, 225, 255 };
            ccColor3B secondary{ 115, 95, 255 };
            bool inward = false;
            bool verticalStrike = true;
            if (id == "OceanBreaker") {
                primary = { 35, 135, 255 }; secondary = { 40, 245, 235 };
                inward = true; verticalStrike = false;
            }
            else if (id == "ToxicOverdrive") {
                primary = { 105, 255, 45 }; secondary = { 225, 255, 45 };
                verticalStrike = false;
            }
            else if (id == "GoldenCataclysm") {
                primary = { 255, 215, 55 }; secondary = { 255, 120, 25 };
                inward = true;
            }
            else if (id == "MagmaCrash") {
                primary = { 255, 65, 25 }; secondary = { 255, 175, 35 };
                inward = true; verticalStrike = false;
            }

            auto wash = CCLayerColor::create({ primary.r, primary.g, primary.b, 0 }, size.width, size.height);
            play(wash, CCSequence::create(
                CCFadeTo::create(0.01f, 0), CCDelayTime::create(0.05f),
                CCFadeTo::create(0.06f, 155), CCFadeOut::create(0.2f),
                CCDelayTime::create(0.09f),
                CCTintTo::create(0.01f, secondary.r, secondary.g, secondary.b),
                CCFadeTo::create(0.05f, 110), CCFadeOut::create(0.42f),
                CCDelayTime::create(0.55f), nullptr
            ), loopPreview);
            root->addChild(wash, 1);

            for (int wave = 0; wave < 5; ++wave) {
                auto frame = CCLayerColor::create({ 0, 0, 0, 0 }, size.width, size.height);
                frame->setAnchorPoint({ 0.5f, 0.5f });
                frame->ignoreAnchorPointForPosition(false);
                frame->setPosition(center);
                frame->setCascadeOpacityEnabled(true);
                float thickness = std::max(2.f, (4.7f - wave * 0.38f) * visual);
                auto color = wave % 2 ? secondary : primary;
                auto addLine = [&](CCPoint pos, CCSize lineSize) {
                    auto line = CCLayerColor::create({ color.r, color.g, color.b, 245 }, lineSize.width, lineSize.height);
                    line->setPosition(pos);
                    frame->addChild(line);
                };
                addLine({ 0.f, 0.f }, { size.width, thickness });
                addLine({ 0.f, size.height - thickness }, { size.width, thickness });
                addLine({ 0.f, 0.f }, { thickness, size.height });
                addLine({ size.width - thickness, 0.f }, { thickness, size.height });

                float startScale = inward ? 1.f : 0.08f;
                float hitScale = inward ? 0.38f : 0.98f;
                float finishScale = inward ? 0.98f : 1.04f;
                play(frame, CCSequence::create(
                    CCScaleTo::create(0.01f, startScale), CCFadeTo::create(0.01f, 0),
                    CCDelayTime::create(0.075f * wave),
                    CCSpawn::create(CCEaseExponentialOut::create(CCScaleTo::create(0.38f, hitScale)), CCFadeTo::create(0.07f, 250), nullptr),
                    CCSpawn::create(CCEaseSineOut::create(CCScaleTo::create(0.34f, finishScale)), CCFadeOut::create(0.34f), nullptr),
                    CCDelayTime::create(0.5f), nullptr
                ), loopPreview);
                root->addChild(frame, 4);
            }

            float beamThickness = std::max(4.f, 8.f * visual);
            auto beam = CCLayerColor::create(
                { secondary.r, secondary.g, secondary.b, 0 },
                verticalStrike ? beamThickness : size.width,
                verticalStrike ? size.height : beamThickness
            );
            beam->setAnchorPoint({ 0.5f, 0.5f });
            beam->ignoreAnchorPointForPosition(false);
            beam->setPosition(center);
            play(beam, CCSequence::create(
                CCScaleTo::create(0.01f, verticalStrike ? 1.f : 0.02f, verticalStrike ? 0.02f : 1.f),
                CCFadeTo::create(0.01f, 0), CCDelayTime::create(0.11f),
                CCSpawn::create(
                    CCEaseBackOut::create(CCScaleTo::create(0.24f, 1.f, 1.f)),
                    CCFadeTo::create(0.06f, 255), nullptr
                ),
                CCSpawn::create(CCScaleTo::create(0.28f, verticalStrike ? 0.25f : 1.f, verticalStrike ? 1.f : 0.25f), CCFadeOut::create(0.28f), nullptr),
                CCDelayTime::create(0.7f), nullptr
            ), loopPreview);
            root->addChild(beam, 7);
        }
        else if (id == "ArcticImpact" || id == "CyberAssault" ||
                 id == "PhantomBurst" || id == "EmeraldQuake" ||
                 id == "RoseDetonation") {
            ccColor3B primary{ 170, 245, 255 };
            ccColor3B secondary{ 60, 155, 255 };
            float spin = 0.f;
            int rayCount = 22;
            if (id == "CyberAssault") {
                primary = { 35, 255, 225 }; secondary = { 75, 85, 255 };
                spin = 12.f; rayCount = 24;
            }
            else if (id == "PhantomBurst") {
                primary = { 205, 95, 255 }; secondary = { 85, 35, 155 };
                spin = -9.f; rayCount = 20;
            }
            else if (id == "EmeraldQuake") {
                primary = { 55, 255, 145 }; secondary = { 15, 125, 75 };
                spin = 7.f; rayCount = 26;
            }
            else if (id == "RoseDetonation") {
                primary = { 255, 85, 165 }; secondary = { 255, 175, 215 };
                spin = -14.f; rayCount = 24;
            }

            auto flash = CCLayerColor::create({ primary.r, primary.g, primary.b, 0 }, size.width, size.height);
            play(flash, CCSequence::create(
                CCFadeTo::create(0.01f, 0), CCDelayTime::create(0.09f),
                CCFadeTo::create(0.055f, 175), CCFadeOut::create(0.5f),
                CCDelayTime::create(0.75f), nullptr
            ), loopPreview);
            root->addChild(flash, 1);

            float rayLength = std::max(size.width, size.height) * 0.82f;
            for (int i = 0; i < rayCount; ++i) {
                float angle = 360.f * static_cast<float>(i) / static_cast<float>(rayCount) + spin;
                auto color = i % 2 ? primary : secondary;
                float thickness = std::max(2.f, (2.f + (i % 4) * 0.8f) * visual);
                auto ray = CCLayerColor::create({ color.r, color.g, color.b, 0 }, rayLength, thickness);
                ray->setAnchorPoint({ 0.f, 0.5f });
                ray->ignoreAnchorPointForPosition(false);
                ray->setPosition(center);
                ray->setRotation(angle);
                play(ray, CCSequence::create(
                    CCScaleTo::create(0.01f, 0.01f, 1.f), CCFadeTo::create(0.01f, 0),
                    CCDelayTime::create(0.014f * (i % 12)),
                    CCSpawn::create(CCEaseExponentialOut::create(CCScaleTo::create(0.32f, 1.f, 1.f)), CCFadeTo::create(0.055f, 245), nullptr),
                    CCSpawn::create(CCScaleTo::create(0.42f, 1.16f, 0.06f), CCFadeOut::create(0.42f), nullptr),
                    CCDelayTime::create(0.55f), nullptr
                ), loopPreview);
                root->addChild(ray, 5);
            }

            for (int pulse = 0; pulse < 3; ++pulse) {
                auto core = starSprite(pulse % 2 ? primary : secondary, (0.32f + pulse * 0.08f) * visual);
                if (!core) continue;
                core->setPosition(center);
                play(core, CCSequence::create(
                    CCScaleTo::create(0.01f, 0.05f), CCFadeTo::create(0.01f, 0),
                    CCDelayTime::create(0.07f * pulse),
                    CCSpawn::create(CCEaseBackOut::create(CCScaleTo::create(0.28f, (0.75f + pulse * 0.12f) * visual)), CCFadeTo::create(0.06f, 255), nullptr),
                    CCSpawn::create(CCScaleTo::create(0.38f, 1.05f * visual), CCFadeOut::create(0.38f), nullptr),
                    CCDelayTime::create(0.62f), nullptr
                ), loopPreview);
                core->runAction(CCRotateBy::create(1.2f, pulse % 2 ? 260.f : -260.f));
                root->addChild(core, 7);
            }
        }
        else if (id == "LaserGrid" || id == "SpectrumStorm" ||
                 id == "ShadowBlitz" || id == "DivineFlash" ||
                 id == "QuantumBreak") {
            ccColor3B primary{ 65, 255, 235 };
            ccColor3B secondary{ 65, 135, 255 };
            float baseAngle = 0.f;
            bool alternate = true;
            if (id == "SpectrumStorm") {
                primary = { 255, 75, 185 }; secondary = { 75, 220, 255 };
                baseAngle = 18.f;
            }
            else if (id == "ShadowBlitz") {
                primary = { 145, 65, 255 }; secondary = { 35, 20, 75 };
                baseAngle = -28.f;
            }
            else if (id == "DivineFlash") {
                primary = { 255, 250, 185 }; secondary = { 255, 195, 65 };
                baseAngle = 45.f; alternate = false;
            }
            else if (id == "QuantumBreak") {
                primary = { 45, 235, 255 }; secondary = { 220, 55, 255 };
                baseAngle = -16.f;
            }

            auto wash = CCLayerColor::create({ primary.r, primary.g, primary.b, 0 }, size.width, size.height);
            play(wash, CCSequence::create(
                CCFadeTo::create(0.01f, 0), CCDelayTime::create(0.16f),
                CCFadeTo::create(0.045f, 135), CCFadeOut::create(0.32f),
                CCDelayTime::create(0.8f), nullptr
            ), loopPreview);
            root->addChild(wash, 1);

            float diagonal = std::sqrt(size.width * size.width + size.height * size.height) * 1.3f;
            for (int i = 0; i < 14; ++i) {
                auto color = i % 2 ? primary : secondary;
                float thickness = std::max(3.f, (4.f + (i % 3) * 1.5f) * visual);
                auto slash = CCLayerColor::create({ color.r, color.g, color.b, 0 }, diagonal, thickness);
                slash->setAnchorPoint({ 0.5f, 0.5f });
                slash->ignoreAnchorPointForPosition(false);
                float lane = static_cast<float>((i % 7) - 3) * size.height * 0.16f;
                CCPoint start{ -size.width * 0.75f, center.y + lane };
                CCPoint end{ size.width * 1.75f, center.y - lane * 0.35f };
                slash->setPosition(start);
                slash->setRotation(baseAngle + ((alternate && i % 2) ? 90.f : 0.f));
                play(slash, CCSequence::create(
                    CCMoveTo::create(0.01f, start), CCFadeTo::create(0.01f, 0),
                    CCDelayTime::create(0.035f * i),
                    CCSpawn::create(CCEaseSineInOut::create(CCMoveTo::create(0.58f, end)),
                        CCSequence::create(CCFadeTo::create(0.055f, 220), CCDelayTime::create(0.27f), CCFadeOut::create(0.255f), nullptr), nullptr),
                    CCDelayTime::create(0.48f), nullptr
                ), loopPreview);
                root->addChild(slash, 4);
            }
        }
        else if (id == "CosmicSlam" || id == "InfernoRing" ||
                 id == "GlacialRupture" || id == "StarlightImpact" ||
                 id == "MeteorBarrage") {
            ccColor3B primary{ 105, 95, 255 };
            ccColor3B secondary{ 235, 95, 255 };
            int motionMode = 0;
            if (id == "InfernoRing") {
                primary = { 255, 65, 25 }; secondary = { 255, 205, 55 };
                motionMode = 1;
            }
            else if (id == "GlacialRupture") {
                primary = { 115, 225, 255 }; secondary = { 225, 255, 255 };
                motionMode = 2;
            }
            else if (id == "StarlightImpact") {
                primary = { 255, 245, 145 }; secondary = { 105, 205, 255 };
                motionMode = 3;
            }
            else if (id == "MeteorBarrage") {
                primary = { 255, 115, 35 }; secondary = { 255, 225, 95 };
                motionMode = 4;
            }

            auto flash = CCLayerColor::create({ primary.r, primary.g, primary.b, 0 }, size.width, size.height);
            play(flash, CCSequence::create(
                CCFadeTo::create(0.01f, 0), CCDelayTime::create(0.1f),
                CCFadeTo::create(0.055f, 145), CCFadeOut::create(0.42f),
                CCDelayTime::create(0.78f), nullptr
            ), loopPreview);
            root->addChild(flash, 1);

            for (int i = 0; i < 24; ++i) {
                float angle = 6.2831853f * static_cast<float>(i) / 24.f;
                auto color = i % 2 ? primary : secondary;
                auto shard = starSprite(color, (0.15f + (i % 4) * 0.035f) * visual);
                if (!shard) continue;
                CCPoint start = center;
                CCPoint end{
                    center.x + std::cos(angle) * size.width * 0.62f,
                    center.y + std::sin(angle) * size.height * 0.72f
                };
                if (motionMode == 2) {
                    start = CCPoint{ i % 2 ? 0.f : size.width, (i % 6) * size.height / 5.f };
                    end = center;
                }
                else if (motionMode == 3) {
                    start = CCPoint{ size.width * (0.04f + 0.04f * i), i % 2 ? size.height : 0.f };
                    end = CCPoint{ center.x + std::cos(angle) * 35.f * unit, center.y + std::sin(angle) * 25.f * unit };
                }
                else if (motionMode == 4) {
                    start = CCPoint{ size.width * (0.12f + 0.055f * (i % 15)), size.height + 20.f * unit };
                    end = CCPoint{ start.x - size.width * 0.42f, -20.f * unit };
                }
                shard->setPosition(start);
                shard->setOpacity(0);
                play(shard, CCSequence::create(
                    CCMoveTo::create(0.01f, start), CCFadeTo::create(0.01f, 0),
                    CCDelayTime::create(0.025f * i),
                    CCSpawn::create(CCEaseExponentialOut::create(CCMoveTo::create(0.58f, end)), CCRotateBy::create(0.58f, 300.f),
                        CCSequence::create(CCFadeTo::create(0.07f, 255), CCDelayTime::create(0.27f), CCFadeOut::create(0.24f), nullptr), nullptr),
                    CCDelayTime::create(0.5f), nullptr
                ), loopPreview);
                root->addChild(shard, 5);
            }

            for (int wave = 0; wave < 3; ++wave) {
                auto ring = CCLayerColor::create({ 0, 0, 0, 0 }, size.width, size.height);
                ring->setAnchorPoint({ 0.5f, 0.5f });
                ring->ignoreAnchorPointForPosition(false);
                ring->setPosition(center);
                ring->setCascadeOpacityEnabled(true);
                float thickness = std::max(2.f, (3.8f + wave * 0.7f) * visual);
                auto color = wave % 2 ? secondary : primary;
                auto addLine = [&](CCPoint pos, CCSize lineSize) {
                    auto line = CCLayerColor::create({ color.r, color.g, color.b, 235 }, lineSize.width, lineSize.height);
                    line->setPosition(pos);
                    ring->addChild(line);
                };
                addLine({ 0.f, 0.f }, { size.width, thickness });
                addLine({ 0.f, size.height - thickness }, { size.width, thickness });
                addLine({ 0.f, 0.f }, { thickness, size.height });
                addLine({ size.width - thickness, 0.f }, { thickness, size.height });
                play(ring, CCSequence::create(
                    CCScaleTo::create(0.01f, motionMode == 1 ? 0.12f : 0.05f), CCFadeTo::create(0.01f, 0),
                    CCDelayTime::create(0.1f * wave),
                    CCSpawn::create(CCEaseExponentialOut::create(CCScaleTo::create(0.48f, 0.98f)), CCFadeTo::create(0.07f, 245), nullptr),
                    CCSpawn::create(CCScaleTo::create(0.34f, 1.03f), CCFadeOut::create(0.34f), nullptr),
                    CCDelayTime::create(0.48f), nullptr
                ), loopPreview);
                root->addChild(ring, 4);
            }
        }
        else if (id == "FireSpark") {
            for (int i = 0; i < 20; ++i) {
                auto spark = starSprite(i % 3 == 0 ? ccColor3B{ 255, 245, 115 } :
                    i % 3 == 1 ? ccColor3B{ 255, 145, 45 } : ccColor3B{ 255, 65, 35 },
                    (0.16f + (i % 3) * 0.04f) * visual);
                if (!spark) continue;
                float x = size.width * (0.035f + 0.05f * i);
                CCPoint start{ x, -6.f * unit };
                CCPoint end{ x + ((i % 2) ? 24.f : -24.f) * unit, size.height * (0.65f + 0.055f * (i % 5)) };
                spark->setPosition(start);
                spark->setOpacity(0);
                play(spark, CCSequence::create(
                    CCMoveTo::create(0.01f, start), CCFadeTo::create(0.01f, 0),
                    CCDelayTime::create(0.035f * i),
                    CCSpawn::create(CCEaseSineOut::create(CCMoveTo::create(0.78f, end)), CCRotateBy::create(0.78f, 260.f),
                        CCSequence::create(CCFadeIn::create(0.1f), CCDelayTime::create(0.4f), CCFadeOut::create(0.28f), nullptr), nullptr),
                    nullptr
                ), loopPreview);
                root->addChild(spark, 4);
            }
        }

        if (!loopPreview) {
            // A shared soft exit prevents any intense effect from popping out
            // abruptly even if one of its individual particles is still alive.
            root->runAction(CCSequence::create(
                CCDelayTime::create(2.25f),
                CCEaseSineIn::create(CCFadeOut::create(0.55f)),
                nullptr
            ));
        }

        return root;
    }
}
