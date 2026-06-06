#pragma once
#include "StreakCommon.h"
#include "../UpdateState.h"
#include "../StatusSpinner.h"
#include "../utils/RoundedProgressBar.h"
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/MDTextArea.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/utils/general.hpp>
#include <Geode/utils/async.hpp>
#include <Geode/loader/Dirs.hpp>
#include <filesystem>
#include <fstream>
#include <system_error>

class UpdatePopup : public Popup {
protected:
    enum class State {
        Checking,
        UpToDate,
        UpdateAvailable,
        Downloading,
        Done,
        Error
    };

    State m_state = State::Checking;
    CCNode* m_content = nullptr;

    VersionInfo m_currentVer = Mod::get()->getVersion();
    VersionInfo m_latestVer  = Mod::get()->getVersion();
    std::string m_latestVerStr;
    std::string m_downloadUrl;
    std::string m_changelog;

    RoundedProgressBar* m_progressBar = nullptr;
    CCLabelBMFont*      m_progressLabel = nullptr;

    async::TaskHolder<web::WebResponse> m_versionTask;
    async::TaskHolder<web::WebResponse> m_downloadTask;

    static constexpr const char* PENDING_UPDATE_KEY = "pending_update_version";

    bool init() override {
        if (!Popup::init(330.f, 240.f, "geode.loader/GE_square03.png")) {
            return false;
        }

        this->setTitle("Streak! Updater");

        m_content = CCNode::create();
        m_content->setContentSize(m_mainLayer->getContentSize());
        m_content->setPosition(0.f, 0.f);
        m_mainLayer->addChild(m_content);

        this->checkForUpdate();
        return true;
    }

    void clearContent() {
        if (m_content) m_content->removeAllChildren();
    }

    CCSize contentSize() const {
        return m_mainLayer->getContentSize();
    }

    StatusSpinner* addStatusSpinner(float cx, float cy) {
        auto sp = StatusSpinner::create();
        if (!sp) return nullptr;
        sp->setPosition({ cx, cy });
        m_content->addChild(sp);
        return sp;
    }

    void showCheckingState() {
        m_state = State::Checking;
        clearContent();
        auto sz = contentSize();
        float cx = sz.width / 2.f;
        float cy = sz.height / 2.f;

        if (auto sp = addStatusSpinner(cx, cy + 15.f)) {
            sp->setLoading("Checking for updates...");
        }

        auto verLabel = CCLabelBMFont::create(
            fmt::format("Current version: {}", m_currentVer).c_str(),
            "chatFont.fnt"
        );
        verLabel->setScale(0.5f);
        verLabel->setOpacity(180);
        verLabel->setPosition({ cx, cy - 60.f });
        m_content->addChild(verLabel);
    }

    void showUpToDateState() {
        m_state = State::UpToDate;
        Mod::get()->setSavedValue<std::string>(PENDING_UPDATE_KEY, "");
        clearContent();
        auto sz = contentSize();
        float cx = sz.width / 2.f;
        float cy = sz.height / 2.f;

        auto check = CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
        if (check) {
            check->setScale(1.4f);
            check->setPosition({ cx, cy + 30.f });
            m_content->addChild(check);
        }

        auto label = CCLabelBMFont::create("You're up to date!", "bigFont.fnt");
        label->setScale(0.7f);
        label->setColor({ 120, 255, 120 });
        label->setPosition({ cx, cy - 15.f });
        m_content->addChild(label);

        auto verLabel = CCLabelBMFont::create(
            fmt::format("Version {}", m_currentVer).c_str(),
            "chatFont.fnt"
        );
        verLabel->setScale(0.55f);
        verLabel->setPosition({ cx, cy - 40.f });
        m_content->addChild(verLabel);

        auto closeSpr = ButtonSprite::create("Close", "goldFont.fnt", "GJ_button_04.png", 0.7f);
        auto closeBtn = CCMenuItemSpriteExtra::create(closeSpr, this, menu_selector(UpdatePopup::onCloseClicked));
        auto menu = CCMenu::createWithItem(closeBtn);
        menu->setPosition({ cx, 28.f });
        m_content->addChild(menu);
    }

