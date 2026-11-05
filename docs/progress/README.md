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
are untouched by allocation. M2-C/D and native MRU remain design proposals; M2-B is described below.
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

## Resuming work

Handoff checked on 2026-11-05. Active branch: `feat/vspace-mapping`.
Production/local implementation checkpoint: `ca48ac8`; source/build artifacts are
unchanged during GUI validation. The following documentation checkpoint records
that validation; inspect `git log` for its exact ID. Cached published remote ref
`origin/feat/vspace-mapping` is `cf20e36` without network refresh, including M2-A
and the portable runner. **The M2-B implementation and GUI records remain local
and unpublished.**

1. Inspect branch/tree and read milestones, D009–D014, latest journal,
   development/architecture and the M2-B GUI report. Authorized M2-B implementation
   and available local GUI validation are complete. M2-C refresh, M2-D persistence,
   native lifecycle/reordering, MRU, GUI product and hotkeys remain proposals.
2. Source `ca48ac8` passed the persistent-signed universal build on macOS 15.6/SIP
   enabled, 43/43 production and 16/16 runner tests, strict signature, zero-finding
   arm64 analysis and static checks. No source changed for this documentation task,
   so no rebuild/retest is needed. Earlier M2-A/runner checks remain historical.
3. Full GUI evidence is `/private/tmp/yabai-v-m2b-gui.mhi73ua9`, with valid-focus
   regressions in `pqaohqqa`. The final independent audit in
   `mhi73ua9/independent-final-recovery-audit.json` compares against its pre-pause
   native bounds/owners/Space memberships and passes every condition, including
   cached frames, layouts/all 10-pixel padding/gaps, original file hashes and SIP.
   `user-visual-confirmation.json` records the user's positive confirmation.
4. The original Homebrew service is restored; no development daemon/fixture
   remains. Historical PID 12255, native IDs and host/session state require fresh
   checks before reuse. User-prepared native desktops remain present. The earlier
   `oxsvv6qz` cache-only mismatch and `chyw7zi8` harness assertion failure remain
   separate records; do not relabel either attempt as a clean pass. Their
   limitations and the accepted measured-baseline transition are in D014/report.
5. No further service operation is pending in this finished test session. Preserve
   the original installation/configuration; future sessions need their applicable
   explicit authorization. Capture native bounds as well as cached query frames
   before a pause. Do not try to restore user windows from unverified stale caches.
   Preserve original per-Space layouts/padding and audit physical frames/recovery.
6. The maintained runner's query mode has run against the actual M2-B daemon.
   `interactive --destroy-created` itself remains fixture-tested; the temporary
   GUI harness exercised the lifecycle commands. Use the standard runner headless
   by default, inspect skipped/unobserved results, and keep original-service
   recovery separate from a development-daemon baseline. One-display GUI and
   unsafe live fault injection remain unrun; safety/race cases have headless tests.
7. Ongoing validation focuses on macOS 15. macOS 26 build/runtime stays deferred
   under D012, with optional manual CI available. New hosted macOS 15 CI requires
   separately authorized publication of the exact reviewed SHA. No push or merge
   is authorized; historical M1 CI does not cover M2-A/runner/M2-B changes.

Temporary reports/fixture helpers remain outside Git and include private window
metadata. No installed binary/config/service definition, TCC/trust, scripting
addition or SIP setting changed. Native MRU remains a separate observed-focus
feature; allocation/release/index gaps do not manufacture history.

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
