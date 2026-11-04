# Development progress

This is the main development record for `Stevenqx/yabai-v`. Start here before
working on the fork, and keep this overview aligned with the supporting documents.

## Current position

Milestone 0 contains all prior setup, ping, workflow, progress-record, CI, and
GUI-validation work. The user authorized Milestone 1 (mapping and focusing) on
2026-11-03 and revised the earlier implementation gate. macOS 26 interactive
runtime remains a limitation; it does not block this authorized implementation.
Historical journal entries retain their original milestone labels; those labels
all refer to work now grouped under Milestone 0.

Milestone 0's upstream base is `dd845723416f5fe92af49fad5ebab00369e07edd`.
Its earlier published checkpoint is `6359db7` on `origin/feat/vspace-smoke`, with
[passing foundation CI](https://github.com/Stevenqx/yabai-v/actions/runs/41403274379).
Foundation records through `93a59ed` are now also published as ancestry of the
Milestone 1 branch; the earlier smoke-test branch was not rewritten.

Milestone 1 is complete for the authorized mapping/focusing scope. The independent
manager and `vspace --list`, `--current`, and `--focus N` preserve ping and native
semantics. Published source `6c4a354` passed the macOS 15/26 matrix (22 tests per
host) and a macOS 15.6 virtual-focus round trip with user visual confirmation.
The original service, desktop, window frames, and installation hashes were
restored/verified; SIP stayed enabled. The first interrupted attempt and recovery
helper assertions are retained separately in the
[latest validation report](../validation.md#milestone-1-publication-and-interactive-validation--2026-11-04).
The corrected local development config uses float `0.0` for animation duration.

[Commands and JSON](../vspace.md) document startup/session policy and errors.
macOS 26 interactive runtime remains unverified without a supplied GUI host.
Milestone 2 has not begun.

## Resuming work

Handoff checked on 2026-11-04. Active branch: `feat/vspace-mapping`.
Published source checkpoint: `6c4a354efb3a75c25906d48a49ad55c224fe175a`, with
[passing CI run 41779098539](https://github.com/Stevenqx/yabai-v/actions/runs/41779098539).
Scope/manager checkpoints are `21a6e6f` and `36e9ac8`.
Local follow-ups are config fix `b195681` and the subsequent validation-record
commit containing this handoff; these follow-ups are **not published**. Inspect
`git log` for the exact final local record checkpoint. No C source changed after
the CI-tested commit, and the local config fix passed the live repeat.

1. Inspect branch/tree and read milestones, decisions, journal, architecture,
   development guide, and latest validation. Recheck all historical host state.
2. Milestone 1's requested build/headless/CI and local interactive checks are
   complete. Review the local config/documentation follow-ups; publish them only
   if authorized. No merge, force-push, or new milestone is authorized.
3. If a macOS 26 GUI host is supplied, plan its runtime validation using the
   existing signing, single-daemon, observation, and recovery procedures.
   Keep that check unrun until real evidence exists. Future service changes
   remain subject to the host-safety approval requirement.
4. Completed evidence: signed universal local build; 22/22 tests, static analysis,
   native-query probe and CLI regressions; universal build/signature/22 tests on
   both hosted macOS versions; empty-active initialization and virtual focusing
   on an inactive native Space on macOS 15.6 with visual confirmation and unchanged
   frames/layouts. Final recovery audit independently passed all conditions.
5. Session-only mapping, conservative unknown occupancy, external native
   Space/display changes, and macOS 26 GUI runtime retain their documented
   limits. Lifecycle, persistence, movement, reordering, GUI, and hotkeys remain
   deferred. Do not begin Milestone 2 without explicit authorization.

The successful repeat evidence is `/private/tmp/yabai-v-m1-repeat.431h1ip7`;
use its passed `final-recovery-audit.json` together with the retained helper
assertion record and focus snapshots. The first/interrupted attempt and native
control are `/private/tmp/yabai-v-m1-approved.shfxktq3`. Final audit observed one
original Homebrew daemon (PID 13873), native visible/active Space 3 on display 1,
and Orca window 6369 focused. Original window frames and installation hashes
matched the immediate baseline. These are historical identifiers, not reusable
host state or blanket authorization for future changes.

## Supporting documents

| Document | Purpose |
| --- | --- |
| [Milestones](milestones.md) | Completed work, active scope, and remaining follow-ups |
| [Decisions](decisions.md) | Accepted decisions, their rationale, and future proposals |
| [Development journal](journal.md) | Dated task history, validation outcomes, and handoffs |

Technical references: [development workflow](../development.md),
[architecture boundaries](../architecture.md), [validation report](../validation.md),
and [agent conventions](../../AGENTS.md).

## Maintaining the record

At task start, read this overview and the milestones, then relevant decisions,
journal entries, and technical references. Identify the authorized scope and
record an active milestone task before implementation changes.

As work progresses, update milestone status and record significant decisions.
Before handing off, refresh this overview and append a journal entry with the
date, changes, checks and results, unresolved issues, and next step. Include
commit references when available. Preserve previous entries; explain corrections
or superseded decisions in a later entry.

Refresh `Resuming work` at every handoff with branch/checkpoint publication
status, validation, blockers, next steps, and authorization or validation gates,
as required by [AGENTS.md](../../AGENTS.md#development-records).

Link to detailed technical documentation and validation evidence. Keep unrun or
skipped checks explicit, and treat future plans as proposals rather than approval
to implement them. Do not store credentials or signing keys in these documents.
