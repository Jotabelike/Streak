const express = require('express');
const admin = require('firebase-admin');
const cors = require('cors');
const compression = require('compression');
const crypto = require('crypto');
const app = express();
const port = process.env.PORT || 3000;

app.use(compression());
app.use(express.json({ limit: '50kb' }));

// =====================
// CORS restringido
// =====================
app.use(cors({
    origin: (origin, callback) => {
        if (!origin) return callback(null, true);
        const allowed = ['https://tu-panel-admin.com'];
        if (allowed.includes(origin)) return callback(null, true);
        callback(new Error('CORS blocked'));
    }
}));

const MANTENIMIENTO_ACTIVO = false;
const SEASON_DURATION_DAYS = 30;
const SUPER_ADMIN_ID = 25601774;
const KNOWN_TASK_IDS = ['task_yt', 'task_tk'];

// =====================
// HMAC_SECRET (variable de entorno)
// =====================
const HMAC_SECRET = process.env.HMAC_SECRET;
if (!HMAC_SECRET) {
    console.error("FATAL: HMAC_SECRET no configurado.");
    process.exit(1);
}

// =====================
// TOKEN CONFIG
// =====================
const TOKEN_EXPIRY_MS = 24 * 60 * 60 * 1000; // 24 horas

// =====================
// Rate Limiting
// =====================
const rateLimitMap = new Map();
const RATE_LIMIT_WINDOW = 60 * 1000;
const RATE_LIMIT_MAX = 30;

function rateLimit(req, res, next) {
    const ip = req.headers['x-forwarded-for'] || req.ip;
    const now = Date.now();
    if (!rateLimitMap.has(ip)) {
        rateLimitMap.set(ip, { count: 1, resetAt: now + RATE_LIMIT_WINDOW });
        return next();
    }
    const entry = rateLimitMap.get(ip);
    if (now > entry.resetAt) { entry.count = 1; entry.resetAt = now + RATE_LIMIT_WINDOW; return next(); }
    entry.count++;
    if (entry.count > RATE_LIMIT_MAX) return res.status(429).json({ error: "Rate limit" });
    next();
}
setInterval(() => {
    const now = Date.now();
    for (const [ip, entry] of rateLimitMap) {
        if (now > entry.resetAt + 60000) rateLimitMap.delete(ip);
    }
}, 5 * 60 * 1000);
app.use(rateLimit);

// =====================
// HMAC Verification (CAPA 1)
// =====================
function verifyHMAC(req, res, next) {
    const signature = req.headers['x-signature'];
    const timestamp = parseInt(req.headers['x-timestamp']);
    const clientAccountID = req.headers['x-account-id'];
    if (!signature || !timestamp || !clientAccountID) return res.status(401).json({ error: "Missing auth" });

    const now = Math.floor(Date.now() / 1000);
    if (Math.abs(now - timestamp) > 300) return res.status(401).json({ error: "Expired" });

    const urlAccountID = req.params.accountID;
    if (urlAccountID && String(clientAccountID) !== String(urlAccountID)) return res.status(403).json({ error: "Mismatch" });

    const bodyStr = JSON.stringify(req.body || {});
    const message = `${clientAccountID}:${timestamp}:${bodyStr}`;
    const expectedSig = crypto.createHmac('sha256', HMAC_SECRET).update(message).digest('hex');

    try {
        if (!crypto.timingSafeEqual(Buffer.from(signature, 'hex'), Buffer.from(expectedSig, 'hex'))) {
            return res.status(401).json({ error: "Bad signature" });
        }
    } catch (e) {
        return res.status(401).json({ error: "Bad signature format" });
    }

    req.verifiedAccountID = parseInt(clientAccountID);
    next();
}

// =====================
// Anti-Replay (CAPA 2)
// =====================
const usedSignatures = new Map();
function antiReplay(req, res, next) {
    const signature = req.headers['x-signature'];
    if (usedSignatures.has(signature)) return res.status(401).json({ error: "Replay" });
    usedSignatures.set(signature, Date.now());
    next();
}
setInterval(() => {
    const cutoff = Date.now() - 10 * 60 * 1000;
    for (const [sig, time] of usedSignatures) {
        if (time < cutoff) usedSignatures.delete(sig);
    }
}, 10 * 60 * 1000);

// =====================
// SESSION TOKEN CACHE (evita lecturas a Firebase en cada petición)
// =====================
const sessionCache = new Map(); // accountID -> { token, created_at }

// Limpiar sesiones expiradas del cache cada 10 minutos
setInterval(() => {
    const now = Date.now();
    for (const [id, session] of sessionCache) {
        if (now - session.created_at > TOKEN_EXPIRY_MS) sessionCache.delete(id);
    }
}, 10 * 60 * 1000);

// =====================
// SESSION TOKEN VALIDATION (CAPA 3)
// Se usa en TODOS los endpoints excepto GET /players/:accountID
// porque ese es el que GENERA el token.
// =====================
async function verifySessionToken(req, res, next) {
    const accountID = req.verifiedAccountID;
    const clientToken = req.headers['x-session-token'];

    if (!clientToken) return res.status(401).json({ error: "No session token" });

    try {
        // Buscar en cache primero (0ms) antes de ir a Firebase (~50-200ms)
        let session = sessionCache.get(accountID);

        // Si no está en cache, ir a Firebase (solo pasa después de un restart del server)
        if (!session) {
            const snap = await db.ref(`players/${accountID}/session`).once('value');
            session = snap.val();
            if (session) sessionCache.set(accountID, session);
        }

        if (!session || !session.token || !session.created_at) {
            return res.status(401).json({ error: "No active session" });
        }

        if (Date.now() - session.created_at > TOKEN_EXPIRY_MS) {
            sessionCache.delete(accountID);
            db.ref(`players/${accountID}/session`).remove().catch(() => { });
            return res.status(401).json({ error: "Session expired" });
        }

        try {
            if (!crypto.timingSafeEqual(
                Buffer.from(clientToken, 'utf8'),
                Buffer.from(session.token, 'utf8')
            )) {
                return res.status(401).json({ error: "Invalid session" });
            }
        } catch (e) {
            return res.status(401).json({ error: "Invalid session format" });
        }

        next();
    } catch (e) {
        console.error("Session verification error:", e);
        return res.status(500).json({ error: "Session check failed" });
    }
}

// =====================
// Input Validation
// =====================
function isValidAccountID(id) {
    const num = parseInt(id);
    return !isNaN(num) && num > 0 && num < 2147483647;
}

function sanitizeString(str, maxLen = 100) {
    if (typeof str !== 'string') return '';
    return str.slice(0, maxLen).replace(/[<>]/g, '');
}

// =====================
// Whitelist / Blacklist de campos
// =====================
const ALLOWED_SAVE_FIELDS = new Set([
    'username', 'userID', 'isGDPS',
    'equipped_badge_id', 'equipped_banner_id',
    'equipped_name_color', 'equipped_name_font',
    'equipped_name_effect', 'equipped_name_animation',
    'last_streak_animated', 'gem_roulette_spin_count',
    'gem_roulette_hash', 'last_roulette_index', 'total_spins',
    'gem_roulette_state', 'taskStatuses', 'unlocked_badges',
    'unlocked_banners', 'pinned_levels', 'missions',
    'has_mythic_color', 'completedLevelMissions',
    'claimed_streak_goals', 'claimed_discord_milestones'
]);

const FORBIDDEN_FIELDS = new Set([
    'role', 'ban', 'ban_reason', 'banned_by', 'streakID',
    'global_rank', 'event_progress', 'session',
    'gems', 'super_stars', 'star_tickets',
    'current_xp', 'current_level',
    'current_streak_days', 'total_streak_points',
    'streakPointsToday', 'lastDay', 'history',
    'streak_updated_today', 'pending_season_reward',
    'season_reward_ts', 'inbox', 'claimed_global_mails',
    'usedCodes', 'claimed_trending_levels',
    'unlocked_name_items'
]);

const NAME_ITEM_BASIC_COLORS = new Set([
    "Black", "Blue", "Brown", "Cyan", "Gold", "Green", "Lime", "Magenta",
    "Maroon", "Mint", "Navy", "Orange", "Peach", "Pink", "Purple", "Red",
    "Silver", "Teal", "Yellow"
]);

// Mirrors StreakData::getNameItemPrice on the client.
// Must stay in sync with src/StreakData.cpp.
function getNameItemPrice(item) {
    if (typeof item !== 'string' || !item) return 0;
    if (item === "Default" || item === "None") return 0;
    if (item.includes("Wave") || item === "Synthwave" || item.includes("Blink") || item === "Rainbow") return 800;
    if (item.includes("Static")) return 350;
    if (item.includes("Font") || item === "Chat" || item === "Gold" || item === "Pusab") return 150;
    if (NAME_ITEM_BASIC_COLORS.has(item)) return 100;
    return 250;
}

function sanitizePlayerData(data) {
    const clean = {};
    for (const [key, value] of Object.entries(data)) {
        if (FORBIDDEN_FIELDS.has(key)) continue;
        if (ALLOWED_SAVE_FIELDS.has(key)) clean[key] = value;
    }
    return clean;
}

// =====================
// SEASON REWARDS
// =====================
const SEASON_REWARDS = {
    1: { gems: 100, super_stars: 250, star_tickets: 25000 },
    2: { gems: 40, super_stars: 100, star_tickets: 15000 },
    3: { gems: 25, super_stars: 60, star_tickets: 5000 }
};

let cachedSettings = { tasks_active: true, discord_goal_active: false, discord_count: 500, discord_goal_max: 1000 };
let lastSettingsUpdate = 0;
let isProcessingSeason = false;

// =====================
// Mantenimiento
// =====================
app.use((req, res, next) => {
    if (MANTENIMIENTO_ACTIVO) {
        const idEnBody = req.body && req.body.accountID;
        const idEnUrl = req.params.accountID;
        const id = parseInt(idEnBody || idEnUrl);
        if (id === SUPER_ADMIN_ID) return next();
        return res.status(503).json({ error: "Maintenance", message: "Server maintenance. Your streak is safe." });
    }
    next();
});

// =====================
// Firebase init
// =====================
try {
    const serviceAccount = require('./serviceAccountKey.json');
    admin.initializeApp({
        credential: admin.credential.cert(serviceAccount),
        databaseURL: 'https://streak-ea03f-default-rtdb.firebaseio.com'
    });
    console.log("Firebase conectado");
} catch (error) {
    console.error("ERROR Firebase:", error);
    process.exit(1);
}
const db = admin.database();

