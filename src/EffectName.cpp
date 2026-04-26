#include "NameModifiers.h"

namespace NameModifiers {

  
    static CCSprite* createGlowDot(float radius, ccColor3B color, GLubyte opacity = 200) {
     
        auto dot = CCSprite::create("square.png");
        if (!dot) dot = CCSprite::create();
        dot->setScale(radius / 16.f);
        dot->setColor(color);
        dot->setOpacity(opacity);
        dot->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });  
        return dot;
    }

   
    static CCNode* createNodeEffect(const std::string& effectID,
        CCPoint centerPos, float halfW, float halfH, float scale) {

        auto container = CCNode::create();
        container->setPosition(centerPos);
        container->setTag(8889);

    
        if (effectID == "Explosion") {
          
            for (int i = 0; i < 3; i++) {
                auto ring = CCSprite::create("square.png");
                ring->setScale(0.05f * scale);
                ring->setOpacity(0);
                ring->setColor({ 255, 120, 0 });
                ring->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });
                ring->setPosition({ 0, 0 });

                float targetScale = (halfW * 2.2f) / (ring->getContentSize().width) * scale;
                float delay = i * 0.7f;
                auto seq = CCSequence::create(
                    CCDelayTime::create(delay),
                    CCSpawn::create(
                        CCFadeIn::create(0.05f),
                        nullptr
                    ),
                    CCSpawn::create(
                        CCScaleTo::create(0.6f, targetScale),
                        CCFadeOut::create(0.6f),
                        nullptr
                    ),
                    CCScaleTo::create(0.01f, 0.05f * scale),
                    nullptr
                );
                ring->runAction(CCRepeatForever::create(
                    CCSequence::create(seq, CCDelayTime::create(2.1f - delay), nullptr)
                ));
                container->addChild(ring);
            }

         
            for (int i = 0; i < 6; i++) {
                auto spark = CCSprite::create("square.png");
                spark->setScale(0.1f * scale);
                spark->setColor({ 255, 200, 50 });
                spark->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });
                spark->setOpacity(0);

                float angle = (360.f / 6) * i;
                float rad = angle * M_PI / 180.f;
                float dist = halfW * 0.6f;
                CCPoint target = { cosf(rad) * dist, sinf(rad) * dist };

                auto seq = CCSequence::create(
                    CCMoveTo::create(0.01f, { 0, 0 }),
                    CCFadeIn::create(0.05f),
                    CCSpawn::create(
                        CCMoveTo::create(0.4f, target),
                        CCFadeOut::create(0.4f),
                        CCScaleTo::create(0.4f, 0.02f * scale),
                        nullptr
                    ),
                    CCDelayTime::create(1.5f),
                    nullptr
                );
                spark->runAction(CCRepeatForever::create(seq));
                container->addChild(spark);
            }
        }
 
        else if (effectID == "ConcertLights") {
            ccColor3B colors[] = {
                {255, 0, 120}, {0, 100, 255}, {255, 220, 0},
                {0, 255, 80}, {200, 0, 255}, {255, 50, 50}, {0, 255, 255}
            };

            
            for (int i = 0; i < 7; i++) {
                auto draw = CCDrawNode::create();
                draw->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });

             
                float beamHeight = halfH * 4.2f;
                float beamWidth = halfW * 0.55f;
                ccColor4F col = {
                    colors[i % 7].r / 255.f,
                    colors[i % 7].g / 255.f,
                    colors[i % 7].b / 255.f,
                    0.35f
                };
                ccColor4F noBorder = { 0.f, 0.f, 0.f, 0.f };

                CCPoint verts[] = {
                    { 0, 0 },                           
                    { -beamWidth / 2, -beamHeight },    
                    { beamWidth / 2, -beamHeight }     
                };
                draw->drawPolygon(verts, 3, col, 0, noBorder);

                draw->setPosition({ 0, halfH * 1.5f });

                float baseAngle = -54.f + i * 18.f;
                draw->setRotation(baseAngle);

              
                float swingRange = 30.f + (i % 3) * 12.f;
                float duration = 0.9f + (i % 4) * 0.3f;
                auto swing = CCSequence::create(
                    CCRotateTo::create(duration, baseAngle - swingRange),
                    CCRotateTo::create(duration, baseAngle + swingRange),
                    nullptr
                );
                draw->runAction(CCRepeatForever::create(swing));

            
                auto flicker = CCSequence::create(
                    CCFadeTo::create(0.15f, 200),
                    CCFadeTo::create(0.12f, 60),
                    CCFadeTo::create(0.1f, 220),
                    CCFadeTo::create(0.2f, 80),
                    nullptr
                );
                draw->runAction(CCRepeatForever::create(flicker));

                container->addChild(draw);
            }

            
            for (int i = 0; i < 5; i++) {
                auto dot = CCSprite::create("square.png");
                dot->setScale(0.25f * scale);
                dot->setColor(colors[i]);
                dot->setOpacity(180);
                dot->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });
                float x = -halfW * 0.8f + i * (halfW * 0.4f);
                dot->setPosition({ x, 0 });

                auto pulse = CCSequence::create(
                    CCFadeTo::create(0.2f, 220),
                    CCFadeTo::create(0.2f, 60),
                    CCFadeTo::create(0.15f, 200),
                    CCFadeTo::create(0.3f, 40),
                    nullptr
                );
                dot->runAction(CCRepeatForever::create(pulse));

                auto dotColor = CCSequence::create(
                    CCTintTo::create(0.4f, colors[(i + 2) % 7].r, colors[(i + 2) % 7].g, colors[(i + 2) % 7].b),
                    CCTintTo::create(0.4f, colors[(i + 4) % 7].r, colors[(i + 4) % 7].g, colors[(i + 4) % 7].b),
                    CCTintTo::create(0.4f, colors[i].r, colors[i].g, colors[i].b),
                    nullptr
                );
                dot->runAction(CCRepeatForever::create(dotColor));
                container->addChild(dot);
            }
        }

       
        else if (effectID == "Shockwave") {
            for (int i = 0; i < 2; i++) {
                auto ring = CCSprite::create("square.png");
                ring->setScale(0.1f * scale);
                ring->setOpacity(0);
                ring->setColor(i == 0 ? ccColor3B{ 100, 200, 255 } : ccColor3B{ 255, 255, 255 });
                ring->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });

                float targetScale = (halfW * 2.5f) / (ring->getContentSize().width) * scale;
                float delay = i * 0.35f;
                auto seq = CCSequence::create(
                    CCDelayTime::create(delay),
                    CCFadeIn::create(0.05f),
                    CCSpawn::create(
                        CCScaleTo::create(0.5f, targetScale),
                        CCFadeOut::create(0.5f),
                        nullptr
                    ),
                    CCScaleTo::create(0.01f, 0.1f * scale),
                    CCDelayTime::create(1.8f - delay),
                    nullptr
                );
                ring->runAction(CCRepeatForever::create(seq));
                container->addChild(ring);
            }
        }
 
        else if (effectID == "Glitch") {
        
            for (int i = 0; i < 14; i++) {
                auto bar = CCSprite::create("square.png");
                float csW = bar->getContentSize().width;
                float csH = bar->getContentSize().height;
                float barWidth = halfW * (0.5f + (i % 4) * 0.4f);
                bar->setScaleX(barWidth / csW);
                bar->setScaleY((halfH * 0.18f) / csH);
                bar->setOpacity(0);

                ccColor3B col = (i % 4 == 0) ? ccColor3B{ 255, 0, 60 } :
                    (i % 4 == 1) ? ccColor3B{ 0, 255, 180 } :
                    (i % 4 == 2) ? ccColor3B{ 60, 60, 255 } :
                    ccColor3B{ 255, 255, 255 };
                bar->setColor(col);
                bar->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });

                float yPos = -halfH + (halfH * 2.f / 14.f) * i;
                float xOffset = ((i * 53) % 7 - 3) * halfW * 0.15f;
                bar->setPosition({ xOffset, yPos });
                 
                float onTime = 0.03f + (i % 3) * 0.02f;
                float offTime = 0.15f + (i % 5) * 0.08f;
                float jumpX = ((i * 41) % 5 - 2) * halfW * 0.3f;

                auto seq = CCSequence::create(
                    CCDelayTime::create(offTime * ((i * 37) % 7) / 7.f),
                    CCSpawn::create(
                        CCFadeIn::create(0.01f),
                        CCMoveBy::create(0.01f, { jumpX, 0 }),
                        nullptr
                    ),
                    CCDelayTime::create(onTime),
                    CCSpawn::create(
                        CCFadeOut::create(0.01f),
                        CCMoveBy::create(0.01f, { -jumpX, 0 }),
                        nullptr
                    ),
                    CCDelayTime::create(offTime * 0.5f),
                
                    CCSpawn::create(
                        CCFadeIn::create(0.01f),
                        CCMoveBy::create(0.01f, { -jumpX * 0.5f, 0 }),
                        nullptr
                    ),
                    CCDelayTime::create(0.02f),
                    CCSpawn::create(
                        CCFadeOut::create(0.01f),
                        CCMoveBy::create(0.01f, { jumpX * 0.5f, 0 }),
                        nullptr
                    ),
                    CCDelayTime::create(offTime),
                    nullptr
                );
                bar->runAction(CCRepeatForever::create(seq));
                container->addChild(bar);
            }

           
            for (int i = 0; i < 4; i++) {
                auto block = CCSprite::create("square.png");
                float csW = block->getContentSize().width;
                float csH = block->getContentSize().height;
                block->setScaleX((halfW * 0.6f) / csW);
                block->setScaleY((halfH * 0.4f) / csH);
                block->setOpacity(0);
                block->setColor(i % 2 == 0 ? ccColor3B{ 255, 0, 80 } : ccColor3B{ 0, 255, 200 });
                block->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });

                float yOff = -halfH * 0.5f + i * halfH * 0.35f;
                block->setPosition({ 0, yOff });

                auto seq = CCSequence::create(
                    CCDelayTime::create(0.5f + i * 0.3f),
                    CCMoveTo::create(0.01f, { ((i * 29) % 5 - 2) * halfW * 0.3f, yOff }),
                    CCFadeTo::create(0.02f, 120),
                    CCDelayTime::create(0.04f),
                    CCFadeTo::create(0.02f, 0),
                    CCDelayTime::create(0.8f + i * 0.2f),
                    nullptr
                );
                block->runAction(CCRepeatForever::create(seq));
                container->addChild(block);
            }
        }

       
        else if (effectID == "Orbit") {
    
            for (int i = 0; i < 8; i++) {
                auto orb = CCSprite::create("square.png");
                orb->setScale(0.18f * scale);
                orb->setColor({ 180, 120, 255 });
                orb->setOpacity(255);
                orb->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });

                auto pivot = CCNode::create();
                pivot->setPosition({ 0, 0 });
                pivot->addChild(orb);

                float radius = halfW * 0.45f + (i % 3) * halfW * 0.08f;
                orb->setPosition({ radius, 0 });

                float startAngle = (360.f / 8) * i;
                pivot->setRotation(startAngle);

                float duration = 0.8f + (i % 3) * 0.1f;
                pivot->runAction(CCRepeatForever::create(
                    CCRotateBy::create(duration, 360.f)
                ));

                auto pulse = CCSequence::create(
                    CCFadeTo::create(0.15f, 255),
                    CCFadeTo::create(0.15f, 150),
                    nullptr
                );
                orb->runAction(CCRepeatForever::create(pulse));
                container->addChild(pivot);
            }

         
            for (int i = 0; i < 6; i++) {
                auto orb = CCSprite::create("square.png");
                orb->setScale(0.22f * scale);
                ccColor3B medColors[] = {
                    {255, 100, 100}, {100, 255, 150}, {100, 150, 255},
                    {255, 255, 100}, {255, 100, 255}, {100, 255, 255}
                };
                orb->setColor(medColors[i]);
                orb->setOpacity(220);
                orb->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });

                auto pivot = CCNode::create();
                pivot->setPosition({ 0, 0 });
                pivot->addChild(orb);

                float radius = halfW * 0.8f + (i % 3) * halfW * 0.08f;
                orb->setPosition({ radius, 0 });

                float startAngle = (360.f / 6) * i + 30.f;
                pivot->setRotation(startAngle);

                float duration = 1.8f + (i % 3) * 0.2f;
                pivot->runAction(CCRepeatForever::create(
                    CCRotateBy::create(duration, 360.f)  
                ));

                auto pulse = CCSequence::create(
                    CCScaleTo::create(0.3f, 0.28f * scale),
                    CCScaleTo::create(0.3f, 0.16f * scale),
                    nullptr
                );
                orb->runAction(CCRepeatForever::create(pulse));
                container->addChild(pivot);
            }

         
            for (int i = 0; i < 4; i++) {
                auto orb = CCSprite::create("square.png");
                orb->setScale(0.3f * scale);
                ccColor3B extColors[] = {
                    {255, 150, 50}, {50, 200, 255}, {200, 50, 255}, {255, 255, 150}
                };
                orb->setColor(extColors[i]);
                orb->setOpacity(180);
                orb->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });

                auto pivot = CCNode::create();
                pivot->setPosition({ 0, 0 });
                pivot->addChild(orb);

                float radius = halfW * 1.25f + (i % 2) * halfW * 0.1f;
                orb->setPosition({ radius, 0 });

                float startAngle = (360.f / 4) * i + 45.f;
                pivot->setRotation(startAngle);

                float duration = 3.5f + i * 0.4f;
                pivot->runAction(CCRepeatForever::create(
                    CCRotateBy::create(duration, 360.f) 
                ));

                auto fade = CCSequence::create(
                    CCFadeTo::create(0.6f, 220),
                    CCFadeTo::create(0.6f, 100),
                    nullptr
                );
                orb->runAction(CCRepeatForever::create(fade));
                container->addChild(pivot);
            }

        
            auto core = CCSprite::create("square.png");
            core->setScale(0.25f * scale);
            core->setColor({ 20, 0, 40 });
            core->setOpacity(200);
            core->setPosition({ 0, 0 });

            auto corePulse = CCSequence::create(
                CCScaleTo::create(0.4f, 0.3f * scale),
                CCScaleTo::create(0.4f, 0.18f * scale),
                nullptr
            );
            core->runAction(CCRepeatForever::create(corePulse));
            container->addChild(core, -1);
        }

      
        else if (effectID == "Pulse") {
            for (int i = 0; i < 3; i++) {
                auto glow = CCSprite::create("square.png");
                float csW = glow->getContentSize().width;
                float csH = glow->getContentSize().height;
                float scX = (halfW * 2.2f) / csW * (0.8f + i * 0.15f);
                float scY = (halfH * 2.2f) / csH * (0.8f + i * 0.15f);
                glow->setScaleX(scX);
                glow->setScaleY(scY);
                glow->setOpacity(80);

                ccColor3B col = (i == 0) ? ccColor3B{ 0, 200, 255 } :
                    (i == 1) ? ccColor3B{ 100, 0, 255 } :
                    ccColor3B{ 255, 0, 150 };
                glow->setColor(col);
                glow->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });

                float delay = i * 0.6f;
                auto seq = CCSequence::create(
                    CCDelayTime::create(delay),
                    CCFadeTo::create(0.3f, 100),
                    CCSpawn::create(
                        CCScaleTo::create(0.7f, scX * 1.6f, scY * 1.6f),
                        CCFadeTo::create(0.7f, 0),
                        nullptr
                    ),
                    CCScaleTo::create(0.01f, scX, scY),
                    CCFadeTo::create(0.01f, 80),
                    CCDelayTime::create(1.2f - delay),
                    nullptr
                );
                glow->runAction(CCRepeatForever::create(seq));
                container->addChild(glow, -1);
            }
        }

     
        else if (effectID == "Scanner") {
            auto line = CCSprite::create("square.png");
            line->setScaleX(0.02f * scale);
            line->setScaleY((halfH * 2.5f) / line->getContentSize().height);
            line->setColor({ 0, 255, 180 });
            line->setOpacity(150);
            line->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });

            auto sweep = CCSequence::create(
                CCMoveTo::create(0.01f, { -halfW * 1.1f, 0 }),
                CCFadeIn::create(0.1f),
                CCMoveTo::create(1.2f, { halfW * 1.1f, 0 }),
                CCFadeOut::create(0.1f),
                CCDelayTime::create(0.8f),
                nullptr
            );
            line->runAction(CCRepeatForever::create(sweep));
            container->addChild(line);

         
            auto trail = CCSprite::create("square.png");
            trail->setScaleX(halfW * 0.5f / trail->getContentSize().width);
            trail->setScaleY((halfH * 2.2f) / trail->getContentSize().height);
            trail->setColor({ 0, 255, 180 });
            trail->setOpacity(40);
            trail->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });

            auto sweepTrail = CCSequence::create(
                CCMoveTo::create(0.01f, { -halfW * 1.3f, 0 }),
                CCFadeIn::create(0.15f),
                CCMoveTo::create(1.2f, { halfW * 0.9f, 0 }),
                CCFadeOut::create(0.2f),
                CCDelayTime::create(0.75f),
                nullptr
            );
            trail->runAction(CCRepeatForever::create(sweepTrail));
            container->addChild(trail, -1);
        }

        
        else if (effectID == "Career") {
            ccColor3B neonColors[] = {
                {255, 0, 200}, {0, 255, 200}, {255, 100, 0}, {0, 150, 255}
            };

            for (int i = 0; i < 4; i++) {
                auto light = CCSprite::create("square.png");
                light->setScale(0.2f * scale);
                light->setColor(neonColors[i]);
                light->setOpacity(220);
                light->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });

                 
                float pw = halfW * 1.15f;
                float ph = halfH * 1.15f;
                float spd = 0.4f + i * 0.05f;

                CCPoint p1 = { -pw, ph };   
                CCPoint p2 = { pw, ph };    
                CCPoint p3 = { pw, -ph };   
                CCPoint p4 = { -pw, -ph };  

                int startCorner = i % 4;
                CCPoint starts[] = { p1, p2, p3, p4 };
                light->setPosition(starts[startCorner]);

                auto path = CCSequence::create(
                    CCMoveTo::create(spd, p1),
                    CCMoveTo::create(spd, p2),
                    CCMoveTo::create(spd, p3),
                    CCMoveTo::create(spd, p4),
                    nullptr
                );
                light->runAction(CCRepeatForever::create(path));

              
                auto pulse = CCSequence::create(
                    CCFadeTo::create(0.15f, 255),
                    CCFadeTo::create(0.15f, 140),
                    nullptr
                );
                light->runAction(CCRepeatForever::create(pulse));

              
                int c1 = (i + 1) % 4, c2 = (i + 2) % 4;
                auto colorShift = CCSequence::create(
                    CCTintTo::create(1.2f, neonColors[c1].r, neonColors[c1].g, neonColors[c1].b),
                    CCTintTo::create(1.2f, neonColors[c2].r, neonColors[c2].g, neonColors[c2].b),
                    CCTintTo::create(1.2f, neonColors[i].r, neonColors[i].g, neonColors[i].b),
                    nullptr
                );
                light->runAction(CCRepeatForever::create(colorShift));

                container->addChild(light);
            }

         
            auto bgGlow = CCSprite::create("square.png");
            float csW = bgGlow->getContentSize().width;
            float csH = bgGlow->getContentSize().height;
            bgGlow->setScaleX((halfW * 2.4f) / csW);
            bgGlow->setScaleY((halfH * 2.4f) / csH);
            bgGlow->setColor({ 255, 0, 200 });
            bgGlow->setOpacity(30);
            bgGlow->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });

            auto bgColor = CCSequence::create(
                CCTintTo::create(1.5f, 0, 255, 200),
                CCTintTo::create(1.5f, 255, 100, 0),
                CCTintTo::create(1.5f, 0, 150, 255),
                CCTintTo::create(1.5f, 255, 0, 200),
                nullptr
            );
            bgGlow->runAction(CCRepeatForever::create(bgColor));

            auto bgPulse = CCSequence::create(
                CCFadeTo::create(0.8f, 50),
                CCFadeTo::create(0.8f, 15),
                nullptr
            );
            bgGlow->runAction(CCRepeatForever::create(bgPulse));
            container->addChild(bgGlow, -1);
        }

        
        else if (effectID == "Spotlight") {
            auto cone = CCSprite::create("square.png");
          
            cone->setScaleX((halfW * 1.5f) / cone->getContentSize().width);
            cone->setScaleY((halfH * 4.f) / cone->getContentSize().height);
            cone->setAnchorPoint({ 0.5f, 1.f });
            cone->setPosition({ 0, halfH * 2.5f });
            cone->setColor({ 255, 255, 200 });
            cone->setOpacity(25);
            cone->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });
 
            auto swing = CCSequence::create(
                CCRotateTo::create(2.f, -12.f),
                CCRotateTo::create(2.f, 12.f),
                nullptr
            );
            cone->runAction(CCRepeatForever::create(swing));
            container->addChild(cone, -1);

            
            auto flare = CCSprite::create("square.png");
            flare->setScale(0.3f * scale);
            flare->setColor({ 255, 255, 220 });
            flare->setOpacity(100);
            flare->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });
            flare->setPosition({ 0, 0 });

            auto pulse = CCSequence::create(
                CCFadeTo::create(0.6f, 140),
                CCFadeTo::create(0.6f, 40),
                nullptr
            );
            flare->runAction(CCRepeatForever::create(pulse));
            container->addChild(flare);
        }

       
        else if (effectID == "Matrix") {
            for (int i = 0; i < 20; i++) {
                const char* digit = (i % 2 == 0) ? "1" : "0";
                auto lbl = CCLabelBMFont::create(digit, "bigFont.fnt");
                lbl->setScale(0.2f * scale + (i % 3) * 0.05f * scale);
                lbl->setColor({ 0, 255, 60 });
                lbl->setOpacity(0);
                lbl->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });

                float xPos = -halfW * 0.95f + (halfW * 1.9f / 20.f) * i;
                float yStart = halfH * 1.5f + (i % 4) * halfH * 0.3f;
                float yEnd = -halfH * 2.0f;
                lbl->setPosition({ xPos, yStart });

                float fallDur = 0.6f + (i % 5) * 0.15f;
                float waitDur = 0.1f + (i % 7) * 0.12f;

                auto seq = CCSequence::create(
                    CCDelayTime::create(waitDur),
                    CCMoveTo::create(0.01f, { xPos, yStart }),
                    CCFadeTo::create(0.05f, 200),
                    CCSpawn::create(
                        CCMoveTo::create(fallDur, { xPos, yEnd }),
                        CCSequence::create(
                            CCDelayTime::create(fallDur * 0.6f),
                            CCFadeTo::create(fallDur * 0.4f, 0),
                            nullptr
                        ),
                        nullptr
                    ),
                    CCDelayTime::create(waitDur * 0.5f),
                    nullptr
                );
                lbl->runAction(CCRepeatForever::create(seq));
                container->addChild(lbl);
            }

            auto bgGlow = CCSprite::create("square.png");
            float csW = bgGlow->getContentSize().width;
            float csH = bgGlow->getContentSize().height;
            bgGlow->setScaleX((halfW * 2.2f) / csW);
            bgGlow->setScaleY((halfH * 2.2f) / csH);
            bgGlow->setColor({ 0, 255, 30 });
            bgGlow->setOpacity(15);
            bgGlow->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });

            auto bgPulse = CCSequence::create(
                CCFadeTo::create(0.5f, 25),
                CCFadeTo::create(0.5f, 8),
                nullptr
            );
            bgGlow->runAction(CCRepeatForever::create(bgPulse));
            container->addChild(bgGlow, -1);
        }

        else {
       
            container->release();
            return nullptr;
        }

        return container;
    }
 
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
        if (auto old = targetLayer->getChildByTag(8889)) {
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

      
        CCNode* nodeEffect = createNodeEffect(effectID, centerPos, halfW, halfH, scale);
        if (nodeEffect) {
            targetLayer->addChild(nodeEffect, parent->getZOrder() - 1);
            return;
        }    
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

       

        else if (effectID == "Sakura") {
          
            particles = CCParticleSnow::create();
            particles->setTotalParticles(50);
            particles->setLife(2.0f);
            particles->setEmissionRate(50.f / 2.0f);
            particles->setStartColor({ 1.0f, 0.7f, 0.8f, 0.9f });
            particles->setStartColorVar({ 0.0f, 0.1f, 0.1f, 0.0f });
            particles->setEndColor({ 1.0f, 0.5f, 0.65f, 0.0f });
            particles->setStartSize(10.f * scale);
            particles->setEndSize(6.f * scale);
            particles->setGravity({ 0.f, -6.f * scale });
            particles->setSpeed(8.f * scale);
            particles->setSpeedVar(3.f * scale);
            particles->setTangentialAccel(30.f * scale);
            particles->setTangentialAccelVar(15.f * scale);
            particles->setPosVar({ halfW * 0.9f, 0.f });
            particles->setAngle(-90.f);
            particles->setAngleVar(30.f);
            particles->setBlendAdditive(false);
            centerPos.y += halfH * 1.3f;
        }
        else if (effectID == "Plasma") {
          
            particles = CCParticleFire::create();
            particles->setTotalParticles(180);
            particles->setLife(0.5f);
            particles->setEmissionRate(180.f / 0.5f);
            particles->setStartColor({ 0.8f, 0.2f, 1.0f, 1.0f });
            particles->setStartColorVar({ 0.2f, 0.1f, 0.0f, 0.0f });
            particles->setEndColor({ 0.2f, 0.0f, 1.0f, 0.0f });
            particles->setStartSize(8.f * scale);
            particles->setEndSize(2.f * scale);
            particles->setSpeed(0.f);
            particles->setPosVar({ halfW * 0.85f, halfH * 0.85f });
            particles->setRadialAccel(-180.f * scale);
            particles->setTangentialAccel(100.f * scale);
            particles->setBlendAdditive(true);
            particles->setAngle(90.f);
            particles->setAngleVar(360.f);
        }
        else if (effectID == "Rainbow") {
            
            particles = CCParticleFire::create();
            particles->setTotalParticles(150);
            particles->setLife(0.8f);
            particles->setEmissionRate(150.f / 0.8f);
            particles->setStartColor({ 1.0f, 0.0f, 0.0f, 1.0f });
            particles->setStartColorVar({ 0.0f, 1.0f, 1.0f, 0.0f }); 
            particles->setEndColor({ 0.5f, 0.5f, 1.0f, 0.0f });
            particles->setEndColorVar({ 0.5f, 0.5f, 0.5f, 0.0f });
            particles->setStartSize(12.f * scale);
            particles->setEndSize(3.f * scale);
            particles->setSpeed(15.f * scale);
            particles->setPosVar({ halfW * 0.85f, halfH * 0.75f });
            particles->setAngle(90.f);
            particles->setAngleVar(360.f);
            particles->setRadialAccel(-25.f * scale);
            particles->setTangentialAccel(40.f * scale);
            particles->setBlendAdditive(true);
        }
        else if (effectID == "Meteor") {
          
            particles = CCParticleFire::create();
            particles->setTotalParticles(150);
            particles->setLife(0.8f);
            particles->setEmissionRate(150.f / 0.8f);
            particles->setStartColor({ 1.0f, 0.7f, 0.2f, 1.0f });
            particles->setStartColorVar({ 0.0f, 0.15f, 0.0f, 0.0f });
            particles->setEndColor({ 1.0f, 0.15f, 0.0f, 0.0f });
            particles->setStartSize(20.f * scale);
            particles->setEndSize(5.f * scale);
            particles->setSpeed(90.f * scale);
            particles->setSpeedVar(20.f * scale);
            particles->setAngle(225.f);
            particles->setAngleVar(12.f);
            particles->setGravity({ 20.f * scale, -35.f * scale });
            particles->setPosVar({ halfW * 0.9f, halfH * 0.5f });
            particles->setBlendAdditive(true);
            centerPos.y += halfH * 0.3f;
        }
        else if (effectID == "Heartbeat") {
        
            particles = CCParticleFire::create();
            particles->setTotalParticles(40);
            particles->setLife(1.5f);
            particles->setEmissionRate(40.f / 1.5f);
            particles->setStartColor({ 1.0f, 0.2f, 0.4f, 1.0f });
            particles->setStartColorVar({ 0.0f, 0.1f, 0.1f, 0.0f });
            particles->setEndColor({ 1.0f, 0.0f, 0.3f, 0.0f });
            particles->setStartSize(14.f * scale);
            particles->setEndSize(7.f * scale);
            particles->setSpeed(12.f * scale);
            particles->setSpeedVar(4.f * scale);
            particles->setGravity({ 0.f, 8.f * scale });
            particles->setPosVar({ halfW * 0.7f, halfH * 0.3f });
            particles->setAngle(90.f);
            particles->setAngleVar(20.f);
            particles->setBlendAdditive(false);
        }
        else if (effectID == "Shadow") {
            
            particles = CCParticleFire::create();
            particles->setTotalParticles(80);
            particles->setLife(1.2f);
            particles->setEmissionRate(80.f / 1.2f);
            particles->setStartColor({ 0.1f, 0.05f, 0.15f, 0.7f });
            particles->setEndColor({ 0.0f, 0.0f, 0.0f, 0.0f });
            particles->setStartSize(20.f * scale);
            particles->setEndSize(8.f * scale);
            particles->setSpeed(5.f * scale);
            particles->setGravity({ 0.f, -15.f * scale });
            particles->setPosVar({ halfW * 0.8f, halfH * 0.4f });
            particles->setAngle(-90.f);
            particles->setAngleVar(25.f);
            particles->setBlendAdditive(false);
        }
        else if (effectID == "Fireflies") {
         
            particles = CCParticleSnow::create();
            particles->setTotalParticles(35);
            particles->setLife(2.5f);
            particles->setEmissionRate(35.f / 2.5f);
            particles->setStartColor({ 0.9f, 1.0f, 0.3f, 1.0f });
            particles->setEndColor({ 0.5f, 0.8f, 0.0f, 0.0f });
            particles->setStartSize(8.f * scale);
            particles->setEndSize(4.f * scale);
            particles->setSpeed(6.f * scale);
            particles->setSpeedVar(3.f * scale);
            particles->setGravity({ 0.f, 3.f * scale });
            particles->setRadialAccel(-20.f * scale);
            particles->setTangentialAccel(25.f * scale);
            particles->setPosVar({ halfW * 0.9f, halfH * 0.9f });
            particles->setAngle(90.f);
            particles->setAngleVar(360.f);
            particles->setBlendAdditive(true);
        }
        else if (effectID == "Ember") {
         
            particles = CCParticleFire::create();
            particles->setTotalParticles(55);
            particles->setLife(1.8f);
            particles->setEmissionRate(55.f / 1.8f);
            particles->setStartColor({ 1.0f, 0.35f, 0.0f, 1.0f });
            particles->setStartColorVar({ 0.0f, 0.15f, 0.0f, 0.0f });
            particles->setEndColor({ 0.3f, 0.0f, 0.0f, 0.0f });
            particles->setStartSize(7.f * scale);
            particles->setEndSize(2.f * scale);
            particles->setSpeed(18.f * scale);
            particles->setSpeedVar(8.f * scale);
            particles->setGravity({ 0.f, 10.f * scale });
            particles->setTangentialAccel(15.f * scale);
            particles->setTangentialAccelVar(10.f * scale);
            particles->setPosVar({ halfW * 0.7f, 0.f });
            particles->setAngle(90.f);
            particles->setAngleVar(15.f);
            particles->setBlendAdditive(true);
            centerPos.y -= halfH * 0.6f;
        }
        else if (effectID == "Bubbles") {
         
            particles = CCParticleSnow::create();
            particles->setTotalParticles(40);
            particles->setLife(2.0f);
            particles->setEmissionRate(40.f / 2.0f);
            particles->setStartColor({ 0.6f, 0.85f, 1.0f, 0.6f });
            particles->setEndColor({ 0.8f, 0.95f, 1.0f, 0.0f });
            particles->setStartSize(10.f * scale);
            particles->setEndSize(14.f * scale);  
            particles->setSpeed(8.f * scale);
            particles->setSpeedVar(3.f * scale);
            particles->setGravity({ 0.f, 10.f * scale });
            particles->setTangentialAccel(8.f * scale);
            particles->setPosVar({ halfW * 0.8f, 0.f });
            particles->setAngle(90.f);
            particles->setAngleVar(15.f);
            particles->setBlendAdditive(false);
            centerPos.y -= halfH * 0.5f;
        }
        else if (effectID == "Supernova") {
          
            particles = CCParticleExplosion::create();
            particles->setDuration(-1);
            particles->setTotalParticles(200);
            particles->setLife(0.6f);
            particles->setEmissionRate(200.f / 0.6f);
            particles->setStartColor({ 1.0f, 0.9f, 0.7f, 1.0f });
            particles->setStartColorVar({ 0.0f, 0.1f, 0.3f, 0.0f });
            particles->setEndColor({ 0.3f, 0.1f, 0.6f, 0.0f });
            particles->setStartSize(10.f * scale);
            particles->setEndSize(2.f * scale);
            particles->setSpeed(45.f * scale);
            particles->setSpeedVar(15.f * scale);
            particles->setPosVar({ halfW * 0.2f, halfH * 0.2f });
            particles->setAngle(90.f);
            particles->setAngleVar(360.f);
            particles->setBlendAdditive(true);
        }
        else if (effectID == "Smoke") {
     
            particles = CCParticleFire::create();
            particles->setTotalParticles(60);
            particles->setLife(1.5f);
            particles->setEmissionRate(60.f / 1.5f);
            particles->setStartColor({ 0.5f, 0.5f, 0.5f, 0.5f });
            particles->setEndColor({ 0.3f, 0.3f, 0.3f, 0.0f });
            particles->setStartSize(20.f * scale);
            particles->setEndSize(35.f * scale);
            particles->setSpeed(4.f * scale);
            particles->setGravity({ 0.f, 8.f * scale });
            particles->setPosVar({ halfW * 0.6f, 0.f });
            particles->setAngle(90.f);
            particles->setAngleVar(20.f);
            particles->setBlendAdditive(false);
            centerPos.y -= halfH * 0.4f;
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