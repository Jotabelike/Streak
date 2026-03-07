#pragma once
#include "StreakCommon.h"
#include <Geode/utils/cocos.hpp>

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

        m_counterLabel = CCLabelBMFont::create(
            std::to_string(m_initialTickets).c_str(),
            "goldFont.fnt"
        );
        m_counterLabel->setScale(0.6f);
        m_counterBG->addChild(m_counterLabel);

        m_counterBG->setContentSize({
            m_counterLabel->getScaledContentSize().width + ticketSprite->getScaledContentSize().width + 25.f,
            40.f
            });

        ticketSprite->setPosition({
            m_counterBG->getContentSize().width - ticketSprite->getScaledContentSize().width / 2 - 10.f,
            m_counterBG->getContentSize().height / 2
            });

        m_counterLabel->setPosition({
            m_counterLabel->getScaledContentSize().width / 2 + 10.f,
            m_counterBG->getContentSize().height / 2
            });

        m_counterBG->setPosition(
            winSize.width - m_counterBG->getContentSize().width / 2 - 5.f,
            winSize.height - m_counterBG->getContentSize().height / 2 - 5.f
        );

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
            bezier.controlPoint_1 = ccp(
                winSize.width / 2 + (rand() % 200) - 100,
                winSize.height / 2 + (rand() % 150) - 75
            );
            bezier.controlPoint_2 = counterPos + ccp(
                (rand() % 100) - 50,
                (rand() % 100) - 50
            );

            auto bezierTo = CCBezierTo::create(duration, bezier);
            auto scaleTo = CCScaleTo::create(duration, 0.1f);
            auto fadeOut = CCFadeOut::create(duration * 0.8f);

            particle->runAction(CCSequence::create(
                CCDelayTime::create(i * delayPerParticle),
                CCSpawn::create(
                    CCEaseSineIn::create(bezierTo),
                    scaleTo,
                    CCSequence::create(
                        CCDelayTime::create(duration * 0.2f),
                        fadeOut,
                        nullptr
                    ),
                    nullptr
                ),
                CCCallFuncN::create(
                    this,
                    callfuncN_selector(TicketAnimationLayer::onParticleHit)
                ),
                CCRemoveSelf::create(),
                nullptr
            ));
        }

        this->runAction(CCSequence::create(
            CCDelayTime::create(particleCount * delayPerParticle + duration),
            CCCallFunc::create(
                this,
                callfunc_selector(TicketAnimationLayer::onAnimationEnd)
            ),
            nullptr
        ));
    }

    void onParticleHit(CCNode* sender) {
        FMODAudioEngine::sharedEngine()->playEffect("coin.mp3"_spr);
        m_ticketAccumulator += m_ticketsPerParticle;

        m_counterLabel->setString(
            std::to_string(m_initialTickets + static_cast<int>(m_ticketAccumulator)).c_str()
        );

        m_counterLabel->runAction(CCSequence::create(
            CCScaleTo::create(0.1f, 0.8f),
            CCScaleTo::create(0.1f, 0.6f),
            nullptr
        ));
    }

    void runSlideOffAnimation() {
        auto slideOff = CCEaseSineIn::create(
            CCMoveBy::create(
                0.4f,
                { m_counterBG->getContentSize().width + 10.f, 0 }
            )
        );
        m_counterBG->runAction(slideOff);
    }

    void onAnimationEnd() {
        m_counterLabel->setString(std::to_string(g_streakData.starTickets).c_str());
        this->runAction(CCSequence::create(
            CCDelayTime::create(0.5f),
            CCCallFunc::create(
                this,
                callfunc_selector(TicketAnimationLayer::runSlideOffAnimation)
            ),
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

class RewardCycleNode : public CCNode {
    struct RewardItem {
        std::string spriteName;
        std::string text;
        float scale;
    };
    std::vector<RewardItem> m_items;
    int m_currentIndex = 0;

    CCSprite* m_icon = nullptr;
    CCLabelBMFont* m_label = nullptr;
    CCNode* m_contentNode = nullptr;

public:
    static RewardCycleNode* create(float width, float height) {
        auto ret = new RewardCycleNode();
        if (ret && ret->init()) {
            ret->ignoreAnchorPointForPosition(false);
            ret->setContentSize({ width, height });
            ret->setAnchorPoint({ 0.5f, 0.5f });

            ret->setupContent();
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    void setupContent() {
        m_contentNode = CCNode::create();
        m_contentNode->setContentSize(m_obContentSize);
        m_contentNode->setAnchorPoint({ 0.5f, 0.5f });
        m_contentNode->setPosition(m_obContentSize / 2);
        this->addChild(m_contentNode);

        m_icon = CCSprite::create("empty.png");
        if (!m_icon) {
            m_icon = CCSprite::create();
        }
        m_icon->setPosition({
            m_obContentSize.width / 2,
            m_obContentSize.height / 2 + 6.f
            });
        m_contentNode->addChild(m_icon);

        m_label = CCLabelBMFont::create("", "goldFont.fnt");
        m_label->setScale(0.35f);
        m_label->setPosition({ m_obContentSize.width / 2, 8.f });
        m_contentNode->addChild(m_label);
    }

    void addReward(std::string sprite, std::string text, float scale) {
        m_items.push_back({ sprite, text, scale });
        if (m_items.size() == 1) {
            updateVisuals(0);
        }
        if (m_items.size() == 2) {
            this->schedule(schedule_selector(RewardCycleNode::cycleLoop), 2.0f);
        }
    }

    void updateVisuals(int index) {
        if (index < 0 || index >= m_items.size()) return;
        auto& item = m_items[index];

        auto newTexture = CCSprite::create(item.spriteName.c_str());
        if (!newTexture) {
            newTexture = CCSprite::createWithSpriteFrameName(item.spriteName.c_str());
        }

        if (newTexture && m_icon) {
            m_icon->setTexture(newTexture->getTexture());
            m_icon->setTextureRect(newTexture->getTextureRect());
            m_icon->setScale(item.scale);
            m_icon->setVisible(true);
        }

        if (m_label) {
            m_label->setString(item.text.c_str());
        }
    }

    void cycleLoop(float dt) {
        auto fadeOut = CCFadeOut::create(0.3f);
        auto changeData = CCCallFunc::create(
            this,
            callfunc_selector(RewardCycleNode::swapItem)
        );

        auto fadeIn = CCFadeIn::create(0.3f);
        m_contentNode->runAction(CCSequence::create(
            fadeOut,
            changeData,
            fadeIn,
            nullptr
        ));
    }

    void swapItem() {
        m_currentIndex = (m_currentIndex + 1) % m_items.size();
        updateVisuals(m_currentIndex);
    }
};

class GenericPrizePopup : public Popup {
protected:
    std::function<void()> m_onCloseCallback = nullptr;

    void onClose(CCObject* sender) override {
        if (m_onCloseCallback) {
            m_onCloseCallback();
        }
        Popup::onClose(sender);
    }

    bool init(GenericPrizeResult prize, std::function<void()> onCloseCallback) {
        if (!Popup::init(280.f, 240.f)) return false;
        m_onCloseCallback = onCloseCallback;
        auto winSize = m_mainLayer->getContentSize();

        this->setTitle("You Won a Prize!");

        auto nameLabel = CCLabelBMFont::create(
            prize.displayName.c_str(),
            "goldFont.fnt"
        );
        nameLabel->setScale(0.7f);
        m_mainLayer->addChild(nameLabel);

        auto categoryLabel = CCLabelBMFont::create(
            g_streakData.getCategoryName(prize.category).c_str(),
            "bigFont.fnt"
        );
        categoryLabel->setColor(g_streakData.getCategoryColor(prize.category));
        categoryLabel->setScale(0.5f);
        m_mainLayer->addChild(categoryLabel);

        if (prize.type == RewardType::Badge) {
            if (!prize.isNew) {
                this->setTitle("Duplicate Badge!");
                auto rewardNode = CCNode::create();
                rewardNode->setPosition({
                    winSize.width / 2,
                    winSize.height / 2 + 55.f
                    });

                m_mainLayer->addChild(rewardNode);

                auto amountLabel = CCLabelBMFont::create(
                    fmt::format("+{}", prize.ticketsFromDuplicate).c_str(),
                    "goldFont.fnt"
                );

                amountLabel->setScale(0.6f);
                rewardNode->addChild(amountLabel);

                auto ticketSprite = CCSprite::create("star_tiket.png"_spr);
                ticketSprite->setScale(0.3f);
                rewardNode->addChild(ticketSprite);

                float totalWidth = amountLabel->getScaledContentSize().width + ticketSprite->getScaledContentSize().width + 5.f;
                amountLabel->setPosition({
                    -totalWidth / 2 + amountLabel->getScaledContentSize().width / 2,
                    0
                    });
                ticketSprite->setPosition({
                    totalWidth / 2 - ticketSprite->getScaledContentSize().width / 2,
                    0
                    });
            }
            else {
                this->setTitle("You Won a Badge!");
            }

            auto badgeSprite = CCSprite::create(prize.spriteName.c_str());
            if (!badgeSprite) {
                badgeSprite = CCSprite::createWithSpriteFrameName("GJ_unknownBtn_001.png");
            }
            badgeSprite->setPosition({ winSize.width / 2, winSize.height / 2 + 20.f });
            badgeSprite->setScale(0.3f);
            m_mainLayer->addChild(badgeSprite);

            nameLabel->setPosition({ winSize.width / 2, winSize.height / 2 - 35.f });
            categoryLabel->setPosition({ winSize.width / 2, winSize.height / 2 - 55.f });
        }
        else if (prize.type == RewardType::Banner) {
            this->setTitle("New Banner Unlocked!");

            auto bannerSprite = CCSprite::create(prize.spriteName.c_str());
            if (!bannerSprite) {
                bannerSprite = CCSprite::create("GJ_button_01.png");
            }

            float maxWidth = 220.f;
            float scale = 1.0f;
            if (bannerSprite->getContentSize().width > 0) {
                scale = maxWidth / bannerSprite->getContentSize().width;
            }
            if (scale > 0.8f) {
                scale = 0.8f;
            }

            bannerSprite->setScale(scale);
            bannerSprite->setPosition({ winSize.width / 2, winSize.height / 2 + 25.f });
            m_mainLayer->addChild(bannerSprite);

            nameLabel->setPosition({ winSize.width / 2, winSize.height / 2 - 25.f });
            categoryLabel->setPosition({ winSize.width / 2, winSize.height / 2 - 45.f });
        }
        else {
            auto itemSprite = CCSprite::create(prize.spriteName.c_str());
            itemSprite->setPosition({ winSize.width / 2, winSize.height / 2 + 30.f });
            itemSprite->setScale(0.6f);
            m_mainLayer->addChild(itemSprite);

            nameLabel->setPosition({ winSize.width / 2, winSize.height / 2 - 25.f });
            categoryLabel->setPosition({ winSize.width / 2, winSize.height / 2 - 45.f });
        }

        auto okBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("OK"),
            this,
            menu_selector(GenericPrizePopup::onClose)
        );
        auto menu = CCMenu::createWithItem(okBtn);
        menu->setPosition({ winSize.width / 2, 40.f });
        m_mainLayer->addChild(menu);

        return true;
    }

public:
    static GenericPrizePopup* create(GenericPrizeResult prize, std::function<void()> onCloseCallback) {
        auto ret = new GenericPrizePopup();
        if (ret && ret->init(prize, onCloseCallback)) {
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

        this->setTouchEnabled(true);
        m_onCompletionCallback = onCompletion;
        auto winSize = CCDirector::sharedDirector()->getWinSize();

        m_mythicColors = {
            ccc3(255, 0, 0),
            ccc3(255, 165, 0),
            ccc3(255, 255, 0),
            ccc3(0, 255, 0),
            ccc3(0, 0, 255),
            ccc3(75, 0, 130),
            ccc3(238, 130, 238)
        };

        m_currentColor = m_mythicColors[0];
        m_targetColor = m_mythicColors[1];

        auto background = CCLayerColor::create({ 0, 0, 0, 0 });
        background->runAction(CCFadeTo::create(0.5f, 200));
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
        if (!badgeSprite) {
            badgeSprite = CCSprite::createWithSpriteFrameName(badge.spriteName.c_str());
        }
        if (!badgeSprite) {
            badgeSprite = CCSprite::createWithSpriteFrameName("GJ_starsIcon_001.png");
        }

        if (badgeSprite) {
            badgeSprite->setScale(0.4f);
            badgeNode->addChild(badgeSprite, 2);
        }

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

        auto okBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("OK"),
            this,
            menu_selector(MythicAnimationLayer::onMythicClose)
        );

        m_okMenu = CCMenu::createWithItem(okBtn);
        m_okMenu->setPosition({ winSize.width / 2, 60.f });
        this->addChild(m_okMenu);
        m_okMenu->setTouchPriority(-514);

        FMODAudioEngine::sharedEngine()->playEffect("achievement.mp3"_spr);

        congratsLabel->runAction(CCSequence::create(
            CCEaseBackOut::create(CCScaleTo::create(0.4f, 1.2f)),
            CCDelayTime::create(0.7f),
            CCEaseBackIn::create(CCScaleTo::create(0.4f, 0.f)),
            nullptr
        ));

        badgeNode->runAction(CCSequence::create(
            CCDelayTime::create(1.5f),
            CCCallFunc::create(
                this,
                callfunc_selector(MythicAnimationLayer::playCustomSound)
            ),
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

        auto cornerTextMove1 = CCEaseSineOut::create(
            CCMoveTo::create(0.5f, { 20.f, winSize.height - 40.f })
        );
        m_mythicLabel->runAction(CCSequence::create(
            CCDelayTime::create(2.5f),
            CCSpawn::create(CCFadeIn::create(0.5f), cornerTextMove1, nullptr),
            nullptr
        ));

        auto cornerTextMove2 = CCEaseSineOut::create(
            CCMoveTo::create(0.5f, { 20.f, winSize.height - 65.f })
        );
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

class MythicBannerAnimationLayer : public CCLayer {
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

    bool ccTouchBegan(CCTouch* touch, CCEvent* event) override { return true; }
    void ccTouchMoved(CCTouch* touch, CCEvent* event) override {}
    void ccTouchEnded(CCTouch* touch, CCEvent* event) override {}
    void ccTouchCancelled(CCTouch* touch, CCEvent* event) override {}

    void playCustomSound() {
        FMODAudioEngine::sharedEngine()->playEffect("mythic_badge.mp3"_spr);
    }

    void onClose(CCObject*) {
        if (m_onCompletionCallback) {
            m_onCompletionCallback();
        }
        this->removeFromParent();
    }

    static MythicBannerAnimationLayer* create(StreakData::BannerInfo banner, std::function<void()> onCompletion = nullptr) {
        auto ret = new MythicBannerAnimationLayer();
        if (ret && ret->init(banner, onCompletion)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    bool init(StreakData::BannerInfo banner, std::function<void()> onCompletion) {
        if (!CCLayer::init()) return false;
        this->setTouchEnabled(true);
        m_onCompletionCallback = onCompletion;

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        m_mythicColors = {
            ccc3(255, 0, 0), ccc3(255, 165, 0), ccc3(255, 255, 0),
            ccc3(0, 255, 0), ccc3(0, 0, 255), ccc3(75, 0, 130), ccc3(238, 130, 238)
        };
        m_currentColor = m_mythicColors[0];
        m_targetColor = m_mythicColors[1];

        auto background = CCLayerColor::create({ 0, 0, 0, 0 });
        background->runAction(CCFadeTo::create(0.5f, 200));
        this->addChild(background);

        auto congratsLabel = CCLabelBMFont::create("Congratulations!", "goldFont.fnt");
        congratsLabel->setColor({ 255, 80, 80 });
        congratsLabel->setPosition(winSize / 2);
        congratsLabel->setScale(0.f);
        this->addChild(congratsLabel);

        auto bannerNode = CCNode::create();
        bannerNode->setPosition(winSize / 2);
        bannerNode->setScale(0.f);
        this->addChild(bannerNode);

        auto shineSprite = CCSprite::createWithSpriteFrameName("shineBurst_001.png");
        shineSprite->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });
        shineSprite->setOpacity(150);
        shineSprite->setScale(3.5f);
        bannerNode->addChild(shineSprite);

        auto bannerSprite = CCSprite::create(banner.spriteName.c_str());
        if (!bannerSprite) {
            bannerSprite = CCSprite::create("GJ_button_01.png");
        }

        if (bannerSprite) {
            float maxW = 320.f;
            float scale = 1.0f;
            if (bannerSprite->getContentSize().width > 0) {
                scale = maxW / bannerSprite->getContentSize().width;
            }
            if (scale > 1.2f) {
                scale = 1.2f;
            }

            bannerSprite->setScale(scale);
            bannerNode->addChild(bannerSprite, 2);
        }

        m_mythicLabel = CCLabelBMFont::create("Mythic", "goldFont.fnt");
        m_mythicLabel->setColor({ 255, 80, 80 });
        m_mythicLabel->setAnchorPoint({ 0, 0.5f });
        m_mythicLabel->setPosition({ -200.f, winSize.height - 40.f });
        m_mythicLabel->setOpacity(0);
        m_mythicLabel->setScale(0.7f);
        this->addChild(m_mythicLabel);

        auto nameLabel = CCLabelBMFont::create(banner.displayName.c_str(), "goldFont.fnt");
        nameLabel->setAnchorPoint({ 0, 0.5f });
        nameLabel->setPosition({ -200.f, winSize.height - 65.f });
        nameLabel->setOpacity(0);
        nameLabel->setScale(0.5f);
        this->addChild(nameLabel);

        auto okBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("OK"),
            this,
            menu_selector(MythicBannerAnimationLayer::onClose)
        );
        m_okMenu = CCMenu::createWithItem(okBtn);
        m_okMenu->setPosition({ winSize.width / 2, 60.f });
        this->addChild(m_okMenu);
        m_okMenu->setTouchPriority(-514);

        FMODAudioEngine::sharedEngine()->playEffect("achievement.mp3"_spr);

        congratsLabel->runAction(CCSequence::create(
            CCEaseBackOut::create(CCScaleTo::create(0.4f, 1.2f)),
            CCDelayTime::create(0.7f),
            CCEaseBackIn::create(CCScaleTo::create(0.4f, 0.f)),
            nullptr
        ));

        bannerNode->runAction(CCSequence::create(
            CCDelayTime::create(1.5f),
            CCCallFunc::create(
                this,
                callfunc_selector(MythicBannerAnimationLayer::playCustomSound)
            ),
            CCEaseBackOut::create(CCScaleTo::create(0.5f, 1.0f)),
            nullptr
        ));

        shineSprite->runAction(CCRepeatForever::create(CCRotateBy::create(4.0f, 360.f)));

        auto floatUp = CCMoveBy::create(1.5f, { 0, 5.f });
        bannerNode->runAction(CCSequence::create(
            CCDelayTime::create(2.0f),
            CCRepeatForever::create(CCSequence::create(
                CCEaseSineInOut::create(floatUp),
                CCEaseSineInOut::create(floatUp->reverse()),
                nullptr
            )),
            nullptr
        ));

        auto cornerTextMove1 = CCEaseSineOut::create(
            CCMoveTo::create(0.5f, { 20.f, winSize.height - 40.f })
        );
        m_mythicLabel->runAction(CCSequence::create(
            CCDelayTime::create(2.5f),
            CCSpawn::create(CCFadeIn::create(0.5f), cornerTextMove1, nullptr),
            nullptr
        ));

        auto cornerTextMove2 = CCEaseSineOut::create(
            CCMoveTo::create(0.5f, { 20.f, winSize.height - 65.f })
        );
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

class MultiPrizePopup : public Popup {
protected:
    std::function<void()> m_onCloseCallback;

    void onClose(CCObject* sender) override {
        if (m_onCloseCallback) {
            m_onCloseCallback();
        }
        Popup::onClose(sender);
    }

    bool init(std::vector<GenericPrizeResult> prizes, std::function<void()> onCloseCallback) {
        if (!Popup::init(380.f, 230.f, "geode.loader/GE_square01.png")) return false;
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
            itemNode->setPosition({
                startPos.x + col * itemWidth,
                startPos.y - row * itemHeight
                });
            prizesContainer->addChild(itemNode);

            auto itemSprite = CCSprite::create(result.spriteName.c_str());
            itemSprite->setScale(0.28f);
            itemNode->addChild(itemSprite);

            auto categoryLabel = CCLabelBMFont::create(
                g_streakData.getCategoryName(result.category).c_str(),
                "goldFont.fnt"
            );
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
                        CCScaleTo::create(0.5f, 0.45f),
                        CCScaleTo::create(0.5f, 0.4f),
                        nullptr
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

                    auto amountLabel = CCLabelBMFont::create(
                        fmt::format("+{}", result.ticketsFromDuplicate).c_str(),
                        "goldFont.fnt"
                    );

                    amountLabel->setScale(0.4f);
                    amountLabel->setAnchorPoint({ 0, 0.5f });
                    amountLabel->setPosition({ 5.f, 0 });
                    ticketNode->addChild(amountLabel);
                }
            }
            else {
                auto amountLabel = CCLabelBMFont::create(
                    fmt::format("x{}", result.quantity).c_str(),
                    "goldFont.fnt"
                );

                amountLabel->setScale(0.5f);
                amountLabel->setPosition({ 18.f, -18.f });
                itemNode->addChild(amountLabel);
            }
        }

        auto okBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("OK"),
            this,
            menu_selector(MultiPrizePopup::onClose)
        );

        auto menu = CCMenu::createWithItem(okBtn);
        menu->setPosition({ winSize.width / 2, 30.f });
        m_mainLayer->addChild(menu);
        return true;
    }

public:
    static MultiPrizePopup* create(std::vector<GenericPrizeResult> prizes, std::function<void()> onCloseCallback = nullptr) {
        auto ret = new MultiPrizePopup();
        if (ret && ret->init(prizes, onCloseCallback)) {
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

        std::vector<ccColor3B> colors = {
            {255, 255, 255},
            {255, 215, 0},
            {255, 165, 0},
            {255, 255, 150}
        };

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
            CCSpawn::create(
                move,
                fadeOut,
                scaleDown,
                rotate,
                nullptr
            ),
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