// =====================
// Funciones helper
// =====================
async function initTrendingLevel() {
    try {
        const snap = await db.ref('trending_level').once('value');
        if (!snap.exists()) {
            await db.ref('trending_level').set({
                levelID: 104245, name: "Bloodbath", creator: "Riot",
                isActive: true, difficultySprite: "difficulty_10_btn_001.png",
                rewards: { super_stars: 100, star_tickets: 500, gems: 10 }
            });
        }
    } catch (e) { console.error("Error initTrendingLevel:", e); }
}
initTrendingLevel();

async function initSettings() {
    try {
        const snap = await db.ref('settings').once('value');
        const cur = snap.val() || {};
        let updates = {};
        let need = false;
        if (cur.tasks_active === undefined) { updates['tasks_active'] = true; need = true; }
        if (cur.discord_goal_active === undefined) { updates['discord_goal_active'] = false; need = true; }
        if (cur.discord_count === undefined) { updates['discord_count'] = 0; need = true; }
        if (cur.discord_goal_max === undefined) { updates['discord_goal_max'] = 1000; need = true; }
        if (cur.discord_milestones === undefined) {
            updates['discord_milestones'] = [
                { req: 100, tickets: 0, stars: 0, gems: 0, spr: "chest_02_02_001.png", isChest: true },
                { req: 200, tickets: 0, stars: 0, gems: 0, spr: "chest_02_02_001.png", isChest: true },
                { req: 300, tickets: 6500, stars: 0, gems: 0, spr: "tickets_pack.png", isChest: false },
                { req: 400, tickets: 0, stars: 0, gems: 0, spr: "chest_02_02_001.png", isChest: true },
                { req: 500, tickets: 0, stars: 0, gems: 0, spr: "chest_02_02_001.png", isChest: true },
                { req: 600, tickets: 10000, stars: 0, gems: 0, spr: "tickets_pack.png", isChest: false },
                { req: 700, tickets: 0, stars: 0, gems: 0, spr: "chest_02_02_001.png", isChest: true },
                { req: 800, tickets: 0, stars: 0, gems: 0, spr: "chest_02_02_001.png", isChest: true },
                { req: 900, tickets: 0, stars: 200, gems: 0, spr: "star_pack.png", isChest: false },
                { req: 1000, tickets: 0, stars: 0, gems: 799, spr: "gem_pack.png", isChest: false }
            ];
            need = true;
        }
        if (need) await db.ref('settings').update(updates);
    } catch (e) { console.error("Error initSettings:", e); }
}
initSettings();

async function getSettings() {
    const now = Date.now();
    if (now - lastSettingsUpdate > 60000) {
        try {
            const snap = await db.ref('settings').once('value');
            const s = snap.val() || {};
            cachedSettings.tasks_active = s.tasks_active === true;
            cachedSettings.discord_goal_active = s.discord_goal_active === true;
            cachedSettings.discord_count = s.discord_count || 0;
            cachedSettings.discord_goal_max = s.discord_goal_max || 1000;
            cachedSettings.discord_milestones = s.discord_milestones || [];
            const date = new Date(); date.setHours(date.getHours() - 5);
            cachedSettings.daily_shop_seed = parseInt(date.toISOString().split('T')[0].replace(/-/g, ''));
            lastSettingsUpdate = now;
        } catch (e) { console.error("Error getSettings:", e); }
    }
    if (!cachedSettings.daily_shop_seed) {
        const date = new Date(); date.setHours(date.getHours() - 5);
        cachedSettings.daily_shop_seed = parseInt(date.toISOString().split('T')[0].replace(/-/g, ''));
    }
    return cachedSettings;
}

function getXPForCurrentStreak(streak) { return 25 + Math.floor(streak / 10) * 15; }
function getXPRequiredForNextLevel(lvl) { return (lvl || 1) * 100; }

function getLevelRewards(level) {
    let tickets = 0, stars = 0, gems = 0;
    if (level < 10) tickets = 40;
    else if (level < 20) tickets = 48;
    else tickets = 48 * Math.pow(2, Math.floor(level / 10) - 1);
    if (level < 10) stars = 5;
    else if (level < 20) stars = 20;
    else stars = 20 + (Math.floor(level / 10) - 1) * 20;
    gems = Math.floor((level - 1) / 10) + 1;
    return { tickets, stars, gems };
}

function processXP(playerData, amount) {
    if (!amount || amount <= 0) return { stars: 0, tickets: 0, levelsGained: 0, gems: 0 };
    if (typeof playerData.current_xp !== 'number') playerData.current_xp = 0;
    if (typeof playerData.current_level !== 'number') playerData.current_level = 1;
    playerData.current_xp += amount;
    let result = { stars: 0, tickets: 0, levelsGained: 0, gems: 0 };
    let required = getXPRequiredForNextLevel(playerData.current_level);
    while (playerData.current_xp >= required) {
        playerData.current_xp -= required;
        playerData.current_level++;
        result.levelsGained++;
        const rewards = getLevelRewards(playerData.current_level);
        playerData.super_stars = (playerData.super_stars || 0) + rewards.stars;
        playerData.star_tickets = (playerData.star_tickets || 0) + rewards.tickets;
        playerData.gems = (playerData.gems || 0) + rewards.gems;
        result.stars += rewards.stars;
        result.tickets += rewards.tickets;
        result.gems += rewards.gems;
        required = getXPRequiredForNextLevel(playerData.current_level);
    }
    return result;
}

function calculatePointsForStars(stars) {
    if (stars <= 3) return 1;
    if (stars <= 5) return 3;
    if (stars <= 7) return 4;
    if (stars <= 9) return 5;
    return 6;
}

function getRequiredPointsForStreak(streak) {
    if (streak >= 100) return 12;
    if (streak >= 90) return 11;
    if (streak >= 80) return 10;
    if (streak >= 70) return 9;
    if (streak >= 60) return 8;
    if (streak >= 50) return 7;
    if (streak >= 40) return 6;
    if (streak >= 30) return 5;
    if (streak >= 20) return 4;
    return streak >= 10 ? 3 : 2;
}

function getTodayStr() {
    const date = new Date(); date.setHours(date.getHours() - 5);
    return date.toISOString().split('T')[0];
}

function generateStreakID() {
    const chars = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789';
    let result = 'STK-';
    for (let i = 0; i < 5; i++) result += chars.charAt(Math.floor(Math.random() * chars.length));
    return result;
}

function getDiffDays(lastDayStr, todayStr) {
    return Math.floor((new Date(todayStr + 'T12:00:00Z') - new Date(lastDayStr + 'T12:00:00Z')) / 86400000);
}

function evaluateStreakReset(data, today) {
    if (!data.lastDay || data.lastDay === today) return false;
    const diffDays = getDiffDays(data.lastDay, today);
    let shouldReset = false;
    if (diffDays > 1) shouldReset = true;
    else if (diffDays === 1) {
        if ((data.streakPointsToday || 0) < getRequiredPointsForStreak(data.current_streak_days || 0))
            shouldReset = true;
    }
    if (shouldReset) {
        data.current_streak_days = 0;
        data.last_streak_animated = 0;
        data.event_progress = null;
        data.streakPointsToday = 0;
    }
    if (diffDays >= 1) {
        data.lastDay = today;
        data.streakPointsToday = 0;
        data.daily_msg_count = 0;
        data.missions = { pm1: false, pm2: false, pm3: false, pm4: false, pm5: false, pm6: false };
        data.streak_updated_today = false;
    }
    return shouldReset;
}

async function checkBanStatus(req, res, next) {
    let id = req.params.accountID || (req.body ? req.body.accountID : undefined);
    if (!id) return next();
    if (parseInt(id) === SUPER_ADMIN_ID) return next();
    try {
        const snap = await db.ref(`players/${id}`).once('value');
        const data = snap.val();
        if (data && data.ban === true) return res.status(403).json({ error: "Banned" });
    } catch (e) { console.error("checkBanStatus error:", e); }
    next();
}

async function checkSeasonTurnover() {
    if (isProcessingSeason) return Date.now() + 10000;
    try {
        const settingsRef = db.ref('settings/season_end');
        const snap = await settingsRef.once('value');
        let endTime = snap.val();
        const now = Date.now();
        if (!endTime) {
            endTime = now + (SEASON_DURATION_DAYS * 24 * 60 * 60 * 1000);
            await settingsRef.set(endTime);
            return endTime;
        }
        if (now >= endTime) {
            isProcessingSeason = true;
            const pSnap = await db.ref('players').orderByChild('current_streak_days').limitToLast(50).once('value');
            const candidates = [];
            pSnap.forEach(c => {
                const p = c.val();
                if (!p.ban && !p.isGDPS) candidates.push({ ...p, accountID: parseInt(c.key) });
            });
            candidates.sort((a, b) => {
                const s = (b.current_streak_days || 0) - (a.current_streak_days || 0);
                if (s !== 0) return s;
                const p = (b.total_streak_points || 0) - (a.total_streak_points || 0);
                if (p !== 0) return p;
                return (a.accountID || 0) - (b.accountID || 0);
            });
            const updates = {};
            for (let i = 0; i < Math.min(candidates.length, 3); i++) {
                const player = candidates[i];
                updates[`players/${player.accountID}/pending_season_reward`] = i + 1;
                updates[`players/${player.accountID}/season_reward_ts`] = now;
            }
            const newEnd = now + (SEASON_DURATION_DAYS * 24 * 60 * 60 * 1000);
            updates[`settings/season_end`] = newEnd;
            await db.ref().update(updates);
            isProcessingSeason = false;
            return newEnd;
        }
        return endTime;
    } catch (e) {
        console.error("Season error:", e);
        isProcessingSeason = false;
        return Date.now() + 86400000;
    }
}

// =====================================================
// ENDPOINTS
// =====================================================

