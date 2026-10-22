# SPDX-License-Identifier: GPL-3.0-or-later
# Run manually in a terminal. OpenPaint only displays/copies this script.
import ast
import fnmatch
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import urllib.request

UUID = "blur-my-shell@aunetx"
SCHEMA = "org.gnome.shell.extensions.blur-my-shell.applications"
URL = "https://extensions.gnome.org/download-extension/blur-my-shell%40aunetx.shell-extension.zip?version_tag=76770"
SHA256 = "0dfde0d3575b4c7e32647e94010fa37f15ac933f4fe61c46bfe200e772114d85"


def run(*args, check=True):
    result = subprocess.run(args, text=True, capture_output=True, timeout=30)
    if check and result.returncode:
        raise RuntimeError(f"{args[0]} failed: {result.stderr.strip()}")
    return result.stdout.strip() if result.returncode == 0 else None


def array(value):
    if value is None:
        raise RuntimeError("Could not read desktop settings; no guessed settings will be written.")
    result = ast.literal_eval(value.removeprefix("@as "))
    if not isinstance(result, list) or not all(isinstance(item, str) for item in result):
        raise RuntimeError("Unrecognized desktop settings.")
    return result


def schema_directory():
    roots = [Path(os.environ.get("XDG_DATA_HOME", Path.home() / ".local/share"))]
    roots += [Path(root) for root in os.environ.get("XDG_DATA_DIRS", "/usr/local/share:/usr/share").split(":") if root]
    for root in roots:
        directory = root / "gnome-shell/extensions" / UUID / "schemas"
        if (directory / "gschemas.compiled").is_file() or any(directory.glob("*.gschema.xml")):
            return directory
    return None


def prepare_settings(directory):
    if not (directory / "gschemas.compiled").is_file():
        run("glib-compile-schemas", str(directory))


def install_extension():
    print("Downloading verified Blur My Shell 76 from GNOME Extensions…")
    with urllib.request.urlopen(URL, timeout=30) as response:
        data = response.read(2 * 1024 * 1024 + 1)
    if len(data) > 2 * 1024 * 1024 or hashlib.sha256(data).hexdigest() != SHA256:
        raise RuntimeError("Extension download verification failed; nothing was installed.")
    with tempfile.TemporaryDirectory(prefix="openpaint-glass-") as directory:
        archive = Path(directory) / "blur-my-shell.zip"
        archive.write_bytes(data)
        # Never overwrite an existing extension or install as root.
        run("gnome-extensions", "install", str(archive))


def configure(directory):
    prefix = ("gsettings", "--schemadir", str(directory))
    values = {key: run(*prefix, "get", SCHEMA, key)
              for key in ("blur", "enable-all", "whitelist", "blacklist", "opacity", "dynamic-opacity", "static-blur")}
    enabled = array(run("gsettings", "get", "org.gnome.shell", "enabled-extensions"))
    disabled = array(run("gsettings", "get", "org.gnome.shell", "disabled-extensions"))
    disabled_all = run("gsettings", "get", "org.gnome.shell", "disable-user-extensions")
    whitelist, blacklist = array(values["whitelist"]), array(values["blacklist"])
    if values["enable-all"] not in ("true", "false"):
        raise RuntimeError("Unrecognized application blur mode.")
    backup = Path(os.environ.get("XDG_STATE_HOME", Path.home() / ".local/state")) / "openpaint/blur-setup-backup.json"
    backup.parent.mkdir(parents=True, exist_ok=True)
    saved = json.loads(backup.read_text()) if backup.exists() else {}
    original = saved.setdefault("applications", {})
    for key, value in values.items():
        original.setdefault(key, value)
    saved.setdefault("enabled-extensions", enabled)
    saved.setdefault("disabled-extensions", disabled)
    saved.setdefault("disable-user-extensions", disabled_all)
    # Replace atomically so a failed write preserves the original backup.
    with tempfile.NamedTemporaryFile(mode="w", dir=backup.parent, delete=False) as output:
        temporary_backup = Path(output.name)
        try:
            json.dump(saved, output, indent=2)
            output.close()
            temporary_backup.replace(backup)
        finally:
            temporary_backup.unlink(missing_ok=True)
    if values["enable-all"] == "true":
        blacklist = [pattern for pattern in blacklist
                     if not fnmatch.fnmatchcase("openpaint", pattern.lower())]
        run(*prefix, "set", SCHEMA, "blacklist", json.dumps(blacklist))
    elif "OpenPaint" not in whitelist:
        run(*prefix, "set", SCHEMA, "whitelist", json.dumps(whitelist + ["OpenPaint"]))
    # Wallpaper blur avoids the live-window partial-redraw path.
    # OpenPaint controls its own surface opacity; keep its text/artwork opaque.
    for key, value in (("blur", "true"), ("opacity", "255"), ("dynamic-opacity", "false"), ("static-blur", "true")):
        run(*prefix, "set", SCHEMA, key, value)
    if UUID not in enabled:
        enabled.append(UUID)
    run("gsettings", "set", "org.gnome.shell", "enabled-extensions", json.dumps(enabled))
    run("gsettings", "set", "org.gnome.shell", "disabled-extensions", json.dumps([item for item in disabled if item != UUID]))
    run("gsettings", "set", "org.gnome.shell", "disable-user-extensions", "false")
    run("gnome-extensions", "enable", UUID, check=False)
    active = run("gnome-extensions", "list", "--active", check=False) or ""
    print("Setup complete. Return to OpenPaint and click Check again." if UUID in active.splitlines()
          else "Setup saved. Save your work, sign out and back in, then open OpenPaint and click Check again.")
    print("Previous settings were saved in the OpenPaint state folder. No restart or sign-out was forced.")


def main():
    if os.geteuid() == 0:
        raise RuntimeError("Run this as your normal desktop user, without sudo.")
    if ("GNOME" not in os.environ.get("XDG_CURRENT_DESKTOP", "").upper().split(":")
            or os.environ.get("XDG_SESSION_TYPE") != "x11"
            or not run("gnome-shell", "--version").startswith("GNOME Shell 46.")):
        raise RuntimeError("This setup supports GNOME 46 on X11. No changes were made.")
    if Path("/.flatpak-info").exists() or "SNAP" in os.environ:
        raise RuntimeError("Run this in your desktop terminal, outside an application sandbox.")
    directory = schema_directory()
    if directory is None:
        listed = run("gnome-extensions", "list")
        if UUID in listed.splitlines():
            raise RuntimeError("Blur My Shell is present but its settings are unavailable. No extension was replaced.")
        install_extension()
        directory = schema_directory()
    if directory is None:
        raise RuntimeError("The extension settings could not be found after installation.")
    prepare_settings(directory)
    configure(directory)


if __name__ == "__main__":
    try:
        main()
    except (OSError, RuntimeError, ValueError, subprocess.SubprocessError) as error:
        print(f"Setup stopped: {error}", file=sys.stderr)
        sys.exit(1)
