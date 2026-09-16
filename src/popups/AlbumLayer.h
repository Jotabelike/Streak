#pragma once

#include "../StreakData.h"
#include <Geode/Geode.hpp>
#include <algorithm>
#include <vector>

using namespace geode::prelude;

class StreakAlbumLayer : public CCLayer {
protected:
    struct AlbumEntry {
        std::string date;
        int points = 0;
        int streakDay = 0;
    };

    CCSprite* m_album = nullptr;
    CCNode* m_stampLayer = nullptr;
    CCMenu* m_stampMenu = nullptr;
    CCMenuItemSpriteExtra* m_previousButton = nullptr;
    CCMenuItemSpriteExtra* m_nextButton = nullptr;
    CCLabelBMFont* m_pageLabel = nullptr;
    std::vector<AlbumEntry> m_entries;
    int m_currentSpread = 0;
    int m_totalSpreads = 1;

    static std::string shortDate(const std::string& date) {
        if (date.size() >= 10) {
            return fmt::format("{}/{}", date.substr(5, 2), date.substr(8, 2));
        }
        return date;
    }

    void loadCompletedDays() {
        m_entries.clear();

        std::vector<std::pair<std::string, int>> history;
        history.reserve(g_streakData.streakPointsHistory.size());
        for (const auto& entry : g_streakData.streakPointsHistory) {
            history.push_back(entry);
        }

        // History receives today's points immediately, before the streak is
        // necessarily completed. Keeping at most currentStreak entries prevents
        // an unfinished day from receiving its sticker early.
        size_t completedCount = std::min(
            history.size(),
            static_cast<size_t>(std::max(0, g_streakData.currentStreak))
        );
        history.resize(completedCount);

        int firstRecordedDay = std::max(
            1,
            g_streakData.currentStreak - static_cast<int>(history.size()) + 1
        );

        for (size_t index = 0; index < history.size(); ++index) {
            m_entries.push_back({
                history[index].first,
                history[index].second,
                firstRecordedDay + static_cast<int>(index)
            });
        }

        m_totalSpreads = std::max(1, static_cast<int>((m_entries.size() + 7) / 8));
        m_currentSpread = m_totalSpreads - 1;
    }

    void onStampPressed(CCObject* sender) {
        auto button = static_cast<CCMenuItemSpriteExtra*>(sender);
        int entryIndex = button->getTag();
        if (entryIndex < 0 || entryIndex >= static_cast<int>(m_entries.size())) return;

        const auto& entry = m_entries[entryIndex];
        auto title = fmt::format("Streak Day {}", entry.streakDay);
        auto message = fmt::format(
            "<cy>Date:</c> {}\n<co>Points earned:</c> +{} pts",
            shortDate(entry.date),
            entry.points
        );
        FLAlertLayer::create(title.c_str(), message, "OK")->show();
    }

    void addStamp(const AlbumEntry& entry, size_t entryIndex, const CCPoint& position) {
        CCSprite* stamp = nullptr;
        switch (std::max(0, entry.streakDay - 1) % 4) {
            case 1:
                stamp = CCSprite::create("streak_album_stamp_2.png"_spr);
                break;
            case 2:
                stamp = CCSprite::create("streak_album_stamp_3.png"_spr);
                break;
            case 3:
                stamp = CCSprite::create("streak_album_stamp_4.png"_spr);
                break;
            default:
                stamp = CCSprite::create("streak_album_stamp.png"_spr);
                break;
        }
        if (!stamp) return;

        auto albumSize = m_album->getContentSize();
        float logicalScale = albumSize.height / 1024.f;
        float stampSize = albumSize.height * (232.f / 1024.f);
        auto baseSize = stamp->getContentSize();
        float stampScale = stampSize / std::max(baseSize.width, baseSize.height);

        auto stampContent = CCNode::create();
        stampContent->setContentSize({ stampSize, stampSize });
        stampContent->setAnchorPoint({ 0.5f, 0.5f });

        stamp->setPosition({ stampSize / 2.f, stampSize / 2.f });
        stamp->setScale(stampScale);
        stampContent->addChild(stamp, 1);

        auto dayLabel = CCLabelBMFont::create(
            fmt::format("Day {}", entry.streakDay).c_str(),
            "goldFont.fnt"
        );
        dayLabel->setScale(1.25f * logicalScale);
        dayLabel->setPosition({
            stampSize / 2.f,
            stampSize / 2.f - albumSize.height * (78.f / 1024.f)
        });
        stampContent->addChild(dayLabel, 4);

        auto streakSprite = CCSprite::create(
            g_streakData.getRachaSprite(entry.streakDay).c_str()
        );
        if (streakSprite) {
            auto streakSize = streakSprite->getContentSize();
            float maxSide = std::max(streakSize.width, streakSize.height);
            float targetSide = albumSize.height * (112.f / 1024.f);
            streakSprite->setScale(maxSide > 0.f ? targetSide / maxSide : 0.15f);
            streakSprite->setPosition({
                stampSize / 2.f,
                stampSize / 2.f + albumSize.height * (12.f / 1024.f)
            });
            stampContent->addChild(streakSprite, 3);
        }

        auto stampButton = CCMenuItemSpriteExtra::create(
            stampContent,
            this,
            menu_selector(StreakAlbumLayer::onStampPressed)
        );
        stampButton->setTag(static_cast<int>(entryIndex));
        stampButton->setPosition(position);
        stampButton->useAnimationType(MenuAnimationType::Scale);
        m_stampMenu->addChild(stampButton, 1);
    }