// =====================
// GET PLAYER — Genera el session token aquí
// Solo necesita HMAC (no session token, porque este endpoint LO CREA)
// =====================
app.get('/players/:accountID', verifyHMAC, async (req, res) => {
    const accountID = req.verifiedAccountID;
    if (!isValidAccountID(accountID)) return res.status(400).json({ error: 'ID' });

    try {
        const playerRef = db.ref(`players/${accountID}`);
        const [playerSnap, settings] = await Promise.all([playerRef.once('value'), getSettings()]);

        if (!playerSnap.exists()) {
            // GENERAR SESSION TOKEN PARA EL REGISTRO
            const sessionToken = crypto.randomBytes(32).toString('hex');
            const sessionData = { token: sessionToken, created_at: Date.now() };

            // Guardamos la sesión en Firebase y Caché
            await playerRef.child('session').set(sessionData);
            sessionCache.set(accountID, sessionData);

            // Devolvemos el 404 para que el cliente sepa que debe mostrar el registro, pero INCLUIMOS el token
            return res.status(404).json({
                error: '404',
                session_token: sessionToken
            });
        }

        let data = playerSnap.val();
        data.task_enabled = settings.tasks_active;
        data.discord_goal_enabled = settings.discord_goal_active;
        data.daily_shop_seed = settings.daily_shop_seed;
        data.discord_count = settings.discord_count;
        data.discord_goal_max = settings.discord_goal_max;
        data.discord_milestones = settings.discord_milestones || [];

        const today = getTodayStr();
        if (data.lastDay && data.lastDay !== today) {
            const diffDays = getDiffDays(data.lastDay, today);
            let shouldReset = false;
            if (diffDays > 1) shouldReset = true;
            else if (diffDays === 1) {
                if ((data.streakPointsToday || 0) < getRequiredPointsForStreak(data.current_streak_days || 0))
                    shouldReset = true;
            }
            if (shouldReset) {
                data.current_streak_days = 0;
                data.last_streak_animated = 0;
                data.streakPointsToday = 0;
                data.lastDay = today;
                await playerRef.update({ current_streak_days: 0, last_streak_animated: 0, streakPointsToday: 0, lastDay: today });
            } else if (diffDays >= 1) {
                data.streakPointsToday = 0;
                data.lastDay = today;
                data.missions = { pm1: false, pm2: false, pm3: false, pm4: false, pm5: false, pm6: false };
                data.streak_updated_today = false;
                await playerRef.update({
                    streakPointsToday: 0, lastDay: today,
                    missions: { pm1: false, pm2: false, pm3: false, pm4: false, pm5: false, pm6: false },
                    streak_updated_today: false
                });
            }
        }

        if (data.pending_season_reward && data.season_reward_ts) {
            if (Date.now() > (data.season_reward_ts + 86400000)) {
                playerRef.update({ pending_season_reward: null, season_reward_ts: null });
                delete data.pending_season_reward;
                delete data.season_reward_ts;
            }
        }

        const savedStatuses = data.taskStatuses || {};
        data.taskStatuses = { ...savedStatuses };
        const taskChecks = KNOWN_TASK_IDS.map(async (taskID) => {
            if (savedStatuses[taskID] !== "claimed") {
                const s = await db.ref(`task_submissions/${taskID}/${accountID}/status`).once('value');
                return { taskID, val: s.val() };
            }
            return null;
        });
        const results = await Promise.all(taskChecks);
        results.forEach(r => { if (r && r.val) data.taskStatuses[r.taskID] = r.val; });

        if (!data.streakID) {
            data.streakID = generateStreakID();
            playerRef.child('streakID').set(data.streakID);
        }

        if (accountID === SUPER_ADMIN_ID) { data.role = 2; data.ban = false; }
        data.global_rank = 0;

        // =====================
        // GENERAR SESSION TOKEN
        // =====================
        const sessionToken = crypto.randomBytes(32).toString('hex');
        const sessionData = { token: sessionToken, created_at: Date.now() };
        await playerRef.child('session').set(sessionData);

        // Guardar en cache para evitar lecturas a Firebase
        sessionCache.set(accountID, sessionData);

        // Incluir el token en la respuesta (el cliente lo guardará)
        data.session_token = sessionToken;

        // No enviar el objeto session interno al cliente
        delete data.session;

        res.json(data);
    } catch (e) {
        console.error(e);
        res.status(500).json({ error: 'Err' });
    }
});

// =====================
// COMPLETE LEVEL — HMAC + Session Token
// =====================
app.post('/players/:accountID/complete-level', verifyHMAC, antiReplay, verifySessionToken, async (req, res) => {
    const accountID = req.verifiedAccountID;
    const clientStars = parseInt(req.body.stars, 10);
    const today = getTodayStr(); // Servidor decide la fecha

    if (isNaN(accountID) || isNaN(clientStars) || clientStars < 1 || clientStars > 10) {
        return res.status(400).json({ error: 'Invalid' });
    }

    const playerRef = db.ref(`players/${accountID}`);
    try {
        const snap = await playerRef.once('value');
        if (!snap.exists()) return res.status(404).json({ error: '404' });
        let data = snap.val();
        if (data.ban === true) return res.status(403).json({ error: 'Banned' });

        if (!data.lastDay) data.lastDay = today;
        else evaluateStreakReset(data, today);

        const points = calculatePointsForStars(clientStars);
        const reqPointsForToday = getRequiredPointsForStreak(data.current_streak_days || 0);
        const hadStreakCompleted = (data.streakPointsToday || 0) >= reqPointsForToday;

        data.streakPointsToday = (data.streakPointsToday || 0) + points;
        data.total_streak_points = (data.total_streak_points || 0) + points;
        if (!data.history) data.history = {};
        data.history[today] = data.streakPointsToday;

        let rewards = { stars: 0, tickets: 0, levels: 0 };
        const currentStreak = data.current_streak_days || 0;
        const conditionNormal = (currentStreak > 0) && !hadStreakCompleted && (data.streakPointsToday >= reqPointsForToday);
        const conditionFix = (currentStreak === 0) && (data.streakPointsToday >= reqPointsForToday);

        if (conditionNormal || conditionFix) {
            data.current_streak_days = (data.current_streak_days || 0) + 1;
            data.streak_updated_today = true;
            const resXP = processXP(data, getXPForCurrentStreak(data.current_streak_days));
            rewards = { stars: resXP.stars, tickets: resXP.tickets, levels: resXP.levelsGained };
        }

        // No guardar el campo session en el update general
        const { session, ...dataToSave } = data;
        await playerRef.update(dataToSave);
        res.json({ ...dataToSave, newRewards: rewards });
    } catch (e) {
        console.error("Error complete-level:", e);
        res.status(500).json({ error: 'Err' });
    }
});

// =====================
// SAVE PLAYER — HMAC + Session Token + Whitelist
// =====================
app.post('/players/:accountID', verifyHMAC, antiReplay, verifySessionToken, checkBanStatus, async (req, res) => {
    const accountID = req.verifiedAccountID;
    if (!isValidAccountID(accountID)) return res.status(400).json({ error: 'ID' });
    const cleanData = sanitizePlayerData(req.body);
    if (Object.keys(cleanData).length === 0) return res.status(400).json({ error: 'No valid fields' });
    try {
        await db.ref(`players/${accountID}`).update(cleanData);
        res.json({ msg: 'OK' });
    } catch (e) { res.status(500).json({ error: 'Error' }); }
});

// =====================
// REDEEM CODE — HMAC + Session Token
// =====================
app.post('/redeem-code', verifyHMAC, antiReplay, verifySessionToken, async (req, res) => {
    const accountID = req.verifiedAccountID;
    const stdCode = req.body.code ? req.body.code.toUpperCase().trim().replace(/ /g, '-') : '';
    if (!stdCode || !isValidAccountID(accountID)) return res.status(400).json({ error: "Inv" });

    try {
        const [codeSnap, playerSnap] = await Promise.all([
            db.ref(`codes/${stdCode}`).once('value'),
            db.ref(`players/${accountID}`).once('value')
        ]);
        if (!codeSnap.exists()) return res.status(404).json({ error: "NoCode" });
        const cData = codeSnap.val();
        if (!cData.isActive || cData.usesLeft <= 0) return res.status(403).json({ error: "Empty" });
        const pData = playerSnap.val() || {};
        if (pData.ban) return res.status(403).json({ error: "Banned" });
        if (pData.usedCodes && pData.usedCodes[stdCode]) return res.status(409).json({ error: "Used" });

        const r = cData.rewards || {};
        let lvlRes = { stars: 0, tickets: 0, levelsGained: 0, gems: 0 };
        if (r.xp) lvlRes = processXP(pData, parseInt(r.xp));
        if (r.super_stars) pData.super_stars = (pData.super_stars || 0) + r.super_stars;
        if (r.star_tickets) pData.star_tickets = (pData.star_tickets || 0) + r.star_tickets;
        if (r.gems) pData.gems = (pData.gems || 0) + r.gems;

        const updates = {
            super_stars: pData.super_stars, star_tickets: pData.star_tickets,
            gems: pData.gems || 0, current_xp: pData.current_xp,
            current_level: pData.current_level, [`usedCodes/${stdCode}`]: true
        };
        if (r.badge) {
            const b = Array.isArray(pData.unlocked_badges) ? pData.unlocked_badges : [];
            if (!b.includes(r.badge)) { b.push(r.badge); updates.unlocked_badges = b; }
        }
        if (r.banner) {
            const bn = Array.isArray(pData.unlocked_banners) ? pData.unlocked_banners : [];
            if (!bn.includes(r.banner)) { bn.push(r.banner); updates.unlocked_banners = bn; }
        }
        await Promise.all([
            db.ref(`players/${accountID}`).update(updates),
            db.ref(`codes/${stdCode}/usesLeft`).set(cData.usesLeft - 1)
        ]);
        // balances reflect the post-redeem state including any XP level-ups.
        // The client must use these directly and NOT re-process XP locally,
        // otherwise level-up rewards get applied twice.
        res.json({
            success: true,
            rewards: r,
            levelUpRewards: lvlRes,
            balances: {
                super_stars: pData.super_stars || 0,
                star_tickets: pData.star_tickets || 0,
                gems: pData.gems || 0
            },
            current_xp: pData.current_xp || 0,
            current_level: pData.current_level || 1
        });
    } catch (e) { console.error(e); res.status(500).json({ error: "Err" }); }
});

// =====================
// EVENT
// =====================
app.get('/event/current/:accountID', verifyHMAC, verifySessionToken, async (req, res) => {
    const accountID = req.verifiedAccountID;
    const isRouletteClient = req.query.type === 'roulette';
    try {
        const eSnap = await db.ref('/current_event').once('value');
        if (!eSnap.exists()) return res.status(404).json({ error: "No event" });
        const eData = eSnap.val();
        if (!isRouletteClient && eData.isActive === false) return res.status(404).json({ error: "Disabled" });
        if (isRouletteClient && eData.isActiveRoulette === false) return res.status(404).json({ error: "Disabled" });
        const pSnap = await db.ref(`players/${accountID}/event_progress/${eData.eventID}`).once('value');
        res.json({ event: eData, progress: pSnap.val() || {} });
    } catch (e) { res.status(500).json({ error: "Error" }); }
});

