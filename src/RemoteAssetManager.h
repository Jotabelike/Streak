#pragma once

#include <functional>
#include <string>

namespace RemoteAssets {

enum class Type {
    Badge,
    Banner,
    Song,
};

using Completion = std::function<void(bool, std::string const&)>;
using Progress = std::function<void(std::size_t completed, std::size_t total)>;

// Registers the persistent cache as a Cocos search path, loads the last valid
// manifest from disk and refreshes it in the background.
void initialize();
void refreshManifest(Completion completion = nullptr);

bool isReady();
bool isInstalled(Type type, std::string const& id);
std::size_t assetCount();
std::size_t installedCount();

// Downloads all files belonging to one catalog item (including HD/UHD sprite
// variants) and verifies byte length + SHA-256 before committing them to disk.
void ensure(Type type, std::string const& id, Completion completion = nullptr);

// Sequential by design: this avoids launching hundreds of simultaneous GitHub
// requests on a first installation.
void downloadAll(Progress progress = nullptr, Completion completion = nullptr);

} // namespace RemoteAssets
