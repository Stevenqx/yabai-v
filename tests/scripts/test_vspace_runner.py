"""Runner orchestration regressions; no daemon or WindowServer access."""

import copy
import importlib.util
import json
import plistlib
from pathlib import Path
from types import SimpleNamespace
import unittest
from unittest.mock import patch


spec = importlib.util.spec_from_file_location(
    "vspace_runner", Path(__file__).resolve().parents[2] / "scripts/test-vspace.py")
runner = importlib.util.module_from_spec(spec)
spec.loader.exec_module(runner)


def fixture():
    return {
        "displays": [dict(id=10, index=1, uuid="a", frame={}, spaces=[1, 2], **{"has-focus": True}),
                     dict(id=20, index=2, uuid="b", frame={}, spaces=[3, 4], **{"has-focus": False})],
        "spaces": [dict(id=i * 100, index=i, display=1 if i < 3 else 2,
                        type="float", windows=[101] if i == 1 else [],
                        **{"has-focus": i == 1, "is-visible": i in (1, 3), "is-native-fullscreen": False})
                   for i in range(1, 5)],
        "windows": [dict(id=101, pid=50, space=1, display=1, frame={"x": 10}, **{"has-focus": True})],
    }


def mapping(index, sid, did):
    return {"index": index, "id": index + 10, "space-id": sid, "display-id": did,
            "has-focus": sid == 100}


class FakeRunner(runner.Runner):
    def __init__(self):
        self.state = fixture()
        self.rows = [mapping(1, 100, 10), mapping(2, 200, 10),
                     mapping(3, 300, 20), mapping(4, 400, 20)]
        self.checks = []
        self.focus_starts = []
        self.args = SimpleNamespace(hold_seconds=0)
        self.native_hidden_observed = True

    def snapshot(self, name):
        return copy.deepcopy(self.state)

    def record(self, name, status="passed", **details):
        self.checks.append(dict(name=name, status=status, **details))

    def listing(self):
        rows = copy.deepcopy(self.rows)
        for row in rows:
            row["has-focus"] = row["space-id"] == runner.active(self.state)
        return rows

    def setup_focus(self, sid):
        target = next(s for s in self.state["spaces"] if s["id"] == sid)
        for space in self.state["spaces"]:
            if space["display"] == target["display"]:
                space["is-visible"] = space["id"] == sid
            space["has-focus"] = space["id"] == sid
        for display in self.state["displays"]:
            display["has-focus"] = display["index"] == target["display"]
        for window in self.state["windows"]:
            window["has-focus"] = window["space"] == target["index"]
        return True

    def observe(self, sid):
        return runner.active(self.state) == sid

    def message(self, *args, check=True):
        if args[:2] == ("window", "--focus"):
            for window in self.state["windows"]:
                window["has-focus"] = window["id"] == args[2]
            return SimpleNamespace(returncode=0, stdout="", stderr="")
        if args[:2] == ("vspace", "--current"):
            import json
            current = next((w for w in self.listing() if w["has-focus"]), None)
            return SimpleNamespace(returncode=0 if current else 1,
                                   stdout=json.dumps(current) if current else "", stderr="" if current else "not mapped")
        if args[:2] == ("vspace", "--create"):
            display = next(d for d in self.state["displays"] if d["has-focus"])
            possible = runner.candidates(self.state, self.rows, display["index"])
            if not possible:
                return SimpleNamespace(returncode=1, stdout="", stderr=
                    "no eligible spare native desktop space on the current display.")
            self.rows.append(mapping(max(w["index"] for w in self.rows) + 1, possible[0]["id"], display["id"]))
            return SimpleNamespace(returncode=0, stdout="", stderr="")
        if args[1] == "--focus":
            sid = (next(s["id"] for s in self.state["spaces"] if s["index"] == args[2])
                   if args[0] == "space" else
                   next(w["space-id"] for w in self.rows if w["index"] == args[2]))
            if runner.active(self.state) == sid:
                return SimpleNamespace(returncode=1, stdout="", stderr="cannot focus an already focused space")
            self.focus_starts.append((args[0], sid, runner.focus_state(self.state)))
            if args[0] == "space" and sid == 400 and not self.native_hidden_observed:
                for space in self.state["spaces"]:
                    if space["display"] == 2:
                        space["is-visible"] = space["id"] == sid
            else:
                self.setup_focus(sid)
            return SimpleNamespace(returncode=0, stdout="", stderr="")
        raise AssertionError("unexpected command: " + repr(args))


