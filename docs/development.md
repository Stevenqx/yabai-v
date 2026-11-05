# Development workflow

The GitHub fork is `Stevenqx/yabai-v`. The working checkout for this setup is
`/Users/steven/Workspace/Github/yabai-v`; the originally preferred location is
`~/Developer/yabai-fork`. Build and run binaries from the checkout; do not replace
a Homebrew binary or install a development service. See `architecture.md` for
scope and `validation.md` for observed results.

Read the [development progress index](progress/README.md) for current milestones,
decisions, and the task journal. Maintain those records as described in `AGENTS.md`.

## Repository setup

With Git and the GitHub CLI authenticated, create the fork and clone its complete
history (no `--depth` or source archives):

```sh
gh repo fork asmvik/yabai --fork-name yabai-v --clone=false
mkdir -p ~/Developer
git clone https://github.com/YOUR_ACCOUNT/yabai-v.git ~/Developer/yabai-fork
cd ~/Developer/yabai-fork
git remote add upstream https://github.com/asmvik/yabai.git
git fetch upstream --tags
git remote -v
git rev-parse --is-shallow-repository  # false
git switch -c feat/vspace-smoke upstream/master
git rev-parse upstream/master        # record the full base SHA
```

If the fork already exists, inspect it instead of repeating creation. If the
checkout was initially cloned from upstream, rename that remote to `upstream`
and add the fork URL as `origin`. This does not publish local commits. Push only
after reviewing the diff and receiving explicit confirmation.

To relocate this existing checkout after finishing the current workspace session,
first ensure its local work is committed and stop any foreground development
daemon. The initial prepared commits are already imported into this checkout.
Run the following manually only if the destination does not already exist:

```sh
if test ! -e ~/Developer/yabai-fork; then
    mkdir -p ~/Developer &&
        mv /Users/steven/Workspace/Github/yabai-v ~/Developer/yabai-fork
fi
```

Move the complete directory, including `.git`, to retain history and commits.
Use the new path for later builds and re-enable Accessibility for that binary if
macOS requests it. This move does not relocate the Homebrew installation.

For later updates, start with a clean working tree, fetch `upstream`, and rebase
unpublished feature commits onto `upstream/master`. Coordinate published history
instead of rewriting it automatically. Use small Conventional Commits, such as
`feat(vspace): add ping command` and `ci: check macOS 15 and 26 builds`.

## Toolchain and upstream baseline

```sh
sw_vers
uname -m
xcode-select -p
xcrun --find clang
xcrun clang --version
xcrun --sdk macosx --show-sdk-version
csrutil status
```

Xcode Command Line Tools are sufficient for the command-line build. Full Xcode
is optional; the Swift GUI project is deliberately deferred. If tools are
missing, install them manually with `xcode-select --install`.

Before modifying upstream, inspect `makefile`, `src/manifest.m`, and
`tests/makefile`, then run `make` and record the result and base SHA. The default
build is a universal x86_64/arm64 debug binary at `bin/yabai`, with `-g -O0`,
warnings, and the existing macOS deployment target. It uses Apple frameworks and
the existing embedded payload sources. No scripting addition is installed or
loaded by the build. Do not run `make clean` unnecessarily: upstream's target
also removes generated payload sources. The ordinary `make` already cleans the
binary directory and recompiles the single manifest translation unit.

## Persistent local signing

Create one local certificate manually in Keychain Access: Certificate Assistant
→ Create a Certificate → name `yabai-fork Development`, identity type
`Self Signed Root`, certificate type `Code Signing`. In that certificate's trust
settings, trust it for code signing. Use the login keychain and retain the same
certificate and private key across builds; do not export them into the repository.

```sh
security find-identity -v -p codesigning
./scripts/build-dev.sh
codesign --verify --strict --verbose=2 ./bin/yabai
codesign -d --verbose=4 -r- ./bin/yabai
./bin/yabai --version
```

The helper uses a fixed development identifier and defaults to this certificate.
Set `YABAI_FORK_SIGN_IDENTITY` to its SHA-1 identity if duplicate names exist.
Keep the checkout path, certificate, and identifier stable so Accessibility
authorization can survive rebuilds. Move to the preferred checkout location
before granting permissions. Recreate the certificate only deliberately.

For build-only CI, `YABAI_FORK_SIGN_IDENTITY=- ./scripts/build-dev.sh` explicitly
selects ad-hoc signing. That verifies executable integrity, but it does not
establish persistent local signing or a stable Accessibility identity.