app.post('/event/claim', verifyHMAC, antiReplay, verifySessionToken, async (req, res) => {
    const accountID = req.verifiedAccountID;
    const { eventID, claimID } = req.body;
    try {
        const pRef = db.ref(`players/${accountID}`);
        const eRef = db.ref('/current_event');
        const [pSnap, eSnap] = await Promise.all([pRef.once('value'), eRef.once('value')]);
        if (!pSnap.exists() || !eSnap.exists()) return res.status(404).json({ error: "404" });
        let pData = pSnap.val();
        const eData = eSnap.val();
        if (eData.eventID !== eventID) return res.status(403).json({ error: "Invalid" });

        let finalRewardID = claimID;
        let reward = null;
        const updates = {};

        if (eData.eventType === 'roulette') {
            if (eData.spinCost > 0) {
                const cost = parseInt(eData.spinCost);
                if (eData.spinCurrency === "stars") {
                    if ((pData.super_stars || 0) < cost) return res.status(402).json({ error: "Funds" });
                    pData.super_stars -= cost; updates['super_stars'] = pData.super_stars;
                } else if (eData.spinCurrency === "gems") {
                    if ((pData.gems || 0) < cost) return res.status(402).json({ error: "Funds" });
                    pData.gems -= cost; updates['gems'] = pData.gems;
                } else {
                    if ((pData.star_tickets || 0) < cost) return res.status(402).json({ error: "Funds" });
                    pData.star_tickets -= cost; updates['star_tickets'] = pData.star_tickets;
                }
            }
            const rewardsMap = eData.rewards;
            const weights = eData.rarityWeights || { "Mythic": 3, "Legendary": 10, "Epic": 50, "Special": 300, "Common": 637 };
            let totalWeight = 0;
            const lottery = [];
            Object.keys(rewardsMap).forEach(key => {
                const r = rewardsMap[key];
                const w = weights[r.rarity || "Common"] || weights["Common"];
                lottery.push({ id: key, w }); totalWeight += w;
            });
            let rnd = Math.random() * totalWeight;
            for (const item of lottery) { if (rnd < item.w) { finalRewardID = item.id; break; } rnd -= item.w; }
            if (!finalRewardID) finalRewardID = Object.keys(rewardsMap)[0];
            reward = eData.rewards[finalRewardID];
        } else {
            reward = eData.rewards ? eData.rewards[claimID] : null;
            if (!reward) return res.status(404).json({ error: "Reward 404" });
            const progSnap = await pRef.child(`event_progress/${eventID}/${claimID}`).once('value');
            if (progSnap.exists()) return res.status(409).json({ error: "Claimed" });
            if ((pData.current_streak_days || 0) < reward.day) return res.status(403).json({ error: "Streak low" });
        }

        if (reward.super_stars) updates['super_stars'] = (pData.super_stars || 0) + reward.super_stars;
        if (reward.star_tickets) updates['star_tickets'] = (pData.star_tickets || 0) + reward.star_tickets;
        if (reward.badge) {
            const b = Array.isArray(pData.unlocked_badges) ? pData.unlocked_badges : [];
            if (!b.includes(reward.badge)) { b.push(reward.badge); updates['unlocked_badges'] = b; }
        }
        if (reward.gems) updates['gems'] = (pData.gems || 0) + reward.gems;
        if (reward.banner) {
            const bn = Array.isArray(pData.unlocked_banners) ? pData.unlocked_banners : [];
            if (!bn.includes(reward.banner)) { bn.push(reward.banner); updates['unlocked_banners'] = bn; }
        }
        if (eData.eventType !== 'roulette') updates[`event_progress/${eventID}/${finalRewardID}`] = true;
        await pRef.update(updates);
        res.json({ success: true, rewardID: finalRewardID });
    } catch (e) { res.status(500).json({ error: "Err" }); }
});

// =====================
// TASKS
// =====================
app.post('/tasks/submit', verifyHMAC, antiReplay, verifySessionToken, async (req, res) => {
    const accountID = req.verifiedAccountID;
    const { taskID, url } = req.body;
    if (!taskID || !url) return res.status(400).json({ error: "Data" });
    try {
        await db.ref(`task_submissions/${taskID}/${accountID}`).set({
            url: sanitizeString(url, 500), status: "pending",
            timestamp: admin.database.ServerValue.TIMESTAMP,
            username: sanitizeString(req.body.username || "Unknown", 50)
        });
        res.json({ success: true, status: "pending" });
    } catch (e) { res.status(500).json({ error: "Err" }); }
});

app.post('/tasks/reset', verifyHMAC, antiReplay, verifySessionToken, async (req, res) => {
    const accountID = req.verifiedAccountID;
    const { taskID } = req.body;
    try {
        await db.ref(`task_submissions/${taskID}/${accountID}`).remove();
        res.json({ success: true, status: "" });
    } catch (e) { res.status(500).json({ error: "Err" }); }
});

// =====================
// PUBLIC PROFILE (público, solo datos cosméticos)
// Para mostrar badges/banners de otros jugadores en comentarios
// NO requiere autenticación porque solo devuelve datos visuales
// =====================
app.get('/players/:accountID/public-profile', async (req, res) => {
    const accountID = req.params.accountID;
    if (!isValidAccountID(accountID)) return res.status(400).json({ error: 'ID' });
    try {
        const snap = await db.ref(`players/${accountID}`).once('value');
        if (!snap.exists()) return res.status(404).json({ error: '404' });
        const data = snap.val();
        // Solo datos de display público, nada modificable
        res.json({
            equipped_badge_id: data.equipped_badge_id || '',
            equipped_banner_id: data.equipped_banner_id || '',
            equipped_name_color: data.equipped_name_color || 'Default',
            equipped_name_font: data.equipped_name_font || 'Default',
            equipped_name_effect: data.equipped_name_effect || 'None',
            equipped_name_animation: data.equipped_name_animation || 'None',
            current_level: data.current_level || 1,
            current_xp: data.current_xp || 0,
            super_stars: data.super_stars || 0,
            star_tickets: data.star_tickets || 0,
            current_streak_days: data.current_streak_days || 0,
            total_streak_points: data.total_streak_points || 0,
            streakID: data.streakID || '???',
            global_rank: data.global_rank || 0,
            role: data.role || 0
        });
    } catch (e) { res.status(500).json({ error: 'Err' }); }
});

// =====================
// LEADERBOARD (público)
// =====================
app.get('/leaderboard', async (req, res) => {
    try {
        const seasonEnd = await checkSeasonTurnover();
        const snap = await db.ref('players').orderByChild('current_streak_days').limitToLast(50).once('value');
        const list = [];
        const today = getTodayStr();
        snap.forEach(c => {
            const p = c.val();
            if (p.ban !== true) {
                let realStreak = p.current_streak_days || 0;
                if (realStreak > 0 && p.lastDay && p.lastDay !== today) {
                    const diffDays = getDiffDays(p.lastDay, today);
                    if (diffDays > 1) {
                        realStreak = 0;
                        db.ref(`players/${p.accountID}`).update({
                            current_streak_days: 0, last_streak_animated: 0,
                            streakPointsToday: 0, lastDay: today
                        }).catch(() => { });
                    } else if (diffDays === 1) {
                        if ((p.streakPointsToday || 0) < getRequiredPointsForStreak(realStreak)) {
                            realStreak = 0;
                            db.ref(`players/${p.accountID}`).update({
                                current_streak_days: 0, last_streak_animated: 0,
                                streakPointsToday: 0, lastDay: today
                            }).catch(() => { });
                        }
                    }
                }
                if (realStreak > 0) {
                    list.push({
                        username: p.username || 'Unknown', accountID: parseInt(c.key),
                        userID: p.userID || 0, current_streak_days: realStreak,
                        total_streak_points: p.total_streak_points || 0,
                        equipped_badge_id: p.equipped_badge_id || '',
                        equipped_banner_id: p.equipped_banner_id || '',
                        equipped_name_color: p.equipped_name_color || 'Default',
                        equipped_name_font: p.equipped_name_font || 'Default',
                        equipped_name_effect: p.equipped_name_effect || 'None',
                        equipped_name_animation: p.equipped_name_animation || 'None',
                        role: p.role || 0, current_level: p.current_level || 1
                    });
                }
            }
        });
        list.sort((a, b) => {
            const s = (b.current_streak_days || 0) - (a.current_streak_days || 0);
            return s !== 0 ? s : (b.total_streak_points || 0) - (a.total_streak_points || 0);
        });
        res.json({ list, seasonEnd });
    } catch (e) { console.error(e); res.status(500).json({ error: 'Err' }); }
});

// =====================
// SEASON CLAIM
// =====================
app.post('/season/claim', verifyHMAC, antiReplay, verifySessionToken, async (req, res) => {
    const accountID = req.verifiedAccountID;
    try {
        const pRef = db.ref(`players/${accountID}`);
        const snap = await pRef.once('value');
        if (!snap.exists()) return res.status(404).json({ error: "Not found" });
        const data = snap.val();
        if (data.season_reward_ts && Date.now() > (data.season_reward_ts + 86400000)) {
            await pRef.update({ pending_season_reward: null, season_reward_ts: null });
            return res.status(400).json({ error: "Expired" });
        }
        const rank = data.pending_season_reward;
        if (!rank) return res.status(400).json({ error: "No reward" });
        const reward = SEASON_REWARDS[rank];
        if (!reward) return res.status(400).json({ error: "Invalid rank" });
        await pRef.update({
            gems: (data.gems || 0) + reward.gems,
            super_stars: (data.super_stars || 0) + reward.super_stars,
            star_tickets: (data.star_tickets || 0) + reward.star_tickets,
            pending_season_reward: null
        });
        res.json({ success: true, rank, reward });
    } catch (e) { console.error(e); res.status(500).json({ error: "Err" }); }
});

// =====================
// MAIL
// =====================
app.get('/players/:accountID/mail', verifyHMAC, verifySessionToken, async (req, res) => {
    const accountID = req.verifiedAccountID;
    try {
        const [pSnap, gSnap, cSnap] = await Promise.all([
            db.ref(`players/${accountID}/inbox`).once('value'),
            db.ref(`global_mails`).limitToLast(10).once('value'),
            db.ref(`players/${accountID}/claimed_global_mails`).once('value')
        ]);
        const inbox = [];
        const personal = pSnap.val() || {};
        const global = gSnap.val() || {};
        const claimed = cSnap.val() || {};
        Object.keys(personal).forEach(k => inbox.push({ id: k, ...personal[k], type: 'personal' }));
        Object.keys(global).forEach(k => { if (!claimed[k]) inbox.push({ id: k, ...global[k], type: 'global' }); });
        inbox.sort((a, b) => b.timestamp - a.timestamp);
        res.json(inbox);
    } catch (e) { res.status(500).json({ error: "Err" }); }
});

