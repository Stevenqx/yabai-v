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

New hosted CI, a standalone single-display GUI repeat,
and populated layout smoke remain follow-ups. See
[final validation evidence](../validation.md#unlocked-repeats-and-final-recovery).

The user subsequently requested a maintained, portable vspace test script and
deferred macOS 26 validation in favor of macOS 15. The new
[standard runner](../development.md#standard-vspace-test-script) provides automatic,
read-only daemon, and interactive checks with private JSON reports. macOS 15 is
the default CI target; macOS 26 is optional for a later manual workflow run.
An unrun OS is not a compatibility pass. See D012 and the latest validation entry.

M2-B is implemented locally under the user's 2026-11-05 prompt: explicit
`--destroy SOURCE [--to TARGET]` evacuates via existing native paths, then releases
only after observed emptiness/visibility/focus. Preflight and partial failures
retain the source; survivor IDs/indices/order remain fixed. The signed universal
build, 43 production tests, 16 runner tests, zero-finding arm64 analysis and a
read-only native uncertain-occupancy rejection passed. Native release/reuse lacked
an eligible spare; actual GUI evacuation is unrun. See
[M2-B evidence and limits](../validation.md#milestone-2-b-evacuation-and-release--2026-11-05).

## Resuming work

Handoff checked on 2026-11-05. Active branch: `feat/vspace-mapping`.
M2-B starts from clean local checkpoint `cf20e36` (portable runner), following
M2-A sources `86dabb5` / `5b199fb` and GUI records `157bae2` / `4f637d7`.
The coherent M2-B implementation/test/documentation checkpoint follows `cf20e36`;
inspect `git log` for its exact ID. Observed remote-tracking checkpoint is
`cf20e36`, with a local `update by push` reflog entry, without network refresh.
This supersedes the older resume record that called M2-A/runner unpublished.
**Only the new M2-B implementation/documentation work is unpublished.**

1. Inspect branch/tree and read milestones, D009–D013, the latest journal,
   architecture, development and M2-B validation. M2-B implementation is authorized
   and locally validated; interactive evacuation remains a follow-up. Refresh,
   persistence, native lifecycle/reordering, MRU, GUI and hotkeys remain proposals.
2. The production source passed a persistent-signed universal build on macOS
   15.6/SIP enabled, 43/43 production and 16/16 runner tests, strict signature,
   zero-finding arm64 analysis, shell/Markdown/whitespace checks. The final full run
   rebuilt/signed the source after AX identity hardening. See the detailed report for exact
   temporary evidence paths and explicitly skipped/unrun scenarios.
3. A read-only native probe rejected uncertain occupancy before mutation. Its
   release/reuse path was skipped: no eligible focused-display spare existed.
   This was not a real daemon/AX move test. M2-B native evacuation, populated
   float/BSP/stack, minimized/hidden state and physical display visibility/global
   focus preservation remain unrun. Do not substitute historical M2-A GUI evidence.
4. The original Homebrew service was observed running and left untouched; no
   development daemon was started. Earlier M2-A service-pause approvals covered
   finished sessions. Prepare disposable existing native desktops/windows, obtain
   explicit approval for another pause, and run only the signed checkout as sole
   daemon. Recheck all host state; historical PID 42612 and native IDs are not
   authorization or current observations for a later session. Restore and audit
   original service/desktop/configuration/binary hashes after any approved test.
5. Use `scripts/test-vspace.py` headless by default. Read-only query mode now
   expects M2-B. Opt-in `interactive --destroy-created` releases this run's
   unchanged hidden empty allocations with local mapped targets. It does not
   populate/evacuate existing user workspaces; use the documented manual procedure
   for mixed/minimized/hidden and populated-layout cases. Inspect skipped checks.
   The runner never manages services; it audits the development-daemon baseline,
   so original-service recovery remains a separate manual procedure.
6. Focus ongoing validation on macOS 15. macOS 26 build/runtime is deferred under
   D012; optional manual CI remains available. New hosted macOS 15 CI awaits
   separately authorized publication of the exact reviewed SHA. No push or merge
   is authorized, and historical M1 CI does not cover M2-A, the runner or M2-B.

Temporary logs/helpers remain outside Git. The local implementation/documentation
commit is not published. No installed service/configuration/binary, TCC/trust,
scripting-addition or SIP setting changed. Native MRU remains a separate future
observed-focus feature; allocation/release/index gaps create no history.

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
