#pragma once
#include <cocos2d.h>
#include <Geode/Geode.hpp>
#include <vector>
#include <algorithm>
#include <string>

using namespace cocos2d;
using namespace geode::prelude;

class BannerNotification : public CCNode {
    static std::vector<BannerNotification*> s_activeBanners;
    bool m_isExiting = false;

protected:
    cocos2d::extension::CCScale9Sprite* m_bg;
    CCPoint m_finalBGPosition;
    std::string m_spriteName;
    std::string m_bannerName;
    bool m_isNewest = true;

public:
    static void show(const std::string& bannerID,
        const std::string& spriteName, const std::string& bannerName,
        const std::string& rarityText,
        ccColor3B rarityColor,
        const std::string& titleText = "BANNER UNLOCKED!") {
        auto scene = CCDirector::sharedDirector()->getRunningScene();
        if (!scene) return;

        auto node = BannerNotification::create(bannerID, spriteName, bannerName, rarityText, rarityColor, titleText);

        s_activeBanners.insert(s_activeBanners.begin(), node);

        node->setTag(1502);
        scene->addChild(node, 100000);

        updatePositions();
    }

    virtual ~BannerNotification() {
        auto it = std::find(s_activeBanners.begin(), s_activeBanners.end(), this);
        if (it != s_activeBanners.end()) {
            s_activeBanners.erase(it);
        }
    }

    static void updatePositions() {
        float startY = 20.f;
        float gap = 65.f;

        for (size_t i = 0; i < s_activeBanners.size(); ++i) {
            auto node = s_activeBanners[i];
            if (!node || !node->m_bg) continue;
            if (node->m_isExiting) continue;

            float targetY = startY + (i * gap);

            if (std::abs(node->m_finalBGPosition.y - targetY) > 1.0f) {
                node->m_finalBGPosition.y = targetY;

                node->m_bg->stopActionByTag(999);
                auto move = CCEaseInOut::create(CCMoveTo::create(0.3f, node->m_finalBGPosition), 2.0f);
                move->setTag(999);
                node->m_bg->runAction(move);
            }
        }
    }

protected:
    static BannerNotification* create(const std::string& bannerID,
        const std::string& spriteName,
        const std::string& bannerName,
        const std::string& rarityText,
        ccColor3B rarityColor,
        const std::string& titleText) {
        auto ret = new BannerNotification();
        if (ret && ret->init(bannerID, spriteName, bannerName, rarityText, rarityColor, titleText)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    bool init(const std::string& bannerID,
        const std::string& spriteName,
        const std::string& bannerName,
        std::string rarityText,
        ccColor3B rarityColor,
        std::string titleText) {
        if (!CCNode::init()) return false;

        auto winSize = CCDirector::sharedDirector()->getWinSize();

        this->setAnchorPoint({ 1.f, 0.f });

        CCSize size = { 240.f, 60.f };
        this->setContentSize(size);

      
        m_bg = cocos2d::extension::CCScale9Sprite::create("square02_001.png");
        m_bg->setContentSize(size);
        m_bg->setPosition({ size.width / 2, size.height / 2 });
        m_bg->setOpacity(200);
        m_bg->setColor({ 0, 0, 0 });
        this->addChild(m_bg);

        float centerX = size.width / 2;

       
        auto titleLabel = CCLabelBMFont::create(titleText.c_str(), "goldFont.fnt");
        titleLabel->setScale(0.4f);
        titleLabel->setPosition({ centerX, size.height - 10.f }); 
        m_bg->addChild(titleLabel, 10);

       
        std::transform(rarityText.begin(), rarityText.end(), rarityText.begin(), ::toupper);

        auto rarityLabel = CCLabelBMFont::create(rarityText.c_str(), "goldFont.fnt");
        rarityLabel->setScale(0.35f);
        rarityLabel->setColor(rarityColor);
        rarityLabel->setPosition({ centerX, 10.f }); 
        m_bg->addChild(rarityLabel, 10);

       
        auto bannerSprite = CCSprite::create(spriteName.c_str());
        if (!bannerSprite) bannerSprite = CCSprite::create("GJ_button_01.png");

        if (bannerSprite) {
            float maxWidth = 200.f;
            float scale = 0.5f;
            if (bannerSprite->getContentSize().width > 0) {
                scale = maxWidth / bannerSprite->getContentSize().width;
                if (scale > 0.65f) scale = 0.65f;
            }
            bannerSprite->setScale(scale);
            bannerSprite->setPosition({ centerX, size.height / 2 });

        
            m_bg->addChild(bannerSprite, 1);
        }

       
        float targetX = winSize.width - 10.f;
        float startY = 20.f;

        m_finalBGPosition = ccp(size.width / 2, size.height / 2);

        this->setPosition({ winSize.width + size.width, startY });

        auto moveIn = CCEaseElasticOut::create(CCMoveTo::create(0.8f, ccp(targetX, startY)), 0.6f);
        this->runAction(moveIn);

        m_finalBGPosition = ccp(targetX, startY);

        this->scheduleOnce(schedule_selector(BannerNotification::triggerExit), 3.5f);
        FMODAudioEngine::sharedEngine()->playEffect("achievement_01.ogg");

        return true;
    }

    void triggerExit(float dt) {
        m_isExiting = true;

        auto it = std::find(s_activeBanners.begin(), s_activeBanners.end(), this);
        if (it != s_activeBanners.end()) {
            s_activeBanners.erase(it);
        }

        BannerNotification::updatePositions();

        float currentY = this->getPositionY();
        auto winSize = CCDirector::sharedDirector()->getWinSize();

        auto moveOut = CCEaseBackIn::create(CCMoveTo::create(0.5f, ccp(winSize.width + this->getContentSize().width, currentY)));
        auto remove = CCCallFunc::create(this, callfunc_selector(BannerNotification::removeFromParent));

        this->runAction(CCSequence::create(moveOut, remove, nullptr));
    }
};

inline std::vector<BannerNotification*> BannerNotification::s_activeBanners;