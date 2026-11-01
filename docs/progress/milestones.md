# Milestones

Statuses: proposed, in progress, implementation complete, complete.
Validation follow-ups remain open until their results are recorded.

## M1 — Development environment and vspace smoke test

Status: implementation complete, 2026-11-01.

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

- [ ] Review the commits and obtain authorization before pushing.
- [ ] Run the GitHub Actions matrix on macOS 15 and 26 after publication.
- [ ] Verify window focusing and existing-Space switching in a suitable session.
- [ ] Validate runtime behavior on macOS 26.

## M2 — Development process records

Status: complete, 2026-11-01.

Scope: documentation and agent instructions.

- [x] Create a main progress document with linked supporting documents.
- [x] Record the existing milestone, decisions, results, and outstanding work.
- [x] Require agents to read and maintain the records through `AGENTS.md`.
- [x] Check document links and whitespace, then record the result in the journal.

## M3 — Conventional Commits and GitHub workflow

Status: complete, 2026-11-01.

Scope: agent instructions and development records.

- [x] Record the completed progress-document task in a focused local commit.
- [x] Require local Conventional Commits for completed, validated work.
- [x] Document focused staging, task branches, history preservation, and PR checks.
- [x] Check documentation links and whitespace and commit the policy update.

## Future milestones

Status: proposed; implementation is not authorized by this record.

After validation follow-ups, define a separate design milestone for daemon-owned
virtual workspace state and SIP-compatible window visibility/assignment. Resolve
its requirements and architecture before approving implementation. Native GUI
and hotkey-engine work need their own scoped milestones afterward.
