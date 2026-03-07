#pragma once
#include "StreakCommon.h"
#include "../StreakData.h"
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/ScrollLayer.hpp>
#include <Geode/binding/GameLevelManager.hpp>
#include <Geode/binding/GJGameLevel.hpp>
#include <Geode/binding/LevelTools.hpp>
#include <Geode/binding/LevelBrowserLayer.hpp>
#include <Geode/binding/GJSearchObject.hpp>
#include "../BadgeNotification.h" 

using namespace geode::prelude;

struct MasteryLevelDef {
    int levelID;
    int difficulty;
    std::string name;
};

struct DisplayItem {
    std::string id;
    std::string name;
    std::string spriteName;
    int daysRequired;
    bool isUnlocked;
    bool isMastery;
};

std::vector<MasteryLevelDef> getLevelsForMastery(const std::string& masteryBadgeID) {
    if (masteryBadgeID == "wave_mastery_badge") {
        return {
            {128692093, 11, "TOUCH THE ORBS.."},
            {128041774, 7,  "OuTbreak EternAl"},
            {127965838, 10, "Arch Linux"},
            {127361523, 7,  "Berserker"},
            {126429443, 12, "BAD NUMBER"},
            {130987453, 5,  "Cybernetic Chris"}
        };
    }
    else if (masteryBadgeID == "ball_mastery_badge") {
        return {
            {126365388, 4, "Meow or Never"},
            {118505448, 5, "Johnny Mnemonic"},
            {116347530, 4, "Balling"},
            {110743683, 5, "Rockstar Maid"},
            {87257787, 10, "V01D"},
            {84837652, 7,  "Shibuya Rush"}
        };
    }
    else if (masteryBadgeID == "cube_mastery_badge") {
        return {
            {129592487, 4, "RuN BelasteT RuN"},
            {129492009, 5, "RANDOMUS TRIG.."},
            {129484160, 7, "Pirate Party"},
            {129471967, 7, "Obsession"},
            {129361202, 7, "ToweRooms"},
            {128220607, 10, "Entertnmnt"}
        };
    }
    else if (masteryBadgeID == "ship_mastery_badge") {
        return {
            {128717428, 7,  "sire timmy"},
            {127965838, 10, "Arch Linux"},
            {127119965, 7,  "Magma Opus"},
            {125881154, 10, "cataclysm"},
            {125746520, 10, "Stomp"},
            {125573777, 7,  "Coast at twilight"}
        };
    }
    else if (masteryBadgeID == "spider_mastery_badge") {
        return {
            {130426740, 10, "Xanthesiderite"},
            {129168190, 11, "Everyday"},
            {129140431, 13, "Heavy Gears"},
            {128227129, 7,  "SpyderFUNK"},
            {123926083, 10, "Himari"},
            {119339739, 11, "Dungewolf"}
        };
    }
    else if (masteryBadgeID == "ufo_mastery_badge") {
        return {
            {126985270, 5,  "P A R A L L A X"},
            {123446731, 10, "COPYCAT"},
            {117234331, 5,  "Firefly Forest"},
            {113854244, 7,  "TROUBLE"},
            {113049620, 10, "Cheat Codes"},
            {107028394, 10, "AMELANCHIER"}
        };
    }

    else if (masteryBadgeID == "dual_mastery_badge") {
        return {
            {130281428, 7,  "Wal Lal Lal Lu"},
            {44062068, 12, "Future Funk"},
            {59626284, 13,  "Future Funk II"},
            {124595512, 10,  "Cut the Check"},
            {124034465, 7, "Streetwise"},
            {123962517, 7, "AntiPixel"}
        };
    }

    else if (masteryBadgeID == "xl_mastery_badge") {
        return {
            {128168373, 10,  "Blend Travel"},
            {127118533, 7, "Cosmic Travel"},
            {126512409, 10,  "Holding On"},
            {125075242, 10,  "Space Invaders"},
            {124931561, 12, "ctrl alt del"},
            {120035482, 7, "Music In My Mind"}
        };
    }

    else if (masteryBadgeID == "swingcopter_mastery_badge") {
        return {
            {130175757, 10,  "Roentgenium"},
            {127118533, 7, "Minun"},
            {116284755, 7,  "Out of Place"},
            {123617195, 10,  "ALLOY"},
            {116581248, 7, "Corrupt cityscapes"},
            {112799855, 10, "Northstar"}
        };
    }

    else if (masteryBadgeID == "memory_mastery_badge") {
        return {
            {130046620, 12,  "Chronologic"},
            {122968809, 11, "TWIN TABULATING"},
            {118365197, 7,  "Winle Domination"},
            {121087325, 11,  "Here we are"},
            {116275252, 7, "Desperation"},
            {112023345, 10, "Ironworks"}
        };
    }

    else if (masteryBadgeID == "robot_mastery_badge") {
        return {
            {129995496, 6,  "Violet Blue"},
            {122968809, 11, "SAFESPACES"},
            {122758067, 7,  "Bajrang Gun"},
            {116159577, 6,  "The Astronaut"},
            {115245249, 6, "robobot"},
            {115198181, 10, "Love we Found"}
        };
    }

    return {};
}

