#pragma once
#include <Geode/Geode.hpp>

namespace geode::prelude {

    inline ccColor3B HSVtoRGB(float h, float s, float v) {
        float r = 0, g = 0, b = 0;
        int i = static_cast<int>(std::floor(h * 6));
        float f = h * 6 - i;
        float p = v * (1 - s);
        float q = v * (1 - f * s);
        float t = v * (1 - (1 - f) * s);
        switch (i % 6) {
        case 0: r = v, g = t, b = p; break;
        case 1: r = q, g = v, b = p; break;
        case 2: r = p, g = v, b = t; break;
        case 3: r = p, g = q, b = v; break;
        case 4: r = t, g = p, b = v; break;
        case 5: r = v, g = p, b = q; break;
        }
        return { static_cast<GLubyte>(r * 255), static_cast<GLubyte>(g * 255), static_cast<GLubyte>(b * 255) };
    }

    class RoundedProgressBar : public CCNode {
    protected:
        CCScale9Sprite* m_bgSprite = nullptr;
        CCScale9Sprite* m_fillStencil = nullptr;
        CCLayerGradient* m_gradient = nullptr;
        CCClippingNode* m_clipper = nullptr;

        float m_progress = 0.0f;
        float m_barWidth = 140.0f;
        float m_barHeight = 16.0f;
        float m_padding = 2.0f;

        bool m_isRainbow = false;
        float m_hue = 0.0f;

        bool init(float width, float height) {
            if (!CCNode::init()) return false;

            m_barWidth = width;
            m_barHeight = height;

            this->setContentSize({ width, height });
            this->setAnchorPoint({ 0.5f, 0.5f });

            float innerW = width - m_padding * 2.0f;
            float innerH = height - m_padding * 2.0f;

            float renderScale = (height < 30.0f) ? (30.0f / height) : 1.0f;

            m_bgSprite = CCScale9Sprite::create("square02_small.png");
            m_bgSprite->setContentSize({ width * renderScale, height * renderScale });
            m_bgSprite->setScale(1.0f / renderScale);
            m_bgSprite->setPosition({ width / 2.0f, height / 2.0f });
            m_bgSprite->setColor({ 30, 30, 45 });
            m_bgSprite->setOpacity(210);
            this->addChild(m_bgSprite, 0);

            m_fillStencil = CCScale9Sprite::create("square02_small.png");
            m_fillStencil->setContentSize({ 0, innerH * renderScale });
            m_fillStencil->setScale(1.0f / renderScale);
            m_fillStencil->setAnchorPoint({ 0.0f, 0.0f });
            m_fillStencil->setPosition({ m_padding, m_padding });

            m_clipper = CCClippingNode::create(m_fillStencil);
            m_clipper->setAlphaThreshold(0.05f);
            this->addChild(m_clipper, 1);

            m_gradient = CCLayerGradient::create(ccc4(255, 255, 255, 255), ccc4(255, 255, 255, 255), ccp(0, -1));
            m_gradient->setContentSize({ innerW, innerH });
            m_gradient->ignoreAnchorPointForPosition(false);
            m_gradient->setAnchorPoint({ 0.0f, 0.0f });
            m_gradient->setPosition({ m_padding, m_padding });
            m_gradient->setVisible(false);
            m_clipper->addChild(m_gradient);

            this->scheduleUpdate();

            return true;
        }

        void updateFill() {
            float innerW = m_barWidth - m_padding * 2.0f;
            float innerH = m_barHeight - m_padding * 2.0f;
            float fillW = innerW * m_progress;

            if (fillW < 1.0f) {
                m_gradient->setVisible(false);
            }
            else {
                m_gradient->setVisible(true);
                float renderScale = (m_barHeight < 30.0f) ? (30.0f / m_barHeight) : 1.0f;
                m_fillStencil->setContentSize({ fillW * renderScale, innerH * renderScale });
            }
        }

    public:
        static RoundedProgressBar* create(float width = 140.0f, float height = 16.0f) {
            auto* ret = new RoundedProgressBar();
            if (ret->init(width, height)) {
                ret->autorelease();
                return ret;
            }
            delete ret;
            return nullptr;
        }

        void setProgress(float p) {
            m_progress = std::max(0.0f, std::min(1.0f, p));
            updateFill();
        }

        void setGradientColors(const ccColor3B& start, const ccColor3B& end) {
            m_gradient->setStartColor(start);
            m_gradient->setEndColor(end);
        }

        void setBackgroundColor(const ccColor3B& col) {
            m_bgSprite->setColor(col);
        }

        void setRainbowMode(bool enable) {
            m_isRainbow = enable;
        }

        void update(float dt) override {
            if (m_isRainbow && m_gradient->isVisible()) {
                m_hue += dt * 0.35f;
                if (m_hue > 1.0f) m_hue -= 1.0f;

                float hueStart = m_hue;
                float hueEnd = m_hue + 0.15f;
                if (hueEnd > 1.0f) hueEnd -= 1.0f;

                m_gradient->setStartColor(HSVtoRGB(hueStart, 0.8f, 1.0f));
                m_gradient->setEndColor(HSVtoRGB(hueEnd, 0.9f, 0.9f));
            }
        }
    };

} // namespace geode::prelude