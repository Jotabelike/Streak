#include "NameModifiers.h"

namespace NameModifiers {
    void applyFont(CCLabelBMFont* label, const std::string& fontID) {
        if (!label) return;

        std::string currentText = label->getString();

        if (fontID == "Pusab") label->setFntFile("bigFont.fnt");
        else if (fontID == "Gold") label->setFntFile("goldFont.fnt");
        else if (fontID == "Chat") label->setFntFile("chatFont.fnt");
        else if (fontID == "Serigrafia") label->setFntFile("gjFont05.fnt");
        else if (fontID == "Pixel") label->setFntFile("gjFont01.fnt");
        else if (fontID == "Blocky") label->setFntFile("gjFont02.fnt");
        else if (fontID == "Mecano") label->setFntFile("gjFont04.fnt");
        else if (fontID == "Monster") label->setFntFile("gjFont08.fnt");
        else if (fontID == "Tech") label->setFntFile("gjFont10.fnt");
        else {
            label->setFntFile("bigFont.fnt");  
        }

        label->setString(currentText.c_str());
    }
}