# Milestones

Statuses: proposed, in progress, implementation complete, complete.
Validation follow-ups remain open until their results are recorded.

## Milestone 0 — Foundation and validation

Status: complete for the foundation scope; macOS 26 interactive validation remains
an explicitly recorded limitation. Milestone numbering was aligned with the
user’s roadmap on 2026-11-03. All work predating this task belongs to Milestone 0.

### Development environment and vspace smoke test

- [x] Preserve complete upstream history and configure fork/upstream remotes.
- [x] Inspect the build system and build unmodified upstream.
- [x] Establish a persistent development signing identity and build helper.
- [x] Add the isolated `vspace --ping` command and exact JSON response.
- [x] Run six headless tests and live ping/query checks on macOS 15.6 arm64.
- [x] Add macOS 15/26 CI configuration and development documentation.
- [x] Restore the original service and verify unchanged installation files.

Evidence: [validation report](../validation.md).
Commits: `271fb07`, `e1f1968`, `c8c4a93`, `3ec8c49`.

Validation follow-ups:

- [x] Review the six local commits through `de6a849`; no blocking findings.
- [x] Obtain explicit authorization before pushing the task branch (2026-11-02).
- [x] Run the GitHub Actions matrix on macOS 15 and 26 after publication;
  [both jobs passed at `6359db7`](https://github.com/Stevenqx/yabai-v/actions/runs/41403274379).
- [x] Verify window focusing and existing-Space switching in a suitable session;
  queries and user visual confirmation passed on macOS 15.6, 2026-11-02.
- [ ] Validate runtime behavior on macOS 26; no GUI host/session is available
  (confirmed by the user on 2026-11-02).

Historical foundation follow-ups (the implementation gate is superseded by
[D007](decisions.md#d007--align-the-roadmap-and-authorize-mappingfocusing)):

Local follow-up, 2026-11-01: continue the foundation validation follow-ups before detailed
design or implementation. Review the local commits, reproduce available local
checks, inspect publication and GUI-test prerequisites, and record evidence and
remaining blockers. Publication and changes to the running service still require
explicit approval; no future subsystem is in scope.

At that check, six headless tests, shell syntax, workflow YAML/matrix,
history/whitespace, installed-service queries and unchanged original file hashes
passed. The persistent-certificate rebuild and signature verification passed
after manual Keychain authorization. GitHub read access works outside the
sandbox, but the task branch is unpublished and
Actions has no runs. Display 1 has two ordinary existing Spaces for an approved
GUI test; suitable disposable windows still need preparation. See the
[follow-up evidence and execution plan](../validation.md#validation-follow-up--2026-11-01).

Validation execution, 2026-11-02: the user approved publishing `feat/vspace-smoke` at
`6359db7` and running macOS 15/26 CI, plus a brief GUI session that pauses the
Homebrew daemon, uses two disposable terminal windows, tests focus and existing
Space switching, and restores the original service, Space, and focus. Recheck
current state, execute those checks, and record results. No merge or future
subsystem work is in scope.

Result: the approved branch was published, both CI jobs passed, and macOS 15.6
focus/Space tests passed queries and user visual confirmation. The original
service, visible Spaces, and focus were restored with matching original file
hashes and SIP enabled. The two windows from the confirmed repeat remain open.
The user has no macOS 26 GUI host/session, so that runtime follow-up remains
unrun and detailed design/implementation remain deferred. See the
[latest evidence](../validation.md#publication-and-gui-validation--2026-11-02).

### Development process records (formerly M2)

Status: complete, 2026-11-01.

Scope: documentation and agent instructions.

- [x] Create a main progress document with linked supporting documents.
- [x] Record the existing milestone, decisions, results, and outstanding work.
- [x] Require agents to read and maintain the records through `AGENTS.md`.
- [x] Check document links and whitespace, then record the result in the journal.

### Conventional Commits and GitHub workflow (formerly M3)

Status: complete, 2026-11-01.

Scope: agent instructions and development records.

- [x] Record the completed progress-document task in a focused local commit.
- [x] Require local Conventional Commits for completed, validated work.
- [x] Document focused staging, task branches, history preservation, and PR checks.
- [x] Check documentation links and whitespace and commit the policy update.

## Milestone 1 — Virtual workspace mapping and focusing

Status: complete for the authorized mapping/focusing scope, 2026-11-04.
macOS 26 interactive runtime remains a documented follow-up without a GUI host.
Task branch: `feat/vspace-mapping`, based on local checkpoint `93a59ed`.
The user explicitly authorizes implementation despite the unavailable macOS 26
GUI host. Earlier validation gates are superseded for this scope; the missing
runtime evidence must remain visible.

Scope: an independent daemon-owned VirtualWorkspaceManager, one-to-one native
Space mappings, session-stable internal IDs and globally unique visible indices,
and `vspace --list`, `--current`, and `--focus N`. Preserve `--ping` and all
upstream domain semantics, layouts, build system, and installed service.

- [x] Inspect the branch, clean tree, previous changes, records, and native APIs.
- [x] Align earlier development tasks under Milestone 0 and record active scope.
- [x] Initialize in native display/Space order using native IDs and occupancy.
- [x] Include ordinary floating windows and conservatively retain unknown Spaces.
- [x] Implement documented JSON queries and native focus with CLI failures.
- [x] Test mapping, ordering, lookup, duplicate prevention, layouts, and bad input.
- [x] Build locally, verify persistent signing, and run headless/regression checks.
- [x] Publish approved `6c4a354` and pass macOS 15/26 CI (22 tests per host).
- [x] Pass the approved macOS 15 interactive focus repeat with visual confirmation
  and verified original-service/desktop/hash recovery.
- [x] Update technical docs, validation, decisions, journal, and resume instructions.
- [x] Commit completed, validated units locally with Conventional Commits.

Local validation: signed universal build, 22/22 headless tests, static analysis,
read-only native occupancy/spare probe and original CLI queries, installation
hashes, links, and whitespace passed.
See [Milestone 1 evidence and test plan](../validation.md#milestone-1-commands-and-local-validation--2026-11-03).

Validation execution, 2026-11-04: the user approved publication of `6c4a354` and
an interactive session. [CI run 41779098539](https://github.com/Stevenqx/yabai-v/actions/runs/41779098539)
passed both platform jobs. The interrupted first focus attempt is retained as
inconclusive after reported manual Mission Control switching. An unchanged-native
control and the idle virtual-focus repeat passed. The repeat registered empty
active Space 7, left empty inactive Space 592 spare, and focused 7 → 3 → 7 → 3
with stable IDs/indices and unchanged window frames/layouts. The user confirmed
visible switching; a final independent audit verified recovery of original
service, visible Space, focus, original window frames, file hashes, and SIP.

The local config fix `b195681` uses accepted animation float `0.0`, replacing
rejected integer `0`. It and the subsequent validation records are unpublished;
no C source changed after the CI-tested checkpoint. Details, unsuccessful
attempts, helper assertions, and evidence paths are in the
[latest validation](../validation.md#milestone-1-publication-and-interactive-validation--2026-11-04).

Remaining follow-up: macOS 26 GUI runtime requires a supplied host/session.
Future publication/service changes remain subject to `AGENTS.md`; no merge is
authorized. The M2-A request below supersedes the former Milestone 2 gate.

## Milestone 2-A — Allocate existing spares and use native focus

Status: complete for authorized implementation and available local validation,
2026-11-04. New hosted CI and additional runtime configurations remain follow-ups.
Authorized by the user on 2026-11-04.
Task branch: `feat/vspace-mapping`, clean baseline `8c5d0b3` (M1 descendant).

Scope: `vspace --create` maps the first confirmed-empty, hidden, unmapped native
desktop on the focused display, with current-state revalidation and no desktop
mutation. Append session-local identity/global index without changing survivors.
Make virtual focus return the native focus result without M1 polling. Preserve
all existing commands, layouts, native handlers, signing, and service boundaries.
Record M2-B/C/D and native MRU proposals; only M2-A implementation is authorized.

- [x] Inspect clean branch, M1 checkpoint, records, build system, and native paths.
- [x] Implement native focus result parity and regression coverage.
- [x] Implement conservative focused-display allocation and revalidation.
- [x] Cover one/two-display exhaustion, ordering/identity, unsafe and stale state.
- [x] Build/sign/verify locally; run headless tests and appropriate checks.
- [x] Record available runtime evidence separately from simulations and CI.
- [x] Update technical docs, follow-on design, decisions, journal, and resume notes.
- [x] Create focused local commits; publication and service changes need approval.

Focus checkpoint: `86dabb5`. Allocation, help, tests, and design records follow in
the next local commit; neither M2-A commit is published. Final local checks:
signed universal build, 30/30 headless tests, zero arm64 analyzer findings,
shell/Markdown/whitespace checks, and read-only native-state allocation probes.
The probes exercised one display, then fresh two-display topology, including
focused-display exhaustion while another display retained an eligible spare.
They did not start a daemon, change focus, or establish GUI behavior.

Live validation authorized, 2026-11-04: the user approved briefly pausing the
Homebrew service, running the signed checkout as the only daemon, and restoring
and auditing the original service/desktop. Recheck fresh topology (currently two
displays), test allocation/native-focus parity and safe layout modes, and record
results. This approval does not authorize publication or later implementation.

Unlocked repeat, authorized 2026-11-04: the user manually unlocked the Mac and
requested continuation. AX focus metadata is available again. External display
identity/topology, native Spaces, and windows changed since the locked attempt;
use a fresh recovery baseline. Continue allocation/focus/layout checks under the
existing single-daemon approval, with no publication or implementation changes.

Open validation:

- [x] Live macOS 15 focused-display allocation/exhaustion and no cross-display fallback.
- [x] Allocation/exhaustion and native/virtual focus comparisons in an unlocked GUI.
- [x] Live two-display/cross-display focus results and empty-view layout smoke checks.
- [ ] Standalone single-display GUI repeat and populated layout smoke (native/headless coverage exists).
- [ ] New macOS 15 hosted checks for M2-A (publication not authorized).
- [ ] macOS 26 build/runtime checks (deferred by the later standard-runner request).

See [M2-A evidence and execution plan](../validation.md#milestone-2-a-allocation-and-local-validation--2026-11-04).

Approved live attempt: real daemon/IPC creation on display 2 allocated hidden
Space 592, then failed without mutation while display 1 retained spare 7. CLI
regressions and startup/frame/allocation invariants passed. The native focus
control returned zero without observed activation; read-only console state then
confirmed the screen was locked. Virtual focus/other-display allocation/layout
checks were not reached. Original service, Spaces, frames, layouts, hashes and
SIP were independently audited after recovery; window focus is unverified while
locked. Await manual unlock, then recheck recovery and continue under the existing
approval. See [the attempt and recovery](../validation.md#milestone-2-a-approved-gui-validation--2026-11-04).

Unlocked result: the user supplied an unlocked idle session and confirmed visible
switching/return. `iaus8mt4` passed creation of 7/592 in order, display-2 exhaustion
with other-display spares, native/virtual same-display and visible cross-display
focus, already-focused errors, and invariant checks. The completion `chsprfea`
passed remaining focus-result and empty-Space float/BSP/stack checks. Native
hidden cross-display dispatch made the target visible without global keyboard
focus; a subsequent virtual call focused the then-visible target. These different
visibility conditions are recorded, with no stronger native guarantee claimed.

Final independent recovery passed against the first idle baseline, including
exact original frames after restoring two floating windows affected by original
service startup rules. Original service/desktop/focus/layouts/hashes/SIP recovered;
no source changed after `5b199fb`. Interrupted/locked attempts remain separate.
See [final evidence and limits](../validation.md#unlocked-repeats-and-final-recovery).

## Reusable vspace validation runner

Status: complete locally, authorized by the user on 2026-11-04.

Maintain a repository script for automatic headless checks, read-only daemon
checks, and explicitly selected interactive allocation/focus/layout checks on
users' own Macs. Reuse existing tests and the completed M2-A GUI procedures,
record host/version and skipped checks, and preserve the manual single-daemon
service workflow. macOS 15 is the active local runtime target; defer macOS 26
runtime/build checks to a later run of this same script. Default hosted CI uses
macOS 15, with macOS 26 retained as an optional manual run. Report unrun platforms
without claiming compatibility. This supersedes earlier dual-OS validation gates.

- [x] Implement and document the portable runner and result/recovery records.
- [x] Exercise automatic checks and 14 runner regression tests on macOS 15.
- [x] Update validation, decisions, journal, and resume instructions; commit locally.

The signed automatic workflow passed 30 production tests and 14 runner tests on
macOS 15.6. Interactive orchestration passed fake-daemon regressions, including a
full sequence and interruption recovery; the new runner itself has not had a live
desktop run. Earlier M2-A GUI results are separate. Hosted CI remains unrun for
this unpublished change. See [runner validation](../validation.md#standard-vspace-test-runner--2026-11-04).

## Milestone 2-B — Explicit destroy with evacuation

Status: implementation and available macOS 15.6 two-display GUI validation complete
locally; additional configurations remain open.
Authorized by `tmp/yabai-v-m2b-codex-prompt.md` on
2026-11-05. Task branch: `feat/vspace-mapping`, clean baseline `cf20e36`.

Implement `--destroy SOURCE [--to TARGET]` in the manager, with conservative
window preflight, observed evacuation and visible-Space handoff, retained mappings
on partial failure, stable survivor IDs/indices/order, and append-only creation.
Use the existing native move/layout and focus paths with SIP enabled. Serialize
deferred lifecycle completion on the daemon loop using stable identities/tokens.
Refresh, persistence, native lifecycle/reordering, MRU, GUI and hotkeys remain
outside scope. Existing service changes and publication require separate approval.

- [x] Reconcile prompt with completed M2-A/runner baseline and read project records.
- [x] Implement manager lifecycle, CLI/help and concrete safety regressions.
- [x] Run persistent-signed build, headless/static checks and available local probes.
- [x] Record interactive/OS limitations and update technical/progress records.
- [x] Review and commit the coherent validated change locally.

Available validation: signed universal build, 43 production tests, 16 runner
regressions and zero-finding arm64 analysis passed on SIP-enabled macOS 15.6.
The approved two-display GUI session passed empty release/reuse, survivor ID/index
stability, mixed tiled/floating/minimized windows, populated BSP/stack/float,
application-hidden windows, source visibility on the other display/global focus,
and explicit cross-display evacuation. Virtual focus round trips, native setup/
recovery focus and already-focused regressions passed. Original-session conditions recovered
against a measured native baseline; the user confirmed visible switching/return.
Physical single-display and unsafe live fault injection remain unrun/headless-only;
macOS 26 stays deferred and new hosted macOS 15 CI requires authorized publication.
See [M2-B GUI validation](../validation.md#milestone-2-b-approved-gui-validation--2026-11-05).

Active follow-up, 2026-11-05: the user explicitly approved a prepared GUI test
session for local source `ca48ac8`: pause the original service, run the signed
checkout as the sole daemon, test disposable-window evacuation using existing
native Spaces, then restore/audit the original session. Publication remains
unauthorized. Recheck host state and preserve all original applications/windows;
prepare an additional empty ordinary desktop manually for isolated populated
layout checks. Build a temporary fixture/recovery harness outside Git, record
actual outcomes, and retain skipped conditions rather than using user workspaces
as disposable sources.

First approved attempt stopped before any lifecycle command: original and newly
initialized daemon queries disagreed on cached user-window frames. Original
service, native order/visibility/focus, window lifetimes/associations, layouts,
installation hashes and SIP were independently recovered; cached frames differ.
The first attempt did not capture independent pre-pause WindowServer bounds, so
the comparison cannot establish physical geometry changes. Read-only diagnosis
confirmed 10-pixel padding everywhere; display 1's 35-pixel menu bar plus padding
exactly yields Zotero's current `(10,45,1332,823)` frame. The old cached Zotero size
matches display 2's padded dimensions despite a display-1 mapping. This attempt
remains inconclusive. Native bounds were added to the temporary recovery harness,
and the user resolved the baseline preference for the measured repeat.

The user accepted a measured native-frame baseline for the repeat, keeping the
original configuration unchanged. The temporary harness now records independent
WindowServer bounds, native memberships and owner PIDs before service pause and
through recovery; cached frame comparisons remain separate observations.

GUI follow-up complete: source `ca48ac8` stayed unchanged. A harness BSP assertion
was corrected after a successful native evacuation, then the complete `mhi73ua9`
run and focused `pqaohqqa` session passed. Independent final recovery confirmed
one original service, exact native frames/membership/owners, topology/visibility/
focus, matching cached frames, original layouts/padding/gaps/file hashes and SIP.
The disposable fixture was closed only during teardown; original applications/
windows remained open. The user confirmed switching and return looked correct.
No further service work or publication is pending/authorized by this handoff.

## Milestone 2-C — Explicit refresh and global compaction

Status: implementation and available local validation complete;
available macOS 15.6 two-display daemon/GUI checks also passed; additional
platform/scenario coverage remains open.
Authorized by `tmp/yabai-v-m2c-codex-prompt.md` on 2026-11-05.
Task branch: `feat/vspace-mapping`, clean baseline `e23bd17`, descendant of
completed M2-A/M2-B. Implement only explicit `vspace --refresh`: release confirmed
empty hidden valid mappings, protect every display's visible Space and global
focus, retain uncertain mappings, and compact survivors in existing global order.
Prepare and revalidate a separate candidate before atomic daemon publication.
Reject refresh while destroy is pending. Preserve native state, MRU and IDs.

- [x] Verify prerequisites, clean checkout, progress and technical records.
- [x] Implement manager operation, CLI/result/help and focused safety regressions.
- [x] Run signed build, headless/static checks and available native probes.
- [x] Update technical/progress records and record remaining runtime/OS gates.
- [x] Review and commit the coherent validated change locally.

Initial local validation passed: signed universal build, 52 production tests,
19 runner tests (Python 3.15
and Apple's 3.9.6), zero-finding arm64 analysis, Markdown/whitespace checks,
two-display native-query probe with isolated mappings and read-only original
daemon queries. Refresh's maintained runner stage is opt-in and fixture-tested.
No native focus/window/layout mutation, service change or publication occurred.
See [M2-C evidence](../validation.md#milestone-2-c-refresh-and-local-validation--2026-11-05).

- [x] Real M2-C daemon/IPC/GUI checks with measured independent recovery and user visual confirmation.
- [ ] Physical single-display GUI repeat.
- [ ] New hosted macOS 15 CI (needs authorized publication).
- [ ] Deferred macOS 26 build/runtime using the standard runner.

Future service changes and publication need separate approval. macOS 15 remains
the active validation target under D012; macOS 26 stays deferred.

GUI follow-up authorization, 2026-11-05: the user approved pausing Homebrew yabai,
running signed source `31b3035` as the sole daemon for refresh/create/focus checks,
then restoring and independently auditing the original service/desktop. Recheck
console/topology/identities and capture native bounds/memberships before the pause.
Use existing empty desktops and disposable fixture windows; preserve original
windows/configuration and keep publication unauthorized. Record actual GUI
outcomes separately from the completed headless/native-query evidence.

The live runner exposed an existing preflight mismatch: native
`focus_follows_mouse` queries return `disabled` for the accepted `off` setting.
Corrected that narrow runner check, retained enabled-mode rejection, and added
focused regressions as part of this validation follow-up. Native daemon source
and the persistently signed binary stay at `31b3035`.

GUI follow-up complete: core `jungx6qq`/`9otq8z8c` evidence passed real maintained
query/refresh stages, destroy-gap compaction (ID 6/index 6 → 4), retained empty
visible/uncertain mappings, release/idempotence and fresh-ID create/focus. Focused
`5tf4x6i3` completed isolated hidden-source ordinary/floating/minimized/hidden-app
occupancy and close-to-empty release with no errors. Earlier runner/fixture
precondition failures remain distinct; every service recovery passed.

The corrected runner passed 21 regressions on Python 3.15/3.9.6; headless
`--skip-build` passed the unchanged signed binary, 52 production and 21 runner
tests. The user requested a watched focus repeat and confirmed switching/return
looked correct. Primary-display switching passed in `bga1388z`; `r63fdz3z` passed
the secondary repeat after separately logging the native visibility-only dispatch
and a retry. No stronger native activation guarantee is claimed.
The final independent audit in `r63fdz3z` matched the first `xxa84kya` native
baseline for every original service/desktop/window/layout/hash/SIP condition.
No development daemon/fixture remains or further service action is pending.
See [M2-C GUI evidence](../validation.md#milestone-2-c-approved-gui-validation--2026-11-05).

## Milestone 2-D — Durable logical identity and validated restoration

Status: implementation and available local validation complete, authorized by
`tmp/yabai-v-m2d-codex-prompt.md` on
2026-11-07. Branch `feat/vspace-mapping`, clean baseline `dbe2cde`.

Scope: private versioned state, validated native Space/display UUID restoration,
unresolved logical records, durable create/destroy/refresh candidate commits,
conservative recovery and focused persistence/fault regressions. Preserve native
focus/layout paths and the running service. Publication and a new GUI service
session require separate authorization; macOS 26 remains deferred under D012.

- [x] Verify clean descendant checkpoint and read implementation/progress records.
- [x] Implement bounded schema, identity verification, restoration and availability.
- [x] Integrate locked atomic snapshots at lifecycle publication boundaries.
- [x] Cover restoration, conflicts, recovery and pre/post replacement failures.
- [x] Run signed build, headless/static checks and update technical/progress records.
- [x] Review and commit the coherent validated change locally.

Available validation: signed universal build, 68 production and 24 runner tests,
zero-finding arm64 analysis, Python 3.15/3.9.6 runner checks and native read-only
UUID/installed-service query probes passed on SIP-enabled macOS 15.6. Real M2-D
daemon/restart/GUI, physical two-display reconnect/renumbering, Dock/recreation/
reboot, actual full-volume/power-loss, new hosted CI and macOS 26 remain unrun.
Service changes and publication still need their applicable explicit approval.
See [M2-D evidence](../validation.md#milestone-2-d-persistence-and-local-validation--2026-11-07).

## Native MRU and later features

Status: design proposals only; implementation is not authorized.

Native Space/window MRU is recorded with its safety and policy boundaries in
[architecture proposals](../architecture.md#follow-on-proposals). Native Space lifecycle,
reordering, GUI, and hotkeys remain deferred.
