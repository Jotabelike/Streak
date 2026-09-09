#pragma once
#include "../StreakData.h"
#include "../NameModifiers.h"
#include <Geode/binding/ProfilePage.hpp>
#include <Geode/binding/GJAccountManager.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/SimplePlayer.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/utils/async.hpp>

using namespace geode::prelude;

// A snapshot of the same ordered response shown in LeaderboardPopup.
// Keeping the previous scene on the stack preserves its popup and scroll position.
class LeaderboardPodiumLayer : public CCLayer {
protected:
    bool m_leaving = false;
    bool m_riseStarted = false;
    float m_stageBottom = 0.f;
    std::vector<std::pair<CCNode*, int>> m_places;
    std::array<async::TaskHolder<web::WebResponse>, 3> m_cubeRequests;

    struct CubeAppearance {
        int frame = 1;
        int color1 = 0;
        int color2 = 3;
        int glowColor = 3;
        bool glow = false;
        std::chrono::steady_clock::time_point fetchedAt;
    };
    inline static std::map<int, CubeAppearance> s_cubeCache;

    static void applyCube(SimplePlayer* cube, const CubeAppearance& appearance) {
        auto gm = GameManager::sharedState();
        int frame = std::clamp(appearance.frame, 1, std::max(1, gm->countForType(IconType::Cube)));
        cube->updatePlayerFrame(frame, IconType::Cube);
        cube->setColor(gm->colorForIdx(appearance.color1));
        cube->setSecondColor(gm->colorForIdx(appearance.color2));
        if (appearance.glow) cube->setGlowOutline(gm->colorForIdx(appearance.glowColor));
        else cube->disableGlowOutline();
        cube->updateColors();
        cube->setOpacity(255);
    }

    void addCube(CCNode* place, int id, int rank, float height, bool isMe) {
        auto cube = SimplePlayer::create(1);
        cube->setID("podium-player-cube");
        cube->setScale(1.f);
        cube->setPosition({ 0.f, height + 1.f });
        place->addChild(cube, 3);

        if (isMe) {
            auto gm = GameManager::sharedState();
            applyCube(cube, { gm->getPlayerFrame(), gm->getPlayerColor(), gm->getPlayerColor2(),
                gm->getPlayerGlowColor(), gm->getPlayerGlow(), std::chrono::steady_clock::now() });
            return;
        }
        if (auto cached = s_cubeCache.find(id); cached != s_cubeCache.end()
            && std::chrono::steady_clock::now() - cached->second.fetchedAt < std::chrono::minutes(5)) {
            applyCube(cube, cached->second);
            return;
        }

        // A dim placeholder is explicitly marked until the real profile arrives.
        cube->setColor({ 125, 135, 150 });
        cube->setSecondColor({ 75, 85, 100 });
        cube->setOpacity(110);
        auto status = label(place, id > 0 ? "..." : "?", "chatFont.fnt", { 0.f, height + 1.f }, 0.6f, 25.f);
        place->reorderChild(status, 4);
        if (id <= 0) return;

        // Independent requests avoid taking over GameLevelManager's single
        // UserInfoDelegate, which ProfilePage needs when a name is clicked.
        auto request = web::WebRequest();
        request.userAgent("");
        request.timeout(std::chrono::seconds(15));
        request.header("Content-Type", "application/x-www-form-urlencoded");
        request.bodyString(fmt::format("targetAccountID={}&secret=Wmfd2893gb7&gameVersion=22", id));
        m_cubeRequests[rank - 1].spawn(request.post("https://www.boomlings.com/database/getGJUserInfo20.php"),
            [cube, status, id](web::WebResponse response) {
                if (!response.ok()) { status->setString("?"); return; }
                std::istringstream stream(response.string().unwrapOr(""));
                std::map<std::string, std::string> fields;
                std::string key, value;
                while (std::getline(stream, key, ':') && std::getline(stream, value, ':')) fields[key] = value;
                auto number = [&](const char* key, int fallback) {
                    auto it = fields.find(key);
                    if (it == fields.end()) return fallback;
                    try {
                        size_t consumed = 0;
                        int value = std::stoi(it->second, &consumed);
                        return consumed == it->second.size() ? value : fallback;
                    } catch (...) { return fallback; }
                };
                if (number("16", 0) != id || number("21", 0) <= 0) { status->setString("?"); return; }
                CubeAppearance appearance;
                appearance.frame = number("21", 1);
                appearance.color1 = std::clamp(number("10", 0), 0, 106);
                appearance.color2 = std::clamp(number("11", 3), 0, 106);
                appearance.glowColor = std::clamp(number("51", 3), 0, 106);
                appearance.glow = number("28", 0) != 0;
                appearance.fetchedAt = std::chrono::steady_clock::now();
                s_cubeCache[id] = appearance;
                applyCube(cube, appearance);
                status->setVisible(false);
            });
    }

