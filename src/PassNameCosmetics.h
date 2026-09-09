#pragma once
#include <array>
#include <cstddef>
#include <string>
#include <string_view>

namespace PassNameCosmetics {
    inline constexpr std::array<std::string_view, 10> EFFECTS = {
        "Origami Flight", "Crown Assembly", "Ink Signature", "Chevron Parade", "Equalizer",
        "Flip Tiles", "Morse Signal", "Curtain Call", "Ribbon Weave", "Corner Lock"
    };

    inline constexpr std::array<std::string_view, 10> LEGACY_EFFECTS = {
        "Astral Rings", "Comet Tail", "Crystal Shards", "Dragon Flame", "Eclipse",
        "Lightning Storm", "Moon Dust", "Neon Wings", "Pixel Burst", "Solar Flare"
    };

    inline constexpr std::array<std::string_view, 5> ANIMATIONS = {
        "Accordion", "Pair Swap", "Shutter Cut", "Rail Switch", "Fan Deck"
    };

    inline constexpr std::array<std::string_view, 5> LEGACY_ANIMATIONS = {
        "Levitate", "Pendulum", "Typewriter", "Warp", "Ripple"
    };

    inline constexpr std::array<std::string_view, 10> COLORS = {
        "Chessboard", "Signal Code", "Ink March", "Split Complement", "Thermal Steps",
        "Hologram Stripes", "Sepia Film", "Stained Glass", "Lunar Phases", "Blueprint"
    };

    inline constexpr std::array<std::string_view, 10> LEGACY_COLORS = {
        "Emerald Wave", "Ruby Wave", "Sapphire Wave", "Solar Wave", "Moonlight Wave",
        "Nebula Wave", "Candy Wave", "Inferno Wave", "Glacier Wave", "Void Wave"
    };

    template <std::size_t N>
    inline bool contains(const std::array<std::string_view, N>& values, const std::string& item) {
        for (auto value : values) {
            if (value == item) return true;
        }
        return false;
    }

    inline bool isEffect(const std::string& item) { return contains(EFFECTS, item); }
    inline bool isLegacyEffect(const std::string& item) { return contains(LEGACY_EFFECTS, item); }
    inline bool isAnimation(const std::string& item) { return contains(ANIMATIONS, item); }
    inline bool isLegacyAnimation(const std::string& item) { return contains(LEGACY_ANIMATIONS, item); }
    inline bool isColor(const std::string& item) { return contains(COLORS, item); }
    inline bool isLegacyColor(const std::string& item) { return contains(LEGACY_COLORS, item); }
    inline bool contains(const std::string& item) {
        return isEffect(item) || isAnimation(item) || isColor(item);
    }
}