app.post('/players/:accountID/mail/:mailID/claim', verifyHMAC, antiReplay, verifySessionToken, async (req, res) => {
    const accountID = req.verifiedAccountID;
    const { mailID } = req.params;
    const { type } = req.body;
    try {
        const playerRef = db.ref(`players/${accountID}`);
        const pSnap = await playerRef.once('value');
        const pData = pSnap.val();
        let mailData, mailRef;
        if (type === 'global') {
            if (pData.claimed_global_mails && pData.claimed_global_mails[mailID]) return res.status(400).json({ error: "Claimed" });
            const gSnap = await db.ref(`global_mails/${mailID}`).once('value');
            mailData = gSnap.val();
        } else {
            mailRef = db.ref(`players/${accountID}/inbox/${mailID}`);
            const mSnap = await mailRef.once('value');
            mailData = mSnap.val();
        }
        if (!mailData) return res.status(404).json({ error: "404" });
        const rewards = mailData.rewards || {};
        const updates = {};
        if (rewards.super_stars) updates.super_stars = (pData.super_stars || 0) + rewards.super_stars;
        if (rewards.star_tickets) updates.star_tickets = (pData.star_tickets || 0) + rewards.star_tickets;
        if (rewards.gems) updates.gems = (pData.gems || 0) + rewards.gems;
        if (type === 'global') updates[`claimed_global_mails/${mailID}`] = true;
        else await mailRef.remove();
        if (Object.keys(updates).length > 0) await playerRef.update(updates);
        res.json({ success: true });
    } catch (e) { res.status(500).json({ error: "Err" }); }
});

// =====================
// ADMIN (HMAC + Session Token + rol verificado en DB)
// =====================
app.post('/admin/review-task', verifyHMAC, antiReplay, verifySessionToken, async (req, res) => {
    const callerID = req.verifiedAccountID;
    if (callerID !== SUPER_ADMIN_ID) {
        const modSnap = await db.ref(`players/${callerID}/role`).once('value');
        if (modSnap.val() !== 2) return res.status(403).json({ error: "Auth" });
    }
    const { targetAccountID, taskID, action } = req.body;
    try {
        await db.ref(`task_submissions/${taskID}/${targetAccountID}`).update({
            status: (action === 'approve' ? 'approved' : 'rejected')
        });
        res.json({ success: true });
    } catch (e) { res.status(500).json({ error: "Err" }); }
});

app.post('/send-mail', verifyHMAC, antiReplay, verifySessionToken, async (req, res) => {
    const callerID = req.verifiedAccountID;
    if (callerID !== SUPER_ADMIN_ID) return res.status(403).json({ error: "Auth" });
    const { targetID, title, body, rewards } = req.body;
    const m = {
        title: sanitizeString(title, 200), body: sanitizeString(body, 1000),
        rewards: rewards || {}, sender: callerID,
        timestamp: admin.database.ServerValue.TIMESTAMP
    };
    try {
        if (targetID === "global") await db.ref('global_mails').push(m);
        else await db.ref(`players/${targetID}/inbox`).push(m);
        res.json({ success: true });
    } catch (e) { res.status(500).json({ error: "Err" }); }
});

// =====================
// BAN
// =====================
app.post('/ban-action', verifyHMAC, antiReplay, verifySessionToken, async (req, res) => {
    const callerID = req.verifiedAccountID;
    const targetInput = req.body.targetAccountID || req.body.targetStreakID || req.body.targetID;
    const { reason, action } = req.body;
    if (!targetInput) return res.status(400).json({ error: "Data missing" });
    try {
        let role = 0;
        if (callerID === SUPER_ADMIN_ID) role = 2;
        else {
            const modSnap = await db.ref(`players/${callerID}/role`).once('value');
            const r = modSnap.val();
            if (r === 2 || r === 'admin') role = 2;
            else if (r === 1 || r === 'mod') role = 1;
        }
        if (role < 1) return res.status(403).json({ error: "Perms" });

        let targetAccountID = null;
        const inputStr = String(targetInput).trim();
        if (isNaN(inputStr) || inputStr.includes('-')) {
            const searchSnap = await db.ref('players').orderByChild('streakID').equalTo(inputStr).limitToFirst(1).once('value');
            if (!searchSnap.exists()) return res.status(404).json({ error: "Not found" });
            targetAccountID = Object.keys(searchSnap.val())[0];
        } else {
            targetAccountID = parseInt(inputStr);
            const check = await db.ref(`players/${targetAccountID}`).once('value');
            if (!check.exists()) return res.status(404).json({ error: "Not found" });
        }
        if (parseInt(targetAccountID) === SUPER_ADMIN_ID) return res.status(403).json({ error: "Nope" });
        const targetRef = db.ref(`players/${targetAccountID}`);
        if (action === 'ban') {
            await targetRef.update({ ban: true, ban_reason: sanitizeString(reason || "No reason", 200), banned_by: callerID });
        } else {
            await targetRef.update({ ban: null, ban_reason: null, banned_by: null });
        }
        res.json({ success: true, target: targetAccountID });
    } catch (e) { console.error(e); res.status(500).json({ error: "Err" }); }
});

// =====================
// CODES
// =====================
app.post('/create-code', verifyHMAC, antiReplay, verifySessionToken, async (req, res) => {
    const callerID = req.verifiedAccountID;
    const { codeName, maxUses, rewards } = req.body;
    const stdCode = codeName ? codeName.toUpperCase().trim().replace(/ /g, '-') : '';
    try {
        let role = 0;
        if (callerID === SUPER_ADMIN_ID) role = 2;
        else {
            const modSnap = await db.ref(`players/${callerID}/role`).once('value');
            if (modSnap.val() === 2 || modSnap.val() === 'admin') role = 2;
        }
        if (role < 2) return res.status(403).json({ error: "Admin" });
        await db.ref(`codes/${stdCode}`).set({
            createdBy: callerID, isActive: true,
            usesLeft: parseInt(maxUses), maxUses: parseInt(maxUses), rewards
        });
        res.json({ success: true });
    } catch (e) { res.status(500).json({ error: "Err" }); }
});

app.post('/my-codes', verifyHMAC, verifySessionToken, async (req, res) => {
    const accountID = req.verifiedAccountID;
    try {
        const snap = await db.ref('codes').orderByChild('createdBy').equalTo(accountID).once('value');
        const list = [];
        snap.forEach(c => {
            const v = c.val();
            list.push({ name: c.key, used: v.maxUses - v.usesLeft, max: v.maxUses, active: v.isActive });
        });
        res.json(list);
    } catch (e) { res.status(500).json({ error: "Err" }); }
});

// =====================
// MESSAGES
// =====================
app.post('/messages', verifyHMAC, antiReplay, verifySessionToken, async (req, res) => {
    const accountID = req.verifiedAccountID;
    const { content, username } = req.body;
    if (!content) return res.status(400).json({ error: "Inv" });
    try {
        const uSnap = await db.ref(`players/${accountID}`).once('value');
        const user = uSnap.val();
        let role = 0;
        if (accountID === SUPER_ADMIN_ID) role = 2;
        else {
            const r = user.role;
            if (r === 2 || r === 'admin') role = 2;
            else if (r === 1 || r === 'mod') role = 1;
        }
        if (role < 1) return res.status(403).json({ error: "Perms" });
        if (role === 1) {
            const today = new Date().toISOString().split('T')[0];
            if (user.last_msg_date !== today) {
                await db.ref(`players/${accountID}`).update({ daily_msg_count: 0, last_msg_date: today });
                user.daily_msg_count = 0;
            }
            if ((user.daily_msg_count || 0) >= 3) return res.status(429).json({ error: "Limit" });
            await db.ref(`players/${accountID}/daily_msg_count`).set((user.daily_msg_count || 0) + 1);
        }
        await db.ref('messages').push({
            accountID, username: sanitizeString(username || "Unk", 50),
            role, content: sanitizeString(content.trim(), 500),
            timestamp: admin.database.ServerValue.TIMESTAMP
        });
        res.json({ success: true });
    } catch (e) { console.error(e); res.status(500).json({ error: "Err" }); }
});

app.get('/messages', async (req, res) => {
    try {
        const snap = await db.ref('messages').orderByChild('timestamp').limitToLast(20).once('value');
        const msgs = [];
        snap.forEach(c => { let m = c.val(); m.id = c.key; msgs.push(m); });
        res.json(msgs);
    } catch (e) { res.status(500).json({ error: "Err" }); }
});

app.post('/messages/:id/like', verifyHMAC, antiReplay, verifySessionToken, async (req, res) => {
    const { id } = req.params;
    const accountID = req.verifiedAccountID;
    if (!id) return res.status(400).json({ error: "Inv" });
    try {
        const msgRef = db.ref(`messages/${id}`);
        const snap = await msgRef.once('value');
        if (!snap.exists()) return res.status(404).json({ error: "404" });
        const msg = snap.val();
        let likedBy = msg.likedBy || {};
        if (likedBy[accountID]) return res.status(409).json({ error: "Already liked" });
        let likesCount = (msg.likesCount || 0) + 1;
        likedBy[accountID] = true;
        await msgRef.update({ likesCount, likedBy });
        res.json({ success: true, likesCount, liked: true });
    } catch (e) { console.error(e); res.status(500).json({ error: "Err" }); }
});

// =====================
// TRENDING
// =====================
app.get('/trending-levels/:accountID', verifyHMAC, verifySessionToken, async (req, res) => {
    const accountID = req.verifiedAccountID;
    try {
        const snap = await db.ref('trending_levels').once('value');
        const levels = snap.val() || {};
        const claimedSnap = await db.ref(`players/${accountID}/claimed_trends`).once('value');
        const claimed = claimedSnap.val() || {};
        const result = [];
        Object.keys(levels).forEach(key => {
            if (levels[key].isActive !== false) {
                result.push({
                    id: key, levelID: levels[key].levelID,
                    name: levels[key].name || "Unknown Level",
                    creator: levels[key].creator || "-",
                    rewards: levels[key].rewards || {},
                    isClaimed: !!claimed[key]
                });
            }
        });
        res.json(result);
    } catch (e) { console.error(e); res.status(500).json({ error: "Err" }); }
});

