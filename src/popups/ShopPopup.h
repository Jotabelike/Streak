#pragma once
#include "StreakCommon.h"
#include "../FirebaseManager.h"
#include <Geode/ui/Popup.hpp>
#include <Geode/utils/cocos.hpp>
#include "../BadgeNotification.h"
#include "../RewardNotification.h"
#include "../BannerNotification.h"
#include "SharedVisuals.h" 
#include "PurchaseConfirmPopup.h"

using namespace geode::prelude;
 
class ItemConfirmPopup : public Popup {
protected:
    std::function<void()> m_onBuy;

    bool init(std::string spriteName, std::string displayName, int price, std::function<void()> onBuy) {
    
        if (!Popup::init(250.f, 200.f, "GJ_square04.png")) return false;

        m_onBuy = onBuy;
        this->setTitle("Confirm Purchase");

     
        auto sprite = CCSprite::create(spriteName.c_str());
        if (sprite) {
            float maxDim = 70.f;  
            float currentMax = std::max(sprite->getContentSize().width, sprite->getContentSize().height);
            if (currentMax > maxDim) {
                sprite->setScale(maxDim / currentMax);
            }
            else {
                sprite->setScale(1.1f);
            }
            sprite->setPosition({ m_size.width / 2, m_size.height / 2 + 30.f });
            m_mainLayer->addChild(sprite);
        }
 
        auto nameLabel = CCLabelBMFont::create(displayName.c_str(), "goldFont.fnt");
        nameLabel->setScale(0.55f);
        nameLabel->setPosition({ m_size.width / 2, m_size.height / 2 - 15.f });
        m_mainLayer->addChild(nameLabel);

       
        auto priceNode = CCNode::create();
        auto priceLabel = CCLabelBMFont::create(std::to_string(price).c_str(), "goldFont.fnt");
        priceLabel->setScale(0.45f);

        auto ticketIcon = CCSprite::create("star_tiket.png"_spr);
        ticketIcon->setScale(0.18f);

        float totalW = priceLabel->getScaledContentSize().width + ticketIcon->getScaledContentSize().width + 5.f;
        priceLabel->setPosition({ -totalW / 2 + priceLabel->getScaledContentSize().width / 2, 0 });
        ticketIcon->setPosition({ priceLabel->getPositionX() + priceLabel->getScaledContentSize().width / 2 + 5.f + ticketIcon->getScaledContentSize().width / 2, 0 });

        priceNode->addChild(priceLabel);
        priceNode->addChild(ticketIcon);

      
        priceNode->setPosition({ m_size.width / 2, m_size.height / 2 - 40.f });
        m_mainLayer->addChild(priceNode);

      
        auto menu = CCMenu::create();
        menu->setPosition({ m_size.width / 2, 28.f });
        m_mainLayer->addChild(menu);

        auto cancelSpr = ButtonSprite::create("Cancel", 0, false, "goldFont.fnt", "GJ_button_06.png", 0, 0.7f);
        auto cancelBtn = CCMenuItemSpriteExtra::create(cancelSpr, this, menu_selector(ItemConfirmPopup::onClose));
        cancelBtn->setPosition({ -65.f, 0 });
        menu->addChild(cancelBtn);

        auto buySpr = ButtonSprite::create("Buy", 0, false, "goldFont.fnt", "GJ_button_01.png", 0, 0.7f);
        auto buyBtn = CCMenuItemSpriteExtra::create(buySpr, this, menu_selector(ItemConfirmPopup::onConfirmBuy));
        buyBtn->setPosition({ 65.f, 0 });
        menu->addChild(buyBtn);

        return true;
    }

