#pragma once
#include "StreakCommon.h"
#include "../FirebaseManager.h"
#include <Geode/ui/Popup.hpp>
#include <Geode/utils/cocos.hpp>
#include "../BadgeNotification.h"
#include "../RewardNotification.h"
#include "../BannerNotification.h"
#include "SharedVisuals.h" 

using namespace geode::prelude;

class ShopPopup : public Popup {
protected:
    enum ShopMode {
        MODE_BADGES,
        MODE_BANNERS
    };

    ShopMode m_currentMode = MODE_BADGES;
    std::function<void()> m_onCloseCallback;
    CCLabelBMFont* m_ticketCounterLabel = nullptr;
    CCMenu* m_itemMenu = nullptr;

    CCLabelBMFont* m_categoryLabel = nullptr;
    int m_categoryIndex = 0;

    int m_colorIndex = 0;
    float m_colorTransitionTime = 0.0f;
    std::vector<ccColor3B> m_mythicColors;
    ccColor3B m_currentColor;
    ccColor3B m_targetColor;

    CCMenuItemSpriteExtra* m_prevBtn = nullptr;
    CCMenuItemSpriteExtra* m_nextBtn = nullptr;
    int m_page = 0;
    int m_itemsPerPage = 6;
    CCSize m_size;

    CCMenuItemToggler* m_badgesToggle = nullptr;
    CCMenuItemToggler* m_bannersToggle = nullptr;

    std::map<std::string, int> m_shopItems;
    std::map<std::string, int> m_bannerItems;

    std::string formatPrice(int price) {
        std::string s = std::to_string(price);
        int n = s.length() - 3;
        while (n > 0) {
            s.insert(n, ".");
            n -= 3;
        }
        return "$" + s;
    }

    bool init() {
        if (!Popup::init(400.f, 280.f, "geode.loader/GE_square01.png")) return false;

        m_size = m_mainLayer->getContentSize();
        this->setTitle("Streak Shop");
        g_streakData.load();

        m_mythicColors = {
            ccc3(255, 0, 0),
            ccc3(255, 165, 0),
            ccc3(255, 255, 0),
            ccc3(0, 255, 0),
            ccc3(0, 0, 255),
            ccc3(75, 0, 130),
            ccc3(238, 130, 238),
            ccc3(255, 105, 180),
            ccc3(255, 215, 0),
            ccc3(192, 192, 192)
        };
        m_currentColor = m_mythicColors[0];
        m_targetColor = m_mythicColors[1];

        m_shopItems = {
            {"diamante_mc_badge", 550},
            {"platino_streak_badge", 30},
            {"diamante_gd_badge", 40},
            {"hounter_badge", 60},
            {"money_badge", 100},
            {"gd_badge", 130},
            {"Steampunk_Dash_badge",150},
            {"dark_streak_badge", 500},
            {"gold_streak_badge", 1200},
            {"bh_badge_1", 29990},
            {"bh_badge_2", 1300},
            {"bh_badge_3", 520},
            {"bh_badge_4", 990},
            {"bh_badge_5", 11990},
            {"bh_badge_6", 1190},
            {"bh_badge_7", 45990},
            {"mc_badge_1", 3999},
            {"mc_badge_2", 1699},
            {"mc_badge_3", 199},
            {"mai_badge", 1200}
        };

        m_bannerItems = {
            {"banner_1", 5600},
            {"banner_2", 8000},
            {"banner_9", 29500},
            {"banner_22",120500},
            {"banner_7", 67500},
            {"banner_11", 72300}
        };

        auto background = cocos2d::extension::CCScale9Sprite::create("square02_001.png");
        background->setColor({ 0, 0, 0 });
        background->setOpacity(120);
        background->setContentSize({
            340.f,
            150.f
            });
        background->setPosition({
            m_size.width / 2,
            m_size.height / 2 - 30.f
            });
        m_mainLayer->addChild(background);

        setupTabs();
        setupTicketCounter();

        m_itemMenu = CCMenu::create();
        m_itemMenu->setContentSize(background->getContentSize());
        m_itemMenu->setPosition(background->getPosition());
        m_itemMenu->ignoreAnchorPointForPosition(false);
        m_mainLayer->addChild(m_itemMenu);

        setupPaginationArrows(background);
        setupCategorySelector(background);

        this->scheduleUpdate();

        m_badgesToggle->toggle(true);
        m_bannersToggle->toggle(false);
        updateItems();

        return true;
    }