## Tests and static checks

```sh
make -C tests
git diff --check
```

The vspace tests exercise the production manager and message dispatcher with a
deterministic native-query source, without initializing WindowServer managers.
They cover mapping/order, conservative occupancy, layout independence, duplicates,
session stability, lookup, JSON, native focus outcomes, strict input, exact ping,
and one/two-display allocation/exhaustion and revalidation failures. M2-B adds
evacuation/retry, visibility/focus handoff, stable survivor gaps, topology/window
races, native window-class policy, and deferred socket ownership regressions.
M2-C adds global interleaving/compaction, all-display visibility, conservative
retention, snapshot/arrival races, pending-destroy rejection and coherent
publication tests. Simulated displays do not establish behavior on physical displays.
The existing upstream tests run alongside them. These checks require neither
Accessibility permissions nor a running daemon. The standard runner below wraps
these checks and is also the CI entry point. Hosted runners do not validate an
interactive window-management session. Runner labels are documented in the
[GitHub runner images catalog](https://github.com/actions/runner-images).

## Standard vspace test script

Maintain [scripts/test-vspace.py](../scripts/test-vspace.py) as the shared test
entry point for developers and users on their own Macs. When a vspace command or
contract changes, update its deterministic fixtures, applicable live checks, and
this guide together. Keep host-specific IDs, paths, and service commands out of
the runner. It uses Python 3's standard library (available with the development
toolchain), the existing build/signing helper, and the existing C/Objective-C
tests; it adds no package dependency.

**Current validation policy:** focus ongoing development and runtime testing on
macOS 15. The user explicitly deferred macOS 26 tests on 2026-11-04; they are not
an ongoing local validation gate. Use this same script on macOS 26 or another
version when a suitable machine is available later. Compatibility targets remain
15/26, but a script's existence is not evidence of a pass on an untested OS.
Automatic push/PR CI now uses macOS 15. A manual `workflow_dispatch` can opt into
the retained macOS 26 build/headless job with `include_macos_26`; no GUI test runs
in hosted CI. Historical 15/26 CI results remain historical evidence.

From any directory, run the checkout script (examples below assume its root):

```sh
# Default: build with make, sign, verify universal binary, run all headless checks.
./scripts/test-vspace.py
# Reuse an already built/signed checkout binary.
./scripts/test-vspace.py headless --skip-build
# Build-only CI without a persistent signing identity; never use this for GUI tests.
YABAI_FORK_SIGN_IDENTITY=- ./scripts/test-vspace.py headless
```

Headless mode includes all 52 current production tests: mapping/order/identity,
JSON/parsing/native dispatch, one/two-display selection and exhaustion, no
cross-display fallback, floating and uncertain occupancy, visible/non-desktop
exclusion, missing state, stale mappings, and allocation revalidation. It also
runs the runner's orchestration regressions, signature/architecture/version/help,
shell syntax, and Git whitespace checks. It neither starts a daemon nor sends
IPC commands. Builds replace only the checkout binary; use the persistent local
certificate for later GUI work. `--skip-build` records that rebuilding was skipped.

