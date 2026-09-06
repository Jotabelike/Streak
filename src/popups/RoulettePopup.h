#pragma once
#include "StreakCommon.h"
#include "SharedVisuals.h" 
#include "ShopPopup.h"
#include "../FirebaseManager.h"
#include <Geode/utils/cocos.hpp>
#include "../BadgeNotification.h"
#include "../RewardNotification.h"
#include "../BannerNotification.h" 
#include "GemRouletteConfig.h" 
#include "PurchaseConfirmPopup.h"
#include <functional>

using namespace geode::prelude;

 
extern void spinStandardRouletteOnServer(int spins, int discountPercent, std::function<void(bool, matjson::Value)> callback);
extern void spinGemRouletteOnServer(int discountPercent, std::function<void(bool, matjson::Value)> callback);

enum class RouletteMode {
    Standard,
    Gem
};

// Precio de la ruleta estandar en super stars: 1 giro = 10, x10 = 100.
// Debe coincidir con STANDARD_SPIN_COST del servidor, que es quien cobra.
inline constexpr int STANDARD_SPIN_COST = 10;

class RoulettePopup : public Popup {
protected:
    CCNode* m_rouletteNode;
    CCSprite* m_selectorSprite;

    CCSprite* m_mainPrizeSprite = nullptr;

    // Ads
    CCClippingNode* m_adsClipper = nullptr;
    CCNode* m_adsContainer = nullptr;
    std::vector<CCSprite*> m_adSprites;
    int m_currentAdIndex = 0;
    int m_lastGemIndex = 0;
    float m_timeSinceLastAdSwitch = 0.0f;
    bool m_isAdAnimating = false;
    const float AD_DISPLAY_TIME = 4.0f;
    const float AD_SLIDE_DURATION = 0.5f;
    const float AD_SIZE = 98.0f;

    // UI
    CCMenuItemSpriteExtra* m_spinBtn;
    CCMenuItemSpriteExtra* m_spin10Btn;
    CCMenu* m_spinMenu = nullptr;
    CCMenu* m_shopMenu = nullptr;
    CCMenu* m_infoMenu = nullptr;
    CCMenuItemToggler* m_skipToggle = nullptr;

    // buttons
    CCMenuItemSpriteExtra* m_standardModeBtn = nullptr;
    CCMenuItemSpriteExtra* m_gemModeBtn = nullptr;
    CCMenu* m_modeMenu = nullptr;

    CCLabelBMFont* m_currencyLabel = nullptr;
    CCSprite* m_currencyIcon = nullptr;

    bool m_isSpinning = false;
    RouletteMode m_currentMode = RouletteMode::Standard;

    // Logic
    std::vector<CCNode*> m_orderedSlots;
    std::vector<RoulettePrize> m_roulettePrizes;
    int m_currentSelectorIndex = 0;
    int m_totalSteps = 0;

    // Background Gradient
    CCLayerGradient* m_bgGradient = nullptr;
    std::vector<ccColor3B> m_gradientColors;
    int m_gradColorIndex = 0;
    float m_gradTransitionTime = 0.0f;
    ccColor3B m_gradCurrentStart;
    ccColor3B m_gradCurrentEnd;
    ccColor3B m_gradTargetStart;
    ccColor3B m_gradTargetEnd;

    float m_slotSize = 0.f;

    // Results
    std::vector<GenericPrizeResult> m_multiSpinResults;
    std::vector<StreakData::BadgeInfo> m_pendingMythics;
    std::vector<std::string> m_newBadgesWon;
    int m_pendingTotalTickets = 0;

    // Server espin results
    struct ServerSpinResult {
        int winningIndex = -1;
        std::string type;
        std::string id;
        int quantity = 0;
        std::string category;
        bool isNewBadge = false;
        int ticketsFromDuplicate = 0;
    };
    std::vector<ServerSpinResult> m_serverResults;
    bool m_useServerResults = false;

    void runPopAnimation(CCNode* node) {
        if (node) {
            node->stopAllActions();
            node->setScale(1.0f);
            node->runAction(CCSequence::create(
                CCScaleTo::create(0.05f, 1.15f),
                CCScaleTo::create(0.05f, 1.0f),
                nullptr)
            );
        }
    }

    void runSlotAnimation(CCNode* node) {
        if (node) {
            node->runAction(CCSequence::create(
                CCScaleTo::create(0.02f, 1.1f),
                CCScaleTo::create(0.02f, 1.0f),
                nullptr)
            );
        }
    }

    void flashSlot(CCNode* slotNode) {
        if (!slotNode) return;
        auto flashSprite = CCSprite::create("cuadro.png"_spr);
        if (flashSprite) {
            flashSprite->setScale(m_slotSize / flashSprite->getContentSize().width);
            flashSprite->setPosition(slotNode->getPosition());
            flashSprite->setBlendFunc({ GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA });
            flashSprite->setOpacity(80);
            m_rouletteNode->addChild(flashSprite, 4);
            flashSprite->runAction(CCSequence::create(CCFadeOut::create(0.6f), CCRemoveSelf::create(), nullptr));
        }
    }

    void toggleUI(bool enabled) {
        if (m_spinBtn) m_spinBtn->setEnabled(enabled);
        if (m_spin10Btn) m_spin10Btn->setEnabled(enabled);
        if (m_shopMenu) m_shopMenu->setEnabled(enabled);
        if (m_infoMenu) m_infoMenu->setEnabled(enabled);
        if (m_skipToggle) m_skipToggle->setEnabled(enabled);
        if (m_closeBtn) m_closeBtn->setEnabled(enabled);

        if (m_standardModeBtn) m_standardModeBtn->setEnabled(enabled && m_currentMode != RouletteMode::Standard);
        if (m_gemModeBtn) m_gemModeBtn->setEnabled(enabled && m_currentMode != RouletteMode::Gem);
    }

    void keyBackClicked() override {
        if (m_isSpinning) return;
        Popup::keyBackClicked();
    }

    void setupAdsCarousel() {
        auto stencil = CCSprite::create("square02_001.png");
        stencil->setScale(AD_SIZE / stencil->getContentSize().width);
        m_adsClipper = CCClippingNode::create(stencil);
        m_adsClipper->setPosition({ 0, 0 });
        m_adsClipper->setAlphaThreshold(0.5f);
        m_rouletteNode->addChild(m_adsClipper, 0);
        m_adsContainer = CCNode::create();
        m_adsClipper->addChild(m_adsContainer);

        std::vector<std::string> images = { "publi_1.png"_spr, "publi_2.png"_spr };
        for (int i = 0; i < images.size(); i++) {
            auto spr = CCSprite::create(images[i].c_str());
            if (!spr) { spr = CCSprite::create("square02_001.png"); spr->setColor({ 255, 0, 0 }); }
            if (spr) {
                float scaleX = AD_SIZE / spr->getContentSize().width;
                float scaleY = AD_SIZE / spr->getContentSize().height;
                float finalScale = std::max(scaleX, scaleY);
                spr->setScale(finalScale);
                if (i == 0) spr->setPosition({ 0, 0 }); else spr->setPosition({ AD_SIZE + 10.f, 0 });
                spr->setVisible(i == 0);
                m_adsContainer->addChild(spr);
                m_adSprites.push_back(spr);
            }
        }
    }

    void switchAd() {
        if (m_adSprites.size() < 2) return;
        m_isAdAnimating = true;
        int nextIndex = (m_currentAdIndex + 1) % m_adSprites.size();
        auto currentSprite = m_adSprites[m_currentAdIndex];
        auto nextSprite = m_adSprites[nextIndex];
        nextSprite->setVisible(true);
        nextSprite->setPosition({ AD_SIZE + 5.f, 0 });
        auto moveOut = CCEaseSineInOut::create(CCMoveTo::create(AD_SLIDE_DURATION, ccp(-(AD_SIZE + 5.f), 0)));
        auto hideAction = CallFuncExt::create([currentSprite]() { currentSprite->setVisible(false); });
        auto hideAfter = CCCallFunc::create(this, callfunc_selector(RoulettePopup::onAdAnimationFinished));
        auto moveIn = CCEaseSineInOut::create(CCMoveTo::create(AD_SLIDE_DURATION, ccp(0, 0)));
        currentSprite->runAction(CCSequence::create(moveOut, hideAction, nullptr));
        nextSprite->runAction(CCSequence::create(moveIn, hideAfter, nullptr));
        m_currentAdIndex = nextIndex;
    }