app.get('/trending-level/:accountID', verifyHMAC, verifySessionToken, async (req, res) => {
    const accountID = req.verifiedAccountID;
    try {
        const snap = await db.ref('trending_level').once('value');
        const levelData = snap.val();
        if (!levelData) return res.json(null);
        if (!(levelData.isActive === true || levelData.isActive === "true")) return res.json(null);
        const claimedSnap = await db.ref(`players/${accountID}/claimed_trending_levels/${levelData.levelID}`).once('value');
        res.json({
            levelID: levelData.levelID || 0, name: levelData.name || "Unknown",
            creator: levelData.creator || "-",
            difficultySprite: levelData.difficultySprite || "difficulty_00_btn_001.png",
            rewards: levelData.rewards || {},
            isClaimed: claimedSnap.exists() && claimedSnap.val() === true
        });
    } catch (e) { console.error(e); res.status(500).json({ error: "Err" }); }
});

app.post('/trending-level/claim', verifyHMAC, antiReplay, verifySessionToken, async (req, res) => {
    const accountID = req.verifiedAccountID;
    const { levelID } = req.body;
    try {
        const [pSnap, tSnap] = await Promise.all([
            db.ref(`players/${accountID}`).once('value'),
            db.ref(`trending_level`).once('value')
        ]);
        if (!pSnap.exists() || !tSnap.exists()) return res.status(404).json({ error: "404" });
        const pData = pSnap.val();
        const tData = tSnap.val();
        if (tData.levelID !== levelID) return res.status(400).json({ error: "Mismatch" });
        if (pData.claimed_trending_levels && pData.claimed_trending_levels[levelID]) {
            return res.status(409).json({ error: "Already claimed" });
        }
        const rewards = tData.rewards || {};
        const updates = {};
        if (rewards.super_stars) updates['super_stars'] = (pData.super_stars || 0) + rewards.super_stars;
        if (rewards.star_tickets) updates['star_tickets'] = (pData.star_tickets || 0) + rewards.star_tickets;
        if (rewards.gems) updates['gems'] = (pData.gems || 0) + rewards.gems;
        updates[`claimed_trending_levels/${levelID}`] = true;
        await db.ref(`players/${accountID}`).update(updates);
        res.json({ success: true, rewards });
    } catch (e) { console.error(e); res.status(500).json({ error: "Err" }); }
});

app.get('/admin/setup-trend', verifyHMAC, verifySessionToken, async (req, res) => {
    if (req.verifiedAccountID !== SUPER_ADMIN_ID) return res.status(403).json({ error: "Auth" });
    try {
        await db.ref('trending_level').set({
            levelID: 127171478, name: "Broken Gameplay", creator: "CloudEngineer",
            isActive: true, rewards: { super_stars: 100, star_tickets: 500 }
        });
        res.send("OK");
    } catch (e) { res.status(500).send("Error: " + e.message); }
});

// =====================
// ROULETTE PRIZE TABLES (mirror del cliente)
// =====================
const STANDARD_ROULETTE_PRIZES = [
    { type: 'badge', id: 'badge_pixel2', quantity: 1, weight: 1, category: 'mythic' },
    { type: 'badge', id: 'badge_cherry', quantity: 1, weight: 3, category: 'legendary' },
    { type: 'badge', id: 'badge_cherry6', quantity: 1, weight: 5, category: 'epic' },
    { type: 'badge', id: 'badge_mc2', quantity: 1, weight: 10, category: 'special' },
    { type: 'badge', id: 'omega_badge', quantity: 1, weight: 70, category: 'common' },
    { type: 'star_ticket', id: 'star_tiket_0', quantity: 1, weight: 70, category: 'common' },
    { type: 'star_ticket', id: 'star_tiket_1', quantity: 10, weight: 70, category: 'common' },
    { type: 'star_ticket', id: 'star_tiket_3', quantity: 3, weight: 70, category: 'common' },
    { type: 'star_ticket', id: 'star_ticket_15', quantity: 15, weight: 70, category: 'common' },
    { type: 'star_ticket', id: 'star_ticket_30', quantity: 30, weight: 10, category: 'special' },
    { type: 'star_ticket', id: 'star_ticket_600', quantity: 600, weight: 5, category: 'epic' },
    { type: 'badge', id: 'badge_cherry10', quantity: 1, weight: 3, category: 'legendary' }
];

const GEM_ROULETTE_PRIZES = [
    { type: 'banner', id: 'banner_54', quantity: 1, weight: 5, category: 'mythic' },
    { type: 'badge', id: 'alan_walker_badge', quantity: 1, weight: 15, category: 'legendary' },
    { type: 'badge', id: 'teto_badge', quantity: 1, weight: 30, category: 'epic' },
    { type: 'super_star', id: 'super_star50', quantity: 50, weight: 45, category: 'special' },
    { type: 'star_ticket', id: 'star_ticket7k', quantity: 7000, weight: 45, category: 'special' },
    { type: 'super_star', id: 'super_star5', quantity: 5, weight: 60, category: 'common' },
    { type: 'star_ticket', id: 'star_ticket1500', quantity: 1500, weight: 60, category: 'common' }
];

const GEM_SPIN_COSTS = [9, 19, 49, 199, 249, 399, 459];
const BADGE_TICKET_VALUES = { common: 5, special: 20, epic: 50, legendary: 100, mythic: 500 };

// Type codes MUST match the client's RewardType enum order
// (Badge=0, SuperStar=1, StarTicket=2, Banner=3) — see src/popups/StreakCommon.h.
// If these diverge the server resets the gem-roulette state every spin.
function computeGemRouletteHash() {
    return GEM_ROULETTE_PRIZES.map(p => `${p.id}:${p.quantity}:${p.type === 'badge' ? 0 : p.type === 'super_star' ? 1 : p.type === 'star_ticket' ? 2 : 3}`).join(';') + ';';
}

function pickWeightedIndex(prizes, excludeIndices = []) {
    let totalWeight = 0;
    for (let i = 0; i < prizes.length; i++) {
        if (!excludeIndices.includes(i)) totalWeight += prizes[i].weight;
    }
    if (totalWeight <= 0) return -1;
    let rnd = Math.random() * totalWeight;
    for (let i = 0; i < prizes.length; i++) {
        if (excludeIndices.includes(i)) continue;
        if (rnd < prizes[i].weight) return i;
        rnd -= prizes[i].weight;
    }
    // fallback
    for (let i = prizes.length - 1; i >= 0; i--) {
        if (!excludeIndices.includes(i)) return i;
    }
    return 0;
}

// =====================
// STANDARD ROULETTE SPIN (1 o 10 spins)
// =====================
app.post('/roulette/spin', verifyHMAC, antiReplay, verifySessionToken, async (req, res) => {
    const accountID = req.verifiedAccountID;
    const spins = parseInt(req.body.spins) || 1;
    if (spins !== 1 && spins !== 10) return res.status(400).json({ error: "Invalid spins" });

    try {
        const pRef = db.ref(`players/${accountID}`);
        const snap = await pRef.once('value');
        if (!snap.exists()) return res.status(404).json({ error: "Not found" });
        const pData = snap.val();
        if (pData.ban) return res.status(403).json({ error: "Banned" });

        const cost = spins;
        if ((pData.super_stars || 0) < cost) return res.status(402).json({ error: "Not enough super stars" });

        pData.super_stars = (pData.super_stars || 0) - cost;
        pData.total_spins = (pData.total_spins || 0) + spins;

        const unlockedBadges = Array.isArray(pData.unlocked_badges) ? [...pData.unlocked_badges] : [];
        const results = [];

        for (let s = 0; s < spins; s++) {
            const winIdx = pickWeightedIndex(STANDARD_ROULETTE_PRIZES);
            const prize = STANDARD_ROULETTE_PRIZES[winIdx];
            const result = {
                winningIndex: winIdx, type: prize.type, id: prize.id,
                quantity: prize.quantity, category: prize.category,
                isNewBadge: false, ticketsFromDuplicate: 0
            };

            if (prize.type === 'badge') {
                if (!unlockedBadges.includes(prize.id)) {
                    unlockedBadges.push(prize.id);
                    result.isNewBadge = true;
                } else {
                    const dupTickets = BADGE_TICKET_VALUES[prize.category] || 5;
                    pData.star_tickets = (pData.star_tickets || 0) + dupTickets;
                    result.ticketsFromDuplicate = dupTickets;
                }
            } else if (prize.type === 'super_star') {
                pData.super_stars += prize.quantity;
            } else if (prize.type === 'star_ticket') {
                pData.star_tickets = (pData.star_tickets || 0) + prize.quantity;
            }
            results.push(result);
        }

        await pRef.update({
            super_stars: pData.super_stars,
            star_tickets: pData.star_tickets || 0,
            total_spins: pData.total_spins,
            unlocked_badges: unlockedBadges
        });

        res.json({
            success: true, results,
            balances: { super_stars: pData.super_stars, star_tickets: pData.star_tickets || 0, gems: pData.gems || 0 },
            totalSpins: pData.total_spins,
            unlocked_badges: unlockedBadges
        });
    } catch (e) {
        console.error("Roulette spin error:", e);
        res.status(500).json({ error: "Err" });
    }
});

