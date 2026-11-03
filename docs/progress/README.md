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

Milestone 1 implementation is complete: the independent manager and `vspace
--list`, `--current`, and `--focus N` preserve `--ping` and native semantics.
The signed universal build, 22 headless tests, static analysis, and local
regression checks pass. [Commands and JSON](../vspace.md) and
[detailed validation](../validation.md#milestone-1-commands-and-local-validation--2026-11-03)
document the session snapshot policy and remaining tests. New CI and interactive
validation are pending approval. Milestone 2 has not begun.

## Resuming work

Handoff checked on 2026-11-03. Active branch: `feat/vspace-mapping`, starting at
`93a59ed`, with scope checkpoint `21a6e6f` and manager checkpoint `36e9ac8`.
The subsequent command-integration commit contains the complete implementation,
22 tests, command docs, and this handoff. All Milestone 1 changes and the three
foundation documentation commits after published `6359db7` are local/unpublished.
Inspect `git log` for the exact final local checkpoint; no Milestone 1 SHA has
been tested by GitHub Actions yet.

1. Inspect branch/tree and read milestones, decisions, journal, architecture,
   development guide, and latest validation. Recheck all historical host state.
2. Review the completed diff. Obtain explicit approval to publish only this
   feature branch to `origin` and run the existing macOS 15/26 matrix. Record
   exact tested SHA/results; no merge or force-push is authorized.
3. Obtain approval to pause the original service for the prepared interactive
   test in the validation report. Use the already signed checkout binary and
   existing float development config, with only one managing daemon at a time.
   Verify list/current/occupancy, a safe virtual-focus round trip, unchanged
   frames/layout settings during focus, then restore and verify original service,
   visible Spaces, focus, and installation hashes after startup.
4. Local completed checks: universal `make` build with persistent signature,
   22/22 tests (five existing upstream deprecation warnings), no static analyzer
   findings, a passing read-only native adapter probe (occupied/spare Spaces),
   unchanged native handler/build files, original query regressions,
   original file hashes, shell syntax, Markdown links, and whitespace.
5. Remaining limits: CI/interactive Milestone 1 checks are unrun pending approval;
   macOS 26 GUI runtime still has no supplied host. Session mappings do not
   dynamically allocate or reassociate Spaces, and conservative occupancy can
   retain visually empty Spaces. Update records and commit test evidence locally.
   Do not begin Milestone 2, GUI, hotkeys, lifecycle, or window movement.

The read-only host baseline is `/private/tmp/yabai-v-m1-baseline.j8r6f3ko`.
It observed one original Homebrew daemon (PID 9357), native active Space 589 on
native display 2, and focus on Orca window 6369. These are historical identifiers,
not permission to change the host. Original binary/config/LaunchAgent hashes
still match Milestone 0. No new daemon or focus test has run in Milestone 1.

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
