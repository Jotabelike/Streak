#pragma once
#include "../StreakData.h"
#include <Geode/utils/cocos.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/ScrollLayer.hpp>
#include <ctime>
#include <iomanip>
#include <sstream>

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
    std::map<std::string, std::vector<std::pair<std::string, int>>> m_weeklyGroups;
    std::vector<std::string> m_orderedWeekKeys;
    std::string m_currentWeekKey;
    ScrollLayer* m_scrollLayer = nullptr;
    CCMenu* m_backMenu = nullptr;

    static std::string mondayOf(const std::string& dateStr) {
        std::tm tm = {};
        std::istringstream ss(dateStr);
        ss >> std::get_time(&tm, "%Y-%m-%d");
        if (ss.fail()) return dateStr;
        std::time_t t = std::mktime(&tm);
        if (t == (std::time_t)-1) return dateStr;
        std::tm* now = std::localtime(&t);
        if (!now) return dateStr;
        int wday = now->tm_wday;
        int offset = (wday == 0) ? 6 : (wday - 1);
        std::time_t monday = t - (std::time_t)offset * 86400;
        std::tm* mtm = std::localtime(&monday);
        if (!mtm) return dateStr;
        char buf[16];
        if (std::strftime(buf, sizeof(buf), "%Y-%m-%d", mtm) == 0) return dateStr;
        return std::string(buf);
    }

    static std::string formatWeekRange(const std::string& mondayStr) {
        std::tm tm = {};
        std::istringstream ss(mondayStr);
        ss >> std::get_time(&tm, "%Y-%m-%d");
        if (ss.fail()) return mondayStr;
        std::time_t mon = std::mktime(&tm);
        if (mon == (std::time_t)-1) return mondayStr;
        std::time_t sun = mon + 6 * 86400;
        std::tm* sunTm = std::localtime(&sun);
        if (!sunTm) return mondayStr;
        char sunBuf[16];
        if (std::strftime(sunBuf, sizeof(sunBuf), "%m-%d", sunTm) == 0) return mondayStr;
        return fmt::format("{} - {}", mondayStr.substr(5), std::string(sunBuf));
    }

    void buildGroups() {
        m_weeklyGroups.clear();
        m_orderedWeekKeys.clear();
        for (const auto& [date, points] : g_streakData.streakPointsHistory) {
            std::string weekKey = mondayOf(date);
            m_weeklyGroups[weekKey].push_back({ date, points });
        }
        for (auto& [_, list] : m_weeklyGroups) {
            std::sort(list.begin(), list.end(),
                [](const auto& a, const auto& b) { return a.first > b.first; });
        }
        for (const auto& [k, _] : m_weeklyGroups) m_orderedWeekKeys.push_back(k);
        std::sort(m_orderedWeekKeys.begin(), m_orderedWeekKeys.end(),
            [](const std::string& a, const std::string& b) { return a > b; });
    }

    bool init() override {
        if (!Popup::init(260.f, 220.f, "geode.loader/GE_square03.png")) return false;

        this->setTitle("Points History");

        auto popupCenter = m_mainLayer->getContentSize() / 2;
        auto listSize = CCSize{ 220.f, 150.f };

        auto bg = cocos2d::extension::CCScale9Sprite::create("square02_001.png");
        bg->setContentSize(listSize);
        bg->setColor({ 0, 0, 0 });
        bg->setOpacity(100);
        bg->setPosition(popupCenter);
        m_mainLayer->addChild(bg);

        g_streakData.load();
        this->buildGroups();
        this->refreshList();

        return true;
    }

    void refreshList() {
        if (m_scrollLayer) { m_scrollLayer->removeFromParent(); m_scrollLayer = nullptr; }
        if (m_backMenu)    { m_backMenu->removeFromParent();    m_backMenu = nullptr; }

        auto popupCenter = m_mainLayer->getContentSize() / 2;
        auto listSize = CCSize{ 220.f, 150.f };

        m_scrollLayer = ScrollLayer::create(listSize);
        m_scrollLayer->setPosition(popupCenter - listSize / 2);

        if (m_currentWeekKey.empty()) {
            this->buildFolderView(listSize);
        } else {
            this->buildWeekView(listSize);
        }

        m_scrollLayer->moveToTop();
        m_mainLayer->addChild(m_scrollLayer);
    }

    void buildFolderView(const CCSize& listSize) {
        float cellHeight = 50.f;
        size_t count = m_orderedWeekKeys.size();
        float totalHeight = std::max(listSize.height, (float)count * cellHeight);

        auto contentLayer = m_scrollLayer->m_contentLayer;
        contentLayer->setContentSize({ listSize.width, totalHeight });

        if (count == 0) {
            auto emptyLabel = CCLabelBMFont::create("No history yet.", "bigFont.fnt");
            emptyLabel->setScale(0.45f);
            emptyLabel->setPosition({ listSize.width / 2, totalHeight / 2 });
            contentLayer->addChild(emptyLabel);
            return;
        }

        for (size_t i = 0; i < count; ++i) {
            const auto& weekKey = m_orderedWeekKeys[i];

            auto cell = CCNode::create();
            cell->setContentSize({ listSize.width, cellHeight });
            cell->ignoreAnchorPointForPosition(false);
            cell->setAnchorPoint({ 0.5f, 0.5f });

            auto bg = CCLayerColor::create({ 0, 0, 0, 50 }, listSize.width, cellHeight - 2);
            bg->ignoreAnchorPointForPosition(false);
            bg->setAnchorPoint({ 0.5f, 0.5f });
            bg->setPosition({ listSize.width / 2, cellHeight / 2 });
            cell->addChild(bg);

            auto folderSpr = CCSprite::create("streak_folder.png"_spr);
            if (folderSpr) {
                folderSpr->setScale(0.3f);
                auto btn = CCMenuItemSpriteExtra::create(
                    folderSpr, this, menu_selector(HistoryPopup::onFolderClick)
                );
                btn->setUserObject(CCString::create(weekKey));
                auto menu = CCMenu::create();
                menu->setPosition({ 28.f, cellHeight / 2 });
                menu->addChild(btn);
                cell->addChild(menu);
            }

            auto rangeLabel = CCLabelBMFont::create(
                formatWeekRange(weekKey).c_str(), "goldFont.fnt"
            );
            rangeLabel->setScale(0.45f);
            rangeLabel->setAnchorPoint({ 0.0f, 0.5f });
            rangeLabel->setPosition({ 55.f, cellHeight / 2 + 7.f });
            cell->addChild(rangeLabel);

            int total = 0;
            for (auto& [_, p] : m_weeklyGroups[weekKey]) total += p;

            auto subLabel = CCLabelBMFont::create(
                fmt::format("{} day(s)  +{} pts",
                    (int)m_weeklyGroups[weekKey].size(), total).c_str(),
                "chatFont.fnt"
            );
            subLabel->setScale(0.5f);
            subLabel->setAnchorPoint({ 0.0f, 0.5f });
            subLabel->setColor({ 200, 200, 200 });
            subLabel->setPosition({ 55.f, cellHeight / 2 - 8.f });
            cell->addChild(subLabel);

            float yPos = totalHeight - ((float)i * cellHeight) - (cellHeight / 2);
            cell->setPosition({ listSize.width / 2, yPos });
            contentLayer->addChild(cell);
        }
    }

    void buildWeekView(const CCSize& listSize) {
        const auto& entries = m_weeklyGroups[m_currentWeekKey];
        float cellHeight = 30.f;
        float totalHeight = std::max(listSize.height, (float)entries.size() * cellHeight);

        auto contentLayer = m_scrollLayer->m_contentLayer;
        contentLayer->setContentSize({ listSize.width, totalHeight });

        for (size_t i = 0; i < entries.size(); ++i) {
            const auto& [date, points] = entries[i];
            auto cell = HistoryCell::create(date, points, listSize.width);
            float yPos = totalHeight - ((float)i * cellHeight) - (cellHeight / 2);
            cell->setPosition({ listSize.width / 2, yPos });
            contentLayer->addChild(cell);
        }

        auto backSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
        if (backSpr) {
            backSpr->setScale(0.7f);
            auto backBtn = CCMenuItemSpriteExtra::create(
                backSpr, this, menu_selector(HistoryPopup::onBack)
            );
            m_backMenu = CCMenu::createWithItem(backBtn);
            m_backMenu->setPosition({ 22.f, m_mainLayer->getContentSize().height - 22.f });
            m_mainLayer->addChild(m_backMenu, 10);
        }
    }

    void onFolderClick(CCObject* sender) {
        auto node = static_cast<CCNode*>(sender);
        if (auto key = static_cast<CCString*>(node->getUserObject())) {
            m_currentWeekKey = key->getCString();
            this->refreshList();
        }
    }

    void onBack(CCObject*) {
        m_currentWeekKey = "";
        this->refreshList();
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
