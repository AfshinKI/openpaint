# SPDX-License-Identifier: GPL-3.0-or-later
import importlib.util
import io
import json
import os
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

SOURCE = Path(__file__).resolve().parents[2] / "qml/setup/enable-glass.py"
spec = importlib.util.spec_from_file_location("blur_setup", SOURCE)
setup = importlib.util.module_from_spec(spec)
spec.loader.exec_module(setup)


class BlurSetupTest(unittest.TestCase):
    def test_variant_arrays(self):
        self.assertEqual(setup.array("@as []"), [])
        self.assertEqual(setup.array("['OtherApp', 'OpenPaint']"), ["OtherApp", "OpenPaint"])
        for value in (None, "false", "[1]", "__import__('os')"):
            with self.assertRaises((RuntimeError, ValueError)):
                setup.array(value)

    def test_bad_download_never_installs(self):
        with patch.object(setup.urllib.request, "urlopen", return_value=io.BytesIO(b"bad")), \
                patch.object(setup, "run") as run:
            with self.assertRaisesRegex(RuntimeError, "verification failed"):
                setup.install_extension()
            run.assert_not_called()

    def test_verified_download_uses_native_installer_without_force(self):
        import hashlib
        payload = b"generated fixture archive"
        with patch.object(setup.urllib.request, "urlopen", return_value=io.BytesIO(payload)), \
                patch.object(setup, "SHA256", hashlib.sha256(payload).hexdigest()), \
                patch.object(setup, "run") as run:
            setup.install_extension()
            args = run.call_args.args
            self.assertEqual(args[:2], ("gnome-extensions", "install"))
            self.assertNotIn("--force", args)
            self.assertFalse(Path(args[2]).exists())

    def test_configure_preserves_unrelated_entries_and_backs_up(self):
        for all_apps in ("false", "true"):
            with self.subTest(mode=all_apps), tempfile.TemporaryDirectory() as directory:
                calls = []
                settings = {"blur": "false", "enable-all": all_apps,
                            "whitelist": "['OtherApp']", "blacklist": "['OtherApp', 'Open*']",
                            "opacity": "215", "dynamic-opacity": "true", "static-blur": "false",
                            "enabled-extensions": "['other@example']",
                            "disabled-extensions": "['blur-my-shell@aunetx', 'disabled@example']",
                            "disable-user-extensions": "true"}
                def run(*args, **kwargs):
                    calls.append(args)
                    if "get" in args:
                        return settings[args[-1]]
                    if args[:2] == ("gnome-extensions", "list"):
                        return ""
                    return ""
                backup = Path(directory) / "openpaint/blur-setup-backup.json"
                backup.parent.mkdir()
                backup.write_text(json.dumps({"applications": {"blur": "false", "opacity": "200"}}))
                with patch.dict(os.environ, {"XDG_STATE_HOME": directory}), patch.object(setup, "run", side_effect=run):
                    setup.configure(Path("/synthetic/schemas"))
                saved = json.loads((Path(directory) / "openpaint/blur-setup-backup.json").read_text())
                self.assertEqual(saved["applications"]["blur"], "false")
                self.assertEqual(saved["applications"]["static-blur"], "false")
                self.assertEqual(saved["applications"]["opacity"], "200")
                writes = {args[-2]: args[-1] for args in calls if "set" in args}
                self.assertEqual(writes["static-blur"], "true")
                self.assertEqual(writes["dynamic-opacity"], "false")
                self.assertEqual(writes["opacity"], "255")
                self.assertIn("other@example", writes["enabled-extensions"])
                self.assertEqual(json.loads(writes["disabled-extensions"]), ["disabled@example"])
                if all_apps == "true":
                    self.assertEqual(json.loads(writes["blacklist"]), ["OtherApp"])
                else:
                    self.assertEqual(json.loads(writes["whitelist"]), ["OtherApp", "OpenPaint"])

    def test_existing_extension_is_not_reinstalled(self):
        with patch.object(setup.os, "geteuid", return_value=1000), \
                patch.dict(os.environ, {"XDG_CURRENT_DESKTOP": "GNOME", "XDG_SESSION_TYPE": "x11"}, clear=True), \
                patch.object(setup, "run", return_value="GNOME Shell 46.0"), \
                patch.object(setup, "schema_directory", return_value=Path("/synthetic/schemas")), \
                patch.object(setup, "install_extension") as install, patch.object(setup, "prepare_settings"), \
                patch.object(setup, "configure") as configure:
            setup.main()
            install.assert_not_called()
            configure.assert_called_once()

    def test_missing_extension_installs_once_then_configures(self):
        with patch.object(setup.os, "geteuid", return_value=1000), \
                patch.dict(os.environ, {"XDG_CURRENT_DESKTOP": "GNOME", "XDG_SESSION_TYPE": "x11"}, clear=True), \
                patch.object(setup, "run", side_effect=["GNOME Shell 46.0", "other@example"]), \
                patch.object(setup, "schema_directory", side_effect=[None, Path("/synthetic/schemas")]), \
                patch.object(setup, "install_extension") as install, patch.object(setup, "prepare_settings"), \
                patch.object(setup, "configure") as configure:
            setup.main()
            install.assert_called_once()
            configure.assert_called_once()

    def test_compile_settings_only_when_needed(self):
        with tempfile.TemporaryDirectory() as directory, patch.object(setup, "run") as run:
            path = Path(directory)
            setup.prepare_settings(path)
            run.assert_called_once_with("glib-compile-schemas", str(path))
            run.reset_mock()
            (path / "gschemas.compiled").write_bytes(b"generated fixture")
            setup.prepare_settings(path)
            run.assert_not_called()

    def test_root_or_unsupported_session_makes_no_changes(self):
        with patch.object(setup.os, "geteuid", return_value=0), patch.object(setup, "run") as run:
            with self.assertRaisesRegex(RuntimeError, "without sudo"):
                setup.main()
            run.assert_not_called()
        with patch.object(setup.os, "geteuid", return_value=1000), \
                patch.dict(os.environ, {"XDG_CURRENT_DESKTOP": "KDE"}, clear=True), \
                patch.object(setup, "run") as run:
            with self.assertRaisesRegex(RuntimeError, "No changes"):
                setup.main()
            run.assert_not_called()
