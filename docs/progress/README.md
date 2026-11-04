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
Milestone 2-A is authorized and in progress: spare mapping creation and native
focus result parity. Later milestone implementations remain proposals.

## Resuming work

Active task checked on 2026-11-04: M2-A, branch `feat/vspace-mapping`, clean
starting checkpoint `8c5d0b3`. This descends from published M1 source `6c4a354`;
config/validation follow-ups `b195681` and `8c5d0b3` are present. No push is
authorized in this task; inspect remote refs before assuming publication state.

Read milestones, decisions, journal, architecture, development, and validation.
M2-A removes virtual-focus polling and adds focused-display spare allocation.
Record M2-B/C/D and native MRU design only. Retain session-local mapping state,
all existing native commands/layouts, and the single-daemon safety boundary.

The M1 macOS 15/26 CI and macOS 15.6 GUI results remain historical evidence for
M1, not validation of new M2-A code. New build/tests/signature and GUI results
will be recorded in the latest validation entry. An existing service must be
rechecked; do not stop or replace it without explicit approval, start a competing
daemon, or reuse historical PIDs/Space/window IDs. New CI requires publication
authorization; macOS 26 GUI still requires a supplied session.

Next: finish M2-A implementation and conservative regression coverage, run local
checks, record concrete runtime gates, then commit locally. Do not implement
follow-on proposals, publish, or merge without authorization.

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