    static CCLabelBMFont* label(CCNode* parent, const std::string& text,
        const char* font, CCPoint position, float scale, float width = 125.f) {
        auto result = CCLabelBMFont::create(text.c_str(), font);
        result->limitLabelWidth(width, scale, 0.1f);
        result->setPosition(position);
        parent->addChild(result, 3);
        return result;
    }

    static int accountID(const matjson::Value& player) {
        if (player["accountID"].isNumber()) return player["accountID"].as<int>().unwrapOr(0);
        if (player["accountID"].isString()) {
            try { return std::stoi(player["accountID"].as<std::string>().unwrapOr("0")); }
            catch (...) { }
        }
        return 0;
    }

    void addPlace(CCNode* stage, const std::vector<matjson::Value>& players,
        int rank, float x, float height, ccColor3B accent) {
        auto place = CCNode::create();
        place->setID(fmt::format("podium-place-{}", rank));
        // Start the entire place (including the name) below the viewport.
        place->setPosition({ x, m_stageBottom - height - 85.f });
        stage->addChild(place, rank == 1 ? 2 : 1);
        m_places.emplace_back(place, rank);

        auto column = CCSprite::create("podium_column.png"_spr);
        if (column) {
            // All three columns retain the original aspect ratio. Only their
            // top positions differ; their long shafts extend below the viewport.
            column->setAnchorPoint({ 0.5f, 1.f });
            column->setPosition({ 0.f, height });
            column->setScale(180.f / column->getContentSize().width);
            column->setColor(accent);
            place->addChild(column);
        }

        auto medal = CCSprite::create(fmt::format("top{}.png"_spr, rank).c_str());
        if (medal) {
            medal->setScale(50.4f / medal->getContentSize().height);
            medal->setPosition({ 0.f, height - 62.f });
            place->addChild(medal, 2);
        } else {
            label(place, fmt::format("#{}", rank), "goldFont.fnt", { 0.f, height - 62.f }, 0.65f);
        }

        if (players.size() < static_cast<size_t>(rank)) {
            label(place, "Open place", "bigFont.fnt", { 0.f, height + 25.f }, 0.38f)
                ->setColor({ 140, 155, 180 });
            return;
        }

        const auto& player = players[rank - 1];
        int id = accountID(player);
        auto am = GJAccountManager::sharedState();
        bool isMe = am && id > 0 && id == am->m_accountID;
        addCube(place, id, rank, height, isMe);
        auto cosmetic = [&](const char* key, const std::string& local, const char* fallback) {
            return isMe ? local : player[key].as<std::string>().unwrapOr(fallback);
        };

        auto badgeID = cosmetic("equipped_badge_id", g_streakData.equippedBadge, "");
        CCSprite* badge = nullptr;
        if (auto info = g_streakData.getBadgeInfo(badgeID)) {
            badge = CCSprite::create(info->spriteName.c_str());
        }
        if (badge) {
            badge->setScale(27.6f / std::max(badge->getContentSize().width, badge->getContentSize().height));
            badge->setPosition({ 0.f, height - 103.f });
            place->addChild(badge, 2);
        }

        auto name = CCLabelBMFont::create(player["username"].as<std::string>().unwrapOr("Unknown").c_str(), "goldFont.fnt");
        NameModifiers::applyFont(name, cosmetic("equipped_name_font", g_streakData.equippedNameFont, "Default"));
        name->limitLabelWidth(126.f, 0.55f, 0.1f);
        NameModifiers::applyColor(name, cosmetic("equipped_name_color", g_streakData.equippedNameColor, "Default"));
        auto profile = CCMenuItemSpriteExtra::create(name, this, menu_selector(LeaderboardPodiumLayer::onProfile));
        profile->setTag(id);
        profile->setEnabled(id > 0);
        profile->setPosition({ 0.f, height + 48.f });
        auto profileMenu = CCMenu::createWithItem(profile);
        profileMenu->setID("podium-profile-menu");
        profileMenu->setTouchEnabled(false);
        profileMenu->setPosition({ 0.f, 0.f });
        place->addChild(profileMenu, 4);
        NameModifiers::applyEffect(name, cosmetic("equipped_name_effect", g_streakData.equippedNameEffect, "None"));
        NameModifiers::applyAnimation(name, cosmetic("equipped_name_animation", g_streakData.equippedNameAnimation, "None"));

        int days = player["current_streak_days"].as<int>().unwrapOr(0);
        auto daysLabel = label(place, fmt::format("{} {}", days, days == 1 ? "day" : "days"),
            "bigFont.fnt", { 0.f, height - 24.f }, 0.42f, 82.f);
        NameModifiers::applyColor(daysLabel,
            cosmetic("equipped_name_color", g_streakData.equippedNameColor, "Default"));

    }

