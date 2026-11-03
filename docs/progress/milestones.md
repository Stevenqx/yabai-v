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

Status: in progress, authorized 2026-11-03.
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
- [ ] Implement documented JSON queries and native focus with CLI failures.
- [ ] Test mapping, ordering, lookup, duplicate prevention, layouts, and bad input.
- [ ] Build locally, verify persistent signing, and run headless/regression checks.
- [ ] Run macOS 15/26 CI for the new implementation when publication is approved.
- [ ] Run an approved interactive focus smoke test and restore host state.
- [ ] Update technical docs, validation, decisions, journal, and resume instructions.
- [ ] Commit completed, validated units locally with Conventional Commits.

Gates: changing the running service and pushing this new branch require explicit
approval. Existing macOS 15/26 CI results cover Milestone 0 only. macOS 26
interactive runtime remains unrun without a GUI host/session.

## Milestone 2 and later

Status: proposed; implementation is not authorized.

Workspace creation/destruction, window assignment/movement, reordering,
persistence across restarts, GUI, and hotkeys are deferred. This task does not
begin any of them.