    void showUpdateAvailableState() {
        m_state = State::UpdateAvailable;
        clearContent();
        auto sz = contentSize();
        float cx = sz.width / 2.f;

        auto title = CCLabelBMFont::create("Update Available!", "bigFont.fnt");
        title->setScale(0.7f);
        title->setColor({ 120, 220, 255 });
        title->setPosition({ cx, sz.height - 35.f });
        m_content->addChild(title);

        auto verLabel = CCLabelBMFont::create(
            fmt::format("{}  >  {}", m_currentVer, m_latestVer).c_str(),
            "goldFont.fnt"
        );
        verLabel->setScale(0.5f);
        verLabel->setPosition({ cx, sz.height - 60.f });
        m_content->addChild(verLabel);

        float boxW = sz.width - 40.f;
        float boxH = 95.f;
        float boxCY = 90.f;

        auto bg = CCScale9Sprite::create("square02_001.png");
        bg->setContentSize({ boxW, boxH });
        bg->setPosition({ cx, boxCY });
        bg->setOpacity(120);
        bg->setColor({ 0, 0, 0 });
        m_content->addChild(bg);

        std::string mdContent = m_changelog.empty() ? "*(no changelog)*" : m_changelog;
        auto md = MDTextArea::create(mdContent, { boxW - 14.f, boxH - 14.f });
        md->setPosition({ cx, boxCY });
        m_content->addChild(md);

        auto dlSpr = ButtonSprite::create("Download", "goldFont.fnt", "GJ_button_01.png", 0.7f);
        auto dlBtn = CCMenuItemSpriteExtra::create(dlSpr, this, menu_selector(UpdatePopup::onDownloadClicked));
        auto menu = CCMenu::createWithItem(dlBtn);
        menu->setPosition({ cx, 28.f });
        m_content->addChild(menu);
    }

    void showDownloadingState() {
        m_state = State::Downloading;
        m_progressBar = nullptr;
        m_progressLabel = nullptr;
        clearContent();
        auto sz = contentSize();
        float cx = sz.width / 2.f;
        float cy = sz.height / 2.f;

        auto label = CCLabelBMFont::create("Downloading update...", "bigFont.fnt");
        label->setScale(0.6f);
        label->setPosition({ cx, cy + 30.f });
        m_content->addChild(label);

        float barW = 220.f;
        float barH = 20.f;
        m_progressBar = RoundedProgressBar::create(barW, barH);
        m_progressBar->setPosition({ cx, cy });
        m_progressBar->setGradientColors({ 100, 220, 255 }, { 50, 130, 255 });
        m_progressBar->setBackgroundColor({ 30, 30, 45 });
        m_content->addChild(m_progressBar);

        m_progressLabel = CCLabelBMFont::create("0%", "bigFont.fnt");
        m_progressLabel->setScale(0.45f);
        m_progressLabel->setPosition({ cx, cy });
        m_content->addChild(m_progressLabel);

        auto sub = CCLabelBMFont::create("Please don't close GD", "chatFont.fnt");
        sub->setScale(0.5f);
        sub->setOpacity(180);
        sub->setPosition({ cx, cy - 35.f });
        m_content->addChild(sub);
    }

    void updateDownloadProgress(float pct) {
        float frac = std::clamp(pct / 100.f, 0.f, 1.f);
        if (m_progressBar)   m_progressBar->setProgress(frac);
        if (m_progressLabel) m_progressLabel->setString(fmt::format("{}%", (int)pct).c_str());
    }

    void showDoneState() {
        m_state = State::Done;
        clearContent();
        auto sz = contentSize();
        float cx = sz.width / 2.f;
        float cy = sz.height / 2.f;

        auto check = CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
        if (check) {
            check->setScale(1.3f);
            check->setPosition({ cx, cy + 35.f });
            m_content->addChild(check);
        }

        auto label = CCLabelBMFont::create("Update installed!", "bigFont.fnt");
        label->setScale(0.7f);
        label->setColor({ 120, 255, 120 });
        label->setPosition({ cx, cy - 5.f });
        m_content->addChild(label);

        auto sub = CCLabelBMFont::create("Restart GD to apply the new version.", "chatFont.fnt");
        sub->setScale(0.55f);
        sub->setPosition({ cx, cy - 30.f });
        m_content->addChild(sub);

        auto restartSpr = ButtonSprite::create("Restart GD", "goldFont.fnt", "GJ_button_01.png", 0.7f);
        auto restartBtn = CCMenuItemSpriteExtra::create(restartSpr, this, menu_selector(UpdatePopup::onRestartClicked));
        auto menu = CCMenu::createWithItem(restartBtn);
        menu->setPosition({ cx, 28.f });
        m_content->addChild(menu);
    }

