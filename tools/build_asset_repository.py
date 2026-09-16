#!/usr/bin/env python3
"""Build the public Streak asset repository from a packaged .geode file.

The Geode packager creates normal, HD and UHD variants for every sprite.  This
script preserves those generated files so externally cached cosmetics render at
the exact same size and quality as their bundled equivalents.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import re
import shutil
import zipfile
from datetime import datetime, timezone
from pathlib import Path
from urllib.parse import quote


MOD_ID = "jotabelike.gd_racha"
DEFAULT_REPOSITORY = "Jotabelike/Streak-Assets"


def parse_catalog(header: Path) -> list[dict[str, str]]:
    text = header.read_text(encoding="utf-8")

    badge_block = text.split("std::vector<BadgeInfo> badges = {", 1)[1].split(
        "std::vector<BannerInfo> banners = {", 1
    )[0]
    banner_block = text.split("std::vector<BannerInfo> banners = {", 1)[1].split(
        "std::vector<SongInfo> songs = {", 1
    )[0]
    song_block = text.split("std::vector<SongInfo> songs = {", 1)[1].split(
        "std::vector<bool> unlockedBadges", 1
    )[0]

    assets: list[dict[str, str]] = []
    badge_pattern = re.compile(
        r'\{\s*\d+\s*,\s*"([^"]+\.png)"_spr\s*,.*?BadgeCategory::\w+\s*,\s*"([^"]+)"'
    )
    for filename, asset_id in badge_pattern.findall(badge_block):
        assets.append({"id": asset_id, "type": "badge", "primary": filename})

    banner_pattern = re.compile(
        r'\{\s*"([^"]+)"\s*,\s*"([^"]+\.png)"_spr\s*,'
    )
    for asset_id, filename in banner_pattern.findall(banner_block):
        assets.append({"id": asset_id, "type": "banner", "primary": filename})

    song_pattern = re.compile(
        r'\{\s*"([^"]+)"\s*,\s*"([^"]+\.mp3)"_spr\s*,\s*"([^"]+\.png)"_spr\s*,'
    )
    for asset_id, audio, icon in song_pattern.findall(song_block):
        assets.append(
            {
                "id": asset_id,
                "type": "song",
                "primary": audio,
                "icon": icon,
            }
        )

    seen: set[tuple[str, str]] = set()
    unique: list[dict[str, str]] = []
    for asset in assets:
        key = (asset["type"], asset["id"])
        if key in seen:
            raise ValueError(f"Duplicate catalog asset: {key}")
        seen.add(key)
        unique.append(asset)
    return unique


def sprite_variants(filename: str) -> list[str]:
    path = Path(filename)
    return [
        filename,
        f"{path.stem}-hd{path.suffix}",
        f"{path.stem}-uhd{path.suffix}",
    ]


def sha256(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def build_repository(
    package: Path,
    header: Path,
    output: Path,
    repository: str,
    ref: str,
) -> None:
    if output.exists():
        # A generated repository may already have its own .git directory. Keep
        # it so the script can be run in-place for later catalog releases.
        for child in output.iterdir():
            if child.name == ".git":
                continue
            if child.is_dir():
                shutil.rmtree(child)
            else:
                child.unlink()
    asset_root = output / "assets" / MOD_ID
    asset_root.mkdir(parents=True)

    catalog = parse_catalog(header)
    encoded_ref = quote(ref, safe="")
    base_url = f"https://raw.githubusercontent.com/{repository}/{encoded_ref}/assets/{MOD_ID}"
    records: list[dict[str, object]] = []
    written: dict[str, dict[str, object]] = {}

    with zipfile.ZipFile(package) as archive:
        archive_names = set(archive.namelist())
        for asset in catalog:
            wanted: list[str] = []
            primary = asset["primary"]
            if primary.endswith(".png"):
                wanted.extend(sprite_variants(primary))
            else:
                wanted.append(primary)

            icon = asset.get("icon")
            if icon:
                wanted.extend(sprite_variants(icon))

            files: list[dict[str, object]] = []
            for filename in wanted:
                if filename in written:
                    files.append(written[filename])
                    continue

                archive_name = f"resources/{MOD_ID}/{filename}"
                if archive_name not in archive_names:
                    raise FileNotFoundError(
                        f"{archive_name} is missing from {package}. Build the mod first."
                    )
                data = archive.read(archive_name)
                (asset_root / filename).write_bytes(data)
                encoded_name = quote(filename)
                descriptor: dict[str, object] = {
                    "name": filename,
                    "url": f"{base_url}/{encoded_name}",
                    "sha256": sha256(data),
                    "size": len(data),
                }
                written[filename] = descriptor
                files.append(descriptor)

            record: dict[str, object] = {
                "id": asset["id"],
                "type": asset["type"],
                "primary": primary,
                "files": files,
            }
            if icon:
                record["icon"] = icon
            records.append(record)

    manifest = {
        "schema": 1,
        "version": datetime.now(timezone.utc).strftime("%Y%m%d%H%M%S"),
        "mod_id": MOD_ID,
        "assets": records,
    }
    (output / "manifest.json").write_text(
        json.dumps(manifest, indent=2, ensure_ascii=False) + "\n",
        encoding="utf-8",
    )
    (output / "README.md").write_text(
        "# Streak Assets\n\n"
        "Remote cosmetic assets for the Geometry Dash mod "
        "[`jotabelike.gd_racha`](https://github.com/Jotabelike/Streak).\n\n"
        "`manifest.json` is generated by the main mod repository and contains "
        "the SHA-256 and byte size of every downloadable file. Do not rename "
        "files manually; rebuild the manifest when adding or replacing assets.\n",
        encoding="utf-8",
    )
    (output / ".gitignore").write_text("*.tmp\n", encoding="utf-8")

    print(
        f"Built {len(records)} catalog entries and {len(written)} files in {output}"
    )


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--package", type=Path, required=True)
    parser.add_argument("--header", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--repository", default=DEFAULT_REPOSITORY)
    parser.add_argument("--ref", default="main")
    args = parser.parse_args()
    build_repository(args.package, args.header, args.output, args.repository, args.ref)


if __name__ == "__main__":
    main()