class RunnerTests(unittest.TestCase):
    def test_default_is_headless(self):
        self.assertEqual(runner.parser().parse_args([]).mode, "headless")

    @patch.object(runner.sys.stdin, "isatty", return_value=False)
    def test_interactive_refuses_nonterminal_before_any_host_commands(self, isatty):
        fake = FakeRunner()
        with patch.object(fake, "run") as commands:
            with self.assertRaisesRegex(runner.TestFailure, "terminal"):
                fake.interactive_preflight()
            commands.assert_not_called()

    def preflight_fixture(self, ffm="disabled", mff="off"):
        fake = FakeRunner()
        fake.binary = runner.ROOT / "bin/yabai"
        fake.report = {}
        fake.save = lambda: None
        session = {"kCGSSessionUserIDKey": runner.os.getuid(), "kCGSSessionOnConsoleKey": True,
                   "kCGSessionLoginDoneKey": True}

        def read_host(args, **kwargs):
            command = args[0]
            if command == "codesign":
                stdout = "Authority=yabai-fork Development\n"
            elif command == "csrutil":
                stdout = "System Integrity Protection status: enabled.\n"
            elif command == "pgrep":
                stdout = "123\n"
            elif command == "ps":
                stdout = str(fake.binary) + "\n"
            elif command == "ioreg":
                stdout = plistlib.dumps({"IOConsoleUsers": [session]}).decode()
            else:
                raise AssertionError("unexpected preflight host command: " + repr(args))
            return SimpleNamespace(returncode=0, stdout=stdout, stderr="")

        fake.run = read_host
        fake.message = lambda domain, option: SimpleNamespace(stdout={"focus_follows_mouse": ffm,
                                                                       "mouse_follows_focus": mff}[option] + "\n")
        return fake

    @patch("builtins.print")
    @patch("builtins.input", return_value="RUN")
    @patch.object(runner.sys.stdin, "isatty", return_value=True)
    def test_interactive_accepts_native_disabled_focus_query(self, isatty, answer, output):
        fake = self.preflight_fixture()
        fake.interactive_preflight()
        self.assertEqual(fake.report["daemon_pid"], 123)
        answer.assert_called_once()

    @patch.object(runner.sys.stdin, "isatty", return_value=True)
    def test_interactive_rejects_enabled_mouse_focus(self, isatty):
        for ffm, mff in (("autofocus", "off"), ("autoraise", "off"), ("disabled", "on")):
            with self.subTest(ffm=ffm, mff=mff):
                fake = self.preflight_fixture(ffm, mff)
                with self.assertRaisesRegex(runner.TestFailure, "off before interactive tests"):
                    fake.interactive_preflight()

    def test_interrupt_in_allocation_runs_recovery(self):
        fake = FakeRunner()
        fake.args = SimpleNamespace(skip_allocation=False)
        fake.report = {"snapshots": {}}
        fake.interactive_preflight = lambda: None
        fake.query_checks = lambda: fake.snapshot("baseline")
        fake.save = lambda: None
        with patch.object(fake, "allocate", side_effect=KeyboardInterrupt):
            with self.assertRaises(KeyboardInterrupt):
                fake.interactive()
        self.assertEqual(fake.checks[-1]["status"], "passed")
        self.assertTrue(fake.checks[-1]["name"].startswith("recovery:"))

    @patch("builtins.input", return_value="yes")
    @patch.object(runner.time, "sleep")
    def test_full_interactive_sequence_with_fake_daemon(self, sleep, answer):
        fake = FakeRunner()
        fake.args = SimpleNamespace(skip_allocation=False, skip_focus=False, skip_layout=False, destroy_created=False, refresh_mappings=False, hold_seconds=0)
        fake.report = {"snapshots": {}}
        fake.interactive_preflight = lambda: None
        fake.query_checks = lambda: fake.snapshot("baseline")
        fake.save = lambda: None
        baseline = fake.snapshot("baseline")
        fake.interactive()
        self.assertEqual(runner.focus_state(fake.state), runner.focus_state(baseline))
        self.assertEqual(sum(c["name"].endswith("result parity") for c in fake.checks), 3)
        self.assertEqual(fake.checks[-1]["name"], "user visual confirmation")
        self.assertFalse(any(c["status"] == "failed" for c in fake.checks))

    def test_candidates_use_native_order_and_skip_visible_floating_fullscreen(self):
        snap = fixture()
        snap["spaces"].reverse()
        snap["spaces"].append(dict(id=500, index=5, display=1, windows=[999],
                                   **{"is-visible": False, "is-native-fullscreen": False}))
        snap["spaces"].append(dict(id=600, index=6, display=1, windows=[],
                                   **{"is-visible": False, "is-native-fullscreen": True}))
        self.assertEqual([s["id"] for s in runner.candidates(snap, [], 1)], [200])
        self.assertEqual([s["id"] for s in runner.candidates(snap, [mapping(9, 200, 10)], 1)], [])

    def test_refresh_requires_explicit_opt_in(self):
        self.assertFalse(runner.parser().parse_args(["interactive"]).refresh_mappings)
        self.assertTrue(runner.parser().parse_args(["interactive", "--refresh-mappings"]).refresh_mappings)

    def refresh_fixture(self, corrupt=None):
        fake = FakeRunner()

        def refresh(*args, **kwargs):
            self.assertEqual(args, ("vspace", "--refresh"))
            before = copy.deepcopy(fake.rows)
            released = [w for w in before if w["space-id"] in (200, 400)]
            if corrupt == "visible":
                released = [w for w in before if w["space-id"] in (100, 200, 400)]
            fake.rows = [dict(w, index=i + 1) for i, w in enumerate(w for w in before if w not in released)]
            result = dict(released=[{k: w[k] for k in ("index", "id", "space-id", "display-id")} for w in released],
                          reindexed=[dict(id=w["id"], index=n["index"], **{"old-index": w["index"]})
                                     for n in fake.rows for w in before if n["id"] == w["id"] and n["index"] != w["index"]],
                          retained=[dict(id=w["id"], index=w["index"], reason="visible") for w in fake.rows])
            if corrupt == "indices":
                result["reindexed"] = []
            return SimpleNamespace(returncode=0, stdout=json.dumps(result), stderr="")

        fake.message = refresh
        return fake

    def test_refresh_checks_compaction_and_idempotence(self):
        fake = self.refresh_fixture()
        fake.refresh_mappings(fake.snapshot("baseline"))
        self.assertEqual([w["id"] for w in fake.rows], [11, 13])
        self.assertEqual([w["index"] for w in fake.rows], [1, 2])
        self.assertEqual(len(fake.checks), 2)

    def test_refresh_rejects_visible_release_and_incorrect_result(self):
        for corrupt, error in (("visible", "occupied or visible"), ("indices", "index changes")):
            with self.subTest(corrupt=corrupt):
                fake = self.refresh_fixture(corrupt)
                with self.assertRaisesRegex(runner.TestFailure, error):
                    fake.refresh_mappings(fake.snapshot("baseline"))

    def test_destroy_created_preserves_survivor_indices(self):
        fake = FakeRunner()
        baseline = fake.snapshot("baseline")
        source = copy.deepcopy(fake.rows[1])
        message = fake.message
        calls = []

        def destroy(*args, **kwargs):
            if args[:2] == ("vspace", "--destroy"):
                calls.append(args)
                fake.rows = [w for w in fake.rows if w["index"] != args[2]]
                return SimpleNamespace(returncode=0, stdout="", stderr="")
            return message(*args, **kwargs)

        fake.message = destroy
        fake.destroy_created([source], baseline)
        self.assertEqual(calls, [("vspace", "--destroy", 2)])
        self.assertEqual([w["index"] for w in fake.rows], [1, 3, 4])
        self.assertEqual(runner.desktop_state(fake.state), runner.desktop_state(baseline))

    def test_destroy_created_refuses_populated_or_changed_mapping(self):
        for changed in (False, True):
            fake = FakeRunner()
            baseline = fake.snapshot("baseline")
            source = copy.deepcopy(fake.rows[1])
            if changed:
                fake.rows[1]["id"] += 1
            else:
                fake.state["spaces"][1]["windows"] = [999]
            with self.subTest(changed=changed), self.assertRaises(runner.TestFailure):
                fake.destroy_created([source], baseline)

    def test_append_preserves_gaps_and_separates_ids(self):
        snap = fixture()
        rows = [mapping(7, 100, 10), mapping(12, 300, 20)]
        new = mapping(13, 200, 10)
        new["id"] = 900
        result, first = runner.check_append(rows, rows + [new], snap, snap["displays"][0])
        self.assertEqual(result["id"], 900)
        self.assertTrue(first)

    def test_append_rejects_survivor_changes_cross_display_and_focus(self):
        snap = fixture()
        rows = [mapping(1, 100, 10)]
        new = mapping(2, 200, 10)
        for field, value in (("display-id", 20), ("has-focus", True), ("index", 9), ("id", 11)):
            broken = dict(new, **{field: value})
            with self.subTest(field=field), self.assertRaises(runner.TestFailure):
                runner.check_append(rows, rows + [broken], snap, snap["displays"][0])
        with self.assertRaises(runner.TestFailure):
            runner.check_append(rows, [mapping(9, 100, 10), new], snap, snap["displays"][0])

    def test_public_query_candidate_is_not_claimed_empty(self):
        snap = fixture()
        extra = copy.deepcopy(snap["spaces"][1])
        extra.update(id=250, index=5)
        snap["spaces"].append(extra)
        rows = [mapping(1, 100, 10)]
        new = mapping(2, 250, 10)
        _, first = runner.check_append(rows, rows + [new], snap, snap["displays"][0])
        self.assertFalse(first)  # Earlier unknown occupancy may legitimately be skipped.

    @patch.object(runner.time, "sleep")
    def test_focus_controls_restore_every_display_and_allow_unobserved_native_success(self, sleep):
        fake = FakeRunner()
        baseline = fake.snapshot("baseline")
        fake.native_hidden_observed = False
        fake.compare_focus("cross-hidden", baseline, baseline["spaces"][3])
        calls = [call for call in fake.focus_starts if call[1] == 400]
        self.assertEqual(len(calls), 2)
        self.assertEqual(calls[0][2], calls[1][2])
        self.assertEqual(calls[1][2][1], [100, 300])
        self.assertEqual(runner.focus_state(fake.state), runner.focus_state(baseline))
        self.assertEqual(next(c["observed"] for c in fake.checks if "observed" in c), [False, True])
        self.assertTrue(any(c["status"] == "unobserved" for c in fake.checks))

    def test_recovery_rejects_reused_focused_window(self):
        fake = FakeRunner()
        baseline = fake.snapshot("baseline")
        fake.state["windows"][0]["pid"] += 1
        with self.assertRaisesRegex(runner.TestFailure, "ownership"):
            fake.restore(baseline)

    def test_already_focused_is_testable_without_a_second_workspace(self):
        fake = FakeRunner()
        fake.rows = fake.rows[:1]
        fake.already_focused(fake.snapshot("baseline"))
        self.assertEqual(fake.checks[-1]["name"], "already-focused parity")

    def test_exhaustion_retains_mapping_with_other_display_candidates(self):
        fake = FakeRunner()
        fake.rows = [mapping(1, 100, 10)]
        message = fake.message
        fake.message = lambda *args, **kw: (SimpleNamespace(returncode=1, stdout="", stderr=
            "no eligible spare native desktop space on the current display.") if args[1] == "--create"
            else message(*args, **kw))
        before = fake.listing()
        fake.allocate(fake.state["displays"][0], [])
        self.assertEqual(fake.listing(), before)
        self.assertEqual(fake.checks[-1]["other_display_query_candidates"], {20: [400]})

    def test_allocation_runs_through_append_and_exhaustion(self):
        fake = FakeRunner()
        fake.rows = fake.rows[:1]
        created = []
        fake.allocate(fake.state["displays"][0], created)
        self.assertEqual([w["space-id"] for w in created], [200])
        self.assertEqual([w["index"] for w in fake.rows], [1, 2])
        self.assertTrue(fake.checks[-1]["name"].endswith("exhaustion/no mutation"))

    def test_layout_restores_setting_after_command_failure(self):
        fake = FakeRunner()
        calls = []

        def message(*args, **kw):
            calls.append(args)
            if args[-1] == "bsp":
                raise runner.TestFailure("layout failed")
            fake.state["spaces"][1]["type"] = args[-1]

        fake.message = message
        with self.assertRaisesRegex(runner.TestFailure, "layout failed"):
            fake.layout_smoke([fake.rows[1]], fake.snapshot("baseline"))
        self.assertEqual(calls[-1], ("space", "2", "--layout", "float"))


if __name__ == "__main__":
    unittest.main()
