#include "RemoteAssetManager.h"

#include <Geode/Geode.hpp>
#include <Geode/utils/async.hpp>
#include <Geode/utils/file.hpp>
#include <Geode/utils/web.hpp>
#include <matjson.hpp>

#include <algorithm>
#include <filesystem>
#include <iomanip>
#include <map>
#include <memory>
#include <sstream>
#include <utility>
#include <vector>

using namespace geode::prelude;

// HMACAuth's public header uses Geode's `web` prelude alias.
#include "HMACAuth.h"

namespace RemoteAssets {
namespace {

constexpr char const* SERVER_MANIFEST_URL =
    "https://streak-servidor.onrender.com/assets/manifest";
constexpr char const* FALLBACK_MANIFEST_URL =
    "https://raw.githubusercontent.com/Jotabelike/Streak-Assets/main/manifest.json";
constexpr char const* MOD_ID = "jotabelike.gd_racha";

struct FileDescriptor {
    std::string name;
    std::string url;
    std::string sha256;
    std::size_t size = 0;
};

struct AssetDescriptor {
    Type type = Type::Badge;
    std::string id;
    std::vector<FileDescriptor> files;
};

std::filesystem::path cacheRoot() {
    return Mod::get()->getSaveDir() / "asset-cache";
}

std::filesystem::path assetDirectory() {
    return cacheRoot() / MOD_ID;
}

std::filesystem::path manifestPath() {
    return cacheRoot() / "manifest.json";
}

std::string typeName(Type type) {
    switch (type) {
        case Type::Badge: return "badge";
        case Type::Banner: return "banner";
        case Type::Song: return "song";
    }
    return "";
}

std::string keyFor(Type type, std::string const& id) {
    return typeName(type) + ":" + id;
}

std::map<std::string, AssetDescriptor> s_assets;
bool s_initialized = false;
bool s_refreshing = false;
std::vector<Completion> s_refreshCallbacks;
async::TaskHolder<web::WebResponse> s_manifestTask;

struct DownloadJob {
    AssetDescriptor asset;
    std::size_t index = 0;
    async::TaskHolder<web::WebResponse> task;
};

std::map<std::string, std::shared_ptr<DownloadJob>> s_downloads;
std::map<std::string, std::vector<Completion>> s_downloadCallbacks;

struct BulkJob {
    std::vector<std::pair<Type, std::string>> assets;
    std::size_t index = 0;
    Progress progress;
    Completion completion;
};

std::vector<std::shared_ptr<BulkJob>> s_bulkJobs;

bool parseType(std::string const& value, Type& out) {
    if (value == "badge") out = Type::Badge;
    else if (value == "banner") out = Type::Banner;
    else if (value == "song") out = Type::Song;
    else return false;
    return true;
}

bool isSafeFilename(std::string const& name) {
    if (name.empty() || name == "." || name == "..") return false;
    return name.find('/') == std::string::npos &&
        name.find('\\') == std::string::npos &&
        std::filesystem::path(name).filename() == std::filesystem::path(name);
}

bool isAllowedUrl(std::string const& url) {
    return url.starts_with("https://raw.githubusercontent.com/Jotabelike/Streak-Assets/") ||
        url.starts_with("https://github.com/Jotabelike/Streak-Assets/");
}

bool parseManifest(matjson::Value const& root, std::string& error) {
    if (!root.isObject() || root["mod_id"].asString().unwrapOr("") != MOD_ID) {
        error = "manifest belongs to another mod";
        return false;
    }

    auto result = root["assets"].as<std::vector<matjson::Value>>();
    if (!result.isOk()) {
        error = "manifest has no asset list";
        return false;
    }

    std::map<std::string, AssetDescriptor> parsed;
    for (auto const& item : result.unwrap()) {
        AssetDescriptor asset;
        asset.id = item["id"].asString().unwrapOr("");
        if (asset.id.empty() || !parseType(item["type"].asString().unwrapOr(""), asset.type)) {
            error = "manifest contains an invalid asset";
            return false;
        }

        auto files = item["files"].as<std::vector<matjson::Value>>();
        if (!files.isOk() || files.unwrap().empty()) {
            error = "asset has no files";
            return false;
        }

        for (auto const& itemFile : files.unwrap()) {
            FileDescriptor file;
            file.name = itemFile["name"].asString().unwrapOr("");
            file.url = itemFile["url"].asString().unwrapOr("");
            file.sha256 = itemFile["sha256"].asString().unwrapOr("");
            auto signedSize = itemFile["size"].as<std::int64_t>().unwrapOr(-1);
            if (!isSafeFilename(file.name) || !isAllowedUrl(file.url) ||
                file.sha256.size() != 64 || signedSize < 0) {
                error = "asset contains an invalid file";
                return false;
            }
            file.size = static_cast<std::size_t>(signedSize);
            asset.files.push_back(std::move(file));
        }

        auto key = keyFor(asset.type, asset.id);
        if (parsed.contains(key)) {
            error = "manifest contains a duplicate asset";
            return false;
        }
        parsed.emplace(std::move(key), std::move(asset));
    }

    if (parsed.empty()) {
        error = "manifest is empty";
        return false;
    }
    s_assets = std::move(parsed);
    return true;
}

std::string hashBytes(ByteVector const& bytes) {
    auto digest = HMACAuth::detail::sha256(bytes.data(), bytes.size());
    std::ostringstream output;
    output << std::hex << std::setfill('0');
    for (auto byte : digest) output << std::setw(2) << static_cast<int>(byte);
    return output.str();
}

bool fileIsInstalled(FileDescriptor const& file) {
    auto path = assetDirectory() / file.name;
    std::error_code error;
    return std::filesystem::is_regular_file(path, error) && !error &&
        std::filesystem::file_size(path, error) == file.size && !error;
}

void notifyRefresh(bool ok, std::string const& message) {
    auto callbacks = std::move(s_refreshCallbacks);
    s_refreshCallbacks.clear();
    s_refreshing = false;
    for (auto& callback : callbacks) {
        if (callback) callback(ok, message);
    }
}

void acceptManifest(web::WebResponse response, bool mayFallback) {
    if (!response.ok()) {
        if (mayFallback) {
            auto request = web::WebRequest();
            request.timeout(std::chrono::seconds(20));
            s_manifestTask.spawn(request.get(FALLBACK_MANIFEST_URL), [](web::WebResponse fallback) {
                acceptManifest(std::move(fallback), false);
            });
            return;
        }
        notifyRefresh(false, fmt::format("catalog request failed ({})", response.code()));
        return;
    }

    auto json = response.json();
    if (!json.isOk()) {
        notifyRefresh(false, "catalog response is not JSON");
        return;
    }

    std::string error;
    if (!parseManifest(json.unwrap(), error)) {
        notifyRefresh(false, error);
        return;
    }

    auto write = file::writeStringSafe(manifestPath(), json.unwrap().dump());
    if (!write.isOk()) log::warn("Could not persist asset manifest: {}", write.unwrapErr());
    notifyRefresh(true, "");
}

void finishDownload(std::string const& key, bool ok, std::string const& message) {
    auto callbacksIt = s_downloadCallbacks.find(key);
    std::vector<Completion> callbacks;
    if (callbacksIt != s_downloadCallbacks.end()) {
        callbacks = std::move(callbacksIt->second);
        s_downloadCallbacks.erase(callbacksIt);
    }
    s_downloads.erase(key);
    for (auto& callback : callbacks) {
        if (callback) callback(ok, message);
    }
}

void downloadNext(std::shared_ptr<DownloadJob> const& job) {
    auto key = keyFor(job->asset.type, job->asset.id);
    while (job->index < job->asset.files.size() && fileIsInstalled(job->asset.files[job->index])) {
        ++job->index;
    }
    if (job->index >= job->asset.files.size()) {
        CCFileUtils::sharedFileUtils()->purgeCachedEntries();
        finishDownload(key, true, "");
        return;
    }

    auto descriptor = job->asset.files[job->index];
    auto request = web::WebRequest();
    request.timeout(std::chrono::seconds(90));
    job->task.spawn(request.get(descriptor.url), [job, descriptor, key](web::WebResponse response) {
        if (!response.ok()) {
            finishDownload(key, false, fmt::format("{}: HTTP {}", descriptor.name, response.code()));
            return;
        }

        auto const& bytes = response.data();
        if (bytes.size() != descriptor.size || hashBytes(bytes) != descriptor.sha256) {
            finishDownload(key, false, descriptor.name + ": integrity check failed");
            return;
        }

        auto write = file::writeBinarySafe(assetDirectory() / descriptor.name, bytes);
        if (!write.isOk()) {
            finishDownload(key, false, descriptor.name + ": could not write cache");
            return;
        }
        ++job->index;
        downloadNext(job);
    });
}

void advanceBulk(std::shared_ptr<BulkJob> const& job) {
    if (job->index >= job->assets.size()) {
        if (job->completion) job->completion(true, "");
        std::erase(s_bulkJobs, job);
        return;
    }

    auto [type, id] = job->assets[job->index];
    ensure(type, id, [job](bool ok, std::string const& message) {
        if (!ok) {
            if (job->completion) job->completion(false, message);
            std::erase(s_bulkJobs, job);
            return;
        }
        ++job->index;
        if (job->progress) job->progress(job->index, job->assets.size());
        advanceBulk(job);
    });
}

} // namespace

void initialize() {
    if (s_initialized) return;
    s_initialized = true;

    auto created = file::createDirectoryAll(assetDirectory());
    if (!created.isOk()) {
        log::error("Could not create remote asset cache: {}", created.unwrapErr());
        return;
    }

    auto path = geode::utils::string::pathToString(cacheRoot());
    CCFileUtils::sharedFileUtils()->addPriorityPath(path.c_str());

    auto cached = file::readJson(manifestPath());
    if (cached.isOk()) {
        std::string error;
        if (!parseManifest(cached.unwrap(), error)) {
            log::warn("Ignoring cached asset manifest: {}", error);
        }
    }
    refreshManifest([](bool ok, std::string const& message) {
        // Cosmetics behave like level thumbnails: the player never has to
        // manage downloads. Missing files are filled in sequentially in the
        // background and are reused from the persistent cache afterwards.
        if (ok || isReady()) {
            downloadAll(nullptr, [](bool downloaded, std::string const& downloadMessage) {
                if (!downloaded) {
                    log::warn("Automatic asset download paused: {}", downloadMessage);
                }
            });
        } else {
            log::warn("Automatic asset catalog unavailable: {}", message);
        }
    });
}

void refreshManifest(Completion completion) {
    if (completion) s_refreshCallbacks.push_back(std::move(completion));
    if (s_refreshing) return;
    s_refreshing = true;

    auto request = web::WebRequest();
    request.timeout(std::chrono::seconds(20));
    s_manifestTask.spawn(request.get(SERVER_MANIFEST_URL), [](web::WebResponse response) {
        acceptManifest(std::move(response), true);
    });
}

bool isReady() {
    return !s_assets.empty();
}

bool isInstalled(Type type, std::string const& id) {
    auto found = s_assets.find(keyFor(type, id));
    if (found == s_assets.end()) return false;
    return std::ranges::all_of(found->second.files, fileIsInstalled);
}

std::size_t assetCount() {
    return s_assets.size();
}

std::size_t installedCount() {
    return static_cast<std::size_t>(std::ranges::count_if(s_assets, [](auto const& entry) {
        return std::ranges::all_of(entry.second.files, fileIsInstalled);
    }));
}

void ensure(Type type, std::string const& id, Completion completion) {
    if (!s_initialized) initialize();
    if (!isReady()) {
        refreshManifest([type, id, completion = std::move(completion)](bool ok, std::string const& message) mutable {
            if (!ok) {
                if (completion) completion(false, message);
                return;
            }
            ensure(type, id, std::move(completion));
        });
        return;
    }

    auto key = keyFor(type, id);
    auto found = s_assets.find(key);
    if (found == s_assets.end()) {
        if (completion) completion(false, "asset is not present in the catalog");
        return;
    }
    if (isInstalled(type, id)) {
        if (completion) completion(true, "");
        return;
    }

    if (completion) s_downloadCallbacks[key].push_back(std::move(completion));
    if (s_downloads.contains(key)) return;

    auto job = std::make_shared<DownloadJob>();
    job->asset = found->second;
    s_downloads[key] = job;
    downloadNext(job);
}

void downloadAll(Progress progress, Completion completion) {
    if (!s_initialized) initialize();
    if (!isReady()) {
        refreshManifest([progress = std::move(progress), completion = std::move(completion)](bool ok, std::string const& message) mutable {
            if (!ok) {
                if (completion) completion(false, message);
                return;
            }
            downloadAll(std::move(progress), std::move(completion));
        });
        return;
    }

    auto job = std::make_shared<BulkJob>();
    job->progress = std::move(progress);
    job->completion = std::move(completion);
    for (auto const& [key, asset] : s_assets) {
        if (!isInstalled(asset.type, asset.id)) job->assets.emplace_back(asset.type, asset.id);
    }
    s_bulkJobs.push_back(job);
    if (job->progress) job->progress(0, job->assets.size());
    advanceBulk(job);
}

} // namespace RemoteAssets