    void setupTabs() {
        auto badgeOn = ButtonSprite::create(
            "Badges", 80, true, "goldFont.fnt", "GJ_button_01.png", 30.f, 0.6f
        );
        auto badgeOff = ButtonSprite::create(
            "Badges", 80, true, "goldFont.fnt", "GJ_button_01.png", 30.f, 0.6f
        );

        auto bannerOn = ButtonSprite::create(
            "Banners", 80, true, "goldFont.fnt", "GJ_button_01.png", 30.f, 0.6f
        );
        auto bannerOff = ButtonSprite::create(
            "Banners", 80, true, "goldFont.fnt", "GJ_button_01.png", 30.f, 0.6f
        );

        badgeOn->setScale(0.8f);
        badgeOff->setScale(0.8f);
        bannerOn->setScale(0.8f);
        bannerOff->setScale(0.8f);

        m_badgesToggle = CCMenuItemToggler::create(
            badgeOff,
            badgeOn,
            this,
            menu_selector(ShopPopup::onSwitchToBadges)
        );
        m_bannersToggle = CCMenuItemToggler::create(
            bannerOff,
            bannerOn,
            this,
            menu_selector(ShopPopup::onSwitchToBanners)
        );

        auto menu = CCMenu::create();
        menu->addChild(m_badgesToggle);
        menu->addChild(m_bannersToggle);

        float topY = m_size.height - 45.f;

        m_badgesToggle->setPosition({
            m_size.width / 2 - 55.f,
            topY
            });
        m_bannersToggle->setPosition({
            m_size.width / 2 + 55.f,
            topY
            });

        menu->setPosition(0, 0);
        m_mainLayer->addChild(menu);
    }

    void onSwitchToBadges(CCObject* sender) {
        if (m_currentMode == MODE_BADGES) {
            m_badgesToggle->toggle(true);
            return;
        }
        m_currentMode = MODE_BADGES;
        m_badgesToggle->toggle(true);
        m_bannersToggle->toggle(false);

        m_page = 0;
        m_categoryIndex = 0;
        updateCategoryVisuals();
        updateItems();
    }

    void onSwitchToBanners(CCObject* sender) {
        if (m_currentMode == MODE_BANNERS) {
            m_bannersToggle->toggle(true);
            return;
        }
        m_currentMode = MODE_BANNERS;
        m_bannersToggle->toggle(true);
        m_badgesToggle->toggle(false);

        m_page = 0;
        m_categoryIndex = 0;
        updateCategoryVisuals();
        updateItems();
    }

    void update(float dt) override {
        if (m_categoryIndex != 5 || !m_categoryLabel) return;

        m_colorTransitionTime += dt;
        if (m_colorTransitionTime >= 1.0f) {
            m_colorTransitionTime = 0.0f;
            m_colorIndex = (m_colorIndex + 1) % m_mythicColors.size();
            m_currentColor = m_targetColor;
            m_targetColor = m_mythicColors[(m_colorIndex + 1) % m_mythicColors.size()];
        }

        float progress = m_colorTransitionTime / 1.0f;
        ccColor3B interpolatedColor = {
            static_cast<GLubyte>(m_currentColor.r + (m_targetColor.r - m_currentColor.r) * progress),
            static_cast<GLubyte>(m_currentColor.g + (m_targetColor.g - m_currentColor.g) * progress),
            static_cast<GLubyte>(m_currentColor.b + (m_targetColor.b - m_currentColor.b) * progress)
        };
        m_categoryLabel->setColor(interpolatedColor);
    }

    void setupTicketCounter() {
        auto ticketNode = CCNode::create();
        m_mainLayer->addChild(ticketNode);

        auto ticketSprite = CCSprite::create("star_tiket.png"_spr);
        ticketSprite->setScale(0.25f);
        ticketNode->addChild(ticketSprite);

        float iconW = ticketSprite->getScaledContentSize().width;

        m_ticketCounterLabel = CCLabelBMFont::create(
            std::to_string(g_streakData.starTickets).c_str(),
            "goldFont.fnt"
        );
        m_ticketCounterLabel->setScale(0.5f);
        m_ticketCounterLabel->setAnchorPoint({ 1.f, 0.5f });
        ticketNode->addChild(m_ticketCounterLabel);

        ticketSprite->setPosition({ 0, 0 });
        m_ticketCounterLabel->setPosition({ -iconW / 2 - 6.f, 0 });
        ticketNode->setPosition({
            m_size.width - 30.f,
            m_size.height - 22.f
            });
    }

