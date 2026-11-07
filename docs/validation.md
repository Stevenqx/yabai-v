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
[documented](vspace.md#startup-restoration-and-identity). Startup snapshot
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

## Milestone 2-B evacuation and release — 2026-11-05

Authorized implementation from `tmp/yabai-v-m2b-codex-prompt.md`, starting clean at
`cf20e36` on `feat/vspace-mapping` (completed M2-A/runner descendant). This request
supersedes the prior proposed-only status of M2-B; M2-C/D and native MRU remain
proposals. Neither publication nor another installed-service pause was authorized.
The current original Homebrew daemon was inspected read-only and left running.

Implemented strict `--destroy SOURCE [--to TARGET]`, nearest preceding-then-following
valid local selection, explicit cross-display targeting, last-workspace protection,
and required local visible-Space replacement. Deferred operation tokens hold stable
mapping/window identities while native events and read queries continue. Existing
native move/untile/retile paths remain unchanged; observed membership reconciles
optimistic bookkeeping. Source mappings survive unsupported/uncertain occupancy,
partial movement, topology changes and unobserved handoff. Successful release
preserves survivor IDs/indices/order; later creation can reuse the spare with a
fresh ID. See [command/classification details](vspace.md) and
[architecture](architecture.md#milestone-2-b--evacuate-and-release).

Executed on macOS 15.6 arm64 (24G84), SDK 15.5, SIP enabled:

- **Passed:** `./scripts/test-vspace.py headless`, including `make`, persistent
  `yabai-fork Development` signing, strict all-architecture signature verification,
  x86_64/arm64 binary checks, help/version, **43/43 production tests**, **16/16 runner
  tests**, shell syntax and whitespace. Full build report:
  `/var/folders/cf/byjbs_c51nb5gm4vtq8jkpv40000gn/T/yabai-v-tests-yr6hn1ae/report.json`.
  Production compilation had no warnings; test compilation retains only the
  five existing upstream CoreVideo deprecations. Four test-format warnings from
  an earlier iteration were corrected before this final full run. The final
  production binary SHA-256
  is `b8649020b8b45d51a8614fa1837dbf91b35bff496ede36d06a3b09664a54a76c`.
- **Passed:** arm64 Clang static analysis, zero diagnostics, retained in
  `/private/tmp/yabai-v-m2b-analysis-commit.plist` and its `.log` sibling.
- **Passed (read-only native adapter probe):** live uncertain occupancy rejected
  destruction from native Space 3 to 7 before mutation, preserving the mapping
  snapshot and active Space. The probe used untracked native candidates, with
  native focus/move callbacks replaced by guards. It is not a second daemon or
  an AX/window-management session. Report:
  `/private/tmp/yabai-v-m2b-native-probe-final.jsonl`; source/build/log siblings
  retain the probe procedure. IDs are historical and must be rechecked.
- **Skipped:** native adapter release/reuse, because no eligible hidden empty
  spare existed on the focused display at probe time. No Space/window was
  created or rearranged to manufacture a candidate.

Production fixtures cover empty hidden release/reuse, distinct IDs and sparse
indices, interleaved displays, preceding/following local selection, invalid/same/
unavailable targets, last mapping, visible/background/focused cross-display
handoff, source-window lifetime and reused IDs, deferred move completion,
partial failure/retry without duplicate moves, source arrivals, topology changes
before/during evacuation, unsupported/unknown occupancy, late layout-settlement
failure before release, stale tokens, busy mutations, strict parsing, and real
socket response ownership across temporary-storage reset. Native class policy
fixtures include floating/minimized/application-hidden root windows, dialogs,
children, sticky/fullscreen and unknown roles. The suite also retains ping/list/
current/create/native-focus and float/BSP/stack command regressions. These are
policy/dispatcher fixtures, not proof of populated native tiling or actual moves.
Initial fixture macro compile errors were corrected before passing validation.

The maintained runner adds negative destroy checks to read-only mode and opt-in
`interactive --destroy-created` for this run's unchanged hidden empty allocations.
Its new release orchestration passed two fixture regressions; it has not run on
an interactive desktop. Populated/minimized/hidden-window movement, native
float/BSP/stack bookkeeping, source visibility on the other physical display,
focus preservation and recovery remain **unrun for M2-B**. Earlier M2-A GUI
results do not validate these new paths. Use the prepared manual procedure in
[development](development.md#manual-daemon-smoke-test); an original-service pause
requires explicit approval under `AGENTS.md`. Do not test against user workspaces
without prepared disposable windows and an audited recovery plan.

New hosted macOS 15 CI is unrun because publication is not authorized. macOS 26
build/runtime remains deferred under D012. No original binary/configuration/service
file, TCC/Keychain trust, scripting addition or SIP setting changed. Temporary
reports are local evidence, not durable cross-version compatibility claims.

Final follow-up checks passed: all 16 runner regressions with Apple's Python
3.9.6 as well as the default Python 3.15, all local Markdown links/anchors across
nine files, and `git diff --check`. The full report identifies baseline `cf20e36`
with its pre-commit diff; the local implementation commit contains that source.
The source build/signature and complete headless checks were rerun after final
AX lifetime hardening, rather than relying on the earlier test-only run.

Checkout reconciliation: the older resume record called M2-A/runner unpublished,
but the observed `origin/feat/vspace-mapping` ref is already `cf20e36`, with a local
reflog entry `update by push`. That cached publication checkpoint includes M2-A
and the runner. No fetch/network verification or push was performed in this task,
and no new hosted CI result is inferred. Only the new M2-B work is unpublished.

## Milestone 2-B approved GUI validation — 2026-11-05

The user approved pausing the original service, running the persistently signed
checkout as the sole daemon, testing disposable windows on existing native
Spaces, and restoring/auditing the original session. They manually prepared
additional empty desktop Spaces on display 2 and confirmed an idle desktop. The
interruption occurred before any service pause; process/service state was checked
again on continuation. Native fullscreen/desktop state had changed externally,
so historical Space/window state was not reused as the new baseline.

Production source stayed at **`ca48ac8c964900264da4d6e4b0928a9c0c16c6c7`** throughout.
macOS 15.6 arm64/SDK 15.5, two physical displays, SIP enabled, no scripting addition.
Sessions used the same persistently signed checkout binary and verified its
signature before startup. No production
source, configuration, service definition or installed binary was modified.
Temporary AppKit fixture windows and native-query helpers were built under
`/private/tmp`, not added as a GUI product or installed service. Only disposable
fixture windows were targeted by evacuation/minimize/hide/cleanup requests; no original
application/window was closed or used as a destroy source.

### Initial cache mismatch and padding diagnosis

Attempt `/private/tmp/yabai-v-m2b-gui.oxsvv6qz` stopped before any lifecycle command
or fixture launch. The original cached Zotero frame `(0,0,1580,855)` differed from
the newly initialized daemon's `(10,45,1332,823)`. Original-service recovery also
reported Chrome height 875 versus the prior cached 855. Absolute resize through
the original CLI was rejected because Chrome was managed; no resize succeeded.
The original daemon, focus/visibility, native order, window lifetimes/associations,
layouts, file hashes and SIP recovered; cached frames did not match that attempt.
Its independent audit is `oxsvv6qz/independent-recovery-audit.json`.

The user asked whether padding caused this. Read-only global/per-Space queries
confirmed **10 pixels on all sides** and no external bar. The native work-area
probe using the existing calculation showed:

| Display | Native display bounds | Menu bar | Intended area with 10-pixel padding |
| --- | --- | --- | --- |
| 1 | `(0,0,1352,878)` | 35 pixels | `(10,45,1332,823)` |
| 2 | `(-248,-900,1600,900)` | 25 pixels | `(-238,-865,1580,855)` |

Zotero's current frame exactly matches display 1's padded area. Its older cached
size matches display 2's padded dimensions despite a display-1 mapping. Native
Chrome bounds were height 875, while the padded target was 855; unchanged padding
alone does not establish why the observed/requested sizes differed. Tracked-window
JSON uses `window->frame`, an event-updated cache. **No independent pre-pause native
bounds were captured for the first attempt**, so its comparison cannot prove
physical movement or establish an exact historical cause. It remains inconclusive.
Evidence: `/private/tmp/yabai-v-m2b-padding-investigation.json`,
`/private/tmp/yabai-v-m2b-padding-probe.json` and their temporary source/build logs.

The user explicitly chose to continue with a measured native-frame baseline,
keeping original configuration unchanged. The revised temporary harness captures
WindowServer bounds, native owner PIDs and exact Space memberships before the
pause and during tests/recovery; cached frames are recorded separately. See D014.

### Measured repeat and complete session

Repeat `/private/tmp/yabai-v-m2b-gui.chyw7zi8` passed real query/negative CLI checks,
empty hidden release/reuse and four-window BSP evacuation, including minimized
and floating states. It then stopped at an incorrect **harness assertion** that
BSP leaves should have stack index 1. Production queries use zero for a singleton
BSP leaf; the saved nodes, split fields, frames and native memberships were correct.
The assertion was corrected and checked against the retained snapshot. This was
not a daemon change. Every recovery condition, including native frames, passed.

Complete evidence: **`/private/tmp/yabai-v-m2b-gui.mhi73ua9`**.
Fresh baseline: one original daemon, native order `3/7` on display 1 and
`848/867/871` on display 2, visible Spaces `3/848`, global Space 848 and Chrome
window 1052 focused. The two prepared empty desktops were 867 and 871. Startup
preserved independent native frames/membership and cached frames. All IDs/PIDs
here are historical and must be rechecked before reuse.

| Live check | Observed result |
| --- | --- |
| Maintained `test-vspace.py query` | Passed exact ping, list/current, native display/Space/window queries and negative CLI/no-mutation checks |
| Empty hidden source, explicit cross-display destination | Released virtual ID/index 4 at native 867; survivors kept indices `1,2,3,5`, IDs and order |
| Create after release | Reused native 867, appended index 6 with fresh ID 6; later releases/reallocations used fresh IDs through 11 without compacting gap 4 |
| Same source/target | Clear preflight rejection without mapping mutation |
| Focused mixed source → BSP, stack, float | Four live windows evacuated; native source empty and mapping released; destination globally focused |
| Populated destination bookkeeping | BSP split nodes/first-last members, stack indices `1/2`, and no managed nodes for float all matched their layouts |
| Minimized/floating windows | Independent native membership confirmed all four at the destination; minimize and explicit float flags preserved |
| Application-hidden windows | All four evacuated while the fixture application stayed hidden; unhide subsequently restored BSP management of ordinary tiled windows |
| Source visible on unfocused display 2 | Local destination 871 became visible; global Space 3 and Orca window 6369 focus were restored |
| Explicit populated cross-display evacuation | Native membership/display association changed to native 3/display 1; display 2 retained mapped visible 871; destination received global focus |
| Original user windows throughout | Every original native owner, membership and physical frame preserved; no original minimize/hidden state changed |

The four fixture windows were two ordinary tiled roots, one explicitly floating
root, and one minimized root. The minimized window was observed as an AX dialog
subrole after minimization and was still accounted for and evacuated. These tests
used the production `window_manager_send_window_to_space` path under SIP. No
custom positioning workaround or stronger native gesture guarantee was added.
The full harness had no test/recovery errors and empty development stderr. Fixture
windows closed only after completion checks, as test teardown; the application
and all windows stayed open through each destroy operation.

A final focused regression session, **`/private/tmp/yabai-v-m2b-gui.pqaohqqa`**, passed
valid `vspace --focus` same-display and visible cross-display round trips, observed
`--current`, matching already-focused errors, unchanged mappings and independent
native window geometry. It restored the original session again. This closes the
remaining live focus regression check without rerunning populated evacuation.

After that session, a separate read-only audit against the complete session's
**pre-pause native baseline** passed every condition:
`mhi73ua9/independent-final-recovery-audit.json`. It confirmed one original Homebrew
service (historical PID 12255), original executable, native topology/order,
visibility/global Space/window focus, all original window lifetimes and native
frames/memberships, matching cached frames, original stack layouts and all
10-pixel padding/gap values, all three installation hashes, SIP enabled, and no
remaining development fixture. The user-prepared native desktops remain present;
no native Space creation/deletion/reordering was performed by the test harness.

**Still headless-only or unrun:** last-total-mapping protection; missing local
replacement/default target with unavailable records; interleaved global indices;
unsupported/sticky/child/fullscreen source injection; forced partial-move failure,
arrival/disconnect and reused-ID races; physical single-display M2-B repeat;
macOS 26; and new hosted CI. Unsafe user-desktop fault injection was deliberately
not manufactured. `interactive --destroy-created` orchestration remains fixture-
tested; the maintained runner's real query mode ran, while the temporary GUI
harness exercised actual lifecycle commands. No push/merge was authorized or run.

The user subsequently confirmed: **“Yes, switching and return looked correct.”**
This visual confirmation is separate from native dispatch/query observations and
is retained in `mhi73ua9/user-visual-confirmation.json`. The completed approved
GUI session requires no additional service/desktop changes.

## Milestone 2-C refresh and local validation — 2026-11-05

Implemented the user's M2-C prompt from clean `e23bd17` on `feat/vspace-mapping`,
a descendant of completed M2-A/M2-B. Refresh releases only confirmed empty hidden
valid mappings, compacts survivors in existing global order, and returns bounded
JSON with released mappings, changed indices and retention reasons. It rejects a
pending destroy, retains uncertain mappings, and publishes a separate candidate
array only after native revalidation. No upstream native handler/layout changed.
See [CLI contract](vspace.md) and [D015](progress/decisions.md#d015--refresh-virtual-metadata-explicitly-and-conservatively).

Executed on macOS 15.6 arm64 (24G84), SDK 15.5, SIP fully enabled:

- **Passed:** `./scripts/test-vspace.py headless --output
  /private/tmp/yabai-v-m2c-validated`: `make`, persistent `yabai-fork Development`
  signing, strict all-architecture signature, universal x86_64/arm64 checks,
  version/help, **52/52 production tests**, **19/19 runner regressions**, shell
  syntax and whitespace. Report: `/private/tmp/yabai-v-m2c-validated/report.json`.
  Production compilation had no warnings; tests retain the five preexisting
  upstream CoreVideo deprecation warnings and no new warnings. Binary SHA-256:
  `880b45f22307dda5651c6abe58482b30bc586ea19ab6fdf5508f1589ca6632f0`.
- **Passed:** all 19 runner regressions with Apple's `/usr/bin/python3` (3.9.6)
  as well as the standard runner's Python 3.15.
- **Passed:** arm64 Clang static analysis, **zero diagnostics** in
  `/private/tmp/yabai-v-m2c-analysis-final.plist`; `.log` is empty.
- **Passed:** local Markdown links/anchors across nine files and `git diff --check`.
- **Passed (read-only native adapter probe):**
  `/private/tmp/yabai-v-m2c-native-probe` used production topology/visibility/identity/
  occupancy queries with isolated hypothetical in-memory mappings. It started no
  daemon, registered no window manager, and replaced native focus/move callbacks
  with guards. With two physical displays and six native Spaces, it released
  hidden empty native desktops 867/871 from its local mappings, retained the
  background visible mapping, two unknown-occupancy mappings and a non-desktop
  mapping, and passed a second unchanged refresh. Native topology/order/visibility/
  global focus were unchanged, `next_id` was preserved, and no guarded mutation
  callback ran. Evidence/source/build-log siblings:
  `/private/tmp/yabai-v-m2c-native-probe.jsonl`, `.m`, `-build.log` and `.log`.
  These IDs are historical. This is native-query evidence, **not** real daemon,
  tracked AX window-class, IPC, frame/layout or GUI validation.
- **Passed (existing service, read-only):** new client native display/Space/window
  queries returned valid JSON against the original Homebrew daemon. Private
  snapshots: `/private/tmp/yabai-v-m2c-original-{displays,spaces,windows}.json`.
  No lifecycle/focus/configuration request was sent to that daemon.

Production fixtures cover interleaved A1/B1/A2/B2/A3/B3 filtering to A1/B2/A3/B3;
destroy-style index gaps; all-empty visible mappings on both displays; the last
mapping when visible native desktops are unmapped; ordinary floating/minimized/
application-hidden/sticky occupants and unknown classifications; unavailable,
stale/reassociated and disconnected mappings; failed/duplicate/incomplete
topology/visibility/focus snapshots; occupied/visible/focused/identity changes
before publication; unavailable/changed topology during final occupancy; coherent
old mapping generation during native callbacks; pending destroy source/target
protection with successful subsequent completion; exact CLI result/errors and
unchanged list contract; idempotence and fresh-ID appended creation with occupancy
revalidation and focus through compact indices. All M2-A/B/native dispatch/layout
regressions remain in the suite. These are deterministic policy tests, not physical
window movement or display-configuration passes.

Runner regressions verify refresh is explicitly enabled, global compaction and
repeat reporting, and rejection of observed visible release/incorrect index
reports. The maintained interactive runner's new `--refresh-mappings` stage has
not run on a real desktop. It may release eligible preexisting mappings and is
never invoked by ordinary query/default interactive checks.

Initial test compilation found fixture macro/initializer issues; a fixture enum
also shadowed the harness's result variable and was renamed before the full pass.
The runner's malformed release-order fixture was corrected before validation.
Sandbox signing/verification could not access the trusted identity; the final
build/signature/test run used the existing identity outside the sandbox without
changing trust/TCC. Earlier incomplete/failed reports remain separately at
`/private/tmp/yabai-v-m2c-headless*`; they are not counted as full passes.

**Unrun/gated:** real M2-C daemon/IPC/GUI validation, physical single-display
repeat, new hosted macOS 15 CI, and macOS 26 build/runtime. The original daemon
was observed as `/opt/homebrew/bin/yabai` (historical PID 12255); no service pause
or competing daemon was attempted. A new measured GUI/recovery session requires
explicit service approval under AGENTS.md; completed M2-B sessions are historical
evidence only. Hosted CI requires authorized publication; macOS 26 stays deferred
under D012. No service/configuration/installed binary, TCC/Keychain trust, scripting
addition or SIP setting changed; no push or merge occurred.

## Milestone 2-C approved GUI validation — 2026-11-05

The user approved pausing Homebrew yabai, running the signed checkout as the sole
daemon for refresh/create/focus checks, and restoring/auditing the original
desktop/service. The first fresh baseline had two displays, native Spaces `3/7`
and `848/867/871`, visible `3/848`, global Space 3 and Orca window 6369 focused.
Desktops 867/871 were empty. Independent WindowServer bounds, owners and exact
memberships were captured for all five original windows before the first pause.
All IDs/PIDs are historical and require rechecking.

Daemon source stayed **`31b3035b75063d925fcdc060b719472a177da018`** and the signed
universal binary stayed SHA-256
`880b45f22307dda5651c6abe58482b30bc586ea19ab6fdf5508f1589ca6632f0` throughout.
macOS 15.6 arm64/SDK 15.5, SIP fully enabled, no scripting addition. The only
repository executable change during validation was the narrow Python runner
preflight fix below; no daemon, native handler, configuration or layout code
changed. Temporary recovery/fixture scripts remain outside Git.

### Runner preflight correction and distinct attempts

`/private/tmp/yabai-v-m2c-gui.xxa84kya` passed startup, maintained read-only query
checks and empty destroy/recreate setup, then stopped before refresh: the runner
expected `focus_follows_mouse` to query as `off`, but upstream serializes this
setting as **`disabled`**. The dev config's `off` command had succeeded. Corrected
only the runner's expected native query value; `mouse_follows_focus` still requires
`off`, and autofocus/autoraise/enabled mouse modes remain rejected. Added two
preflight regressions. This is a runner defect, not a vspace or configuration fix.
Every original-service recovery condition passed.

`jungx6qq` then passed the maintained interactive refresh stage, compaction and
create/focus checks below, but stopped at fixture setup: AppKit's explicit window
coordinates placed the disposable window on the primary display. `9otq8z8c` added
explicit native placement of only the fixture, then exposed a second harness
precondition: switching global focus to display 1 left the source visible on
display 2, so refresh correctly reported `visible`. The harness's expectation of
`occupied` was inappropriate for that setup. Both attempts independently
recovered every original condition and remain recorded as partial attempts.

The focused follow-up `5tf4x6i3` explicitly selected the original desktop on the
source display before each occupancy refresh, ensuring the source was hidden.
It completed all four isolated window cases, empty release and repeat checks
with no test/recovery errors. Only disposable windows were placed/minimized/hidden/
closed; original windows were never moved, minimized, hidden or closed by tests.

### Live results

Core report: **`/private/tmp/yabai-v-m2c-gui.jungx6qq`** (also repeated in
`9otq8z8c`). Occupancy completion: **`/private/tmp/yabai-v-m2c-gui.5tf4x6i3`**.

| Live check | Observed result |
| --- | --- |
| Maintained runner query mode | Passed ping/list/current, native queries and negative CLI/no-mutation checks |
| Empty allocation and destroy gap | Allocated IDs 4/5 at 867/871, destroyed hidden ID 4, then recreated 867 as fresh ID 6/index 6; survivor indices remained `1,2,3,5,6` until refresh |
| Real `interactive --refresh-mappings` stage | Released only hidden empty ID 5/native 871; kept empty background-visible ID 6/native 867 and compacted its index from 6 to 4 |
| Identity, order and diagnostics | Survivors remained IDs `1,2,3,6` with the same native/display associations; focused 1, occupied 2, uncertain-occupancy 3 and visible 6 were diagnosed separately |
| Repeat refresh | Maintained runner's second refresh and later direct repeats changed no IDs, mappings or indices |
| Empty visible Space on unfocused display | Native 867 remained mapped while display 1 retained global focus; explicit refresh returned `visible` for it |
| Explicit empty release | After selecting original native 848 on display 2, refresh released the now-hidden empty 867 mapping |
| Create/focus after compaction | Fresh ID 7 appended at index 4 (`N+1`); compact-index and new-index focus/current resolved correctly; native/virtual same-display focus and already-focused parity passed |
| Isolated ordinary/floating/minimized/hidden application | One disposable source window at a time, exact native source membership, source hidden; all four returned `occupied` and retained the mapping with window state preserved |
| Fixture close then refresh | Once its last window closed, refresh released that hidden mapping; next refresh was unchanged |
| Refresh native invariants | Per-call native topology/order, visibility/global/window focus, layouts, cached frames/associations and original independent native bounds/memberships remained unchanged |

The maintained interactive runner used a pseudo-terminal with the already
approved `RUN` input; its visual answer was explicitly **`unobserved`**, not an
agent-supplied positive confirmation. Actual user visual feedback is separate.
Native setup/recovery focus sometimes needed a retry after successful dispatch;
these are separately logged and do not change M2-A's native result semantics.

### User-observed repeat

The user had not watched the initial checks and explicitly requested a repeat.
`/private/tmp/yabai-v-m2c-gui.bga1388z` passed observed virtual focus on the primary
display from native 3 to 7 and back, with eight-/six-second viewing pauses. Its
secondary-display request returned zero but global activation was unobserved;
the harness stopped and fully recovered, so that attempt is not a full pass.

The focused secondary repeat **`/private/tmp/yabai-v-m2c-gui.r63fdz3z`** recorded
the same native fallback limit: its first request made 867 visible while global
focus remained at native 3. A separately logged second request then focused the
already-visible target. It held that desktop eight seconds, returned display 2
to native 848 for six seconds, then returned the original primary desktop for
six seconds. Actual activation was observed at each successful stage. These are
different visibility conditions; no guaranteed first-dispatch activation or
stronger SIP behavior is claimed. No daemon code change was made.

The user then confirmed: **“Yes, switching and return looked correct.”**
This feedback is stored in `r63fdz3z/user-visual-confirmation.json`, separate from
the maintained runner's earlier `unobserved` answer. All service/desktop recovery
conditions passed after both repeats. The final independent audit after the
watched repeat, again against the very first `xxa84kya` native baseline, is
`r63fdz3z/independent-final-recovery-audit.json`; **every condition passed**.
The sole original daemon's observed PID was 48377 (historical). No fixture or
development daemon remains, and no further service action is pending.

The occupancy fixtures used the development float layout. Prior populated
BSP/stack/float M2-B evidence remains historical; original stack layouts and all
10-pixel padding/gaps were restored and verified here. No new layout behavior
or native Space creation/deletion/reordering was introduced.

### Validation and recovery

**Passed:** all **21 runner tests** on Python 3.15 and Apple's 3.9.6. The standard
headless runner with `--skip-build` passed strict signature/universal/version/help,
**52/52 production tests**, **21/21 runner tests**, syntax and whitespace:
`/private/tmp/yabai-v-m2c-gui-runner-fix-headless/report.json`. Rebuilding was
explicitly skipped because no daemon C/Objective-C source changed; the existing
persistently signed binary was checked and used throughout. The prior full build
and zero-finding analyzer evidence remain unchanged.

A separate read-only recovery audit after occupancy completion compared against
the **first pre-pause baseline** from `xxa84kya`, not merely the last daemon's
snapshot: `5tf4x6i3/independent-final-recovery-audit.json`. Every condition passed:
one original executable/service, exact native topology/order/visibility/global
Space/window focus, all original window lifetimes/owners/memberships/states/native
frames, matching cached frames, original per-Space stack layouts/10-pixel padding/
gaps, original binary/config/service-definition hashes, unchanged signed checkout
binary, no remaining fixture, and SIP fully enabled. Each earlier recovery also
passed. No TCC/trust, installed file/config/service definition, scripting addition,
SIP, push or merge change occurred.

**Still headless-only/unrun:** simultaneously empty visible mappings on both
physical displays (display 1 had occupied desktops), interleaved occupied global
survivors, last-total-mapping protection, pending destroy conflict, forced query/
identity/topology/arrival races, physical single-display M2-C GUI, new hosted CI,
and macOS 26. Real uncertain occupancy retention was observed for native 848;
unsafe fault injection and user-window relocation were not manufactured. Hosted
macOS 15 checks need authorized publication; macOS 26 stays deferred under D012.

## Milestone 2-D persistence and local validation — 2026-11-07

Implemented the user's M2-D prompt from clean `dbe2cde` on
`feat/vspace-mapping`, the completed M2-C descendant. Added bounded versioned
private state, lossless IDs/high-water mark, UUID-validated startup restoration,
unresolved records and additive availability/durability reporting. Create/refresh/
final destroy removal persist complete candidates before publication. Atomic
replacement is the commit point; subsequent durability errors publish the committed
generation and are reported separately from rejected pre-replacement writes.
See [D016](progress/decisions.md#d016--persist-logical-identity-with-validated-native-uuids)
and [schema/recovery](development.md#virtual-workspace-state-and-recovery).

Executed on macOS 15.6 arm64 (24G84), SDK 15.5, SIP fully enabled:

- **Passed:** final standard headless runner, persistent `yabai-fork Development`
  signing and strict all-architecture verification, universal x86_64/arm64 build,
  version/help, **68/68 production tests**, **24/24 runner tests**, shell syntax
  and Git whitespace. Final report: `/private/tmp/yabai-v-m2d-final/report.json`.
  No production build warnings; tests retain the five upstream CoreVideo
  deprecation warnings. Earlier signed pass (67 tests before the final added CLI
  regression) is `/private/tmp/yabai-v-m2d-validated/report.json`. Post-review
  availability filtering in the runner was checked with the unchanged signed
  binary using `--skip-build`:
  `/private/tmp/yabai-v-m2d-final-checked/report.json`. No daemon source changed
  after the full final build.
- **Passed:** arm64 Clang analysis, zero diagnostics:
  `/private/tmp/yabai-v-m2d-analysis-final.plist`. Compiler driver framework flags
  were omitted for analysis to avoid irrelevant unused-linker warnings.
- **Passed:** runner regressions with Apple's Python 3.9.6 as well as the standard
  runner's Python 3.15; local Markdown links/anchors and whitespace checked.
- **Passed (read-only native identity probe):** the production identity adapter
  returned unique nonnil Space UUIDs and one common expected display UUID for
  both ordinary desktops on the currently connected physical display.
  `/private/tmp/yabai-v-m2d-identity-probe-native.json`; source/build-log siblings
  are retained outside Git. The probe used only native queries, initialized no
  window manager/daemon, wrote no state file and made no focus/window/layout call.
  This establishes availability of the identity fields on this host, not their
  persistence across a native lifecycle event or macOS reboot.
- **Passed (existing service, read-only):** native CLI queries returned JSON for
  one display, two Spaces and four windows, and exactly one yabai process was
  observed. Private results: `/private/tmp/yabai-v-m2d-original-{displays,spaces,windows}.json`.
  No vspace mutation/configuration/focus request was sent to that service.

New production regressions use real private temporary files and the production
codec/publication paths. They cover hidden-empty create/restart, middle/tail
removal and retired allocator IDs, index gaps, explicit refresh/restart, fresh
creation, interleaved two-display order, synthetic runtime display/Space renumbering
with unchanged UUIDs, disconnect/reconnect, reused numeric IDs with changed UUIDs,
missing/duplicate native Space/display identity, repeated unresolved startup,
uint64 extrema, invalid types/overflow/duplicate IDs/UUIDs/indices/order/high-water,
truncated/oversized recovery with byte preservation, preservation failure and
future schema fallback. Lock contention prevents an independent store owner from
writing. Checked-allocation and real directory-permission failures leave the old
generation; injected partial ENOSPC and pre-rename failures reject create/refresh/
destroy metadata. Post-replacement sync faults publish and diagnose uncertainty.
Forked process exits before and after rename test crash recovery and abandoned-temp
cleanup. Mid-evacuation restart observes actual fixture membership, retains the
source and moves only remaining windows on retry. Unresolved records cannot focus,
serve as destinations or satisfy last-usable protection; refresh retains them.
CLI tests distinguish moved-window/source-retained errors from committed uncertainty.
These are deterministic native-policy fixtures, **not** actual display/Space
recreation, window evacuation, daemon restart, reboot or GUI passes.

Runner checks accept unresolved numeric hints without treating them as active
native ownership/focus, validate durability states and use only available mappings
for candidate/focus/destination comparisons. Query/default interactive checks do
not invoke refresh or restart a daemon. Focus/layout/native CLI regressions remain
in the production suite; no native domain handler or layout algorithm changed.

Initial sandbox signing could not access the persistent identity; its failed report
is `/private/tmp/yabai-v-m2d-headless/report.json`. The native probe also saw no
WindowServer displays in the sandbox. Authorized build/signing and read-only probe
execution outside it completed without changing Keychain trust/TCC. Early test
attempts corrected fixture macro syntax/visibility assumptions before the final
passes; they are not relabeled as full passes. Final review also fixed repeated
blank unresolved-hint appends after ambiguous native evidence.

**Unrun/gated:** real M2-D daemon/IPC/restart/GUI validation, physical two-display
reconnection/renumbering, Dock/native recreation, macOS reboot, actual full-volume
ENOSPC/power-loss, new hosted macOS 15 CI and macOS 26. Only simulated/isolated
versions of these failure/topology conditions ran. A new service pause/sole-daemon/
recovery session requires explicit approval under AGENTS.md and D014's independent
native-frame baseline. Existing service, installed binaries/configuration/plist,
TCC/trust, scripting addition and SIP were not changed. No second daemon, push or
merge was attempted. Future publication needs authorization for the reviewed SHA.
