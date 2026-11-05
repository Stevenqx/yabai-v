#!/usr/bin/env python3
"""Repeatable vspace checks. See docs/development.md before interactive use."""

import argparse
import hashlib
import json
import math
import os
from pathlib import Path
import platform
import plistlib
import subprocess
import sys
import tempfile
import time


ROOT = Path(__file__).resolve().parent.parent


class TestFailure(RuntimeError):
    pass


def require(condition, message):
    if not condition:
        raise TestFailure(message)


def active(snapshot):
    return next((s["id"] for s in snapshot["spaces"] if s["has-focus"]), 0)


def focused_window(snapshot):
    return next((w["id"] for w in snapshot["windows"] if w["has-focus"]), 0)


def focus_state(snapshot):
    return (active(snapshot), sorted(s["id"] for s in snapshot["spaces"]
                                     if s["is-visible"]), focused_window(snapshot))


def identities(rows):
    return [{k: v for k, v in row.items() if k != "has-focus"} for row in rows]


def desktop_state(snapshot):
    # Do not compare volatile window focus/visibility flags during Space switches.
    return {
        "displays": [(d["id"], d["uuid"], d["frame"], d["spaces"])
                     for d in snapshot["displays"]],
        "spaces": [(s["id"], s["index"], s["display"], s["is-native-fullscreen"])
                   for s in snapshot["spaces"]],
        "layouts": {s["id"]: s["type"] for s in snapshot["spaces"]},
        "windows": {w["id"]: {k: w[k] for k in ("pid", "frame", "space", "display")}
                    for w in snapshot["windows"]},
    }


def candidates(snapshot, rows, display_index):
    """Public-query candidates, NOT proof of native occupancy or eligibility."""
    mapped = {w["space-id"] for w in rows}
    return sorted((s for s in snapshot["spaces"]
                   if s["display"] == display_index and s["id"] not in mapped
                   and not s["is-visible"] and not s["is-native-fullscreen"]
                   and not s["windows"]), key=lambda s: s["index"])


def check_append(before, after, snapshot, display):
    require(len(after) == len(before) + 1 and after[:-1] == before,
            "allocation rewrote existing mappings, IDs, indices, or order")
    new = after[-1]
    require(new["id"] not in {w["id"] for w in before}, "allocation reused an ID")
    require(new["index"] == max(w["index"] for w in before) + 1,
            "allocation did not use highest index plus one")
    require(new["display-id"] == display["id"] and not new["has-focus"],
            "allocation crossed displays or changed focus")
    possible = candidates(snapshot, before, display["index"])
    require(new["space-id"] in {s["id"] for s in possible},
            "allocation selected a visible, occupied, mapped, fullscreen, or foreign Space")
    return new, possible[0]["id"] == new["space-id"]