    void setupPaginationArrows(CCNode* bg) {
        auto navMenu = CCMenu::create();
        navMenu->setContentSize(m_size);
        navMenu->setPosition({ 0, 0 });
        m_mainLayer->addChild(navMenu);

        auto spriteLeft = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        spriteLeft->setScale(0.8f);
        m_prevBtn = CCMenuItemSpriteExtra::create(
            spriteLeft,
            this,
            menu_selector(ShopPopup::onPageChange)
        );
        m_prevBtn->setTag(-1);

        float bgCenterY = bg->getPositionY();
        float bgLeftX = bg->getPositionX() - (bg->getContentSize().width / 2);

        m_prevBtn->setPosition({ bgLeftX - 25.f, bgCenterY });
        navMenu->addChild(m_prevBtn);

        auto spriteRight = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        spriteRight->setFlipX(true);
        spriteRight->setScale(0.8f);
        m_nextBtn = CCMenuItemSpriteExtra::create(
            spriteRight,
            this,
            menu_selector(ShopPopup::onPageChange)
        );
        m_nextBtn->setTag(1);

        float bgRightX = bg->getPositionX() + (bg->getContentSize().width / 2);

        m_nextBtn->setPosition({ bgRightX + 25.f, bgCenterY });
        navMenu->addChild(m_nextBtn);
    }

    void setupCategorySelector(CCNode* bg) {
        auto selectorMenu = CCMenu::create();
        float yPos = bg->getPositionY() + (bg->getContentSize().height / 2) + 25.f;

        selectorMenu->setPosition({ m_size.width / 2, yPos });
        m_mainLayer->addChild(selectorMenu);

        m_categoryLabel = CCLabelBMFont::create("ALL", "bigFont.fnt");
        m_categoryLabel->setScale(0.5f);
        selectorMenu->addChild(m_categoryLabel);

        auto leftSpr = CCSprite::createWithSpriteFrameName("edit_leftBtn_001.png");
        auto leftBtn = CCMenuItemSpriteExtra::create(
            leftSpr,
            this,
            menu_selector(ShopPopup::onCycleCategory)
        );
        leftBtn->setTag(-1);
        leftBtn->setPosition({ -90.f, 0 });
        selectorMenu->addChild(leftBtn);

        auto rightSpr = CCSprite::createWithSpriteFrameName("edit_rightBtn_001.png");
        auto rightBtn = CCMenuItemSpriteExtra::create(
            rightSpr,
            this,
            menu_selector(ShopPopup::onCycleCategory)
        );
        rightBtn->setTag(1);
        rightBtn->setPosition({ 90.f, 0 });
        selectorMenu->addChild(rightBtn);

        updateCategoryVisuals();
    }

    void onCycleCategory(CCObject* sender) {
        int direction = sender->getTag();
        int totalCategories = 6;
        m_categoryIndex += direction;

        if (m_categoryIndex < 0) {
            m_categoryIndex = totalCategories - 1;
        }
        if (m_categoryIndex >= totalCategories) {
            m_categoryIndex = 0;
        }

        m_page = 0;
        updateCategoryVisuals();
        updateItems();
    }

    void updateCategoryVisuals() {
        if (!m_categoryLabel) return;
        std::string text = "";
        m_categoryLabel->setColor({ 255, 255, 255 });

        if (m_categoryIndex == 0) {
            text = "ALL";
        }
        else {
            StreakData::BadgeCategory cat = static_cast<StreakData::BadgeCategory>(m_categoryIndex - 1);
            text = g_streakData.getCategoryName(cat);
            if (cat != StreakData::BadgeCategory::MYTHIC) {
                m_categoryLabel->setColor(g_streakData.getCategoryColor(cat));
            }
        }
        std::transform(text.begin(), text.end(), text.begin(), ::toupper);
        m_categoryLabel->setString(text.c_str());
    }

    void onPageChange(CCObject* sender) {
        m_page += sender->getTag();
        updateItems();
    }

