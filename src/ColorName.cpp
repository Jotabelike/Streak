#include "NameModifiers.h"
#include <cmath>

namespace NameModifiers {

    class GradientUpdater : public cocos2d::CCSprite {
        cocos2d::CCLabelBMFont* m_label;
        float m_t = 0.f;
        std::string m_style;

    public:
        static GradientUpdater* create(cocos2d::CCLabelBMFont* label, const std::string& style) {
            auto ret = new GradientUpdater();

            if (ret && ret->initWithTexture(label->getTexture())) {
                ret->autorelease();
                ret->m_label = label;
                ret->m_style = style;

                ret->setVisible(false);
                ret->scheduleUpdate();
                return ret;
            }
            CC_SAFE_DELETE(ret);
            return nullptr;
        }

        void update(float dt) override {
            if (!m_label) return;
            auto children = m_label->getChildren();
            if (!children) return;

            m_t += dt * 2.f;
            int count = children->count();

            for (int i = 0; i < count; i++) {
                auto letter = static_cast<cocos2d::CCSprite*>(children->objectAtIndex(i));
                if (!letter) continue;

                float phase = m_t + i * 0.3f;
                float wave = sinf(phase) * 0.5f + 0.5f;

                GLubyte r = 255, g = 255, b = 255;

                if (m_style == "Rainbow Wave") {
                    r = wave * 255;
                    g = (sinf(phase + 2.094f) * 0.5f + 0.5f) * 255;
                    b = (sinf(phase + 4.188f) * 0.5f + 0.5f) * 255;
                }
                else if (m_style == "Fire Wave") {
                    r = 255;
                    g = wave * 180;
                    b = 0;
                }
                else if (m_style == "Ocean Wave") {
                    r = 0;
                    g = wave * 150 + 100;
                    b = 255;
                }
                else if (m_style == "Cyberpunk Wave") {
                    r = wave * 255;
                    g = (1.0f - wave) * 255;
                    b = 255;
                }
                else if (m_style == "Sunset Wave") {
                    r = 255;
                    g = wave * 100 + 50;
                    b = (1.0f - wave) * 150 + 100;
                }
                else if (m_style == "Toxic Wave") {
                    r = wave * 150;
                    g = 255;
                    b = (1.0f - wave) * 200;
                }
                else if (m_style == "Ice Wave") {
                    r = 100 + (wave * 155);    
                    g = 200 + (wave * 55);     
                    b = 255;                  
                }
                else if (m_style == "Royal Wave") {
                    r = wave * 100 + 155;
                    g = (1.0f - wave) * 200 + 55;
                    b = wave * 150 + 105;
                }
                else if (m_style == "Golden Wave") {
                    r = 255;
                    g = wave * 75 + 180;
                    b = wave * 255;
                }
                else if (m_style == "Abyss Wave") {
                    r = wave * 60;
                    g = 0;
                    b = wave * 150 + 105;
                }
                else if (m_style == "Synthwave") {
                   
                    r = 150 + (wave * 105);    
                    g = wave * wave * 255;     
                    b = 255;                 
                }
                else if (m_style == "Pastel Wave") {
                    r = wave * 55 + 200;
                    g = (sinf(phase + 2.0f) * 0.5f + 0.5f) * 55 + 200;
                    b = (sinf(phase + 4.0f) * 0.5f + 0.5f) * 55 + 200;
                }
                else if (m_style == "Aurora Wave") {
                    r = wave * 50;
                    g = wave * 150 + 105;
                    b = (1.0f - wave) * 100 + 155;
                }
                else if (m_style == "Crazy Wave") {
                    r = (1.0f - wave) * 205.f + 50.f;
                    g = wave * 170.f + 50.f;
                    b = 40.f;
                }
                else if (m_style == "Galaxy Wave") {
                    float w1 = sinf(phase) * 0.5f + 0.5f;
                    float w2 = sinf(phase + 2.094f) * 0.5f + 0.5f;
                    float w3 = sinf(phase + 4.188f) * 0.5f + 0.5f;
                    float blueWeight = w1;
                    float purpleWeight = w2;
                    float whiteWeight = w3;
                    float total = blueWeight + purpleWeight + whiteWeight;
                    if (total < 0.0001f) total = 1.f;
                    blueWeight /= total;
                    purpleWeight /= total;
                    whiteWeight /= total;
                    float fr = blueWeight * 60.f + purpleWeight * 170.f + whiteWeight * 255.f;
                    float fg = blueWeight * 120.f + purpleWeight * 70.f + whiteWeight * 255.f;
                    float fb = blueWeight * 255.f + purpleWeight * 230.f + whiteWeight * 255.f;
                    r = (GLubyte)std::min(255.f, fr);
                    g = (GLubyte)std::min(255.f, fg);
                    b = (GLubyte)std::min(255.f, fb);
                }
                else if (m_style == "Bronze Wave") {
                    // brown <-> white shimmer
                    r = (GLubyte)(150.f + wave * 105.f);
                    g = (GLubyte)(90.f + wave * 165.f);
                    b = (GLubyte)(45.f + wave * 210.f);
                }
                else if (m_style == "Platinum Wave") {
                    // gray <-> white
                    GLubyte v = (GLubyte)(165.f + wave * 90.f);
                    r = v; g = v; b = v;
                }
                else if (m_style == "Gold Wave") {
                    // yellow / orange / white
                    r = 255;
                    g = (GLubyte)(150.f + wave * 105.f);   // orange .. yellow
                    b = (GLubyte)(wave * wave * 255.f);    // white flashes
                }
                else if (m_style == "Diamond Wave") {
                    // purple / blue / white blend
                    float w1 = sinf(phase) * 0.5f + 0.5f;
                    float w2 = sinf(phase + 2.094f) * 0.5f + 0.5f;
                    float w3 = sinf(phase + 4.188f) * 0.5f + 0.5f;
                    float blueW = w1, purpleW = w2, whiteW = w3;
                    float total = blueW + purpleW + whiteW;
                    if (total < 0.0001f) total = 1.f;
                    blueW /= total; purpleW /= total; whiteW /= total;
                    float fr = blueW * 50.f + purpleW * 150.f + whiteW * 255.f;
                    float fg = blueW * 100.f + purpleW * 50.f + whiteW * 255.f;
                    float fb = blueW * 255.f + purpleW * 255.f + whiteW * 255.f;
                    r = (GLubyte)std::min(255.f, fr);
                    g = (GLubyte)std::min(255.f, fg);
                    b = (GLubyte)std::min(255.f, fb);
                }

                letter->setColor({ r, g, b });
            }
        }
    };

