#pragma once
#include "StreakCommon.h"
#include "../StreakData.h"
#include "../NameModifiers.h"
#include "../utils/RoundedProgressBar.h"
#include <Geode/ui/Popup.hpp>
#include <Geode/binding/FLAlertLayer.hpp>

using namespace geode::prelude;

// Full Bronze -> Diamond III ladder shown when the player taps their rank badge.
// Laid out as a 3-column x 4-row grid of cards.
class RankLadderPopup : public Popup {
protected:
    bool init() override {
        if (!Popup::init(340.f, 250.f, "geode.loader/GE_square03.png")) return false;
        this->setTitle("Rank Ladder");
        auto winSize = this->m_mainLayer->getContentSize();

        int currentIdx = StreakData::getRankIndexForTokens(g_streakData.streakTokens);

        float colX[3] = { 57.f, 170.f, 283.f };
        float rowTopY = winSize.height - 52.f;
        float rowStep = 49.f;

        for (int i = 0; i < StreakData::RANK_COUNT; ++i) {
            int col = i % 3;
            int row = i / 3;
            float cx = colX[col];
            float cy = rowTopY - row * rowStep;
            bool locked = (i > currentIdx);

            if (auto badge = CCSprite::create(StreakData::getRankSpriteForIndex(i).c_str())) {
                badge->setScale(0.14f); // ~65px
                badge->setPosition({ cx, cy + 1.f });
                if (locked) { badge->setColor({ 150, 150, 150 }); badge->setOpacity(130); }
                m_mainLayer->addChild(badge, 1);
            }

            auto nameLbl = CCLabelBMFont::create(
                StreakData::getRankNameForIndex(i).c_str(), "bigFont.fnt");
            nameLbl->setScale(0.4f);
            nameLbl->setPosition({ cx, cy - 16.f });
            m_mainLayer->addChild(nameLbl, 2);
            // Animated tier color for every rank name; locked ranks just dimmed.
            NameModifiers::applyColor(nameLbl, StreakData::getRankColorStyleForIndex(i));
            if (locked) {
                nameLbl->setCascadeOpacityEnabled(true);
                nameLbl->setOpacity(120);
            }

            auto thrLbl = CCLabelBMFont::create(
                fmt::format("{} tk", StreakData::getRankThreshold(i)).c_str(), "bigFont.fnt");
            thrLbl->setScale(0.3f);
            thrLbl->setColor({ 200, 200, 200 });
            thrLbl->setPosition({ cx, cy - 26.f });
            m_mainLayer->addChild(thrLbl, 2);
        }

        return true;
    }

public:
    static RankLadderPopup* create() {
        auto ret = new RankLadderPopup();
        if (ret && ret->init()) { ret->autorelease(); return ret; }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

// Compact individual rank progression: big current rank badge (tap for the full
// ladder), rank name, progress bar toward the next rank, and the streak-token
// total in the top corner.
class RankPopup : public Popup {
protected:
    void onInfo(CCObject*) {
        std::string text =
            "<cy>Streak Tokens</c> are earned only when your <cg>daily streak advances</c> "
            "(one grant per day). The more tokens you hold, the higher your rank, "
            "climbing from <co>Bronze</c> all the way to <cl>Diamond III</c>.\n\n"
            "<cr>Each season the ladder soft-resets</c> based on your rank:\n"
            "<co>Bronze:</c> keeps all tokens\n"
            "<cl>Platinum:</c> -800 tokens\n"
            "<cy>Gold:</c> -2000 tokens\n"
            "<cb>Diamond:</c> -4000 tokens";
        FLAlertLayer::create("Rank Ladder", text, "OK")->show();
    }

    void onShowAllRanks(CCObject*) {
        RankLadderPopup::create()->show();
    }

    bool init() override {
        if (!Popup::init(250.f, 205.f, "geode.loader/GE_square03.png")) return false;
        this->setTitle("Your Rank");
        auto winSize = this->m_mainLayer->getContentSize();

        int tokens = g_streakData.streakTokens;
        int rankIdx = StreakData::getRankIndexForTokens(tokens);

        // Info button (top-right)
        auto infoSpr = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
        infoSpr->setScale(0.55f);
        auto infoBtn = CCMenuItemSpriteExtra::create(infoSpr, this, menu_selector(RankPopup::onInfo));
        auto infoMenu = CCMenu::createWithItem(infoBtn);
        infoMenu->setPosition({ winSize.width - 17.f, winSize.height - 17.f });
        m_mainLayer->addChild(infoMenu);

        // Big current rank badge (tap to open the full ladder)
        auto badge = CCSprite::create(StreakData::getRankSpriteForIndex(rankIdx).c_str());
        if (badge) {
            badge->setScale(0.5f); // content ~467px tall -> ~233px

            if (auto shine = CCSprite::createWithSpriteFrameName("shineBurst_001.png")) {
                shine->setColor({ 255, 220, 120 });
                shine->setOpacity(90);
                shine->setScale(1.7f);
                shine->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });
                shine->setPosition(badge->getContentSize() / 2.f);
                shine->runAction(CCRepeatForever::create(CCRotateBy::create(6.0f, 360.f)));
                badge->addChild(shine, -1);
            }

            auto badgeBtn = CCMenuItemSpriteExtra::create(
                badge, this, menu_selector(RankPopup::onShowAllRanks));
            auto badgeMenu = CCMenu::createWithItem(badgeBtn);
            badgeMenu->setPosition({ winSize.width / 2, winSize.height - 80.f });
            m_mainLayer->addChild(badgeMenu);
        }

        // Rank name (on top of the badge's lower area), with animated tier color
        auto rankName = CCLabelBMFont::create(
            StreakData::getRankNameForIndex(rankIdx).c_str(), "bigFont.fnt");
        rankName->setScale(0.8f);
        rankName->setPosition({ winSize.width / 2, 60.f });
        m_mainLayer->addChild(rankName, 5);
        NameModifiers::applyColor(rankName, StreakData::getRankColorStyle(tokens));

        // Token total, centered below the rank name
        {
            auto tokenLabel = CCLabelBMFont::create(std::to_string(tokens).c_str(), "goldFont.fnt");
            tokenLabel->setScale(0.55f);
            float w = tokenLabel->getContentSize().width * tokenLabel->getScale();
            auto tokenIcon = CCSprite::create("streak_token.png"_spr);
            float iconScale = 0.08f;
            float iconW = tokenIcon ? tokenIcon->getContentSize().width * iconScale : 0.f;
            float totalW = iconW + w;
            float startX = winSize.width / 2 - totalW / 2.f;
            float y = 42.f;

            if (tokenIcon) {
                tokenIcon->setScale(iconScale);
                tokenIcon->setAnchorPoint({ 0.f, 0.5f }); // center vertically with the label
                tokenIcon->setPosition({ startX, y });
                m_mainLayer->addChild(tokenIcon, 6);
            }
            tokenLabel->setAnchorPoint({ 0.f, 0.5f });
            tokenLabel->setPosition({ startX + iconW + 2.f, y });
            m_mainLayer->addChild(tokenLabel, 6);
        }

        // Progress bar toward the next rank
        int nextThreshold = StreakData::getNextRankThreshold(tokens);
        float progress = 1.0f;
        std::string progressTxt;
        if (nextThreshold < 0) {
            progressTxt = "MAX RANK";
        }
        else {
            int curThreshold = StreakData::getRankThreshold(rankIdx);
            int span = std::max(1, nextThreshold - curThreshold);
            progress = (float)(tokens - curThreshold) / (float)span;
            progressTxt = fmt::format("{} / {}", tokens, nextThreshold);
        }

        auto progressBar = RoundedProgressBar::create(215.f, 17.f);
        progressBar->setPosition({ winSize.width / 2, 20.f });
        progressBar->setProgress(progress);
        progressBar->setGradientColors({ 255, 200, 60 }, { 255, 240, 150 });
        m_mainLayer->addChild(progressBar);

        auto progressLabel = CCLabelBMFont::create(progressTxt.c_str(), "bigFont.fnt");
        progressLabel->setScale(0.4f);
        progressLabel->setPosition({ winSize.width / 2, 20.f });
        m_mainLayer->addChild(progressLabel, 2);

        return true;
    }

public:
    static RankPopup* create() {
        auto ret = new RankPopup();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};