    void onAdAnimationFinished() { m_isAdAnimating = false; m_timeSinceLastAdSwitch = 0.0f; }

    void getPrizesForCurrentMode() {
        if (m_currentMode == RouletteMode::Gem) {
            m_roulettePrizes = GemRouletteConfig::getPrizes();
        }
        else if (!g_streakData.serverStandardRoulette.empty()
                 && g_streakData.serverStandardRoulette.size() <= 12) {
            // Config del servidor (settings/roulette_config). Max 12: el anillo
            // de la UI tiene 12 casillas fijas.
            m_roulettePrizes = GemRouletteConfig::fromServerDefs(g_streakData.serverStandardRoulette);
        }
        else {
            m_roulettePrizes = {
                { RewardType::Badge, "super_star_badge", 1, "", "Super Star", 1, StreakData::BadgeCategory::MYTHIC },
                { RewardType::Badge, "badge_cherry9", 1, "", "Tsukasa", 3, StreakData::BadgeCategory::LEGENDARY },
                { RewardType::Badge, "moon_badge", 1, "", "Moon", 5, StreakData::BadgeCategory::EPIC },
                { RewardType::Badge, "bagde_destello2", 1, "", "cat", 10, StreakData::BadgeCategory::SPECIAL},
                { RewardType::Badge, "mc_badge_3", 1, "", "mmm", 70, StreakData::BadgeCategory::COMMON },
                { RewardType::StarTicket, "star_tiket_0", 1, "tickets_pack.png"_spr, "1 star tiket", 70, StreakData::BadgeCategory::COMMON },
                { RewardType::StarTicket, "star_tiket_1", 10, "tickets_pack.png"_spr, "10 star tiket", 70, StreakData::BadgeCategory::COMMON },
                { RewardType::StarTicket, "star_tiket_3", 3, "tickets_pack.png"_spr, "3 star tiket", 70, StreakData::BadgeCategory::COMMON },
                { RewardType::StarTicket, "star_ticket_15", 15, "tickets_pack.png"_spr, "15 Tickets", 70, StreakData::BadgeCategory::COMMON },
                { RewardType::StarTicket, "star_ticket_30", 30, "tickets_pack.png"_spr, "30 Tickets", 10, StreakData::BadgeCategory::SPECIAL },
                { RewardType::StarTicket, "star_ticket_600", 600, "tickets_pack.png"_spr, "60 Tickets", 5, StreakData::BadgeCategory::EPIC },
                { RewardType::Badge, "badge_cherry4", 1, "", "I don't want to get up", 3, StreakData::BadgeCategory::LEGENDARY }
            };
        }
    }

    void reloadRouletteContent() {
        for (auto node : m_orderedSlots) node->removeFromParent();
        m_orderedSlots.clear();
        if (m_selectorSprite) m_selectorSprite->removeFromParent();


        {
            std::vector<CCNode*> toRemove;
            for (auto* child : CCArrayExt<CCNode*>(m_rouletteNode->getChildren())) {
                if (child->getTag() == 600) toRemove.push_back(child);
            }
            for (auto* n : toRemove) n->removeFromParent();
        }

        if (m_mainPrizeSprite) {
            m_mainPrizeSprite->removeFromParent();
            m_mainPrizeSprite = nullptr;
        }

        getPrizesForCurrentMode();

        m_slotSize = 38.f;
        const float spacing = 5.f;
        std::vector<CCPoint> slotPositions;
        auto winSize = m_mainLayer->getContentSize();

        if (m_currentMode == RouletteMode::Gem) {
            m_rouletteNode->setPositionY(winSize.height / 2 - 35.f);

            auto allPrizes = GemRouletteConfig::getPrizes();
            if (!allPrizes.empty()) {
                auto mainPrize = allPrizes[0];
                std::string spriteName = "";

                if (mainPrize.type == RewardType::Banner) {
                    if (auto info = g_streakData.getBannerInfo(mainPrize.id)) spriteName = info->spriteName;
                }
                else if (mainPrize.type == RewardType::Badge) {
                    if (auto info = g_streakData.getBadgeInfo(mainPrize.id)) spriteName = info->spriteName;
                }

                if (!spriteName.empty()) {
                    m_mainPrizeSprite = CCSprite::create(spriteName.c_str());
                    if (!m_mainPrizeSprite) m_mainPrizeSprite = CCSprite::create("GJ_unknownBtn_001.png");

                    m_mainPrizeSprite->setScale(0.60f);
                    m_mainPrizeSprite->setPosition({ winSize.width / 2, winSize.height / 2 + 55.f });

                    auto glow = CCSprite::createWithSpriteFrameName("shineBurst_001.png");
                    if (glow) {
                        glow->setColor({ 200, 0, 255 });
                        glow->setOpacity(180);
                        glow->setPosition(m_mainPrizeSprite->getContentSize() / 2);
                        glow->setScale(4.0f);
                        glow->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });
                        m_mainPrizeSprite->addChild(glow, -1);
                        glow->runAction(CCRepeatForever::create(CCRotateBy::create(8.0f, 360.f)));
                    }

                    auto moveSeq = CCSequence::create(
                        CCMoveBy::create(1.5f, { 0, 8.f }),
                        CCMoveBy::create(1.5f, { 0, -8.f }),
                        nullptr
                    );
                    m_mainPrizeSprite->runAction(CCRepeatForever::create(CCEaseSineInOut::create(moveSeq)));

                    m_mainLayer->addChild(m_mainPrizeSprite);
                }
            }

