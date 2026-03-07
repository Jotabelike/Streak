#pragma once
#include "StreakCommon.h"
#include "../StreakData.h"
#include <Geode/ui/Popup.hpp>
#include <Geode/binding/SimplePlayer.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/ui/ScrollLayer.hpp>

using namespace geode::prelude;


class BundleCard : public CCNode {
public:
    static BundleCard* create(std::string iconSprite, std::string amount, std::string price) {
        auto ret = new BundleCard();
        if (ret && ret->init(iconSprite, amount, price)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    bool init(std::string iconSprite, std::string amount, std::string price) {
        if (!CCNode::init()) return false;

      
        CCSize cardSize = { 100.f, 180.f };
        this->setContentSize(cardSize);

        auto cardBg = cocos2d::extension::CCScale9Sprite::create("geode.loader/GE_square03.png");
        cardBg->setContentSize(cardSize);
        cardBg->setPosition(cardSize / 2);
        this->addChild(cardBg);

        auto icon = CCSprite::create(iconSprite.c_str());
        if (!icon) icon = CCSprite::createWithSpriteFrameName("GJ_diamondsIcon_001.png");
 
        float maxIconSize = 55.f;
        if (icon->getContentSize().width > maxIconSize) {
            icon->setScale(maxIconSize / icon->getContentSize().width);
        }
      
        icon->setPosition({ cardSize.width / 2, cardSize.height / 2 + 25.f });
        this->addChild(icon);

        auto amountLabel = CCLabelBMFont::create(amount.c_str(), "bigFont.fnt");
        amountLabel->setScale(0.40f);  
        amountLabel->setPosition({ cardSize.width / 2, cardSize.height / 2 - 25.f });
        this->addChild(amountLabel);

        auto priceLabel = CCLabelBMFont::create(price.c_str(), "goldFont.fnt");
        priceLabel->setScale(0.5f);  
      
        priceLabel->setPosition({ cardSize.width / 2, 45.f });
        this->addChild(priceLabel);

        return true;
    }
};
 
class RewardsListPopup : public Popup {
protected:
    bool init(int tier) {
        if (!Popup::init(260.f, 220.f, "geode.loader/GE_square01.png")) return false;
        std::string titleText = "Rewards";
        if (tier == 1) titleText = "Basic Rewards";
        else if (tier == 2) titleText = "VIP Rewards";
        else if (tier == 3) titleText = "Stellar Rewards";
        this->setTitle(titleText);

        auto winSize = m_mainLayer->getContentSize();

        auto list = CCNode::create();
        list->setContentSize({ 220.f, 160.f });
        list->setAnchorPoint({ 0.5f, 0.5f });
        list->setPosition(winSize / 2);

        list->setLayout(
            ColumnLayout::create()
            ->setGap(5.f)
            ->setAxisAlignment(AxisAlignment::Center)
        );

        if (tier == 1) {
            addRewardRow(list, "super_star.png"_spr, "120 Super Stars");
            addRewardRow(list, "star_tiket.png"_spr, "20k Tickets");
            addRewardRow(list, "gem.png"_spr, "50 Gems");
            addRewardRow(list, "banner19.png"_spr, "BANNER");
        }
        else if (tier == 2) {
            addRewardRow(list, "super_star.png"_spr, "300 Super Stars");
            addRewardRow(list, "star_tiket.png"_spr, "45K Tickets");
            addRewardRow(list, "gem.png"_spr, "100 Gems");
            addRewardRow(list, "banner26.png"_spr, "BANNER");
            addRewardRow(list, "banner41.png"_spr, "BANNER");
        }
        else if (tier == 3) {
            addRewardRow(list, "super_star.png"_spr, "500 Super Stars");
            addRewardRow(list, "star_tiket.png"_spr, "85K Tickets");
            addRewardRow(list, "gem.png"_spr, "200 Gems");
            addRewardRow(list, "banner16.png"_spr, "BANNER");
            addRewardRow(list, "banner32.png"_spr, "BANNER");
            addRewardRow(list, "banner40.png"_spr, "BANNER");
        }

        list->updateLayout();
        m_mainLayer->addChild(list);

        return true;
    }

    void addRewardRow(CCNode* parent, std::string spriteName, std::string text) {
        auto row = CCNode::create();
        row->setContentSize({ 200.f, 30.f });

        auto bg = cocos2d::extension::CCScale9Sprite::create("square02b_001.png");
        bg->setColor({ 0, 0, 0 });
        bg->setOpacity(75);
        bg->setContentSize({ 200.f, 30.f });
        bg->setPosition({ 100.f, 15.f });
        row->addChild(bg);

        bool isBanner = (text == "BANNER");

        auto icon = CCSprite::create(spriteName.c_str());
        if (!icon) icon = CCSprite::createWithSpriteFrameName("GJ_questionMark_001.png");

        float maxIconSize = 19.f;

        if (isBanner) {
            if (icon->getContentSize().height > 28.f) {
                icon->setScale(25.f / icon->getContentSize().height);
            }
        }
        else {
            float scale = 1.0f;
            if (icon->getContentSize().height > maxIconSize) scale = maxIconSize / icon->getContentSize().height;
            icon->setScale(scale);
        }

        float iconX = isBanner ? 100.f : 25.f;
        icon->setPosition({ iconX, 15.f });
        row->addChild(icon, 1);

        auto label = CCLabelBMFont::create(text.c_str(), "bigFont.fnt");
        label->setScale(0.4f);
        label->setAnchorPoint({ 0.f, 0.5f });
        label->setPosition({ 55.f, 15.f });
        row->addChild(label, 10);

        parent->addChild(row);
    }

public:
    static RewardsListPopup* create(int tier) {
        auto ret = new RewardsListPopup();
        if (ret && ret->init(tier)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

 
class DonationTierCard : public CCNode {
    int m_tierLevel = 1;

public:
    static DonationTierCard* create(std::string titleSpriteName, std::string price, std::string bgImageName, int tier) {
        auto ret = new DonationTierCard();
        if (ret && ret->init(titleSpriteName, price, bgImageName, tier)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    bool init(std::string titleSpriteName, std::string price, std::string bgImageName, int tier) {
        if (!CCNode::init()) return false;
        m_tierLevel = tier;

        CCSize cardSize = { 100.f, 180.f };
        this->setContentSize(cardSize);

        auto cardBg = cocos2d::extension::CCScale9Sprite::create("geode.loader/GE_square03.png");
        cardBg->setContentSize(cardSize);
        cardBg->setPosition(cardSize / 2);
        cardBg->setOpacity(255);
        this->addChild(cardBg);

        auto titleSprite = CCSprite::create(titleSpriteName.c_str());
        if (!titleSprite) titleSprite = CCSprite::createWithSpriteFrameName("GJ_button_01.png");

        float maxTitleWidth = cardSize.width - 10.f;
        if (titleSprite->getContentSize().width > maxTitleWidth) {
            titleSprite->setScale(maxTitleWidth / titleSprite->getContentSize().width);
        }
        else {
            titleSprite->setScale(0.7f);
        }
        titleSprite->setPosition({ cardSize.width / 2, cardSize.height - 20.f });
        this->addChild(titleSprite);

        auto holeBg = cocos2d::extension::CCScale9Sprite::create("square02b_001.png");
        holeBg->setContentSize({ 80.f, 80.f });
        holeBg->setColor({ 0, 0, 0 });
        holeBg->setOpacity(100);
        holeBg->setPosition({ cardSize.width / 2, cardSize.height / 2 + 10.f });
        this->addChild(holeBg);

        auto clipper = CCClippingNode::create();
        clipper->setContentSize({ 80.f, 80.f });
        clipper->setAnchorPoint({ 0.5f, 0.5f });
        clipper->setPosition({ cardSize.width / 2, cardSize.height / 2 + 10.f });
        clipper->setAlphaThreshold(0.05f);

        auto stencil = cocos2d::extension::CCScale9Sprite::create("square02_001.png");
        stencil->setContentSize({ 80.f, 80.f });
        stencil->setAnchorPoint({ 0.5f, 0.5f });
        stencil->setPosition({ 40.f, 40.f });
        clipper->setStencil(stencil);

        this->addChild(clipper);

        auto movingBg = CCSprite::create(bgImageName.c_str());
        if (!movingBg) movingBg = CCSprite::create("GJ_button_02.png");

        float targetSize = 100.f;
        float scale = targetSize / std::min(movingBg->getContentSize().width,
        movingBg->getContentSize().height);
        movingBg->setScale(scale);
        movingBg->setAnchorPoint({ 0.5f, 0.5f });
        movingBg->setPosition({ 40.f, 40.f });

        auto moveAction = CCSequence::create(
            CCMoveTo::create(4.0f, { 50.f, 50.f }),
            CCMoveTo::create(4.0f, { 30.f, 30.f }),
            nullptr
        );
        movingBg->runAction(CCRepeatForever::create(moveAction));

        clipper->addChild(movingBg);

        std::string badgeName;
        if (tier == 1) badgeName = "magic_flower_badge.png"_spr;
        else if (tier == 2) badgeName = "vip_badge.png"_spr;
        else if (tier == 3) badgeName = "stellar_badge.png"_spr;
        else badgeName = "vip_badge.png"_spr;

        auto badge = CCSprite::create(badgeName.c_str());
        if (!badge) badge = CCSprite::create(badgeName.c_str());
        if (!badge) badge = CCSprite::createWithSpriteFrameName("starSmall_001.png");

        badge->setPosition(clipper->getPosition());
        badge->setScale(0.45f);

        badge->runAction(CCRepeatForever::create(CCSequence::create(
            CCScaleTo::create(1.0f, 0.48f),
            CCScaleTo::create(1.0f, 0.45f),
            nullptr
        )));
        this->addChild(badge, 10);

        auto priceLabel = CCLabelBMFont::create(price.c_str(), "bigFont.fnt");
        priceLabel->setScale(0.5f);
        priceLabel->setPosition({ cardSize.width / 2, 45.f });
        this->addChild(priceLabel);

        auto menu = CCMenu::create();
        menu->setPosition({ cardSize.width / 2, 20.f });

        auto infoSprite = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
        infoSprite->setScale(0.6f);

        auto infoBtn = CCMenuItemSpriteExtra::create(
            infoSprite,
            this,
            menu_selector(DonationTierCard::onInfo)
        );
        menu->addChild(infoBtn);
        this->addChild(menu);

        return true;
    }

    void onInfo(CCObject*) {
        RewardsListPopup::create(m_tierLevel)->show();
    }
};

 
class DonationPopup : public Popup {
protected:
    CCNode* mTiersLayer;
    CCNode* mGemsLayer;
    CCNode* mStarsLayer;

    CCMenuItemToggler* mTiersBtn;
    CCMenuItemToggler* mGemsBtn;
    CCMenuItemToggler* mStarsBtn;

    void onOpenLink(CCObject*) {
        cocos2d::CCApplication::sharedApplication()->openURL("https://ko-fi.com/streakservers");
    }

    void onTabToggled(CCObject* sender) {
        int tag = sender->getTag();

      
        mTiersBtn->toggle(false);
        mGemsBtn->toggle(false);
        mStarsBtn->toggle(false);

       
        auto clicked = static_cast<CCMenuItemToggler*>(sender);
        clicked->toggle(true);

       
        mTiersLayer->setVisible(tag == 0);
        mGemsLayer->setVisible(tag == 1);
        mStarsLayer->setVisible(tag == 2);
    }

    bool init() override{
       
        if (!Popup::init(440.f, 280.f, "geode.loader/GE_square03.png")) return false;
        auto winSize = m_mainLayer->getContentSize();
        this->setTitle("Support");

       
        mTiersLayer = CCNode::create();
        mGemsLayer = CCNode::create();
        mStarsLayer = CCNode::create();

        mTiersLayer->setContentSize({ 420.f, 180.f });
        mGemsLayer->setContentSize({ 420.f, 180.f });
        mStarsLayer->setContentSize({ 420.f, 180.f });

        mTiersLayer->setPosition({ winSize.width / 2, winSize.height / 2 - 10.f });
        mGemsLayer->setPosition({ winSize.width / 2, winSize.height / 2 - 10.f });
        mStarsLayer->setPosition({ winSize.width / 2, winSize.height / 2 - 10.f });

        mTiersLayer->setAnchorPoint({ 0.5f, 0.5f });
        mGemsLayer->setAnchorPoint({ 0.5f, 0.5f });
        mStarsLayer->setAnchorPoint({ 0.5f, 0.5f });
 
        mTiersLayer->setLayout(RowLayout::create()->setGap(5.f)->setAxisAlignment(AxisAlignment::Center));
        mTiersLayer->addChild(DonationTierCard::create("basic.png"_spr, "$10", "basic_bg.png"_spr, 1));
        mTiersLayer->addChild(DonationTierCard::create("vip.png"_spr, "$15", "vip_bg.png"_spr, 2));
        mTiersLayer->addChild(DonationTierCard::create("stellar.png"_spr, "$25", "stellar_bg.png"_spr, 3));
        mTiersLayer->updateLayout();

      
        mGemsLayer->setLayout(RowLayout::create()->setGap(5.f)->setAxisAlignment(AxisAlignment::Center));
        mGemsLayer->addChild(BundleCard::create("gem_pack.png"_spr, "100 Gems", "$3.99"));
        mGemsLayer->addChild(BundleCard::create("gem_pack.png"_spr, "500 Gems", "$5.99"));
        mGemsLayer->addChild(BundleCard::create("gem_pack.png"_spr, "800 Gems", "$8.99"));
        mGemsLayer->updateLayout();
        mGemsLayer->setVisible(false); 

    
        mStarsLayer->setLayout(RowLayout::create()->setGap(5.f)->setAxisAlignment(AxisAlignment::Center));
        mStarsLayer->addChild(BundleCard::create("star_pack.png"_spr, "200 Stars", "$1.99"));
        mStarsLayer->addChild(BundleCard::create("star_pack.png"_spr, "500 Stars", "$2.99"));
        mStarsLayer->addChild(BundleCard::create("star_pack.png"_spr, "900 Stars", "$4.99"));
        mStarsLayer->updateLayout();
        mStarsLayer->setVisible(false);  

        m_mainLayer->addChild(mTiersLayer);
        m_mainLayer->addChild(mGemsLayer);
        m_mainLayer->addChild(mStarsLayer);

       
        auto tabMenu = CCMenu::create();
        tabMenu->setLayout(RowLayout::create()->setGap(5.f));

        auto createTabBtn = [this](std::string text, int tag) {
            auto on = ButtonSprite::create(text.c_str(), 60, true, "bigFont.fnt", "GJ_button_02.png", 25, 0.5f);
            auto off = ButtonSprite::create(text.c_str(), 60, true, "bigFont.fnt", "GJ_button_04.png", 25, 0.5f);
            auto toggle = CCMenuItemToggler::create(off, on, this, menu_selector(DonationPopup::onTabToggled));
            toggle->setTag(tag);
            return toggle;
            };

        mTiersBtn = createTabBtn("Tiers", 0);
        mGemsBtn = createTabBtn("Gems", 1);
        mStarsBtn = createTabBtn("Stars", 2);

        mTiersBtn->toggle(true);  

        tabMenu->addChild(mTiersBtn);
        tabMenu->addChild(mGemsBtn);
        tabMenu->addChild(mStarsBtn);
        tabMenu->updateLayout();
        tabMenu->setPosition({ winSize.width / 2, winSize.height - 45.f });
        m_mainLayer->addChild(tabMenu);

      
        auto menu = CCMenu::create();
        auto btnSprite = ButtonSprite::create("Donate Here", 0, 0, "goldFont.fnt", "GJ_button_01.png", 0, 0.8f);
        auto btn = CCMenuItemSpriteExtra::create(btnSprite, this, menu_selector(DonationPopup::onOpenLink));

       
        btn->setPosition({ 0, -117.f });
        menu->setPosition(winSize.width / 2, winSize.height / 2);
        menu->addChild(btn);

      
        m_mainLayer->addChild(menu);

        return true;
    }



public:
    static DonationPopup* create() {
        auto ret = new DonationPopup();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

 
class AllRachasPopup : public Popup {
protected:
    bool init() {
        if (!Popup::init(360.f, 240.f, "geode.loader/GE_square01.png")) return false;
        this->setTitle("All Streaks");
        auto winSize = m_mainLayer->getContentSize();

        std::vector<std::tuple<std::string, int, int, int>> rachas = {
            { "racha1.png"_spr, 1, 2, 25 },
            { "racha2.png"_spr, 10, 3, 40 },
            { "racha3.png"_spr, 20, 4, 55 },
            { "racha4.png"_spr, 30, 5, 70 },
            { "racha5.png"_spr, 40, 6, 85},
            { "racha6.png"_spr, 50, 7, 100 },
            { "racha6.png"_spr, 60, 8, 115 },  
            { "racha8.png"_spr, 70, 9, 130 },
            { "racha9.png"_spr, 80, 10, 145 },
            { "racha10.png"_spr, 90, 11, 160 },
            { "racha11.png"_spr, 100, 12, 175 }
        };

        auto limitNodeSize = [](CCNode* node, float maxSize) {
            if (!node) return;
            CCSize size = node->getContentSize();
            if (size.width <= 0 || size.height <= 0) return;
            float maxSide = std::max(size.width, size.height);
            if (maxSide > maxSize) {
                float scale = maxSize / maxSide;
                node->setScale(scale);
            }
            else {
                node->setScale(1.0f);
            }
            };

        CCSize scrollSize = { winSize.width - 40.f, 150.f };

        auto scrollLayer = ScrollLayer::create(scrollSize);
        scrollLayer->setPosition((winSize.width - scrollSize.width) / 2, 35.f);

        auto bg = CCScale9Sprite::create("square02_001.png");
        bg->setContentSize(scrollSize);
        bg->setOpacity(50);
        bg->setPosition(winSize.width / 2, scrollLayer->getPositionY() + scrollSize.height / 2);
        m_mainLayer->addChild(bg);

        auto content = scrollLayer->m_contentLayer;
        content->setLayout(
            ColumnLayout::create()
            ->setAxisReverse(true)
            ->setGap(5.f)
            ->setAxisAlignment(AxisAlignment::End)
            ->setAutoGrowAxis(scrollSize.height)
        );

        for (const auto& [spriteName, day, firePoints, xpAmount] : rachas) {
            auto cell = CCNode::create();
            cell->setContentSize({ scrollSize.width, 40.f });

            auto cellBg = CCScale9Sprite::create("geode.loader/GE_square03.png");
            cellBg->setContentSize({ scrollSize.width, 40.f });
            cellBg->setOpacity(75);
            cellBg->setPosition(cell->getContentSize() / 2);
            cell->addChild(cellBg);

            auto spr = CCSprite::create(spriteName.c_str());
            if (spr) {
                spr->setScale(0.25f);
                spr->setPosition({ 30.f, 20.f });
                cell->addChild(spr);
            }

            auto label = CCLabelBMFont::create(
                CCString::createWithFormat("Day %d", day)->getCString(),
                "goldFont.fnt"
            );
            label->setScale(0.5f);
            label->setAnchorPoint({ 0.f, 0.5f });
            label->setPosition({ 60.f, 20.f });
            cell->addChild(label);

            auto pointsNode = CCNode::create();
            pointsNode->setContentSize({ 130.f, 40.f });
            pointsNode->setAnchorPoint({ 1.f, 0.5f });
            pointsNode->setPosition({ scrollSize.width - 10.f, 20.f });

            auto layout = RowLayout::create();
            layout->setGap(10.f);
            layout->setAxisAlignment(AxisAlignment::End);
            layout->setAutoScale(false);
            pointsNode->setLayout(layout);

            auto xpIcon = CCSprite::create("xp.png"_spr);
            if (xpIcon) {
                limitNodeSize(xpIcon, 18.0f);
                pointsNode->addChild(xpIcon);
            }

            auto xpLabel = CCLabelBMFont::create(std::to_string(xpAmount).c_str(), "bigFont.fnt");
            xpLabel->setScale(0.35f);
            pointsNode->addChild(xpLabel);

            auto pointIcon = CCSprite::create("streak_point.png"_spr);
            if (pointIcon) {
                limitNodeSize(pointIcon, 18.0f);
                pointsNode->addChild(pointIcon);
            }

            auto pointsLabel = CCLabelBMFont::create(std::to_string(firePoints).c_str(), "bigFont.fnt");
            pointsLabel->setScale(0.35f);
            pointsNode->addChild(pointsLabel);

            pointsNode->updateLayout();
            cell->addChild(pointsNode);

            content->addChild(cell);
        }

        content->updateLayout();
        m_mainLayer->addChild(scrollLayer);

        return true;
    }

public:
    static AllRachasPopup* create() {
        auto ret = new AllRachasPopup();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

 

class StreakProgressBar : public cocos2d::CCLayerColor {
protected:
    int m_pointsGained;
    int m_pointsBefore;
    int m_pointsRequired;
    cocos2d::CCLabelBMFont* m_pointLabel;
    cocos2d::CCNode* m_barContainer;
    cocos2d::CCLayer* m_barFg;
    float m_barWidth;
    float m_barHeight;
    float m_currentPercent;
    float m_targetPercent;
    float m_currentPointsDisplay;
    float m_targetPointsDisplay;

    bool init(int pointsGained, int pointsBefore, int pointsRequired) {
        if (!CCLayerColor::initWithColor({ 0, 0, 0, 0 })) return false;
        m_pointsGained = pointsGained;
        m_pointsBefore = pointsBefore;
        m_pointsRequired = pointsRequired;
        m_currentPercent = std::min(
            1.f,
            static_cast<float>(m_pointsBefore) / m_pointsRequired
        );

        m_targetPercent = m_currentPercent;
        m_currentPointsDisplay = static_cast<float>(m_pointsBefore);
        m_targetPointsDisplay = m_currentPointsDisplay;
        auto winSize = cocos2d::CCDirector::sharedDirector()->getWinSize();
        m_barWidth = 160.0f;
        m_barHeight = 15.0f;
        m_barContainer = CCNode::create();
        m_barContainer->setPosition(
            30,
            winSize.height - 280
        );

        this->addChild(m_barContainer);
        auto streakIcon = CCSprite::create(g_streakData.getRachaSprite().c_str());
        streakIcon->setScale(0.2f);
        streakIcon->setRotation(-15.f);
        streakIcon->setPosition({
            -5, (m_barHeight + 6) / 2
            }
        );

        m_barContainer->addChild(streakIcon, 10);
        auto barBg = cocos2d::extension::CCScale9Sprite::create("GJ_button_01.png");
        barBg->setContentSize({ m_barWidth + 6, m_barHeight + 6 });
        barBg->setColor({ 0, 0, 0 });
        barBg->setOpacity(120);
        barBg->setAnchorPoint({ 0, 0 });
        barBg->setPosition({ 0, 0 });
        m_barContainer->addChild(barBg);
        auto stencil = cocos2d::extension::CCScale9Sprite::create("GJ_button_01.png");
        stencil->setContentSize({ m_barWidth, m_barHeight });
        stencil->setAnchorPoint({ 0, 0 });
        stencil->setPosition({ 3, 3 });
        auto clipper = CCClippingNode::create();
        clipper->setStencil(stencil);
        barBg->addChild(clipper);
        m_barFg = CCLayerGradient::create(
            { 255, 225, 60, 255 },
            { 255, 165, 0, 255 }
        );

        m_barFg->setContentSize({
            m_barWidth * m_currentPercent, m_barHeight
            }
        );

        m_barFg->setAnchorPoint({ 0, 0 });
        m_barFg->setPosition({ 0, 0 });
        clipper->addChild(m_barFg);
        m_pointLabel = CCLabelBMFont::create(
            CCString::createWithFormat(
                "%d/%d",
                m_pointsBefore,
                m_pointsRequired)->getCString(),
            "bigFont.fnt"
        );

        m_pointLabel->setAnchorPoint({ 1, 0.5f });
        m_pointLabel->setScale(0.4f);
        m_pointLabel->setPosition({
            m_barFg->getContentSize().width - 5, m_barHeight / 2
            }
        );

        m_barFg->addChild(m_pointLabel, 5);
        this->runAnimations();
        this->scheduleUpdate();
        return true;
    }

    void update(float dt) override {
        float smoothingFactor = 8.0f;
        m_currentPercent = m_currentPercent + (m_targetPercent - m_currentPercent) * dt * smoothingFactor;
        m_currentPointsDisplay = m_currentPointsDisplay + (m_targetPointsDisplay - m_currentPointsDisplay) * dt * smoothingFactor;
        m_barFg->setContentSize({
            m_barWidth * m_currentPercent, m_barHeight
            }
        );

        m_pointLabel->setString(
            CCString::createWithFormat(
                "%d/%d",
                static_cast<int>(
                    round
                    (m_currentPointsDisplay)),
                m_pointsRequired)->getCString()
        );

        m_pointLabel->setPosition({ m_barFg->getContentSize().width - 5, m_barHeight / 2 });
    }

    void runAnimations() {
        CCPoint onScreenPos = m_barContainer->getPosition();
        CCPoint offScreenPos = m_barContainer->getPosition() + CCPoint(-250, 0);
        m_barContainer->setPosition(offScreenPos);
        m_barContainer->runAction(CCSequence::create(CCEaseSineOut::create(
            CCMoveTo::create(0.4f, onScreenPos)),
            CCCallFunc::create(
                this,
                callfunc_selector(StreakProgressBar::spawnPointParticles)),
            CCDelayTime::create(2.5f + m_pointsGained * 0.15f),
            CCEaseSineIn::create(CCMoveTo::create(0.4f, offScreenPos)),
            CCCallFunc::create(this, callfunc_selector(StreakProgressBar::stopUpdateLoop)),
            CCRemoveSelf::create(), nullptr));
    }

    void stopUpdateLoop() {
        this->unscheduleUpdate();
    }

    void spawnPointParticles() {
        auto winSize = CCDirector::sharedDirector()->getWinSize();
        CCPoint center = winSize / 2;
        float delayPerPoint = 0.1f;
        for (int i = 0; i < m_pointsGained; ++i) {
            auto pointParticle = CCSprite::create("streak_point.png"_spr);
            pointParticle->setScale(0.25f);
            pointParticle->setPosition(center);
            this->addChild(pointParticle, 10);

            CCPoint endPos = m_barContainer->getPosition() + CCPoint(3 + m_barWidth * (
                std::min(
                    1.f,
                    (float)(m_pointsBefore + i + 1) / m_pointsRequired)),
                3 + m_barHeight / 2);

            ccBezierConfig bezier;
            bezier.endPosition = endPos;
            float explosionRadius = 150.f;
            float randomAngle = (float)(rand() % 360);
            bezier.controlPoint_1 = center + CCPoint((explosionRadius + (rand() % 50)) * cos(CC_DEGREES_TO_RADIANS(randomAngle)),
                (explosionRadius + (rand() % 50)) * sin(CC_DEGREES_TO_RADIANS(randomAngle)));
            bezier.controlPoint_2 = endPos + CCPoint(0, 100);

            auto bezierTo = CCBezierTo::create(1.0f, bezier);
            auto rotateAction = CCRotateBy::create(1.0f, 360 + (rand() % 180));
            auto scaleAction = CCScaleTo::create(1.0f, 0.1f);
            auto pointIndexObj = CCInteger::create(i + 1);
            pointParticle->runAction(CCSequence::create(
                CCDelayTime::create(i * delayPerPoint),
                CCSpawn::create(bezierTo, rotateAction, scaleAction, nullptr),
                CCCallFuncO::create(
                    this,
                    callfuncO_selector(StreakProgressBar::onPointHitBar),
                    pointIndexObj
                ),
                CCRemoveSelf::create(),
                nullptr
            )
            );
        }
    }

    void onPointHitBar(CCObject* sender) {
        if (!sender) return;

        int pointsToAdd = 0;
        if (auto node = dynamic_cast<CCNode*>(sender)) {
            auto userObj = node->getUserObject();

            if (userObj) {
                if (auto strVal = dynamic_cast<CCString*>(userObj)) {
                    pointsToAdd = strVal->intValue();
                }

                else if (auto intVal = dynamic_cast<CCInteger*>(userObj)) {
                    pointsToAdd = intVal->getValue();
                }
            }
        }

        else if (auto intVal = dynamic_cast<CCInteger*>(sender)) {
            pointsToAdd = intVal->getValue();
        }

        if (pointsToAdd == 0) return;

        int currentTotalPoints = m_pointsBefore + pointsToAdd;

        m_targetPercent = std::min(1.f, static_cast<float>(currentTotalPoints) / m_pointsRequired);
        m_targetPointsDisplay = static_cast<float>(currentTotalPoints);
        auto popUp = CCEaseSineOut::create(
            CCScaleTo::create(0.1f, 1.0f, 1.2f)
        );

        auto popDown = CCEaseSineIn::create(
            CCScaleTo::create(0.1f, 1.0f, 1.0f)
        );

        if (m_barFg) m_barFg->runAction(
            CCSequence::create(popUp, popDown, nullptr)
        );
        FMODAudioEngine::sharedEngine()->playEffect("coin.mp3"_spr);
    }

public:
    static StreakProgressBar* create(int pointsGained, int pointsBefore, int pointsRequired) {
        auto ret = new StreakProgressBar();
        if (ret && ret->init(pointsGained, pointsBefore, pointsRequired)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};