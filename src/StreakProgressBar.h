#pragma once
#include <Geode/Geode.hpp>
#include "utils/RoundedProgressBar.h"

using namespace geode::prelude;

class StreakProgressBar : public CCNode {
protected:
    RoundedProgressBar* m_progressBar;
    CCLabelBMFont* m_label;
    int m_startPoints;
    int m_targetPoints;
    int m_reqPoints;
    float m_duration;
    float m_elapsed;
    float m_barWidth;

    bool init(int startPoints, int gainedPoints, int reqPoints) {
        if (!CCNode::init()) return false;

        m_startPoints = startPoints;
        m_targetPoints = startPoints + gainedPoints;
        m_reqPoints = reqPoints > 0 ? reqPoints : 1;
        m_duration = 1.0f;
        m_elapsed = 0.0f;
        m_barWidth = 200.0f;
        float m_barHeight = 14.0f; 
        m_progressBar = RoundedProgressBar::create(m_barWidth, m_barHeight);
        m_progressBar->setPosition({ 0.f, 0.f });
        m_progressBar->setGradientColors({ 0, 255, 100 }, { 0, 200, 80 });
        this->addChild(m_progressBar, 0);

 
        auto icon = CCSprite::create("streak_point.png"_spr);
        if (!icon) icon = CCSprite::createWithSpriteFrameName("starSmall_001.png");
        icon->setScale(0.15f);
        icon->setPosition({ -m_barWidth / 2.f - 15.f, 0.f });
        this->addChild(icon, 1);
        m_label = CCLabelBMFont::create("", "bigFont.fnt");
        m_label->setScale(0.35f); 
        m_label->setPosition({ 0.f, 0.f }); 
        this->addChild(m_label, 1); 
        updateBarUI(m_startPoints);

        return true;
    }

    void updateBarUI(int currentPoints) {
        float percentage = std::clamp(static_cast<float>(currentPoints) / m_reqPoints, 0.0f, 1.0f);
        m_progressBar->setProgress(percentage);
        m_label->setString(fmt::format("{} / {}", currentPoints, m_reqPoints).c_str());
    }

    void updateBar(float dt) {
        m_elapsed += dt;
        float t = std::clamp(m_elapsed / m_duration, 0.0f, 1.0f);
        float easeT = 1.0f - std::pow(1.0f - t, 3.0f);
        int currentPoints = m_startPoints + static_cast<int>((m_targetPoints - m_startPoints) * easeT);

        updateBarUI(currentPoints);

        if (t >= 1.0f) {
            this->unschedule(schedule_selector(StreakProgressBar::updateBar));      
            if (currentPoints >= m_reqPoints) {
                m_progressBar->setRainbowMode(true);
            }
        }
    }

public:
    static StreakProgressBar* create(int startPoints, int gainedPoints, int reqPoints) {
        auto ret = new StreakProgressBar();
        if (ret && ret->init(startPoints, gainedPoints, reqPoints)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    void animate() {
        this->schedule(schedule_selector(StreakProgressBar::updateBar));
    }
};