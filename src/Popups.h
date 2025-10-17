#pragma once
#include "FirebaseManager.h"
#include "StreakData.h"
#include <Geode/ui/Popup.hpp>
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/ui/ListView.hpp>
#include <Geode/ui/ScrollLayer.hpp>
#include <Geode/cocos/particle_nodes/CCParticleSystemQuad.h>
#include <Geode/cocos/extensions/cocos-ext.h>
#include <Geode/ui/TextInput.hpp>

class ShopPopup;



enum class RewardType { Badge, SuperStar, StarTicket };

struct RoulettePrize {
    RewardType type;
    std::string id;
    int quantity;
    std::string spriteName;
    std::string displayName;
    int probabilityWeight;
    StreakData::BadgeCategory category;
};

struct GenericPrizeResult {
    RewardType type;
    std::string id;
    int quantity;
    std::string displayName;
    std::string spriteName;
    StreakData::BadgeCategory category;
    bool isNew = false;
    int ticketsFromDuplicate = 0;
};

ccColor3B getBrightQualityColor(StreakData::BadgeCategory category) {
    switch (category) {
    case StreakData::BadgeCategory::COMMON:   return ccc3(220, 220, 220);
    case StreakData::BadgeCategory::SPECIAL:  return ccc3(0, 255, 80);
    case StreakData::BadgeCategory::EPIC:     return ccc3(255, 0, 255);
    case StreakData::BadgeCategory::LEGENDARY:return ccc3(255, 200, 0);
    case StreakData::BadgeCategory::MYTHIC:   return ccc3(255, 60, 60);
    default:                                  return ccc3(255, 255, 255);
    }
}

