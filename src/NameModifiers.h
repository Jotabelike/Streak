#pragma once
#include <Geode/utils/cocos.hpp>

using namespace geode::prelude;

namespace NameModifiers {
    void applyColor(CCLabelBMFont* label, const std::string& colorID);
    void applyFont(CCLabelBMFont* label, const std::string& fontID);
    void applyEffect(CCLabelBMFont* label, const std::string& effectID);
    void applyAnimation(CCLabelBMFont* label, const std::string& animID);
}