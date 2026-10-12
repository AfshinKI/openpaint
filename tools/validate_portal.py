#!/usr/bin/env python3
"""Exercise only uniquely titled synthetic OpenPaint portal dialogs on a desktop.

Requires python3-gi, gir1.2-atspi-2.0, a running file portal and the dev shell tests.
Does not inspect file contents or save captures. Test artwork/settings are temporary.
"""
import os
from pathlib import Path
import subprocess
import tempfile
import time
import uuid

import gi

gi.require_version("Atspi", "2.0")
from gi.repository import Atspi  # noqa: E402


def children(node):
    for index in range(node.get_child_count()):
        child = node.get_child_at_index(index)
        if child:
            yield child


def descendants(node):
    yield node
    for child in children(node):
        yield from descendants(child)


def activate(node):
    action = node.get_action_iface()
    return action is not None and action.get_n_actions() > 0 and action.do_action(0)


def main():
    root = Path(__file__).resolve().parent.parent
    token = uuid.uuid4().hex
    prefix = "OpenPaint validation "
    handled = set()
    with tempfile.TemporaryDirectory(prefix="openpaint-portal-") as directory:
        env = dict(os.environ, QT_QPA_PLATFORM="xcb", QT_QPA_PLATFORMTHEME="xdgdesktopportal",
                   OPENPAINT_PORTAL_DIR=directory, OPENPAINT_PORTAL_TOKEN=token)
        process = subprocess.Popen([str(root / "build/dev/openpaint_shell_tests"),
                                    "nativePortalWorkflow"], env=env)
        deadline = time.monotonic() + 100
        try:
            while process.poll() is None and time.monotonic() < deadline:
                desktop = Atspi.get_desktop(0)
                for app in children(desktop):
                    for dialog in children(app):
                        title = dialog.get_name()
                        stage = next((stage for stage in ("save", "open", "cancel")
                                      if title == prefix + stage + " " + token), None)
                        if stage is None or stage in handled:
                            continue
                        nodes = list(descendants(dialog))
                        if stage == "open":
                            matches = [node for node in nodes if node.get_name() == "portal.png"
                                       and node.get_role_name() == "table cell"]
                            if not matches:
                                continue
                            if not activate(matches[0]):
                                continue
                        names = {"save": {"Save"}, "open": {"Open", "Select"},
                                 "cancel": {"Cancel"}}[stage]
                        buttons = [node for node in nodes if node.get_role_name() == "push button"
                                   and node.get_name() in names]
                        if buttons and activate(buttons[0]):
                            handled.add(stage)
                            print("Portal action:", stage, flush=True)
                time.sleep(0.2)
            if process.poll() is None:
                raise RuntimeError("Portal validation timed out")
            return process.returncode if process.returncode else (0 if len(handled) == 3 else 1)
        finally:
            if process.poll() is None:
                process.terminate()
                process.wait(timeout=5)


if __name__ == "__main__":
    raise SystemExit(main())
