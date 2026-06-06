#pragma once
#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/utils/async.hpp>

using namespace geode::prelude;

namespace StreakUpdate {
    inline bool s_checked = false;
    inline std::string s_latestVersion;
    inline std::string s_downloadUrl;
    inline std::string s_changelog;

    inline constexpr const char* AVAILABLE_VERSION_KEY = "update_available_version";

    inline bool isAvailable() {
        std::string saved = Mod::get()->getSavedValue<std::string>(AVAILABLE_VERSION_KEY, "");
        if (saved.empty()) return false;
        auto parsed = VersionInfo::parse(saved);
        if (!parsed.isOk()) return false;
        return parsed.unwrap() > Mod::get()->getVersion();
    }

    inline void markAvailable(const std::string& ver, const std::string& url, const std::string& changelog) {
        s_latestVersion = ver;
        s_downloadUrl = url;
        s_changelog = changelog;
        Mod::get()->setSavedValue<std::string>(AVAILABLE_VERSION_KEY, ver);
    }

    inline void clearAvailable() {
        Mod::get()->setSavedValue<std::string>(AVAILABLE_VERSION_KEY, std::string(""));
    }

    inline void checkOnce(std::function<void()> onUpdateFound) {
        if (s_checked) return;
        s_checked = true;

        static async::TaskHolder<web::WebResponse> task;
        auto req = web::WebRequest();
        req.timeout(std::chrono::seconds(15));
        task.spawn(
            req.get("https://streak-servidor.onrender.com/mod/version"),
            [onUpdateFound](web::WebResponse res) {
                if (!res.ok()) return;
                auto jsonRes = res.json();
                if (!jsonRes.isOk()) return;
                auto data = jsonRes.unwrap();
                std::string verStr = data["version"].asString().unwrapOr("");
                std::string url = data["url"].asString().unwrapOr("");
                std::string changelog = data["changelog"].asString().unwrapOr("");
                if (verStr.empty()) return;
                auto parsed = VersionInfo::parse(verStr);
                if (!parsed.isOk()) return;
                VersionInfo latest = parsed.unwrap();
                VersionInfo current = Mod::get()->getVersion();
                std::string pending = Mod::get()->getSavedValue<std::string>("pending_update_version", "");
                if (latest > current && pending != verStr) {
                    markAvailable(verStr, url, changelog);
                    if (onUpdateFound) onUpdateFound();
                } else {
                    clearAvailable();
                }
            }
        );
    }
}
