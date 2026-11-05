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
are untouched by allocation. M2-D and native MRU remain design proposals;
M2-B and M2-C are described below.
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

## Resuming work

Handoff checked on 2026-11-05. Active branch: `feat/vspace-mapping`.
Daemon implementation/source checkpoint: `31b3035` (unchanged during GUI work).
The local follow-up commit containing this handoff records the runner preflight
correction and GUI results; inspect `git log` for its exact ID. Cached published
remote ref `origin/feat/vspace-mapping` remains `cf20e36`, without network refresh.
**M2-B, its GUI records, M2-C and this follow-up remain local/unpublished.**

1. Inspect branch/tree, milestones, D009–D015, latest journal, development/
   architecture and the M2-C GUI report. Authorized M2-C implementation and available
   local GUI validation are complete. M2-D persistence, native lifecycle/reordering,
   broader topology reconciliation, MRU, GUI product and hotkeys remain proposals.
2. Source `31b3035` passed the full persistently signed universal build, strict
   signature and zero-finding arm64 analysis on macOS 15.6/SIP enabled. Original
   build report: `/private/tmp/yabai-v-m2c-validated/report.json`. No daemon source
   changed in the GUI follow-up. With the corrected Python preflight, all 21 runner
   tests passed on Python 3.15 and Apple's 3.9.6; `--skip-build` passed the existing
   signature/binary, 52 production and 21 runner tests, syntax and whitespace:
   `/private/tmp/yabai-v-m2c-gui-runner-fix-headless/report.json`. Markdown links/
   anchors and whitespace were checked before the follow-up commit.
3. Core real daemon/IPC evidence is `/private/tmp/yabai-v-m2c-gui.jungx6qq`, repeated
   in `9otq8z8c`; both later stopped at fixture setup assertions, so do not relabel
   their entire runs as passes. The focused occupancy completion `5tf4x6i3` passed
   all isolated window cases and final empty release/idempotence without errors.
   `xxa84kya` found the runner preflight defect before refresh. Every attempt's
   original-service recovery passed. The detailed report distinguishes defects
   in runner/setup assertions from correct conservative daemon behavior.
4. The user asked to watch a repeat. `bga1388z` passed the primary-display round
   trip, then stopped on unobserved global secondary activation. `r63fdz3z` completed
   the secondary round trip after logging one visibility-only native dispatch and
   a separate retry. The user confirmed switching and return looked correct;
   `r63fdz3z/user-visual-confirmation.json` records that feedback. The maintained
   runner's earlier automated visual response remains `unobserved`.
5. The final independent audit is
   `/private/tmp/yabai-v-m2c-gui.r63fdz3z/independent-final-recovery-audit.json`,
   against the very first `xxa84kya` pre-pause native bounds/owners/memberships.
   Every condition passed: sole original executable/service, exact topology/
   visibility/global Space/window focus, all original window lifetimes/states/
   associations/native and cached frames, original stack layouts/all 10-pixel
   padding/gaps, original installed binary/config/plist hashes, unchanged checkout
   binary, no fixture and SIP enabled. The original Homebrew service is running;
   historical PID 48377 and all host/session IDs require fresh checks before reuse.
6. No further service operation is pending in this completed approved session.
   Preserve the original installation/configuration. Future service sessions need
   their applicable explicit authorization and independent native-frame baselines
   per D014. The maintained runner's refresh stage has now run live; its
   `--destroy-created` orchestration remains fixture-tested. Ordinary query/default
   interactive modes never refresh mappings. Use current indices after refresh.
7. Remaining coverage: simultaneously empty visible mappings on both physical
   displays, interleaved occupied survivors, last mapping and unsafe pending/
   query/topology/identity/arrival injections remain headless-only; physical
   single-display M2-C GUI remains unrun. Ongoing validation focuses on macOS 15.
   macOS 26 stays deferred under D012. New hosted macOS 15 CI needs separately
   authorized publication of the exact reviewed SHA; no push or merge is authorized.

Temporary reports/fixture helpers stay outside Git and include private window
metadata. No original binary/config/service definition, TCC/trust, scripting
addition or SIP setting changed. Native focus dispatch is not guaranteed observed
activation; the GUI repeat retains that limitation. Native MRU remains a separate
observed-focus feature; allocation/release/compaction do not manufacture history.

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
