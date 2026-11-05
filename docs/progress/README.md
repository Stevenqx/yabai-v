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

M2-C implementation and available local checks are complete. Explicit
`vspace --refresh` releases only revalidated empty hidden valid mappings and
compacts survivors in the existing global order, including interleaved displays.
It protects global focus/every display's visible Space, retains uncertain
mappings with reasons, rejects pending destroy, and publishes one complete
candidate without native mutation or recency changes. The signed universal build,
52 production tests, 19 runner tests, zero-finding arm64 analysis, native read-only
two-display probe and existing-service read-only queries passed. Real M2-C
daemon/GUI tests need a newly approved service session; publication and macOS 26
remain gated/deferred. See [M2-C validation](../validation.md#milestone-2-c-refresh-and-local-validation--2026-11-05).

## Resuming work

Handoff checked on 2026-11-05. Active branch: `feat/vspace-mapping`.
M2-C starts from clean `e23bd17` (M2-B implementation `ca48ac8` plus GUI records).
The local feature commit containing this handoff is the M2-C implementation
checkpoint; verify its exact SHA with `git log`. Cached published remote ref
`origin/feat/vspace-mapping` is `cf20e36` without network refresh, including M2-A
and the portable runner. **M2-B, its GUI records, and M2-C remain local/unpublished.**

1. Inspect branch/tree, milestones, D009–D015, latest journal, development/
   architecture and the M2-C report. Authorized M2-C implementation/local checks
   are complete. M2-D persistence, native lifecycle/reordering, broader topology
   reconciliation, MRU, GUI product and hotkeys remain unauthorized proposals.
2. M2-C source passed `./scripts/test-vspace.py headless` on macOS 15.6/SIP enabled:
   persistent-signed universal build, strict signature, 52/52 production and 19/19
   runner tests, help/version/syntax/whitespace. Evidence:
   `/private/tmp/yabai-v-m2c-validated/report.json`; arm64 analysis has zero findings
   in `/private/tmp/yabai-v-m2c-analysis-final.plist`. Apple's Python 3.9.6 also
   passed all 19 runner tests. Markdown links/anchors passed across nine files.
3. The read-only two-display probe released two hidden empty mappings from
   isolated hypothetical state, retained uncertain/visible mappings and passed
   idempotence with native topology/visibility/global focus unchanged. Evidence:
   `/private/tmp/yabai-v-m2c-native-probe.jsonl` and source/build siblings. It started
   no daemon, tracked no AX windows and guarded native mutation callbacks. Original
   native display/Space/window queries also passed. These do not establish M2-C
   daemon/IPC, real window classifications, frames/layouts or interactive behavior.
4. The original Homebrew daemon remains the sole daemon (`/opt/homebrew/bin/yabai`,
   historical PID 12255); no service pause was attempted. Recheck process/console/
   topology/window state before any new session. A real M2-C GUI test requires
   explicit approval to pause the original service, run the signed checkout alone,
   and restore/audit the original session. Prior completed M2-B approval is not a
   new session's permission. Capture independent native bounds/owners/memberships
   alongside cached frames per D014, preserve layouts/padding and original files.
5. Next concrete validation step, after approval: prepare an idle measured baseline,
   test refresh release/compaction/protection/idempotence on existing desktops,
   verify fresh-ID create/compact-index focus and native command regressions, then
   independently audit original-service recovery. The standard runner has explicit
   `interactive --refresh-mappings`; it can release preexisting eligible mappings.
   Its refresh stage and `--destroy-created` orchestration remain fixture-tested.
   Query/default interactive modes do not trigger refresh.
6. Historical M2-B GUI evidence remains `/private/tmp/yabai-v-m2b-gui.mhi73ua9`
   and `pqaohqqa`, with positive user confirmation and independent native recovery.
   Earlier inconclusive cache-only/harness-error attempts remain distinct. Do not
   apply those results to M2-C source. Physical single-display M2-C GUI and unsafe
   live fault injection remain unrun; policy/race cases have headless coverage.
7. Ongoing validation focuses on macOS 15. macOS 26 build/runtime stays deferred
   under D012, with optional manual CI available. New hosted macOS 15 CI requires
   separately authorized publication of the exact reviewed SHA. No push or merge
   is authorized; historical M1 CI does not cover M2-A/runner/M2-B/M2-C changes.

Temporary reports/fixture helpers remain outside Git and include private window
metadata. No installed binary/config/service definition, TCC/trust, scripting
addition or SIP setting changed. Native MRU remains a separate observed-focus
feature; allocation/release/compaction do not manufacture history.

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