            float row1Y = 20.f; float row2Y = -25.f;
            float startX1 = -((4 * m_slotSize + 3 * spacing) / 2.0f) + (m_slotSize / 2.0f);
            for (int i = 0; i < 4; i++) slotPositions.push_back({ startX1 + i * (m_slotSize + spacing), row1Y });
            float startX2 = -((3 * m_slotSize + 2 * spacing) / 2.0f) + (m_slotSize / 2.0f);
            for (int i = 0; i < 3; i++) slotPositions.push_back({ startX2 + i * (m_slotSize + spacing), row2Y });
        }
        else {
            m_rouletteNode->setPositionY(winSize.height / 2 + 15.f);

            const int gridSize = 4;
            const float totalSize = (gridSize * m_slotSize) + ((gridSize - 1) * spacing);
            const float startPos = -totalSize / 2.f + m_slotSize / 2.f;
            for (int i = 0; i < gridSize; ++i) slotPositions.push_back({ startPos + i * (m_slotSize + spacing), startPos + (gridSize - 1) * (m_slotSize + spacing) });
            for (int i = gridSize - 2; i > 0; --i) slotPositions.push_back({ startPos + (gridSize - 1) * (m_slotSize + spacing), startPos + i * (m_slotSize + spacing) });
            for (int i = gridSize - 1; i >= 0; --i) slotPositions.push_back({ startPos + i * (m_slotSize + spacing), startPos });
            for (int i = 1; i < gridSize - 1; ++i) slotPositions.push_back({ startPos, startPos + i * (m_slotSize + spacing) });
        }

        for (size_t i = 0; i < slotPositions.size(); ++i) {
            if (i >= m_roulettePrizes.size()) continue;
            auto& currentPrize = m_roulettePrizes[i];

            auto slotContainer = CCNode::create();
            slotContainer->setPosition(slotPositions[i]);
            m_rouletteNode->addChild(slotContainer);
            m_orderedSlots.push_back(slotContainer);

            auto qualitySprite = CCSprite::create(getQualitySpriteName(currentPrize.category).c_str());
            qualitySprite->setScale((m_slotSize - 4.f) / qualitySprite->getContentSize().width);
            slotContainer->addChild(qualitySprite, 1);

            bool isClaimedGem = (m_currentMode == RouletteMode::Gem)
                && g_streakData.claimedGemRoulettePrizes.count(currentPrize.id) > 0;

            if (currentPrize.type == RewardType::Badge) {
                auto* badgeInfo = g_streakData.getBadgeInfo(currentPrize.id);
                if (badgeInfo) {
                    auto rewardIcon = CCSprite::create(badgeInfo->spriteName.c_str());
                    rewardIcon->setScale(0.15f);
                    slotContainer->addChild(rewardIcon, 2);

                    bool showCheck = (m_currentMode == RouletteMode::Standard && g_streakData.claimedStandardRoulettePrizes.count(badgeInfo->badgeID) > 0) || isClaimedGem;
                    if (showCheck) {
                        auto claimedIcon = CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
                        claimedIcon->setScale(0.5f); claimedIcon->setPosition({ m_slotSize / 2 - 5, -m_slotSize / 2 + 5 }); claimedIcon->setTag(199);
                        slotContainer->addChild(claimedIcon, 3);
                        if (isClaimedGem) rewardIcon->setColor({ 100,100,100 });
                    }
                }
            }
            else if (currentPrize.type == RewardType::Banner) {
                auto* bannerInfo = g_streakData.getBannerInfo(currentPrize.id);
                if (bannerInfo) {
                    auto rewardIcon = CCSprite::create(bannerInfo->spriteName.c_str());
                    if (!rewardIcon) rewardIcon = CCSprite::create("GJ_unknownBtn_001.png");

                    float maxWidth = m_slotSize - 4.f;
                    float scale = maxWidth / rewardIcon->getContentSize().width;
                    if (scale > 0.6f) scale = 0.6f;

                    rewardIcon->setScale(scale);
                    slotContainer->addChild(rewardIcon, 2);

                    bool showCheck = (m_currentMode == RouletteMode::Standard && g_streakData.claimedStandardRoulettePrizes.count(bannerInfo->bannerID) > 0) || isClaimedGem;
                    if (showCheck) {
                        auto claimedIcon = CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
                        claimedIcon->setScale(0.5f); claimedIcon->setPosition({ m_slotSize / 2 - 5, -m_slotSize / 2 + 5 }); claimedIcon->setTag(199);
                        slotContainer->addChild(claimedIcon, 3);
                        if (isClaimedGem) rewardIcon->setColor({ 100,100,100 });
                    }
                }
            }
            else {
                auto rewardIcon = CCSprite::create(currentPrize.spriteName.c_str());
                if (!rewardIcon) rewardIcon = CCSprite::create("square02_001.png");
                rewardIcon->setScale(0.2f);
                slotContainer->addChild(rewardIcon, 2);
                if (isClaimedGem) {
                    auto claimedIcon = CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
                    claimedIcon->setScale(0.5f); claimedIcon->setPosition({ m_slotSize / 2 - 5, -m_slotSize / 2 + 5 }); claimedIcon->setTag(199);
                    slotContainer->addChild(claimedIcon, 3);
                    rewardIcon->setColor({ 100,100,100 });
                }
                else {
                    auto quantityLabel = CCLabelBMFont::create(CCString::createWithFormat("x%d", currentPrize.quantity)->getCString(), "goldFont.fnt");
                    quantityLabel->setScale(0.3f); quantityLabel->setPosition(slotPositions[i].x, slotPositions[i].y - m_slotSize / 2 + 5);
                    quantityLabel->setTag(600);
                    m_rouletteNode->addChild(quantityLabel, 6);
                }
            }
        }

        m_selectorSprite = CCSprite::create("casilla_selector.png"_spr);
        m_selectorSprite->setScale((m_slotSize + 1.f) / m_selectorSprite->getContentSize().width);

        if (m_currentMode == RouletteMode::Standard) {
            m_currentSelectorIndex = g_streakData.lastRouletteIndex;
        }
        else {
            m_currentSelectorIndex = m_lastGemIndex;
        }

        if (m_currentSelectorIndex >= m_orderedSlots.size()) m_currentSelectorIndex = 0;
        if (!m_orderedSlots.empty()) m_selectorSprite->setPosition(m_orderedSlots[m_currentSelectorIndex]->getPosition());
        m_rouletteNode->addChild(m_selectorSprite, 5);

        updateButtons();
    }

    void updateButtons() {
        if (!m_spinBtn || !m_spin10Btn || !m_currencyLabel || !m_currencyIcon) return;
        auto winSize = m_mainLayer->getContentSize();

        m_standardModeBtn->setEnabled(m_currentMode != RouletteMode::Standard);
        m_gemModeBtn->setEnabled(m_currentMode != RouletteMode::Gem);

        m_spinMenu->removeChild(m_spinBtn, true);
        m_spin10Btn->retain();
        m_spinMenu->removeChild(m_spin10Btn, false);

        if (m_currentMode == RouletteMode::Standard) {
            m_adsClipper->setVisible(true);
            m_spinBtn = CCMenuItemSpriteExtra::create(
                ButtonSprite::create(fmt::format("x{}", STANDARD_SPIN_COST).c_str()),
                this, menu_selector(RoulettePopup::onSpin));
            m_spinMenu->addChild(m_spinBtn);
            m_spinMenu->addChild(m_spin10Btn);
            m_spin10Btn->release();
            m_spin10Btn->setVisible(true);
            m_currencyLabel->setString(std::to_string(g_streakData.superStars).c_str());
            auto texture = CCSprite::create("super_star.png"_spr)->getTexture();
            m_currencyIcon->setTexture(texture);
            m_currencyIcon->setColor({ 255,255,255 });

            m_spinMenu->setPosition({ winSize.width / 2, 30.f });
            m_spinBtn->setPosition({ 0, 0 });
            m_spin10Btn->setPosition({ 0, 0 });
            m_spinMenu->alignItemsHorizontallyWithPadding(5.f);
        }
        else {
            m_adsClipper->setVisible(false);
            int cost = GemRouletteConfig::getCostForStep(g_streakData.gemRouletteSpinCount);

            int claimedCount = 0;
            for (size_t i = 0; i < 7 && i < m_roulettePrizes.size(); i++) {
                if (g_streakData.claimedGemRoulettePrizes.count(m_roulettePrizes[i].id) > 0) claimedCount++;
            }

            if (claimedCount >= 7) {
                m_spinBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("Done", 0, 0, "goldFont.fnt", "GJ_button_02.png", 0, 0.8f), this, menu_selector(RoulettePopup::onSpin));
                m_spinMenu->addChild(m_spinBtn);
                m_spinBtn->setEnabled(false);
            }
            else {
                m_spinBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create(std::to_string(cost).c_str()), this, menu_selector(RoulettePopup::onSpin));
                m_spinMenu->addChild(m_spinBtn);
                m_spinBtn->setEnabled(true);
            }

            m_spinMenu->addChild(m_spin10Btn);
            m_spin10Btn->release();
            m_spin10Btn->setVisible(false);
            m_currencyLabel->setString(std::to_string(g_streakData.gems).c_str());
            auto texture = CCSprite::create("gem.png"_spr)->getTexture();
            m_currencyIcon->setTexture(texture);
            m_currencyIcon->setColor({ 255,255,255 });

            m_spinMenu->setPosition({ winSize.width / 2, 25.f });
            m_spinBtn->setPosition({ 0.f, 0.f });
        }

        float labelW = m_currencyLabel->getScaledContentSize().width;
        float iconW = m_currencyIcon->getScaledContentSize().width;
        float spacing = 2.0f;
        float totalWidth = labelW + spacing + iconW;
        float startX = -totalWidth / 2.0f;

        m_currencyLabel->setAnchorPoint({ 0.0f, 0.5f });
        m_currencyLabel->setPosition({ startX, 0 });
        m_currencyIcon->setAnchorPoint({ 0.0f, 0.5f });
        m_currencyIcon->setPosition({ startX + labelW + spacing, 0 });
    }

    void onSelectStandardMode(CCObject*) {
        if (m_isSpinning || m_currentMode == RouletteMode::Standard) return;
        m_currentMode = RouletteMode::Standard;
        reloadRouletteContent();
    }

    void onSelectGemMode(CCObject*) {
        if (m_isSpinning || m_currentMode == RouletteMode::Gem) return;
        m_currentMode = RouletteMode::Gem;
        reloadRouletteContent();
    }

    bool init() override {
        if (!Popup::init(260.f, 260.f, "GJ_square02.png")) return false;

        std::srand(static_cast<unsigned int>(std::time(nullptr)));

        this->setTitle("Roulette");
        auto winSize = m_mainLayer->getContentSize();
        g_streakData.load();

        // One-time reset for players whose claimedStandardRoulettePrizes were
        // polluted while the client/server IDs were out of sync.
        if (!Mod::get()->getSavedValue<bool>("standard_roulette_v2_reset_done", false)) {
            g_streakData.claimedStandardRoulettePrizes.clear();
            Mod::get()->setSavedValue<bool>("standard_roulette_v2_reset_done", true);
            g_streakData.save();
        }

        std::string currentHash = "v2|";
        auto prizes = GemRouletteConfig::getPrizes();
        for (const auto& p : prizes) {
            currentHash += fmt::format("{}:{}:{};", p.id, p.quantity, (int)p.type);
        }

        if (g_streakData.gemRouletteHash != currentHash) {
            g_streakData.gemRouletteSpinCount = 0;
            g_streakData.gemRouletteState.assign(7, false);
            g_streakData.gemRouletteHash = currentHash;
            g_streakData.claimedGemRoulettePrizes.clear();
            g_streakData.save();
        }

        // Saneamiento: si quedan IDs en el set que ya no estan en la lista actual
        // de premios (porque el dev cambio un slot), eliminarlos para que no
        // aparezcan ghost-claims.
        {
            std::set<std::string> validIds;
            for (const auto& p : prizes) validIds.insert(p.id);
            bool changed = false;
            for (auto it = g_streakData.claimedGemRoulettePrizes.begin();
                 it != g_streakData.claimedGemRoulettePrizes.end();) {
                if (validIds.find(*it) == validIds.end()) {
                    it = g_streakData.claimedGemRoulettePrizes.erase(it);
                    changed = true;
                } else {
                    ++it;
                }
            }
            if (changed) g_streakData.save();
        }

        // Saneamiento equivalente para la ruleta Standard. Necesitamos la lista
        // standard, asi que la construimos llamando getPrizesForCurrentMode con
        // el modo temporal Standard y restaurando.
        {
            auto prevMode = m_currentMode;
            m_currentMode = RouletteMode::Standard;
            getPrizesForCurrentMode();
            std::set<std::string> validStdIds;
            for (const auto& p : m_roulettePrizes) validStdIds.insert(p.id);
            bool changedStd = false;
            for (auto it = g_streakData.claimedStandardRoulettePrizes.begin();
                 it != g_streakData.claimedStandardRoulettePrizes.end();) {
                if (validStdIds.find(*it) == validStdIds.end()) {
                    it = g_streakData.claimedStandardRoulettePrizes.erase(it);
                    changedStd = true;
                } else {
                    ++it;
                }
            }
            m_currentMode = prevMode;
            m_roulettePrizes.clear();
            if (changedStd) g_streakData.save();
        }

        m_gradientColors = {
        ccc3(255, 50, 50),
        ccc3(255, 150, 50),
        ccc3(255, 255, 50),
        ccc3(50, 255, 50),
        ccc3(50, 255, 255),
        ccc3(50, 100, 255),
        ccc3(150, 50, 255),
        ccc3(255, 50, 150)
        };
        m_gradCurrentStart = m_gradientColors[0];
        m_gradCurrentEnd = m_gradientColors[5];
        m_gradTargetStart = m_gradientColors[1];
        m_gradTargetEnd = m_gradientColors[6];

        m_rouletteNode = CCNode::create();
        m_rouletteNode->setPosition({ winSize.width / 2, winSize.height / 2 + 15.f });
        m_mainLayer->addChild(m_rouletteNode);


        auto bgSize = m_bgSprite->getContentSize();
        auto gradStencil = CCScale9Sprite::create("GJ_square02.png");
        gradStencil->setContentSize(bgSize);
        gradStencil->setPosition(bgSize / 2);

        auto gradClipper = CCClippingNode::create(gradStencil);
        gradClipper->setAlphaThreshold(0.05f);
        gradClipper->setContentSize(bgSize);
        gradClipper->setPosition({ 0, 0 });
        gradClipper->ignoreAnchorPointForPosition(false);
        gradClipper->setAnchorPoint({ 0, 0 });

        m_bgGradient = CCLayerGradient::create(
            ccc4(m_gradCurrentStart.r, m_gradCurrentStart.g, m_gradCurrentStart.b, 100),
            ccc4(m_gradCurrentEnd.r, m_gradCurrentEnd.g, m_gradCurrentEnd.b, 100)
        );
        m_bgGradient->setContentSize(bgSize);
        m_bgGradient->setPosition({ 0, 0 });
        m_bgGradient->ignoreAnchorPointForPosition(false);
        m_bgGradient->setAnchorPoint({ 0, 0 });

        gradClipper->addChild(m_bgGradient);
        m_bgSprite->addChild(gradClipper, 1);

        this->setupAdsCarousel();

        // Los botones muestran lo que cuesta cada tirada en super stars.
        m_spinBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create(fmt::format("x{}", STANDARD_SPIN_COST).c_str()),
            this, menu_selector(RoulettePopup::onSpin));
        m_spin10Btn = CCMenuItemSpriteExtra::create(ButtonSprite::create(
            fmt::format("x{}", STANDARD_SPIN_COST * 10).c_str(),
            0, 0, "goldFont.fnt", "GJ_button_03.png",
            0, 0.8f),
            this, menu_selector(RoulettePopup::onSpinMultiple));

        m_spinMenu = CCMenu::create();
        m_spinMenu->addChild(m_spinBtn);
        m_spinMenu->addChild(m_spin10Btn);
        m_mainLayer->addChild(m_spinMenu);

        auto stdSpr = CCSprite::create("star_gacha.png"_spr);
        stdSpr->setScale(0.6f);
        m_standardModeBtn = CCMenuItemSpriteExtra::create(stdSpr, this, menu_selector(RoulettePopup::onSelectStandardMode));

        auto gemSpr = CCSprite::create("gem_gacha.png"_spr);
        gemSpr->setScale(0.6f);
        m_gemModeBtn = CCMenuItemSpriteExtra::create(gemSpr, this, menu_selector(RoulettePopup::onSelectGemMode));

        m_modeMenu = CCMenu::create();
        m_modeMenu->addChild(m_standardModeBtn);
        m_modeMenu->addChild(m_gemModeBtn);
        m_modeMenu->alignItemsVerticallyWithPadding(-10.f);
        m_modeMenu->setPosition({ -40.f, winSize.height / 2 });
        m_mainLayer->addChild(m_modeMenu);

        auto shopSprite = CCSprite::create("shop_btn.png"_spr); shopSprite->setScale(0.7f);
        auto shopBtn = CCMenuItemSpriteExtra::create(shopSprite, this, menu_selector(RoulettePopup::onOpenShop));
        m_shopMenu = CCMenu::createWithItem(shopBtn);
        m_shopMenu->setPosition({ winSize.width - 35.f, 30.f });
        m_mainLayer->addChild(m_shopMenu);

        auto infoIcon = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png"); infoIcon->setScale(0.8f);
        auto infoBtn = CCMenuItemSpriteExtra::create(infoIcon, this, menu_selector(RoulettePopup::onShowProbabilities));
        m_infoMenu = CCMenu::createWithItem(infoBtn);
        m_infoMenu->setPosition({ 25.f, winSize.height - 25.f });
        m_mainLayer->addChild(m_infoMenu);

        auto skipMenu = CCMenu::create(); skipMenu->setPosition({ 35.f, 35.f }); m_mainLayer->addChild(skipMenu);
        m_skipToggle = CCMenuItemToggler::createWithStandardSprites(this, menu_selector(RoulettePopup::onToggleSkip), 0.6f);
        m_skipToggle->setPosition({ 0, 0 }); skipMenu->addChild(m_skipToggle);
        auto skipLabel = CCLabelBMFont::create("Skip", "bigFont.fnt");
        skipLabel->setScale(0.35f); skipLabel->setPosition({ 0.f, 20.f }); skipLabel->setAlignment(kCCTextAlignmentCenter);
        skipMenu->addChild(skipLabel);

        auto currencyNode = CCNode::create();
        m_currencyLabel = CCLabelBMFont::create("0", "goldFont.fnt");
        m_currencyLabel->setScale(0.4f);
        m_currencyLabel->setID("roulette-currency-label");
        m_currencyIcon = CCSprite::create("super_star.png"_spr);
        m_currencyIcon->setScale(0.12f);
        currencyNode->addChild(m_currencyLabel);
        currencyNode->addChild(m_currencyIcon);
        currencyNode->setPosition({ winSize.width - 35.f, winSize.height - 25.f });
        m_mainLayer->addChild(currencyNode);

        m_currentMode = RouletteMode::Standard;
        reloadRouletteContent();

        this->scheduleUpdate();
        return true;
    }

    void update(float dt) override {

        if (m_bgGradient && !m_gradientColors.empty()) {
            m_gradTransitionTime += dt * 0.25f;
            if (m_gradTransitionTime >= 1.0f) {
                m_gradTransitionTime = 0.0f;
                m_gradColorIndex = (m_gradColorIndex + 1) % m_gradientColors.size();
                m_gradCurrentStart = m_gradTargetStart;
                m_gradCurrentEnd = m_gradTargetEnd;
                m_gradTargetStart = m_gradientColors[(m_gradColorIndex + 1) % m_gradientColors.size()];
                m_gradTargetEnd = m_gradientColors[(m_gradColorIndex + 2) % m_gradientColors.size()];
            }
            float p = m_gradTransitionTime;
            ccColor3B startC = {
                static_cast<GLubyte>(m_gradCurrentStart.r + (m_gradTargetStart.r - m_gradCurrentStart.r) * p),
                static_cast<GLubyte>(m_gradCurrentStart.g + (m_gradTargetStart.g - m_gradCurrentStart.g) * p),
                static_cast<GLubyte>(m_gradCurrentStart.b + (m_gradTargetStart.b - m_gradCurrentStart.b) * p)
            };
            ccColor3B endC = {
                static_cast<GLubyte>(m_gradCurrentEnd.r + (m_gradTargetEnd.r - m_gradCurrentEnd.r) * p),
                static_cast<GLubyte>(m_gradCurrentEnd.g + (m_gradTargetEnd.g - m_gradCurrentEnd.g) * p),
                static_cast<GLubyte>(m_gradCurrentEnd.b + (m_gradTargetEnd.b - m_gradCurrentEnd.b) * p)
            };
            m_bgGradient->setStartColor(startC);
            m_bgGradient->setEndColor(endC);
            m_bgGradient->setStartOpacity(150);
            m_bgGradient->setEndOpacity(50);
        }
        if (!m_isAdAnimating && !m_adSprites.empty() && m_currentMode == RouletteMode::Standard) {
            m_timeSinceLastAdSwitch += dt;
            if (m_timeSinceLastAdSwitch >= AD_DISPLAY_TIME) switchAd();
        }
    }


    void spawnImpactParticles(CCPoint pos) {
        for (int i = 0; i < 8; i++) {
            auto particle = CCSprite::createWithSpriteFrameName("GJ_featuredCoin_001.png");
            particle->setPosition(pos);

            float startScale = 0.2f + (static_cast<float>(rand() % 15) / 100.0f);
            particle->setScale(startScale);

            if (rand() % 2 == 0) particle->setColor({ 255, 255, 255 });
            else particle->setColor({ 255, 235, 150 });

            particle->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });
            m_rouletteNode->addChild(particle, 100);

            float angle = static_cast<float>(rand() % 360);
            float distance = 30.0f + static_cast<float>(rand() % 20);

            float rads = CC_DEGREES_TO_RADIANS(angle);
            CCPoint dest = {
                cos(rads) * distance,
                sin(rads) * distance
            };

            auto move = CCEaseOut::create(CCMoveBy::create(0.4f, dest), 2.0f);
            auto fade = CCFadeOut::create(0.4f);
            auto scale = CCScaleTo::create(0.4f, 0.0f);

            particle->runAction(CCSequence::create(
                CCSpawn::create(move, fade, scale, nullptr),
                CCRemoveSelf::create(),
                nullptr
            ));
        }
    }


    void runSpinAnimationForIndex(int winningIndex, std::vector<int> logicalAvailableIndices) {
        
        if (m_skipToggle->isToggled()) {
            m_currentSelectorIndex = winningIndex;
            m_selectorSprite->setPosition(m_orderedSlots[winningIndex]->getPosition());
            this->spawnImpactParticles(m_orderedSlots[winningIndex]->getPosition());
            this->onSpinEnd();
            return;
        }

         
        if (m_currentMode == RouletteMode::Gem && logicalAvailableIndices.size() == 1) {
            m_totalSteps = 1;
            auto moveAction = CCEaseSineOut::create(CCMoveTo::create(0.4f, m_orderedSlots[winningIndex]->getPosition()));
            auto arriveEffect = CallFuncExt::create([this, winningIndex]() {
                this->playTickSound();
                this->runPopAnimation(m_orderedSlots[winningIndex]);
                this->flashSlot(m_orderedSlots[winningIndex]);
                this->spawnImpactParticles(m_orderedSlots[winningIndex]->getPosition());
                m_currentSelectorIndex = winningIndex;
                });
            auto endCall = CCCallFunc::create(this, callfunc_selector(RoulettePopup::onSpinEnd));
            m_selectorSprite->runAction(CCSequence::create(moveAction, arriveEffect, endCall, nullptr));
            return;
        }

    
        std::vector<int> animationPathIndices;
        if (m_currentMode == RouletteMode::Gem) {
            for (int i = 0; i < (int)m_orderedSlots.size(); i++) {
                bool isTaken = false;
                if (i < (int)m_roulettePrizes.size()) {
                    isTaken = g_streakData.claimedGemRoulettePrizes.count(m_roulettePrizes[i].id) > 0;
                }
                if (!isTaken || i == winningIndex) animationPathIndices.push_back(i);
            }
            std::sort(animationPathIndices.begin(), animationPathIndices.end());
        }
        else {
            for (int i = 0; i < (int)m_orderedSlots.size(); i++) animationPathIndices.push_back(i);
        }
        if (animationPathIndices.empty()) {
            for (int i = 0; i < (int)m_orderedSlots.size(); i++) animationPathIndices.push_back(i);
        }

        int pathSize = animationPathIndices.size();
        int startPosInPath = pathSize - 1;
        for (int i = 0; i < pathSize; i++) {
            if (animationPathIndices[i] == m_currentSelectorIndex) { startPosInPath = i; break; }
            if (animationPathIndices[i] < m_currentSelectorIndex) startPosInPath = i;
            else break;
        }
        int targetPosInPath = 0;
        for (int i = 0; i < pathSize; i++) {
            if (animationPathIndices[i] == winningIndex) { targetPosInPath = i; break; }
        }
        int stepsInPath = (5 * pathSize) + (targetPosInPath - startPosInPath + pathSize) % pathSize;
        if (stepsInPath < pathSize) stepsInPath += pathSize;
        m_totalSteps = stepsInPath;

        auto actions = CCArray::create();
        for (int i = 1; i <= stepsInPath; ++i) {
            int nextPathIndex = (startPosInPath + i) % pathSize;
            int realSlotIndex = animationPathIndices[nextPathIndex];
            float duration = (i < 5) ? 0.2f - (i * 0.03f) :
                (i > stepsInPath - 10) ? 0.05f + ((i - (stepsInPath - 10)) * 0.04f) : 0.05f;
            auto moveAction = CCEaseSineInOut::create(CCMoveTo::create(duration, m_orderedSlots[realSlotIndex]->getPosition()));
            auto arriveEffects = CallFuncExt::create([this, realSlotIndex]() {
                this->playTickSound();
                this->runPopAnimation(m_orderedSlots[realSlotIndex]);
                this->flashSlot(m_orderedSlots[realSlotIndex]);
                m_currentSelectorIndex = realSlotIndex;
                });
            actions->addObject(CCSequence::create(moveAction, arriveEffects, nullptr));
        }
        auto finalParticles = CallFuncExt::create([this, winningIndex]() {
            this->spawnImpactParticles(m_orderedSlots[winningIndex]->getPosition());
            });
        actions->addObject(finalParticles);
        actions->addObject(CCCallFunc::create(this, callfunc_selector(RoulettePopup::onSpinEnd)));
        m_selectorSprite->runAction(CCSequence::create(actions));
    }

    void onSpin(CCObject*) {
        if (m_isSpinning) return;
        int price = m_currentMode == RouletteMode::Standard
            ? STANDARD_SPIN_COST
            : GemRouletteConfig::getCostForStep(g_streakData.gemRouletteSpinCount);
        auto currency = m_currentMode == RouletteMode::Standard
            ? PurchaseCurrency::SuperStars
            : PurchaseCurrency::Gems;
        std::string sprite = m_currentMode == RouletteMode::Standard
            ? fmt::format("{}"_spr, "star_gacha.png")
            : fmt::format("{}"_spr, "gem.png");
        std::string name = m_currentMode == RouletteMode::Standard ? "Roulette Spin" : "Gem Roulette Spin";

        auto popup = PurchaseConfirmPopup::create(sprite, name, price, currency,
            [this, keepAlive = Ref<CCNode>(this)](int discountPercent) {
                this->performSpin(discountPercent);
            });
        if (popup) popup->show();
    }

    void performSpin(int discountPercent) {
        if (m_isSpinning) return;
         
        std::vector<int> logicalAvailableIndices;
        if (m_currentMode == RouletteMode::Standard) {
            int finalCost = discountedPurchasePrice(STANDARD_SPIN_COST, discountPercent);
            if (g_streakData.superStars < finalCost) {
                FLAlertLayer::create("Not Enough Super Stars",
                    fmt::format("You need {} Super Stars.", finalCost).c_str(), "OK")->show();
                return;
            }
        }
        else {
            for (int i = 0; i < 7; i++) {
                bool isTaken = false;
                if (i < (int)m_roulettePrizes.size()) {
                    isTaken = g_streakData.claimedGemRoulettePrizes.count(m_roulettePrizes[i].id) > 0;
                }
                if (!isTaken) logicalAvailableIndices.push_back(i);
            }
            if (logicalAvailableIndices.empty()) {
                FLAlertLayer::create("Completed", "You have claimed all Gem Rewards!", "OK")->show();
                return;
            }
            int spinCost = GemRouletteConfig::getCostForStep(g_streakData.gemRouletteSpinCount);
            int finalCost = discountedPurchasePrice(spinCost, discountPercent);
            if (g_streakData.gems < finalCost) {
                FLAlertLayer::create("Not Enough Gems", fmt::format("You need {} Gems.", finalCost), "OK")->show();
                return;
            }
        }

        m_isSpinning = true;
        toggleUI(false);
 
        if (m_currentMode == RouletteMode::Standard) {
            spinStandardRouletteOnServer(1, discountPercent, [this, keepAlive = Ref<CCNode>(this)](bool ok, matjson::Value data) {
                if (!ok) {
                    m_isSpinning = false;
                    toggleUI(true);
                    FLAlertLayer::create("Error", "Connection error. Try again.", "OK")->show();
                    return;
                }
                
                m_serverResults.clear();
                m_useServerResults = true;
                auto resultsArr = data["results"].as<std::vector<matjson::Value>>();
                int winningIndex = 0;
                if (resultsArr.isOk() && !resultsArr.unwrap().empty()) {
                    auto& r = resultsArr.unwrap()[0];
                    ServerSpinResult sr;
                    sr.winningIndex = r["winningIndex"].as<int>().unwrapOr(0);
                    sr.type = r["type"].as<std::string>().unwrapOr(std::string(""));
                    sr.id = r["id"].as<std::string>().unwrapOr(std::string(""));
                    sr.quantity = r["quantity"].as<int>().unwrapOr(0);
                    sr.category = r["category"].as<std::string>().unwrapOr(std::string(""));
                    sr.isNewBadge = r["isNewBadge"].as<bool>().unwrapOr(false);
                    sr.ticketsFromDuplicate = r["ticketsFromDuplicate"].as<int>().unwrapOr(0);
                    m_serverResults.push_back(sr);
                    winningIndex = sr.winningIndex;
                }
                updateButtons();
                runSpinAnimationForIndex(winningIndex, {});
                });
        }
        else {
            auto capturedAvailable = logicalAvailableIndices;
            spinGemRouletteOnServer(discountPercent, [this, capturedAvailable, keepAlive = Ref<CCNode>(this)](bool ok, matjson::Value data) {
                if (!ok) {
                    m_isSpinning = false;
                    toggleUI(true);
                    FLAlertLayer::create("Error", "Connection error. Try again.", "OK")->show();
                    return;
                }
                m_serverResults.clear();
                m_useServerResults = true;
                auto result = data["result"];
                ServerSpinResult sr;
                sr.winningIndex = result["winningIndex"].as<int>().unwrapOr(0);
                sr.type = result["type"].as<std::string>().unwrapOr(std::string(""));
                sr.id = result["id"].as<std::string>().unwrapOr(std::string(""));
                sr.quantity = result["quantity"].as<int>().unwrapOr(0);
                sr.category = result["category"].as<std::string>().unwrapOr(std::string(""));
                m_serverResults.push_back(sr);
                updateButtons();
                runSpinAnimationForIndex(sr.winningIndex, capturedAvailable);
                });
        }
    }

    void onSpinMultiple(CCObject*) {
        if (m_currentMode != RouletteMode::Standard) return;
        if (m_isSpinning) return;
        auto popup = PurchaseConfirmPopup::create(
            fmt::format("{}"_spr, "star_gacha.png"), "10 Roulette Spins",
            STANDARD_SPIN_COST * 10, PurchaseCurrency::SuperStars,
            [this, keepAlive = Ref<CCNode>(this)](int discountPercent) {
                this->performSpinMultiple(discountPercent);
            });
        if (popup) popup->show();
    }

    void performSpinMultiple(int discountPercent) {
        if (m_currentMode != RouletteMode::Standard) return;
        int finalCost = discountedPurchasePrice(STANDARD_SPIN_COST * 10, discountPercent);
        if (g_streakData.superStars < finalCost) {
            FLAlertLayer::create("Error",
                fmt::format("Need {} Super Stars.", finalCost).c_str(), "OK")->show();
            return;
        }
        if (m_isSpinning) return;
        m_isSpinning = true;
        toggleUI(false);
 
        spinStandardRouletteOnServer(10, discountPercent, [this, keepAlive = Ref<CCNode>(this)](bool ok, matjson::Value data) {
            if (!ok) {
                m_isSpinning = false;
                toggleUI(true);
                FLAlertLayer::create("Error", "Connection error. Try again.", "OK")->show();
                return;
            }

            m_serverResults.clear();
            m_useServerResults = true;
            m_multiSpinResults.clear();
            m_pendingMythics.clear();
            m_newBadgesWon.clear();

            std::vector<int> winningIndices;
            auto resultsArr = data["results"].as<std::vector<matjson::Value>>();
            if (resultsArr.isOk()) {
                for (auto& r : resultsArr.unwrap()) {
                    ServerSpinResult sr;
                    sr.winningIndex = r["winningIndex"].as<int>().unwrapOr(0);
                    sr.type = r["type"].as<std::string>().unwrapOr(std::string(""));
                    sr.id = r["id"].as<std::string>().unwrapOr(std::string(""));
                    sr.quantity = r["quantity"].as<int>().unwrapOr(0);
                    sr.category = r["category"].as<std::string>().unwrapOr(std::string(""));
                    sr.isNewBadge = r["isNewBadge"].as<bool>().unwrapOr(false);
                    sr.ticketsFromDuplicate = r["ticketsFromDuplicate"].as<int>().unwrapOr(0);
                    m_serverResults.push_back(sr);
                    winningIndices.push_back(sr.winningIndex);

                
                    if (sr.winningIndex < (int)m_roulettePrizes.size()) {
                        auto& prize = m_roulettePrizes[sr.winningIndex];
                        GenericPrizeResult res;
                        res.type = prize.type;
                        res.id = prize.id;
                        res.quantity = prize.quantity;
                        res.displayName = prize.displayName;
                        res.category = prize.category;
                        res.isNew = sr.isNewBadge;
                        res.ticketsFromDuplicate = sr.ticketsFromDuplicate;
                        if (prize.type == RewardType::Badge) {
                            auto* bInfo = g_streakData.getBadgeInfo(prize.id);
                            if (bInfo) {
                                res.spriteName = bInfo->spriteName;
                                if (sr.isNewBadge) {
                                    m_newBadgesWon.push_back(prize.id);
                                    if (bInfo->category == StreakData::BadgeCategory::MYTHIC)
                                        m_pendingMythics.push_back(*bInfo);
                                }
                            }
                        }
                        else {
                            res.spriteName = prize.spriteName;
                        }
                        m_multiSpinResults.push_back(res);
                    }
                }
            }

            updateButtons();
 
            if (m_skipToggle->isToggled() && !winningIndices.empty()) {
                m_currentSelectorIndex = winningIndices.back();
                m_selectorSprite->setPosition(m_orderedSlots[m_currentSelectorIndex]->getPosition());
                this->spawnImpactParticles(m_orderedSlots[m_currentSelectorIndex]->getPosition());
                this->onMultiSpinEnd();
                return;
            }

            auto actions = CCArray::create();
            int totalSlots = m_orderedSlots.size();
            int lastIdx = m_currentSelectorIndex;

            for (int pIdx : winningIndices) {
                int dist = (pIdx - lastIdx + totalSlots) % totalSlots;
                if (dist == 0) dist = totalSlots;
                for (int i = 1; i <= dist; ++i) {
                    int currentSlotIdx = (lastIdx + i) % totalSlots;
                    auto moveAction = CCMoveTo::create(0.04f, m_orderedSlots[currentSlotIdx]->getPosition());
                    auto arriveEffects = CallFuncExt::create([this, currentSlotIdx]() {
                        this->playTickSound();
                        this->runSlotAnimation(m_orderedSlots[currentSlotIdx]);
                        this->flashSlot(m_orderedSlots[currentSlotIdx]);
                        });
                    actions->addObject(CCSequence::create(moveAction, arriveEffects, nullptr));
                }
                auto hitPrizeEffect = CallFuncExt::create([this, pIdx]() {
                    this->spawnImpactParticles(m_orderedSlots[pIdx]->getPosition());
                    });
                actions->addObject(hitPrizeEffect);
                actions->addObject(CCDelayTime::create(0.25f));
                lastIdx = pIdx;
            }
            m_currentSelectorIndex = lastIdx;
            actions->addObject(CCCallFunc::create(this, callfunc_selector(RoulettePopup::onMultiSpinEnd)));
            m_selectorSprite->runAction(CCSequence::create(actions));
            });
    }

    void processMythicQueue() {
        if (!m_pendingMythics.empty()) {
            auto mb = m_pendingMythics.front(); m_pendingMythics.erase(m_pendingMythics.begin());
            auto al = MythicAnimationLayer::create(mb, [this]() { this->processMythicQueue(); });
            CCDirector::sharedDirector()->getRunningScene()->addChild(al, 400);
        }
        else { showMultiSpinSummary(); }
    }

    void onMultiSpinEnd() {
        m_isSpinning = false;
        if (m_currentMode == RouletteMode::Standard) g_streakData.lastRouletteIndex = m_currentSelectorIndex;

        // Marca cada premio ganado (por ID) en el set correspondiente.
        for (const auto& r : m_multiSpinResults) {
            if (r.id.empty()) continue;
            if (r.type != RewardType::Badge && r.type != RewardType::Banner) continue;
            if (m_currentMode == RouletteMode::Gem) {
                g_streakData.claimedGemRoulettePrizes.insert(r.id);
            }
            else {
                g_streakData.claimedStandardRoulettePrizes.insert(r.id);
            }
        }

        updatePlayerDataInFirebase();
        updateAllCheckmarks();
        m_pendingTotalTickets = 0;
        for (const auto& r : m_multiSpinResults) {
            if (r.type == RewardType::StarTicket) m_pendingTotalTickets += r.quantity;
            if (r.type == RewardType::Badge && !r.isNew) m_pendingTotalTickets += r.ticketsFromDuplicate;
        }
        m_useServerResults = false;
        m_serverResults.clear();
        if (!m_pendingMythics.empty()) processMythicQueue(); else showMultiSpinSummary();
    }

    void showMultiSpinSummary() {
        MultiPrizePopup::create(m_multiSpinResults, [this]() {
            this->toggleUI(true);
            for (const auto& badgeID : m_newBadgesWon) BadgeNotification::show(badgeID);
            if (m_pendingTotalTickets > 0) {
                int startAmount = g_streakData.starTickets - m_pendingTotalTickets;
                auto winSize = CCDirector::sharedDirector()->getWinSize();

                RewardNotification::show("star_tiket.png"_spr, startAmount, m_pendingTotalTickets, winSize / 2);
            }
            updateButtons();
            })->show();
    }

    void onSpinEnd() {
        m_isSpinning = false;
        toggleUI(true);

        if (m_currentSelectorIndex >= (int)m_roulettePrizes.size()) m_currentSelectorIndex = 0;
        RoulettePrize prize = m_roulettePrizes[m_currentSelectorIndex];

        int pendingTickets = 0;
        int pendingStars = 0;
        bool showBadgeNotify = false;
        bool isMythicAnimation = false;

        if (m_currentMode == RouletteMode::Gem) {
            m_lastGemIndex = m_currentSelectorIndex;
            // Marcar el premio (por ID) como obtenido en la ruleta de gemas.
            if (!prize.id.empty()) g_streakData.claimedGemRoulettePrizes.insert(prize.id);
            if (prize.type == RewardType::Badge) {
                auto* bi = g_streakData.getBadgeInfo(prize.id);
                if (bi) {
                    if (bi->category == StreakData::BadgeCategory::MYTHIC) {
                        isMythicAnimation = true;
                        auto animLayer = MythicAnimationLayer::create(*bi, [this, prize]() { BadgeNotification::show(prize.id); });
                        CCDirector::sharedDirector()->getRunningScene()->addChild(animLayer, 400);
                    }
                    else {
                        showBadgeNotify = true;
                    }
                }
            }
            else if (prize.type == RewardType::Banner) {
                auto* bi = g_streakData.getBannerInfo(prize.id);
                if (bi) {
                    if (bi->rarity == StreakData::BadgeCategory::MYTHIC) {
                        isMythicAnimation = true;
                        auto animLayer = MythicBannerAnimationLayer::create(*bi, [bi]() {
                            BannerNotification::show(bi->bannerID, bi->spriteName, bi->displayName,
                                g_streakData.getCategoryName(bi->rarity), g_streakData.getCategoryColor(bi->rarity));
                            });
                        CCDirector::sharedDirector()->getRunningScene()->addChild(animLayer, 400);
                    }
                    else {
                        BannerNotification::show(bi->bannerID, bi->spriteName, bi->displayName,
                            g_streakData.getCategoryName(bi->rarity), g_streakData.getCategoryColor(bi->rarity));
                    }
                }
            }
            else if (prize.type == RewardType::SuperStar) {
                pendingStars = prize.quantity;
            }
            else if (prize.type == RewardType::StarTicket) {
                pendingTickets = prize.quantity;
            }
        }
        else {
            g_streakData.lastRouletteIndex = m_currentSelectorIndex;
            // Marcar el premio como obtenido en la ruleta estandar (por ID).
            if (!prize.id.empty() && (prize.type == RewardType::Badge || prize.type == RewardType::Banner)) {
                g_streakData.claimedStandardRoulettePrizes.insert(prize.id);
            }

            if (prize.type == RewardType::Badge) {
                auto* bi = g_streakData.getBadgeInfo(prize.id);
                if (bi) {
               
                    bool isNew = false;
                    int dupTickets = 0;
                    if (m_useServerResults && !m_serverResults.empty()) {
                        isNew = m_serverResults[0].isNewBadge;
                        dupTickets = m_serverResults[0].ticketsFromDuplicate;
                    }
                    else {
                        isNew = !g_streakData.isBadgeUnlocked(prize.id);
                    }

                    if (isNew) {
                        if (bi->category == StreakData::BadgeCategory::MYTHIC) {
                            isMythicAnimation = true;
                            auto animLayer = MythicAnimationLayer::create(*bi, [this, prize]() { BadgeNotification::show(prize.id); });
                            CCDirector::sharedDirector()->getRunningScene()->addChild(animLayer, 400);
                        }
                        else {
                            showBadgeNotify = true;
                        }
                    }
                    else {
                        pendingTickets = dupTickets > 0 ? dupTickets : g_streakData.getTicketValueForRarity(bi->category);
                    }
                }
            }
            else if (prize.type == RewardType::SuperStar) {
                pendingStars = prize.quantity;
            }
            else if (prize.type == RewardType::StarTicket) {
                pendingTickets = prize.quantity;
            }
        }

       
        updatePlayerDataInFirebase();
        updateButtons();

        if (m_currentMode == RouletteMode::Gem) {
            reloadRouletteContent();
        }
        else {
            updateAllCheckmarks();
        }

        if (!isMythicAnimation) {
            CCPoint spawnPos = m_orderedSlots[m_currentSelectorIndex]->convertToWorldSpaceAR(CCPointZero);
            if (showBadgeNotify) BadgeNotification::show(prize.id);
            if (pendingTickets > 0) {
                RewardNotification::show("star_tiket.png"_spr, g_streakData.starTickets - pendingTickets, pendingTickets, spawnPos);
            }
            if (pendingStars > 0) {
                RewardNotification::show("super_star.png"_spr, g_streakData.superStars - pendingStars, pendingStars, spawnPos);
            }
        }

        m_useServerResults = false;
        m_serverResults.clear();
    }

    void updateAllCheckmarks() {
        if (m_currentMode == RouletteMode::Gem) return;

        for (size_t i = 0; i < m_orderedSlots.size(); ++i) {
            if (i >= m_roulettePrizes.size()) continue;
            const auto& p = m_roulettePrizes[i];
            if (p.type != RewardType::Badge && p.type != RewardType::Banner) continue;
            if (g_streakData.claimedStandardRoulettePrizes.count(p.id) > 0 && !m_orderedSlots[i]->getChildByTag(199)) {
                auto cm = CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
                cm->setScale(0.5f);
                cm->setPosition({ m_slotSize / 2 - 5, -m_slotSize / 2 + 5 });
                cm->setTag(199);
                m_orderedSlots[i]->addChild(cm, 3);
            }
        }
    }

    void playTickSound() {
        FMODAudioEngine::sharedEngine()->playEffect("ruleta_sfx.mp3"_spr);
    }


    void onOpenShop(CCObject*) {

        auto shopScene = ShopLayer::scene([this]() {
            this->updateAllCheckmarks();
            this->updateButtons();
            });


        auto transition = CCTransitionFade::create(0.5f, shopScene);
        CCDirector::sharedDirector()->pushScene(transition);
    }



    void onToggleSkip(CCObject* sender) {}

    void onShowProbabilities(CCObject*) {
        g_streakData.load();

        std::string infoText = "";

        struct CategoryDisplay { StreakData::BadgeCategory cat; std::string name; std::string colorTag; };
        std::vector<CategoryDisplay> displayOrder = {
            { StreakData::BadgeCategory::COMMON, "Common", "<cl>" },
            { StreakData::BadgeCategory::SPECIAL, "Special", "<cy>" },
            { StreakData::BadgeCategory::EPIC, "Epic", "<co>" },
            { StreakData::BadgeCategory::LEGENDARY, "Legendary", "<cr>" },
            { StreakData::BadgeCategory::MYTHIC, "Mythic", "<cp>" }
        };

        if (m_currentMode == RouletteMode::Standard) {
            int totalWeight = 0; for (const auto& prize : m_roulettePrizes) totalWeight += prize.probabilityWeight;
            std::map<StreakData::BadgeCategory, int> weightsByCategory;
            for (const auto& prize : m_roulettePrizes) weightsByCategory[prize.category] += prize.probabilityWeight;

            infoText = fmt::format("Total Spins: {}\n\n", g_streakData.totalSpins);

            for (const auto& item : displayOrder) {
                int weight = weightsByCategory[item.cat];
                if (weight > 0) {
                    float percent = (static_cast<float>(weight) / totalWeight) * 100.0f;
                    infoText += fmt::format("{}{}</c>: {:.1f}%\n", item.colorTag, item.name, percent);
                }
            }
        }
        else {
            int totalWeight = 0;
            std::map<StreakData::BadgeCategory, int> weightsByCategory;

            for (size_t i = 0; i < m_roulettePrizes.size(); i++) {
                bool isTaken = g_streakData.claimedGemRoulettePrizes.count(m_roulettePrizes[i].id) > 0;

                if (!isTaken) {
                    totalWeight += m_roulettePrizes[i].probabilityWeight;
                    weightsByCategory[m_roulettePrizes[i].category] += m_roulettePrizes[i].probabilityWeight;
                }
            }

            if (totalWeight == 0) {
                infoText = "GEM ROULETTE\n\nAll rewards obtained!";
            }
            else {
                infoText = "GEM ROULETTE (Current Odds)\n\n";

                for (const auto& item : displayOrder) {
                    int weight = weightsByCategory[item.cat];
                    if (weight > 0) {
                        float percent = (static_cast<float>(weight) / totalWeight) * 100.0f;
                        infoText += fmt::format("{}{}</c>: {:.1f}%\n", item.colorTag, item.name, percent);
                    }
                }

                int currentCost = GemRouletteConfig::getCostForStep(g_streakData.gemRouletteSpinCount);
                infoText += fmt::format("\nNext Spin Cost: {} Gems", currentCost);
            }
        }

        FLAlertLayer::create("Probabilities", infoText, "OK")->show();
    }

    std::string getQualitySpriteName(StreakData::BadgeCategory c) {
        switch (c) {
        case StreakData::BadgeCategory::SPECIAL: return "casilla_especial.png"_spr;
        case StreakData::BadgeCategory::EPIC: return "casilla_epica.png"_spr;
        case StreakData::BadgeCategory::LEGENDARY: return "casilla_legendaria.png"_spr;
        case StreakData::BadgeCategory::MYTHIC: return "casilla_mitica.png"_spr;
        default: return "casilla_comun.png"_spr;
        }
    }

public:
    static RoulettePopup* create() {
        auto ret = new RoulettePopup();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};