class HistoryCell : public cocos2d::CCLayer {
protected:
    bool init(const std::string& date, int points) {
        if (!cocos2d::CCLayer::init()) return false;
        this->setContentSize({ 280.f, 25.f });
        float cellHeight = this->getContentSize().height;

        auto dateLabel = cocos2d::CCLabelBMFont::create(date.c_str(), "goldFont.fnt");
        dateLabel->setScale(0.5f);
        dateLabel->setAnchorPoint({ 0.0f, 0.5f });
        dateLabel->setPosition({ 10.f, cellHeight / 2 });
        this->addChild(dateLabel);

        auto pointsLabel = cocos2d::CCLabelBMFont::create(std::to_string(points).c_str(), "bigFont.fnt");
        pointsLabel->setScale(0.4f);
        pointsLabel->setAnchorPoint({ 1.0f, 0.5f });
        pointsLabel->setPosition({ this->getContentSize().width - 10.f, cellHeight / 2 });
        this->addChild(pointsLabel);

        auto pointIcon = cocos2d::CCSprite::create("streak_point.png"_spr);
        pointIcon->setScale(0.15f);
        pointIcon->setPosition({ pointsLabel->getPositionX() - pointsLabel->getScaledContentSize().width - 5.f, cellHeight / 2 });
        this->addChild(pointIcon);

        return true;
    }
public:
    static HistoryCell* create(const std::string& date, int points) {
        auto ret = new HistoryCell();
        if (ret && ret->init(date, points)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

class HistoryPopup : public Popup<> {
protected:
    std::vector<std::pair<std::string, int>> m_historyEntries;
    int m_currentPage = 0;
    int m_totalPages = 0;
    const int m_itemsPerPage = 8;
    CCLayer* m_pageContainer;
    CCMenuItemSpriteExtra* m_leftArrow;
    CCMenuItemSpriteExtra* m_rightArrow;

    bool setup() override {
        this->setTitle("Streak Point History");
        g_streakData.load();
        auto listSize = CCSize{ 280.f, 200.f };
        auto popupCenter = m_mainLayer->getContentSize() / 2;
        auto listBg = cocos2d::extension::CCScale9Sprite::create("square02_001.png");
        listBg->setContentSize(listSize);
        listBg->setColor({ 0, 0, 0 });
        listBg->setOpacity(100);
        listBg->setPosition(popupCenter);
        m_mainLayer->addChild(listBg);
        m_historyEntries = std::vector<std::pair<std::string, int>>(g_streakData.streakPointsHistory.begin(), g_streakData.streakPointsHistory.end());
        std::sort(m_historyEntries.begin(), m_historyEntries.end(), [](const auto& a, const auto& b) {
            return a.first < b.first;
            });
        m_totalPages = static_cast<int>(ceil(static_cast<float>(m_historyEntries.size()) / m_itemsPerPage));
        m_currentPage = (m_totalPages > 0) ? (m_totalPages - 1) : 0;
        m_pageContainer = CCLayer::create();
        m_pageContainer->setPosition(popupCenter - listSize / 2);
        m_mainLayer->addChild(m_pageContainer);

        auto leftSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
        m_leftArrow = CCMenuItemSpriteExtra::create(leftSpr, this, menu_selector(HistoryPopup::onPrevPage));
        auto rightSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
        rightSpr->setFlipX(true);
        m_rightArrow = CCMenuItemSpriteExtra::create(rightSpr, this, menu_selector(HistoryPopup::onNextPage));


        CCArray* navItems = CCArray::create();
        navItems->addObject(m_leftArrow);
        navItems->addObject(m_rightArrow);
        auto navMenu = CCMenu::createWithArray(navItems);

        navMenu->alignItemsHorizontallyWithPadding(listSize.width + 25.f);
        navMenu->setPosition(popupCenter);
        m_mainLayer->addChild(navMenu);
        this->updatePage();
        return true;
    }

    void updatePage() {
        this->setTitle(m_totalPages > 0 ? fmt::format("Point History - Week {}", m_currentPage + 1).c_str() : "Point History");
        m_pageContainer->removeAllChildren();
        int startIndex = m_currentPage * m_itemsPerPage;
        for (int i = 0; i < m_itemsPerPage; ++i) {
            int entryIndex = startIndex + i;
            if (entryIndex < m_historyEntries.size()) {
                const auto& [date, points] = m_historyEntries[entryIndex];
                auto cell = HistoryCell::create(date, points);
                cell->setPosition({ 0, 200.f - (i + 1) * 25.f });
                m_pageContainer->addChild(cell);
            }
        }
        m_leftArrow->setVisible(m_currentPage > 0);
        m_rightArrow->setVisible(m_currentPage < m_totalPages - 1);
    }

    void onPrevPage(CCObject*) { if (m_currentPage > 0) { m_currentPage--; this->updatePage(); } }
    void onNextPage(CCObject*) { if (m_currentPage < m_totalPages - 1) { m_currentPage++; this->updatePage(); } }
public:
    static HistoryPopup* create() {
        auto ret = new HistoryPopup();
        if (ret && ret->initAnchored(340.f, 250.f)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

class TicketAnimationLayer : public CCLayer {
protected:
    int m_ticketsWon;
    CCLabelBMFont* m_counterLabel;
    cocos2d::extension::CCScale9Sprite* m_counterBG;
    int m_initialTickets;
    float m_ticketsPerParticle;
    float m_ticketAccumulator = 0.f;

    bool init(int ticketsWon) {
        if (!CCLayer::init()) return false;
        m_ticketsWon = ticketsWon;

        if (m_ticketsWon <= 0) {
            this->removeFromParent();
            return true;
        }

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        m_initialTickets = g_streakData.starTickets - m_ticketsWon;

        m_counterBG = cocos2d::extension::CCScale9Sprite::create("square02_001.png");
        m_counterBG->setColor({ 0, 0, 0 });
        m_counterBG->setOpacity(120);
        this->addChild(m_counterBG);

        auto ticketSprite = CCSprite::create("star_tiket.png"_spr);
        ticketSprite->setScale(0.35f);
        m_counterBG->addChild(ticketSprite);

        m_counterLabel = CCLabelBMFont::create(std::to_string(m_initialTickets).c_str(), "goldFont.fnt");
        m_counterLabel->setScale(0.6f);
        m_counterBG->addChild(m_counterLabel);

        m_counterBG->setContentSize({ m_counterLabel->getScaledContentSize().width + ticketSprite->getScaledContentSize().width + 25.f, 40.f });
        ticketSprite->setPosition({ m_counterBG->getContentSize().width - ticketSprite->getScaledContentSize().width / 2 - 10.f, m_counterBG->getContentSize().height / 2 });
        m_counterLabel->setPosition({ m_counterLabel->getScaledContentSize().width / 2 + 10.f, m_counterBG->getContentSize().height / 2 });
        m_counterBG->setPosition(winSize.width - m_counterBG->getContentSize().width / 2 - 5.f, winSize.height - m_counterBG->getContentSize().height / 2 - 5.f);

        this->runAnimation(m_counterBG->getPosition());
        return true;
    }

    void runAnimation(CCPoint counterPos) {
        auto winSize = CCDirector::sharedDirector()->getWinSize();
        int particleCount = std::min(m_ticketsWon, 30);
        m_ticketsPerParticle = static_cast<float>(m_ticketsWon) / particleCount;
        float duration = 0.5f;
        float delayPerParticle = 0.05f;

        for (int i = 0; i < particleCount; ++i) {
            auto particle = CCSprite::create("star_tiket.png"_spr);
            particle->setPosition(winSize / 2);
            particle->setScale(0.4f);
            particle->setRotation((rand() % 40) - 20);
            this->addChild(particle);

            ccBezierConfig bezier;
            bezier.endPosition = counterPos;
            bezier.controlPoint_1 = ccp(winSize.width / 2 + (rand() % 200) - 100, winSize.height / 2 + (rand() % 150) - 75);
            bezier.controlPoint_2 = counterPos + ccp((rand() % 100) - 50, (rand() % 100) - 50);

            auto bezierTo = CCBezierTo::create(duration, bezier);
            auto scaleTo = CCScaleTo::create(duration, 0.1f);
            auto fadeOut = CCFadeOut::create(duration * 0.8f);

            particle->runAction(CCSequence::create(
                CCDelayTime::create(i * delayPerParticle),
                CCSpawn::create(CCEaseSineIn::create(bezierTo), scaleTo, CCSequence::create(CCDelayTime::create(duration * 0.2f), fadeOut, nullptr), nullptr),
                CCCallFuncN::create(this, callfuncN_selector(TicketAnimationLayer::onParticleHit)),
                CCRemoveSelf::create(),
                nullptr
            ));
        }

        this->runAction(CCSequence::create(
            CCDelayTime::create(particleCount * delayPerParticle + duration),
            CCCallFunc::create(this, callfunc_selector(TicketAnimationLayer::onAnimationEnd)),
            nullptr
        ));
    }

    void onParticleHit(CCNode* sender) {
        FMODAudioEngine::sharedEngine()->playEffect("coin.mp3"_spr);
        m_ticketAccumulator += m_ticketsPerParticle;
        m_counterLabel->setString(std::to_string(m_initialTickets + static_cast<int>(m_ticketAccumulator)).c_str());
        m_counterLabel->runAction(CCSequence::create(
            CCScaleTo::create(0.1f, 0.8f),
            CCScaleTo::create(0.1f, 0.6f),
            nullptr
        ));
    }

    void runSlideOffAnimation() {
        auto slideOff = CCEaseSineIn::create(
            CCMoveBy::create(0.4f, { m_counterBG->getContentSize().width + 10.f, 0 })
        );
        m_counterBG->runAction(slideOff);
    }

    void onAnimationEnd() {
        m_counterLabel->setString(std::to_string(g_streakData.starTickets).c_str());
        this->runAction(CCSequence::create(
            CCDelayTime::create(0.5f),
            CCCallFunc::create(this, callfunc_selector(TicketAnimationLayer::runSlideOffAnimation)),
            CCDelayTime::create(0.5f),
            CCRemoveSelf::create(),
            nullptr
        ));
    }

public:
    static TicketAnimationLayer* create(int ticketsWon) {
        auto ret = new TicketAnimationLayer();
        if (ret && ret->init(ticketsWon)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

class GenericPrizePopup : public Popup<GenericPrizeResult, std::function<void()>> {
protected:
    std::function<void()> m_onCloseCallback = nullptr;

    void onClose(CCObject* sender) override {
        if (m_onCloseCallback) {
            m_onCloseCallback();
        }
        Popup::onClose(sender);
    }

    bool setup(GenericPrizeResult prize, std::function<void()> onCloseCallback) override {
        m_onCloseCallback = onCloseCallback;
        auto winSize = m_mainLayer->getContentSize();

        this->setTitle("You Won a Prize!");

        auto nameLabel = CCLabelBMFont::create(prize.displayName.c_str(), "goldFont.fnt");
        nameLabel->setScale(0.7f);
        m_mainLayer->addChild(nameLabel);

        auto categoryLabel = CCLabelBMFont::create(g_streakData.getCategoryName(prize.category).c_str(), "bigFont.fnt");
        categoryLabel->setColor(g_streakData.getCategoryColor(prize.category));
        categoryLabel->setScale(0.5f);
        m_mainLayer->addChild(categoryLabel);

        if (prize.type == RewardType::Badge) {
            if (!prize.isNew) {
                this->setTitle("Duplicate Badge!");
                auto rewardNode = CCNode::create();
                rewardNode->setPosition({ winSize.width / 2, winSize.height / 2 + 55.f });
                m_mainLayer->addChild(rewardNode);
                auto amountLabel = CCLabelBMFont::create(fmt::format("+{}", prize.ticketsFromDuplicate).c_str(), "goldFont.fnt");
                amountLabel->setScale(0.6f);
                rewardNode->addChild(amountLabel);
                auto ticketSprite = CCSprite::create("star_tiket.png"_spr);
                ticketSprite->setScale(0.3f);
                rewardNode->addChild(ticketSprite);
                float totalWidth = amountLabel->getScaledContentSize().width + ticketSprite->getScaledContentSize().width + 5.f;
                amountLabel->setPosition({ -totalWidth / 2 + amountLabel->getScaledContentSize().width / 2, 0 });
                ticketSprite->setPosition({ totalWidth / 2 - ticketSprite->getScaledContentSize().width / 2, 0 });
            }
            else {
                this->setTitle("You Won a Badge!");
            }
            auto badgeSprite = CCSprite::create(prize.spriteName.c_str());
            badgeSprite->setPosition({ winSize.width / 2, winSize.height / 2 + 20.f });
            badgeSprite->setScale(0.3f);
            m_mainLayer->addChild(badgeSprite);
            nameLabel->setPosition({ winSize.width / 2, winSize.height / 2 - 35.f });
            categoryLabel->setPosition({ winSize.width / 2, winSize.height / 2 - 55.f });
        }
        else {
            auto itemSprite = CCSprite::create(prize.spriteName.c_str());
            itemSprite->setPosition({ winSize.width / 2, winSize.height / 2 + 30.f });
            itemSprite->setScale(0.6f);
            m_mainLayer->addChild(itemSprite);
            nameLabel->setPosition({ winSize.width / 2, winSize.height / 2 - 25.f });
            categoryLabel->setPosition({ winSize.width / 2, winSize.height / 2 - 45.f });
        }

        auto okBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("OK"), this, menu_selector(GenericPrizePopup::onClose));
        auto menu = CCMenu::createWithItem(okBtn);
        menu->setPosition({ winSize.width / 2, 40.f });
        m_mainLayer->addChild(menu);

        return true;
    }

public:
    static GenericPrizePopup* create(GenericPrizeResult prize, std::function<void()> onCloseCallback) {
        auto ret = new GenericPrizePopup();
        if (ret && ret->initAnchored(280.f, 240.f, prize, onCloseCallback)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

class MythicAnimationLayer : public CCLayer {
public:
    CCMenu* m_okMenu;
    CCLabelBMFont* m_mythicLabel;
    std::vector<ccColor3B> m_mythicColors;
    int m_colorIndex = 0;
    float m_colorTransitionTime = 0.0f;
    ccColor3B m_currentColor;
    ccColor3B m_targetColor;
    std::function<void()> m_onCompletionCallback;

    void onEnter() override {
        CCLayer::onEnter();
        CCDirector::sharedDirector()->getTouchDispatcher()->addTargetedDelegate(this, -513, true);
    }

    void onExit() override {
        CCDirector::sharedDirector()->getTouchDispatcher()->removeDelegate(this);
        CCLayer::onExit();
    }

    bool ccTouchBegan(CCTouch* touch, CCEvent* event) override {
        auto location = this->convertTouchToNodeSpace(touch);
        if (m_okMenu) {
            CCRect menuBox = m_okMenu->boundingBox();
            CCRect largerClickArea = CCRect(
                menuBox.origin.x - 15,
                menuBox.origin.y - 15,
                menuBox.size.width + 30,
                menuBox.size.height + 30
            );
            if (largerClickArea.containsPoint(location)) {
                return false;
            }
        }
        return true;
    }

    void ccTouchMoved(CCTouch* touch, CCEvent* event) override {}
    void ccTouchEnded(CCTouch* touch, CCEvent* event) override {}
    void ccTouchCancelled(CCTouch* touch, CCEvent* event) override {}

    void playCustomSound() {
        FMODAudioEngine::sharedEngine()->playEffect("mythic_badge.mp3"_spr);
    }

    void onMythicClose(CCObject*) {
        if (m_onCompletionCallback) {
            m_onCompletionCallback();
        }
        this->removeFromParent();
    }

    static MythicAnimationLayer* create(StreakData::BadgeInfo badge, std::function<void()> onCompletion = nullptr) {
        auto ret = new MythicAnimationLayer();
        if (ret && ret->init(badge, onCompletion)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    bool init(StreakData::BadgeInfo badge, std::function<void()> onCompletion) {
        if (!CCLayer::init()) return false;
        m_onCompletionCallback = onCompletion;
        auto winSize = CCDirector::sharedDirector()->getWinSize();
        m_mythicColors = {
            ccc3(255, 0, 0), ccc3(255, 165, 0), ccc3(255, 255, 0), ccc3(0, 255, 0),
            ccc3(0, 0, 255), ccc3(75, 0, 130), ccc3(238, 130, 238)
        };
        m_currentColor = m_mythicColors[0];
        m_targetColor = m_mythicColors[1];
        auto background = CCLayerColor::create({ 0, 0, 0, 0 });
        background->runAction(CCFadeTo::create(0.5f, 180));
        this->addChild(background);
        auto congratsLabel = CCLabelBMFont::create("Congratulations!", "goldFont.fnt");
        congratsLabel->setColor({ 255, 80, 80 });
        congratsLabel->setPosition(winSize / 2);
        congratsLabel->setScale(0.f);
        this->addChild(congratsLabel);
        auto badgeNode = CCNode::create();
        badgeNode->setPosition(winSize / 2);
        badgeNode->setScale(0.f);
        this->addChild(badgeNode);
        auto shineSprite = CCSprite::createWithSpriteFrameName("shineBurst_001.png");
        shineSprite->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });
        shineSprite->setOpacity(150);
        shineSprite->setScale(3.f);
        badgeNode->addChild(shineSprite);
        auto badgeSprite = CCSprite::create(badge.spriteName.c_str());
        badgeSprite->setScale(0.4f);
        badgeNode->addChild(badgeSprite, 2);
        m_mythicLabel = CCLabelBMFont::create("Mythic", "goldFont.fnt");
        m_mythicLabel->setColor({ 255, 80, 80 });
        m_mythicLabel->setAnchorPoint({ 0, 0.5f });
        m_mythicLabel->setPosition({ -200.f, winSize.height - 40.f });
        m_mythicLabel->setOpacity(0);
        m_mythicLabel->setScale(0.7f);
        this->addChild(m_mythicLabel);
        auto nameLabel = CCLabelBMFont::create(badge.displayName.c_str(), "goldFont.fnt");
        nameLabel->setAnchorPoint({ 0, 0.5f });
        nameLabel->setPosition({ -200.f, winSize.height - 65.f });
        nameLabel->setOpacity(0);
        nameLabel->setScale(0.5f);
        this->addChild(nameLabel);
        auto okBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("OK"), this, menu_selector(MythicAnimationLayer::onMythicClose));
        m_okMenu = CCMenu::createWithItem(okBtn);
        m_okMenu->setPosition({ winSize.width / 2, 60.f });
        this->addChild(m_okMenu);
        FMODAudioEngine::sharedEngine()->playEffect("achievement.mp3"_spr);
        congratsLabel->runAction(CCSequence::create(
            CCEaseBackOut::create(CCScaleTo::create(0.4f, 1.2f)),
            CCDelayTime::create(0.7f),
            CCEaseBackIn::create(CCScaleTo::create(0.4f, 0.f)),
            nullptr
        ));
        badgeNode->runAction(CCSequence::create(
            CCDelayTime::create(1.5f),
            CCCallFunc::create(this, callfunc_selector(MythicAnimationLayer::playCustomSound)),
            CCEaseBackOut::create(CCScaleTo::create(0.5f, 1.0f)),
            nullptr
        ));
        shineSprite->runAction(CCRepeatForever::create(CCRotateBy::create(4.0f, 360.f)));
        auto floatUp = CCMoveBy::create(1.5f, { 0, 5.f });
        badgeNode->runAction(CCSequence::create(
            CCDelayTime::create(2.0f),
            CCRepeatForever::create(CCSequence::create(
                CCEaseSineInOut::create(floatUp),
                CCEaseSineInOut::create(floatUp->reverse()),
                nullptr
            )),
            nullptr
        ));
        auto cornerTextMove1 = CCEaseSineOut::create(CCMoveTo::create(0.5f, { 20.f, winSize.height - 40.f }));
        m_mythicLabel->runAction(CCSequence::create(
            CCDelayTime::create(2.5f),
            CCSpawn::create(CCFadeIn::create(0.5f), cornerTextMove1, nullptr),
            nullptr
        ));
        auto cornerTextMove2 = CCEaseSineOut::create(CCMoveTo::create(0.5f, { 20.f, winSize.height - 65.f }));
        nameLabel->runAction(CCSequence::create(
            CCDelayTime::create(2.5f),
            CCSpawn::create(CCFadeIn::create(0.5f), cornerTextMove2, nullptr),
            nullptr
        ));
        this->scheduleUpdate();
        return true;
    }

    void update(float dt) override {
        m_colorTransitionTime += dt;
        float transitionDuration = 1.0f;
        if (m_colorTransitionTime >= transitionDuration) {
            m_colorTransitionTime = 0.0f;
            m_colorIndex = (m_colorIndex + 1) % m_mythicColors.size();
            m_currentColor = m_targetColor;
            m_targetColor = m_mythicColors[(m_colorIndex + 1) % m_mythicColors.size()];
        }
        float progress = m_colorTransitionTime / transitionDuration;
        ccColor3B interpolatedColor = {
            static_cast<GLubyte>(m_currentColor.r + (m_targetColor.r - m_currentColor.r) * progress),
            static_cast<GLubyte>(m_currentColor.g + (m_targetColor.g - m_currentColor.g) * progress),
            static_cast<GLubyte>(m_currentColor.b + (m_targetColor.b - m_currentColor.b) * progress)
        };
        m_mythicLabel->setColor(interpolatedColor);
    }
};

class MultiPrizePopup : public Popup<std::vector<GenericPrizeResult>, std::function<void()>> {
protected:
    std::function<void()> m_onCloseCallback;

    void onClose(CCObject* sender) override {
        if (m_onCloseCallback) {
            m_onCloseCallback();
        }
        Popup::onClose(sender);
    }

    bool setup(std::vector<GenericPrizeResult> prizes, std::function<void()> onCloseCallback) override {
        this->setTitle("Congratulations!");
        m_onCloseCallback = onCloseCallback;
        auto winSize = m_mainLayer->getContentSize();
        auto prizesContainer = CCNode::create();
        m_mainLayer->addChild(prizesContainer);
        const int cols = 5;
        const float itemWidth = 70.f;
        const float itemHeight = 75.f;
        const CCPoint startPos = {
            (winSize.width - (cols - 1) * itemWidth) / 2.f,
            winSize.height / 2.f + 45.f
        };
        for (size_t i = 0; i < prizes.size(); ++i) {
            auto& result = prizes[i];
            int row = i / cols;
            int col = i % cols;
            auto itemNode = CCNode::create();
            itemNode->setPosition({ startPos.x + col * itemWidth, startPos.y - row * itemHeight });
            prizesContainer->addChild(itemNode);
            auto itemSprite = CCSprite::create(result.spriteName.c_str());
            itemSprite->setScale(0.28f);
            itemNode->addChild(itemSprite);
            auto categoryLabel = CCLabelBMFont::create(g_streakData.getCategoryName(result.category).c_str(), "goldFont.fnt");
            categoryLabel->setColor(getBrightQualityColor(result.category));
            categoryLabel->setScale(0.4f);
            categoryLabel->setPosition({ 0, -25.f });
            itemNode->addChild(categoryLabel);
            if (result.type == RewardType::Badge) {
                if (result.isNew) {
                    auto newLabel = CCLabelBMFont::create("NEW!", "bigFont.fnt");
                    newLabel->setColor(getBrightQualityColor(result.category));
                    newLabel->setScale(0.4f);
                    newLabel->setPosition({ 18.f, 18.f });
                    itemNode->addChild(newLabel);
                    newLabel->runAction(CCRepeatForever::create(CCSequence::create(
                        CCScaleTo::create(0.5f, 0.45f), CCScaleTo::create(0.5f, 0.4f), nullptr
                    )));
                }
                else {
                    auto ticketNode = CCNode::create();
                    ticketNode->setPosition({ 20.f, 18.f });
                    itemNode->addChild(ticketNode);
                    auto ticketSprite = CCSprite::create("star_tiket2.png"_spr);
                    ticketSprite->setScale(0.18f);
                    ticketSprite->setPosition({ -5.f, 0 });
                    ticketNode->addChild(ticketSprite);
                    auto amountLabel = CCLabelBMFont::create(fmt::format("+{}", result.ticketsFromDuplicate).c_str(), "goldFont.fnt");
                    amountLabel->setScale(0.4f);
                    amountLabel->setAnchorPoint({ 0, 0.5f });
                    amountLabel->setPosition({ 5.f, 0 });
                    ticketNode->addChild(amountLabel);
                }
            }
            else {
                auto amountLabel = CCLabelBMFont::create(fmt::format("x{}", result.quantity).c_str(), "goldFont.fnt");
                amountLabel->setScale(0.5f);
                amountLabel->setPosition({ 18.f, -18.f });
                itemNode->addChild(amountLabel);
            }
        }
        auto okBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("OK"), this, menu_selector(MultiPrizePopup::onClose));
        auto menu = CCMenu::createWithItem(okBtn);
        menu->setPosition({ winSize.width / 2, 30.f });
        m_mainLayer->addChild(menu);
        return true;
    }

public:
    static MultiPrizePopup* create(std::vector<GenericPrizeResult> prizes, std::function<void()> onCloseCallback = nullptr) {
        auto ret = new MultiPrizePopup();
        if (ret && ret->initAnchored(380.f, 230.f, prizes, onCloseCallback)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};


class RedeemCodePopup : public Popup<> {
protected:
    TextInput* m_textInput;
    CCMenuItemSpriteExtra* m_okBtn;
    LoadingCircle* m_loadingCircle;
    EventListener<web::WebTask> m_redeemListener;
    bool m_isRequesting;

    bool setup() override {
        this->setTitle("Redeem Code");
        auto winSize = m_mainLayer->getContentSize();



        m_textInput = TextInput::create(280.f, "Enter Code Here");
        m_textInput->setPosition({ winSize.width / 2, winSize.height / 2 + 10.f }); 
        m_textInput->setFilter("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-");
        m_textInput->setMaxCharCount(24);
        m_mainLayer->addChild(m_textInput);

        m_okBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("OK"),
            this,
            menu_selector(RedeemCodePopup::onOk)
        );

        auto menu = CCMenu::createWithItem(m_okBtn);
        menu->setPosition({ winSize.width / 2, winSize.height / 2 - 45.f });
        m_mainLayer->addChild(menu);

        m_loadingCircle = nullptr;
        m_isRequesting = false;

        m_redeemListener.bind(this, &RedeemCodePopup::onWebResponse);

        return true;
    }

    void onOk(CCObject*) {
        if (m_isRequesting) {
            return;
        }

        this->showLoading();

        std::string code = m_textInput->getString();
        int accountID = GJAccountManager::sharedState()->m_accountID;

        if (accountID == 0) {
            FLAlertLayer::create("Error", "You must be logged in to redeem a code.", "OK")->show();
            this->hideLoading();
            return;
        }

        if (code.empty()) {
            FLAlertLayer::create("Error", "Please enter a code.", "OK")->show();
            this->hideLoading();
            return;
        }

        matjson::Value payload = matjson::Value::object();
        payload.set("code", code);
        payload.set("accountID", accountID);

        auto req = web::WebRequest();
        m_redeemListener.setFilter(
            req.bodyJSON(payload).post("https://streak-servidor.onrender.com/redeem-code")
        );
    }

    void onWebResponse(web::WebTask::Event* e) {
        if (!e->getValue() && !e->isCancelled()) {
            return;
        }

        this->hideLoading();

        if (web::WebResponse* res = e->getValue()) {
            if (res->ok() && res->json().isOk()) {
                auto responseJson = res->json().unwrap();
                std::string message = responseJson["message"].as<std::string>().unwrapOr("Success!");

                if (responseJson.contains("rewardType")) {
                    std::string rewardType = responseJson["rewardType"].as<std::string>().unwrapOr("");
                    if (rewardType == "super_stars") {
                        g_streakData.superStars += responseJson["rewardValue"].as<int>().unwrapOr(0);
                    }
                    else if (rewardType == "star_tickets") {
                        g_streakData.starTickets += responseJson["rewardValue"].as<int>().unwrapOr(0);
                    }
                    else if (rewardType == "badge") {
                        g_streakData.unlockBadge(responseJson["rewardValue"].as<std::string>().unwrapOr(""));
                    }
                    updatePlayerDataInFirebase();
                }

                FLAlertLayer::create("Success", message.c_str(), "OK")->show();
                this->onClose(nullptr);
            }
            else {
                std::string errorMsg = "An unknown error occurred.";
                if (res->json().isOk()) {
                    errorMsg = res->json().unwrap()["error"].as<std::string>().unwrapOr("Invalid request.");
                }
                FLAlertLayer::create("Error", errorMsg.c_str(), "OK")->show();
            }
        }
        else if (e->isCancelled()) {
            log::error("[Streak!]: Connection Error or task cancelled.");
            FLAlertLayer::create("Error", "Could not connect to the server.", "OK")->show();
        }
    }

    void showLoading() {
        if (m_isRequesting) return;
        m_isRequesting = true;

        m_okBtn->setEnabled(false);
        m_textInput->getInputNode()->setTouchEnabled(false);

        if (m_textInput->getInputNode()->m_textField) {
            m_textInput->getInputNode()->m_textField->setOpacity(100);
        }
        m_loadingCircle = LoadingCircle::create();
        this->addChild(m_loadingCircle);
        m_loadingCircle->show();
    }

    void hideLoading() {
        m_isRequesting = false;

        if (m_loadingCircle) {

            m_loadingCircle->removeFromParent();


            m_loadingCircle = nullptr;
        }
        m_okBtn->setEnabled(true);
        m_textInput->getInputNode()->setTouchEnabled(true);
        if (m_textInput->getInputNode()->m_textField) {
            m_textInput->getInputNode()->m_textField->setOpacity(255);
        }
    }

public:
    static RedeemCodePopup* create() {
        auto ret = new RedeemCodePopup();
        if (ret && ret->initAnchored(340.f, 160.f)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

class ShopPopup : public Popup<> {
protected:
    CCLabelBMFont* m_ticketCounterLabel = nullptr;
    CCMenu* m_itemMenu = nullptr;
    std::map<std::string, int> m_shopItems;

    bool setup() override {
        this->setTitle("Streak Shop");
        auto winSize = m_mainLayer->getContentSize();
        g_streakData.load();
        m_shopItems = {
            {"beta_badge", 25},
            {"diamante_mc_badge", 550},
            {"platino_streak_badge", 30},
            {"diamante_gd_badge", 40},
            {"hounter_badge", 60},
            {"gd_badge", 130},
            {"ncs_badge", 150},
            {"dark_streak_badge", 500},
            {"gold_streak_badge", 1200},
            {"super_star_badge", 5000}
        };
        auto background = cocos2d::extension::CCScale9Sprite::create("square02_001.png");
        background->setColor({ 0, 0, 0 });
        background->setOpacity(120);
        background->setContentSize({ 340.f, 220.f });
        background->setPosition(winSize / 2);
        m_mainLayer->addChild(background);
        auto ticketNode = CCNode::create();
        m_mainLayer->addChild(ticketNode);
        m_ticketCounterLabel = CCLabelBMFont::create(std::to_string(g_streakData.starTickets).c_str(), "goldFont.fnt");
        m_ticketCounterLabel->setScale(0.5f);
        ticketNode->addChild(m_ticketCounterLabel);
        auto ticketSprite = CCSprite::create("star_tiket.png"_spr);
        ticketSprite->setScale(0.25f);
        ticketNode->addChild(ticketSprite);
        ticketNode->setContentSize({
            m_ticketCounterLabel->getScaledContentSize().width + ticketSprite->getScaledContentSize().width + 5.f,
            ticketSprite->getScaledContentSize().height
            });
        m_ticketCounterLabel->setPosition({ -ticketSprite->getScaledContentSize().width / 2, 0 });
        ticketSprite->setPosition({ m_ticketCounterLabel->getScaledContentSize().width / 2 + 5.f, 0 });
        ticketNode->setPosition({ m_size.width - ticketNode->getContentSize().width / 2 - 15.f, m_size.height - 25.f });
        m_itemMenu = CCMenu::create();
        m_itemMenu->setContentSize(background->getContentSize());
        m_itemMenu->ignoreAnchorPointForPosition(false);
        m_itemMenu->setPosition(background->getPosition());
        m_mainLayer->addChild(m_itemMenu);
        updateItems();
        return true;
    }

    void updateItems() {
        m_itemMenu->removeAllChildren();
        g_streakData.load();
        std::vector<std::pair<std::string, int>> sortedItems(m_shopItems.begin(), m_shopItems.end());
        std::sort(sortedItems.begin(), sortedItems.end(), [this](const auto& a, const auto& b) {
            auto* badgeA = g_streakData.getBadgeInfo(a.first);
            auto* badgeB = g_streakData.getBadgeInfo(b.first);
            if (badgeA && badgeB) {
                return static_cast<int>(badgeA->category) < static_cast<int>(badgeB->category);
            }
            return false;
            });
        const int cols = 4;
        const float itemSize = 60.f;
        const float spacingX = 80.f;
        const float spacingY = 65.f;
        const CCPoint startPos = {
            m_itemMenu->getContentSize().width / 2 - (spacingX * 1.5f),
            m_itemMenu->getContentSize().height - 45.f
        };
        for (size_t i = 0; i < sortedItems.size(); ++i) {
            int row = i / cols;
            int col = i % cols;
            std::string badgeID = sortedItems[i].first;
            int price = sortedItems[i].second;
            auto* badge = g_streakData.getBadgeInfo(badgeID);
            if (!badge) continue;
            auto itemNode = CCNode::create();
            itemNode->setContentSize({ itemSize, itemSize });
            auto badgeSprite = CCSprite::create(badge->spriteName.c_str());
            badgeSprite->setScale(0.22f);
            badgeSprite->setPosition({ itemSize / 2, itemSize / 2 + 5.f });
            itemNode->addChild(badgeSprite);
            auto priceLabel = CCLabelBMFont::create(std::to_string(price).c_str(), "goldFont.fnt");
            priceLabel->setScale(0.35f);
            itemNode->addChild(priceLabel);
            auto priceTicketIcon = CCSprite::create("star_tiket.png"_spr);
            priceTicketIcon->setScale(0.15f);
            itemNode->addChild(priceTicketIcon);
            float totalWidth = priceLabel->getScaledContentSize().width + priceTicketIcon->getScaledContentSize().width + 3.f;
            priceLabel->setPosition({ itemSize / 2 - totalWidth / 2 + priceLabel->getScaledContentSize().width / 2, 8.f });
            priceTicketIcon->setPosition({ itemSize / 2 + totalWidth / 2 - priceTicketIcon->getScaledContentSize().width / 2, 8.f });
            CCMenuItemSpriteExtra* buyBtn = CCMenuItemSpriteExtra::create(
                itemNode, this, menu_selector(ShopPopup::onBuyItem)
            );
            buyBtn->setUserObject(CCString::create(badge->badgeID));
            buyBtn->setPosition({ startPos.x + col * spacingX, startPos.y - row * spacingY });
            m_itemMenu->addChild(buyBtn);
            if (g_streakData.isBadgeUnlocked(badge->badgeID)) {
                buyBtn->setEnabled(false);
                for (auto* child : CCArrayExt<CCNode*>(itemNode->getChildren())) {
                    if (auto* rgbaNode = dynamic_cast<CCRGBAProtocol*>(child)) {
                        rgbaNode->setOpacity(100);
                    }
                }
                auto checkmark = CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
                checkmark->setScale(0.4f);
                checkmark->setPosition(badgeSprite->getPosition());
                itemNode->addChild(checkmark, 2);
            }
            updatePlayerDataInFirebase();
        }
    }

    void onBuyItem(CCObject* sender) {
        std::string badgeID = static_cast<CCString*>(static_cast<CCNode*>(sender)->getUserObject())->getCString();
        auto badgeInfo = g_streakData.getBadgeInfo(badgeID);
        if (!badgeInfo) return;
        int price = m_shopItems[badgeID];
        if (g_streakData.starTickets < price) {
            FLAlertLayer::create("Not Enough Tickets", "You don't have enough tickets to buy this item.", "OK")->show();
            return;
        }
        createQuickPopup(
            "Confirm Purchase",
            fmt::format("Do you want to buy <cg>{}</c> for <cr>{}</c> tickets?", badgeInfo->displayName, price),
            "Cancel", "Buy",
            [this, badgeID, price](FLAlertLayer*, bool btn2) {
                if (btn2) {
                    g_streakData.load();
                    g_streakData.starTickets -= price;
                    g_streakData.unlockBadge(badgeID);
                    g_streakData.save();
                    FMODAudioEngine::sharedEngine()->playEffect("buy_obj.mp3"_spr);
                    FLAlertLayer::create("Purchase Successful", "You have unlocked a new badge!", "OK")->show();
                    m_ticketCounterLabel->setString(std::to_string(g_streakData.starTickets).c_str());
                    updateItems();
                }
                updatePlayerDataInFirebase();
            }
        );
    }

public:
    static ShopPopup* create() {
        auto ret = new ShopPopup();
        if (ret && ret->initAnchored(420.f, 280.f)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

class RoulettePopup : public Popup<> {
protected:
    CCNode* m_rouletteNode;
    CCSprite* m_selectorSprite;
    CCMenuItemSpriteExtra* m_spinBtn;
    CCMenuItemSpriteExtra* m_spin10Btn;
    bool m_isSpinning = false;
    std::vector<CCNode*> m_orderedSlots;
    std::vector<RoulettePrize> m_roulettePrizes;
    int m_currentSelectorIndex = 0;
    int m_totalSteps = 0;
    std::vector<CCSprite*> m_mythicSprites;
    std::vector<ccColor3B> m_mythicColors;
    int m_colorIndex = 0;
    float m_colorTransitionTime = 0.0f;
    ccColor3B m_currentColor;
    ccColor3B m_targetColor;
    float m_slotSize = 0.f;
    std::vector<GenericPrizeResult> m_multiSpinResults;
    std::vector<StreakData::BadgeInfo> m_pendingMythics;

    void runPopAnimation(CCObject* sender) {
        if (auto node = dynamic_cast<CCNode*>(sender)) {
            auto popAction = CCSequence::create(
                CCScaleTo::create(0.025f, 1.1f),
                CCScaleTo::create(0.025f, 1.0f),
                nullptr
            );
            node->runAction(popAction);
        }
    }

    void runSlotAnimation(CCObject* sender) {
        if (auto node = dynamic_cast<CCNode*>(sender)) {
            auto popAction = CCSequence::create(
                CCScaleTo::create(0.02f, 1.1f),
                CCScaleTo::create(0.02f, 1.0f),
                nullptr
            );
            node->runAction(popAction);
        }

    }

    bool setup() override {
        this->setTitle("Roulette");
        auto winSize = m_mainLayer->getContentSize();
        g_streakData.load();
        m_currentSelectorIndex = g_streakData.lastRouletteIndex;
        m_mythicColors = {
            ccc3(255, 0, 0), ccc3(255, 165, 0), ccc3(255, 255, 0), ccc3(0, 255, 0),
            ccc3(0, 0, 255), ccc3(75, 0, 130), ccc3(238, 130, 238)
        };
        m_currentColor = m_mythicColors[0];
        m_targetColor = m_mythicColors[1];
        m_rouletteNode = CCNode::create();
        m_rouletteNode->setPosition({ winSize.width / 2, winSize.height / 2 + 10.f });
        m_mainLayer->addChild(m_rouletteNode);
        m_roulettePrizes = {
            { RewardType::Badge, "super_star_badge", 1, "", "First Mythic", 1, StreakData::BadgeCategory::MYTHIC },
            { RewardType::Badge, "gold_streak_badge", 1, "", "Gold Legend's", 3, StreakData::BadgeCategory::LEGENDARY },
            { RewardType::Badge, "dark_streak_badge", 1, "", "dark side", 5, StreakData::BadgeCategory::EPIC },
            { RewardType::Badge, "ncs_badge", 1, "", "NCS lover", 10, StreakData::BadgeCategory::SPECIAL },
            { RewardType::Badge, "beta_badge", 1, "", "Player beta?", 70, StreakData::BadgeCategory::COMMON },
            { RewardType::Badge, "platino_streak_badge", 1, "", "platino badge", 70, StreakData::BadgeCategory::COMMON },
            { RewardType::StarTicket, "star_tiket_1", 1, "star_tiket.png"_spr, "1 star tiket", 70, StreakData::BadgeCategory::COMMON },
            { RewardType::StarTicket, "star_tiket_3", 3, "star_tiket.png"_spr, "3 star tiket", 70, StreakData::BadgeCategory::COMMON },
            { RewardType::StarTicket, "star_ticket_15", 15, "star_tiket.png"_spr, "15 Tickets", 70, StreakData::BadgeCategory::COMMON },
            { RewardType::StarTicket, "star_ticket_30", 30, "star_tiket.png"_spr, "30 Tickets", 10, StreakData::BadgeCategory::SPECIAL },
            { RewardType::StarTicket, "star_ticket_60", 60, "star_tiket.png"_spr, "60 Tickets", 5, StreakData::BadgeCategory::EPIC },
            { RewardType::StarTicket, "star_ticket_100", 100, "star_tiket.png"_spr, "100 Tickets", 3, StreakData::BadgeCategory::LEGENDARY }
        };
        const int gridSize = 4;
        m_slotSize = 38.f;
        const float spacing = 5.f;
        const float totalSize = (gridSize * m_slotSize) + ((gridSize - 1) * spacing);
        const float startPos = -totalSize / 2.f + m_slotSize / 2.f;
        std::vector<CCPoint> slotPositions;
        for (int i = 0; i < gridSize; ++i) slotPositions.push_back({ startPos + i * (m_slotSize + spacing), startPos + (gridSize - 1) * (m_slotSize + spacing) });
        for (int i = gridSize - 2; i > 0; --i) slotPositions.push_back({ startPos + (gridSize - 1) * (m_slotSize + spacing), startPos + i * (m_slotSize + spacing) });
        for (int i = gridSize - 1; i >= 0; --i) slotPositions.push_back({ startPos + i * (m_slotSize + spacing), startPos });
        for (int i = 1; i < gridSize - 1; ++i) slotPositions.push_back({ startPos, startPos + i * (m_slotSize + spacing) });
        for (size_t i = 0; i < slotPositions.size(); ++i) {
            if (i >= m_roulettePrizes.size()) continue;
            auto& currentPrize = m_roulettePrizes[i];
            auto slotContainer = CCNode::create();
            slotContainer->setPosition(slotPositions[i]);
            m_rouletteNode->addChild(slotContainer);
            m_orderedSlots.push_back(slotContainer);
            auto slotBG = cocos2d::extension::CCScale9Sprite::create("square02_001.png");
            slotBG->setContentSize({ m_slotSize, m_slotSize });
            slotBG->setColor({ 0, 0, 0 });
            slotBG->setOpacity(150);
            slotContainer->addChild(slotBG);
            auto qualitySprite = CCSprite::create(getQualitySpriteName(currentPrize.category).c_str());
            qualitySprite->setScale((m_slotSize - 4.f) / qualitySprite->getContentSize().width);
            slotContainer->addChild(qualitySprite, 1);
            if (currentPrize.category == StreakData::BadgeCategory::MYTHIC) m_mythicSprites.push_back(qualitySprite);
            if (currentPrize.type == RewardType::Badge) {
                auto* badgeInfo = g_streakData.getBadgeInfo(currentPrize.id);
                if (badgeInfo) {
                    auto rewardIcon = CCSprite::create(badgeInfo->spriteName.c_str());
                    rewardIcon->setScale(0.15f);
                    slotContainer->addChild(rewardIcon, 2);
                    if (g_streakData.isBadgeUnlocked(badgeInfo->badgeID)) {
                        auto claimedIcon = CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
                        claimedIcon->setScale(0.5f);
                        claimedIcon->setPosition({ m_slotSize / 2 - 5, -m_slotSize / 2 + 5 });
                        claimedIcon->setTag(199);
                        slotContainer->addChild(claimedIcon, 3);
                    }
                }
            }
            else {
                auto rewardIcon = CCSprite::create(currentPrize.spriteName.c_str());
                rewardIcon->setScale(0.2f);
                slotContainer->addChild(rewardIcon, 2);
                auto quantityLabel = CCLabelBMFont::create(CCString::createWithFormat("x%d", currentPrize.quantity)->getCString(), "goldFont.fnt");
                quantityLabel->setScale(0.3f);
                quantityLabel->setPosition(0, -m_slotSize / 2 + 5);
                slotContainer->addChild(quantityLabel, 3);
            }
        }
        m_selectorSprite = CCSprite::create("casilla_selector.png"_spr);
        m_selectorSprite->setScale(m_slotSize / m_selectorSprite->getContentSize().width);
        if (!m_orderedSlots.empty()) m_selectorSprite->setPosition(m_orderedSlots[m_currentSelectorIndex]->getPosition());
        m_rouletteNode->addChild(m_selectorSprite, 4);

        m_spinBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("Spin"), this, menu_selector(RoulettePopup::onSpin));
        m_spin10Btn = CCMenuItemSpriteExtra::create(ButtonSprite::create("x10"), this, menu_selector(RoulettePopup::onSpinMultiple));
        auto spinMenu = CCMenu::create();
        spinMenu->addChild(m_spinBtn);
        spinMenu->addChild(m_spin10Btn);
        spinMenu->alignItemsHorizontallyWithPadding(10.f);
        spinMenu->setPosition({ winSize.width / 2, 30.f });
        m_mainLayer->addChild(spinMenu);
        auto shopSprite = CCSprite::create("shop_btn.png"_spr);
        shopSprite->setScale(0.7f);
        auto shopBtn = CCMenuItemSpriteExtra::create(shopSprite, this, menu_selector(RoulettePopup::onOpenShop));
        auto shopMenu = CCMenu::createWithItem(shopBtn);
        shopMenu->setPosition({ winSize.width - 35.f, 30.f });
        m_mainLayer->addChild(shopMenu);
        auto superStarCounterNode = CCNode::create();
        auto haveAmountLabel = CCLabelBMFont::create(std::to_string(g_streakData.superStars).c_str(), "goldFont.fnt");
        haveAmountLabel->setScale(0.4f);
        haveAmountLabel->setAnchorPoint({ 1.f, 0.5f });
        haveAmountLabel->setID("roulette-super-star-label");
        superStarCounterNode->addChild(haveAmountLabel);
        auto haveStar = CCSprite::create("super_star.png"_spr);
        haveStar->setScale(0.12f);
        superStarCounterNode->addChild(haveStar);
        haveAmountLabel->setPosition({ -haveStar->getScaledContentSize().width / 2 - 2, 0 });
        haveStar->setPosition({ haveAmountLabel->getScaledContentSize().width / 2, 0 });
        superStarCounterNode->setPosition({ winSize.width - 35.f, winSize.height - 25.f });
        m_mainLayer->addChild(superStarCounterNode);
        auto infoIcon = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
        infoIcon->setScale(0.8f);
        auto infoBtn = CCMenuItemSpriteExtra::create(infoIcon, this, menu_selector(RoulettePopup::onShowProbabilities));
        auto infoMenu = CCMenu::createWithItem(infoBtn);
        infoMenu->setPosition({ 25.f, winSize.height - 25.f });
        m_mainLayer->addChild(infoMenu);
        this->scheduleUpdate();
        return true;
    }

    void update(float dt) override {
        if (m_mythicSprites.empty()) return;
        m_colorTransitionTime += dt;
        float transitionDuration = 1.0f;
        if (m_colorTransitionTime >= transitionDuration) {
            m_colorTransitionTime = 0.0f;
            m_colorIndex = (m_colorIndex + 1) % m_mythicColors.size();
            m_currentColor = m_targetColor;
            m_targetColor = m_mythicColors[(m_colorIndex + 1) % m_mythicColors.size()];
        }
        float progress = m_colorTransitionTime / transitionDuration;
        ccColor3B interpolatedColor = {
            static_cast<GLubyte>(m_currentColor.r + (m_targetColor.r - m_currentColor.r) * progress),
            static_cast<GLubyte>(m_currentColor.g + (m_targetColor.g - m_currentColor.g) * progress),
            static_cast<GLubyte>(m_currentColor.b + (m_targetColor.b - m_currentColor.b) * progress)
        };
        for (auto* sprite : m_mythicSprites) sprite->setColor(interpolatedColor);
    }

    void onSpin(CCObject*) {
        g_streakData.load();
        if (g_streakData.superStars < 1) {
            FLAlertLayer::create("Not Enough Super Stars", "You need at least <cy>1 Super Star</c> to spin.", "OK")->show();
            return;
        }
        if (m_isSpinning) return;
        m_isSpinning = true;
        m_spinBtn->setEnabled(false);
        m_spin10Btn->setEnabled(false);
        g_streakData.superStars -= 1;
        g_streakData.totalSpins += 1;
        g_streakData.save();
        auto starCountLabel = static_cast<CCLabelBMFont*>(m_mainLayer->getChildByIDRecursive("roulette-super-star-label"));
        if (starCountLabel) starCountLabel->setString(std::to_string(g_streakData.superStars).c_str());
        auto spinCountLabel = static_cast<CCLabelBMFont*>(m_mainLayer->getChildByIDRecursive("roulette-spin-count-label"));
        if (spinCountLabel) spinCountLabel->setString(CCString::createWithFormat("Spins: %d", g_streakData.totalSpins)->getCString());
        int totalWeight = 0;
        for (const auto& prize : m_roulettePrizes) totalWeight += prize.probabilityWeight;
        int randomValue = rand() % totalWeight;
        int winningIndex = 0;
        for (size_t i = 0; i < m_roulettePrizes.size(); ++i) {
            if (randomValue < m_roulettePrizes[i].probabilityWeight) {
                winningIndex = i;
                break;
            }
            randomValue -= m_roulettePrizes[i].probabilityWeight;
        }
        int totalSlots = m_orderedSlots.size();
        int laps = 5;
        int distance = (winningIndex - m_currentSelectorIndex + totalSlots) % totalSlots;
        m_totalSteps = (laps * totalSlots) + distance;
        auto actions = CCArray::create();
        for (int i = 1; i <= m_totalSteps; ++i) {
            int stepIndex = (m_currentSelectorIndex + i) % totalSlots;
            auto targetSlot = m_orderedSlots[stepIndex];
            float duration = 0.05f;
            if (i < 5) duration = 0.2f - (i * 0.03f);
            else if (i > m_totalSteps - 10) duration = 0.05f + ((i - (m_totalSteps - 10)) * 0.04f);
            auto moveAction = CCEaseSineInOut::create(CCMoveTo::create(duration, targetSlot->getPosition()));
            auto soundAction = CCCallFunc::create(this, callfunc_selector(RoulettePopup::playTickSound));
            auto slotCall = CCCallFuncO::create(this, callfuncO_selector(RoulettePopup::runPopAnimation), targetSlot);
            actions->addObject(CCSpawn::create(moveAction, soundAction, slotCall, nullptr));
        }
        actions->addObject(CCCallFunc::create(this, callfunc_selector(RoulettePopup::onSpinEnd)));
        m_selectorSprite->runAction(CCSequence::create(actions));
        updatePlayerDataInFirebase();
    }

    void onSpinMultiple(CCObject*) {
        g_streakData.load();
        if (g_streakData.superStars < 10) {
            FLAlertLayer::create("Not Enough Super Stars", "You need at least <cy>10 Super Stars</c> to spin x10.", "OK")->show();
            return;
        }
        if (m_isSpinning) return;
        m_isSpinning = true;
        m_spinBtn->setEnabled(false);
        m_spin10Btn->setEnabled(false);
        g_streakData.superStars -= 10;
        g_streakData.totalSpins += 10;
        std::vector<GenericPrizeResult> allPrizes;
        int totalTicketsWon = 0;
        m_pendingMythics.clear();
        std::vector<int> winningIndices;
        int totalWeight = 0;
        for (const auto& prize : m_roulettePrizes) totalWeight += prize.probabilityWeight;
        for (int i = 0; i < 10; ++i) {
            int randomValue = rand() % totalWeight;
            int winningIndex = 0;
            for (size_t j = 0; j < m_roulettePrizes.size(); ++j) {
                if (randomValue < m_roulettePrizes[j].probabilityWeight) {
                    winningIndex = j;
                    break;
                }
                randomValue -= m_roulettePrizes[j].probabilityWeight;
            }
            winningIndices.push_back(winningIndex);
            auto& prize = m_roulettePrizes[winningIndex];
            GenericPrizeResult result;
            result.type = prize.type;
            result.id = prize.id;
            result.quantity = prize.quantity;
            result.displayName = prize.displayName;
            result.category = prize.category;
            switch (prize.type) {
            case RewardType::Badge: {
                auto* badgeInfo = g_streakData.getBadgeInfo(prize.id);
                if (badgeInfo) {
                    result.spriteName = badgeInfo->spriteName;
                    result.isNew = !g_streakData.isBadgeUnlocked(prize.id);
                    if (result.isNew) {
                        g_streakData.unlockBadge(prize.id);
                        if (badgeInfo->category == StreakData::BadgeCategory::MYTHIC) {
                            m_pendingMythics.push_back(*badgeInfo);
                        }
                    }
                    else {
                        int tickets = g_streakData.getTicketValueForRarity(badgeInfo->category);
                        g_streakData.starTickets += tickets;
                        totalTicketsWon += tickets;
                        result.ticketsFromDuplicate = tickets;
                        g_streakData.save();
                    }
                }
                break;
            }
            case RewardType::SuperStar: {
                g_streakData.superStars += prize.quantity;
                result.spriteName = prize.spriteName;
                break;
            }
            case RewardType::StarTicket: {
                g_streakData.starTickets += prize.quantity;
                totalTicketsWon += prize.quantity;
                result.spriteName = prize.spriteName;
                break;
            }
            }
            allPrizes.push_back(result);
        }
        m_multiSpinResults = allPrizes;
        g_streakData.save();
        if (auto starCountLabel = static_cast<CCLabelBMFont*>(m_mainLayer->getChildByIDRecursive("roulette-super-star-label"))) {
            starCountLabel->setString(std::to_string(g_streakData.superStars).c_str());
        }
        if (auto spinCountLabel = static_cast<CCLabelBMFont*>(m_mainLayer->getChildByIDRecursive("roulette-spin-count-label"))) {
            spinCountLabel->setString(CCString::createWithFormat("Spins: %d", g_streakData.totalSpins)->getCString());
        }
        auto actions = CCArray::create();
        int totalSlots = m_orderedSlots.size();
        int lastIndex = m_currentSelectorIndex;
        for (int prizeIndex : winningIndices) {
            int distance = (prizeIndex - lastIndex + totalSlots) % totalSlots;
            if (distance == 0) distance = totalSlots;
            for (int i = 1; i <= distance; ++i) {
                int stepIndex = (lastIndex + i) % totalSlots;
                auto targetSlot = m_orderedSlots[stepIndex];
                float moveDuration = 0.04f;
                auto moveAction = CCMoveTo::create(moveDuration, targetSlot->getPosition());
                auto soundAction = CCCallFunc::create(this, callfunc_selector(RoulettePopup::playTickSound));
                auto slotCall = CCCallFuncO::create(this, callfuncO_selector(RoulettePopup::runSlotAnimation), targetSlot);
                actions->addObject(CCSpawn::create(moveAction, soundAction, slotCall, nullptr));
            }
            actions->addObject(CCDelayTime::create(0.25f));
            actions->addObject(CCCallFuncO::create(this, callfuncO_selector(RoulettePopup::flashWinningSlot), CCInteger::create(prizeIndex)));
            lastIndex = prizeIndex;
        }
        m_currentSelectorIndex = lastIndex;
        actions->addObject(CCCallFunc::create(this, callfunc_selector(RoulettePopup::onMultiSpinEnd)));
        m_selectorSprite->runAction(CCSequence::create(actions));
        updatePlayerDataInFirebase();
    }

    void flashWinningSlot(CCObject* pSender) {
        int slotIndex = static_cast<CCInteger*>(pSender)->getValue();
        if (static_cast<size_t>(slotIndex) >= m_orderedSlots.size()) return;
        auto targetSlot = m_orderedSlots[slotIndex];
        auto glow = CCSprite::create("cuadro.png"_spr);
        if (!glow) return;
        glow->setPosition(targetSlot->getContentSize() / 2);
        glow->setScale(m_slotSize / glow->getContentSize().width);
        glow->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });
        glow->setColor({ 255, 255, 150 });
        glow->setOpacity(0);
        glow->runAction(CCSequence::create(
            CCFadeTo::create(0.2f, 200),
            CCFadeTo::create(0.3f, 0),
            CCRemoveSelf::create(),
            nullptr
        ));
        targetSlot->addChild(glow, 5);
    }

    void processMythicQueue() {
        if (!m_pendingMythics.empty()) {
            auto mythicBadge = m_pendingMythics.front();
            m_pendingMythics.erase(m_pendingMythics.begin());
            auto animLayer = MythicAnimationLayer::create(mythicBadge, [this]() {
                this->processMythicQueue();
                });
            CCDirector::sharedDirector()->getRunningScene()->addChild(animLayer, 400);
        }
        else {
            MultiPrizePopup::create(m_multiSpinResults, [this]() {
                if (m_spinBtn && m_spin10Btn) {
                    m_spinBtn->setEnabled(true);
                    m_spin10Btn->setEnabled(true);
                }
                })->show();
        }
    }

    void onMultiSpinEnd() {
        m_isSpinning = false;
        updateAllCheckmarks();
        g_streakData.lastRouletteIndex = m_currentSelectorIndex;
        g_streakData.save();
        m_pendingMythics.clear();
        int totalTicketsWonInSpin = 0;
        for (const auto& result : m_multiSpinResults) {
            if (result.isNew && result.type == RewardType::Badge && result.category == StreakData::BadgeCategory::MYTHIC) {
                auto* badgeInfo = g_streakData.getBadgeInfo(result.id);
                if (badgeInfo) {
                    m_pendingMythics.push_back(*badgeInfo);
                }
            }
            if (result.type == RewardType::StarTicket) {
                totalTicketsWonInSpin += result.quantity;
            }
            else if (result.type == RewardType::Badge && !result.isNew) {
                totalTicketsWonInSpin += result.ticketsFromDuplicate;
            }
        }
        if (!m_pendingMythics.empty()) {
            processMythicQueue();
        }
        else {
            MultiPrizePopup::create(m_multiSpinResults, [this, totalTicketsWonInSpin]() {
                m_spinBtn->setEnabled(true);
                m_spin10Btn->setEnabled(true);
                if (totalTicketsWonInSpin > 0) {
                    CCDirector::sharedDirector()->getRunningScene()->addChild(TicketAnimationLayer::create(totalTicketsWonInSpin), 1000);
                }
                })->show();
        }
    }

    void updateAllCheckmarks() {
        for (size_t i = 0; i < m_orderedSlots.size(); ++i) {
            if (i >= m_roulettePrizes.size()) continue;
            auto& prize = m_roulettePrizes[i];
            if (prize.type != RewardType::Badge) continue;
            auto slotNode = m_orderedSlots[i];
            if (g_streakData.isBadgeUnlocked(prize.id) && !slotNode->getChildByTag(199)) {
                auto claimedIcon = CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
                claimedIcon->setScale(0.5f);
                claimedIcon->setPosition({ m_slotSize / 2 - 5, -m_slotSize / 2 + 5 });
                claimedIcon->setTag(199);
                slotNode->addChild(claimedIcon, 3);
            }
        }
    }

    void playTickSound() {
        FMODAudioEngine::sharedEngine()->playEffect("ruleta_sfx.mp3"_spr);
    }

    void onSpinEnd() {
        m_isSpinning = false;
        m_spinBtn->setEnabled(true);
        m_spin10Btn->setEnabled(true);
        int winningIndex = (m_currentSelectorIndex + m_totalSteps) % m_orderedSlots.size();
        m_currentSelectorIndex = winningIndex;
        g_streakData.lastRouletteIndex = m_currentSelectorIndex;
        int ticketsWon = 0;
        if (static_cast<size_t>(m_currentSelectorIndex) < m_roulettePrizes.size()) {
            auto& prize = m_roulettePrizes[m_currentSelectorIndex];
            GenericPrizeResult result;
            result.type = prize.type;
            result.id = prize.id;
            result.quantity = prize.quantity;
            result.displayName = prize.displayName;
            result.category = prize.category;
            switch (prize.type) {
            case RewardType::Badge: {
                auto* badgeInfo = g_streakData.getBadgeInfo(prize.id);
                if (badgeInfo) {
                    result.spriteName = badgeInfo->spriteName;
                    result.isNew = !g_streakData.isBadgeUnlocked(prize.id);
                    if (result.isNew) {
                        g_streakData.unlockBadge(prize.id);
                    }
                    else {
                        ticketsWon = g_streakData.getTicketValueForRarity(badgeInfo->category);
                        g_streakData.starTickets += ticketsWon;
                        result.ticketsFromDuplicate = ticketsWon;
                    }
                }
                break;
            }
            case RewardType::SuperStar: {
                g_streakData.superStars += prize.quantity;
                result.spriteName = prize.spriteName;
                break;
            }
            case RewardType::StarTicket: {
                ticketsWon = prize.quantity;
                g_streakData.starTickets += ticketsWon;
                result.spriteName = prize.spriteName;
                break;
            }
            }
            g_streakData.save();

            GenericPrizePopup::create(result, [ticketsWon]() {
                if (ticketsWon > 0) {
                    CCDirector::sharedDirector()->getRunningScene()->addChild(TicketAnimationLayer::create(ticketsWon), 1000);
                }
                })->show();
        }
        g_streakData.save();
        updateAllCheckmarks();
    }

    void onOpenShop(CCObject*) {
        ShopPopup::create()->show();

    }

    void onShowProbabilities(CCObject*) {
        std::map<StreakData::BadgeCategory, int> categoryWeights;
        int totalWeight = 0;
        for (const auto& prize : m_roulettePrizes) {
            categoryWeights[prize.category] += prize.probabilityWeight;
            totalWeight += prize.probabilityWeight;
        }

        std::string message = "";
        auto getColorForCategory = [](StreakData::BadgeCategory cat) -> std::string {
            switch (cat) {
            case StreakData::BadgeCategory::MYTHIC:   return "<cr>";
            case StreakData::BadgeCategory::LEGENDARY: return "<co>";
            case StreakData::BadgeCategory::EPIC:     return "<cp>";
            case StreakData::BadgeCategory::SPECIAL:  return "<cg>";
            default:                                  return "<cy>";
            }
            };

        for (const auto& [category, weight] : categoryWeights) {
            float percentage = (static_cast<float>(weight) / totalWeight) * 100.f;
            message += fmt::format("{}{}:</c> {:.2f}%\n",
                getColorForCategory(category),
                g_streakData.getCategoryName(category),
                percentage
            );
        }

        message += fmt::format("\n<cp>Total Spins:</c> {}", g_streakData.totalSpins);
        FLAlertLayer::create("Probabilities", message, "OK")->show();
    }

    std::string getQualitySpriteName(StreakData::BadgeCategory category) {
        switch (category) {
        case StreakData::BadgeCategory::SPECIAL:   return "casilla_especial.png"_spr;
        case StreakData::BadgeCategory::EPIC:      return "casilla_epica.png"_spr;
        case StreakData::BadgeCategory::LEGENDARY: return "casilla_legendaria.png"_spr;
        case StreakData::BadgeCategory::MYTHIC:    return "casilla_mitica.png"_spr;
        default:                                   return "casilla_comun.png"_spr;
        }
    }

public:
    static RoulettePopup* create() {
        auto ret = new RoulettePopup();
        if (ret && ret->initAnchored(260.f, 260.f)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

class EquipBadgePopup : public Popup<std::string> {
protected:
    std::string m_badgeID;
    bool m_isCurrentlyEquipped;

    bool setup(std::string badgeID) override {
        m_badgeID = badgeID;
        auto winSize = m_mainLayer->getContentSize();

        auto badgeInfo = g_streakData.getBadgeInfo(badgeID);
        if (!badgeInfo) return false;

        auto equippedBadge = g_streakData.getEquippedBadge();
        m_isCurrentlyEquipped = (equippedBadge && equippedBadge->badgeID == badgeID);

        this->setTitle(m_isCurrentlyEquipped ? "Badge Equipped" : "Equip Badge");

        auto badgeSprite = CCSprite::create(badgeInfo->spriteName.c_str());
        if (badgeSprite) {
            badgeSprite->setScale(0.3f);
            badgeSprite->setPosition({ winSize.width / 2, winSize.height / 2 + 20 });
            m_mainLayer->addChild(badgeSprite);
        }

        auto nameLabel = CCLabelBMFont::create(badgeInfo->displayName.c_str(), "goldFont.fnt");
        nameLabel->setScale(0.6f);
        nameLabel->setPosition({ winSize.width / 2, winSize.height / 2 - 20 });
        m_mainLayer->addChild(nameLabel);

        auto categoryLabel = CCLabelBMFont::create(
            g_streakData.getCategoryName(badgeInfo->category).c_str(), "bigFont.fnt"
        );
        categoryLabel->setScale(0.4f);
        categoryLabel->setPosition({ winSize.width / 2, winSize.height / 2 - 40 });
        categoryLabel->setColor(g_streakData.getCategoryColor(badgeInfo->category));
        m_mainLayer->addChild(categoryLabel);

        auto mainBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create(m_isCurrentlyEquipped ? "Unequip" : "Equip"),
            this,
            menu_selector(EquipBadgePopup::onToggleEquip)
        );

        auto menu = CCMenu::create();
        menu->addChild(mainBtn);
        menu->setPosition(winSize.width / 2, winSize.height / 2 - 70);
        m_mainLayer->addChild(menu);

        return true;
    }

    void onToggleEquip(CCObject*) {
        if (m_isCurrentlyEquipped) {
            g_streakData.unequipBadge();
            FLAlertLayer::create("Success", "Badge unequipped!", "OK")->show();
        }
        else {
            g_streakData.equipBadge(m_badgeID);
            FLAlertLayer::create("Success", "Badge equipped!", "OK")->show();
        }


        g_streakData.save();


        updatePlayerDataInFirebase();

        this->onClose(nullptr);
    }

public:
    static EquipBadgePopup* create(std::string badgeID) {
        auto ret = new EquipBadgePopup();
        if (ret && ret->initAnchored(250.f, 200.f, badgeID)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};



class AllRachasPopup : public Popup<> {
protected:
    bool setup() override {
        this->setTitle("All Streaks");
        auto winSize = m_mainLayer->getContentSize();

        float startX = 50.f;
        float y = winSize.height / 2 + 20.f;
        float spacing = 50.f;


        std::vector<std::tuple<std::string, int, int>> rachas = {
            { "racha1.png"_spr, 1,  2 },
            { "racha2.png"_spr, 10, 3 },
            { "racha3.png"_spr, 20, 4 },
            { "racha4.png"_spr, 30, 5 },
            { "racha5.png"_spr, 40, 6 },
            { "racha6.png"_spr, 50, 7 },
            { "racha7.png"_spr, 60, 8 },
            { "racha8.png"_spr, 70, 9 },
            { "racha9.png"_spr, 80, 10 }
        };

        int i = 0;
        for (auto& [sprite, day, requiredPoints] : rachas) {
            auto spr = CCSprite::create(sprite.c_str());
            spr->setScale(0.22f);
            spr->setPosition({ startX + i * spacing, y });
            m_mainLayer->addChild(spr);

            auto label = CCLabelBMFont::create(
                CCString::createWithFormat("Day %d", day)->getCString(), "goldFont.fnt"
            );
            label->setScale(0.35f);
            label->setPosition({ startX + i * spacing, y - 40 });
            m_mainLayer->addChild(label);

            auto pointIcon = CCSprite::create("streak_point.png"_spr);
            pointIcon->setScale(0.12f);
            pointIcon->setPosition({ startX + i * spacing - 4, y - 60 });
            m_mainLayer->addChild(pointIcon);

            auto pointsLabel = CCLabelBMFont::create(
                std::to_string(requiredPoints).c_str(), "bigFont.fnt"
            );
            pointsLabel->setScale(0.3f);
            pointsLabel->setPosition({ startX + i * spacing + 6, y - 60 });
            m_mainLayer->addChild(pointsLabel);
            i++;
        }

        return true;
    }

public:
    static AllRachasPopup* create() {
        auto ret = new AllRachasPopup();
        if (ret && ret->initAnchored(500.f, 155.f)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

class DayProgressPopup : public Popup<> {
protected:
    std::vector<StreakData::BadgeInfo> m_streakBadges;
    int m_currentGoalIndex = 0;
    CCLabelBMFont* m_titleLabel = nullptr;
    CCLayerColor* m_barBg = nullptr;
    CCLayerGradient* m_barFg = nullptr;
    CCLayerColor* m_border = nullptr;
    CCLayerColor* m_outer = nullptr;
    CCSprite* m_rewardSprite = nullptr;
    CCLabelBMFont* m_dayText = nullptr;

    bool setup() override {
        auto winSize = m_mainLayer->getContentSize();
        g_streakData.load();
        for (const auto& badge : g_streakData.badges) {
            if (!badge.isFromRoulette) {
                m_streakBadges.push_back(badge);
            }
        }

        auto leftArrow = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        leftArrow->setScale(0.8f);
        auto leftBtn = CCMenuItemSpriteExtra::create(leftArrow, this, menu_selector(DayProgressPopup::onPreviousGoal));
        leftBtn->setPosition({ -winSize.width / 2 + 30, 0 });

        auto rightArrow = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        rightArrow->setScale(0.8f);
        rightArrow->setFlipX(true);
        auto rightBtn = CCMenuItemSpriteExtra::create(rightArrow, this, menu_selector(DayProgressPopup::onNextGoal));
        rightBtn->setPosition({ winSize.width / 2 - 30, 0 });

        auto arrowMenu = CCMenu::create();
        arrowMenu->addChild(leftBtn);
        arrowMenu->addChild(rightBtn);
        arrowMenu->setPosition({ winSize.width / 2, winSize.height / 2 });
        m_mainLayer->addChild(arrowMenu);

        m_barBg = CCLayerColor::create(ccc4(45, 45, 45, 255), 180.0f, 16.0f);
        m_barFg = CCLayerGradient::create(ccc4(250, 225, 60, 255), ccc4(255, 165, 0, 255));
        m_border = CCLayerColor::create(ccc4(255, 255, 255, 255), 182.0f, 18.0f);
        m_outer = CCLayerColor::create(ccc4(0, 0, 0, 255), 186.0f, 22.0f);

        m_barBg->setVisible(false);
        m_barFg->setVisible(false);
        m_border->setVisible(false);
        m_outer->setVisible(false);

        m_mainLayer->addChild(m_barBg, 1);
        m_mainLayer->addChild(m_barFg, 2);
        m_mainLayer->addChild(m_border, 4);
        m_mainLayer->addChild(m_outer, 0);

        m_titleLabel = CCLabelBMFont::create("", "goldFont.fnt");
        m_titleLabel->setScale(0.6f);
        m_titleLabel->setPosition({ winSize.width / 2, winSize.height / 2 + 60 });
        m_mainLayer->addChild(m_titleLabel, 5);

        m_dayText = CCLabelBMFont::create("", "goldFont.fnt");
        m_dayText->setScale(0.5f);
        m_dayText->setPosition({ winSize.width / 2, winSize.height / 2 - 35 });
        m_mainLayer->addChild(m_dayText, 5);

        updateDisplay();
        return true;
    }

    void updateDisplay() {
        if (m_streakBadges.empty()) return;
        auto winSize = m_mainLayer->getContentSize();
        g_streakData.load();

        if (m_currentGoalIndex >= m_streakBadges.size()) {
            m_currentGoalIndex = 0;
        }
        auto& badge = m_streakBadges[m_currentGoalIndex];
        int currentDays = g_streakData.isBadgeUnlocked(badge.badgeID) ? badge.daysRequired : std::min(g_streakData.currentStreak, badge.daysRequired);
        float percent = badge.daysRequired > 0 ? currentDays / static_cast<float>(badge.daysRequired) : 0.f;

        m_titleLabel->setString(CCString::createWithFormat("Progress to %d Days", badge.daysRequired)->getCString());
        float barWidth = 180.0f;
        float barHeight = 16.0f;
        m_barBg->setContentSize({ barWidth, barHeight });
        m_barBg->setPosition({ winSize.width / 2 - barWidth / 2, winSize.height / 2 - 10 });
        m_barBg->setVisible(true);
        m_barFg->setContentSize({ barWidth * percent, barHeight });
        m_barFg->setPosition({ winSize.width / 2 - barWidth / 2, winSize.height / 2 - 10 });
        m_barFg->setVisible(true);
        m_border->setContentSize({ barWidth + 2, barHeight + 2 });
        m_border->setPosition({ winSize.width / 2 - barWidth / 2 - 1, winSize.height / 2 - 11 });
        m_border->setVisible(true);
        m_border->setOpacity(120);
        m_outer->setContentSize({ barWidth + 6, barHeight + 6 });
        m_outer->setPosition({ winSize.width / 2 - barWidth / 2 - 3, winSize.height / 2 - 13 });
        m_outer->setVisible(true);
        m_outer->setOpacity(70);

        if (m_rewardSprite) {
            m_rewardSprite->removeFromParent();
            m_rewardSprite = nullptr;
        }
        m_rewardSprite = CCSprite::create(badge.spriteName.c_str());
        if (m_rewardSprite) {
            m_rewardSprite->setScale(0.25f);
            m_rewardSprite->setPosition({ winSize.width / 2, winSize.height / 2 + 30 });
            m_mainLayer->addChild(m_rewardSprite, 5);
        }
        m_dayText->setString(CCString::createWithFormat("Day %d / %d", currentDays, badge.daysRequired)->getCString());
    }

    void onNextGoal(CCObject*) {
        if (m_streakBadges.empty()) return;
        m_currentGoalIndex = (m_currentGoalIndex + 1) % m_streakBadges.size();
        updateDisplay();
    }

    void onPreviousGoal(CCObject*) {
        if (m_streakBadges.empty()) return;
        m_currentGoalIndex = (m_currentGoalIndex - 1 + m_streakBadges.size()) % m_streakBadges.size();
        updateDisplay();
    }

public:
    static DayProgressPopup* create() {
        auto ret = new DayProgressPopup();
        if (ret && ret->initAnchored(300.f, 180.f)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};





class MissionsPopup : public Popup<> {
protected:
    int m_currentPage = 0;
    CCLayer* m_pageContainer;
    CCMenuItemSpriteExtra* m_leftArrow;
    CCMenuItemSpriteExtra* m_rightArrow;
    bool m_isAnimating = false;

    CCNode* createPointMissionNode(int missionID) {
        int targetPoints, reward;
        bool isClaimed;

        switch (missionID) {
        case 0: targetPoints = 5;  reward = 2; isClaimed = g_streakData.pointMission1Claimed; break;
        case 1: targetPoints = 10; reward = 3; isClaimed = g_streakData.pointMission2Claimed; break;
        case 2: targetPoints = 15; reward = 5; isClaimed = g_streakData.pointMission3Claimed; break;
        case 3: targetPoints = 20; reward = 8; isClaimed = g_streakData.pointMission4Claimed; break;
        case 4: targetPoints = 25; reward = 9; isClaimed = g_streakData.pointMission5Claimed; break;
        case 5: targetPoints = 30; reward = 10; isClaimed = g_streakData.pointMission6Claimed; break;
        default: return nullptr;
        }

        bool isComplete = g_streakData.streakPointsToday >= targetPoints;
        auto container = cocos2d::extension::CCScale9Sprite::create("GJ_square01.png");
        container->setContentSize({ 250.f, 45.f });
        auto missionIcon = CCSprite::create("streak_point.png"_spr);
        missionIcon->setScale(0.25f);
        missionIcon->setPosition({ 20.f, 28.f });
        container->addChild(missionIcon);
        auto descLabel = CCLabelBMFont::create(CCString::createWithFormat("Get %d Points", targetPoints)->getCString(), "goldFont.fnt");
        descLabel->setScale(0.45f);
        descLabel->setAnchorPoint({ 0, 0.5f });
        descLabel->setPosition({ 40.f, 28.f });
        container->addChild(descLabel);
        float barWidth = 120.f;
        float barHeight = 8.f;
        CCPoint barPosition = { descLabel->getPositionX(), descLabel->getPositionY() - 20.f };
        auto barBg = CCLayerColor::create({ 0, 0, 0, 120 });
        barBg->setContentSize({ barWidth, barHeight });
        barBg->setPosition(barPosition);
        container->addChild(barBg);
        float progressPercent = std::min(1.f, static_cast<float>(g_streakData.streakPointsToday) / targetPoints);
        if (progressPercent > 0.f) {
            auto barFill = CCLayerColor::create({ 120, 255, 120, 255 });
            barFill->setContentSize({ barWidth * progressPercent, barHeight });
            barFill->setPosition(barPosition);
            container->addChild(barFill);
        }
        auto progressLabel = CCLabelBMFont::create(CCString::createWithFormat("%d/%d", std::min(g_streakData.streakPointsToday, targetPoints), targetPoints)->getCString(), "bigFont.fnt");
        progressLabel->setScale(0.4f);
        progressLabel->setPosition(barPosition + CCPoint(barWidth / 2, barHeight / 2));
        container->addChild(progressLabel);

        if (isClaimed) {
            container->setOpacity(100);
            auto claimedLabel = CCLabelBMFont::create("CLAIMED", "goldFont.fnt");
            claimedLabel->setScale(0.7f);
            claimedLabel->setPosition(container->getContentSize() / 2);
            container->addChild(claimedLabel);
        }
        else if (isComplete) {
            auto claimBtnSprite = ButtonSprite::create("Claim");
            claimBtnSprite->setScale(0.7f);
            auto claimBtn = CCMenuItemSpriteExtra::create(claimBtnSprite, this, menu_selector(MissionsPopup::onClaimReward));
            claimBtn->setTag(missionID);
            auto menu = CCMenu::createWithItem(claimBtn);
            menu->setPosition({ 215.f, 22.5f });
            container->addChild(menu);
        }
        else {
            auto rewardSprite = CCSprite::create("super_star.png"_spr);
            rewardSprite->setScale(0.2f);
            rewardSprite->setPosition({ 205.f, 22.5f });
            container->addChild(rewardSprite);
            auto rewardLabel = CCLabelBMFont::create(CCString::createWithFormat("x%d", reward)->getCString(), "bigFont.fnt");
            rewardLabel->setScale(0.5f);
            rewardLabel->setAnchorPoint({ 0, 0.5f });
            rewardLabel->setPosition({ 218.f, 22.5f });
            container->addChild(rewardLabel);
        }
        return container;
    }

    bool setup() override {
        this->setTitle("Missions");
        auto winSize = m_mainLayer->getContentSize();
        g_streakData.load();

        auto background = cocos2d::extension::CCScale9Sprite::create("square02_001.png");
        background->setColor({ 0, 0, 0 });
        background->setOpacity(120);
        background->setContentSize({ 280.f, 160.f });
        background->setPosition({ winSize.width / 2, winSize.height / 2 - 15.f });
        m_mainLayer->addChild(background);

        m_pageContainer = CCLayer::create();
        m_pageContainer->setPosition(background->getPosition()); 
     
        m_mainLayer->addChild(m_pageContainer);

        auto leftSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
        m_leftArrow = CCMenuItemSpriteExtra::create(leftSpr, this, menu_selector(MissionsPopup::onSwitchPage));
        m_leftArrow->setTag(-1);

        auto rightSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
        rightSpr->setFlipX(true);
        m_rightArrow = CCMenuItemSpriteExtra::create(rightSpr, this, menu_selector(MissionsPopup::onSwitchPage));
        m_rightArrow->setTag(1);

        CCArray* navItems = CCArray::create();
        navItems->addObject(m_leftArrow);
        navItems->addObject(m_rightArrow);
        auto navMenu = CCMenu::createWithArray(navItems);
        navMenu->alignItemsHorizontallyWithPadding(background->getContentSize().width + 25.f);
        navMenu->setPosition(background->getPosition());
        m_mainLayer->addChild(navMenu);

        updatePage();

        auto starSprite = CCSprite::create("super_star.png"_spr);
        starSprite->setScale(0.18f);
        starSprite->setID("super-star-icon");
        starSprite->setAnchorPoint({ 0.f, 0.5f });
        m_mainLayer->addChild(starSprite);
        auto countLabel = CCLabelBMFont::create(std::to_string(g_streakData.superStars).c_str(), "goldFont.fnt");
        countLabel->setScale(0.5f);
        countLabel->setID("super-star-label");
        countLabel->setAnchorPoint({ 0.f, 0.5f });
        m_mainLayer->addChild(countLabel);
        CCPoint const counterPos = { 25.f, winSize.height - 25.f };
        starSprite->setPosition(counterPos);
        countLabel->setPosition(starSprite->getPosition() + CCPoint{ starSprite->getScaledContentSize().width + 5.f, 0 });

        return true;
    }

    std::vector<int> getAvailableMissionIDs() {
        std::vector<int> ids;
        if (!g_streakData.pointMission1Claimed) ids.push_back(0);
        if (!g_streakData.pointMission2Claimed) ids.push_back(1);
        if (!g_streakData.pointMission3Claimed) ids.push_back(2);
        if (!g_streakData.pointMission4Claimed) ids.push_back(3);
        if (!g_streakData.pointMission5Claimed) ids.push_back(4);
        if (!g_streakData.pointMission6Claimed) ids.push_back(5);
        return ids;
    }

    void updatePage() {
        m_pageContainer->removeAllChildren();
        auto availableMissions = getAvailableMissionIDs();

        if (availableMissions.empty()) {
            m_leftArrow->setVisible(false);
            m_rightArrow->setVisible(false);

            auto allDoneText = "You have claimed all missions.\nCome back tomorrow.";
            auto allDoneLabel = CCLabelBMFont::create(allDoneText, "bigFont.fnt");
            allDoneLabel->setScale(0.5f);
            allDoneLabel->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);

           
            allDoneLabel->setPosition({ 0.f, 0.f });

            allDoneLabel->setID("all-done-label");
            m_pageContainer->addChild(allDoneLabel);
            return;
        }
       

        int missionsPerPage = 3;
        int startIndex = m_currentPage * missionsPerPage;

        float topY = 50.f;
        float spacing = 50.f; 

        for (int i = 0; i < missionsPerPage; ++i) {
            int missionIndex = startIndex + i;
            if (missionIndex < availableMissions.size()) {
                int missionID = availableMissions[missionIndex];
                if (auto missionNode = createPointMissionNode(missionID)) {
                   
                    missionNode->setPosition(0, topY - (i * spacing));
                    missionNode->setTag(missionID);
                    m_pageContainer->addChild(missionNode);
                }
            }
        }

        int totalPages = static_cast<int>(ceil(static_cast<float>(availableMissions.size()) / missionsPerPage));
        if (totalPages == 0) totalPages = 1;

        if (m_currentPage >= totalPages && m_currentPage > 0) {
            m_currentPage = totalPages - 1;
        }

        m_leftArrow->setVisible(m_currentPage > 0);
        m_rightArrow->setVisible(m_currentPage < totalPages - 1);
    }


    void onSwitchPage(CCObject* sender) {
        if (m_isAnimating) return;
        int direction = sender->getTag();
        m_currentPage += direction;
        updatePage();
    }

    void onClaimReward(CCObject* sender) {
        if (m_isAnimating) return;
        m_isAnimating = true;

        FMODAudioEngine::sharedEngine()->playEffect("claim_mission.mp3"_spr);

        int missionID = sender->getTag();
        this->setTag(missionID);

        auto claimedNode = m_pageContainer->getChildByTag(missionID);
        if (!claimedNode) {
            m_isAnimating = false;
            return;
        }

       
        claimedNode->runAction(CCSequence::create(
            CCSpawn::create(CCEaseSineIn::create(CCMoveBy::create(0.3f, { 20, 0 })), CCFadeOut::create(0.3f), nullptr),
            CCRemoveSelf::create(),
            nullptr
        ));

        this->runAction(CCSequence::create(
            CCDelayTime::create(0.4f),
            CCCallFunc::create(this, callfunc_selector(MissionsPopup::onAnimationEnd)),
            nullptr
        ));
    }

    void onAnimationEnd() {
        int missionID = this->getTag();

       
        switch (missionID) {
        case 0: g_streakData.superStars += 2; g_streakData.pointMission1Claimed = true; break;
        case 1: g_streakData.superStars += 3; g_streakData.pointMission2Claimed = true; break;
        case 2: g_streakData.superStars += 5; g_streakData.pointMission3Claimed = true; break;
        case 3: g_streakData.superStars += 8; g_streakData.pointMission4Claimed = true; break;
        case 4: g_streakData.superStars += 9; g_streakData.pointMission5Claimed = true; break;
        case 5: g_streakData.superStars += 10; g_streakData.pointMission6Claimed = true; break;
        }

       
        g_streakData.save();

       
        auto countLabel = static_cast<CCLabelBMFont*>(this->m_mainLayer->getChildByIDRecursive("super-star-label"));
        if (countLabel) {
            countLabel->setString(std::to_string(g_streakData.superStars).c_str());
            countLabel->runAction(CCSequence::create(
                CCScaleTo::create(0.1f, 0.7f),
                CCScaleTo::create(0.1f, 0.5f),
                nullptr
            ));
            if (auto icon = static_cast<CCSprite*>(this->m_mainLayer->getChildByIDRecursive("super-star-icon"))) {
                icon->runAction(CCSequence::create(
                    CCScaleTo::create(0.1f, 0.22f),
                    CCScaleTo::create(0.1f, 0.18f),
                    nullptr
                ));
            }
        }

       
        auto availableMissions = getAvailableMissionIDs();
        int missionsPerPage = 3;
        int startIndex = m_currentPage * missionsPerPage;
        if (startIndex >= availableMissions.size() && m_currentPage > 0) {
            m_currentPage = 0;
        }

       
        updatePage();
        m_isAnimating = false;
    }

public:
    static MissionsPopup* create() {
        auto ret = new MissionsPopup();
        if (ret && ret->initAnchored(320.f, 240.f)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

class RewardsPopup : public Popup<> {
protected:
    int m_currentCategory = 0;
    int m_currentPage = 0;
    int m_totalPages = 0;
    CCLabelBMFont* m_categoryLabel = nullptr;
    CCNode* m_badgeContainer = nullptr;
    CCMenuItemSpriteExtra* m_pageLeftArrow = nullptr;
    CCMenuItemSpriteExtra* m_pageRightArrow = nullptr;
    CCLabelBMFont* m_counterText = nullptr;
    int m_colorIndex = 0;
    float m_colorTransitionTime = 0.0f;
    std::vector<ccColor3B> m_mythicColors;
    ccColor3B m_currentColor;
    ccColor3B m_targetColor;

    bool setup() override {
        this->setTitle("Badges Collection");
        auto winSize = m_mainLayer->getContentSize();
        g_streakData.load();
        m_mythicColors = {
            ccc3(255, 0, 0), ccc3(255, 165, 0), ccc3(255, 255, 0),
            ccc3(0, 255, 0), ccc3(0, 0, 255), ccc3(75, 0, 130),
            ccc3(238, 130, 238), ccc3(255, 105, 180), ccc3(255, 215, 0),
            ccc3(192, 192, 192)
        };
        m_colorIndex = 0;
        m_currentColor = m_mythicColors[0];
        m_targetColor = m_mythicColors[1];
        m_colorTransitionTime = 0.0f;
        auto background = cocos2d::extension::CCScale9Sprite::create("square02_001.png");
        background->setColor({ 0, 0, 0 });
        background->setOpacity(120);
        background->setContentSize({ 320.f, 150.f });
        background->setPosition({ winSize.width / 2, winSize.height / 2 - 15.f });
        m_mainLayer->addChild(background);
        auto catLeftArrow = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        auto catLeftBtn = CCMenuItemSpriteExtra::create(catLeftArrow, this, menu_selector(RewardsPopup::onPreviousCategory));
        catLeftBtn->setPosition(-110.f, 0);
        auto catRightArrow = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        catRightArrow->setFlipX(true);
        auto catRightBtn = CCMenuItemSpriteExtra::create(catRightArrow, this, menu_selector(RewardsPopup::onNextCategory));
        catRightBtn->setPosition(110.f, 0);
        auto catArrowMenu = CCMenu::create();
        catArrowMenu->addChild(catLeftBtn);
        catArrowMenu->addChild(catRightBtn);
        catArrowMenu->setPosition({ winSize.width / 2, winSize.height - 40.f });
        m_mainLayer->addChild(catArrowMenu);
        m_categoryLabel = CCLabelBMFont::create("", "goldFont.fnt");
        m_categoryLabel->setScale(0.7f);
        m_categoryLabel->setPosition({ winSize.width / 2, winSize.height - 40.f });
        m_mainLayer->addChild(m_categoryLabel);
        auto pageLeftArrowSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
        m_pageLeftArrow = CCMenuItemSpriteExtra::create(pageLeftArrowSprite, this, menu_selector(RewardsPopup::onPreviousBadgePage));
        m_pageLeftArrow->setPosition(-background->getContentSize().width / 2 - 5.f, 0);
        auto pageRightArrowSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
        pageRightArrowSprite->setFlipX(true);
        m_pageRightArrow = CCMenuItemSpriteExtra::create(pageRightArrowSprite, this, menu_selector(RewardsPopup::onNextBadgePage));
        m_pageRightArrow->setPosition(background->getContentSize().width / 2 + 5.f, 0);
        auto pageArrowMenu = CCMenu::create();
        pageArrowMenu->addChild(m_pageLeftArrow);
        pageArrowMenu->addChild(m_pageRightArrow);
        pageArrowMenu->setPosition(background->getPosition());
        m_mainLayer->addChild(pageArrowMenu);
        m_badgeContainer = CCNode::create();
        m_mainLayer->addChild(m_badgeContainer);
        m_counterText = CCLabelBMFont::create("", "bigFont.fnt");
        m_counterText->setScale(0.5f);
        m_counterText->setPosition({ winSize.width / 2, 32.f });
        m_mainLayer->addChild(m_counterText);
        this->scheduleUpdate();
        updateCategoryDisplay();
        return true;
    }

    void update(float dt) override {
        StreakData::BadgeCategory currentCat = static_cast<StreakData::BadgeCategory>(m_currentCategory);
        if (currentCat != StreakData::BadgeCategory::MYTHIC || !m_categoryLabel) {
            m_categoryLabel->setColor(g_streakData.getCategoryColor(currentCat));
            return;
        }
        m_colorTransitionTime += dt;
        float transitionDuration = 1.0f;
        if (m_colorTransitionTime >= transitionDuration) {
            m_colorTransitionTime = 0.0f;
            m_colorIndex = (m_colorIndex + 1) % m_mythicColors.size();
            m_currentColor = m_targetColor;
            m_targetColor = m_mythicColors[(m_colorIndex + 1) % m_mythicColors.size()];
        }
        float progress = m_colorTransitionTime / transitionDuration;
        ccColor3B interpolatedColor = {
            static_cast<GLubyte>(m_currentColor.r + (m_targetColor.r - m_currentColor.r) * progress),
            static_cast<GLubyte>(m_currentColor.g + (m_targetColor.g - m_currentColor.g) * progress),
            static_cast<GLubyte>(m_currentColor.b + (m_targetColor.b - m_currentColor.b) * progress)
        };
        m_categoryLabel->setColor(interpolatedColor);
    }

    void updateCategoryDisplay() {
        m_badgeContainer->removeAllChildren();
        StreakData::BadgeCategory currentCat = static_cast<StreakData::BadgeCategory>(m_currentCategory);
        std::vector<StreakData::BadgeInfo> categoryBadges;
        for (auto& badge : g_streakData.badges) {
            if (badge.category == currentCat) {
                categoryBadges.push_back(badge);
            }
        }
        std::string categoryName = g_streakData.getCategoryName(currentCat);
        m_categoryLabel->setString(categoryName.c_str());
        const int badgesPerPage = 6;
        m_totalPages = static_cast<int>(ceil(static_cast<float>(categoryBadges.size()) / badgesPerPage));
        if (m_currentPage >= m_totalPages && m_totalPages > 0) {
            m_currentPage = m_totalPages - 1;
        }
        int startIndex = m_currentPage * badgesPerPage;
        int endIndex = std::min(static_cast<int>(categoryBadges.size()), startIndex + badgesPerPage);
        const int badgesPerRow = 3;
        const float horizontalSpacing = 65.f;
        const float verticalSpacing = 65.f;
        auto winSize = m_mainLayer->getContentSize();
        const CCPoint startPosition = { winSize.width / 2 - horizontalSpacing, winSize.height / 2 + 10.f };
        for (int i = startIndex; i < endIndex; ++i) {
            int indexOnPage = i - startIndex;
            int row = indexOnPage / badgesPerRow;
            int col = indexOnPage % badgesPerRow;
            auto& badge = categoryBadges[i];
            auto badgeSprite = CCSprite::create(badge.spriteName.c_str());
            badgeSprite->setScale(0.3f);
            bool unlocked = g_streakData.isBadgeUnlocked(badge.badgeID);
            if (!unlocked) {
                badgeSprite->setColor({ 100, 100, 100 });
            }
            CCMenuItemSpriteExtra* badgeBtn;
            if (unlocked) {
                badgeBtn = CCMenuItemSpriteExtra::create(badgeSprite, this, menu_selector(RewardsPopup::onBadgeClick));
                badgeBtn->setUserObject(CCString::create(badge.badgeID));
            }
            else {
                badgeBtn = CCMenuItemSpriteExtra::create(badgeSprite, this, nullptr);
                badgeBtn->setEnabled(false);
            }
            CCPoint position = { startPosition.x + col * horizontalSpacing, startPosition.y - row * verticalSpacing };
            auto badgeMenu = CCMenu::createWithItem(badgeBtn);
            badgeMenu->setPosition(position);
            m_badgeContainer->addChild(badgeMenu);
            if (!badge.isFromRoulette) {
                auto daysLabel = CCLabelBMFont::create(CCString::createWithFormat("%d days", badge.daysRequired)->getCString(), "goldFont.fnt");
                daysLabel->setScale(0.35f);
                daysLabel->setPosition(position - CCPoint(0, 30.f));
                m_badgeContainer->addChild(daysLabel);
            }
            if (!unlocked) {
                auto lockIcon = CCSprite::createWithSpriteFrameName("GJ_lock_001.png");
                lockIcon->setPosition(position);
                m_badgeContainer->addChild(lockIcon, 5);
            }
        }
        m_pageLeftArrow->setVisible(m_currentPage > 0);
        m_pageRightArrow->setVisible(m_currentPage < m_totalPages - 1);
        int unlockedCount = 0;
        for (auto& badge : categoryBadges) {
            if (g_streakData.isBadgeUnlocked(badge.badgeID)) {
                unlockedCount++;
            }
        }
        m_counterText->setString(CCString::createWithFormat("Unlocked: %d/%d", unlockedCount, static_cast<int>(categoryBadges.size()))->getCString());
    }

    void onBadgeClick(CCObject* sender) {
        auto badgeID = static_cast<CCString*>(static_cast<CCNode*>(sender)->getUserObject())->getCString();
        EquipBadgePopup::create(badgeID)->show();
    }

    void onNextBadgePage(CCObject*) {
        if (m_currentPage < m_totalPages - 1) {
            m_currentPage++;
            updateCategoryDisplay();
        }
    }

    void onPreviousBadgePage(CCObject*) {
        if (m_currentPage > 0) {
            m_currentPage--;
            updateCategoryDisplay();
        }
    }

    void onNextCategory(CCObject*) {
        m_currentCategory = (m_currentCategory + 1) % 5;
        m_currentPage = 0;
        updateCategoryDisplay();
    }

    void onPreviousCategory(CCObject*) {
        m_currentCategory = (m_currentCategory - 1 + 5) % 5;
        m_currentPage = 0;
        updateCategoryDisplay();
    }

    void onClose(CCObject* sender) override {
        this->unscheduleUpdate();
        Popup::onClose(sender);
    }

public:
    static RewardsPopup* create() {
        auto ret = new RewardsPopup();
        if (ret && ret->initAnchored(360.f, 250.f)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

class ManualParticleEmitter : public cocos2d::CCNode {
protected:
    float m_spawnTimer = 0.0f;
    float m_spawnRate = 0.005f;
    bool m_emitting = true;

public:
    void update(float dt) override {
        if (!m_emitting) {
            if (this->getChildrenCount() == 0) {
                this->unscheduleUpdate();
                return;
            }
            return;
        }
        m_spawnTimer += dt;
        if (m_spawnTimer >= m_spawnRate) {
            m_spawnTimer = 0.0f;
            spawnParticle();
        }
    }

    void spawnParticle() {
        auto particle = CCSprite::create("cuadro.png"_spr);
        if (!particle) return;
        particle->setPosition({ 0, 0 });
        particle->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });
        std::vector<ccColor3B> colors = { {255, 255, 255}, {255, 215, 0}, {255, 165, 0}, {255, 255, 150} };
        particle->setColor(colors[rand() % colors.size()]);
        particle->setOpacity(200);
        particle->setScale(0.05f + (static_cast<float>(rand() % 10) / 100.0f));
        float angle = static_cast<float>(rand() % 360);
        float speed = 30.0f + (rand() % 20);
        float lifespan = 0.3f + (static_cast<float>(rand() % 20) / 100.0f);
        auto move = CCEaseExponentialOut::create(
            CCMoveBy::create(lifespan, {
                speed * cos(CC_DEGREES_TO_RADIANS(angle)),
                speed * sin(CC_DEGREES_TO_RADIANS(angle))
                })
        );
        auto fadeOut = CCFadeOut::create(lifespan);
        auto scaleDown = CCScaleTo::create(lifespan, 0.0f);
        auto rotate = CCRotateBy::create(lifespan, (rand() % 180) - 90);
        particle->runAction(CCSequence::create(
            CCSpawn::create(move, fadeOut, scaleDown, rotate, nullptr),
            CCRemoveSelf::create(),
            nullptr
        ));
        this->addChild(particle);
    }

    void stopEmitting() {
        m_emitting = false;
    }

    static ManualParticleEmitter* create() {
        auto ret = new ManualParticleEmitter();
        if (ret && ret->init()) {
            ret->autorelease();
            ret->scheduleUpdate();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

CCAction* createShakeAction(float duration, float strength) {
    auto shake = CCArray::create();
    int steps = static_cast<int>(duration * 30);
    for (int i = 0; i < steps; ++i) {
        float dx = (rand() % 2 == 0 ? 1 : -1) * (static_cast<float>(rand() % 100) / 100.f) * strength;
        float dy = (rand() % 2 == 0 ? 1 : -1) * (static_cast<float>(rand() % 100) / 100.f) * strength;
        shake->addObject(CCMoveBy::create(duration / steps, ccp(dx, dy)));
    }
    shake->addObject(CCMoveTo::create(0.f, ccp(0, 0)));
    return CCSequence::create(shake);
}

class InfoPopup : public Popup<> {
protected:
   
    CCLabelBMFont* m_streakLabel = nullptr;
    CCLayerGradient* m_barFg = nullptr;
    CCLabelBMFont* m_barText = nullptr;
    CCSprite* m_rachaIndicator = nullptr;

   
    bool setup() override;

  
public:
    void updateDisplay();

protected:
    
    void showStreakAnimation(int streakLevel);
    void onAnimationExit();
    void onOpenHistory(CCObject*);
    void onOpenStats(CCObject*);
    void onOpenRewards(CCObject*);
    void onRachaClick(CCObject*);
    void onOpenMissions(CCObject*);
    void onRedeemCode(CCObject*);
    void onInfo(CCObject*);
    void onOpenRoulette(CCObject*);

public:
   
    static InfoPopup* create();
};


bool InfoPopup::setup() {

    this->setTitle("My Streak");
    auto winSize = m_mainLayer->getContentSize();
    float centerY = winSize.height / 2 + 25;

  
    auto rachaSprite = CCSprite::create(g_streakData.getRachaSprite().c_str());
    rachaSprite->setScale(0.4f);
    auto rachaBtn = CCMenuItemSpriteExtra::create(rachaSprite, this, menu_selector(InfoPopup::onRachaClick));
    auto menuRacha = CCMenu::createWithItem(rachaBtn);
    menuRacha->setPosition({ winSize.width / 2, centerY });
    m_mainLayer->addChild(menuRacha, 3);
    rachaSprite->runAction(CCRepeatForever::create(CCSequence::create(CCMoveBy::create(1.5f, { 0, 8 }), CCMoveBy::create(1.5f, { 0, -8 }), nullptr)));

  
    m_streakLabel = CCLabelBMFont::create("Daily streak: ?", "goldFont.fnt");
    m_streakLabel->setScale(0.55f);
    m_streakLabel->setPosition({ winSize.width / 2, centerY - 60 });
    m_mainLayer->addChild(m_streakLabel);

    float barWidth = 140.0f;
    float barHeight = 16.0f;
    auto barBg = CCLayerColor::create({ 45, 45, 45, 255 }, barWidth, barHeight);
    barBg->setPosition({ winSize.width / 2 - barWidth / 2, centerY - 90 });
    m_mainLayer->addChild(barBg, 1);
    m_barFg = CCLayerGradient::create({ 250, 225, 60, 255 }, { 255, 165, 0, 255 });
    m_barFg->setContentSize({ 0, barHeight });
    m_barFg->setPosition({ winSize.width / 2 - barWidth / 2, centerY - 90 });
    m_mainLayer->addChild(m_barFg, 2);
    auto border = CCLayerColor::create({ 255, 255, 255, 120 }, barWidth + 2, barHeight + 2);
    border->setPosition({ winSize.width / 2 - barWidth / 2 - 1, centerY - 91 });
    m_mainLayer->addChild(border, 4);
    auto outer = CCLayerColor::create({ 0, 0, 0, 70 }, barWidth + 6, barHeight + 6);
    outer->setPosition({ winSize.width / 2 - barWidth / 2 - 3, centerY - 93 });
    m_mainLayer->addChild(outer, 0);
    auto pointIcon = CCSprite::create("streak_point.png"_spr);
    pointIcon->setScale(0.12f);
    pointIcon->setPosition({ winSize.width / 2 - 35, centerY - 82 });
    m_mainLayer->addChild(pointIcon, 5);
    m_barText = CCLabelBMFont::create("? / ?", "bigFont.fnt");
    m_barText->setScale(0.45f);
    m_barText->setPosition({ winSize.width / 2 + 10, centerY - 82 });
    m_mainLayer->addChild(m_barText, 5);
    m_rachaIndicator = CCSprite::create("racha0.png"_spr);
    m_rachaIndicator->setScale(0.14f);
    m_rachaIndicator->setPosition({ winSize.width / 2 + barWidth / 2 + 20, centerY - 82 });
    m_mainLayer->addChild(m_rachaIndicator, 5);

    
    auto menuSide = CCMenu::create();
    menuSide->setPosition(0, 0);
    m_mainLayer->addChild(menuSide, 10);

    auto statsIcon = CCSprite::create("BtnStats.png"_spr);
    statsIcon->setScale(0.7f);
    auto statsBtn = CCMenuItemSpriteExtra::create(statsIcon, this, menu_selector(InfoPopup::onOpenStats));
    statsBtn->setPosition({ winSize.width - 22, centerY });
    menuSide->addChild(statsBtn);

    auto rewardsIcon = CCSprite::create("RewardsBtn.png"_spr);
    rewardsIcon->setScale(0.7f);
    auto rewardsBtn = CCMenuItemSpriteExtra::create(rewardsIcon, this, menu_selector(InfoPopup::onOpenRewards));
    rewardsBtn->setPosition({ winSize.width - 22, centerY - 37 });
    menuSide->addChild(rewardsBtn);

    auto missionsIcon = CCSprite::create("super_star_btn.png"_spr);
    missionsIcon->setScale(0.7f);
    auto missionsBtn = CCMenuItemSpriteExtra::create(missionsIcon, this, menu_selector(InfoPopup::onOpenMissions));
    missionsBtn->setPosition({ 22, centerY });
    menuSide->addChild(missionsBtn);

    auto rouletteIcon = CCSprite::create("boton_ruleta.png"_spr);
    rouletteIcon->setScale(0.7f);
    auto rouletteBtn = CCMenuItemSpriteExtra::create(rouletteIcon, this, menu_selector(InfoPopup::onOpenRoulette));
    rouletteBtn->setPosition({ 22, centerY - 37 });
    menuSide->addChild(rouletteBtn);

    auto infoIcon = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
    infoIcon->setScale(0.6f);
    auto infoBtn = CCMenuItemSpriteExtra::create(infoIcon, this, menu_selector(InfoPopup::onInfo));
    infoBtn->setPosition({ winSize.width - 20, winSize.height - 20 });
    menuSide->addChild(infoBtn);

    auto historyIcon = CCSprite::create("historial_btn.png"_spr);
    historyIcon->setScale(0.7f);
    auto historyBtn = CCMenuItemSpriteExtra::create(historyIcon, this, menu_selector(InfoPopup::onOpenHistory));
    historyBtn->setPosition({ 20, 20 });
    menuSide->addChild(historyBtn);

    auto redeemIcon = CCSprite::create("redemcode_btn.png"_spr);
    redeemIcon->setScale(0.7f);
    auto redeemBtn = CCMenuItemSpriteExtra::create(redeemIcon, this, menu_selector(InfoPopup::onRedeemCode));
    redeemBtn->setPosition({ winSize.width - 20, 20 });
    menuSide->addChild(redeemBtn);

    
    this->updateDisplay();

    if (g_streakData.shouldShowAnimation()) {
        this->showStreakAnimation(g_streakData.currentStreak);
    }
    return true;
}

void InfoPopup::updateDisplay() {
    if (!m_streakLabel || !m_barFg || !m_barText || !m_rachaIndicator) {
        log::warn("Intentando actualizar InfoPopup antes de que sus elementos esten listos.");
        return;
    }

    int currentStreak = g_streakData.currentStreak;
    int pointsToday = g_streakData.streakPointsToday;
    int requiredPoints = g_streakData.getRequiredPoints();
    float percent = requiredPoints > 0 ? std::min(static_cast<float>(pointsToday) / requiredPoints, 1.0f) : 0.f;
    float barWidth = 140.0f;
    float barHeight = 16.0f;

    m_streakLabel->setString(fmt::format("Daily streak: {}", currentStreak).c_str());
    m_barFg->setContentSize({ barWidth * percent, barHeight });
    m_barText->setString(fmt::format("{}/{}", pointsToday, requiredPoints).c_str());

    std::string indicatorSpriteName = (pointsToday >= requiredPoints) ? g_streakData.getRachaSprite() : "racha0.png"_spr;
    if (auto newSprite = CCSprite::create(indicatorSpriteName.c_str())) {
        if (auto newTexture = newSprite->getTexture()) {
            m_rachaIndicator->setTexture(newTexture);
          
            m_rachaIndicator->setTextureRect(newSprite->getTextureRect());
        }
    }
}


void InfoPopup::onOpenHistory(CCObject*) { HistoryPopup::create()->show(); }
void InfoPopup::onOpenStats(CCObject*) { DayProgressPopup::create()->show(); }
void InfoPopup::onOpenRewards(CCObject*) { RewardsPopup::create()->show(); }
void InfoPopup::onRachaClick(CCObject*) { AllRachasPopup::create()->show(); }
void InfoPopup::onOpenMissions(CCObject*) { MissionsPopup::create()->show(); }
void InfoPopup::onRedeemCode(CCObject*) { RedeemCodePopup::create()->show(); }

void InfoPopup::onInfo(CCObject*) {
    std::string message =
        "Complete rated levels every day to earn <cp>Streak Points</c> and increase your streak!\n\n"
        "<cg>Point Rewards:</c>\n"
        "- <cy>Auto / Easy / Normal</c> (1-3⭐): <cl>1 Point</c>\n"
        "- <co>Hard</c> (4-5): <cl>3 Points</c>\n"
        "- <cr>Harder</c> (6-7): <cl>4 Points</c>\n"
        "- <cp>Insane</c> (8-9): <cl>5 Points</c>\n"
        "- <cj>Demon</c> (10): <cl>6 Points</c>";
    FLAlertLayer::create("About Streak!", message, "OK")->show();
}

void InfoPopup::onOpenRoulette(CCObject*) {
    if (g_streakData.currentStreak < 1) {
        FLAlertLayer::create("Roulette Locked", "You need a streak of at least <cg>1 day</c> to use the roulette.", "OK")->show();
        return;
    }
    RoulettePopup::create()->show();
}


void InfoPopup::showStreakAnimation(int streakLevel) {
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    auto animLayer = CCLayer::create();
    animLayer->setTag(111);
    this->addChild(animLayer, 1000);
    auto bg = CCLayerColor::create({ 0, 0, 0, 180 });
    bg->setOpacity(0);
    bg->runAction(CCFadeTo::create(0.3f, 180));
    animLayer->addChild(bg);
    auto rachaSprite = CCSprite::create(g_streakData.getRachaSprite().c_str());
    rachaSprite->setPosition(winSize / 2);
    rachaSprite->setScale(0.1f);
    rachaSprite->setOpacity(0);
    animLayer->addChild(rachaSprite);
    auto daysLabel = CCLabelBMFont::create(CCString::createWithFormat("Day %d!", streakLevel)->getCString(), "goldFont.fnt");
    daysLabel->setPosition({ winSize.width / 2, winSize.height / 2 - 80.f });
    daysLabel->setScale(0.1f);
    daysLabel->setOpacity(0);
    animLayer->addChild(daysLabel);
    FMODAudioEngine::sharedEngine()->playEffect("achievement.mp3"_spr);
    rachaSprite->runAction(CCSpawn::create(
        CCFadeIn::create(0.4f),
        CCEaseBackOut::create(CCScaleTo::create(0.4f, 1.0f)),
        nullptr
    ));
    daysLabel->runAction(CCSequence::create(
        CCDelayTime::create(0.2f),
        CCSpawn::create(
            CCFadeIn::create(0.4f),
            CCEaseBackOut::create(CCScaleTo::create(0.4f, 1.0f)),
            nullptr
        ),
        nullptr
    ));
    animLayer->runAction(CCSequence::create(
        CCDelayTime::create(2.5f),
        CCCallFunc::create(this, callfunc_selector(InfoPopup::onAnimationExit)),
        nullptr
    ));
}

void InfoPopup::onAnimationExit() {
    auto animLayer = this->getChildByTag(111);
    if (!animLayer) return;
    animLayer->runAction(CCSequence::create(
        CCFadeOut::create(0.5f),
        CCRemoveSelf::create(),
        nullptr
    ));
}

InfoPopup* InfoPopup::create() {
    auto ret = new InfoPopup();
    if (ret && ret->initAnchored(260.f, 220.f)) { 
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

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
        m_currentPercent = std::min(1.f, static_cast<float>(m_pointsBefore) / m_pointsRequired);
        m_targetPercent = m_currentPercent;
        m_currentPointsDisplay = static_cast<float>(m_pointsBefore);
        m_targetPointsDisplay = m_currentPointsDisplay;
        auto winSize = cocos2d::CCDirector::sharedDirector()->getWinSize();
        m_barWidth = 160.0f;
        m_barHeight = 15.0f;
        m_barContainer = CCNode::create();
        m_barContainer->setPosition(30, winSize.height - 280);
        this->addChild(m_barContainer);
        auto streakIcon = CCSprite::create(g_streakData.getRachaSprite().c_str());
        streakIcon->setScale(0.2f);
        streakIcon->setRotation(-15.f);
        streakIcon->setPosition({ -5, (m_barHeight + 6) / 2 });
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
        m_barFg = CCLayerGradient::create({ 255, 225, 60, 255 }, { 255, 165, 0, 255 });
        m_barFg->setContentSize({ m_barWidth * m_currentPercent, m_barHeight });
        m_barFg->setAnchorPoint({ 0, 0 });
        m_barFg->setPosition({ 0, 0 });
        clipper->addChild(m_barFg);
        m_pointLabel = CCLabelBMFont::create(
            CCString::createWithFormat("%d/%d", m_pointsBefore, m_pointsRequired)->getCString(), "bigFont.fnt"
        );
        m_pointLabel->setAnchorPoint({ 1, 0.5f });
        m_pointLabel->setScale(0.4f);
        m_pointLabel->setPosition({ m_barFg->getContentSize().width - 5, m_barHeight / 2 });
        m_barFg->addChild(m_pointLabel, 5);
        this->runAnimations();
        this->scheduleUpdate();
        return true;
    }

    void update(float dt) override {
        float smoothingFactor = 8.0f;
        m_currentPercent = m_currentPercent + (m_targetPercent - m_currentPercent) * dt * smoothingFactor;
        m_currentPointsDisplay = m_currentPointsDisplay + (m_targetPointsDisplay - m_currentPointsDisplay) * dt * smoothingFactor;
        m_barFg->setContentSize({ m_barWidth * m_currentPercent, m_barHeight });
        m_pointLabel->setString(CCString::createWithFormat("%d/%d", static_cast<int>(round(m_currentPointsDisplay)), m_pointsRequired)->getCString());
        m_pointLabel->setPosition({ m_barFg->getContentSize().width - 5, m_barHeight / 2 });
    }

    void runAnimations() {
        CCPoint onScreenPos = m_barContainer->getPosition();
        CCPoint offScreenPos = m_barContainer->getPosition() + CCPoint(-250, 0);
        m_barContainer->setPosition(offScreenPos);
        m_barContainer->runAction(CCSequence::create(
            CCEaseSineOut::create(CCMoveTo::create(0.4f, onScreenPos)),
            CCCallFunc::create(this, callfunc_selector(StreakProgressBar::spawnPointParticles)),
            CCDelayTime::create(2.5f + m_pointsGained * 0.15f),
            CCEaseSineIn::create(CCMoveTo::create(0.4f, offScreenPos)),
            CCCallFunc::create(this, callfunc_selector(StreakProgressBar::stopUpdateLoop)),
            CCRemoveSelf::create(),
            nullptr
        ));
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
            CCPoint endPos = m_barContainer->getPosition() + CCPoint(3 + m_barWidth * (std::min(1.f, (float)(m_pointsBefore + i + 1) / m_pointsRequired)), 3 + m_barHeight / 2);
            ccBezierConfig bezier;
            bezier.endPosition = endPos;
            float explosionRadius = 150.f;
            float randomAngle = (float)(rand() % 360);
            bezier.controlPoint_1 = center + CCPoint((explosionRadius + (rand() % 50)) * cos(CC_DEGREES_TO_RADIANS(randomAngle)), (explosionRadius + (rand() % 50)) * sin(CC_DEGREES_TO_RADIANS(randomAngle)));
            bezier.controlPoint_2 = endPos + CCPoint(0, 100);
            auto bezierTo = CCBezierTo::create(1.0f, bezier);
            auto rotateAction = CCRotateBy::create(1.0f, 360 + (rand() % 180));
            auto scaleAction = CCScaleTo::create(1.0f, 0.1f);
            auto pointIndexObj = CCInteger::create(i + 1);
            pointParticle->runAction(CCSequence::create(
                CCDelayTime::create(i * delayPerPoint),
                CCSpawn::create(bezierTo, rotateAction, scaleAction, nullptr),
                CCCallFuncO::create(this, callfuncO_selector(StreakProgressBar::onPointHitBar), pointIndexObj),
                CCRemoveSelf::create(),
                nullptr
            ));
        }
    }

    void onPointHitBar(CCObject* pointIndexObj) {
        CCInteger* indexInt = static_cast<CCInteger*>(pointIndexObj);
        int pointIndex = indexInt->getValue();
        int currentTotalPoints = m_pointsBefore + pointIndex;
        m_targetPercent = std::min(1.f, static_cast<float>(currentTotalPoints) / m_pointsRequired);
        m_targetPointsDisplay = static_cast<float>(currentTotalPoints);
        auto popUp = CCEaseSineOut::create(CCScaleTo::create(0.1f, 1.0f, 1.2f));
        auto popDown = CCEaseSineIn::create(CCScaleTo::create(0.1f, 1.0f, 1.0f));
        m_barFg->runAction(CCSequence::create(popUp, popDown, nullptr));
        float currentBarWidth = m_barWidth * m_currentPercent;
        CCPoint flashPosition = m_barContainer->getPosition() + CCPoint(3 + currentBarWidth, 3 + m_barHeight / 2);
        auto flash = CCSprite::create("streak_point.png"_spr);
        flash->setPosition(flashPosition);
        flash->setScale(0.05f);
        flash->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });
        flash->runAction(CCSequence::create(
            CCSpawn::create(CCScaleTo::create(0.3f, 0.4f), CCFadeOut::create(0.3f), nullptr),
            CCRemoveSelf::create(),
            nullptr
        ));
        this->addChild(flash, 20);
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