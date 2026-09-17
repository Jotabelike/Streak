#pragma once

#include <array>
#include <string>

namespace ProfileCardStyles {
    struct StyleInfo {
        const char* id;
        const char* name;
    };

    inline constexpr const char* DEFAULT_STYLE = "geode.loader/GE_square01.png";

    // Every popup-style nine-slice shipped with Geometry Dash, plus the
    // existing Geode frame so current profiles keep their original look.
    inline constexpr std::array<StyleInfo, 8> STYLES = {{
        { "geode.loader/GE_square01.png", "Default" },
        { "GJ_square01.png", "GD Square 01" },
        { "GJ_square02.png", "GD Square 02" },
        { "GJ_square03.png", "GD Square 03" },
        { "GJ_square04.png", "GD Square 04" },
        { "GJ_square05.png", "GD Square 05" },
        { "GJ_square06.png", "GD Square 06" },
        { "GJ_squareB_01.png", "GD Square B" }
    }};

    inline const StyleInfo* getInfo(const std::string& id) {
        for (auto const& style : STYLES) {
            if (id == style.id) return &style;
        }
        return nullptr;
    }

    inline const char* resolve(const std::string& id) {
        return getInfo(id) ? getInfo(id)->id : DEFAULT_STYLE;
    }
}
