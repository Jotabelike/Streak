#pragma once
#include <Geode/Geode.hpp>
#include "../StreakData.h"

using namespace geode::prelude;

class XPPopup : public Popup {
protected:
    ScrollLayer* m_list = nullptr;

    bool init() {
        if (!Popup::init(380.f, 260.f, "geode.loader/GE_square01.png")) return false;
        this->setTitle("Level Rewards");

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        auto popupSize = m_mainLayer->getContentSize();
        auto gm = GameManager::sharedState();  

    
        float bottomHeight = 80.f;
        float topMargin = 50.f;

        CCSize listSize = { 340.f, popupSize.height - bottomHeight - topMargin };

       

        auto listBg = cocos2d::extension::CCScale9Sprite::create("square02_001.png");
        listBg->setContentSize(listSize + CCSize{ 10.f, 10.f });
        listBg->setColor({ 0, 0, 0 });
        listBg->setOpacity(100);

        float listCenterY = bottomHeight + (listSize.height / 2) + 5.f;
        listBg->setPosition({ popupSize.width / 2, listCenterY });
        m_mainLayer->addChild(listBg);

        m_list = ScrollLayer::create(listSize);
        m_list->setPosition({ (popupSize.width - listSize.width) / 2, bottomHeight + 5.f });

        int maxLevel = 100;
        float itemHeight = 40.f;
        float totalHeight = std::max(listSize.height, maxLevel * itemHeight);

        m_list->m_contentLayer->setContentSize({ listSize.width, totalHeight });

        for (int i = 1; i <= maxLevel; i++) {
            float yPos = totalHeight - ((i - 1) * itemHeight) - (itemHeight / 2);
            bool isPast = i < g_streakData.currentLevel;
            bool isCurrent = i == g_streakData.currentLevel;

            int opacity = (i % 2 == 0) ? 100 : 50;
            if (isCurrent) opacity = 180;

            auto rowBg = CCLayerColor::create({ 0, 0, 0, (GLubyte)opacity }, listSize.width, itemHeight);
            rowBg->setPosition({ 0.f, yPos - (itemHeight / 2) });
            m_list->m_contentLayer->addChild(rowBg);

            auto lblLvl = CCLabelBMFont::create(fmt::format("Lv.{}", i).c_str(), "goldFont.fnt");
            lblLvl->setScale(0.5f);
            lblLvl->setAnchorPoint({ 0.f, 0.5f });
            lblLvl->setPosition({ 15.f, yPos });
            if (isPast) lblLvl->setColor({ 150, 150, 150 });
            if (isCurrent) lblLvl->setColor({ 255, 255, 0 });
            m_list->m_contentLayer->addChild(lblLvl);

            auto rewards = g_streakData.getRewardsForLevel(i);
            float iconX = 90.f;

          
            if (rewards.gems > 0) {

                auto gemIcon = CCSprite::create("gem.png"_spr);
                gemIcon->setScale(0.25f);
                gemIcon->setPosition({ iconX, yPos });
                m_list->m_contentLayer->addChild(gemIcon);

                auto gemTxt = CCLabelBMFont::create(fmt::format("x{}", rewards.gems).c_str(), "bigFont.fnt");
                gemTxt->setScale(0.35f);
                gemTxt->setAnchorPoint({ 0.f, 0.5f });
                gemTxt->setPosition({ iconX + 15.f, yPos });
                m_list->m_contentLayer->addChild(gemTxt);

                iconX += 70.f; 
            }

          
            if (rewards.stars > 0) {
                auto starIcon = CCSprite::create("super_star.png"_spr);
                starIcon->setScale(0.25f);
                starIcon->setPosition({ iconX, yPos });
                m_list->m_contentLayer->addChild(starIcon);

                auto starTxt = CCLabelBMFont::create(fmt::format("x{}", rewards.stars).c_str(), "bigFont.fnt");
                starTxt->setScale(0.35f);
                starTxt->setAnchorPoint({ 0.f, 0.5f });
                starTxt->setPosition({ iconX + 15.f, yPos });
                m_list->m_contentLayer->addChild(starTxt);

                iconX += 70.f;  
            }

        
            if (rewards.tickets > 0) {
                auto tickIcon = CCSprite::create("star_tiket.png"_spr);
                tickIcon->setScale(0.25f);
                tickIcon->setPosition({ iconX, yPos });
                m_list->m_contentLayer->addChild(tickIcon);

                auto tickTxt = CCLabelBMFont::create(fmt::format("x{}", rewards.tickets).c_str(), "bigFont.fnt");
                tickTxt->setScale(0.35f);
                tickTxt->setAnchorPoint({ 0.f, 0.5f });
                tickTxt->setPosition({ iconX + 15.f, yPos });
                m_list->m_contentLayer->addChild(tickTxt);

              
            }
           
            float indicatorX = listSize.width - 30.f;

            if (isPast) {
                auto check = CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
                check->setScale(0.6f);
                check->setPosition({ indicatorX, yPos });
                m_list->m_contentLayer->addChild(check);
            }
            else if (isCurrent) {
              
                auto playerContainer = CCNode::create();
                playerContainer->setPosition({ indicatorX, yPos - 5.f });
                m_list->m_contentLayer->addChild(playerContainer);

               
                float floorWidth = 30.f;
                float floorHeight = 2.f;
                auto floorLine = CCLayerColor::create({ 255, 255, 255, 200 }, floorWidth, floorHeight);

                
                floorLine->setPosition({ -floorWidth / 2, -floorHeight - 6.f });

                playerContainer->addChild(floorLine);
 
                auto playerIcon = SimplePlayer::create(gm->getPlayerFrame());
                playerIcon->updatePlayerFrame(gm->getPlayerFrame(), IconType::Cube);
                playerIcon->setColor(gm->colorForIdx(gm->getPlayerColor()));
                playerIcon->setSecondColor(gm->colorForIdx(gm->getPlayerColor2()));

               
                if (gm->getPlayerGlow()) {
                    playerIcon->setGlowOutline(gm->colorForIdx(gm->getPlayerGlowColor()));
                }
                else {
                    playerIcon->disableGlowOutline();
                }

                playerIcon->updateColors();
                playerIcon->setScale(0.45f);
 
                float scaledHalfHeight = (playerIcon->getContentSize().height * playerIcon->getScale()) / 2;
                playerIcon->setPosition({ 0.f, scaledHalfHeight });

                playerContainer->addChild(playerIcon);

             
                auto jumpAction = CCJumpBy::create(0.7f, { 0.f, 0.f }, 12.f, 1);
                auto delayAction = CCDelayTime::create(0.1f);
                auto sequence = CCSequence::create(jumpAction, delayAction, nullptr);

                playerIcon->runAction(CCRepeatForever::create(sequence));
            }
        }

       
        if (g_streakData.currentLevel > 1) {
            float targetRow = g_streakData.currentLevel - 1;
            float targetY = totalHeight - (targetRow * itemHeight) - (itemHeight / 2);
            float layerY = listSize.height / 2 - targetY;
            float minY = listSize.height - totalHeight;
            if (layerY < minY) layerY = minY;
            if (layerY > 0) layerY = 0;
            m_list->m_contentLayer->setPositionY(layerY);
        }
        else {
            m_list->moveToTop();
        }
        m_mainLayer->addChild(m_list);


     

        float centerX = popupSize.width / 2;
        float barY = bottomHeight / 2 - 5.f;
        float barWidth = 220.0f;
        float barHeight = 15.0f;

        auto outer = CCLayerColor::create({ 0, 0, 0, 100 }, barWidth + 6, barHeight + 6);
        outer->setPosition({ centerX - barWidth / 2 - 3, barY - 3 });
        m_mainLayer->addChild(outer);

        auto border = CCLayerColor::create({ 255, 255, 255, 100 }, barWidth + 2, barHeight + 2);
        border->setPosition({ centerX - barWidth / 2 - 1, barY - 1 });
        m_mainLayer->addChild(border);

        auto bgBar = CCLayerColor::create({ 40, 40, 40, 255 }, barWidth, barHeight);
        bgBar->setPosition({ centerX - barWidth / 2, barY });
        m_mainLayer->addChild(bgBar);

        float percentage = g_streakData.getXPPercentage();
        auto fgBar = CCLayerGradient::create({ 0, 100, 255, 255 }, { 0, 200, 255, 255 });
        fgBar->setContentSize({ barWidth * percentage, barHeight });
        fgBar->setPosition({ centerX - barWidth / 2, barY });
        m_mainLayer->addChild(fgBar);

        auto xpText = CCLabelBMFont::create(
            fmt::format("{} / {} ({:.1f}%)",
                g_streakData.currentXP,
                g_streakData.getXPRequiredForNextLevel(),
                percentage * 100.0f
            ).c_str(),
            "bigFont.fnt"
        );
        xpText->setPosition({ centerX, barY + barHeight / 2 + 2.f });
        xpText->setScale(0.35f);
        xpText->setZOrder(10);

        auto shadow = CCLabelBMFont::create(xpText->getString(), "bigFont.fnt");
        shadow->setPosition({ centerX + 1, barY + barHeight / 2 + 1.f });
        shadow->setScale(0.35f);
        shadow->setColor({ 0,0,0 });
        shadow->setOpacity(150);
        shadow->setZOrder(9);
        m_mainLayer->addChild(shadow);
        m_mainLayer->addChild(xpText);

        auto lvlTxt = CCLabelBMFont::create(fmt::format("Current: Level {}", g_streakData.currentLevel).c_str(), "goldFont.fnt");
        lvlTxt->setScale(0.5f);
        lvlTxt->setPosition({ centerX, barY + barHeight + 15.f });
        m_mainLayer->addChild(lvlTxt);

        return true;
    }

public:
    static XPPopup* create() {
        auto ret = new XPPopup();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};