class Runner:
    def __init__(self, args):
        self.args = args
        self.binary = Path(args.binary).expanduser().resolve()
        self.env = dict(os.environ, USER=args.user)
        if args.output:
            self.output = Path(args.output).expanduser().resolve()
            self.output.mkdir(mode=0o700, parents=True, exist_ok=False)
        else:
            self.output = Path(tempfile.mkdtemp(prefix="yabai-v-tests-"))
        self.report = {
            "schema": 1, "mode": args.mode, "started": time.strftime("%Y-%m-%dT%H:%M:%S%z"),
            "host": {"system": platform.system(), "version": platform.mac_ver()[0],
                     "architecture": platform.machine()},
            "options": vars(args),
            "binary": str(self.binary), "socket_user": args.user,
            "checks": [], "commands": [], "snapshots": {}, "completed": False,
        }
        print("Results: " + str(self.output / "report.json"), flush=True)
        self.save()

    def save(self):
        path = self.output / "report.json"
        temporary = self.output / "report.tmp"
        temporary.write_text(json.dumps(self.report, indent=2) + "\n")
        temporary.replace(path)

    def record(self, name, status="passed", **details):
        self.report["checks"].append(dict(name=name, status=status, **details))
        self.save()
        print("{}: {}".format(status.upper(), name), flush=True)
        if status == "failed":
            print(details.get("error", "See report for details."), file=sys.stderr, flush=True)

    def run(self, argv, check=True, timeout=20, env=None):
        started = time.monotonic()
        row = {"argv": [str(a) for a in argv]}
        try:
            result = subprocess.run(row["argv"], cwd=ROOT, env=env, text=True,
                                    capture_output=True, timeout=timeout)
            row.update(returncode=result.returncode, stdout=result.stdout, stderr=result.stderr)
        except subprocess.TimeoutExpired as error:
            row.update(timeout=True, stdout=str(error.stdout or ""), stderr=str(error.stderr or ""))
            raise TestFailure("command timed out: " + repr(row["argv"])) from error
        finally:
            row["seconds"] = round(time.monotonic() - started, 3)
            self.report["commands"].append(row)
            self.save()
        if check:
            require(result.returncode == 0, "command failed: {}\n{}".format(
                row["argv"], result.stdout + result.stderr))
        return result

    def message(self, *args, check=True):
        return self.run([self.binary, "-m", *map(str, args)], check=check, env=self.env)

    def listing(self):
        return json.loads(self.message("vspace", "--list").stdout)

    def snapshot(self, name):
        value = {k: json.loads(self.message("query", "--" + k).stdout)
                 for k in ("displays", "spaces", "windows")}
        require(all(isinstance(v, list) for v in value.values()), "native query was not an array")
        self.report["snapshots"][name] = value
        self.save()
        return value

    def metadata(self):
        require(platform.system() == "Darwin", "these checks require macOS")
        self.report["source"] = self.run(["git", "rev-parse", "HEAD"]).stdout.strip()
        self.report["working_tree"] = self.run(["git", "status", "--short"]).stdout
        for command in (["sw_vers"], ["xcrun", "--sdk", "macosx", "--show-sdk-version"],
                        ["csrutil", "status"]):
            self.run(command)
        self.save()

    def headless(self):
        if not self.args.skip_build:
            require(self.binary == ROOT / "bin/yabai", "build mode uses the checkout binary")
            self.run([ROOT / "scripts/build-dev.sh"], timeout=300)
            self.record("build and sign")
        else:
            self.record("build", "skipped", reason="--skip-build; testing the existing binary")
        self.run(["codesign", "--verify", "--strict", "--all-architectures", self.binary])
        self.run(["lipo", self.binary, "-verify_arch", "x86_64", "arm64"])
        self.run([self.binary, "--version"])
        help_result = self.run([self.binary, "--help"])
        require("--create" in help_result.stdout, "binary help is missing vspace creation")
        require("--destroy SOURCE [--to TARGET]" in help_result.stdout, "binary help is missing vspace destruction")
        require("--refresh" in help_result.stdout, "binary help is missing explicit vspace refresh")
        self.report["binary_sha256"] = hashlib.sha256(self.binary.read_bytes()).hexdigest()
        self.record("universal binary, signature, version, and help")
        self.run(["make", "-C", "tests"], timeout=300)
        self.record("production headless suite (including vspace safety fixtures)")
        self.run([sys.executable, "-B", "-m", "unittest", "discover", "-s", "tests/scripts", "-v"])
        self.record("runner regression suite")
        self.run(["sh", "-n", "scripts/build-dev.sh", "examples/yabairc.dev"])
        self.run(["git", "diff", "--check"])
        self.run(["git", "show", "--format=", "--check", "HEAD"])
        self.record("shell syntax and whitespace")
        self.record("daemon and visual checks", "skipped", reason="headless mode")

    def rejected(self, args, expected):
        before = self.listing()
        result = self.message(*args, check=False)
        require(result.returncode != 0 and not result.stdout and expected in result.stderr,
                "unexpected error response: {}: {}".format(args, result.stderr))
        require(self.listing() == before, "rejected command mutated the mapping")

    def query_checks(self):
        self.run([self.binary, "--version"])
        self.report["binary_sha256"] = hashlib.sha256(self.binary.read_bytes()).hexdigest()
        ping = self.message("vspace", "--ping").stdout
        require(ping == '{"status":"ok","component":"vspace"}\n', "ping response changed")
        rows = self.listing()
        require(isinstance(rows, list) and rows, "no virtual workspace mappings")
        for field in ("id", "index", "space-id"):
            require(all(type(w[field]) is int and w[field] > 0 for w in rows)
                    and len({w[field] for w in rows}) == len(rows), "invalid/duplicate " + field)
        require([w["index"] for w in rows] == sorted(w["index"] for w in rows), "mapping order changed")
        snap = self.snapshot("query-baseline")
        self.report["display_count"] = len(snap["displays"])
        displays = {d["index"]: d["id"] for d in snap["displays"]}
        spaces = {s["id"]: s for s in snap["spaces"]}
        for row in rows:
            require(row["space-id"] in spaces and
                    displays.get(spaces[row["space-id"]]["display"]) == row["display-id"],
                    "stale native mapping; use a fresh daemon session for live tests")
            require(row["has-focus"] == (row["space-id"] == active(snap)), "incorrect has-focus")
        current = self.message("vspace", "--current", check=False)
        expected = next((w for w in rows if w["space-id"] == active(snap)), None)
        if expected:
            require(current.returncode == 0 and json.loads(current.stdout) == expected,
                    "current does not match native focus")
        else:
            require(current.returncode != 0 and "not mapped" in current.stderr,
                    "unmapped current Space should fail")
        self.record("ping/list/current and native display/Space/window queries")
        for args, error in [
            (("vspace",), "unknown command"),
            (("vspace", "--unknown"), "unknown command"),
            (("vspace", "--ping", "extra"), "unexpected argument"),
            (("vspace", "--list", "extra"), "unexpected argument"),
            (("vspace", "--current", "extra"), "unexpected argument"),
            (("vspace", "--create", "extra"), "unexpected argument"),
            (("vspace", "--refresh", "extra"), "unexpected argument"),
            (("vspace", "--focus", "1", "extra"), "unexpected argument"),
            (("vspace", "--destroy", "1", "extra"), "unexpected argument"),
            (("vspace", "--destroy", "1", "--to"), "not a valid virtual workspace index"),
        ]:
            self.rejected(args, error)
        for value in ("", "0", "-1", "+1", "1.0", "prev", "1x", "2147483648"):
            self.rejected(("vspace", "--focus", value), "not a valid virtual workspace index")
            self.rejected(("vspace", "--destroy", value), "not a valid virtual workspace index")
        missing = next(i for i in range(1, len(rows) + 2) if i not in {w["index"] for w in rows})
        self.rejected(("vspace", "--focus", str(missing)), "could not locate")
        self.rejected(("vspace", "--destroy", str(missing)), "could not locate")
        self.record("CLI rejection and no-mutation regressions")
        self.record("stale/disconnected mapping and uncertain occupancy injection", "skipped",
                    reason="covered by headless fixtures; live runner does not alter topology or AX state")
        return snap

    def interactive_preflight(self):
        require(sys.stdin.isatty(), "interactive mode requires a terminal")
        self.run(["codesign", "--verify", "--strict", "--all-architectures", self.binary])
        signature = self.run(["codesign", "-d", "--verbose=4", self.binary])
        require("Signature=adhoc" not in signature.stdout + signature.stderr,
                "interactive use requires a persistent signing identity")
        sip = self.run(["csrutil", "status"]).stdout
        require("enabled" in sip.lower() and "disabled" not in sip.lower()
                and "unknown" not in sip.lower() and "custom" not in sip.lower(),
                "interactive tests require SIP fully enabled")
        processes = self.run(["pgrep", "-x", "yabai"], check=False)
        require(processes.returncode == 0 and len(processes.stdout.split()) == 1,
                "run exactly one signed checkout daemon; this script never changes services")
        pid = int(processes.stdout.strip())
        executable = self.run(["ps", "-p", str(pid), "-o", "comm="]).stdout.strip()
        require(Path(executable).resolve() == self.binary, "running daemon is not --binary")
        self.report["daemon_pid"] = pid
        raw = self.run(["ioreg", "-n", "Root", "-d1", "-a"]).stdout
        root = plistlib.loads(raw.encode())
        if isinstance(root, list):
            root = root[0]
        session = next((s for s in root.get("IOConsoleUsers", [])
                        if s.get("kCGSSessionUserIDKey") == os.getuid()
                        and s.get("kCGSSessionOnConsoleKey")), None)
        self.report["console_session"] = session
        self.save()
        require(session and session.get("kCGSessionLoginDoneKey") is True
                and not session.get("CGSSessionScreenIsLocked", False),
                "unlock the Mac manually and complete console login")
        # Config accepts 'off' for FFM, but its native query returns 'disabled'.
        for option, disabled in (("focus_follows_mouse", "disabled"), ("mouse_follows_focus", "off")):
            require(self.message("config", option).stdout.strip() == disabled,
                    "use examples/yabairc.dev with " + option + " off before interactive tests")
        print("\nThis uses the ALREADY RUNNING checkout daemon. It will append session-local\n"
              "mappings through exhaustion on reachable displays, switch Spaces/windows,\n"
              "and cycle layouts only on newly allocated empty Spaces. It will restore\n"
              "the captured visible Spaces, focus and layout settings, then audit frames.\n"
              "Created mappings remain unless --destroy-created releases hidden empty ones.\n"
              "That opt-in may evacuate windows arriving during its native preflight.\n"
              "--refresh-mappings explicitly releases ALL eligible hidden empty mappings\n"
              "and renumbers survivors, including mappings present before this run.\n"
              "No service/config is changed.\n"
              "Use a disposable test session; leave keyboard/mouse idle during the run.\n"
              "Ctrl-C requests recovery. Keep the report private (window metadata).")
        require(input("Type RUN to begin (anything else cancels): ").strip() == "RUN", "cancelled")

    def observe(self, sid):
        deadline = time.monotonic() + self.args.observe_seconds
        while time.monotonic() < deadline:
            spaces = json.loads(self.message("query", "--spaces").stdout)
            if active({"spaces": spaces}) == sid:
                return True
            time.sleep(0.2)
        return False

    def setup_focus(self, sid):
        # Retries are setup/recovery only. Each comparison dispatches exactly once.
        for _ in range(3):
            now = self.snapshot("last-setup")
            if active(now) == sid:
                time.sleep(0.5)
                return True
            target = next((s for s in now["spaces"] if s["id"] == sid), None)
            require(target, "setup/recovery Space disappeared: " + str(sid))
            result = self.message("space", "--focus", target["index"], check=False)
            if result.returncode == 0 and self.observe(sid):
                time.sleep(0.5)
                return True
        return False

    def restore(self, baseline):
        for old in baseline["spaces"]:
            if old["is-visible"]:
                now = self.snapshot("last-recovery")
                target = next((s for s in now["spaces"] if s["id"] == old["id"]), None)
                require(target, "original visible Space disappeared")
                if not target["is-visible"]:
                    require(self.setup_focus(old["id"]), "could not restore a visible Space")
        require(self.setup_focus(active(baseline)), "could not restore original active Space")
        wid = focused_window(baseline)
        if wid:
            now = self.snapshot("last-recovery")
            prior = next(w for w in baseline["windows"] if w["id"] == wid)
            survivor = next((w for w in now["windows"] if w["id"] == wid), None)
            require(survivor and all(survivor[k] == prior[k] for k in ("pid", "space", "display")),
                    "original focused window disappeared or changed ownership/association")
            if focused_window(now) != wid:
                self.message("window", "--focus", wid)
                time.sleep(0.5)
        require(focus_state(self.snapshot("last-recovery")) == focus_state(baseline),
                "visible Spaces or Space/window focus were not fully restored")

    def allocate(self, display, created):
        anchor = next(s for s in self.snapshot("allocation-setup")["spaces"]
                      if s["display"] == display["index"] and s["is-visible"])
        if not self.setup_focus(anchor["id"]):
            self.record("allocation on display " + str(display["id"]), "skipped",
                        reason="native setup dispatch did not produce observed display focus")
            return
        bound = len(self.snapshot("allocation-start")["spaces"]) + 1
        for step in range(bound):
            label = "display-{}-create-{}".format(display["id"], step)
            before = self.snapshot(label + "-before")
            require(next(d["id"] for d in before["displays"] if d["has-focus"]) == display["id"],
                    "focused display changed during allocation")
            rows = self.listing()
            before_current = self.message("vspace", "--current", check=False)
            result = self.message("vspace", "--create", check=False)
            after_rows = self.listing()
            after_current = self.message("vspace", "--current", check=False)
            after = self.snapshot(label + "-after")
            require(desktop_state(before) == desktop_state(after)
                    and focus_state(before) == focus_state(after), "allocation changed native state")
            require((before_current.returncode, before_current.stdout, before_current.stderr) ==
                    (after_current.returncode, after_current.stdout, after_current.stderr),
                    "allocation changed the current workspace response")
            if result.returncode:
                require("no eligible spare native desktop space on the current display." in result.stderr,
                        "unexpected allocation failure: " + result.stderr)
                require(not result.stdout and after_rows == rows, "failed allocation mutated mapping")
                other = {d["id"]: [s["id"] for s in candidates(before, rows, d["index"])]
                         for d in before["displays"] if d["id"] != display["id"]}
                self.record(label + " exhaustion/no mutation", other_display_query_candidates=other,
                            note="public queries cannot prove candidate occupancy; headless fixtures do")
                return
            require(not result.stdout and not result.stderr, "successful allocation emitted output")
            new, first = check_append(rows, after_rows, before, display)
            created.append(new)
            self.record(label + " append/invariants", mapping=new)
            self.record(label + " native order", "passed" if first else "skipped",
                        reason="first public-query candidate selected" if first else
                        "earlier query candidates may have uncertain native occupancy; deterministic fixture covers order")
        raise TestFailure("allocation did not reach exhaustion within the native Space count")

    def compare_focus(self, label, baseline, target):
        rows = self.listing()
        results = []
        observations = []
        starts = []
        for domain, index in (("space", target["index"]),
                              ("vspace", next(w["index"] for w in rows if w["space-id"] == target["id"]))):
            self.restore(baseline)  # Restore visibility on EVERY display between controls.
            start = self.snapshot(label + "-" + domain + "-start")
            starts.append(focus_state(start))
            require(desktop_state(start) == desktop_state(baseline), "desktop changed before focus control")
            result = self.message(domain, "--focus", index, check=False)
            observed = self.observe(target["id"]) if result.returncode == 0 else False
            after = self.snapshot(label + "-" + domain + "-after")
            require(desktop_state(after) == desktop_state(baseline), "focus changed windows/layouts/topology")
            require(identities(self.listing()) == identities(rows), "focus rewrote virtual mappings")
            if observed:
                require(json.loads(self.message("vspace", "--current").stdout)["space-id"] == target["id"],
                        "current does not match observed focus")
            results.append(result)
            observations.append(observed)
            time.sleep(self.args.hold_seconds)
        require(starts[0] == starts[1], "native and virtual controls had different starting visibility/focus")
        require(results[0].returncode == results[1].returncode, "native/virtual focus result mismatch")
        require(all(not r.stdout for r in results), "focus emitted unexpected output")
        if results[0].returncode:
            # Compare error categories, not wording containing native/virtual nouns.
            errors = ("already focused", "animation", "mission-control", "scripting-addition")
            require(any(all(e in r.stderr for r in results) for e in errors), "focus error category mismatch")
        self.record(label + " native/virtual result parity", returncodes=[r.returncode for r in results],
                    observed=observations, identical_starting_focus=True)
        self.record(label + " queried activation", "passed" if all(observations) else "unobserved",
                    note="observations are separate from native dispatch status under SIP")
        self.restore(baseline)

    def already_focused(self, baseline):
        self.restore(baseline)
        rows = self.listing()
        source = next(w for w in rows if w["space-id"] == active(baseline))
        native = next(s for s in baseline["spaces"] if s["id"] == active(baseline))
        n = self.message("space", "--focus", native["index"], check=False)
        v = self.message("vspace", "--focus", source["index"], check=False)
        require(n.returncode != 0 and n.returncode == v.returncode
                and all(not r.stdout and "already focused" in r.stderr for r in (n, v)),
                "already-focused result mismatch")
        require(self.listing() == rows and focus_state(self.snapshot("already-focused")) == focus_state(baseline),
                "already-focused request mutated mapping or focus")
        self.record("already-focused parity")

    def layout_smoke(self, created, baseline):
        if not created:
            self.record("empty-Space layout smoke", "skipped", reason="no newly allocated empty Spaces")
            return
        for row in created:
            snap = self.snapshot("layout-before-" + str(row["space-id"]))
            space = next(s for s in snap["spaces"] if s["id"] == row["space-id"])
            require(not space["windows"] and not any(w["space"] == space["index"] for w in snap["windows"]),
                    "new allocation gained windows; refusing layout smoke")
            original = space["type"]
            try:
                for layout in ("float", "bsp", "stack"):
                    current = self.snapshot("layout-preflight")
                    require(desktop_state(current)["spaces"] == desktop_state(snap)["spaces"],
                            "native Space order changed during layout smoke")
                    require(not next(s["windows"] for s in current["spaces"] if s["id"] == space["id"])
                            and not any(w["space"] == space["index"] for w in current["windows"]),
                            "allocation gained windows during layout smoke")
                    self.message("space", str(space["index"]), "--layout", layout)
                    now = self.snapshot("layout-{}-{}".format(space["id"], layout))
                    require(next(s["type"] for s in now["spaces"] if s["id"] == space["id"]) == layout,
                            "layout command did not take effect")
            finally:
                now = self.snapshot("layout-recovery")
                survivor = next((s for s in now["spaces"] if s["id"] == space["id"]), None)
                require(survivor, "layout recovery Space disappeared")
                self.message("space", str(survivor["index"]), "--layout", original)
            require(desktop_state(self.snapshot("layout-restored")) == desktop_state(baseline),
                    "layout smoke changed desktop state")
            self.record("empty-Space float/BSP/stack " + str(space["id"]))
        self.record("populated layout smoke", "skipped", reason="requires separately prepared disposable windows")

    def destroy_created(self, created, baseline):
        self.restore(baseline)
        for source in reversed(created):
            before = self.listing()
            if source not in before:
                raise TestFailure("created mapping changed before destroy")
            snap = self.snapshot("destroy-preflight-{}".format(source["id"]))
            space = next(s for s in snap["spaces"] if s["id"] == source["space-id"])
            require(not space["is-visible"] and not space["windows"] and
                    not any(w["space"] == space["index"] for w in snap["windows"]),
                    "destroy smoke requires a hidden allocation with no observed windows")
            local = [w for w in before if w["display-id"] == source["display-id"] and w["id"] != source["id"]]
            if not local:
                self.record("destroy allocation " + str(source["id"]), "skipped", reason="no local mapped destination")
                continue
            self.message("vspace", "--destroy", source["index"])
            require(self.listing() == [w for w in before if w["id"] != source["id"]],
                    "destroy changed survivor IDs, indices, order or focus")
            require(desktop_state(self.snapshot("destroy-after-{}".format(source["id"]))) == desktop_state(baseline),
                    "hidden empty destroy changed the native desktop")
            self.record("destroy hidden empty allocation " + str(source["id"]))

    def refresh_mappings(self, baseline):
        for attempt in range(2):
            before = self.listing()
            snap = self.snapshot("refresh-before-{}".format(attempt))
            result = json.loads(self.message("vspace", "--refresh").stdout)
            require(isinstance(result, dict) and set(result) == {"released", "reindexed", "retained"},
                    "invalid refresh result")
            after = self.listing()
            released = {w["id"] for w in result["released"]}
            survivors = [w for w in before if w["id"] not in released]
            expected = [dict(w, index=i + 1) for i, w in enumerate(survivors)]
            require(after and after == expected, "refresh changed survivor identity, mapping, order or focus")
            require(result["released"] == [{k: w[k] for k in ("index", "id", "space-id", "display-id")}
                                           for w in before if w["id"] in released], "incorrect released mappings")
            require(result["reindexed"] == [{"id": w["id"], "old-index": w["index"], "index": i + 1}
                                            for i, w in enumerate(survivors) if w["index"] != i + 1],
                    "incorrect refresh index changes")
            reasons = {"focused", "visible", "occupied", "occupancy-unknown", "display-unavailable",
                       "mapping-unavailable", "last-workspace"}
            require([(w["id"], w["index"]) for w in result["retained"]] ==
                    [(w["id"], w["index"]) for w in after] and
                    all(w["reason"] in reasons for w in result["retained"]), "invalid retention diagnostics")
            protected = {s["id"] for s in snap["spaces"] if s["is-visible"] or s["has-focus"] or s["windows"]}
            require(not any(w["id"] in released and w["space-id"] in protected for w in before),
                    "refresh released an observed occupied or visible workspace")
            require(desktop_state(self.snapshot("refresh-after-{}".format(attempt))) == desktop_state(baseline),
                    "refresh changed the native desktop")
            if attempt:
                require(after == before and not released and not result["reindexed"],
                        "repeat refresh changed mappings; inspect native activity in the saved snapshots")
            self.record("explicit refresh" if not attempt else "settled refresh idempotence", result=result)

    def interactive(self):
        self.interactive_preflight()
        baseline = self.query_checks()
        require(focused_window(baseline), "no AX focused-window metadata; check Accessibility/unlocked session")
        self.report["snapshots"]["recovery-baseline"] = baseline
        self.save()
        created = []
        try:
            if self.args.skip_allocation:
                self.record("allocation", "skipped", reason="--skip-allocation")
            else:
                # Exhaust the smallest apparent pool before touching another display's spares.
                rows = self.listing()
                displays = sorted(baseline["displays"],
                                  key=lambda d: len(candidates(baseline, rows, d["index"])))
                for display in displays:
                    self.allocate(display, created)
            self.restore(baseline)
            rows = self.listing()
            mapped = {w["space-id"] for w in rows}
            source = next(s for s in baseline["spaces"] if s["id"] == active(baseline))
            possible = [s for s in baseline["spaces"] if s["id"] in mapped and s["id"] != source["id"]]
            groups = {
                "same-display": [s for s in possible if s["display"] == source["display"]],
                "cross-display-visible": [s for s in possible if s["display"] != source["display"] and s["is-visible"]],
                "cross-display-hidden": [s for s in possible if s["display"] != source["display"] and not s["is-visible"]],
            }
            for label, targets in groups.items():
                if self.args.skip_focus or not targets or source["id"] not in mapped:
                    self.record(label + " focus", "skipped", reason="disabled or no suitable mapped source/target")
                else:
                    self.compare_focus(label, baseline, targets[0])
            if not self.args.skip_focus and source["id"] in mapped:
                self.already_focused(baseline)
            if self.args.skip_layout:
                self.record("layout smoke", "skipped", reason="--skip-layout")
            else:
                self.layout_smoke(created, baseline)
            if self.args.destroy_created:
                self.destroy_created(created, baseline)
            else:
                self.record("destroy created mappings", "skipped", reason="requires --destroy-created")
            if self.args.refresh_mappings:
                self.refresh_mappings(baseline)
            else:
                self.record("explicit refresh", "skipped", reason="requires --refresh-mappings")
        finally:
            try:
                self.restore(baseline)
                after = self.snapshot("recovery-final")
                require(desktop_state(after) == desktop_state(baseline),
                        "window frames/associations, layouts, or native topology changed; inspect saved snapshots")
                self.record("recovery: all visible Spaces, focus, frames, layouts, native order")
            except (Exception, KeyboardInterrupt) as error:
                self.record("recovery", "failed", error=str(error),
                            action="use recovery-baseline in report.json; service recovery remains manual")
                raise
        answer = input("Were Space switches and the return visibly correct? [yes/no/unobserved]: ").strip().lower()
        self.record("user visual confirmation", "passed" if answer == "yes" else
                    "failed" if answer == "no" else "unobserved", answer=answer)


