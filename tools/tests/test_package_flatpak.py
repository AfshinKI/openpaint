"""Reject dependency drift and accidental sandbox expansion."""
import importlib.util
from pathlib import Path
import unittest

SPEC = importlib.util.spec_from_file_location('package_flatpak', Path(__file__).parents[1] / 'package_flatpak.py')
PACKAGE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(PACKAGE)
BASE = '[Context]\nshared=ipc;\nsockets=x11;wayland;fallback-x11;\ndevices=dri;\n[Environment]\nQT_QPA_PLATFORMTHEME=xdgdesktopportal\n'


class FlatpakTests(unittest.TestCase):
    def test_expected_permissions(self):
        self.assertIn('Context', PACKAGE.audit_permissions(BASE))

    def test_network_and_home_are_rejected(self):
        for text in (BASE.replace('shared=ipc;', 'shared=ipc;network;'),
                     BASE.replace('[Environment]', 'filesystems=home;\n[Environment]'),
                     BASE + '[Session Bus Policy]\norg.example.Host=talk\n'):
            with self.assertRaises(ValueError):
                PACKAGE.audit_permissions(text)

    def test_dependency_drift_is_rejected(self):
        PACKAGE.check_commit('a' * 64, 'a' * 64)
        with self.assertRaises(ValueError):
            PACKAGE.check_commit('b' * 64, 'a' * 64)
        with self.assertRaises(ValueError):
            PACKAGE.check_commit('moving-branch', 'moving-branch')
