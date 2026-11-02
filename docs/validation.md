# Validation report — 2026-11-01

Latest update: the [2026-11-02 publication and GUI validation](#publication-and-gui-validation--2026-11-02)
records passing macOS 15/26 CI and visually confirmed macOS 15 GUI checks.
macOS 26 interactive runtime remains unverified; the user has no available host.
The initial results below are retained as historical evidence.

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
