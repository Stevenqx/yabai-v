# Validation report — 2026-11-01

Previous completed milestone: [Milestone 1 publication and interactive validation](#milestone-1-publication-and-interactive-validation--2026-11-04).
The macOS 15/26 matrix and a visually confirmed macOS 15 virtual-focus round trip
passed. The original service, desktop, and file hashes were restored/verified.
macOS 26 interactive runtime still lacks a supplied GUI host. Earlier results
and unsuccessful attempts are retained as historical evidence.

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