    void updateItems() {
        m_itemMenu->removeAllChildren();
        g_streakData.load();

        int filterValue = (m_categoryIndex == 0) ? -1 : (m_categoryIndex - 1);

        struct DisplayItem {
            std::string id;
            int price;
            std::string spriteName;
            bool unlocked;
            StreakData::BadgeCategory category;
        };
        std::vector<DisplayItem> filteredItems;

        if (m_currentMode == MODE_BADGES) {
            m_itemsPerPage = 6;
            for (const auto& item : m_shopItems) {
                auto* info = g_streakData.getBadgeInfo(item.first);
                if (!info) continue;
                if (filterValue == -1 || static_cast<int>(info->category) == filterValue) {
                    filteredItems.push_back({
                        item.first,
                        item.second,
                        info->spriteName,
                        g_streakData.isBadgeUnlocked(item.first),
                        info->category
                        });
                }
            }
        }
        else {
            m_itemsPerPage = 4;
            for (const auto& item : m_bannerItems) {
                auto* info = g_streakData.getBannerInfo(item.first);
                if (!info) continue;
                if (filterValue == -1 || static_cast<int>(info->rarity) == filterValue) {
                    filteredItems.push_back({
                        item.first,
                        item.second,
                        info->spriteName,
                        g_streakData.isBannerUnlocked(item.first),
                        info->rarity
                        });
                }
            }
        }

        std::sort(filteredItems.begin(), filteredItems.end(), [](const DisplayItem& a, const DisplayItem& b) {
            return a.price < b.price;
            });

        int totalItems = filteredItems.size();
        int startIdx = m_page * m_itemsPerPage;
        int endIdx = std::min(startIdx + m_itemsPerPage, totalItems);
        int totalPages = (totalItems + m_itemsPerPage - 1) / m_itemsPerPage;
        if (totalPages == 0) totalPages = 1;

        m_prevBtn->setVisible(m_page > 0);
        m_nextBtn->setVisible(m_page < totalPages - 1);

        int cols = 3;
        float spacingX = 100.f;

        if (m_currentMode == MODE_BANNERS) {
            cols = 2;
            spacingX = 150.f;
        }

        float itemSize = 60.f;
        float spacingY = 70.f;
        float menuWidth = m_itemMenu->getContentSize().width;
        float menuHeight = m_itemMenu->getContentSize().height;

        for (int i = startIdx; i < endIdx; ++i) {
            int localIndex = i - startIdx;
            int row = localIndex / cols;
            int col = localIndex % cols;
            auto& data = filteredItems[i];

            auto itemNode = CCNode::create();
            itemNode->setContentSize({ itemSize, itemSize });

            auto sprite = CCSprite::create(data.spriteName.c_str());
            if (sprite) {
                float scale = 0.28f;
                if (m_currentMode == MODE_BANNERS) {
                    float targetWidth = 100.f;
                    if (sprite->getContentSize().width > targetWidth) {
                        scale = targetWidth / sprite->getContentSize().width;
                    }
                    else {
                        scale = 0.5f;
                    }
                }
                sprite->setScale(scale);
                sprite->setPosition({ itemSize / 2, itemSize / 2 + 10.f });
                itemNode->addChild(sprite);
            }

            auto priceLabel = CCLabelBMFont::create(
                formatPrice(data.price).c_str(),
                "goldFont.fnt"
            );
            priceLabel->setScale(0.35f);

            auto priceTicketIcon = CCSprite::create("star_tiket.png"_spr);
            priceTicketIcon->setScale(0.15f);

            float totalW = priceLabel->getScaledContentSize().width + priceTicketIcon->getScaledContentSize().width + 3.f;

            priceLabel->setPosition({
                (itemSize - totalW) / 2 + priceLabel->getScaledContentSize().width / 2,
                0.f
                });
            priceTicketIcon->setPosition({
                priceLabel->getPositionX() + priceLabel->getScaledContentSize().width / 2 + 3.f + priceTicketIcon->getScaledContentSize().width / 2,
                0.f
                });

            itemNode->addChild(priceLabel);
            itemNode->addChild(priceTicketIcon);

            CCMenuItemSpriteExtra* buyBtn = CCMenuItemSpriteExtra::create(
                itemNode,
                this,
                menu_selector(ShopPopup::onBuyItem)
            );
            buyBtn->setUserObject("item_id"_spr, CCString::create(data.id));

            float posX = (menuWidth / 2) + (col - (cols - 1) / 2.0f) * spacingX;
            float startY = menuHeight - 35.f;
            float posY = startY - (row * spacingY);

            buyBtn->setPosition({ posX, posY });
            m_itemMenu->addChild(buyBtn);

            if (data.unlocked) {
                buyBtn->setEnabled(false);
                for (auto* child : CCArrayExt<CCNode*>(itemNode->getChildren())) {
                    if (auto* rgbaNode = typeinfo_cast<CCRGBAProtocol*>(child)) {
                        rgbaNode->setOpacity(100);
                    }
                }
                auto checkmark = CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
                checkmark->setScale(0.5f);
                checkmark->setPosition({ itemSize / 2, itemSize / 2 + 10.f });
                itemNode->addChild(checkmark, 10);
            }
        }
        updatePlayerDataInFirebase();
    }

