#include <Geode/Geode.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>

using namespace geode::prelude;

 int getStreakPointsForLevel(int stars) {
    if (stars == 0) return 0;
    if (stars <= 3) return 1;
    if (stars <= 5) return 3;
    if (stars <= 7) return 4;
    if (stars <= 9) return 5;
    return 6;
}

 
 class $modify(StreakLevelInfo, LevelInfoLayer) {
     bool init(GJGameLevel * level, bool challenge) {
         if (!LevelInfoLayer::init(level, challenge)) return false;
         bool showPoints = Mod::get()->getSavedValue<bool>("enable_level_points", true);
         if (!showPoints) return true;
         int points = getStreakPointsForLevel(level->m_stars);
         if (points <= 0) return true;

         auto starsLabel = this->getChildByID("stars-label");
         auto starsIcon = this->getChildByID("stars-icon");

         if (!starsLabel && starsIcon) starsLabel = static_cast<CCNode*>(starsIcon);
         if (!starsLabel) return true;
         float shiftAmount = -12.f;

         starsLabel->setPositionX(starsLabel->getPositionX() + shiftAmount);
         if (starsIcon) {
             starsIcon->setPositionX(starsIcon->getPositionX() + shiftAmount);
         }

         CCPoint refPos = starsLabel->getPosition();
         float contentWidth = starsLabel->getScaledContentSize().width;
         float xPosition = refPos.x + contentWidth + 3.f;
         auto container = CCNode::create();
         container->setID("streak-points-display");
         container->setPosition({ xPosition, refPos.y });
         auto label = CCLabelBMFont::create(
             fmt::format("{}", points).c_str(),
             "bigFont.fnt"
         );
         label->setScale(0.45f);
         label->setAnchorPoint({ 0.0f, 0.5f });
         label->setPosition({ 0.f, 0.f });
         container->addChild(label);
         auto icon = CCSprite::create("streak_point.png"_spr);
         if (!icon) icon = CCSprite::createWithSpriteFrameName("GJ_starsIcon_001.png");
         icon->setScale(0.08f);
         float labelWidth = label->getScaledContentSize().width;
         icon->setPosition({ labelWidth + 7.f, 0.f });
         container->addChild(icon);
         this->addChild(container);

         return true;
     }
 };