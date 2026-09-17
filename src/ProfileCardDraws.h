#pragma once

#include <Geode/loader/Mod.hpp>
#include <array>
#include <string>

namespace ProfileCardDraws {
    enum class DrawType {
        Masked,
        Overlay
    };

    struct DrawInfo {
        const char* id;
        const char* name;
        const char* sprite;
        DrawType type;
        float displayHeight;
        float centerX;
        float centerY;
    };

    inline constexpr std::size_t MAX_EQUIPPED = 5;

    // Placement values use the profile card's 280x180 design space. Keeping
    // one uniform scale preserves the original artwork proportions.
    inline constexpr std::array<DrawInfo, 5> DRAWS = {{
        { "dr1", "Draw 1", "dr1.png", DrawType::Masked, 180.f, 140.f, 90.f },
        { "dr2", "Draw 2", "dr2.png", DrawType::Masked, 130.f, 140.f, 65.f },
        { "dr3", "Draw 3", "dr3.png", DrawType::Masked, 180.f, 140.f, 90.f },
        { "dr4", "Draw 4", "dr4.png", DrawType::Masked, 213.f, 140.f, 106.5f },
        { "dr5", "Draw 5", "dr5.png", DrawType::Masked, 180.f, 217.f, 90.f }
    }};

    inline const DrawInfo* getInfo(const std::string& id) {
        for (auto const& draw : DRAWS) {
            if (id == draw.id) return &draw;
        }
        return nullptr;
    }

    inline std::string resolveSprite(const DrawInfo& draw) {
        return geode::Mod::get()->expandSpriteName(draw.sprite);
    }
}
