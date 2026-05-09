# Streak!
## 1.10.43 Beta 3
- Removed XOR obfuscation from HMACAuth.h (this was flagged as vivecode in Beta 2)
- All economy transactions (gems, super stars, star tickets, XP) are now processed server-side
- Added server endpoints for: shop purchases, daily shop, roulette spins, streak goal claims, discord milestone claims, level mission claims, task reward claims, and chest rewards
- Fixed a bug where spending currencies (spinning roulettes, buying items, claiming rewards) would reset after restarting the game
- The client no longer modifies currency values locally — all changes come from the server response

### Why not GlobedGD/Argon?
Argon (https://github.com/GlobedGD/argon) is an excellent API for verifying Geometry Dash account ownership. It was designed for multiplayer mods like Globed, where the main concern is identity — making sure Player A can't impersonate Player B.

Streak! has a different security need. This mod manages a virtual economy (gems, super stars, star tickets, XP, levels, badges, banners) where the main concern is transaction integrity — making sure nobody can forge a request to give themselves free currency, duplicate rewards, or tamper with purchase amounts.

Argon solves: "Is this player really who they say they are?"
HMAC solves: "Was this specific request (with this exact body, at this exact time) actually sent by the mod, and has it not been modified in transit?"

Argon tokens verify identity once, but they don't sign individual request bodies. A valid Argon token could be reused to send modified requests (e.g., changing `"gems": 5` to `"gems": 999999` in a purchase request). HMAC signs every request body with a timestamp, so any modification invalidates the signature.

Additionally:
- **GDPS compatibility**: Streak! supports GDPS players. Argon's official server (argon.globed.dev) only works with official GD servers. GDPS support would require self-hosting an Argon server, adding infrastructure complexity for GDPS server owners.
- **No external dependency**: Argon depends on argon.globed.dev being online. If the Argon server goes down, authentication breaks. Streak!'s HMAC system is self-contained between the mod and its own server.
- **Request-level protection**: Every single API call is individually signed with HMAC-SHA256 (account ID + timestamp + request body), making replay attacks and request tampering impossible even with a valid session.

The security model has 5 layers:
1. **HMAC-SHA256 signatures** — every request is signed, preventing forgery and tampering
2. **Timestamp-based anti-replay** — requests expire after 30 seconds
3. **Session tokens** — issued on first load, required for all write operations
4. **Server-side validation** — the server validates all economy transactions and rejects invalid ones
5. **FORBIDDEN_FIELDS** — the client cannot directly set economy values (gems, stars, tickets, XP, level, streak)

### Sources
- **GlobedGD/Argon**: https://github.com/GlobedGD/argon
- **GlobedGD/Argon Server API**: https://github.com/GlobedGD/argon-server/blob/main/docs/server-api.md
- **RFC 2104** — HMAC: Keyed-Hashing for Message Authentication (IETF, 1997): https://datatracker.ietf.org/doc/html/rfc2104
- **RFC 4868** — Using HMAC-SHA-256 with IPsec (IETF, 2007): https://tools.ietf.org/html/rfc4868
- **AWS** — HMAC-SHA Signature for API Authentication: https://docs.aws.amazon.com/AmazonSimpleDB/latest/DeveloperGuide/HMACAuth.html
- **GitGuardian** — HMAC Secrets Explained: https://blog.gitguardian.com/hmac-secrets-explained-authentication/

## 1.10.41
- More settings were added
- The progress bar at the end of the levels returned
- Some errors were fixed

## 1.10.40
- New cosmetics added
- New Banners
- The name personalization interface was revamped
- Now the cosmetics names can be seen in the comments
- New configurations were added
- about.md was updated
- The terms and conditions were updated along with a new interface
- A category for GDPS players was added
- The streak point bar upon completing a level was removed and replaced with RobTop's reward animation.
- The design of the rarities was changed

## 1.10.39
- Added Discord Community Goal Popup: A new interactive menu to track Discord server member milestones and unlock rewards.
- Chest Reward Integration: Added randomized chest drops alongside fixed major rewards (gems, stars, and tickets).

## 1.10.38
- updated star roulette
- updated some progress bar layouts
- New Badges
- New Event

## 1.10.37
- New logo 
- The percentage bug has been fixed
- Banners have been added to level comments (this option can be disabled in the settings)
- The mod settings design was improved
- The rewards for the gem rule have been changed.
- Some textures were updated
- New badges have been added to the store
- New country badges were added
- Some errors in the textures were corrected
- The design of the system notifications was changed slightly, making them a little more transparent.
- The animation of the streak in the InfoPopup was slightly modified

## 1.10.36 Beta 5
- improvements in progression
- new styles for names

# 1.10.34 Alpha 2
- Fix Streak Progress bar

# 1.10.34 Alpha 1
- Fix Bugs
- 5 New Masteries

# 1.10.33
- New Daily Shop
- New masteries
- Streak points in level information
- Rank/Role Status 

# 1.10.32
- Fix bugs
- Update Support
- Update Streak Animation
- New Streak Points Progression
- seasons were included every 30 days (beta)

# 1.10.2 Banners & Custom Profiles
- New events
- Texture update
- Banners implementation
- Custom profiles
- Experience levels
- New streak design

# 1.10.1 
- New style for roulette
- New badges
- Improved notifications
- Event!
- improved level challenge interface
- improved store interface
- Some textures were updated!

# 1.9.7
- Fix bugs

# 1.8.9
- The design of the upper part was improved

# 1.8.7 New badges and Fix bugs
- I've added a more efficient registration system to avoid data duplication.
- New challenges have been added
- It was added to messaging for moderators' announcements
- A future button was added to store upcoming events
- animation returned

# 1.8.5 New Style And Badges
- More badges were added
- New design in the main menu
- new challenge interface
- new global top (BETA)
- New code redemption interface

# 1.8.5 (Spanish) Nuevo estilo e insignias
- Se agregaron mas insignias
- Nuevo estilo del menu principal
- Nueva interfaz de desafios
- Nuevo top global (BETA)
- Nueva interfaz de canje de codigos

# 1.7.9 Online Mode 
- online mode was added

# 1.7.7 Bye bye Stars
- New scoring system
- Added total streak points to profile stats
- fix bugs

# 1.7.0 Fix bugs
- fix compatibility

# 1.5.7 BETA New Roulette and Badges!
- New Roulette
- New Badge collection
- New progress bar when completing a level (BETA)
- New badge shop!
- New star history section (BETA)
- Added an effect when obtaining a mythic badge (BETA)
- New daily missions
- New change "Star tickets" have been added
- "Superstars" were added to spin the roulette wheel
- Added streak to the main menu (configurable in mod settings)

# WARNING
Only you will be able to see your badges since the "ONLINE" mode has not been added in this update.

# Spanish
# 1.5.7 BETA Nueva Ruleta e Insignias!
- Nueva Ruleta
- Nueva Coleccion de Insignias
- Nueva Barra de Progreso al Completar un Nivel (BETA)
- Nueva Tienda de Insignias!
- Nueva Seccion de Historial de Estrellas (BETA)
- Se agrego un Efecto al Obtener una Insignia Mitica (BETA)
- Nuevas Misiones Diarias
- Nuevo Cambio: Se agregaron los "Tickets Estrella"
- Se agregaron las "Superestrellas" para Girar la Ruleta
- Se agrego  una Racha al Menu Principal (Configurable en los Ajustes del Mod)

# ADVERTENCIA
Solo tu podras ver tus insignias, ya que el modo "ONLINE" no se ha agregado en esta actualizacion.

# 1.5.3 BETA
- New progress bar that appears upon completing a level
- Added an option to show the streak in the pause menu
- Improved the animation when unlocking a new streak

# 1.4.9 BETA
- Added a new animation when unlocking a new day
- some bugs were fixed

# 1.4.2 BETA
- Fix Bugs
- This version is going to have many changes so a new beta will be activated.

# 1.3.4 Fix bugs
- Bugs Fixes
- update Streak animation
- New Badges
- New equip badge (No oline mode for now)

# 1.2.2
- More Badges!!
- More Missions
- Added badge quality from Common to Mythic
- Fix Bugs

# 1.1.8
- [New Badges] Increase your streak days to unlock them
- New missions

# 1.1.5
- Added a button to view progress over 100 days
- rewards button coming soon

# 1.1.0
- added streak display
- add new streak animation!

# BETA 0.1.2
- added display of all streaks
- streaks now have animation
- UI improvements

# BETA 0.0.2
- Added a daily streak system for collecting stars