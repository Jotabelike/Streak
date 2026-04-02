#pragma once
#include <cocos2d.h>
#include <Geode/Geode.hpp>
#include <vector>
#include <string>
#include <queue>
#include <limits>
#include <algorithm>

using namespace cocos2d;
using namespace geode::prelude;

struct NotificationData {
    std::string title;
    std::string message;
    std::string iconName;
    float iconScale;
};

class SystemNotification : public cocos2d::CCLayer {
    static std::queue<NotificationData> s_queue;
    static SystemNotification* s_activeNotification;

    bool m_isExiting = false;
    bool m_isEntering = true;
    bool m_isShortened = false;

    CCSprite* m_icon = nullptr;
    CCSprite* m_textContainer = nullptr;
    cocos2d::extension::CCScale9Sprite* m_bg = nullptr;

public:
    static void show(const std::string& title, const std::string& message,
        const std::string& iconName = "face_ui.png"_spr, float iconScale = 0.2f) {
        if (s_activeNotification && !s_activeNotification->m_isExiting) {
            s_queue.push({ title, message, iconName, iconScale });
            if (!s_activeNotification->m_isShortened && !s_activeNotification->m_isEntering) {
                s_activeNotification->shortenDuration();
            }
        }
        else {
            createAndShow(title, message, iconName, iconScale);
        }
    }

private:
    static void createAndShow(const std::string& title, const std::string& message,
        const std::string& iconName, float iconScale) {

        auto node = SystemNotification::create(title, message, iconName, iconScale);
        if (!node) return;

        s_activeNotification = node;
        geode::OverlayManager::get()->addChild(node);
    }