    void applyColor(cocos2d::CCLabelBMFont* label, const std::string& colorID) {
        if (!label) return;

        label->stopActionByTag(7777);
        label->removeChildByTag(8888);

        auto children = label->getChildren();
        if (children) {
            for (int i = 0; i < children->count(); i++) {
                if (auto letter = static_cast<cocos2d::CCSprite*>(children->objectAtIndex(i))) {
                    letter->setColor({ 255, 255, 255 });
                }
            }
        }

        label->setCascadeColorEnabled(true);

        if (colorID == "Rainbow Wave" || colorID == "Fire Wave" || colorID == "Ocean Wave" ||
            colorID == "Cyberpunk Wave" || colorID == "Sunset Wave" || colorID == "Toxic Wave" ||
            colorID == "Ice Wave" || colorID == "Royal Wave" || colorID == "Golden Wave" ||
            colorID == "Abyss Wave" || colorID == "Synthwave" || colorID == "Pastel Wave" ||
            colorID == "Aurora Wave" || colorID == "Galaxy Wave" || colorID == "Crazy Wave" ||
            colorID == "Bronze Wave" || colorID == "Platinum Wave" ||
            colorID == "Gold Wave" || colorID == "Diamond Wave") {
            label->setCascadeColorEnabled(false);
            auto updater = GradientUpdater::create(label, colorID);
            updater->setTag(8888);
            label->addChild(updater);
        }
        else if (colorID.find("Static ") != std::string::npos) {
            label->setCascadeColorEnabled(false);

            if (children) {
                int count = children->count();
                for (int i = 0; i < count; i++) {
                    if (auto letter = static_cast<cocos2d::CCSprite*>(children->objectAtIndex(i))) {
                        float factor = (count > 1) ? static_cast<float>(i) / (count - 1) : 0.0f;
                        GLubyte r = 255, g = 255, b = 255;

                        if (colorID == "Static Vaporwave") {
                            r = factor * 144;
                            g = (1.0f - factor) * 200 + 55;
                            b = 122;
                        }
                        else if (colorID == "Static Deep Sea") {
                            r = 0;
                            g = factor * 255;
                            b = factor * 150 + 100;
                        }
                        else if (colorID == "Static Blood") {
                            r = (factor < 0.5f) ? (factor * 2.0f * 255) : 255;
                            g = (factor > 0.5f) ? (factor - 0.5f) * 2.0f * 200 : 0;
                            b = (factor > 0.5f) ? (factor - 0.5f) * 2.0f * 200 : 0;
                        }
                        else if (colorID == "Static Toxic") {
                            r = factor * 200;
                            g = 255;
                            b = 0;
                        }

                        letter->setColor({ r, g, b });
                    }
                }
            }
        }
        else if (colorID == "Rainbow") {
            auto tint1 = cocos2d::CCTintTo::create(0.5f, 255, 50, 50);
            auto tint2 = cocos2d::CCTintTo::create(0.5f, 255, 165, 50);
            auto tint3 = cocos2d::CCTintTo::create(0.5f, 255, 255, 50);
            auto tint4 = cocos2d::CCTintTo::create(0.5f, 50, 255, 50);
            auto tint5 = cocos2d::CCTintTo::create(0.5f, 50, 100, 255);
            auto tint6 = cocos2d::CCTintTo::create(0.5f, 150, 50, 255);
            auto seq = cocos2d::CCSequence::create(tint1, tint2, tint3, tint4, tint5, tint6, nullptr);
            auto repeat = cocos2d::CCRepeatForever::create(seq);
            repeat->setTag(7777);
            label->runAction(repeat);
        }

        else if (colorID == "Disco Blink") {
            auto seq = cocos2d::CCSequence::create(
                cocos2d::CCTintTo::create(0.1f, 255, 0, 0), cocos2d::CCTintTo::create(0.1f, 0, 255, 0),
                cocos2d::CCTintTo::create(0.1f, 0, 0, 255), cocos2d::CCTintTo::create(0.1f, 255, 255, 0),
                cocos2d::CCTintTo::create(0.1f, 0, 255, 255), cocos2d::CCTintTo::create(0.1f, 255, 0, 255), nullptr);
            auto repeat = cocos2d::CCRepeatForever::create(seq);
            repeat->setTag(7777);
            label->runAction(repeat);
        }
    
        else if (colorID == "Red") label->setColor({ 255, 50, 50 });
        else if (colorID == "Red") label->setColor({ 255, 50, 50 });
        else if (colorID == "Blue") label->setColor({ 50, 100, 255 });
        else if (colorID == "Green") label->setColor({ 50, 255, 50 });
        else if (colorID == "Yellow") label->setColor({ 255, 255, 50 });
        else if (colorID == "Purple") label->setColor({ 150, 50, 255 });
        else if (colorID == "Orange") label->setColor({ 255, 150, 50 });
        else if (colorID == "Black") label->setColor({ 0, 0, 0 });
        else if (colorID == "Cyan") label->setColor({ 0, 255, 255 });
        else if (colorID == "Pink") label->setColor({ 255, 105, 180 });
        else if (colorID == "Lime") label->setColor({ 150, 255, 50 });
        else if (colorID == "Magenta") label->setColor({ 255, 0, 255 });
        else if (colorID == "Gold") label->setColor({ 255, 215, 0 });
        else if (colorID == "Teal") label->setColor({ 0, 128, 128 });
        else if (colorID == "Silver") label->setColor({ 192, 192, 192 });
        else if (colorID == "Brown") label->setColor({ 139, 69, 19 });
        else if (colorID == "Navy") label->setColor({ 0, 0, 128 });
        else if (colorID == "Peach") label->setColor({ 255, 218, 185 });
        else if (colorID == "Maroon") label->setColor({ 128, 0, 0 });
        else if (colorID == "Mint") label->setColor({ 152, 255, 152 });
        else {
            label->setColor({ 255, 255, 255 });
        }
    }
}