    void updateNavigation() {
        if (m_previousButton) m_previousButton->setVisible(m_currentSpread > 0);
        if (m_nextButton) m_nextButton->setVisible(m_currentSpread < m_totalSpreads - 1);
        if (m_pageLabel) {
            m_pageLabel->setVisible(m_totalSpreads > 1);
            m_pageLabel->setString(
                fmt::format("{} / {}", m_currentSpread + 1, m_totalSpreads).c_str()
            );
        }
    }

    void renderSpread() {
        if (!m_stampLayer || !m_stampMenu || !m_album) return;
        m_stampLayer->removeAllChildren();
        m_stampMenu->removeAllChildren();

        if (m_entries.empty()) {
            auto emptyLabel = CCLabelBMFont::create(
                "No completed\nstreak days yet.",
                "bigFont.fnt"
            );
            emptyLabel->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
            emptyLabel->setScale(
                1.15f * (m_album->getContentSize().height / 1024.f)
            );
            emptyLabel->setColor({ 120, 70, 25 });
            emptyLabel->setPosition({
                -m_album->getContentSize().width * 0.25f,
                0.f
            });
            m_stampLayer->addChild(emptyLabel, 2);
            updateNavigation();
            return;
        }

        static const CCPoint normalizedSlots[8] = {
            { 250.f / 1536.f, 660.f / 1024.f }, { 520.f / 1536.f, 660.f / 1024.f },
            { 250.f / 1536.f, 365.f / 1024.f }, { 520.f / 1536.f, 365.f / 1024.f },
            { 1016.f / 1536.f, 660.f / 1024.f }, { 1286.f / 1536.f, 660.f / 1024.f },
            { 1016.f / 1536.f, 365.f / 1024.f }, { 1286.f / 1536.f, 365.f / 1024.f }
        };

        size_t start = static_cast<size_t>(m_currentSpread) * 8;
        size_t end = std::min(start + 8, m_entries.size());
        auto albumSize = m_album->getContentSize();
        for (size_t index = start; index < end; ++index) {
            auto slot = normalizedSlots[index - start];
            addStamp(m_entries[index], index, {
                (slot.x - 0.5f) * albumSize.width,
                (slot.y - 0.5f) * albumSize.height
            });
        }

        updateNavigation();
    }

    void onPreviousSpread(CCObject*) {
        if (m_currentSpread <= 0) return;
        --m_currentSpread;
        renderSpread();
    }

    void onNextSpread(CCObject*) {
        if (m_currentSpread >= m_totalSpreads - 1) return;
        ++m_currentSpread;
        renderSpread();
    }

