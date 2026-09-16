# Podium column

Asset: `resources/extra/podium_column.png` (1024 × 1536, RGBA).
Generated with the built-in imagegen tool. The original alpha channel is preserved.
The layer reuses the sprite for the second, first and third places at the same
uniform scale, with different vertical positions and silver, gold and bronze tints.
The long shafts are clipped by the viewport. The podiums rise from below the screen
after the scene transition. Player names and scores are drawn by Geode.

## Player cubes

The layer draws cubes with Geode's `SimplePlayer`, using the current local
equipment for the signed-in player and public GD profiles for other players.
Requests have independent lifetimes, a 15-second timeout and a five-minute
in-memory cache. A dim cube with `...` indicates loading; `?` indicates missing
profile data. Equipped Streak badges remain beside the cubes. Each column shows
only the streak-day count directly below its cube, without a panel; that label
uses the player's equipped name-color style. A large rank medal sits below the
day count, followed by the player's equipped Streak badge.

Protocol references: [public profile endpoint](https://gddocs.omgrod.me/endpoints/users/getgjuserinfo20/)
and [user fields](https://boomlings.dev/resources/server/user).

## Generation prompt

Use case: stylized-concept. Asset type: transparent PNG game sprite for a Geometry Dash leaderboard podium. Generate ONE complete isolated classical Greek/Roman white marble column pedestal, front-facing, symmetrical, orthographic with slight view of the top surface. A wide thick circular capital with a flat elliptical top, several sculpted ring moldings beneath it, a straight vertically fluted cylindrical shaft, and a restrained molded circular foot. Like an elegant Doric column used as a winners podium. Clean polished stylized 3D game art, soft grayscale ivory shading, clear edges and readable deep flutes at small sizes, light from upper left. Tall narrow object approximately 1:2.4 width to height, fully visible centered, little empty padding, transparent background with actual alpha. Single white marble column ONLY. No scene, ground, cast shadow outside object, text, numbers, laurels, players, watermark or additional objects. This sprite will be reused at three different heights and tinted slightly in engine.
