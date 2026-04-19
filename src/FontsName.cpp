#include "NameModifiers.h"

namespace NameModifiers {
    void applyFont(CCLabelBMFont* label, const std::string& fontID) {
        if (!label) return;

        std::string currentText = label->getString();

     
        if (fontID == "Pusab")            label->setFntFile("bigFont.fnt");
        else if (fontID == "Gold")        label->setFntFile("goldFont.fnt");
        else if (fontID == "Chat")        label->setFntFile("chatFont.fnt");
        else if (fontID == "Font1")       label->setFntFile("gjFont01.fnt");
        else if (fontID == "Font2")       label->setFntFile("gjFont02.fnt");
        else if (fontID == "Font3")       label->setFntFile("gjFont03.fnt");
        else if (fontID == "Font4")       label->setFntFile("gjFont04.fnt");
        else if (fontID == "Font5")       label->setFntFile("gjFont05.fnt");
        else if (fontID == "Font6")       label->setFntFile("gjFont06.fnt");
        else if (fontID == "Font7")       label->setFntFile("gjFont07.fnt");
        else if (fontID == "Font8")       label->setFntFile("gjFont08.fnt");
        else if (fontID == "Font9")       label->setFntFile("gjFont09.fnt");
        else if (fontID == "Font10")      label->setFntFile("gjFont10.fnt");
        else if (fontID == "Font11")      label->setFntFile("gjFont11.fnt");
        else if (fontID == "Font12")      label->setFntFile("gjFont12.fnt");
        else if (fontID == "Font13")      label->setFntFile("gjFont13.fnt");
        else if (fontID == "Font14")      label->setFntFile("gjFont14.fnt");
        else if (fontID == "Font15")      label->setFntFile("gjFont15.fnt");
        else if (fontID == "Font16")      label->setFntFile("gjFont16.fnt");
        else if (fontID == "Font17")      label->setFntFile("gjFont17.fnt");
        else if (fontID == "Font18")      label->setFntFile("gjFont18.fnt");
        else if (fontID == "Font19")      label->setFntFile("gjFont19.fnt");
        else if (fontID == "Font20")      label->setFntFile("gjFont20.fnt");
        else if (fontID == "Font21")      label->setFntFile("gjFont21.fnt");
        else if (fontID == "Font22")      label->setFntFile("gjFont22.fnt");
        else if (fontID == "Font23")      label->setFntFile("gjFont23.fnt");
        else if (fontID == "Font24")      label->setFntFile("gjFont24.fnt");
        else if (fontID == "Font25")      label->setFntFile("gjFont25.fnt");
        else if (fontID == "Font26")      label->setFntFile("gjFont26.fnt");
        else if (fontID == "Font27")      label->setFntFile("gjFont27.fnt");
        else if (fontID == "Font28")      label->setFntFile("gjFont28.fnt");
        else if (fontID == "Font29")      label->setFntFile("gjFont29.fnt");
        else if (fontID == "Font30")      label->setFntFile("gjFont30.fnt");
        else if (fontID == "Font31")      label->setFntFile("gjFont31.fnt");
        else if (fontID == "Font32")      label->setFntFile("gjFont32.fnt");
        else if (fontID == "Font33")      label->setFntFile("gjFont33.fnt");
        else if (fontID == "Font34")      label->setFntFile("gjFont34.fnt");
        else if (fontID == "Font35")      label->setFntFile("gjFont35.fnt");
        else if (fontID == "Font36")      label->setFntFile("gjFont36.fnt");
        else if (fontID == "Font37")      label->setFntFile("gjFont37.fnt");
        else if (fontID == "Font38")      label->setFntFile("gjFont38.fnt");
        else if (fontID == "Font39")      label->setFntFile("gjFont39.fnt");
        else if (fontID == "Font40")      label->setFntFile("gjFont40.fnt");
        else if (fontID == "Font41")      label->setFntFile("gjFont41.fnt");
        else if (fontID == "Font42")      label->setFntFile("gjFont42.fnt");
        else if (fontID == "Font43")      label->setFntFile("gjFont43.fnt");
        else if (fontID == "Font44")      label->setFntFile("gjFont44.fnt");
        else if (fontID == "Font45")      label->setFntFile("gjFont45.fnt");
        else if (fontID == "Font46")      label->setFntFile("gjFont46.fnt");
        else if (fontID == "Font47")      label->setFntFile("gjFont47.fnt");
        else if (fontID == "Font48")      label->setFntFile("gjFont48.fnt");
        else if (fontID == "Font49")      label->setFntFile("gjFont49.fnt");
        else if (fontID == "Font50")      label->setFntFile("gjFont50.fnt");
        else if (fontID == "Font51")      label->setFntFile("gjFont51.fnt");
        else if (fontID == "Font52")      label->setFntFile("gjFont52.fnt");
        else if (fontID == "Font53")      label->setFntFile("gjFont53.fnt");
        else if (fontID == "Font54")      label->setFntFile("gjFont54.fnt");
        else if (fontID == "Font55")      label->setFntFile("gjFont55.fnt");
        else if (fontID == "Font56")      label->setFntFile("gjFont56.fnt");
        else if (fontID == "Font57")      label->setFntFile("gjFont57.fnt");
        else if (fontID == "Font58")      label->setFntFile("gjFont58.fnt");
        else if (fontID == "Font59")      label->setFntFile("gjFont59.fnt");
        else                              label->setFntFile("bigFont.fnt");

        label->setString(currentText.c_str());
    }
}