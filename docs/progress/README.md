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

Local validation passed: persistent-signed universal build, 30/30 headless tests,
arm64 analysis, static checks, native read-only mapping probes on one/two displays,
and approved real daemon/IPC checks on SIP-enabled macOS 15.6. Creation preserved
IDs/indices/order/focus/frames/layouts and exhausted the focused display without
falling back to another display's spares. Native/virtual same-display and visible
cross-display focusing and already-focused errors passed. Empty-view float/BSP/
stack commands passed; the user confirmed visible switching and return.

Native cross-display hidden-target dispatch made the target visible without
keyboard focus in one observation. The subsequent virtual call focused the then-
visible target. Both returned native success; target visibility differed between
those controls, so no equal activation-timing or stronger SIP guarantee is claimed.
The final independent audit passed every recovery condition against the first
idle baseline, after restoring two surviving floating frames changed by original
service startup. The original service, desktop, focus, frames, layouts, hashes,
and SIP are verified. Locked/interrupted attempts remain separately recorded.

New hosted macOS 15/26 CI, macOS 26 GUI, a standalone single-display GUI repeat,
and populated layout smoke remain follow-ups. See
[final validation evidence](../validation.md#unlocked-repeats-and-final-recovery).

## Resuming work

Handoff checked on 2026-11-04. Active branch: `feat/vspace-mapping`.
Baseline/local remote-tracking checkpoint: `8c5d0b3` (no network refresh).
Source checkpoints: `86dabb5` (native focus) and `5b199fb` (allocation/tests/help/
design). `157bae2` records the earlier locked attempt. The following documentation
commit contains final unlocked results and this handoff; inspect `git log` for
its exact ID. **All M2-A code and new validation commits remain unpublished.**
No executable source changed after `5b199fb`.

1. Inspect branch/tree and read milestones, D009–D011, journal, architecture,
   development, and final validation. Authorized implementation and available
   local checks are complete; do not repeat completed checks without a new reason.
2. The user approved a sole signed-checkout daemon session with Homebrew pause
   and restoration, manually unlocked the desktop, supplied an idle period, and
   confirmed visible switching/return. Those sessions are finished. Future service
   work must remain within existing approval/safety scope; no competing daemon.
3. Final recovery evidence is
   `/private/tmp/yabai-v-m2a-gui.chsprfea/independent-final-recovery-audit.json`,
   comparing against `iaus8mt4/immediate-original.json`. Every condition passed:
   one original Homebrew daemon (PID 42612), original two-display UUIDs/topology,
   native order 3/7/592 and 733, visible 3/733, active 3, Orca window 6369 focused,
   all original frames/associations, stack layouts, installation hashes and SIP.
   These are historical identifiers; recheck before any use. Two floating frames
   changed during original-service startup and were restored with native commands.
4. Successful comparison/allocation evidence is `iaus8mt4`; completion/layout and
   final audit evidence is `chsprfea`, all under `/private/tmp/yabai-v-m2a-gui.*`.
   `iaus8mt4` retains its native-activation assertion and first frame-recovery
   failure; use the final independent audit for recovery, not that helper exit.
   `chsprfea/results.json` has no test/recovery errors and its visual-confirmation
   record contains the user's explicit reply. Read the focus observation limits.
5. Preserve earlier `f2_iljuh` (locked) and `g55ljkm7` (window/focus activity)
   attempts as inconclusive. External topology/window changes prevent restoring
   the old locked-session desktop as though it were the fresh idle baseline.
   The temporary helper now records console lock-key absence after unlock and
   separates native dispatch result from global-focus observation.
6. Publication/merge is not authorized. Review local commits and obtain separate
   authorization before pushing to `origin`; then run the existing macOS 15/26
   matrix for the exact published SHA. Historical M1 CI does not cover M2-A.
   macOS 26 GUI needs a supplied session. Single-display GUI and populated layout
   checks remain explicit limits; headless/native-probe evidence is separate.
7. M2-B/C/D and native MRU remain design proposals. Do not implement native
   lifecycle, reordering, GUI, hotkeys, or later milestones without authorization.

Temporary logs/helpers remain outside Git. Detailed durable evidence, failed/
skipped conditions, command semantics, and follow-on proposals are in the linked
documents. The original installation and configuration files were not overwritten;
no TCC reset, trust change, scripting-addition load, or SIP change occurred.

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
