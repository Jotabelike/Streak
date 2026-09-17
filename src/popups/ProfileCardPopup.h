#pragma once
#include <Geode/ui/Popup.hpp>
#include <Geode/utils/cocos.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/utils/async.hpp>
#include <Geode/binding/ProfilePage.hpp>
#include "../StreakData.h"
#include "../StatusSpinner.h"
#include "../NameModifiers.h"
#include "../HMACAuth.h"
#include "../RemoteAssetManager.h"
#include "../ProfileCardEffects.h"
#include "../ProfileCardStyles.h"
#include "../ProfileCardDraws.h"

using namespace geode::prelude;

struct ProfileData {
    int accountID = 0;
    std::string username;
    int currentStreak = 0;
    int totalSP = 0;
    std::string badgeID;

    int level = 1;
    int currentXP = 0;
    int streakTokens = 0;
    int superStars = 0;
    int starTickets = 0;
    int gems = 0;
    std::string bannerID;
    std::string streakID;
    int globalRank = 0;
    bool isMythic = false;

     
    std::string nameColor = "Default";
    std::string nameFont = "Default";
    std::string nameEffect = "None";
    std::string nameAnimation = "None";
    std::string profileEffect = "None";
    std::string profilePopup = ProfileCardStyles::DEFAULT_STYLE;
    std::vector<std::string> profileDraws;

    bool isPartialData = false;
    bool showGDProfileButton = false;
};

class ProfileCardPopup : public Popup {
protected:
    ProfileData m_data;
    async::TaskHolder<web::WebResponse> m_fullDataListener;
    StatusSpinner* m_loadingSpinner = nullptr;
    bool m_profileBuilt = false;
    bool m_transitionFinished = false;
    bool m_effectStarted = false;

    CCSprite* createDrawSprite(const ProfileCardDraws::DrawInfo& draw, const CCSize& size, float extraScale = 1.f) {
        auto sprite = CCSprite::create(ProfileCardDraws::resolveSprite(draw).c_str());
        if (!sprite || sprite->getContentSize().width <= 0.f || sprite->getContentSize().height <= 0.f) return nullptr;
        float layoutScale = size.height / 180.f;
        float spriteScale = (draw.displayHeight * layoutScale / sprite->getContentSize().height) * extraScale;
        sprite->setScale(spriteScale);
        sprite->setPosition({
            draw.centerX * (size.width / 280.f),
            draw.centerY * layoutScale
        });
        return sprite;
    }

    void buildProfileDraws() {
        if (m_data.profileDraws.empty()) return;

        auto winSize = m_mainLayer->getContentSize();
        auto maskedLayer = CCClippingNode::create();
        maskedLayer->setContentSize(winSize);
        maskedLayer->setAlphaThreshold(0.08f);

        auto stencil = cocos2d::extension::CCScale9Sprite::create(ProfileCardStyles::resolve(m_data.profilePopup));
        stencil->setContentSize({ winSize.width - 6.f, winSize.height - 6.f });
        stencil->setPosition({ winSize.width / 2.f, winSize.height / 2.f });
        maskedLayer->setStencil(stencil);

        bool hasMaskedDraw = false;
        std::size_t drawCount = 0;
        for (auto const& drawID : m_data.profileDraws) {
            if (drawCount++ >= ProfileCardDraws::MAX_EQUIPPED) break;
            auto info = ProfileCardDraws::getInfo(drawID);
            if (!info) continue;

            if (info->type == ProfileCardDraws::DrawType::Masked) {
                if (auto sprite = createDrawSprite(*info, winSize)) {
                    maskedLayer->addChild(sprite);
                    hasMaskedDraw = true;
                }
            }
            else if (auto sprite = createDrawSprite(*info, winSize, 1.04f)) {
                m_mainLayer->addChild(sprite, 11);
            }
        }

        // Masked drawings decorate the card artwork but remain behind every
        // label and stat. Overlay drawings intentionally stay above at z=11.
        if (hasMaskedDraw) m_mainLayer->addChild(maskedLayer, 1);
    }