    void onBuyItem(CCObject* sender) {
        std::string itemID = static_cast<CCString*>(static_cast<CCNode*>(sender)->getUserObject("item_id"_spr))->getCString();
        int price = 0;
        std::string displayName = "";
        StreakData::BadgeCategory category = StreakData::BadgeCategory::COMMON;

        if (m_currentMode == MODE_BADGES) {
            auto info = g_streakData.getBadgeInfo(itemID);
            if (!info) return;
            price = m_shopItems[itemID];
            displayName = info->displayName;
            category = info->category;
        }
        else {
            auto info = g_streakData.getBannerInfo(itemID);
            if (!info) return;
            price = m_bannerItems[itemID];
            displayName = info->displayName;
            category = info->rarity;
        }

        if (g_streakData.starTickets < price) {
            FLAlertLayer::create(
                "Not Enough Tickets",
                "You need more tickets!",
                "OK"
            )->show();
            return;
        }

        createQuickPopup(
            "Confirm",
            fmt::format("Buy <cg>{}</c>?", displayName),
            "No", "Yes",
            [this, itemID, price, category](FLAlertLayer*, bool btn2) {
                if (btn2) {
                    g_streakData.load();
                    if (g_streakData.starTickets >= price) {
                        g_streakData.starTickets -= price;

                        if (m_currentMode == MODE_BADGES) {
                            g_streakData.unlockBadge(itemID);
                        }
                        else {
                            g_streakData.unlockBanner(itemID);
                        }

                        g_streakData.save();
                        m_ticketCounterLabel->setString(
                            std::to_string(g_streakData.starTickets).c_str()
                        );
                        updateItems();
                        FMODAudioEngine::sharedEngine()->playEffect("buy_obj.mp3"_spr);

                        if (m_currentMode == MODE_BADGES) {
                            if (category == StreakData::BadgeCategory::MYTHIC) {
                                auto badgeInfo = g_streakData.getBadgeInfo(itemID);
                                if (badgeInfo) {
                                    auto animLayer = MythicAnimationLayer::create(
                                        *badgeInfo,
                                        [itemID]() {
                                            BadgeNotification::show(itemID);
                                        }
                                    );
                                    CCDirector::sharedDirector()->getRunningScene()->addChild(animLayer, 99999);
                                }
                            }
                            else {
                                BadgeNotification::show(itemID);
                            }
                        }
                        else {
                            auto info = g_streakData.getBannerInfo(itemID);
                            if (info) {
                                if (category == StreakData::BadgeCategory::MYTHIC) {
                                    auto animLayer = MythicBannerAnimationLayer::create(
                                        *info,
                                        [itemID, info]() {
                                            std::string rTxt = g_streakData.getCategoryName(info->rarity);
                                            ccColor3B rCol = g_streakData.getCategoryColor(info->rarity);
                                            BannerNotification::show(
                                                itemID,
                                                info->spriteName,
                                                info->displayName,
                                                rTxt,
                                                rCol
                                            );
                                        }
                                    );
                                    CCDirector::sharedDirector()->getRunningScene()->addChild(animLayer, 99999);
                                }
                                else {
                                    std::string rTxt = g_streakData.getCategoryName(info->rarity);
                                    ccColor3B rCol = g_streakData.getCategoryColor(info->rarity);
                                    BannerNotification::show(
                                        itemID,
                                        info->spriteName,
                                        info->displayName,
                                        rTxt,
                                        rCol
                                    );
                                }
                            }
                        }
                    }
                }
                updatePlayerDataInFirebase();
            }
        );
    }

    void onClose(CCObject* sender) override {
        this->unscheduleUpdate();
        if (m_onCloseCallback) {
            m_onCloseCallback();
        }
        Popup::onClose(sender);
    }

public:
    static ShopPopup* create(std::function<void()> callback = nullptr) {
        auto ret = new ShopPopup();
        ret->m_onCloseCallback = callback;
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};