// =====================
// GEM ROULETTE SPIN
// =====================
app.post('/gem-roulette/spin', verifyHMAC, antiReplay, verifySessionToken, async (req, res) => {
    const accountID = req.verifiedAccountID;

    try {
        const pRef = db.ref(`players/${accountID}`);
        const snap = await pRef.once('value');
        if (!snap.exists()) return res.status(404).json({ error: "Not found" });
        const pData = snap.val();
        if (pData.ban) return res.status(403).json({ error: "Banned" });

        // Verificar hash: si los premios cambiaron, resetear estado
        const currentHash = computeGemRouletteHash();
        if (pData.gem_roulette_hash !== currentHash) {
            pData.gem_roulette_spin_count = 0;
            pData.gem_roulette_state = [false, false, false, false, false, false, false];
            pData.gem_roulette_hash = currentHash;
        }

        let gemState = Array.isArray(pData.gem_roulette_state)
            ? [...pData.gem_roulette_state] : [false, false, false, false, false, false, false];
        while (gemState.length < 7) gemState.push(false);
        let spinCount = pData.gem_roulette_spin_count || 0;
        const unlockedBadges = Array.isArray(pData.unlocked_badges) ? [...pData.unlocked_badges] : [];
        const unlockedBanners = Array.isArray(pData.unlocked_banners) ? [...pData.unlocked_banners] : [];

        // Marcar items ya desbloqueados como claimed
        for (let i = 0; i < GEM_ROULETTE_PRIZES.length && i < 7; i++) {
            const p = GEM_ROULETTE_PRIZES[i];
            if (p.type === 'badge' && unlockedBadges.includes(p.id)) gemState[i] = true;
            if (p.type === 'banner' && unlockedBanners.includes(p.id)) gemState[i] = true;
        }

        // Indices disponibles
        const excluded = [];
        for (let i = 0; i < 7; i++) { if (gemState[i]) excluded.push(i); }
        if (excluded.length >= 7) return res.status(400).json({ error: "All claimed" });

        // Costo escalonado
        const costIdx = Math.min(spinCount, GEM_SPIN_COSTS.length - 1);
        const cost = GEM_SPIN_COSTS[Math.max(0, costIdx)];
        if ((pData.gems || 0) < cost) return res.status(402).json({ error: "Not enough gems" });

        pData.gems = (pData.gems || 0) - cost;
        pData.total_spins = (pData.total_spins || 0) + 1;
        spinCount++;

        const winIdx = pickWeightedIndex(GEM_ROULETTE_PRIZES, excluded);
        if (winIdx < 0) return res.status(400).json({ error: "No prizes" });

        gemState[winIdx] = true;
        const prize = GEM_ROULETTE_PRIZES[winIdx];
        const result = { winningIndex: winIdx, type: prize.type, id: prize.id, quantity: prize.quantity, category: prize.category };

        if (prize.type === 'badge' && !unlockedBadges.includes(prize.id)) unlockedBadges.push(prize.id);
        else if (prize.type === 'banner' && !unlockedBanners.includes(prize.id)) unlockedBanners.push(prize.id);
        else if (prize.type === 'super_star') pData.super_stars = (pData.super_stars || 0) + prize.quantity;
        else if (prize.type === 'star_ticket') pData.star_tickets = (pData.star_tickets || 0) + prize.quantity;

        await pRef.update({
            gems: pData.gems, super_stars: pData.super_stars || 0, star_tickets: pData.star_tickets || 0,
            total_spins: pData.total_spins, gem_roulette_state: gemState,
            gem_roulette_spin_count: spinCount, gem_roulette_hash: currentHash,
            unlocked_badges: unlockedBadges, unlocked_banners: unlockedBanners
        });

        res.json({
            success: true, result,
            balances: { super_stars: pData.super_stars || 0, star_tickets: pData.star_tickets || 0, gems: pData.gems },
            totalSpins: pData.total_spins, gemRouletteState: gemState,
            gemRouletteSpinCount: spinCount,
            unlocked_badges: unlockedBadges, unlocked_banners: unlockedBanners
        });
    } catch (e) {
        console.error("Gem roulette error:", e);
        res.status(500).json({ error: "Err" });
    }
});


// =============================================
// SHOP CATALOG (tickets) - mirror del cliente
// =============================================
const TICKET_SHOP_PRICES = {
    "diamante_mc_badge": 550, "platino_streak_badge": 320, "diamante_gd_badge": 700,
    "hounter_badge": 600, "money_badge": 500, "gd_badge": 200, "Steampunk_Dash_badge": 150,
    "dark_streak_badge": 500, "gold_streak_badge": 1200,
    "bh_badge_1": 29990, "bh_badge_2": 1300, "bh_badge_3": 520, "bh_badge_4": 990,
    "bh_badge_5": 11990, "bh_badge_6": 1190, "bh_badge_7": 45990,
    "mc_badge_1": 3999, "mc_badge_2": 1699, "mc_badge_3": 199, "mai_badge": 1200,
    "colombia_badge": 1400, "usa_badge": 1400, "venezuela_badge": 1400, "paraguay_badge": 1400,
    "filipinas_badge": 1400, "mexico_badge": 1400, "spain_badge": 1400, "arg_badge": 1400,
    "nzl_badge": 1400, "bra_badge": 1400, "eng_badge": 1400, "peru_badge": 1400,
    "fra_badge": 1400, "rus_badge": 1400,
    "banner_1": 5600, "banner_2": 8000, "banner_9": 29500,
    "banner_22": 120500, "banner_7": 67500, "banner_11": 72300
};
const TICKET_SHOP_BANNERS = new Set(["banner_1","banner_2","banner_9","banner_22","banner_7","banner_11"]);

// =============================================
// STREAK GOALS - mirror del cliente
// =============================================
const STREAK_GOALS = [
    { pointsRequired: 500, levelRequired: 5, rewardTickets: 1000, rewardBannerID: "", rewardBadgeID: "" },
    { pointsRequired: 1500, levelRequired: 5, rewardTickets: 0, rewardBannerID: "banner_28", rewardBadgeID: "" },
    { pointsRequired: 5000, levelRequired: 10, rewardTickets: 0, rewardBannerID: "", rewardBadgeID: "diamond_streak_badge" },
    { pointsRequired: 9000, levelRequired: 20, rewardTickets: 25000, rewardBannerID: "", rewardBadgeID: "" },
    { pointsRequired: 15000, levelRequired: 30, rewardTickets: 0, rewardBannerID: "banner_29", rewardBadgeID: "" },
    { pointsRequired: 20000, levelRequired: 40, rewardTickets: 0, rewardBannerID: "banner_42", rewardBadgeID: "" },
    { pointsRequired: 30000, levelRequired: 50, rewardTickets: 0, rewardBannerID: "banner_44", rewardBadgeID: "" }
];

// =============================================
// SHOP PURCHASE (tickets → badges/banners)
// =============================================
app.post('/shop/purchase', verifyHMAC, antiReplay, verifySessionToken, async (req, res) => {
    const accountID = req.verifiedAccountID;
    const { itemID } = req.body;
    if (!itemID || !TICKET_SHOP_PRICES[itemID]) return res.status(400).json({ error: "Invalid item" });

    try {
        const pRef = db.ref(`players/${accountID}`);
        const snap = await pRef.once('value');
        if (!snap.exists()) return res.status(404).json({ error: "Not found" });
        const pData = snap.val();
        if (pData.ban) return res.status(403).json({ error: "Banned" });

        const price = TICKET_SHOP_PRICES[itemID];
        if ((pData.star_tickets || 0) < price) return res.status(402).json({ error: "Not enough tickets" });

        const isBanner = TICKET_SHOP_BANNERS.has(itemID);
        const field = isBanner ? 'unlocked_banners' : 'unlocked_badges';
        const arr = Array.isArray(pData[field]) ? [...pData[field]] : [];
        if (arr.includes(itemID)) return res.status(409).json({ error: "Already owned" });

        arr.push(itemID);
        const updates = { star_tickets: (pData.star_tickets || 0) - price, [field]: arr };
        await pRef.update(updates);

        res.json({ success: true, balances: {
            super_stars: pData.super_stars || 0,
            star_tickets: updates.star_tickets,
            gems: pData.gems || 0
        }});
    } catch (e) { console.error(e); res.status(500).json({ error: "Err" }); }
});

// =============================================
// DAILY SHOP PURCHASE - cosmetics (gems → badge/banner)
// =============================================
app.post('/daily-shop/purchase', verifyHMAC, antiReplay, verifySessionToken, async (req, res) => {
    const accountID = req.verifiedAccountID;
    const { itemID, price, isBadge } = req.body;
    if (!itemID || !price || price <= 0 || price > 50000) return res.status(400).json({ error: "Invalid" });

    try {
        const pRef = db.ref(`players/${accountID}`);
        const snap = await pRef.once('value');
        if (!snap.exists()) return res.status(404).json({ error: "Not found" });
        const pData = snap.val();
        if (pData.ban) return res.status(403).json({ error: "Banned" });
        if ((pData.gems || 0) < price) return res.status(402).json({ error: "Not enough gems" });

        const field = isBadge ? 'unlocked_badges' : 'unlocked_banners';
        const arr = Array.isArray(pData[field]) ? [...pData[field]] : [];
        if (arr.includes(itemID)) return res.status(409).json({ error: "Already owned" });

        arr.push(itemID);
        const updates = { gems: (pData.gems || 0) - price, [field]: arr };
        await pRef.update(updates);

        res.json({ success: true, balances: {
            super_stars: pData.super_stars || 0,
            star_tickets: pData.star_tickets || 0,
            gems: updates.gems
        }});
    } catch (e) { console.error(e); res.status(500).json({ error: "Err" }); }
});

// =============================================
// NAME ITEM PURCHASE (gems → name effect/animation/color/font)
// =============================================
app.post('/name-item/purchase', verifyHMAC, antiReplay, verifySessionToken, async (req, res) => {
    const accountID = req.verifiedAccountID;
    const { itemID } = req.body;
    if (!itemID || typeof itemID !== 'string' || itemID.length > 64) {
        return res.status(400).json({ error: "Invalid item" });
    }

    const price = getNameItemPrice(itemID);
    if (price <= 0) return res.status(400).json({ error: "Not purchasable" });

    try {
        const pRef = db.ref(`players/${accountID}`);
        const snap = await pRef.once('value');
        if (!snap.exists()) return res.status(404).json({ error: "Not found" });
        const pData = snap.val();
        if (pData.ban) return res.status(403).json({ error: "Banned" });
        if ((pData.gems || 0) < price) return res.status(402).json({ error: "Not enough gems" });

        const owned = Array.isArray(pData.unlocked_name_items) ? [...pData.unlocked_name_items] : [];
        if (owned.includes(itemID)) return res.status(409).json({ error: "Already owned" });

        owned.push(itemID);
        const newGems = (pData.gems || 0) - price;
        await pRef.update({ gems: newGems, unlocked_name_items: owned });

        res.json({
            success: true,
            balances: {
                super_stars: pData.super_stars || 0,
                star_tickets: pData.star_tickets || 0,
                gems: newGems
            },
            unlocked_name_items: owned
        });
    } catch (e) { console.error(e); res.status(500).json({ error: "Err" }); }
});

