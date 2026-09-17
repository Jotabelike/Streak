#pragma once
#include "StreakCommon.h"
#include "../StreakData.h"
#include "../NameModifiers.h"
#include "../utils/RoundedProgressBar.h"
#include <Geode/ui/Popup.hpp>
#include <Geode/binding/FLAlertLayer.hpp>

using namespace geode::prelude;

// Full Bronze -> Definitive III ladder shown when the player taps their rank badge.
// The original twelve ranks stay on page one; the three Definitive ranks use page two.
class RankLadderPopup : public Popup {
protected:
    CCNode* m_firstPage = nullptr;
    CCNode* m_definitivePage = nullptr;
    CCMenuItemSpriteExtra* m_prevButton = nullptr;
    CCMenuItemSpriteExtra* m_nextButton = nullptr;
    CCLabelBMFont* m_pageLabel = nullptr;
    int m_page = 0;

    void addRank(CCNode* page, int rankIndex, float cx, float cy, float badgeScale = 0.14f) {
        bool locked = rankIndex > StreakData::getRankIndexForTokens(g_streakData.streakTokens);

        if (auto badge = CCSprite::create(StreakData::getRankSpriteForIndex(rankIndex).c_str())) {
            badge->setScale(badgeScale);
            badge->setPosition({ cx, cy + 1.f });
            if (locked) {
                badge->setColor({ 150, 150, 150 });
                badge->setOpacity(130);
            }
            page->addChild(badge, 1);
        }

        auto nameLabel = CCLabelBMFont::create(
            StreakData::getRankNameForIndex(rankIndex).c_str(), "bigFont.fnt");
        nameLabel->setScale(0.4f);
        nameLabel->setPosition({ cx, cy - 16.f });
        page->addChild(nameLabel, 2);
        NameModifiers::applyColor(nameLabel, StreakData::getRankColorStyleForIndex(rankIndex));
        if (locked) {
            nameLabel->setCascadeOpacityEnabled(true);
            nameLabel->setOpacity(120);
        }

        auto thresholdLabel = CCLabelBMFont::create(
            fmt::format("{} tk", StreakData::getRankThreshold(rankIndex)).c_str(), "bigFont.fnt");
        thresholdLabel->setScale(0.3f);
        thresholdLabel->setColor({ 200, 200, 200 });
        thresholdLabel->setPosition({ cx, cy - 26.f });
        page->addChild(thresholdLabel, 2);
    }

    void updatePage() {
        m_firstPage->setVisible(m_page == 0);
        m_definitivePage->setVisible(m_page == 1);
        m_prevButton->setVisible(m_page > 0);
        m_prevButton->setEnabled(m_page > 0);
        m_nextButton->setVisible(m_page < 1);
        m_nextButton->setEnabled(m_page < 1);
        m_pageLabel->setString(m_page == 0 ? "1 / 2" : "2 / 2");
    }

    void onPreviousPage(CCObject*) {
        m_page = 0;
        updatePage();
    }

    void onNextPage(CCObject*) {
        m_page = 1;
        updatePage();
    }

    bool init() override {
        if (!Popup::init(340.f, 250.f, "geode.loader/GE_square03.png")) return false;
        this->setTitle("Rank Ladder");
        auto winSize = this->m_mainLayer->getContentSize();

        m_firstPage = CCNode::create();
        m_firstPage->setContentSize(winSize);
        m_mainLayer->addChild(m_firstPage);

        m_definitivePage = CCNode::create();
        m_definitivePage->setContentSize(winSize);
        m_mainLayer->addChild(m_definitivePage);

        float colX[3] = { 57.f, 170.f, 283.f };
        float rowTopY = winSize.height - 52.f;
        float rowStep = 49.f;

        for (int i = 0; i < 12; ++i) {
            int col = i % 3;
            int row = i / 3;
            addRank(m_firstPage, i, colX[col], rowTopY - row * rowStep);
        }

        // The final tier gets its own clean page and slightly larger emblems.
        for (int i = 12; i < StreakData::RANK_COUNT; ++i) {
            addRank(m_definitivePage, i, colX[i - 12], winSize.height / 2.f + 20.f, 0.21f);
        }

        auto arrowMenu = CCMenu::create();
        arrowMenu->setPosition({ 0.f, 0.f });
        m_mainLayer->addChild(arrowMenu, 20);

        auto previousSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        previousSprite->setScale(0.55f);
        m_prevButton = CCMenuItemSpriteExtra::create(
            previousSprite, this, menu_selector(RankLadderPopup::onPreviousPage));
        m_prevButton->setPosition({ -18.f, winSize.height / 2.f });
        arrowMenu->addChild(m_prevButton);

        auto nextSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        nextSprite->setFlipX(true);
        nextSprite->setScale(0.55f);
        m_nextButton = CCMenuItemSpriteExtra::create(
            nextSprite, this, menu_selector(RankLadderPopup::onNextPage));
        m_nextButton->setPosition({ winSize.width + 18.f, winSize.height / 2.f });
        arrowMenu->addChild(m_nextButton);

        m_pageLabel = CCLabelBMFont::create("1 / 2", "bigFont.fnt");
        m_pageLabel->setScale(0.32f);
        m_pageLabel->setColor({ 190, 205, 230 });
        m_pageLabel->setPosition({ winSize.width / 2.f, 9.f });
        m_mainLayer->addChild(m_pageLabel, 20);

        updatePage();

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
            "climbing from <co>Bronze</c> all the way to <cr>Definitive III</c>.\n\n"
            "<cr>Each season the ladder soft-resets</c> based on your rank:\n"
            "<co>Bronze:</c> keeps all tokens\n"
            "<cl>Platinum:</c> -800 tokens\n"
            "<cy>Gold:</c> -2000 tokens\n"
            "<cb>Diamond:</c> -4000 tokens\n"
            "<cr>Definitive:</c> -8000 tokens";
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
