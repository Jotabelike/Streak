#pragma once
#include "../StreakData.h"
#include <Geode/utils/cocos.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/ScrollLayer.hpp>

using namespace geode::prelude;

class HistoryCell : public cocos2d::CCNode {
protected:
    bool init(const std::string& date, int points, float width) {
        if (!cocos2d::CCNode::init()) return false;

        float height = 30.f;
        this->setContentSize({ width, height });
        this->ignoreAnchorPointForPosition(false);
        this->setAnchorPoint({ 0.5f, 0.5f });

        auto bg = CCLayerColor::create({ 0, 0, 0, 50 }, width, height - 2);
        bg->ignoreAnchorPointForPosition(false);
        bg->setAnchorPoint({ 0.5f, 0.5f });
        bg->setPosition({ width / 2, height / 2 });
        this->addChild(bg);

        auto dateLabel = cocos2d::CCLabelBMFont::create(date.c_str(), "goldFont.fnt");
        dateLabel->setScale(0.5f);
        dateLabel->setAnchorPoint({ 0.0f, 0.5f });
        dateLabel->setPosition({ 10.f, height / 2 });
        this->addChild(dateLabel);

        auto pointsLabel = cocos2d::CCLabelBMFont::create(
            fmt::format("+{} Points", points).c_str(),
            "bigFont.fnt"
        );
        pointsLabel->setScale(0.4f);
        pointsLabel->setAnchorPoint({ 1.0f, 0.5f });
        pointsLabel->setPosition({ width - 10.f, height / 2 });
        pointsLabel->setColor({ 100, 255, 100 });
        this->addChild(pointsLabel);

        return true;
    }

public:
    static HistoryCell* create(const std::string& date, int points, float width) {
        auto ret = new HistoryCell();
        if (ret && ret->init(date, points, width)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

class HistoryPopup : public Popup {
protected:
    std::vector<std::pair<std::string, int>> m_historyEntries;

    bool init() {
        if (!Popup::init(260.f, 220.f, "geode.loader/GE_square03.png")) return false;

        this->setTitle("Points History");

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        auto popupCenter = m_mainLayer->getContentSize() / 2;
        auto listSize = CCSize{ 220.f, 150.f };

        auto bg = cocos2d::extension::CCScale9Sprite::create("square02_001.png");
        bg->setContentSize(listSize);
        bg->setColor({ 0, 0, 0 });
        bg->setOpacity(100);
        bg->setPosition(popupCenter);
        m_mainLayer->addChild(bg);

        g_streakData.load();

        m_historyEntries = std::vector<std::pair<std::string, int>>(
            g_streakData.streakPointsHistory.begin(),
            g_streakData.streakPointsHistory.end());

        std::sort(m_historyEntries.begin(), m_historyEntries.end(),
            [](const auto& a, const auto& b) {
                return a.first > b.first;
            }
        );

        auto scroll = ScrollLayer::create(listSize);
        scroll->setPosition(popupCenter - listSize / 2);

        float cellHeight = 30.f;
        float totalHeight = std::max(listSize.height, (float)m_historyEntries.size() * cellHeight);

        auto contentLayer = scroll->m_contentLayer;
        contentLayer->setContentSize({ listSize.width, totalHeight });

        for (size_t i = 0; i < m_historyEntries.size(); ++i) {
            const auto& [date, points] = m_historyEntries[i];
            auto cell = HistoryCell::create(date, points, listSize.width);

            float yPos = totalHeight - (i * cellHeight) - (cellHeight / 2);
            cell->setPosition({ listSize.width / 2, yPos });
            contentLayer->addChild(cell);
        }

        scroll->moveToTop();
        m_mainLayer->addChild(scroll);

        return true;
    }

public:
    static HistoryPopup* create() {
        auto ret = new HistoryPopup();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};