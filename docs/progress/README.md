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

Milestone 1 is complete for its authorized scope. Its published source `6c4a354`
passed the macOS 15/26 matrix (22 tests per host) and an approved macOS 15.6 focus
round trip with user visual confirmation and audited recovery. That is historical
M1 evidence; see [validation](../validation.md#milestone-1-publication-and-interactive-validation--2026-11-04).

M2-A implementation is complete locally: `vspace --create` allocates the first
revalidated hidden empty native desktop on the focused display; `--focus` returns
native results without extra polling. Existing IDs/indices/order survive creation.
Only explicit mapping creation is new; focus, windows, layouts, and native order
are untouched by allocation. M2-B/C/D and native MRU are design proposals only.
[Commands](../vspace.md) and [architecture](../architecture.md) describe the contract.

Final local validation passed: persistent-signed universal build, 30/30 headless
tests, arm64 analysis, shell/Markdown/whitespace checks, and native read-only
mapping probes on one/two displays. The user subsequently approved live testing.
A real daemon/IPC session passed focused-display creation, exhaustion without
falling back to another display's spare, CLI errors/queries, and preservation of
existing mappings, focus/visibility, frames, layouts, and native order.

The unchanged native focus control returned success without observed activation.
Read-only inspection confirmed the screen was locked, making the GUI attempt
inconclusive for focus. The development daemon stopped and the original service
restarted. An independent audit passed service/Space/frame/layout/hash/SIP checks;
window-focus recovery remains unverified until manual unlock. The remaining
live focus/layout/other-display checks and new hosted CI are open. macOS 26 GUI
has no supplied session. See [the latest evidence](../validation.md#milestone-2-a-approved-gui-validation--2026-11-04).

## Resuming work

Handoff checked on 2026-11-04. Active branch: `feat/vspace-mapping`.
Baseline and local remote-tracking checkpoint: `8c5d0b3` (no fetch this task).
M2-A source checkpoints: `86dabb5` (native focus results) and `5b199fb` (allocation,
coverage, help, and future design). The next documentation commit records the
approved live attempt and contains this handoff; inspect `git log` for its ID.
**All M2-A source and new validation records remain local/unpublished.**

1. Inspect branch/tree and read milestones, D009–D011, journal, architecture,
   development, and the latest validation. Code and local build/headless checks
   are complete; no source changed after `5b199fb`.
2. The user has explicitly approved pausing Homebrew, running only the signed
   checkout daemon, and restoring/auditing the original service and desktop.
   That authorization persists for finishing this session; do not request it
   again. Publication, merge, and later implementation are not authorized.
3. **Manual unlock is pending.** Do not attempt to unlock or reset TCC. Recheck
   `IOConsoleUsers`/`CGSSessionScreenIsLocked` before another service pause, and
   wait for an unlocked desktop. The helper now has this guard. A native success
   exit while locked supplies no visible-focus evidence.
4. Recovery audit `/private/tmp/yabai-v-m2a-gui.f2_iljuh/independent-recovery-audit.json`
   observed one original `/opt/homebrew/bin/yabai` daemon (PID 98745), visible Spaces
   3/674, active Space 674, original frames/layouts/order/hashes, and SIP enabled.
   Native window query reported focus 0 while locked; captured original focus was
   Zotero window 5943. These identifiers are historical. Recheck after unlock,
   restore and verify original focus, and do not claim full recovery beforehand.
5. Continue the approved tests against fresh two-display state, using
   `/private/tmp/yabai-v-m2a-gui-smoke.py` only after reviewing it. It dynamically
   captures a new recovery baseline, tests creation/pool exhaustion, native and
   virtual focus parity (same/cross display and already-focused), and layout modes
   on empty workspaces. Never start it beside the original daemon. Preserve the
   failed locked attempt separately from any clean repeat. Use an independently
   verified recovery audit, not just the helper's exit code.
6. New macOS 15/26 CI requires a separately authorized push of reviewed commits;
   old M1 CI does not cover M2-A. macOS 26 GUI needs a supplied session. M2-B/C/D,
   native MRU, native lifecycle, reordering, GUI, and hotkeys remain deferred.

Current live evidence: `/private/tmp/yabai-v-m2a-gui.f2_iljuh` (attempt and recovery).
Preflight-only signature-output assertion: `/private/tmp/yabai-v-m2a-gui.kpx8ldjh`;
it made no service changes and the corrected combined-output check passed.
Earlier build/headless/probe logs use `/private/tmp/yabai-v-m2a-*` and remain
historical. Temporary evidence is not committed; the detailed report is durable.

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
