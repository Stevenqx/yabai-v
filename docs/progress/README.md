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
The published checkpoint is `6359db7` on `origin/feat/vspace-smoke`, with
[passing macOS 15/26 CI](https://github.com/Stevenqx/yabai-v/actions/runs/41403274379).
The local foundation checkpoint is `93a59ed`; later documentation commits are
unpublished. macOS 15.6 window/Space focus passed visual confirmation, and the
original service was restored. See [validation](../validation.md).

Milestone 1 adds an independent daemon-owned manager and `vspace --list`,
`--current`, and `--focus N`, preserving `--ping` and existing domain semantics.
Only mapping and focusing are authorized; lifecycle, window movement, GUI,
hotkeys, and Milestone 2 remain deferred.

## Resuming work

Handoff/task start checked on 2026-11-03. Active branch: `feat/vspace-mapping`,
starting from `93a59ed`, with scope checkpoint `21a6e6f`. The manager foundation
is implemented and passes a universal build and 13 headless tests. CLI/startup
wiring remains next. This branch and its documentation are local; no Milestone 1
implementation has been published or CI-tested.

1. Inspect branch/tree and read milestones, decisions, journal, architecture,
   development guide, and latest validation. Recheck all historical host state.
2. Implement the authorized Milestone 1 scope in the milestone checklist; run
   focused tests, local build/signature verification, and regression checks.
3. Obtain approval for publishing the completed branch before new macOS 15/26
   CI and for changing the running service before interactive tests. Keep these
   tests unrun until evidence exists. Preserve and restore the installed service.
4. Update all records and create focused local commits. Do not begin Milestone 2.

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