    void onEnterTransitionDidFinish() override {
        CCLayer::onEnterTransitionDidFinish();
        if (m_riseStarted) return;
        m_riseStarted = true;
        for (auto [place, rank] : m_places) {
            place->runAction(CCSequence::create(
                CCDelayTime::create((3 - rank) * 0.12f),
                CCEaseSineOut::create(CCMoveTo::create(0.95f, { place->getPositionX(), 27.f })),
                CallFuncExt::create([place]() {
                    if (auto menu = typeinfo_cast<CCMenu*>(place->getChildByID("podium-profile-menu"))) {
                        menu->setTouchEnabled(true);
                    }
                }),
                nullptr
            ));
        }
    }

    bool init(const std::vector<matjson::Value>& players) {
        if (!CCLayer::init()) return false;
        this->setKeypadEnabled(true);
        this->setID("leaderboard-podium-layer");
        auto size = CCDirector::sharedDirector()->getWinSize();
        auto background = CCLayerGradient::create({ 19, 34, 61, 255 }, { 5, 10, 22, 255 });
        background->setContentSize(size);
        this->addChild(background, -2);

        auto stage = CCNode::create();
        stage->setID("podium-stage");
        stage->setContentSize({ 480.f, 320.f });
        stage->setAnchorPoint({ 0.5f, 0.5f });
        stage->setPosition(size / 2);
        float stageScale = std::min(size.width / 480.f, size.height / 320.f);
        stage->setScale(stageScale);
        m_stageBottom = (320.f - size.height / stageScale) / 2.f;
        // This is a full-screen scene: the render viewport already clips the
        // column shafts. A stencil around the whole UI can mask every child,
        // including the title and back button, on the game's renderer.
        this->addChild(stage);

        label(stage, "Top Streaks", "goldFont.fnt", { 240.f, 296.f }, 0.85f, 270.f);

        addPlace(stage, players, 2, 100.f, 120.f, { 215, 231, 255 });
        addPlace(stage, players, 1, 240.f, 165.f, { 255, 232, 175 });
        addPlace(stage, players, 3, 380.f, 98.f, { 238, 193, 157 });

        auto backSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        backSprite->setScale(0.75f);
        auto back = CCMenuItemSpriteExtra::create(backSprite, this, menu_selector(LeaderboardPodiumLayer::onBack));
        back->setID("podium-back-button");
        back->setPosition({ 25.f, 295.f });
        auto menu = CCMenu::createWithItem(back);
        menu->setPosition({ 0.f, 0.f });
        stage->addChild(menu, 10);
        return true;
    }

    void onProfile(CCObject* sender) {
        int id = static_cast<CCNode*>(sender)->getTag();
        if (id > 0) ProfilePage::create(id, false)->show();
    }

    void onBack(CCObject*) {
        if (m_leaving) return;
        m_leaving = true;
        CCDirector::sharedDirector()->popSceneWithTransition(0.3f, PopTransition::kPopTransitionFade);
    }

    void keyBackClicked() override { onBack(nullptr); }

public:
    static CCScene* scene(const std::vector<matjson::Value>& players) {
        auto layer = new LeaderboardPodiumLayer();
        if (!layer->init(players)) {
            delete layer;
            return nullptr;
        }
        layer->autorelease();
        auto scene = CCScene::create();
        scene->addChild(layer);
        return scene;
    }
};