def parser():
    result = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="No mode starts/stops a daemon, installs files, changes SIP, or loads the scripting addition.\n"
               "Headless is the default. Interactive requires typed approval and a sole signed checkout daemon.")
    result.add_argument("mode", nargs="?", choices=("headless", "query", "interactive"), default="headless")
    result.add_argument("--binary", default=str(ROOT / "bin/yabai"))
    result.add_argument("--user", default=os.environ.get("USER", ""), help="daemon's USER/socket namespace")
    result.add_argument("--output", help="new private result directory (must not already exist)")
    result.add_argument("--skip-build", action="store_true", help="headless: verify existing binary without rebuilding")
    result.add_argument("--skip-allocation", action="store_true", help="interactive: skip mapping allocation")
    result.add_argument("--skip-focus", action="store_true", help="interactive: skip focus comparisons")
    result.add_argument("--skip-layout", action="store_true", help="interactive: skip empty-Space layout changes")
    result.add_argument("--destroy-created", action="store_true", help="interactive: release this run's hidden empty allocations after checks")
    result.add_argument("--refresh-mappings", action="store_true", help="interactive: explicitly refresh all mappings and compact indices (twice for idempotence)")
    result.add_argument("--observe-seconds", type=float, default=8, help="query observation budget per focus (default: 8)")
    result.add_argument("--hold-seconds", type=float, default=3, help="visual pause after each focus control (default: 3)")
    return result


def main():
    arg_parser = parser()
    args = arg_parser.parse_args()
    if (not math.isfinite(args.observe_seconds) or not math.isfinite(args.hold_seconds)
            or args.observe_seconds <= 0 or args.hold_seconds < 0):
        arg_parser.error("observation budget must be finite and positive; hold must be finite and nonnegative")
    runner = Runner(args)
    try:
        runner.metadata()
        if args.mode == "headless":
            runner.headless()
        elif args.mode == "query":
            runner.query_checks()
            runner.record("allocation/focus/layout/visual checks", "skipped", reason="read-only query mode")
        else:
            runner.interactive()
        runner.report["completed"] = True
    except (Exception, KeyboardInterrupt) as error:
        runner.record("run", "failed", error=str(error) or "interrupted")
    finally:
        runner.report["finished"] = time.strftime("%Y-%m-%dT%H:%M:%S%z")
        runner.save()
    failed = any(c["status"] == "failed" for c in runner.report["checks"])
    print("{}; report: {}".format("FAILED" if failed else "PASSED (review skipped/unobserved checks)",
                                  runner.output / "report.json"))
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
