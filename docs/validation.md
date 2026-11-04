# Validation report — 2026-11-01

Latest update: [M2-A unlocked GUI repeats and recovery](#unlocked-repeats-and-final-recovery).
M2-A has passed the signed universal build, 30 headless tests, native probes,
real daemon allocation/exhaustion, native/virtual focus result comparisons, and
empty-Space float/BSP/stack smoke checks on SIP-enabled macOS 15.6. The user
confirmed visible switching and return. Independent final recovery passed every
condition against the idle-session baseline. Native gesture dispatch retains its
observed cross-display focus limits. New hosted CI needs publication approval;
macOS 26 GUI and a standalone single-display GUI repeat remain unrun.

## Environment and upstream

- macOS 15.6 (24G84), Apple Silicon (`arm64`); SIP enabled throughout.
- Xcode Command Line Tools, Apple Clang 17.0.0 (`clang-1700.0.13.5`),
  macOS SDK 15.5. Full Xcode is not installed; LLDB is available.
- Upstream base: `dd845723416f5fe92af49fad5ebab00369e07edd`, yabai 7.1.25.
  Complete history: 1,706 upstream commits; not a shallow clone.
- Fork: `Stevenqx/yabai-v`, created and confirmed by the user.
- Checkout: `/Users/steven/Workspace/Github/yabai-v`, branch `feat/vspace-smoke`.
  `~/Developer/yabai-fork` is outside the session's writable roots; relocation
  instructions are in `development.md`.

## Results

| Check | Result |
| --- | --- |
| Inspect existing makefiles and manifest before edits | Passed; build system unchanged |
| Build unmodified upstream with `make` | Passed, universal x86_64/arm64 |
| Upstream `make -C tests` | Passed, 2/2 |
| Modified `YABAI_FORK_SIGN_IDENTITY=- ./scripts/build-dev.sh` | Passed; CI ad-hoc signing path |
| Certificate-signed `./scripts/build-dev.sh` outside sandbox | Passed |
| `codesign --verify --strict --all-architectures --verbose=2 ./bin/yabai` outside sandbox | Passed |
| `lipo ./bin/yabai -verify_arch x86_64 arm64` | Passed |
| `./bin/yabai --version` | Passed, `yabai-v7.1.25` |
| Modified `make -C tests` | Passed, 6/6 |
| `git diff --check`, shell syntax, workflow YAML/matrix | Passed |
| Clang static analyzer, arm64 `src/manifest.m` | No findings |
| Live `USER=yabai-fork-dev ./bin/yabai -m vspace --ping` | Exact JSON: `{"status":"ok","component":"vspace"}` |
| Live missing/unknown vspace command and `--ping extra` | Expected errors and unsuccessful exits |
| Live `query --displays`, `--spaces`, `--windows` | Passed for original and development daemons |
| Window focus and switching existing Spaces | GUI follow-up attempted; both skipped by safe-test conditions |
| GitHub Actions macOS 15/26 | Workflow configured; not published or run |

The query snapshots return arrays with integer IDs, matching field names and
counts: 3 displays, 4 Spaces, and 5 windows. The four new headless tests exercise
the production dispatcher: exact ping JSON, missing/unknown commands, and extra
arguments. The two existing geometry tests still pass. Existing domain handlers
and BSP/stack/float implementations are unchanged.

The GUI follow-up found no suitable pair of visible terminal/project windows on
the focused Space and no second ordinary Space on the focused display. It skipped
those operations instead of creating Spaces or focusing unrelated applications.
Actual focusing and Space switching remain unverified in this report. macOS 26
runtime behavior also requires a separate host/session.

## Signing, failures, and limitations

The user created the persistent `yabai-fork Development` certificate. The binary
has identifier `com.yabai-fork.development` and a certificate-based designated
requirement with leaf SHA-1 `85c2410619477a69b324ed91bb1ff677f4e2e775`.
Retain the certificate and private key across rebuilds. Both the signing log and
live test verify the certificate-signed universal executable outside the sandbox.
Sandboxed verification cannot read the user's trust and reports
`CSSMERR_TP_NOT_TRUSTED`; this does not reproduce in Terminal.

The first daemon startup failed because Accessibility permission was missing.
The user granted permission manually and subsequent smoke tests passed. An initial
manual focus command used an `ID` placeholder, which did not validate focusing;
the follow-up applied the safe-test conditions described above. Baseline and
modified test builds emit the same five upstream CoreVideo deprecation warnings;
production builds emit no warnings. No regression was detected in the tested
paths. GUI skips and unrun macOS 26 checks limit that conclusion.

## Original installation and recovery

With explicit approval, the manual test temporarily paused the original service,
ran the development daemon with dedicated IPC names and an explicit float config,
then stopped it and restored the original service. Restoration succeeded after
each attempt, including the permission failure. The final original LaunchAgent
is running from `/opt/homebrew/bin/yabai` (PID 83673 when checked), and its
`query --displays` succeeds. No competing window-managing daemons were started.

The following SHA-256 values match before and after validation:

| Original file | SHA-256 |
| --- | --- |
| `/opt/homebrew/Cellar/yabai/7.1.25/bin/yabai` | `372ad557a7c54a6199a78dcbcefe5b60fd0224e5c1f5139cfaed00dfdaa44501` |
| `/Users/steven/.dotfile/yabai/yabairc` | `712f2e9cb720bb811aa0a86010d0881d9db018b9876beae8c7a1a86c7125f6fc` |
| `/Users/steven/Library/LaunchAgents/com.asmvik.yabai.plist` | `0a9af809f52d335cda53616a8f6c1d89dd0bc6b8ccf5074a9c7d8f476cf7b1b7` |

No existing configuration or binary was replaced. Accessibility was granted
manually for development; no TCC reset was performed. No scripting addition was
installed or loaded, no sudo was used, and SIP remained fully enabled.

## Initial workflow and next step

During initial setup, the sandbox could not access GitHub or write the checkout's
`.git`. Four focused Conventional Commits were prepared in
`/private/tmp/yabai-fork-setup.git`, with
the full upstream history. A complete Git bundle and guarded import helper allow
manual import into the checkout without replacing working files. The helper also
sets `origin` to `https://github.com/Stevenqx/yabai-v.git`; `upstream` remains
`https://github.com/asmvik/yabai.git`. Nothing has been pushed.

Logs: `/private/tmp/yabai-fork-signing.log`,
`/private/tmp/yabai-fork-runtime.log`, and
`/private/tmp/yabai-fork-runtime-gui.log`.

The prepared commits have since been imported into this checkout; the follow-up
below supersedes that setup-time GitHub/access status.

Next: review the commits and run the CI matrix after pushing is explicitly
authorized; complete macOS 26 and safe GUI validation. Then design daemon-owned
virtual workspace state and SIP-compatible window visibility/assignment boundaries
before implementing workspace logic. No GUI project or workspace management is
included in this milestone.

## Validation follow-up — 2026-11-01

Scope: finish available M1 validation and prepare the remaining checks before
detailed design or implementation. Starting HEAD: `de6a849` on
`feat/vspace-smoke`; working tree and index were clean. The same upstream base
and all 1,706 upstream commits remain present, and the checkout is not shallow.

### Commit review

Reviewed all six local commits, `271fb07` through `de6a849`, against
`dd845723416f5fe92af49fad5ebab00369e07edd`. No blocking findings were identified.
Only `src/message.c` and the new `src/vspace.c` change production code. The
handler uses the existing token and failure protocol, is included once after
the shared helpers, and introduces no state or macOS calls. Existing handlers,
layout implementations, makefiles, and scripting-addition code are unchanged.
The tests exercise the production dispatcher; development signing and CI stay
separate from service installation. The six commits are focused and retain
upstream history.

### Repeated checks and current prerequisites

| Check | Result |
| --- | --- |
| `make -C tests` | Passed, 6/6; the same five upstream CoreVideo deprecation warnings |
| `git diff BASE..HEAD --check` and shell syntax | Passed |
| Workflow YAML parsing and runner matrix | Passed; `macos-15` and `macos-26` |
| Certificate-signed `./scripts/build-dev.sh` | Passed after the user manually authorized Keychain; no production build warnings |
| Existing development binary signature, before rebuilding | Passed outside sandbox, all architectures |
| Rebuilt binary signature and identity | Passed outside sandbox, all architectures; identifier and certificate leaf SHA-1 match the original development signature above |
| Rebuilt architectures and version | Passed; universal x86_64/arm64 and `yabai-v7.1.25` |
| GitHub branch and Actions inspection | Read access succeeded outside sandbox; task branch absent and no runs exist |
| Original service and read-only queries | Passed; one Homebrew daemon, PID 83673; 2 displays, 3 Spaces, 5 windows |
| Original binary, config, and LaunchAgent hashes | All three still match the values above |
| SIP | Enabled |
| Local Markdown links and whitespace | Passed; 29 links including anchors across 8 documents; `git diff --check` |
| New development daemon/GUI test | Not run; service-pause approval and suitable test windows are pending |
| macOS 26 runtime | Not run; this host is macOS 15.6 and no macOS 26 GUI session was supplied |

The desktop changed since the first report; the new query counts are a current
snapshot, not a comparison of original and development daemons. The focused
Space is index 3 on display 2 and has no eligible terminal/project window pair.
Display 1 now has two ordinary existing Spaces (indices 1 and 2), so an approved
GUI session can test switching there without creating a native Space. Re-query
IDs and indices immediately before use because the desktop can change again.

### Remaining execution

1. With explicit publication approval, push only `feat/vspace-smoke` to `origin`
   using `git push -u origin feat/vspace-smoke`. Check both Actions jobs for the
   exact pushed HEAD and record their URLs and conclusions. Do not merge.
2. With explicit approval to pause the original service, record current focus,
   Space/display, service state, and original file hashes. Prepare two disposable
   terminal windows on an existing ordinary Space on display 1. Run the signed
   checkout daemon
   with `examples/yabairc.dev` after confirming the original daemon has stopped.
   Repeat ping, invalid-command, and query checks, then confirm the test window
   IDs from the development daemon's query. Focus each selected window and
   verify both visibly and through queries. Switch between the two existing
   ordinary Spaces with `space --focus INDEX`, re-querying indices before each
   operation; verify visible results and queried IDs. Restore the original Space
   and focus, stop the development process, restore the original service, and
   verify queries, hashes, and SIP. Record failures and skips explicitly. See the
   [manual smoke-test procedure](development.md#manual-daemon-smoke-test).
3. On a supplied macOS 26 GUI host/session, establish its own persistent signing
   identity and repeat the build, headless, live ping/query, focus, Space-switch,
   and recovery checks with SIP enabled. CI results cannot close this runtime
   follow-up.

No publication, service change, new daemon, window focusing, or Space switching
has been performed in this follow-up. Detailed workspace design and implementation
remain deferred until the validation follow-ups are resolved.

## Publication and GUI validation — 2026-11-02

The user explicitly approved pushing the reviewed branch at `6359db7` and
running its CI, plus a brief GUI test session with recovery of the original
service, Space, and focus. Published only `feat/vspace-smoke` to the fork's
`origin`; no pull request or merge was performed. Tested source commit:
`6359db75df35aa875b780a4e83fc89ab533b4015`.

### GitHub Actions

[Run 41403274379](https://github.com/Stevenqx/yabai-v/actions/runs/41403274379)
completed successfully for that exact commit. Both jobs built the universal
x86_64/arm64 binary with the existing makefile, verified its ad-hoc signature,
checked the architectures and version, passed all six headless tests, and passed
whitespace and shell-syntax checks.

| Job | Actual hosted OS/image | Result |
| --- | --- | --- |
| [macOS 15](https://github.com/Stevenqx/yabai-v/actions/runs/41403274379/job/125139569754) | 15.8.2 (24H232), `macos-15-arm64` image `20261028.0489.1` | Passed, 6/6 tests |
| [macOS 26](https://github.com/Stevenqx/yabai-v/actions/runs/41403274379/job/125139569916) | 26.6.2 (25G83), `macos-26-arm64` image `20261026.0455.2` | Passed, 6/6 tests |

Both headless test builds emit the same five upstream CoreVideo deprecation
warnings. The Actions logs are also saved locally at
`/private/tmp/yabai-v-ci-41403274379.log`. Hosted build/test success does not
establish interactive macOS 26 runtime compatibility.

### Approved macOS 15.6 GUI session

The existing certificate-signed binary passed strict verification for all
architectures before startup. Confirmed exactly one original Homebrew daemon
before stopping it and no competing daemon before starting the checkout binary
with dedicated IPC names and `examples/yabairc.dev`. The float configuration,
exact live ping JSON, invalid-command failures, and display/Space/window queries
passed again.

Computer Use refused access to Terminal for safety; a separate Finder inspection
timed out and reset its runtime. The user manually prepared the disposable idle
terminal windows. The first window-identification check missed Terminal's
localized app name (`终端`); correcting that check identified the intended pair
on display 1, Space 1. No terminal commands were entered into those windows.

The first completed focus sequence (windows 7864 → 7875 → 7864) and existing-Space
round trip passed queries. Those disposable windows were closed during cleanup.
The user requested a repeat to observe the changes, prepared a new pair, and
confirmed **both worked visibly** after the following slower sequence:

| Operation | Query evidence | Visual evidence |
| --- | --- | --- |
| Focus windows 7966 → 7980 → 7966 | Each selected window became focused and visible | User confirmed; each step held for 5 seconds |
| Switch display 1 from Space ID 3 (index 1) to ID 7 (index 2), then back to ID 3 | Correct ID, `has-focus`, and `is-visible` after each command | User confirmed; each destination held for 5 seconds |

No native Space was created or deleted, and no scripting addition was installed
or loaded. These are real macOS 15.6 GUI results with SIP enabled; they do not
establish GUI runtime behavior on macOS 26.

### Recovery and remaining limitation

All test sessions stopped the development process and restored the original
service. The first restoration check found that original-service startup had
changed focus after the pre-shutdown restoration. Restoring focus again through
the original daemon corrected it. Subsequent sessions verified restoration both
before stopping the development daemon and after the original service started.

Automatic approval review held completion of the final test session until the
user's visual confirmation arrived. The session then ended successfully without
closing the repeat-test windows. Final verification found exactly one original
Homebrew daemon, PID 9357, running `/opt/homebrew/bin/yabai`. All three query
domains passed; original Space ID 395 on display 2 and Orca window 6369 were
focused, and the original set of visible Spaces was restored. The original
binary, user configuration, and LaunchAgent hashes still match the values above.
SIP remained enabled. Windows 7966 and 7980 remain open as promised.

Logs and event snapshots:

- Initial Space-query round trip: `/private/tmp/yabai-fork-runtime.08BMuJ`.
- First focus/Space sequence and recovery: `/private/tmp/yabai-fork-runtime.gLZGpg`.
- Visually confirmed repeat: `/private/tmp/yabai-fork-runtime.zKEAQX`, including
  `gui-events-repeat.json`, signature, daemon, recovery, and hash records.
- Original and final desktop snapshots:
  `/private/tmp/yabai-v-approved-gui-yuh58lw1`.

The user confirmed that no macOS 26 GUI host/session is available. That runtime
check remains **unrun**, independently of the passing macOS 26 CI job. It is the
remaining M1 validation follow-up; detailed design and implementation remain
deferred pending its evidence or an explicitly revised validation gate.

The resulting documentation changes passed all 33 local Markdown link checks
(including anchors across eight documents) and `git diff --check`. No executable
behavior changed after the CI-tested commit.

## Milestone 1 manager foundation — 2026-11-03

Starting checkpoint: `21a6e6f` on `feat/vspace-mapping`. Local macOS 15.6 arm64,
SIP enabled. `make` passed without warnings and produced the universal binary.
`make -C tests` passed 13/13 tests; only the same five upstream CoreVideo
deprecation warnings appeared. Added tests cover native order across displays,
empty active versus spare Spaces, unknown occupancy, duplicate prevention,
atomic enumeration failures, stable session mappings, lookup, ordinary floating
and minimized windows under BSP/float layouts, native focus IDs, stale targets,
native errors, and silent activation failure. `git diff --check` passed.

This checkpoint compiles/tests the independent manager without starting it in
the daemon; CLI/startup wiring follows. The original Homebrew daemon remains
PID 9357 at this read-only check. No service, desktop, or signing trust changed.
The only GitHub CI run remains Milestone 0's `41403274379`; Milestone 1 has not
been published. No interactive Milestone 1 runtime result is claimed.

## Milestone 1 commands and local validation — 2026-11-03

Branch: `feat/vspace-mapping`. Scope checkpoint `21a6e6f`, manager checkpoint
`36e9ac8`; the next implementation commit integrates startup and CLI commands.
The user authorizes this milestone despite the prior unavailable macOS 26 GUI
host. All earlier tasks belong to Milestone 0; old milestone labels in historical
entries do not change the current roadmap.

### Executed checks

| Check | Result |
| --- | --- |
| `./scripts/build-dev.sh` (runs `make`) | Passed; no production compiler warnings |
| Persistent certificate signature, strict/all architectures | Passed; `yabai-fork Development`, identifier `com.yabai-fork.development`, unchanged certificate leaf SHA-1 `85c2410619477a69b324ed91bb1ff677f4e2e775` |
| Universal architecture check and version | Passed; x86_64/arm64, `yabai-v7.1.25` |
| `make -C tests` | Passed, 22/22; same five upstream CoreVideo deprecation warnings |
| Clang static analyzer, arm64 manifest | Passed, no findings |
| `sh -n scripts/build-dev.sh examples/yabairc.dev` | Passed |
| Local Markdown links/anchors and `git diff --check` | Passed |
| Compare native handlers, layout code, and build/test/CI files to `93a59ed` | Unchanged; includes `src/message.c`, native Space/display/window managers, and `src/view.c` |
| Checkout CLI read-only queries against installed daemon | Passed; 2 displays, 4 native Spaces, 5 windows |
| Read-only native adapter probe (no daemon/focus) | Passed; native Spaces 3/589 allocated as unknown ordinary candidates, 7/592 confirmed empty/spare; JSON/current and global indices matched |
| Original binary/config/LaunchAgent SHA-256 | Matches all three Milestone 0 values above |
| New macOS 15/26 GitHub Actions matrix | **Unrun**, local branch needs publication approval |
| New daemon and interactive vspace focus | **Unrun**, requires explicit service-pause approval |
| macOS 26 interactive runtime | **Unrun**, no GUI host/session supplied |

The 22 tests include the two upstream geometry tests and original ping/error
tests. New tests exercise an all-empty multi-display snapshot (only the globally
active Space is allocated), occupied/spare/unknown mappings, display/Space order,
global indices, session stability, duplicate/conflicting ownership, atomic query
failure, lookup, BSP/float occupancy including minimized and floating windows,
64-bit serialization, strict/overflowing indices, pre-action argument checks,
current/unmapped responses, exact JSON, native focus ID dispatch and failures,
stale targets, and a dispatched gesture that never activates the target.
Known nonordinary/desktop windows, native system-UI tags, ordinary tags at
unusual levels, and missing/unknown AX metadata are covered.

Review strengthened conservative occupancy: explicit AXUnknown data and untracked native
ordinary-window candidates remain unknown, regardless of floating level. The
existing native ordinary-window tag predicate excludes nonordinary system UI. False-positive allocation
of visually empty Spaces with unidentified system UI is intentional and
[documented](vspace.md#startup-policy-and-session-stability). Startup snapshot
freezing and focus timeout behavior are limitations, not lifecycle support.

The signed read-only native probe compiled the production manifest with the
normal daemon entry point disabled and initialized empty lookup tables. It did
not register event handlers, load configuration, start a daemon, or call focus.
It verified the real display/Space lists and native occupancy adapter: occupied
Spaces 3/589 remained conservatively allocated without AX tracking, while empty
Spaces 7/592 remained spare after excluding native nonordinary system UI. Its
list/current JSON matched the active native Space and had unique global indices.
This is native-query evidence, not an interactive daemon session. The initial
probe crashed because its temporary harness omitted the lookup-table setup;
the crash report confirmed that cause. Correcting only the probe setup fixed it.
The first successful run then exposed overly broad unknown classification of
system UI, which was corrected in production using upstream's native tag test
and covered by focused unit cases and a successful probe rerun.

No regression was detected in these checks. Read-only installed-daemon queries
exercise the unchanged CLI client and the installed native handlers; they do not
validate the new manager in a real daemon. No new binary was installed, no daemon
was started/stopped, and no desktop focus/layout was changed. SIP remains enabled.

Logs: `/private/tmp/yabai-v-m1-build.log`, `yabai-v-m1-signature.log`,
`yabai-v-m1-tests.log`, and `yabai-v-m1-analyzer.log` in the same directory.
Read-only installation/query baseline: `/private/tmp/yabai-v-m1-baseline.j8r6f3ko`.
Probe source: `/private/tmp/yabai-v-m1-native-probe.m`; results:
`/private/tmp/yabai-v-m1-native-probe.jsonl` and the corresponding `.log`.
An initial baseline helper used the wrong LaunchAgent filename and failed after
successful queries; inspecting the directory and rerunning with
`com.asmvik.yabai.plist` completed the baseline. No files were changed by it.

### Prepared approval-dependent checks

1. After diff review and publication approval, push only `feat/vspace-mapping`
   to `origin` (including its unpublished foundation documentation ancestry).
   Wait for the existing macOS 15/26 matrix and record the exact tested SHA,
   results, and logs. Do not merge or rewrite the published smoke-test branch.
2. After explicit approval to pause the original service, recheck all host state,
   signature, original file hashes, visible Spaces, focus, and a safe mapped
   destination. The read-only baseline currently shows original PID 9357,
   active native Space 589 on display 2, Orca window 6369, and occupied native
   Space 3 visible on display 1. These identifiers are historical at handoff.
3. Pause only the inspected original LaunchAgent, verify no daemon remains,
   start the signed checkout binary with the existing float development config,
   and query ping/list/current plus native displays/Spaces/windows. Validate
   actual occupancy and index/native-ID associations before focusing anything.
4. Focus a safe mapped existing destination and return to the original virtual
   index, holding each state for five seconds for visual confirmation. Verify
   current/list/native state and unchanged window frames, associations, and
   layout settings across the focus calls. Never move windows or create Spaces
   to prepare a target. If no safe mapped target exists, record a skipped focus
   check instead of manufacturing one. Distinguish cross-display focusing of
   visible Spaces from switching to an inactive Space.
5. Stop only the development process and restore the original service. Verify
   original visible Spaces and focused window after service startup too, compare
   installation hashes, and record any upstream startup effects separately.
   Keep query evidence separate from visual confirmation.

Both operations need user approval under `AGENTS.md`; earlier approval covered
only the old smoke-test branch/session. The prepared result is fully local and
reviewable. Milestone 1 implementation is complete, but these checks remain open;
Milestone 2 has not begun.

## Milestone 1 publication and interactive validation — 2026-11-04

The user explicitly approved publishing `feat/vspace-mapping` at `6c4a354` to
`origin`, running the existing macOS 15/26 matrix, and temporarily pausing the
original service for a signed development-daemon focus round trip with recovery.
Published source SHA: `6c4a354efb3a75c25906d48a49ad55c224fe175a`. No merge,
force-push, installation, SIP change, or scripting-addition operation occurred.

### GitHub Actions

[Run 41779098539](https://github.com/Stevenqx/yabai-v/actions/runs/41779098539)
passed for that exact SHA. Each job built the universal x86_64/arm64 binary,
verified its ad-hoc signature, checked its version/architectures, passed 22/22
headless tests, and passed whitespace and shell-syntax checks. Both test builds
reported only the same five upstream CoreVideo deprecation warnings.

| Job | Actual hosted OS/image | Result |
| --- | --- | --- |
| [macOS 15](https://github.com/Stevenqx/yabai-v/actions/runs/41779098539/job/126406264703) | 15.8.2, `macos-15-arm64` | Passed, 22/22 |
| [macOS 26](https://github.com/Stevenqx/yabai-v/actions/runs/41779098539/job/126406264390) | 26.6.2, `macos-26-arm64` | Passed, 22/22 |

Logs and job metadata are `/private/tmp/yabai-v-m1-ci-41779098539.log` and `.json`.
Hosted build/headless tests do not establish macOS 26 interactive compatibility.

### Local GUI setup and interrupted first attempt

The existing universal checkout binary passed strict/all-architecture signature
verification with `yabai-fork Development`, identifier
`com.yabai-fork.development`, and the unchanged certificate leaf SHA-1
`85c2410619477a69b324ed91bb1ff677f4e2e775`. No rebuild or TCC reset was needed.

Fresh inspection found one display, occupied native Space 3, and empty native
Spaces 7 and 592. The earlier two-display identifiers were historical. To test
empty-active initialization without creating Spaces or moving windows, the
original daemon focused existing empty Space 7 before being paused. Each session
verified that no managing daemon remained before starting the checkout daemon.
The explicit development config kept layout float and mouse focus disabled.

The first session correctly mapped occupied Space 3 and active-empty Space 7,
left Space 592 spare, and passed ping/list/current plus invalid-index,
overflow/unmapped-index, extra-argument, and already-focused error checks. The
first virtual activation then failed its observed-focus check, and the native
recovery command reported Mission Control active. The user reported that they
might have switched Spaces manually through Mission Control during the test.
That interference is a plausible cause; this attempt does not establish a
product regression or a successful focus round trip.

The development process stopped and the original service restarted. An immediate
helper recovery assertion was not diagnostic; an explicit follow-up restored and
verified the captured original Chrome window 1052 on Space 3, one original daemon,
and matching file hashes. One window frame differed across this attempt's
original-service restart, so the attempt does not establish frame preservation.
A Computer Use Dock inspection timed out; it supplied no visual evidence or UI
input. No native/window-layout source change was made in response.

The unchanged installed daemon then passed an independent native focus control,
Space 3 → 7 → 3, holding each destination for five seconds and restoring the
freshly captured focus. Its results are in the first session's evidence folder.

The first session also exposed an existing example-config error: integer `0`
was rejected for `window_animation_duration`. The checkout-only fix in local
commit `b195681` changes it to float `0.0`; no native parser semantics changed.
The published C source and CI-tested executable are unchanged. Earlier float
layout checks did not establish success of this animation-setting line.

### Successful repeat and visual confirmation

The repeat allowed native transitions and startup/config processing to settle,
and the user left manual switching idle. It used the same signed production
binary with the corrected local config. Animation duration queried `0.000000`,
global layout queried `float`, and development stderr was empty.

Initialization and error checks passed again. `--list` had exactly these stable
mappings, with unique IDs/indices and native display ID 1:

| Virtual index | Internal ID | Native Space ID | Startup state |
| --- | --- | --- | --- |
| 1 | 1 | 3 | Occupied, inactive |
| 2 | 2 | 7 | Empty, active |

Native Space 592 remained unallocated. Each virtual-focus call succeeded and was
checked with `--current`, `--list`, and native queries:

| Operation | Query evidence | Visual evidence |
| --- | --- | --- |
| Virtual 2 → 1 (native 7 → 3) | Correct active native ID and focused JSON entry | User confirmed the repeated switches worked visibly |
| Virtual 1 → 2 (native 3 → 7) | Correct empty active Space and stable mapping | Same confirmation; each destination held for 5 seconds |
| Virtual 2 → 1 (native 7 → 3) | Original active Space and stable mapping | User confirmed return to the original desktop |

All five ordinary windows retained exactly the same frames and display/Space
associations across every focus call. All native view layout values were
unchanged during those calls. The original native CLI handlers remained intact.
This is a real SIP-enabled macOS 15.6 interactive test, including switching to
an inactive empty mapped Space; it is separate from the headless CI evidence.

### Recovery, local records, and remaining limits

The repeat stopped only its development process and restored the original
LaunchAgent. The helper again ended with a generic recovery assertion after its
saved snapshot already showed the original Space/focus. A separate final audit
explicitly restored focus, verified every recovery condition, and saved a passed
result; the generic assertion's precise cause was not established and is not
reported as a production failure.

Final verification found exactly one original Homebrew daemon, PID 13873,
running `/opt/homebrew/bin/yabai`; original visible/active Space 3 and Orca window
6369 were restored. All five original window frames and Space associations also
matched the repeat's immediate baseline after recovery. Original binary,
configuration, and LaunchAgent hashes match the Milestone 0 values above. SIP is
enabled. Host identifiers are historical and must be rechecked before reuse.

Evidence folders:

- First attempt, native control, and follow-up recovery:
  `/private/tmp/yabai-v-m1-approved.shfxktq3`.
- Successful repeat: `/private/tmp/yabai-v-m1-repeat.431h1ip7`, including
  `events.json`, per-focus snapshots, `initial-vspaces.json`, daemon logs,
  the helper's retained `results.json`, and passed `final-recovery-audit.json`.
- Temporary test helpers: `/private/tmp/yabai-v-m1-approved-smoke.py`,
  `yabai-v-m1-native-control.py`, and `yabai-v-m1-repeat-smoke.py`.

Milestone 1's requested local GUI and macOS 15/26 CI checks are complete. The
local config fix `b195681` and subsequent validation-record commit are not
published or covered by a new GitHub run; the corrected config passed the live
repeat and shell syntax checks. No C source changed after the tested SHA.
macOS 26 interactive runtime remains unrun because no GUI host was supplied.
Session-only mapping, conservative unknown occupancy, and external native
Space/display changes retain the documented limitations. Milestone 2 has not
begun, and no merge is authorized.

## Milestone 2-A native focus — 2026-11-04

Baseline: clean `feat/vspace-mapping` at `8c5d0b3`, an M1 descendant. `make`
passed the universal debug build without warnings. `make -C tests` passed 22/22
with the existing five CoreVideo deprecation warnings. Updated focus tests cover
native success while active state remains unchanged, native failures, invalid
indices, stale targets, cross-display native-ID dispatch, and existing CLI/layout
regressions. Virtual focus no longer polls or adds a timeout. These are headless
results; no new GUI/CI result or installed-service change is claimed.

Logs: `/private/tmp/yabai-v-m2a-focus-build.log` and
`/private/tmp/yabai-v-m2a-focus-tests.log`. SIP reports enabled on macOS 15.6.
Sandbox process-list inspection was unavailable; recheck outside the sandbox
before any real daemon session. Creation and final signing/runtime audit follow.

## Milestone 2-A allocation and local validation — 2026-11-04

Baseline: clean `feat/vspace-mapping` at `8c5d0b3ec02b6fdf78ee48aa0fcd785c09b6e3a9`,
an M1 descendant. Local native-focus checkpoint: `86dabb5`; allocation and records
are the following local commit. The local remote-tracking ref also points at
`8c5d0b3`; no fetch/push or new CI run was performed. Earlier notes describing
that checkpoint as unpublished are historical. Neither M2-A change is published.
The original upstream baseline/build evidence above remains applicable; this task
does not recreate or rebase that base.

### Local build and headless checks

Host: macOS 15.6 (24G84), arm64, SIP enabled; existing CLT/SDK and build system.

| Check | Result |
| --- | --- |
| `./scripts/build-dev.sh` (`make` plus persistent signing) | Passed final universal x86_64/arm64 build, no build warnings |
| Strict/all-architecture signature verification and designated requirement | Passed, `com.yabai-fork.development`, existing `yabai-fork Development` certificate leaf `85c2410619477a69b324ed91bb1ff677f4e2e775` |
| `lipo -verify_arch x86_64 arm64`, version/help | Passed; help includes create/native-focus semantics |
| `make -C tests` | 30/30 passed; five existing upstream CoreVideo deprecation warnings |
| arm64 Clang static analyzer | Passed, no findings |
| Shell syntax, local Markdown links/anchors, `git diff --check` | Passed |
| Original native handlers/layouts/build/test makefiles/CI versus baseline | Unchanged; only fork manager/handler and CLI help changed production behavior |
| Original daemon display/Space/window queries | Passed; service remained running |
| Original binary/configuration/LaunchAgent hashes | Match the three foundation hashes above |
| New macOS 15/26 hosted matrix | **Unrun**; existing workflow retained, publication not authorized |
| New daemon/GUI/focus/layout session | **Unrun**; needs explicit service-pause approval |
| macOS 26 GUI | **Unrun**; no supplied session |

Tests cover one/two-display native-order selection through exhaustion; no fallback
when the focused display is exhausted; exact survivor IDs/indices/order;
independent virtual IDs and indices; empty allocation retention and subsequent
focus; visible unmapped and non-desktop candidates; floating-window occupancy
without a layout node; unknown/custom AX state; unavailable display, topology,
and visibility; changes in visibility, occupancy (occupied/unknown), type,
topology/ownership, focused display, and query availability during revalidation;
index/ID exhaustion; CLI JSON, strict arguments, and failure atomicity. Existing
ping/current/list, malformed/stale/reassociated focus, native focus error
categories, delayed native success, and BSP/stack/float dispatcher checks pass.
Unknown/incomplete native window-query results use the existing conservative
adapter; deterministic fixtures inject unknown occupancy, rather than claiming
that a real WindowServer query failed during this session.

Logs: `/private/tmp/yabai-v-m2a-signed-build-final.log`,
`yabai-v-m2a-tests-final.log`, and `yabai-v-m2a-analysis-final.log`.
The first sandbox signing attempt reported invalid parameters; signing outside
the sandbox with the same existing identity passed. An initial new test fixture
failed compilation because the upstream test macro requires parentheses around
comma-containing compound literals; corrected before the passing runs.

### Read-only native allocation probes

Temporary probe: `/private/tmp/yabai-v-m2a-native-probe.m`, with build log,
`.jsonl` mapping snapshots, and `.log` native observations at the same prefix.
It initializes native query access and empty lookup tables only, never native
managers/event processing, and invokes the production allocation policy against
current WindowServer state. It does not start a managing daemon or call focus,
window movement, layout, Space lifecycle, or service operations. Untracked
ordinary windows are conservatively unknown in this probe, unlike a full daemon
with AX tracking; safety classification is additionally tested headlessly.

An initial one-display probe mapped native Space 3 at startup, then allocated
hidden empty desktops 7 and 592 in enumeration order and failed at exhaustion,
leaving prior mapping bytes and next ID unchanged on failure. Current/list stayed
consistent and native window IDs/frames/associations matched the immediate
one-display baseline. That probe preceded the final custom-AX tightening; the
final probe was rebuilt from the final production source.

Host topology changed between observations without any service/focus commands
from this task. The final probe saw two displays and focused display 2:

| Display | Native Space order | Probe occupancy/visibility |
| --- | --- | --- |
| 1 | 3, 7 | 3 visible/unknown; 7 hidden/empty |
| 2 (focused) | 666, 592 | 666 visible/unknown; 592 hidden/empty |

Startup mapped 3 and 666. Creation appended 592 with a fresh ID/index; the next
call failed with no spare on display 2 even though hidden empty Space 7 on display
1 remained unmapped. Survivor bytes, counter on failure, and globally focused
current mapping were preserved. The original Homebrew daemon remained PID 13873.
All identifiers and topology are historical; recheck before use. Do not compare
frames across the external display-topology change as proof of preservation.
The first audit tried nonexistent `~/.yabairc`; rerunning with the repository's
recorded `~/.dotfile/yabai/yabairc` confirmed all original hashes.

These are native adapter/policy checks in process-local memory, not evidence of
IPC execution in a real development daemon or visible focus behavior.

### Concrete live/CI follow-ups

With explicit approval under `AGENTS.md`, recheck current processes, native
queries, signed binary, original hashes, visible Spaces and focused window.
Pause only the original Homebrew service, confirm no daemon remains, and launch
the signed checkout binary with `examples/yabairc.dev` and dedicated IPC names.
Use the current topology rather than the historical IDs above.

1. Check ping/list/current and native queries; record stable virtual IDs/indices.
2. Create mappings in current-display enumeration order through exhaustion;
   verify no focus/frame/layout/native-order changes or mutation on failure.
   With two displays, leave a spare on one while exhausting the focused display,
   then focus the other display and allocate there. Check visible unmapped and
   floating occupancy when safe existing state permits; retain fixture evidence
   for unsafe/unavailable state that cannot be produced without disruption.
3. Compare native and virtual focus to the same mapped native Space, restoring
   the source between controls and letting transitions settle. Record native
   exit status separately from five-second query/visual observations. Include
   already-focused and cross-display cases; bad/stale targets remain covered
   headlessly when native removal would be unsafe.
4. Smoke-test float/BSP/stack on safe disposable/empty workspaces, restore view
   settings, all original visible Spaces and focus, then stop only the development
   daemon and restore the original service. Independently audit original-service
   queries, focus, frames, hashes, SIP, and one-daemon state after recovery.

Do not start this session alongside the existing daemon. Publication/CI is a
separate authorization gate: push only reviewed M2-A commits when approved, then
run the existing macOS 15/26 matrix for that exact published SHA. Historical M1
CI is not a substitute. A macOS 26 GUI host and real two-display session are
still required for their interactive compatibility claims.

## Milestone 2-A approved GUI validation — 2026-11-04

The user approved the documented service pause, signed checkout daemon session,
and restoration/audit. Source remains `5b199fb` with focus checkpoint `86dabb5`;
no publication is authorized. Fresh preflight found two displays, native orders
3/7 and 674/592, original daemon PID 13873, active Space 674 and focused Zotero
window 5943. Persistent signature and all original installation hashes passed.

### Locked-screen attempt and recovery

Evidence: `/private/tmp/yabai-v-m2a-gui.f2_iljuh`, helper
`/private/tmp/yabai-v-m2a-gui-smoke.py`. The first preflight-only helper attempt
(`/private/tmp/yabai-v-m2a-gui.kpx8ldjh`) incorrectly checked only signature stderr
for a designated requirement printed to stdout. It stopped before any service
change; the corrected combined-output check passed.

Paused the original service and verified no daemon remained before starting the
signed checkout daemon as the only managing process. Float config, animation
`0.0`, ping/list/current, native queries, invalid/overflow/unmapped indices, and
strict create/ping argument checks passed. Startup preserved native order and
window frames. Startup mapped Spaces 3/674 as IDs/indices 1/2. Creation appended
hidden empty Space 592 as ID/index 3 on focused display 2. Focus, visibility,
window frames/associations, layouts, native order, and survivor JSON were unchanged.
The next create failed without mutation while display 1 retained unmapped spare
7. This is real daemon/IPC evidence for allocation and no cross-display fallback.

The unchanged native cross-display focus control to Space 3 returned zero but
was not observed within eight seconds. The helper stopped the sequence before
virtual focus, second-display allocation, and layout checks. Both development
and original window-focus recovery commands could not locate captured window
5943 in tracked AX state. Native queries still enumerated all five windows, but
newly started daemons had no AX role/subrole or focused-window metadata.

Read-only console inspection established `CGSSessionScreenIsLocked: true`.
The screen lock explains the unavailable interactive test conditions; this
attempt does not establish a fork focus regression or a successful GUI switch.
The helper now checks the console lock state before any future service pause.
No TCC reset, Keychain change, unlock attempt, scripting addition, or SIP change
was made. Manual unlock is requested before further dependent GUI work.

The development process stopped and original service restarted. Independent
`independent-recovery-audit.json` confirmed one original Homebrew daemon (PID
98745), original visible Spaces 3/674 and active Space 674, unchanged native
order/layouts, all original window frames/associations, all three installation
hashes, and SIP enabled. Window focus remains **unverified while locked**:
expected 5943, native query reports no focused window. Do not report full
window-focus recovery until an unlocked-session audit verifies it. The native
service is running; no development daemon remains.

### Unlocked repeats and final recovery

The user manually unlocked the Mac and authorized continuation, then confirmed
an idle desktop for the controlled repeat. Source is unchanged at `5b199fb`;
checkout records were at `157bae2`. No push or implementation change occurred.
AX roles and focused-window queries became available again. On this host the
console lock key is absent after unlock; the helper records that absence and
requires an active completed console login rather than mistaking it for a lock.

External host changes had replaced the second display UUID/frame and native
Space 674 with Space 733, moved windows, and opened new application windows.
Rechecking the historical recovery baseline therefore reported mismatches; those
are not a new fork mutation or a valid baseline for this repeat. The old audit
helper regenerated `independent-recovery-audit.json` and its current snapshot on
recheck; its post-unlock comparison is copied to
`post-unlock-historical-baseline-comparison.json`. The earlier locked-state audit
results are retained in the preceding report and conversation evidence.

The first unlocked attempt (`/private/tmp/yabai-v-m2a-gui.g55ljkm7`) was interrupted
by actual focus and application-window changes. Its cross-display baseline already
showed target Space 3 focused before the native command, which correctly rejected
it as already focused. Transient windows closed/opened during the attempt, so
original focus/frame recovery for those historical windows could not be claimed.
The original service restarted; a fresh idle baseline was captured next rather
than trying to recreate closed windows or undo the user's changed desktop.

Controlled idle evidence: `/private/tmp/yabai-v-m2a-gui.iaus8mt4`.
Baseline had one original Homebrew daemon (PID 40328), native order 3/7/592 on
display 1 and 733 on display 2, visible Spaces 3/733, active Space 3, and Orca
window 6369 focused. Signed checkout startup preserved all native window frames.
The sole development daemon initialized mappings `(index,id,space,display)` as
`(1,1,3,1)` and `(2,2,733,2)`.

| Live check | Evidence/result |
| --- | --- |
| Ping/list/current, native display/Space/window queries, invalid/overflow/unmapped index, extra create/ping arguments | Passed |
| Focused display 2 exhausted while display 1 has hidden spares 7/592 | Create failed without mapping mutation or fallback |
| Allocate on display 1 | Appended 7 as index/ID 3, then 592 as index/ID 4, in native order; subsequent exhaustion failed without mutation |
| Creation invariants | Existing IDs/indices/order, current/focus/visibility, frames/associations, layouts, and native order unchanged |
| Native and virtual focus from 3 to visible cross-display 733 | Both returned zero and observed the correct global focus; each held five seconds |
| Native and virtual focus from 3 to newly allocated 7 | Both returned zero and observed the correct focus; each held five seconds |
| Already-focused controls for both targets | Native and virtual both failed with their matching already-focused errors |
| Occupancy safety | Existing occupied Spaces remained mapped; artificial unknown/non-desktop/floating-candidate and stale-mapping cases remain headless evidence, not manufactured live failures |

The initial idle helper stopped at a stricter-than-native assertion on the next
cross-display hidden target. Its native call to 592 returned zero and made 592
visible on display 1, while global keyboard focus remained on 733/display 2.
That is an observed native behavior, not a virtual timeout or a fork regression.
The helper was corrected to compare dispatch status and log visibility/global
focus separately, consistent with D009 and the requested native guarantee.

Focused completion evidence: `/private/tmp/yabai-v-m2a-gui.chsprfea`, using the
same source and native topology. Repeated CLI/allocation/exhaustion/invariant
checks passed. Native focus from 733 to 592 returned zero, made 592 visible, and
retained global focus on 733 during the eight-second observation. The virtual
call to the same native ID also returned zero and then globally focused 592.
**The target was already visible for that second call**: setup restored the
source focus but did not restore the target display's earlier visible Space.
Thus this pair establishes native dispatch-result parity and a successful virtual
cross-display activation of a visible empty mapping; it does not establish equal
activation timing or identical hidden-target starting conditions. The earlier
visible cross-display and same-display comparisons had observed success for both.
No stronger SIP activation guarantee is claimed and no native path was modified.
Already-focused native/virtual calls to 592 both failed as expected.

Float → BSP → stack → float smoke checks passed on both empty mapped Spaces
7/592, restored each development view setting, and preserved all window frames,
focus, native order, and mapping identities. They exercise empty-view layout
commands; populated tiling/layout algorithms were not retested on user windows.
The completion helper has no test/recovery errors and empty development stderr.
The user explicitly confirmed: **“Yes, switching and return were visible.”**
That confirmation is separate from the command/query observations above.

Original-service recovery reapplied its existing window rules and changed two
surviving floating-window frames (13736/13727); all frames remained unchanged
during the allocation/focus commands themselves. After stopping the development
daemon and restarting the original service, restored only those captured floating
frames using existing native move/resize commands, checking PID and native
associations first. One width needed a second adjustment after its original
position was restored; the first constrained-width audit is retained as
`first-frame-recovery-audit.json`. No config/rule/native layout was modified.

The final independent audit, against the **first idle pre-test baseline** in
`iaus8mt4`, passed every condition: one original `/opt/homebrew/bin/yabai` daemon
(PID 42612), original display UUIDs/topology/native Space order, visible 3/733,
active 3, Orca window 6369 focused, every original window ID/frame/association,
original stack layouts, all three installation hashes, and SIP enabled. Use
`chsprfea/independent-final-recovery-audit.json` as the final recovery evidence,
plus its user visual-confirmation record. PIDs/IDs/topology are historical.

The locked/interrupted attempts and their helper exits are retained separately;
they are not represented as clean passes. Manual single-display GUI allocation
was not repeated because the available unlocked host had two displays; earlier
one-display native-adapter and deterministic coverage passed. macOS 26 GUI,
populated layout smoke, and new macOS 15/26 CI remain unrun. New CI requires a
separately authorized publication; no push, merge, TCC reset, SA, or SIP change ran.

## Standard vspace test runner — 2026-11-04

User-authorized follow-up from clean `4f637d7` on `feat/vspace-mapping`: maintain
a script that users can run on their own Macs, including other OS versions later.
The user chose macOS 15 for ongoing validation and explicitly deferred macOS 26
tests. This revises earlier dual-OS follow-up gates; it does not turn unrun
macOS 26 checks into passes. Production daemon source remains `5b199fb`.

Added [scripts/test-vspace.py](../scripts/test-vspace.py), with default automatic
build/sign/headless checks, read-only daemon contract checks, and opt-in terminal
interactive allocation/focus/empty-layout checks. It saves private JSON reports
with version/source/binary metadata, command outputs, individual results, and live
baseline/recovery snapshots. It does not manage services or install files.
Focus comparisons restore every display's visibility and Space/window focus
between controls; native results, observed activation, and visual confirmation
remain separate. Public-query empty candidates are not treated as proof of native
occupancy. Unsafe-state injection stays in the production deterministic fixtures.

Updated the development guide, agent workflow, compatibility notes, and progress
records. CI calls this same script on macOS 15 by default; a manual dispatch can
include macOS 26 later. No hosted workflow has run for this unpublished change.

Executed locally on macOS 15.6 arm64, SDK 15.5, with SIP enabled:

- Passed full `./scripts/test-vspace.py headless`: universal build through the
  existing helper, persistent development signing, strict all-architecture
  verification, version/help, production tests, initial runner regressions, and
  static checks. No production build warnings. Full-build evidence:
  `/var/folders/cf/byjbs_c51nb5gm4vtq8jkpv40000gn/T/yabai-v-tests-my64ynpj/report.json`.
- Passed final `./scripts/test-vspace.py headless --skip-build` after runner-only
  refinements: signature/architecture/version/help, **30/30 production tests**,
  **14/14 runner tests**, shell syntax and whitespace. Build was explicitly
  skipped, retaining the already verified build of unchanged daemon source.
  Existing five CoreVideo deprecation warnings remain in the production test
  compilation. Final report:
  `/var/folders/cf/byjbs_c51nb5gm4vtq8jkpv40000gn/T/yabai-v-tests-nmtuhy6f/report.json`.
- Runner fixtures cover a full interactive sequence, append/exhaustion with
  other-display candidates, distinct IDs/index gaps and survivor preservation,
  visible/floating/fullscreen candidates, uncertain public-query candidates,
  identical cross-display starting visibility, unobserved native dispatch success,
  already-focused with one workspace, interruption recovery, reused window IDs,
  layout restoration after a failed command, and nonterminal interactive refusal.
- Passed all 14 runner regressions with Apple's Python 3.9.6 as well as the
  default Homebrew Python 3.15 used by the standard automatic run.
- Passed runner help and rejection of an infinite observation budget; workflow
  YAML parsing/runner wiring, local Markdown links/anchors and whitespace passed.

Two sandbox attempts stopped at local signing/trust access (first signing
parameters, later `CSSMERR_TP_NOT_TRUSTED` during verification). Both were retried
with the established unsandboxed signing/verification workflow and passed; no
certificate, trust, or TCC settings changed. Failed reports remain separately in
`yabai-v-tests-vncv3fr6` and `yabai-v-tests-lxed_045` under the same temporary root.
The recorded source SHA is `4f637d7` with the task's pre-commit working-tree diff;
the new runner commit is the next local checkpoint, not a claimed hosted result.

**Not run:** the new script's query/interactive modes against a real daemon,
new hosted CI, or macOS 26. Live orchestration is fixture evidence; the earlier
M2-A GUI sessions above remain separate runtime evidence. No desktop/service
change was needed for this task. One-display GUI and populated layout smoke
remain limitations to exercise later using a prepared session/manual procedure.