    void startProfileEffect() {
        if (m_effectStarted || !m_profileBuilt || m_data.profileEffect == "None") return;
        m_effectStarted = true;

        auto winSize = m_mainLayer->getContentSize();
        auto effectMask = CCClippingNode::create();
        effectMask->setContentSize(winSize);
        effectMask->setAlphaThreshold(0.08f);

        // Match the rounded inner body of GE_square01 instead of using the
        // full rectangular layer bounds.
        auto stencil = cocos2d::extension::CCScale9Sprite::create(ProfileCardStyles::resolve(m_data.profilePopup));
        stencil->setContentSize({ winSize.width - 6.f, winSize.height - 6.f });
        stencil->setPosition({ winSize.width / 2.f, winSize.height / 2.f });
        effectMask->setStencil(stencil);

        // The clipping stencil already keeps the animation inside the rounded
        // popup. Use almost all of that space so full-card effects actually
        // reach the visible edges instead of forming a smaller inner box.
        constexpr float effectInsetX = 3.f;
        constexpr float effectInsetY = 3.f;
        auto profileEffect = ProfileCardEffects::createEffect(
            m_data.profileEffect,
            { winSize.width - effectInsetX * 2.f, winSize.height - effectInsetY * 2.f },
            false
        );
        profileEffect->setPosition({ effectInsetX, effectInsetY });
        effectMask->addChild(profileEffect);
        m_mainLayer->addChild(effectMask, 9);

        // Every profile animation is an entrance effect. Removing the entire
        // clipped layer guarantees no glow or wave can remain afterwards.
        effectMask->runAction(CCSequence::create(
            CCDelayTime::create(3.1f),
            CallFuncExt::create([effectMask]() {
                effectMask->removeFromParentAndCleanup(true);
            }),
            nullptr
        ));
    }

    void onEnterTransitionDidFinish() override {
        CCLayer::onEnterTransitionDidFinish();
        m_transitionFinished = true;
        startProfileEffect();
    }

    void onCopyID(CCObject*) {
        if (m_data.streakID.empty() || m_data.streakID == "Pending...") return;
        clipboard::write(m_data.streakID);
        Notification::create("ID Copied!", NotificationIcon::Success)->show();
    }

    void onOpenGDProfile(CCObject*) {
        if (m_data.accountID > 0) {
            ProfilePage::create(m_data.accountID, false)->show();
        }
    }

    void addStatItem(float x, float y, const std::string& iconName, const std::string& textStr, float iconScale) {
        auto icon = CCSprite::create(iconName.c_str());
        if (!icon) icon = CCSprite::createWithSpriteFrameName("GJ_starsIcon_001.png");
        icon->setScale(iconScale);
        icon->setPosition({ x, y });
        m_mainLayer->addChild(icon, 2);

        auto label = CCLabelBMFont::create(textStr.c_str(), "bigFont.fnt");
        label->setScale(0.35f);
        label->setAnchorPoint({ 0.0f, 0.5f });
        label->setPosition({ x + 12.f, y });
        m_mainLayer->addChild(label, 2);
    }

