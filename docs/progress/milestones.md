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
Future publication/service changes remain subject to `AGENTS.md`; no merge or
Milestone 2 implementation is authorized.

## Milestone 2 and later

Status: proposed; implementation is not authorized.

Workspace creation/destruction, window assignment/movement, reordering,
persistence across restarts, GUI, and hotkeys are deferred. This task does not
begin any of them.
