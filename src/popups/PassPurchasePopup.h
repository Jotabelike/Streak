#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>

using namespace geode::prelude;

class PassPurchasePopup : public Popup {
protected:
    int m_price = 0;
    std::function<void()> m_onConfirm;

    bool init() {
        if (!Popup::init(380.f, 320.f, "geode.loader/GE_square03.png")) return false;
        auto winSize = m_mainLayer->getContentSize();

        if (m_bgSprite) m_bgSprite->setVisible(false);

        float artW = 0.f, artH = 0.f;
        float artCenterY = winSize.height / 2.f + 14.f;
        auto art = CCSprite::create("pass_img.png"_spr);
        if (art) {
            float maxW = winSize.width - 24.f;
            float maxH = winSize.height - 70.f;
            float s = std::min(
                maxW / std::max(art->getContentSize().width, 1.f),
                maxH / std::max(art->getContentSize().height, 1.f)
            );
            art->setScale(s);
            art->setPosition({ winSize.width / 2.f, artCenterY });
            m_mainLayer->addChild(art, 1);
            artW = art->getScaledContentSize().width;
            artH = art->getScaledContentSize().height;
        }
        float artBottom = artCenterY - artH / 2.f;
        float artTop = artCenterY + artH / 2.f;

        auto menu = CCMenu::create();
        menu->setPosition({ 0.f, 0.f });
        m_mainLayer->addChild(menu, 5);

        CCMenuItemSpriteExtra* buyBtn = nullptr;
        if (m_price <= 0) {
            auto spr = ButtonSprite::create("Free", 0, 0, "goldFont.fnt", "GJ_button_01.png", 0, 0.9f);
            buyBtn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(PassPurchasePopup::onBuy));
        } else {
            auto spr = ButtonSprite::create(
                fmt::format("   {}", m_price).c_str(), 0, 0, "bigFont.fnt", "GJ_button_01.png", 0, 0.9f
            );
            auto gem = CCSprite::create("gem.png"_spr);
            if (gem) {
                auto sprSize = spr->getContentSize();
                gem->setScale(0.55f);
                gem->setPosition({ 18.f, sprSize.height / 2.f });
                spr->addChild(gem);
            }
            buyBtn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(PassPurchasePopup::onBuy));
        }
        buyBtn->setPosition({ winSize.width / 2.f, artBottom + 16.f });
        menu->addChild(buyBtn);

        if (m_closeBtn && m_closeBtn->getParent()) {
            auto screen = CCDirector::sharedDirector()->getWinSize();
            CCPoint worldCorner = { 25.f, screen.height - 25.f };
            m_closeBtn->setPosition(m_closeBtn->getParent()->convertToNodeSpace(worldCorner));
        }

        return true;
    }

    void onBuy(CCObject*) {
        FMODAudioEngine::sharedEngine()->playEffect("buyItem01.ogg");
        auto cb = m_onConfirm;
        this->onClose(nullptr);
        if (cb) cb();
    }

public:
    static PassPurchasePopup* create(int price, std::function<void()> onConfirm) {
        auto ret = new PassPurchasePopup();
        ret->m_price = price;
        ret->m_onConfirm = onConfirm;
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};