    void buildProfileUI() {
        auto winSize = m_mainLayer->getContentSize();

        if (m_data.globalRank > 0) {
            float rankY = winSize.height - 22.f;

            auto rankIcon = CCSprite::createWithSpriteFrameName("rankIcon_top10_001.png");
            if (rankIcon) {
                rankIcon->setScale(0.5f);
                rankIcon->setPosition({ winSize.width - 50.f, rankY });
                m_mainLayer->addChild(rankIcon, 10);
            }

            auto rankLabel = CCLabelBMFont::create(fmt::format("#{}", m_data.globalRank).c_str(), "goldFont.fnt");
            rankLabel->setScale(0.4f);
            rankLabel->setAnchorPoint({ 0.0f, 0.5f });
            rankLabel->setPosition({ winSize.width - 38.f, rankY });
            m_mainLayer->addChild(rankLabel, 10);
        }

        float bannerCenterX = (winSize.width / 2) + 10.f;
        float bannerCenterY = winSize.height / 2 + 25.f;

        if (auto bannerInfo = g_streakData.getBannerInfo(m_data.bannerID)) {
            auto bannerSprite = CCSprite::create(bannerInfo->spriteName.c_str());
            if (bannerSprite) {
                bannerSprite->setPosition({ bannerCenterX, bannerCenterY });
                float targetWidth = 290.0f;
                float scale = targetWidth / bannerSprite->getContentSize().width;
                if (bannerSprite->getContentSize().height * scale > 130.f) scale = 130.f / bannerSprite->getContentSize().height;
                bannerSprite->setScale(scale);

                bannerSprite->setOpacity(0);
                bannerSprite->runAction(CCFadeIn::create(0.3f));

                m_mainLayer->addChild(bannerSprite, 1);
            }
        }

        if (auto badgeInfo = g_streakData.getBadgeInfo(m_data.badgeID)) {
            auto badge = CCSprite::create(badgeInfo->spriteName.c_str());
            if (badge) {
                badge->setPosition({ bannerCenterX - 120.f, bannerCenterY });
                badge->setScale(0.25f);
                m_mainLayer->addChild(badge, 6);
            }
        }

        float fireX = bannerCenterX + 105.f;
        auto fireIcon = CCSprite::create(g_streakData.getRachaSprite(m_data.currentStreak).c_str());
        if (!fireIcon) fireIcon = CCSprite::createWithSpriteFrameName("fireIcon_001.png");
        fireIcon->setPosition({ fireX, bannerCenterY + 5.f });
        fireIcon->setScale(0.24f);
        m_mainLayer->addChild(fireIcon, 6);

        auto streakLabel = CCLabelBMFont::create(std::to_string(m_data.currentStreak).c_str(), "bigFont.fnt");
        streakLabel->setScale(0.35f);
        streakLabel->setPosition({ fireX, bannerCenterY - 20.f });
        m_mainLayer->addChild(streakLabel, 6);

        float textStartX = bannerCenterX - 95.f;
        std::string userName = m_data.username.empty() ? "Player" : m_data.username;

      
        auto nameContainer = CCNode::create();
        nameContainer->setPosition({ textStartX, bannerCenterY + 8.f });
        m_mainLayer->addChild(nameContainer, 5);

        auto nameLabel = CCLabelBMFont::create(userName.c_str(), "bigFont.fnt");
        nameLabel->setScale(0.6f);
        nameLabel->setAnchorPoint({ 0.0f, 0.5f });
        nameLabel->setPosition({ 0, 0 });
        nameContainer->addChild(nameLabel);
         
        NameModifiers::applyFont(nameLabel, m_data.nameFont);
        NameModifiers::applyColor(nameLabel, m_data.nameColor);
        NameModifiers::applyAnimation(nameLabel, m_data.nameAnimation);
        NameModifiers::applyEffect(nameLabel, m_data.nameEffect);

   

        auto xpIcon = CCSprite::create("xp.png"_spr);
        if (!xpIcon) xpIcon = CCSprite::createWithSpriteFrameName("GJ_starsIcon_001.png");
        xpIcon->setScale(0.08f);
        float iconHalfWidth = (xpIcon->getContentSize().width * xpIcon->getScale()) / 2;
        xpIcon->setPosition({ textStartX + iconHalfWidth, bannerCenterY - 12.f });
        m_mainLayer->addChild(xpIcon, 5);

        auto levelLabel = CCLabelBMFont::create(fmt::format("Lvl {}", m_data.level).c_str(), "goldFont.fnt");
        levelLabel->setScale(0.45f);
        levelLabel->setAnchorPoint({ 0.0f, 0.5f });
        float textOffsetX = (xpIcon->getContentSize().width * xpIcon->getScale()) + 3.f;
        levelLabel->setPosition({ textStartX + textOffsetX, bannerCenterY - 12.f });
        m_mainLayer->addChild(levelLabel, 5);

        float rankCursorX = levelLabel->getPositionX()
            + levelLabel->getContentSize().width * levelLabel->getScaleX() + 6.f;
        if (auto rankSprite = CCSprite::create(StreakData::getRankSprite(m_data.streakTokens).c_str())) {
            float targetHeight = xpIcon->getContentSize().height * xpIcon->getScaleY();
            float spriteHeight = rankSprite->getContentSize().height;
            rankSprite->setScale((spriteHeight > 0.f ? targetHeight / spriteHeight : 0.03f) * 1.1f);
            rankSprite->setAnchorPoint({ 0.f, 0.5f });
            rankSprite->setPosition({ rankCursorX, bannerCenterY - 13.f });
            m_mainLayer->addChild(rankSprite, 5);
            rankCursorX += rankSprite->getContentSize().width * rankSprite->getScaleX() + 3.f;
        }

        auto rankNameLabel = CCLabelBMFont::create(
            StreakData::getRankName(m_data.streakTokens).c_str(), "bigFont.fnt");
        rankNameLabel->limitLabelWidth(std::max(28.f, fireX - rankCursorX - 13.f), 0.3f, 0.1f);
        rankNameLabel->setAnchorPoint({ 0.f, 0.5f });
        rankNameLabel->setPosition({ rankCursorX, bannerCenterY - 12.f });
        m_mainLayer->addChild(rankNameLabel, 5);
        NameModifiers::applyColor(rankNameLabel, StreakData::getRankColorStyle(m_data.streakTokens));

        float col1_X = 35.0f;
        float col2_X = winSize.width / 2 + 35.f;
        float row1_Y = 60.0f;
        float row2_Y = 35.0f;
        float statsScale = 0.12f;
        float dotScale = 0.12f;

        this->addStatItem(col1_X, row1_Y, "streak_point.png"_spr, fmt::format("{} SP", m_data.totalSP), dotScale);
        this->addStatItem(col1_X, row2_Y, "gem.png"_spr, fmt::format("{}", m_data.gems), statsScale);
        this->addStatItem(col2_X, row1_Y, "super_star.png"_spr, fmt::format("{}", m_data.superStars), statsScale);
        this->addStatItem(col2_X, row2_Y, "star_tiket.png"_spr, fmt::format("{}", m_data.starTickets), statsScale);

        std::string idStr = fmt::format("ID: {}", m_data.streakID);
        auto idLabel = CCLabelBMFont::create(idStr.c_str(), "chatFont.fnt");
        idLabel->setScale(0.5f);
        idLabel->setOpacity(150);
        idLabel->setColor({ 200, 200, 255 });

        auto idBtn = CCMenuItemSpriteExtra::create(idLabel, this, menu_selector(ProfileCardPopup::onCopyID));
        idBtn->setPosition({ winSize.width - 8.f, 5.f });
        idBtn->setAnchorPoint({ 1.0f, 0.0f });
        auto menuID = CCMenu::create();
        menuID->setPosition({ 0, 0 });
        menuID->addChild(idBtn);
        m_mainLayer->addChild(menuID, 10);

        buildProfileDraws();

        m_profileBuilt = true;
        if (m_transitionFinished) startProfileEffect();
    }