    bool init() override {
        if (!CCLayer::init()) return false;

        g_streakData.load();
        loadCompletedDays();

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        this->setContentSize(winSize);
        this->setTouchEnabled(true);
        this->setKeypadEnabled(true);

        auto dimmer = CCLayerColor::create({ 0, 0, 0, 0 }, winSize.width, winSize.height);
        dimmer->runAction(CCFadeTo::create(0.2f, 150));
        this->addChild(dimmer, 0);

        m_album = CCSprite::create("streak_album_open.png"_spr);
        if (!m_album) return false;

        auto albumSize = m_album->getContentSize();
        float maxWidth = winSize.width - 64.f;
        float maxHeight = winSize.height - 28.f;
        float albumScale = std::min(
            maxWidth / albumSize.width,
            maxHeight / albumSize.height
        );

        m_album->setPosition(winSize / 2.f);
        m_album->setScale(albumScale * 0.82f);
        m_album->setID("album-sprite");
        this->addChild(m_album, 1);

        m_stampLayer = CCNode::create();
        m_stampLayer->setContentSize(albumSize);
        m_stampLayer->setPosition(winSize / 2.f);
        m_stampLayer->setScale(albumScale * 0.82f);
        this->addChild(m_stampLayer, 3);

        m_stampMenu = CCMenu::create();
        m_stampMenu->setContentSize(albumSize);
        m_stampMenu->setAnchorPoint({ 0.f, 0.f });
        m_stampMenu->setPosition(winSize / 2.f);
        m_stampMenu->setScale(albumScale * 0.82f);
        m_stampMenu->setTouchPriority(-514);
        this->addChild(m_stampMenu, 2);

        m_album->runAction(CCEaseBackOut::create(
            CCScaleTo::create(0.28f, albumScale)
        ));
        m_stampLayer->runAction(CCEaseBackOut::create(
            CCScaleTo::create(0.28f, albumScale)
        ));
        m_stampMenu->runAction(CCEaseBackOut::create(
            CCScaleTo::create(0.28f, albumScale)
        ));

        auto navigationMenu = CCMenu::create();
        navigationMenu->setPosition({ 0.f, 0.f });
        navigationMenu->setTouchPriority(-514);
        this->addChild(navigationMenu, 4);

        float albumWidth = albumSize.width * albumScale;
        float albumHeight = albumSize.height * albumScale;

        auto previousSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        if (previousSprite) {
            previousSprite->setScale(0.62f);
            m_previousButton = CCMenuItemSpriteExtra::create(
                previousSprite,
                this,
                menu_selector(StreakAlbumLayer::onPreviousSpread)
            );
            m_previousButton->setPosition({
                winSize.width / 2.f - albumWidth / 2.f + 18.f,
                winSize.height / 2.f
            });
            navigationMenu->addChild(m_previousButton);
        }

        auto nextSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        if (nextSprite) {
            nextSprite->setFlipX(true);
            nextSprite->setScale(0.62f);
            m_nextButton = CCMenuItemSpriteExtra::create(
                nextSprite,
                this,
                menu_selector(StreakAlbumLayer::onNextSpread)
            );
            m_nextButton->setPosition({
                winSize.width / 2.f + albumWidth / 2.f - 18.f,
                winSize.height / 2.f
            });
            navigationMenu->addChild(m_nextButton);
        }

        m_pageLabel = CCLabelBMFont::create("", "goldFont.fnt");
        m_pageLabel->setScale(0.38f);
        m_pageLabel->setPosition({
            winSize.width / 2.f,
            winSize.height / 2.f - albumHeight / 2.f + 12.f
        });
        this->addChild(m_pageLabel, 3);

        auto closeSprite = CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png");
        if (closeSprite) {
            closeSprite->setScale(0.8f);
            auto closeButton = CCMenuItemSpriteExtra::create(
                closeSprite,
                this,
                menu_selector(StreakAlbumLayer::onClose)
            );

            closeButton->setPosition({
                winSize.width / 2.f + albumWidth / 2.f - 5.f,
                winSize.height / 2.f + albumHeight / 2.f - 5.f
            });

            auto closeMenu = CCMenu::createWithItem(closeButton);
            closeMenu->setPosition({ 0.f, 0.f });
            closeMenu->setTouchPriority(-514);
            this->addChild(closeMenu, 2);
        }

        renderSpread();

        return true;
    }

    void onClose(CCObject*) {
        this->removeFromParentAndCleanup(true);
    }

public:
    void onEnter() override {
        CCLayer::onEnter();
        CCDirector::sharedDirector()->getTouchDispatcher()->addTargetedDelegate(this, -513, true);
    }

    void onExit() override {
        CCDirector::sharedDirector()->getTouchDispatcher()->removeDelegate(this);
        CCLayer::onExit();
    }

    bool ccTouchBegan(CCTouch*, CCEvent*) override {
        return true;
    }

    void ccTouchMoved(CCTouch*, CCEvent*) override {}
    void ccTouchEnded(CCTouch*, CCEvent*) override {}
    void ccTouchCancelled(CCTouch*, CCEvent*) override {}

    void keyBackClicked() override {
        onClose(nullptr);
    }

    static StreakAlbumLayer* create() {
        auto ret = new StreakAlbumLayer();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};