For live checks, first prepare a sole compatible checkout daemon using the
[manual workflow](#manual-daemon-smoke-test) below, with persistent signing,
manual Accessibility permission, and SIP fully enabled. An older running daemon
does not acquire vspace commands just because the client is new. The runner
**never starts/stops services or daemons**, overwrites configuration/installation
files, loads the scripting addition, or changes macOS permissions. Service pause
and recovery remain manual, explicitly approved operations.

```sh
# Read-only contract/query checks against the already running development daemon.
./scripts/test-vspace.py query --user yabai-fork-dev
# Interactive test with typed approval, switching, allocation and empty layouts.
./scripts/test-vspace.py interactive --user yabai-fork-dev
# Focus comparison without allocating mappings or changing layouts.
./scripts/test-vspace.py interactive --user yabai-fork-dev --skip-allocation --skip-layout
```

`query` checks exact ping, list/current/native queries, invalid commands/indices,
and rejected requests without mapping mutation. It sends no valid create/focus/
layout request. Use `--binary PATH` and `--user NAME` to select the client and
existing daemon's socket namespace; neither option isolates window management.

`interactive` requires a terminal, exactly one daemon running the selected
checkout executable, a persistent signature, an unlocked completed console
session, usable AX focus metadata, and both mouse-focus options off (as in
`examples/yabairc.dev`). Read the printed plan, type `RUN`, and leave the desktop,
keyboard and mouse idle until prompted for separate visual confirmation. Allow
several minutes; displays/Spaces and native focus observations determine duration.
Use existing disposable test Spaces/windows in a suitable login session.

The interactive stages capture a fresh recovery baseline; exhaust the smallest
apparent spare pool first, then try each connected display; verify append-only
IDs/indices/order and native frame/layout/focus invariants; compare same-display,
visible cross-display, and hidden cross-display native/virtual focus where
targets exist; check already-focused errors; and cycle float/BSP/stack only on
newly allocated, still-empty Spaces. All displays' visible Spaces and global
Space/window focus are restored between focus controls, giving identical starting
visibility. Each comparison dispatches once; setup/recovery retries are separate.
Native return status, observed activation, and user visual confirmation are
recorded separately. A native SIP gesture can return success without observed
keyboard focus; that is reported as `unobserved`, not changed into a vspace timeout.

Successful `--create` tests leave their mappings allocated for that daemon
session by default. Opt into M2-B hidden-empty release checks with
`interactive --destroy-created`: after focus/layout recovery, it destroys only
this run's unchanged, hidden allocations with no observed windows and a local
mapped destination, checking exact survivor IDs/indices/order and native desktop
invariants. The daemon still uses authoritative native occupancy; a window arriving
after the runner's snapshot can be evacuated. Use disposable Spaces and keep the
desktop idle. Missing local destinations are recorded as skipped. The script does not
create/delete/reorder native Spaces or move windows to manufacture cases. Public
window queries cannot prove native emptiness: apparent spare candidates and
other-display candidates are recorded as observations. Exact deterministic
ordering, uncertain/incomplete AX state, stale mappings, and missing displays
are covered by the production headless fixtures; they are not fabricated live.
Unavailable targets are skipped explicitly. Use `--skip-allocation`, `--skip-focus`,
or `--skip-layout` to omit stages; display setup/recovery can still switch focus.
`--observe-seconds` and `--hold-seconds` adjust observation/visual pauses only.
Run on both one- and two-display configurations later for separate physical
coverage. Populated layout checks still require prepared disposable windows and
the manual procedure below; empty-view smoke is not populated tiling coverage.

Opt into M2-C with `interactive --refresh-mappings`. This explicitly sends
`vspace --refresh` twice after allocation/focus/layout and optional destroy checks.
It may release **all** eligible mappings, including preexisting ones, and compact
their global indices. It verifies survivor ID/mapping/order, exact release and
index-change reports, retention diagnostics, observed occupied/visible protections,
native desktop invariants and settled-state idempotence. Refresh never runs in
default headless/query/interactive checks. `--skip-allocation --skip-focus
--skip-layout --refresh-mappings` isolates this stage (setup/recovery still applies).
Only the daemon's native scanner proves emptiness; public queries supplement it.
The report keeps the exact refresh results. Use current `--list` indices afterward.

Recovery runs in `finally`, including Ctrl-C during the test stages. It restores
captured visibility/focus and each touched layout, then audits native order,
window lifetimes/associations, exact frames, and layouts. It refuses to focus a
reused window ID with a changed PID/association. It does not recreate closed
windows or guess how to repair a changed desktop; a recovery failure is reported
with baseline/final snapshots. After manually restoring an original service,
perform the separate original-installation recovery audit below: the script's
baseline describes the already running **development daemon**, not a previous
service. A killed process or lost daemon can prevent automatic recovery.

Every run prints a private result-directory path containing `report.json`.
`--output /path/to/NEW-directory` chooses it explicitly; existing directories are
refused to avoid overwriting evidence. Reports contain host/OS/SDK/architecture,
source SHA and dirty-tree status, binary version/hash, commands/outputs/statuses,
passed/failed/skipped/unobserved checks and, for live modes, window metadata and
snapshots. Review/redact them before sharing. Exit 1 means a failure (including
recovery or negative visual confirmation); exit 0 can include skipped/unobserved
checks and does not establish complete GUI or cross-version compatibility. Check
`completed` and each check's status. Archive results with the tested commit and
OS/display configuration; temporary paths are not durable project evidence.

## Manual daemon smoke test

First inspect the existing service and confirm its executable, configuration,
and current state. Verify an unlocked, completed console session and usable AX
window/focus metadata before pausing it. Keep keyboard/mouse activity idle during
focus comparisons. On this host the console lock key is absent after unlock;
record raw state rather than treating every missing key as a locked session.
Never stop or replace it without explicit approval. If it is
running, defer the real daemon test until an approved test session is available.
Do not run two window-managing daemons against the same desktop, even if their
sockets differ. Prefer a separate macOS login session for manual testing.

Sign the development binary and grant that exact binary Accessibility permission
in System Settings → Privacy & Security → Accessibility. Keep SIP enabled and
do not load the scripting addition. Screen Recording is unnecessary for this
test because the development config disables animations.

Once no competing daemon is running, launch in a terminal from the checkout:

```sh
USER=yabai-fork-dev ./bin/yabai --verbose --config "$PWD/examples/yabairc.dev"
```

This selects dedicated socket/lock names and an explicit development config.
It does not install a service. In another terminal in the same checkout:

```sh
USER=yabai-fork-dev ./bin/yabai -m vspace --ping
# {"status":"ok","component":"vspace"}
USER=yabai-fork-dev ./bin/yabai -m vspace --list
USER=yabai-fork-dev ./bin/yabai -m vspace --current
USER=yabai-fork-dev ./bin/yabai -m query --displays
USER=yabai-fork-dev ./bin/yabai -m query --spaces
USER=yabai-fork-dev ./bin/yabai -m query --windows
```

Check that the native queries and vspace list return JSON arrays, and vspace
current matches the focused native Space ID. Confirm all development config
commands succeed; `window_animation_duration` requires the float token `0.0`,
and the existing parser rejects the integer token `0`. See [vspace](vspace.md)
for fields, occupancy policy, and unmapped-current behavior. Check that `vspace`,
`vspace --unknown`, and `vspace --ping extra` exit unsuccessfully. If a disposable
window and a safe GUI session are available, query its ID and use
`USER=yabai-fork-dev ./bin/yabai -m window --focus ID`. Replace `ID` with the
integer `id` from the query output before running the command; verify focus
visually and restore the original focus afterward.
Switch between existing Spaces with Mission Control or the system shortcuts and
query `--spaces` again. Optionally test `space --focus INDEX` on an existing
Space: this upstream base has a gesture fallback that does not require the
scripting addition. Verify the visible result and query it again, then restore
the original Space. Do not create/delete native Spaces or load the scripting
addition; do not substitute `space --switch`, whose implementation differs.

For Milestone 1, allow native transitions and startup/config commands to finish
before the first focus call. Keep manual Space switching and other input idle
during the observed sequence; record an interrupted attempt separately from a
clean repeat. First record original visible native Spaces and focused window,
then choose a mapped existing destination from `vspace --list`. Use its virtual
`index` with `vspace --focus N`, observe it for five seconds, and check `--current`,
`--list`, and native `query --spaces`. Restore the original mapped workspace by
its virtual index and verify the return visually. If startup's globally active
Space was empty, confirm it was still allocated. Compare native window IDs,
Space associations, frames, and layout settings before/after; record intentional
focus/visibility changes separately. Never use a Mission Control index as a
virtual index or allocate/move windows merely to make the test pass.

For M2-A, capture list/current, native display/Space/window queries, and existing
IDs/indices before `vspace --create`. Confirm native enumeration order selects
the first eligible hidden empty desktop on the focused display. Repeat to
exhaustion, checking that failures do not mutate mappings and empty allocations
stay mapped. On two displays, exhaust the focused display while another still
has a spare; creation must fail. Switch focused display and repeat there. Check
visible unmapped, fullscreen, occupied floating-window, and unknown-state
candidates using safe existing state or headless fixtures; do not change the
user's desktop merely to manufacture cases.

Compare native `space --focus` and virtual `vspace --focus` for the same native
ID, including already-focused and cross-display targets. Restore all displays'
visible Spaces as well as global focus between controls when comparing identical
starting conditions. A native call can make a target visible while keyboard
focus remains on another display; restoring only source focus leaves a different
visibility setup for the next call. M2-A returns native
results without polling/timeout; observe switching separately through queries
and visually after allowing transitions to settle. Test bad/stale indices,
ping/list/current, original queries, and float/BSP/stack with disposable windows
where approved. Restore original visible Spaces, focus, service, frames, layouts,
and installation hashes. Report live results separately from native read-only
probes, deterministic fixtures, and hosted CI. See the current
[M2-A validation plan](validation.md#milestone-2-a-allocation-and-local-validation--2026-11-04).

For M2-B, use fresh prepared disposable windows and existing native desktops in
an approved sole-checkout-daemon session. First capture the original service,
native topology/visibility, window IDs/PIDs/frames/associations/states, layouts,
and all virtual IDs/indices. Check strict destroy parsing, missing/same target,
last-workspace protection, local preceding/following selection with interleaved
displays, and cross-display missing-replacement rejection without effects.
Use hidden empty sources for release/reuse checks; the runner's optional
`--destroy-created` stage automates that limited case.

Frame queries need particular care across daemon restarts: tracked
`query --windows` frames come from yabai's event-updated cache. Capture independent
WindowServer bounds, owner PIDs and exact native Space memberships before pausing
the service and again during recovery. Preserve cached and native observations
separately; a fresh daemon reporting a different cached frame does not establish
that the physical window moved. Do not force a window to an old cached frame
without independent evidence. Record per-Space padding and the display's menu-bar/
Dock work area when diagnosing geometry. On the tested host, 10-pixel padding
and a 35-pixel menu bar on a 1352×878 display yield `(10,45,1332,823)` exactly.
The [M2-B GUI report](validation.md#milestone-2-b-approved-gui-validation--2026-11-05)
records the initial inconclusive cache comparison and the measured repeat.

Separately test mixed ordinary tiled/floating windows, minimized windows and
hidden applications with float/BSP/stack destinations. Verify all exact native
memberships, unchanged minimize/hidden/float state where supported, and managed
layout queries after completion. Check a globally focused source and a source
visible on the other display: the latter must show its local mapped replacement
and recover original global Space/window focus. Compare native cross-display
window placement rather than assuming geometry preservation. Sticky/child/
fullscreen/unknown source state must retain the mapping. Source and destination
native desktop restrictions are in [the CLI contract](vspace.md).

Use deterministic fixtures for failed move, incomplete native query, reused ID,
new arrival and disconnect races; do not manufacture unsafe user-desktop faults.
Observe nonzero partial results and retained mappings, then retry the remaining
windows; do not equate dispatch success with evacuation or visibility. Native
requests may complete after a reported failure. Capture exact results in the
private report and record skipped scenarios. Restore disposable window
memberships/frames/states and original visibility/focus/layouts before the
separate original-service recovery audit. The runner deliberately does not
automatically populate, minimize/hide, or evacuate existing user workspaces.
See [M2-B evidence and remaining gates](validation.md#milestone-2-b-evacuation-and-release--2026-11-05).

For M2-C, capture mappings and native topology/visibility/focus/windows/layouts
before `--refresh`. Allocate hidden empty spares explicitly if available, keep
empty visible mappings on both displays, and observe floating/minimized/hidden
windows without moving user windows to manufacture cases. Verify exact survivor
IDs/native mappings/global order and compact indices, per-workspace diagnostics,
unchanged native state, repeated refresh, and fresh-ID create/focus afterward.
The maintained runner's `--refresh-mappings` automates report/invariant/idempotence
checks; pending destroy and unsafe native query/topology races use headless
fixtures. Recheck current indices after compaction. An original-service pause
still requires explicit approval and an independent measured recovery baseline.

Stop only the foreground development process with Ctrl-C. An existing Homebrew
binary, service plist, and user configuration must remain intact. If the original
service was stopped during an approved test, restore its prior state explicitly
and verify its queries; do not leave recovery to an automated cleanup guess.
Original-service startup can reapply existing rules and change floating-window
frames. If recovery needs to restore them, validate surviving PID/Space/display
associations and restore only captured frames through existing native commands;
verify actual results because application size constraints may require restoring
position before a final size adjustment. Do not recreate closed user windows.

Verify the original visible Spaces and focused window again after the original
service starts: startup can change focus even if it was restored before stopping
the development daemon. Keep visual confirmation distinct from command exits
and queried state, and leave disposable test windows open when a repeat check
has been requested until its result is confirmed.

## Debugging

Use the signed `bin/yabai` and its default debug symbols:

```sh
USER=yabai-fork-dev lldb -- ./bin/yabai --verbose --config "$PWD/examples/yabairc.dev"
# (lldb) breakpoint set --name handle_domain_vspace
# (lldb) run
```

The same no-competing-daemon rule applies under LLDB. macOS may require a manual
Developer Tools authorization or prevent debugging a particular signed process;
use foreground verbose logs if access is denied. Never change SIP to debug.
After the ordinary tests pass, `make asan` is available for targeted memory-error
investigation; it rebuilds `bin`, so sign the result again before a GUI session.