    static void processQueue() {
        s_activeNotification = nullptr;
        if (!s_queue.empty()) {
            NotificationData data = s_queue.front();
            s_queue.pop();
            createAndShow(data.title, data.message, data.iconName, data.iconScale);
        }
    }

public:
    void shortenDuration() {
        m_isShortened = true;
        this->unschedule(schedule_selector(SystemNotification::triggerExit));
        this->scheduleOnce(schedule_selector(SystemNotification::triggerExit), 2.0f);
    }

protected:
    static SystemNotification* create(const std::string& title,
        const std::string& message, const std::string& iconName,
        float iconScale) {
        auto ret = new SystemNotification();
        if (ret && ret->init(title, message, iconName, iconScale)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    void animateChildrenEntry() {
        CCSize fullSize = this->getContentSize();

        auto stretchBg = CCEaseBackOut::create(CCScaleTo::create(0.4f, 1.0f, 1.0f));
        if (m_bg) m_bg->runAction(stretchBg);

        auto slideIcon = CCEaseBackOut::create(CCMoveTo::create(0.4f, ccp(20.f, fullSize.height / 2)));
        if (m_icon) m_icon->runAction(slideIcon);
    }

    void animateTextEntry() {
        if (m_textContainer) {
            m_textContainer->setVisible(true);
            m_textContainer->runAction(CCFadeIn::create(0.2f));
        }
    }

    void animateTextExit() {
        if (m_textContainer) {
            m_textContainer->runAction(CCFadeOut::create(0.15f));
        }
    }

    void animateChildrenExit() {
        CCSize size = this->getContentSize();
        float squareSize = 34.f / 230.f;

        auto shrinkBg = CCEaseBackIn::create(CCScaleTo::create(0.3f, squareSize, 1.0f));
        if (m_bg) m_bg->runAction(shrinkBg);

        auto centerIcon = CCEaseBackIn::create(CCMoveTo::create(0.3f, size / 2));
        if (m_icon) m_icon->runAction(centerIcon);
    }

    bool init(const std::string& title, const std::string& message,
        const std::string& iconName,
        float iconScale) {
        if (!CCLayer::init()) return false;

        this->setTouchEnabled(false);
        this->setKeypadEnabled(false);
        this->ignoreAnchorPointForPosition(false);

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        m_isEntering = true;

        CCSize fullSize = { 230.f, 34.f };
        this->setContentSize(fullSize);
        this->setAnchorPoint({ 0.5f, 0.5f });

        m_bg = cocos2d::extension::CCScale9Sprite::create("square02b_001.png");  
        m_bg->setCapInsets({ 10.f, 10.f, 60.f, 60.f });
        

        m_bg->setContentSize(fullSize);
        m_bg->setPosition(fullSize / 2);
        m_bg->setOpacity(150);
        m_bg->setColor({ 0, 0, 0 });
        this->addChild(m_bg);

        m_icon = CCSprite::create(iconName.c_str());
        if (!m_icon) m_icon = CCSprite::createWithSpriteFrameName(iconName.c_str());
        if (!m_icon) m_icon = CCSprite::create("face_ui.png"_spr);
        if (!m_icon) m_icon = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");

        if (m_icon) {
            m_icon->setScale(iconScale);
            this->addChild(m_icon, 10);
        }

        m_textContainer = CCSprite::create();
        m_textContainer->setContentSize(fullSize);
        m_textContainer->setAnchorPoint({ 0.5f, 0.5f });
        m_textContainer->setPosition(fullSize / 2);

        m_textContainer->setVisible(false);
        m_textContainer->setCascadeOpacityEnabled(true);
        m_textContainer->setOpacity(0);
        this->addChild(m_textContainer, 5);

        auto titleLabel = CCLabelBMFont::create(title.c_str(), "goldFont.fnt");
        titleLabel->setScale(0.45f);
        titleLabel->setAnchorPoint({ 0.f, 0.5f });
        titleLabel->setPosition({ 40.f, fullSize.height / 2 + 5.f });
        m_textContainer->addChild(titleLabel);

        auto msgLabel = CCLabelBMFont::create(message.c_str(), "bigFont.fnt");
        msgLabel->setScale(0.3f);
        msgLabel->setColor({ 255, 255, 255 });
        msgLabel->setAnchorPoint({ 0.f, 0.5f });
        msgLabel->setPosition({ 40.f, fullSize.height / 2 - 5.f });
        msgLabel->limitLabelWidth(170.f, 0.3f, 0.1f);
        m_textContainer->addChild(msgLabel);

        float finalY = winSize.height - 25.f;
        float startY = winSize.height + 60.f;

        this->setPosition({ winSize.width / 2, startY });

        float squareSize = 34.f / 230.f;
        m_bg->setScaleX(squareSize);
        if (m_icon) m_icon->setPosition(fullSize / 2);

        auto moveDown = CCEaseOut::create(CCMoveTo::create(0.4f, ccp(winSize.width / 2, finalY)), 2.0f);
        auto playSound = CCCallFunc::create(this, callfunc_selector(SystemNotification::playNotificationSound));
        auto animateChildren = CCCallFunc::create(this, callfunc_selector(SystemNotification::animateChildrenEntry));
        auto animateText = CCCallFunc::create(this, callfunc_selector(SystemNotification::animateTextEntry));

        auto sequence = CCSequence::create(
            CCSpawn::create(moveDown, playSound, nullptr),
            animateChildren,
            CCDelayTime::create(0.4f + 0.1f),
            animateText,
            CCDelayTime::create(0.2f),
            CCCallFunc::create(this, callfunc_selector(SystemNotification::onEnterFinished)),
            nullptr
        );

        this->runAction(sequence);

        return true;
    }

    void onEnterFinished() {
        m_isEntering = false;
        float duration = s_queue.empty() ? 4.5f : 2.0f;
        if (!s_queue.empty()) m_isShortened = true;
        this->scheduleOnce(schedule_selector(SystemNotification::triggerExit), duration);
    }

    void playNotificationSound() {
        FMODAudioEngine::sharedEngine()->playEffect("notification_ui_1.mp3"_spr);
    }

    void triggerExit(float dt) {
        if (m_isExiting) return;
        m_isExiting = true;

        auto hideTextFn = CCCallFunc::create(this, callfunc_selector(SystemNotification::animateTextExit));
        auto shrinkFn = CCCallFunc::create(this, callfunc_selector(SystemNotification::animateChildrenExit));

        auto moveUp = CCEaseIn::create(CCMoveBy::create(0.3f, ccp(0, 50.f)), 2.0f);
        auto fadeAll = CCFadeOut::create(0.3f);

        auto exitSeq = CCSequence::create(
            hideTextFn,
            CCDelayTime::create(0.15f),
            shrinkFn,
            CCDelayTime::create(0.3f),
            CCSpawn::create(moveUp, fadeAll, nullptr),
            CCCallFunc::create(this, callfunc_selector(SystemNotification::removeAndCheckQueue)),
            nullptr
        );

        this->runAction(exitSeq);
    }

    void removeAndCheckQueue() {
        this->removeFromParent();
        SystemNotification::processQueue();
    }

    virtual ~SystemNotification() {
        if (s_activeNotification == this) s_activeNotification = nullptr;
    }
};