class MasteryInfoPanel : public CCNode {
    std::vector<MasteryLevelDef> m_currentLevels;
    std::string m_masteryName;
    std::string m_masteryID;
    int m_currentIndex = 0;

public:
    static MasteryInfoPanel* create(float width, float height) {
        auto ret = new MasteryInfoPanel();
        if (ret && ret->init(width, height)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    bool init(float width, float height) {
        if (!CCNode::init()) return false;

        this->ignoreAnchorPointForPosition(false);
        this->setAnchorPoint({ 0.5f, 0.5f });
        this->setContentSize({ width, height });

        auto bg = cocos2d::extension::CCScale9Sprite::create("square02b_001.png");
        bg->setColor({ 0, 0, 0 });
        bg->setOpacity(100);
        bg->setContentSize({ width, height });
        bg->setPosition(width / 2, height / 2);
        this->addChild(bg);

        auto label = CCLabelBMFont::create(
            "Select a Mastery",
            "goldFont.fnt"
        );
        label->setTag(99);
        label->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
        label->setPosition(width / 2, height / 2);
        label->setScale(0.5f);
        this->addChild(label);

        return true;
    }

    void onPlayLevel(CCObject* sender) {
        int levelID = sender->getTag();
        if (levelID == 0) return;

        auto searchObj = GJSearchObject::create(
            SearchType::Search,
            std::to_string(levelID)
        );

        auto browserLayer = LevelBrowserLayer::create(searchObj);
        auto scene = CCScene::create();
        scene->addChild(browserLayer);

        CCDirector::sharedDirector()->replaceScene(
            CCTransitionFade::create(0.5f, scene)
        );
    }

    void onPinLevel(CCObject*) {
        if (m_currentLevels.empty()) return;

        int currentLevelID = m_currentLevels[m_currentIndex].levelID;
        int pinnedID = g_streakData.getPinnedLevel(m_masteryID);

        if (pinnedID == currentLevelID) {
            g_streakData.setPinnedLevel(m_masteryID, 0);
            Notification::create(
                "Level Unpinned",
                NotificationIcon::None
            )->show();
        }
        else {
            g_streakData.setPinnedLevel(m_masteryID, currentLevelID);
            Notification::create(
                "Level Pinned!",
                NotificationIcon::Success
            )->show();
        }

        g_streakData.save();
        refreshDisplay();
    }

    void onPrevLevel(CCObject*) {
        if (m_currentLevels.empty()) return;
        m_currentIndex--;
        if (m_currentIndex < 0) {
            m_currentIndex = m_currentLevels.size() - 1;
        }
        refreshDisplay();
    }

    void onNextLevel(CCObject*) {
        if (m_currentLevels.empty()) return;
        m_currentIndex++;
        if (m_currentIndex >= m_currentLevels.size()) {
            m_currentIndex = 0;
        }
        refreshDisplay();
    }

    void updateInfo(const std::string& masteryID, const std::string& masteryName) {
        m_masteryName = masteryName;
        m_masteryID = masteryID;
        m_currentLevels = getLevelsForMastery(masteryID);
        m_currentIndex = 0;

        int pinnedID = g_streakData.getPinnedLevel(masteryID);
        bool foundPin = false;

        if (pinnedID > 0) {
            for (int i = 0; i < m_currentLevels.size(); i++) {
                if (m_currentLevels[i].levelID == pinnedID) {
                    m_currentIndex = i;
                    foundPin = true;
                    break;
                }
            }
        }

        if (!foundPin) {
            for (int i = 0; i < m_currentLevels.size(); i++) {
                auto lvl = GameLevelManager::sharedState()->getSavedLevel(
                    m_currentLevels[i].levelID
                );
                if (!lvl || lvl->m_normalPercent < 100) {
                    m_currentIndex = i;
                    break;
                }
            }
        }

        refreshDisplay();
    }

    void refreshDisplay() {
        this->removeAllChildren();
        float w = getContentSize().width;
        float h = getContentSize().height;

        auto bg = cocos2d::extension::CCScale9Sprite::create("square02b_001.png");
        bg->setColor({ 0, 0, 0 });
        bg->setOpacity(120);
        bg->setContentSize({ w, h });
        bg->setPosition(w / 2, h / 2);
        this->addChild(bg);

        auto title = CCLabelBMFont::create(
            m_masteryName.c_str(),
            "goldFont.fnt"
        );
        title->setScale(0.55f);
        title->setPosition({ w / 2, h + 22.f });
        this->addChild(title);

        if (m_currentLevels.empty()) return;

        auto targetLevel = m_currentLevels[m_currentIndex];
        auto lvlData = GameLevelManager::sharedState()->getSavedLevel(
            targetLevel.levelID
        );
        int percent = lvlData ? lvlData->m_normalPercent : 0;

        std::string lvlName = targetLevel.name;
        if (lvlData && std::string(lvlData->m_levelName).length() > 0) {
            lvlName = lvlData->m_levelName;
        }

        auto nameLbl = CCLabelBMFont::create(
            lvlName.c_str(),
            "bigFont.fnt"
        );
        nameLbl->setAnchorPoint({ 0.5f, 0.5f });
        nameLbl->setPosition({ w / 2, h - 18.f });
        nameLbl->setScale(0.35f);
        nameLbl->limitLabelWidth(w - 70.f, 0.35f, 0.1f);
        this->addChild(nameLbl);

        int pinnedID = g_streakData.getPinnedLevel(m_masteryID);
        bool isPinned = (pinnedID == targetLevel.levelID);

        auto pinSprite = CCSprite::createWithSpriteFrameName("GJ_starsIcon_001.png");
        if (!isPinned) {
            pinSprite->setColor({ 150, 150, 150 });
            pinSprite->setOpacity(100);
            pinSprite->setScale(0.7f);
        }
        else {
            pinSprite->setColor({ 255, 255, 255 });
            pinSprite->setOpacity(255);
            pinSprite->setScale(0.8f);
        }

        auto pinBtn = CCMenuItemSpriteExtra::create(
            pinSprite,
            this,
            menu_selector(MasteryInfoPanel::onPinLevel)
        );
        auto pinMenu = CCMenu::create();
        pinMenu->setPosition({ w - 165.f, 13.f });
        pinMenu->addChild(pinBtn);
        this->addChild(pinMenu);

        std::string diffSprite = "difficulty_00_btn_001.png";
        switch (targetLevel.difficulty) {
        case 2: diffSprite = "difficulty_auto_btn_001.png"; break;
        case 3: diffSprite = "difficulty_01_btn_001.png"; break;
        case 4: diffSprite = "difficulty_03_btn_001.png"; break;
        case 5: diffSprite = "difficulty_04_btn_001.png"; break;
        case 6: diffSprite = "difficulty_04_btn_001.png"; break;
        case 7: diffSprite = "difficulty_05_btn_001.png"; break;
        case 10: diffSprite = "difficulty_07_btn_001.png"; break;
        case 11: diffSprite = "difficulty_08_btn_001.png"; break;
        case 12: diffSprite = "difficulty_06_btn_001.png"; break;
        case 13: diffSprite = "difficulty_09_btn_001.png"; break;
        case 14: diffSprite = "difficulty_10_btn_001.png"; break;
        default: diffSprite = "difficulty_01_btn_001.png"; break;
        }

        auto diffIcon = CCSprite::createWithSpriteFrameName(diffSprite.c_str());
        if (!diffIcon) {
            diffIcon = CCSprite::createWithSpriteFrameName("difficulty_00_btn_001.png");
        }
        diffIcon->setPosition({ 28.f, h / 2 + 5.f });
        diffIcon->setScale(0.85f);
        this->addChild(diffIcon);

        float barW = 85.f;
        float barH = 8.f;
        float barX = (w / 2) - (barW / 2);
        float barY = h / 2 - 2.f;

        auto border = CCLayerColor::create({ 255, 255, 255, 80 });
        border->setContentSize({ barW + 2, barH + 2 });
        border->setPosition({ barX - 1, barY - 1 });
        this->addChild(border);

        auto barBg = CCLayerColor::create({ 0, 0, 0, 200 });
        barBg->setContentSize({ barW, barH });
        barBg->setPosition({ barX, barY });
        this->addChild(barBg);

        if (percent > 0) {
            float fillW = barW * (percent / 100.f);
            auto fill = CCLayerGradient::create(
                { 0, 255, 0, 255 },
                { 0, 200, 0, 255 }
            );
            fill->setContentSize({ fillW, barH });
            fill->setPosition({ barX, barY });
            this->addChild(fill);
        }

        auto perLbl = CCLabelBMFont::create(
            fmt::format("{}%", percent).c_str(),
            "bigFont.fnt"
        );
        perLbl->setScale(0.3f);
        perLbl->setPosition({ barX + barW / 2, barY + barH / 2 + 1.f });
        perLbl->setZOrder(5);
        this->addChild(perLbl);

        auto playSpr = CCSprite::createWithSpriteFrameName("GJ_playBtn2_001.png");
        playSpr->setScale(0.35f);
        auto playBtn = CCMenuItemSpriteExtra::create(
            playSpr,
            this,
            menu_selector(MasteryInfoPanel::onPlayLevel)
        );
        playBtn->setTag(targetLevel.levelID);

        float controlsY = 14.f;
        auto prevSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png");
        prevSpr->setFlipX(false);
        prevSpr->setScale(0.5f);
        auto prevBtn = CCMenuItemSpriteExtra::create(
            prevSpr,
            this,
            menu_selector(MasteryInfoPanel::onPrevLevel)
        );

        auto nextSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png");
        nextSpr->setFlipX(true);
        nextSpr->setScale(0.5f);
        auto nextBtn = CCMenuItemSpriteExtra::create(
            nextSpr,
            this,
            menu_selector(MasteryInfoPanel::onNextLevel)
        );

        auto counterStr = fmt::format(
            "{}/{}",
            m_currentIndex + 1,
            m_currentLevels.size()
        );
        auto counter = CCLabelBMFont::create(counterStr.c_str(), "bigFont.fnt");
        counter->setScale(0.4f);
        counter->setPosition({ w / 2, controlsY });
        counter->setColor({ 0, 255, 255 });
        this->addChild(counter);

        auto menu = CCMenu::create();
        menu->setPosition(0, 0);
        playBtn->setPosition({ w - 28.f, h / 2 + 5.f });
        prevBtn->setPosition({ w / 2 - 35.f, controlsY });
        nextBtn->setPosition({ w / 2 + 35.f, controlsY });
        menu->addChild(playBtn);
        menu->addChild(prevBtn);
        menu->addChild(nextBtn);
        this->addChild(menu);
    }
};

class MasteryCell : public CCNode {
    DisplayItem m_item;
    CCNode* m_progressNode = nullptr;

public:
    static MasteryCell* create(const DisplayItem& item, float width, SEL_MenuHandler callback, CCObject* target) {
        auto ret = new MasteryCell();
        if (ret && ret->init(item, width, callback, target)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    bool init(const DisplayItem& item, float width, SEL_MenuHandler callback, CCObject* target) {
        if (!CCNode::init()) return false;
        m_item = item;
        float height = 55.0f;
        this->setContentSize({ width, height });

        auto bg = cocos2d::extension::CCScale9Sprite::create("square02b_001.png");
        bg->setColor({ 0, 0, 0 });
        bg->setOpacity(80);
        bg->setContentSize({ width, height - 2 });
        bg->setPosition(width / 2, height / 2);
        this->addChild(bg);

        auto icon = CCSprite::create(item.spriteName.c_str());
        if (!icon) {
            icon = CCSprite::createWithSpriteFrameName("GJ_questionMark_001.png");
        }

        float scale = (icon->getContentSize().height > 85.f)
            ? 65.f / icon->getContentSize().height
            : 4.0f;

        icon->setScale(scale);
        icon->setPosition({ 25.f, height / 2 + 1.f });
        if (!item.isUnlocked) {
            icon->setColor({ 100, 100, 100 });
        }
        this->addChild(icon);

        auto nameLabel = CCLabelBMFont::create(
            item.name.c_str(),
            "bigFont.fnt"
        );
        nameLabel->setAnchorPoint({ 0.f, 0.5f });
        nameLabel->setPosition({ 50.f, height - 15.f });
        nameLabel->setScale(0.35f);
        if (item.isUnlocked) {
            nameLabel->setColor({ 255, 215, 0 });
        }
        this->addChild(nameLabel);

        m_progressNode = CCNode::create();
        this->addChild(m_progressNode);
        updateProgressState(width, height);

        auto arrowSpr = CCSprite::createWithSpriteFrameName("edit_rightBtn_001.png");
        arrowSpr->setScale(0.7f);
        auto btn = CCMenuItemSpriteExtra::create(
            arrowSpr,
            target,
            callback
        );
        btn->setUserObject(CCString::create(item.id));
        auto menu = CCMenu::create();
        menu->setPosition({ width - 20.f, height / 2 });
        menu->addChild(btn);
        this->addChild(menu);

        return true;
    }

    void onClaim(CCObject*) {
        if (m_item.id.empty()) return;
        g_streakData.unlockBadge(m_item.id);
        g_streakData.save();
        BadgeNotification::show(m_item.id);
        m_item.isUnlocked = true;
        updateProgressState(getContentSize().width, getContentSize().height);
    }

    void updateProgressState(float width, float height) {
        m_progressNode->removeAllChildren();
        float totalProgress = 0.f;
        bool allLevelsCompleted = true;

        if (m_item.isMastery) {
            auto levels = getLevelsForMastery(m_item.id);
            float sumPercent = 0.f;
            for (const auto& lvl : levels) {
                auto gdLvl = GameLevelManager::sharedState()->getSavedLevel(lvl.levelID);
                if (gdLvl) sumPercent += gdLvl->m_normalPercent;
                if (!gdLvl || gdLvl->m_normalPercent < 100) allLevelsCompleted = false;
            }
            if (!levels.empty()) {
                totalProgress = sumPercent / (float)levels.size();
            }
        }

        float barW = width - 90.f;
        float barH = 8.f;
        float barX = 50.f;
        float barY = 12.f;

        if (allLevelsCompleted && !m_item.isUnlocked) {
            auto claimSpr = ButtonSprite::create(
                "Claim", 0, 0,
                "goldFont.fnt",
                "GJ_button_01.png",
                0, 0.5f
            );

            auto claimBtn = CCMenuItemSpriteExtra::create(
                claimSpr,
                this,
                menu_selector(MasteryCell::onClaim)
            );
            claimBtn->setScale(0.8f);

            auto scaleUp = CCScaleTo::create(0.5f, 0.85f);
            auto scaleDown = CCScaleTo::create(0.5f, 0.8f);
            auto seq = CCSequence::create(scaleUp, scaleDown, nullptr);
            auto repeat = CCRepeatForever::create(seq);
            claimBtn->runAction(repeat);

            auto menu = CCMenu::create();
            menu->setPosition({ barX + barW / 2, barY + 2.f });
            menu->addChild(claimBtn);
            m_progressNode->addChild(menu);
        }
        else {
            auto border = CCLayerColor::create({ 255, 255, 255, 80 });
            border->setContentSize({ barW + 2, barH + 2 });
            border->setPosition({ barX - 1, barY - 1 });
            m_progressNode->addChild(border);

            auto barTrack = CCLayerColor::create({ 40, 40, 40, 255 }, barW, barH);
            barTrack->setPosition({ barX, barY });
            m_progressNode->addChild(barTrack);

            if (totalProgress > 0) {
                auto barFill = CCLayerGradient::create(
                    { 0, 255, 0, 255 },
                    { 0, 200, 0, 255 }
                );
                barFill->setContentSize({ barW * (totalProgress / 100.f), barH });
                barFill->setPosition({ barX, barY });
                m_progressNode->addChild(barFill);
            }

            auto numLbl = CCLabelBMFont::create(
                fmt::format("{}%", (int)totalProgress).c_str(),
                "bigFont.fnt"
            );
            numLbl->setScale(0.3f);
            numLbl->setPosition({ barX + barW / 2, barY + barH / 2 + 1.f });
            numLbl->setZOrder(5);
            m_progressNode->addChild(numLbl);
        }
    }
};

class StreakGoalCell : public CCNode {
public:
    static StreakGoalCell* create(const DisplayItem& item, float width) {
        auto ret = new StreakGoalCell();
        if (ret && ret->init(item, width)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    bool init(const DisplayItem& item, float width) {
        if (!CCNode::init()) return false;
        float height = 45.0f;
        this->setContentSize({ width, height });

        auto bg = cocos2d::extension::CCScale9Sprite::create("square02b_001.png");
        bg->setColor({ 0, 0, 0 });
        bg->setOpacity(80);
        bg->setContentSize({ width, height - 2 });
        bg->setPosition(width / 2, height / 2);
        this->addChild(bg);

        auto icon = CCSprite::create(item.spriteName.c_str());
        if (!icon) {
            icon = CCSprite::createWithSpriteFrameName("GJ_questionMark_001.png");
        }

        float scale = (icon->getContentSize().height > 30.f)
            ? 30.f / icon->getContentSize().height
            : 1.0f;
        icon->setScale(scale);
        icon->setPosition({ width - 25.f, height / 2 });
        this->addChild(icon);

        int current = g_streakData.currentStreak;
        int target = item.daysRequired;
        if (item.isUnlocked) {
            current = target;
        }

        float pct = (target > 0) ? (float)current / (float)target : 0.f;
        if (pct > 1.f) pct = 1.f;

        float barW = width - 55.f;
        float barH = 10.f;
        float barX = 10.f;
        float barY = 10.f;

        auto border = CCLayerColor::create({ 255, 255, 255, 80 });
        border->setContentSize({ barW + 2, barH + 2 });
        border->setPosition({ barX - 1, barY - 1 });
        this->addChild(border);

        auto track = CCLayerColor::create({ 40, 40, 40, 255 }, barW, barH);
        track->setPosition({ barX, barY });
        this->addChild(track);

        if (pct > 0) {
            auto bar = CCLayerGradient::create(
                { 0, 255, 255, 255 },
                { 0, 100, 200, 255 }
            );
            bar->setContentSize({ barW * pct, barH });
            bar->setPosition({ barX, barY });
            this->addChild(bar);
        }

        auto title = CCLabelBMFont::create(
            fmt::format("{} Days", target).c_str(),
            "goldFont.fnt"
        );
        title->setScale(0.35f);
        title->setAnchorPoint({ 0.f, 0.5f });
        title->setPosition({ 10.f, height - 12.f });
        this->addChild(title);

        auto prog = CCLabelBMFont::create(
            fmt::format("{}/{}", std::min(current, target), target).c_str(),
            "bigFont.fnt"
        );
        prog->setScale(0.35f);
        prog->setPosition({ 10.f + barW / 2, 15.f });
        this->addChild(prog);

        return true;
    }
};

class DayProgressPopup : public Popup {
    MasteryInfoPanel* m_infoPanel = nullptr;
    std::map<std::string, std::string> m_masteryNames;

protected:
    void onSelectMastery(CCObject* sender) {
        auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
        if (!btn) return;

        auto idStr = static_cast<CCString*>(btn->getUserObject());
        if (!idStr) return;

        std::string mID = idStr->getCString();
        std::string mName = m_masteryNames[mID];

        if (m_infoPanel) {
            m_infoPanel->updateInfo(mID, mName);
        }
    }

    bool init() override {
        if (!Popup::init(440.f, 290.f, "geode.loader/GE_square03.png")) return false;

        this->setTitle("Collection");
        auto winSize = m_mainLayer->getContentSize();
        g_streakData.load();

        float listWidth = 190.f;
        float gap = 20.f;
        float bottomMargin = 25.f;

        float leftColumnX = (winSize.width / 2) - (listWidth / 2) - (gap / 2);
        float rightColumnX = (winSize.width / 2) + (listWidth / 2) + (gap / 2);

        float boxHeight = 75.f;
        float rightListHeight = 95.f;
        float verticalGap = 30.f;

        float leftListHeight = boxHeight + verticalGap + rightListHeight;

        auto lblLeft = CCLabelBMFont::create("Masteries", "goldFont.fnt");
        lblLeft->setScale(0.6f);
        lblLeft->setPosition({ leftColumnX, winSize.height - 42.f });
        m_mainLayer->addChild(lblLeft);

        std::vector<DisplayItem> masteryList;
        std::vector<std::pair<std::string, std::string>> masteryDefs = {
            {"Cube Mastery", "cube_mastery_badge"},
            {"Ship Mastery", "ship_mastery_badge"},
            {"Ball Mastery", "ball_mastery_badge"},
            {"UFO Mastery", "ufo_mastery_badge"},
            {"Wave Mastery", "wave_mastery_badge"},
            {"Spider Mastery", "spider_mastery_badge"},
            {"Swing Copter Mastery", "swingcopter_mastery_badge"},
            {"Robot Mastery", "robot_mastery_badge"},
            {"Dual Mastery", "dual_mastery_badge"},
            {"XL Mastery", "xl_mastery_badge"},
            {"Memory Mastery", "memory_mastery_badge"}
        };

        for (const auto& pair : masteryDefs) {
            m_masteryNames[pair.second] = pair.first;
            DisplayItem item;
            item.name = pair.first;
            item.id = pair.second;
            item.isMastery = true;

            auto* info = g_streakData.getBadgeInfo(item.id);
            item.spriteName = info ? info->spriteName : "GJ_questionMark_001.png";
            item.isUnlocked = g_streakData.isBadgeUnlocked(item.id);
            masteryList.push_back(item);
        }

        auto scrollLeft = ScrollLayer::create({ listWidth, leftListHeight });
        float cellH_Left = 55.f;
        float contentH_Left = std::max(
            leftListHeight,
            (float)masteryList.size() * cellH_Left
        );
        scrollLeft->m_contentLayer->setContentSize({ listWidth, contentH_Left });

        int i = 0;
        for (const auto& it : masteryList) {
            auto cell = MasteryCell::create(
                it,
                listWidth,
                menu_selector(DayProgressPopup::onSelectMastery),
                this
            );
            float y = contentH_Left - ((i + 1) * cellH_Left);
            cell->setPosition({ 0, y });
            scrollLeft->m_contentLayer->addChild(cell);
            i++;
        }
        scrollLeft->m_contentLayer->setPositionY(leftListHeight - contentH_Left);
        scrollLeft->setPosition({ leftColumnX - listWidth / 2, bottomMargin });

        auto bgLeft = cocos2d::extension::CCScale9Sprite::create("square02b_001.png");
        bgLeft->setColor({ 0,0,0 });
        bgLeft->setContentSize({ listWidth + 6, leftListHeight + 6 });
        bgLeft->setOpacity(50);
        bgLeft->setPosition(
            scrollLeft->getPosition() + (scrollLeft->getContentSize() / 2)
        );
        m_mainLayer->addChild(bgLeft);
        m_mainLayer->addChild(scrollLeft);

        float panelCenterY = bottomMargin + leftListHeight - (boxHeight / 2);

        m_infoPanel = MasteryInfoPanel::create(listWidth + 2.f, boxHeight);
        m_infoPanel->setPosition({ rightColumnX, panelCenterY });

        m_mainLayer->addChild(m_infoPanel);
        if (!masteryList.empty()) {
            m_infoPanel->updateInfo(masteryList[0].id, masteryList[0].name);
        }

        auto lblRight = CCLabelBMFont::create("Streaks", "goldFont.fnt");
        lblRight->setScale(0.6f);
        float gapCenterY = bottomMargin + rightListHeight + (verticalGap / 2);
        lblRight->setPosition({ rightColumnX, gapCenterY });
        m_mainLayer->addChild(lblRight);

        std::vector<DisplayItem> streakList;
        for (const auto& badge : g_streakData.badges) {
            if (badge.daysRequired > 0 && !badge.isFromRoulette) {
                DisplayItem item;
                item.id = badge.badgeID;
                item.daysRequired = badge.daysRequired;
                item.spriteName = badge.spriteName;
                item.isUnlocked = g_streakData.isBadgeUnlocked(badge.badgeID);
                streakList.push_back(item);
            }
        }

        std::sort(
            streakList.begin(),
            streakList.end(),
            [](const DisplayItem& a, const DisplayItem& b) {
                return a.daysRequired < b.daysRequired;
            }
        );

        auto scrollRight = ScrollLayer::create({ listWidth, rightListHeight });
        float cellH_Right = 45.f;
        float contentH_Right = std::max(
            rightListHeight,
            (float)streakList.size() * cellH_Right
        );
        scrollRight->m_contentLayer->setContentSize({ listWidth, contentH_Right });

        int j = 0;
        for (const auto& it : streakList) {
            auto cell = StreakGoalCell::create(it, listWidth);
            float y = contentH_Right - ((j + 1) * cellH_Right);
            cell->setPosition({ 0, y });
            scrollRight->m_contentLayer->addChild(cell);
            j++;
        }
        scrollRight->m_contentLayer->setPositionY(rightListHeight - contentH_Right);
        scrollRight->setPosition({ rightColumnX - listWidth / 2, bottomMargin });

        auto bgRight = cocos2d::extension::CCScale9Sprite::create("square02b_001.png");
        bgRight->setColor({ 0,0,0 });
        bgRight->setContentSize({ listWidth + 6, rightListHeight + 6 });
        bgRight->setOpacity(50);
        bgRight->setPosition(
            scrollRight->getPosition() + (scrollRight->getContentSize() / 2)
        );
        m_mainLayer->addChild(bgRight);
        m_mainLayer->addChild(scrollRight);

        auto line = CCLayerColor::create({ 255, 255, 255, 80 }, 2.f, 200.f);
        line->setPosition({ winSize.width / 2 - 1.f, bottomMargin });
        m_mainLayer->addChild(line);

        return true;
    }

public:
    static DayProgressPopup* create() {
        auto ret = new DayProgressPopup();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};