    void showErrorState(std::string const& msg) {
        m_state = State::Error;
        clearContent();
        auto sz = contentSize();
        float cx = sz.width / 2.f;
        float cy = sz.height / 2.f;

        if (auto sp = addStatusSpinner(cx, cy + 15.f)) {
            sp->setError(msg);
        }

        auto retrySpr = ButtonSprite::create("Retry", "goldFont.fnt", "GJ_button_02.png", 0.7f);
        auto retryBtn = CCMenuItemSpriteExtra::create(retrySpr, this, menu_selector(UpdatePopup::onRetryClicked));
        auto menu = CCMenu::createWithItem(retryBtn);
        menu->setPosition({ cx, 28.f });
        m_content->addChild(menu);
    }

    void checkForUpdate() {
        showCheckingState();
        auto req = web::WebRequest();
        req.timeout(std::chrono::seconds(15));
        m_versionTask.spawn(
            req.get("https://streak-servidor.onrender.com/mod/version"),
            [this](web::WebResponse res) {
                this->onVersionResponse(std::move(res));
            }
        );
    }

    void onVersionResponse(web::WebResponse res) {
        if (!res.ok()) {
            showErrorState(fmt::format("Network error (code {})", res.code()));
            return;
        }
        auto jsonRes = res.json();
        if (!jsonRes.isOk()) {
            showErrorState("Bad server response");
            return;
        }
        auto data = jsonRes.unwrap();
        std::string verStr   = data["version"].asString().unwrapOr("");
        std::string url      = data["url"].asString().unwrapOr("");
        std::string changelog = data["changelog"].asString().unwrapOr("");
        if (verStr.empty() || url.empty()) {
            showErrorState("Invalid server data");
            return;
        }
        auto parsed = VersionInfo::parse(verStr);
        if (!parsed.isOk()) {
            showErrorState(fmt::format("Bad version: {}", verStr));
            return;
        }
        m_latestVer = parsed.unwrap();
        m_latestVerStr = verStr;
        m_downloadUrl = url;
        m_changelog = changelog;

        if (m_latestVer > m_currentVer) {
            std::string pending = Mod::get()->getSavedValue<std::string>(PENDING_UPDATE_KEY, "");
            if (!pending.empty() && pending == m_latestVerStr) {
                showDoneState();
                return;
            }
            StreakUpdate::markAvailable(m_latestVerStr, m_downloadUrl, m_changelog);
            showUpdateAvailableState();
        } else {
            StreakUpdate::clearAvailable();
            showUpToDateState();
        }
    }

    void onDownloadClicked(CCObject*) {
        showDownloadingState();
        auto req = web::WebRequest();
        req.timeout(std::chrono::seconds(120));
        req.onProgress([this](web::WebProgress const& p) {
            float pct = p.downloadProgress().value_or(0.f);
            this->updateDownloadProgress(pct);
        });
        m_downloadTask.spawn(
            req.get(m_downloadUrl),
            [this](web::WebResponse res) {
                this->onDownloadComplete(std::move(res));
            }
        );
    }

    void onDownloadComplete(web::WebResponse res) {
        if (!res.ok()) {
            showErrorState(fmt::format("Download failed (code {})", res.code()));
            return;
        }
        auto bytes = res.data();
        if (bytes.size() < 1024) {
            showErrorState("Downloaded file is too small");
            return;
        }

        std::error_code ec;
        std::filesystem::path target = geode::dirs::getModsDir() / "jotabelike.gd_racha.geode";
        std::filesystem::path backup = target;
        backup += ".bak";

        if (std::filesystem::exists(target, ec)) {
            std::filesystem::remove(backup, ec);
            std::filesystem::rename(target, backup, ec);
            if (ec) {
                showErrorState("Could not move old version (file locked?)");
                return;
            }
        }

        std::ofstream out(target, std::ios::binary | std::ios::trunc);
        if (!out) {
            if (std::filesystem::exists(backup, ec)) {
                std::filesystem::rename(backup, target, ec);
            }
            showErrorState("Could not write new .geode file");
            return;
        }
        out.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
        bool writeOk = out.good();
        out.close();
        if (!writeOk) {
            std::filesystem::remove(target, ec);
            if (std::filesystem::exists(backup, ec)) {
                std::filesystem::rename(backup, target, ec);
            }
            showErrorState("Write failed (disk full?)");
            return;
        }

        std::filesystem::remove(backup, ec);
        Mod::get()->setSavedValue<std::string>(PENDING_UPDATE_KEY, m_latestVerStr);
        StreakUpdate::clearAvailable();
        showDoneState();
    }

    void onRetryClicked(CCObject*)   { checkForUpdate(); }
    void onCloseClicked(CCObject*)   { this->onClose(nullptr); }
    void onRestartClicked(CCObject*) { geode::utils::game::restart(true); }

public:
    static UpdatePopup* create() {
        auto ret = new UpdatePopup();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};