    void onConfirmBuy(CCObject* sender) {
        if (m_onBuy) m_onBuy();
        this->onClose(sender);  
    }

public:
    static ItemConfirmPopup* create(std::string spriteName, std::string displayName, int price, std::function<void()> onBuy) {
        auto ret = new ItemConfirmPopup();
        if (ret && ret->init(spriteName, displayName, price, onBuy)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};
 


class ShopLayer : public CCLayer {
protected:
    enum ItemType {
        TYPE_BADGE,
        TYPE_BANNER
    };

    struct DisplayItem {
        std::string id;
        int price;
        std::string spriteName;
        bool unlocked;
        StreakData::BadgeCategory category;
        ItemType type;
    };

    std::function<void()> m_onCloseCallback;
    CCLabelBMFont* m_ticketCounterLabel = nullptr;
    CCMenu* m_itemMenu = nullptr;

    CCMenuItemSpriteExtra* m_prevBtn = nullptr;
    CCMenuItemSpriteExtra* m_nextBtn = nullptr;
    int m_page = 0;
    int m_itemsPerPage = 8;
    CCSize m_size;

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

    bool init() override {
        if (!CCLayer::init()) return false;

        m_size = CCDirector::sharedDirector()->getWinSize();
        g_streakData.load();
        this->setKeypadEnabled(true);

        GameManager::sharedState()->fadeInMusic("Streak_shop_loop.mp3"_spr);

        m_shopItems = {
            {"diamante_mc_badge", 550},
            {"platino_streak_badge", 320},
            {"diamante_gd_badge", 700},
            {"hounter_badge", 600},
            {"money_badge", 500},
            {"gd_badge", 200},
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
            {"mai_badge", 1200},
            {"colombia_badge", 1400},
            {"usa_badge", 1400},
            {"venezuela_badge", 1400},
            {"paraguay_badge", 1400},
            {"filipinas_badge", 1400},
            {"mexico_badge", 1400},
            {"spain_badge", 1400},
            {"arg_badge", 1400},
            {"nzl_badge", 1400},
            {"bra_badge", 1400},
            {"eng_badge", 1400},
            {"peru_badge", 1400},
            {"fra_badge", 1400 },
            {"rus_badge", 1400 }
        };

        m_bannerItems = {
            {"banner_1", 5600},
            {"banner_2", 8000},
            {"banner_9", 29500},
            {"banner_22",120500},
            {"banner_7", 67500},
            {"banner_11", 72300}
        };

        auto bg = CCSprite::create("shopBG_001.png"_spr);
        if (!bg) bg = CCSprite::create("game_bg_01_001.png");
        bg->setPosition(m_size / 2);
        bg->setScaleX(m_size.width / bg->getContentSize().width);
        bg->setScaleY(m_size.height / bg->getContentSize().height);
        this->addChild(bg, -3);

        auto desk = CCSprite::create("storeDesk_001.png"_spr);
        if (desk) {
            desk->setAnchorPoint({ 0.5f, 0.5f });
            desk->setPosition({ m_size.width / 2, m_size.height / 2 - 75.f });
            desk->setScale(0.85f);
            this->addChild(desk, -2);
        }

        auto jotabelike = CCSprite::create("jotabelike_cube.png"_spr);
        if (jotabelike) {
            jotabelike->setPosition({ m_size.width / 2 + 130.f, m_size.height / 2 + 37.f });
            jotabelike->setScale(2.0f); 
            this->addChild(jotabelike, -1);
        }

        auto sign = CCSprite::create("shopSign_001.png"_spr);
        if (sign) {
            sign->setPosition({ m_size.width / 2 - 120.f, m_size.height - 35.f });
            sign->setScale(0.55f);
            this->addChild(sign, -1);
        }

        auto backBtnMenu = CCMenu::create();
        backBtnMenu->setPosition({ 25.f, m_size.height - 25.f });
        this->addChild(backBtnMenu, 10);

        auto backSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        auto backBtn = CCMenuItemSpriteExtra::create(backSpr, this, menu_selector(ShopLayer::onBack));
        backBtnMenu->addChild(backBtn);

        setupTicketCounter();

        m_itemMenu = CCMenu::create();
        m_itemMenu->setContentSize(m_size);
        m_itemMenu->setPosition({ 0, 0 });
        this->addChild(m_itemMenu);

        setupPaginationArrows();

        updateItems();

        return true;
    }

    void setupTicketCounter() {
        auto ticketNode = CCNode::create();
        this->addChild(ticketNode, 5);

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
        ticketNode->setPosition({ m_size.width - 30.f, m_size.height - 22.f });
    }

    void setupPaginationArrows() {
        auto navMenu = CCMenu::create();
        navMenu->setContentSize(m_size);
        navMenu->setPosition({ 0, 0 });
        this->addChild(navMenu, 5);

        auto spriteLeft = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        m_prevBtn = CCMenuItemSpriteExtra::create(spriteLeft, this, menu_selector(ShopLayer::onPageChange));
        m_prevBtn->setTag(-1);
        m_prevBtn->setPosition({ 25.f, m_size.height / 2 });
        navMenu->addChild(m_prevBtn);

        auto spriteRight = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        spriteRight->setFlipX(true);
        m_nextBtn = CCMenuItemSpriteExtra::create(spriteRight, this, menu_selector(ShopLayer::onPageChange));
        m_nextBtn->setTag(1);
        m_nextBtn->setPosition({ m_size.width - 25.f, m_size.height / 2 });
        navMenu->addChild(m_nextBtn);
    }

    void onPageChange(CCObject* sender) {
        m_page += sender->getTag();
        updateItems();
    }

    void updateItems() {
        m_itemMenu->removeAllChildren();
        g_streakData.load();

        std::vector<DisplayItem> filteredItems;

        for (const auto& item : m_shopItems) {
            auto* info = g_streakData.getBadgeInfo(item.first);
            if (!info) continue;
            filteredItems.push_back({
                item.first, item.second, info->spriteName,
                g_streakData.isBadgeUnlocked(item.first), info->category, TYPE_BADGE
                });
        }

        for (const auto& item : m_bannerItems) {
            auto* info = g_streakData.getBannerInfo(item.first);
            if (!info) continue;
            filteredItems.push_back({
                item.first, item.second, info->spriteName,
                g_streakData.isBannerUnlocked(item.first), info->rarity, TYPE_BANNER
                });
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

        int cols = 4;
        float spacingX = 90.f;
        float itemSize = 60.f;

        float topShelfY = m_size.height / 2 - 42.f;
        float bottomShelfY = m_size.height / 2 - 110.f;

        for (int i = startIdx; i < endIdx; ++i) {
            int localIndex = i - startIdx;
            int row = localIndex / cols;
            int col = localIndex % cols;
            auto& data = filteredItems[i];

            auto itemNode = CCNode::create();
            itemNode->setContentSize({ itemSize, itemSize });

            auto sprite = CCSprite::create(data.spriteName.c_str());
            if (sprite) {
                float maxDimension = 60.f;
                float currentMax = std::max(sprite->getContentSize().width, sprite->getContentSize().height);
                if (currentMax > maxDimension) {
                    sprite->setScale(maxDimension / currentMax);
                }
                else {
                    sprite->setScale(0.8f);
                }

                sprite->setPosition({ itemSize / 2, itemSize / 2 + 10.f });
                itemNode->addChild(sprite);
            }

            auto priceLabel = CCLabelBMFont::create(formatPrice(data.price).c_str(), "goldFont.fnt");
            priceLabel->setScale(0.35f);

            auto priceTicketIcon = CCSprite::create("star_tiket.png"_spr);
            priceTicketIcon->setScale(0.15f);

            float totalW = priceLabel->getScaledContentSize().width + priceTicketIcon->getScaledContentSize().width + 3.f;

            priceLabel->setPosition({ (itemSize - totalW) / 2 + priceLabel->getScaledContentSize().width / 2, 8.f });
            priceTicketIcon->setPosition({ priceLabel->getPositionX() + priceLabel->getScaledContentSize().width / 2 + 3.f + priceTicketIcon->getScaledContentSize().width / 2, 8.f });

            itemNode->addChild(priceLabel);
            itemNode->addChild(priceTicketIcon);

            CCMenuItemSpriteExtra* buyBtn = CCMenuItemSpriteExtra::create(itemNode, this, menu_selector(ShopLayer::onBuyItem));
            buyBtn->setUserObject("item_id"_spr, CCString::create(data.id));

            float posX = (m_size.width / 2) + (col - (cols - 1) / 2.0f) * spacingX;
            float posY = (row == 0) ? topShelfY : bottomShelfY;

            buyBtn->setPosition({ posX, posY });
            m_itemMenu->addChild(buyBtn);

            buyBtn->setScale(0.f);

            float delay = localIndex * 0.04f;

            buyBtn->runAction(CCSequence::create(
                CCDelayTime::create(delay),
                CCEaseElasticOut::create(CCScaleTo::create(0.5f, 1.0f), 0.6f),
                nullptr
            ));

            if (data.unlocked) {
                buyBtn->setEnabled(false);

                priceLabel->setVisible(false);
                priceTicketIcon->setVisible(false);

                for (auto* child : CCArrayExt<CCNode*>(itemNode->getChildren())) {
                    if (auto* rgbaNode = typeinfo_cast<CCRGBAProtocol*>(child)) {
                        rgbaNode->setOpacity(100);
                    }
                }
                auto checkmark = CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
                checkmark->setScale(0.6f);
                checkmark->setPosition({ itemSize / 2, itemSize / 2 + 10.f });
                itemNode->addChild(checkmark, 10);
            }
        }
        updatePlayerDataInFirebase();
    }

    void onBuyItem(CCObject* sender) {
        std::string itemID = static_cast<CCString*>(static_cast<CCNode*>(sender)->getUserObject("item_id"_spr))->getCString();

        bool isBadge = (m_shopItems.find(itemID) != m_shopItems.end());

        int price = 0;
        std::string displayName = "";
        std::string spriteName = "";
        StreakData::BadgeCategory category = StreakData::BadgeCategory::COMMON;

        if (isBadge) {
            auto info = g_streakData.getBadgeInfo(itemID);
            if (!info) return;
            price = m_shopItems[itemID];
            displayName = info->displayName;
            spriteName = info->spriteName;
            category = info->category;
        }
        else {
            auto info = g_streakData.getBannerInfo(itemID);
            if (!info) return;
            price = m_bannerItems[itemID];
            displayName = info->displayName;
            spriteName = info->spriteName;
            category = info->rarity;
        }

        PurchaseConfirmPopup::create(spriteName, displayName, price, PurchaseCurrency::StarTickets,
            [this, itemID, price, category, isBadge](int discountPercent) {
            g_streakData.load();
            int finalPrice = discountedPurchasePrice(price, discountPercent);
            if (g_streakData.starTickets >= finalPrice) {
                matjson::Value payload = matjson::Value::object();
                payload.set("itemID", itemID);
                payload.set("discount_percent", discountPercent);
                claimOnServer("/shop/purchase", payload, [this, itemID, category, isBadge, keepAlive = Ref<CCNode>(this)](bool ok) {
                    if (!ok) {
                        FLAlertLayer::create("Error", "Purchase failed. Try again.", "OK")->show();
                        return;
                    }
                    if (isBadge) g_streakData.unlockBadge(itemID);
                    else g_streakData.unlockBanner(itemID);

                    m_ticketCounterLabel->setString(std::to_string(g_streakData.starTickets).c_str());
                    updateItems();
                    FMODAudioEngine::sharedEngine()->playEffect("buy_obj.mp3"_spr);

                    if (isBadge) {
                        if (category == StreakData::BadgeCategory::MYTHIC) {
                            auto badgeInfo = g_streakData.getBadgeInfo(itemID);
                            if (badgeInfo) {
                                auto animLayer = MythicAnimationLayer::create(
                                    *badgeInfo, [itemID]() { BadgeNotification::show(itemID); }
                                );
                                CCDirector::sharedDirector()->getRunningScene()->addChild(animLayer, 99999);
                            }
                        } else {
                            BadgeNotification::show(itemID);
                        }
                    } else {
                        auto info = g_streakData.getBannerInfo(itemID);
                        if (info) {
                            if (category == StreakData::BadgeCategory::MYTHIC) {
                                auto animLayer = MythicBannerAnimationLayer::create(
                                    *info, [itemID, info]() {
                                        std::string rTxt = g_streakData.getCategoryName(info->rarity);
                                        ccColor3B rCol = g_streakData.getCategoryColor(info->rarity);
                                        BannerNotification::show(itemID, info->spriteName, info->displayName, rTxt, rCol);
                                    }
                                );
                                CCDirector::sharedDirector()->getRunningScene()->addChild(animLayer, 99999);
                            } else {
                                std::string rTxt = g_streakData.getCategoryName(info->rarity);
                                ccColor3B rCol = g_streakData.getCategoryColor(info->rarity);
                                BannerNotification::show(itemID, info->spriteName, info->displayName, rTxt, rCol);
                            }
                        }
                    }
                    updatePlayerDataInFirebase();
                });
            } else {
                FLAlertLayer::create("Not Enough Tickets", "You need more tickets!", "OK")->show();
            }
            })->show();
    }

    void onBack(CCObject* sender) {
        if (m_onCloseCallback) {
            m_onCloseCallback();
        }

        GameManager::sharedState()->fadeInMenuMusic();

        CCDirector::sharedDirector()->popSceneWithTransition(0.5f, PopTransition::kPopTransitionFade);
    }

    void keyBackClicked() override {
        onBack(nullptr);
    }

public:
    static ShopLayer* create(std::function<void()> callback = nullptr) {
        auto ret = new ShopLayer();
        ret->m_onCloseCallback = callback;
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    static CCScene* scene(std::function<void()> callback = nullptr) {
        auto scene = CCScene::create();
        auto layer = ShopLayer::create(callback);
        scene->addChild(layer);
        return scene;
    }
};
