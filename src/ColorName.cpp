#include "NameModifiers.h"

namespace NameModifiers {
    void applyColor(CCLabelBMFont* label, const std::string& colorID) {
        if (!label) return;

        label->stopActionByTag(7777);

        if (colorID == "Rainbow") {
            auto tint1 = CCTintTo::create(0.5f, 255, 50, 50);
            auto tint2 = CCTintTo::create(0.5f, 255, 165, 50);
            auto tint3 = CCTintTo::create(0.5f, 255, 255, 50);
            auto tint4 = CCTintTo::create(0.5f, 50, 255, 50);
            auto tint5 = CCTintTo::create(0.5f, 50, 100, 255);
            auto tint6 = CCTintTo::create(0.5f, 150, 50, 255);
            auto seq = CCSequence::create(tint1, tint2, tint3, tint4, tint5, tint6, nullptr);
            auto repeat = CCRepeatForever::create(seq);
            repeat->setTag(7777);
            label->runAction(repeat);
        }
        else if (colorID == "Red") label->setColor({ 255, 50, 50 });
        else if (colorID == "Blue") label->setColor({ 50, 100, 255 });
        else if (colorID == "Green") label->setColor({ 50, 255, 50 });
        else if (colorID == "Yellow") label->setColor({ 255, 255, 50 });
        else if (colorID == "Purple") label->setColor({ 150, 50, 255 });
        else if (colorID == "Orange") label->setColor({ 255, 150, 50 });  
        else if (colorID == "Black") label->setColor({ 0, 0, 0 });
        else if (colorID == "Cyan") label->setColor({ 0, 255, 255 });
        else if (colorID == "Pink") label->setColor({ 255, 105, 180 });
        else if (colorID == "Lime") label->setColor({ 150, 255, 50 });
        else if (colorID == "Magenta") label->setColor({ 255, 0, 255 });
        else {
            label->setColor({ 255, 255, 255 }); 
        }
    }
}