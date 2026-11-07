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
are untouched by allocation. M2-D is implemented locally as described below; native MRU remains a separate
proposal. M2-B and M2-C are described below.
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

At the M2-A handoff, new hosted CI, a standalone single-display GUI repeat
and populated layout smoke remained follow-ups. Populated layout checks have
since passed under M2-B below. See
[final validation evidence](../validation.md#unlocked-repeats-and-final-recovery).

The user subsequently requested a maintained, portable vspace test script and
deferred macOS 26 validation in favor of macOS 15. The new
[standard runner](../development.md#standard-vspace-test-script) provides automatic,
read-only daemon, and interactive checks with private JSON reports. macOS 15 is
the default CI target; macOS 26 is optional for a later manual workflow run.
An unrun OS is not a compatibility pass. See D012 and the latest validation entry.

M2-B implementation and available macOS 15.6 two-display GUI checks are complete
locally. `--destroy SOURCE [--to TARGET]` evacuates through existing native paths,
then releases only after observed emptiness/visibility/focus. Preflight/partial
failures retain the source and survivor IDs/indices/order remain fixed. The signed
universal build, 43 production tests, 16 runner tests and zero-finding arm64
analysis passed. Approved GUI checks now also passed empty release/reuse, mixed
tiled/floating/minimized windows into BSP/stack/float destinations, hidden
applications, other-display visibility/global-focus recovery, explicit cross-display
evacuation and existing focus round trips. The user confirmed visible switching
and return. See [M2-B GUI evidence](../validation.md#milestone-2-b-approved-gui-validation--2026-11-05).

An initial cached-frame mismatch is retained as inconclusive: it had no independent
pre-pause native bounds. Read-only padding diagnosis and the user's explicit
measured-baseline choice led to an independent WindowServer audit. A second
attempt stopped at an incorrect harness assertion (BSP stack indices); native
evacuation had passed. The corrected complete run and focused follow-up recovered
every original-session condition against their measured native baseline. No
production source change was needed during validation.

M2-C implementation and available macOS 15.6 two-display GUI checks are complete.
Explicit `vspace --refresh` releases only revalidated empty hidden valid mappings
and compacts the existing global order. It protects global focus/every display's
visible Space, retains uncertainty with reasons, rejects pending destroy and
publishes one complete candidate without native mutation or recency changes.
The signed universal build and zero-finding arm64 analysis passed; the current
headless suite passed 52 production and 21 runner tests. The live runner exposed
a native-query preflight mismatch (`disabled` versus the `off` setting), corrected
in the follow-up without changing the daemon source/binary at `31b3035`.

Live checks passed destroy-gap compaction, hidden-empty release/idempotence,
background-visible protection, uncertain retention, fresh-ID create/focus and
isolated floating/minimized/application-hidden occupancy. The user requested a
watched focus repeat and confirmed switching/return looked correct. A native
secondary-display request made the target visible without global focus; a
separate retry focused it. That documented native SIP limit remains unchanged.
An independent audit against the first pre-pause native baseline passed every
original service/desktop/window/layout/hash/SIP condition. No development daemon
or fixture remains. See [M2-C GUI evidence](../validation.md#milestone-2-c-approved-gui-validation--2026-11-05).

M2-D persistence is implemented locally from `dbe2cde`. Logical IDs/global order,
destroy gaps and the allocator high-water mark survive durable restarts. Startup
uses unique current Space/display UUIDs with native ownership/type checks; uncertain
records remain unresolved and own no native mapping. Empty visible desktops on
every display are represented. Create/refresh/final destroy release atomically save
complete candidates; post-replacement sync failure publishes committed metadata
with an uncertainty diagnostic. Invalid state is preserved, and future-schema/
read/lock/preservation/startup-save failures use explicit session-only fallback.
See [state and recovery](../development.md#virtual-workspace-state-and-recovery).

Available checks passed: persistent-signed universal build, 68 production and
24 runner tests, zero-finding arm64 analysis, Python 3.15/Apple 3.9.6 runner checks,
read-only native UUID availability and original-service native CLI queries on
SIP-enabled macOS 15.6. Real M2-D daemon/restart/GUI, hardware reconnect/Dock/reboot,
new hosted CI and macOS 26 remain unrun. No service or publication action occurred.

## Resuming work

Handoff checked on 2026-11-07. Active branch: `feat/vspace-mapping`.
Starting local checkpoint: `dbe2cde` (completed M2-C GUI/runner follow-up).
The local M2-D feature commit containing this handoff is the new implementation
checkpoint; obtain its exact ID with `git log -1`. Cached published remote ref
`origin/feat/vspace-mapping` remains `cf20e36`, without network refresh.
**M2-B/C/D and their follow-up records remain local/unpublished.**

1. Inspect branch/tree, milestones, D009–D016 and the latest journal. M2-D's
   authorized implementation and available local checks are complete. MRU,
   native lifecycle/reordering, automatic event-driven reconciliation, GUI and
   hotkeys remain proposals. No push, merge or new service pause is authorized.
2. Full signed build/headless report: `/private/tmp/yabai-v-m2d-final/report.json`,
   68 production/24 runner tests. The post-review runner-only correction uses
   the same signed binary with `--skip-build` in
   `/private/tmp/yabai-v-m2d-final-checked/report.json`. That report explicitly
   skips rebuilding; no daemon source changed after the full signed build.
   Strict signature/universal checks, both Python versions, shell syntax,
   Markdown links/anchors, staged whitespace and zero-finding arm64 analysis
   passed. Analysis: `/private/tmp/yabai-v-m2d-analysis-final.plist`.
3. Native identity probe evidence is
   `/private/tmp/yabai-v-m2d-identity-probe-native.json`: one physical display,
   two ordinary desktops, valid unique Space UUIDs and their display UUID.
   No daemon/window manager/persistence/focus/move path ran. Original-service
   native queries also passed (`/private/tmp/yabai-v-m2d-original-*.json`), with
   one observed yabai process. These are read-only field/CLI checks, not a
   M2-D runtime/restart/reboot pass. Host IDs/topology require fresh checks.
4. Persistence fixtures cover private real files, snapshot/restart order/gaps/IDs,
   synthetic identity/disconnect/reconnect/renumbering, malformed/oversized/future
   recovery, competing writer ownership, allocation/permission/ENOSPC faults,
   pre/post replacement failures, forked crash boundaries/abandoned temps and
   partial native-fixture evacuation/retry. They neither manipulate physical
   displays nor establish native UUID continuity across reboot/Dock/recreation.
5. Next concrete validation: prepare an isolated XDG state directory and a measured
   sole-daemon M2-D restart session. Under a new explicit service approval, capture
   independent native bounds/owners/memberships before pausing the original,
   test hidden-empty persistence, destroy gaps, refresh/restart and fresh IDs,
   then restore/audit the original service/session per D014. Keep development
   state separate from the default state file. Never run two window managers.
6. Physical display reconnect/renumbering, Dock/native recreation, reboot,
   actual full-volume/power-loss and M2-D GUI remain unrun. macOS 26 stays
   deferred under D012. Hosted macOS 15 CI requires separately authorized
   publication of the exact reviewed SHA. The earlier M2-C GUI/recovery evidence
   at source `31b3035` remains historical; see the validation report, not old
   PIDs/window IDs, for that session's results. No recovery action remains pending.

Private reports/fixture helpers remain outside Git. No installed binary/config/
service definition, TCC/Keychain trust, scripting addition or SIP setting changed.
State recovery requires an approved owner stop; do not delete a live `state.lock`.
Native UUIDs are conditional evidence, never a positional or cross-reboot promise.
Restoration/compaction do not manufacture MRU history; window-handle persistence
remains deferred until the independent MRU feature defines lifetime policy.

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