    void fetchFullProfile() {
        std::string url = fmt::format("https://streak-servidor.onrender.com/players/{}/public-profile", m_data.accountID);
        auto req = web::WebRequest();

        m_fullDataListener.spawn(req.get(url), [this](web::WebResponse res) {
            if (res.ok() && res.json().isOk()) {
                auto json = res.json().unwrap();

                m_data.level = json["current_level"].as<int>().unwrapOr(1);
                m_data.currentXP = json["current_xp"].as<int>().unwrapOr(0);
                // Keep the value supplied by the leaderboard if an older or
                // incomplete public-profile response does not include tokens.
                m_data.streakTokens = json["streak_tokens"].as<int>().unwrapOr(m_data.streakTokens);
                m_data.superStars = json["super_stars"].as<int>().unwrapOr(0);
                m_data.starTickets = json["star_tickets"].as<int>().unwrapOr(0);
                m_data.gems = json["gems"].as<int>().unwrapOr(0);
                m_data.bannerID = json["equipped_banner_id"].as<std::string>().unwrapOr("");
                if (!m_data.badgeID.empty()) {
                    RemoteAssets::ensure(RemoteAssets::Type::Badge, m_data.badgeID);
                }
                if (!m_data.bannerID.empty()) {
                    RemoteAssets::ensure(RemoteAssets::Type::Banner, m_data.bannerID);
                }
                m_data.streakID = json["streakID"].as<std::string>().unwrapOr("???");
                m_data.nameFont = json["equipped_name_font"].as<std::string>().unwrapOr("Default");
                m_data.nameColor = json["equipped_name_color"].as<std::string>().unwrapOr("Default");
                m_data.nameEffect = json["equipped_name_effect"].as<std::string>().unwrapOr("None");
                m_data.nameAnimation = json["equipped_name_animation"].as<std::string>().unwrapOr("None");
                m_data.profileEffect = json["equipped_profile_effect"].as<std::string>().unwrapOr("None");
                m_data.profilePopup = json["equipped_profile_popup"].as<std::string>().unwrapOr(ProfileCardStyles::DEFAULT_STYLE);
                m_data.profileDraws = json["equipped_profile_draws"].as<std::vector<std::string>>().unwrapOr(std::vector<std::string>{});

                if (json.contains("rank")) m_data.globalRank = json["rank"].as<int>().unwrapOr(0);
                else if (json.contains("global_rank")) m_data.globalRank = json["global_rank"].as<int>().unwrapOr(0);

                if (m_loadingSpinner) {
                    m_loadingSpinner->removeFromParent();
                    m_loadingSpinner = nullptr;
                }

                this->buildProfileUI();
            }
            else {
                if (m_loadingSpinner) m_loadingSpinner->setError("Error");
            }
            });
    }