// =============================================
// DAILY SHOP PURCHASE - consumables (gems → tickets/stars)
// =============================================
app.post('/daily-shop/purchase-consumable', verifyHMAC, antiReplay, verifySessionToken, async (req, res) => {
    const accountID = req.verifiedAccountID;
    const { price, amount, isTickets } = req.body;
    if (!price || price <= 0 || price > 50000 || !amount || amount <= 0) return res.status(400).json({ error: "Invalid" });

    try {
        const pRef = db.ref(`players/${accountID}`);
        const snap = await pRef.once('value');
        if (!snap.exists()) return res.status(404).json({ error: "Not found" });
        const pData = snap.val();
        if (pData.ban) return res.status(403).json({ error: "Banned" });
        if ((pData.gems || 0) < price) return res.status(402).json({ error: "Not enough gems" });

        const updates = { gems: (pData.gems || 0) - price };
        if (isTickets) updates.star_tickets = (pData.star_tickets || 0) + amount;
        else updates.super_stars = (pData.super_stars || 0) + amount;
        await pRef.update(updates);

        res.json({ success: true, balances: {
            super_stars: updates.super_stars ?? (pData.super_stars || 0),
            star_tickets: updates.star_tickets ?? (pData.star_tickets || 0),
            gems: updates.gems
        }});
    } catch (e) { console.error(e); res.status(500).json({ error: "Err" }); }
});

// =============================================
// DISCORD MILESTONE CLAIM
// =============================================
app.post('/discord-milestone/claim', verifyHMAC, antiReplay, verifySessionToken, async (req, res) => {
    const accountID = req.verifiedAccountID;
    const { requirement, tickets, stars, gems, isChest } = req.body;
    if (!requirement) return res.status(400).json({ error: "Invalid" });

    try {
        const pRef = db.ref(`players/${accountID}`);
        const snap = await pRef.once('value');
        if (!snap.exists()) return res.status(404).json({ error: "Not found" });
        const pData = snap.val();
        if (pData.ban) return res.status(403).json({ error: "Banned" });

        const claimed = pData.claimed_discord_milestones || [];
        if (claimed.includes(requirement)) return res.status(409).json({ error: "Already claimed" });

        const updates = {};
        claimed.push(requirement);
        updates.claimed_discord_milestones = claimed;

        if (!isChest) {
            if (tickets > 0) updates.star_tickets = (pData.star_tickets || 0) + tickets;
            if (stars > 0) updates.super_stars = (pData.super_stars || 0) + stars;
            if (gems > 0) updates.gems = (pData.gems || 0) + gems;
        }
        await pRef.update(updates);

        res.json({ success: true, balances: {
            super_stars: updates.super_stars ?? (pData.super_stars || 0),
            star_tickets: updates.star_tickets ?? (pData.star_tickets || 0),
            gems: updates.gems ?? (pData.gems || 0)
        }});
    } catch (e) { console.error(e); res.status(500).json({ error: "Err" }); }
});

// =============================================
// STREAK GOAL CLAIM
// =============================================
app.post('/streak-goal/claim', verifyHMAC, antiReplay, verifySessionToken, async (req, res) => {
    const accountID = req.verifiedAccountID;
    const { goalIndex } = req.body;
    if (goalIndex < 0 || goalIndex >= STREAK_GOALS.length) return res.status(400).json({ error: "Invalid" });

    try {
        const pRef = db.ref(`players/${accountID}`);
        const snap = await pRef.once('value');
        if (!snap.exists()) return res.status(404).json({ error: "Not found" });
        const pData = snap.val();
        if (pData.ban) return res.status(403).json({ error: "Banned" });

        const goal = STREAK_GOALS[goalIndex];
        if ((pData.current_level || 1) < goal.levelRequired) return res.status(403).json({ error: "Level too low" });
        if ((pData.total_streak_points || 0) < goal.pointsRequired) return res.status(403).json({ error: "Not enough points" });

        const claimed = Array.isArray(pData.claimed_streak_goals) ? [...pData.claimed_streak_goals] : [];
        if (claimed.includes(goalIndex)) return res.status(409).json({ error: "Already claimed" });

        claimed.push(goalIndex);
        const updates = { claimed_streak_goals: claimed };

        if (goal.rewardTickets > 0) updates.star_tickets = (pData.star_tickets || 0) + goal.rewardTickets;
        if (goal.rewardBadgeID) {
            const badges = Array.isArray(pData.unlocked_badges) ? [...pData.unlocked_badges] : [];
            if (!badges.includes(goal.rewardBadgeID)) { badges.push(goal.rewardBadgeID); updates.unlocked_badges = badges; }
        }
        if (goal.rewardBannerID) {
            const banners = Array.isArray(pData.unlocked_banners) ? [...pData.unlocked_banners] : [];
            if (!banners.includes(goal.rewardBannerID)) { banners.push(goal.rewardBannerID); updates.unlocked_banners = banners; }
        }
        await pRef.update(updates);

        res.json({ success: true, balances: {
            super_stars: pData.super_stars || 0,
            star_tickets: updates.star_tickets ?? (pData.star_tickets || 0),
            gems: pData.gems || 0
        }});
    } catch (e) { console.error(e); res.status(500).json({ error: "Err" }); }
});

// =============================================
// LEVEL MISSION CLAIM
// =============================================
app.post('/level-mission/claim', verifyHMAC, antiReplay, verifySessionToken, async (req, res) => {
    const accountID = req.verifiedAccountID;
    const { levelID, rewardBadgeID, secondaryType, secondaryAmount } = req.body;
    if (!levelID) return res.status(400).json({ error: "Invalid" });

    try {
        const pRef = db.ref(`players/${accountID}`);
        const snap = await pRef.once('value');
        if (!snap.exists()) return res.status(404).json({ error: "Not found" });
        const pData = snap.val();
        if (pData.ban) return res.status(403).json({ error: "Banned" });

        const completed = pData.completedLevelMissions || {};
        if (completed[String(levelID)]) return res.status(409).json({ error: "Already claimed" });

        const updates = { [`completedLevelMissions/${levelID}`]: true };

        if (rewardBadgeID) {
            const badges = Array.isArray(pData.unlocked_badges) ? [...pData.unlocked_badges] : [];
            if (!badges.includes(rewardBadgeID)) { badges.push(rewardBadgeID); updates.unlocked_badges = badges; }
        }
        const amt = parseInt(secondaryAmount) || 0;
        if (secondaryType === 'super_stars' && amt > 0) updates.super_stars = (pData.super_stars || 0) + amt;
        if (secondaryType === 'star_tickets' && amt > 0) updates.star_tickets = (pData.star_tickets || 0) + amt;

        await pRef.update(updates);

        res.json({ success: true, balances: {
            super_stars: updates.super_stars ?? (pData.super_stars || 0),
            star_tickets: updates.star_tickets ?? (pData.star_tickets || 0),
            gems: pData.gems || 0
        }});
    } catch (e) { console.error(e); res.status(500).json({ error: "Err" }); }
});

// =============================================
// TASK REWARD CLAIM
// =============================================
app.post('/task/claim', verifyHMAC, antiReplay, verifySessionToken, async (req, res) => {
    const accountID = req.verifiedAccountID;
    const { taskID, rewardStars, badgeID, bannerID } = req.body;
    if (!taskID) return res.status(400).json({ error: "Invalid" });

    try {
        const pRef = db.ref(`players/${accountID}`);
        const snap = await pRef.once('value');
        if (!snap.exists()) return res.status(404).json({ error: "Not found" });
        const pData = snap.val();
        if (pData.ban) return res.status(403).json({ error: "Banned" });

        // Verificar que la task fue aprobada
        const subSnap = await db.ref(`task_submissions/${taskID}/${accountID}/status`).once('value');
        if (subSnap.val() !== 'approved') return res.status(403).json({ error: "Not approved" });

        const updates = {};
        const stars = parseInt(rewardStars) || 0;
        if (stars > 0) {
            updates.super_stars = (pData.super_stars || 0) + stars;
            // XP
            const lvlRes = processXP(pData, stars);
            updates.current_xp = pData.current_xp;
            updates.current_level = pData.current_level;
            if (lvlRes.stars) updates.super_stars = (updates.super_stars || pData.super_stars || 0) + lvlRes.stars;
            if (lvlRes.tickets) updates.star_tickets = (pData.star_tickets || 0) + lvlRes.tickets;
        }
        if (badgeID) {
            const badges = Array.isArray(pData.unlocked_badges) ? [...pData.unlocked_badges] : [];
            if (!badges.includes(badgeID)) { badges.push(badgeID); updates.unlocked_badges = badges; }
        }
        if (bannerID) {
            const banners = Array.isArray(pData.unlocked_banners) ? [...pData.unlocked_banners] : [];
            if (!banners.includes(bannerID)) { banners.push(bannerID); updates.unlocked_banners = banners; }
        }
        await pRef.update(updates);

        res.json({ success: true, balances: {
            super_stars: updates.super_stars ?? (pData.super_stars || 0),
            star_tickets: updates.star_tickets ?? (pData.star_tickets || 0),
            gems: pData.gems || 0,
            current_xp: pData.current_xp, current_level: pData.current_level
        }});
    } catch (e) { console.error(e); res.status(500).json({ error: "Err" }); }
});

// =============================================
// CHEST REWARD (genérico — para StreakChestPopup)
// =============================================
app.post('/chest/claim', verifyHMAC, antiReplay, verifySessionToken, async (req, res) => {
    const accountID = req.verifiedAccountID;
    const { superStars, starTickets, gems, xp, context } = req.body;

    try {
        const pRef = db.ref(`players/${accountID}`);
        const snap = await pRef.once('value');
        if (!snap.exists()) return res.status(404).json({ error: "Not found" });
        const pData = snap.val();
        if (pData.ban) return res.status(403).json({ error: "Banned" });

        const updates = {};
        const ss = Math.min(parseInt(superStars) || 0, 1000);
        const st = Math.min(parseInt(starTickets) || 0, 50000);
        const gm = Math.min(parseInt(gems) || 0, 1000);
        const xpAmt = Math.min(parseInt(xp) || 0, 5000);

        if (ss > 0) updates.super_stars = (pData.super_stars || 0) + ss;
        if (st > 0) updates.star_tickets = (pData.star_tickets || 0) + st;
        if (gm > 0) updates.gems = (pData.gems || 0) + gm;
        if (xpAmt > 0) {
            processXP(pData, xpAmt);
            updates.current_xp = pData.current_xp;
            updates.current_level = pData.current_level;
        }
        if (Object.keys(updates).length > 0) await pRef.update(updates);

        res.json({ success: true, balances: {
            super_stars: updates.super_stars ?? (pData.super_stars || 0),
            star_tickets: updates.star_tickets ?? (pData.star_tickets || 0),
            gems: updates.gems ?? (pData.gems || 0),
            current_xp: pData.current_xp, current_level: pData.current_level
        }});
    } catch (e) { console.error(e); res.status(500).json({ error: "Err" }); }
});


app.listen(port, () => console.log(`Server running on port ${port}`));