    bool init(ProfileData data) {
        if (!Popup::init(280.f, 180.f, ProfileCardStyles::resolve(data.profilePopup))) return false;
        m_data = data;
        this->setTitle("Player Profile");

        // Keep the Geometry Dash profile accessible without replacing the
        // Streak card shown when a leaderboard name is selected.
        if (m_data.showGDProfileButton) {
            if (auto profileSprite = CCSprite::createWithSpriteFrameName("GJ_profileButton_001.png")) {
                profileSprite->setScale(0.55f);
                auto profileButton = CCMenuItemSpriteExtra::create(
                    profileSprite,
                    this,
                    menu_selector(ProfileCardPopup::onOpenGDProfile)
                );
                profileButton->setID("gd-profile-button");
                auto size = m_mainLayer->getContentSize();
                profileButton->setPosition({ size.width + 17.f, size.height - 20.f });
                profileButton->setEnabled(m_data.accountID > 0);

                auto profileMenu = CCMenu::createWithItem(profileButton);
                profileMenu->setID("gd-profile-menu");
                profileMenu->setPosition({ 0.f, 0.f });
                m_mainLayer->addChild(profileMenu, 1000);
            }
        }

        if (m_data.isPartialData) {
            m_loadingSpinner = StatusSpinner::create();
            m_loadingSpinner->setLoading("Loading Profile...");
            m_loadingSpinner->setPosition(m_mainLayer->getContentSize() / 2);
            m_mainLayer->addChild(m_loadingSpinner, 100);

            this->fetchFullProfile();
        }
        else {
            this->buildProfileUI();
        }

        return true;
    }

public:
    static ProfileCardPopup* create(ProfileData data) {
        auto ret = new ProfileCardPopup();
        if (ret && ret->init(data)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};
