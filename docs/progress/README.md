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
tests, arm64 static analysis, shell/Markdown/whitespace checks, original daemon
queries/hashes, and native read-only mapping probes. The probes covered one
display and then fresh two-display topology, including focused-display exhaustion
with another display's spare still available. They did not start a daemon or
validate GUI focus. New live daemon/GUI and hosted matrix checks remain open;
macOS 26 GUI has no supplied session. See the
[latest report and concrete plan](../validation.md#milestone-2-a-allocation-and-local-validation--2026-11-04).

## Resuming work

Handoff checked on 2026-11-04. Active branch: `feat/vspace-mapping`.
M1 baseline `8c5d0b3ec02b6fdf78ee48aa0fcd785c09b6e3a9` is also the current local
`origin/feat/vspace-mapping` tracking checkpoint (no network refresh this task).
Earlier notes calling its config/validation follow-ups unpublished are historical.
New local focus checkpoint: `86dabb52075ded8ebd794c28b6808478346ccaf5`.
The following allocation/tests/design-record commit contains this handoff; inspect
`git log` for its exact ID. **Neither M2-A commit is published.** No merge,
rebase, service change, installed-file replacement, or later implementation ran.

1. Inspect branch/tree/checkpoints and read milestones, decisions D009–D011,
   journal, architecture, development, and latest validation before continuing.
2. Code/local validation is complete. Do not redo implementation or infer GUI
   success from headless/native-probe results. Native handlers/layouts, build
   system, and macOS 15/26 workflow remain unchanged.
3. Obtain explicit approval before pausing the existing Homebrew service for the
   recorded signed-checkout daemon test. Recheck current topology, processes,
   mappings, visible Spaces, focused window, and hashes; never reuse recorded
   PIDs/IDs as live state or start a competing managing daemon. Recovery must
   restore/audit the original service, desktop, focus, frames, layouts, and files.
4. The host changed from one to two displays during read-only checks; the latest
   probe saw display 2 focused, native orders 3/7 and 666/592, with original daemon
   PID 13873. These are historical only. No focus/service command caused a change
   during this task, and old frame/topology snapshots are not a recovery baseline.
5. Publication requires separate authorization. Push reviewed commits only when
   approved, then run the existing macOS 15/26 matrix for the exact new SHA.
   M1 CI run 41779098539 does not cover M2-A. macOS 26 interactive compatibility
   still requires a supplied GUI host/session.
6. Destroy, refresh, persistence, and MRU implementation remain proposals;
   native Space lifecycle, explicit reordering, GUI, and hotkeys are deferred.
   Keep SIP enabled and follow existing manual Accessibility/Keychain rules.

Evidence: `/private/tmp/yabai-v-m2a-{signed-build-final,tests-final,analysis-final}.log`
and `/private/tmp/yabai-v-m2a-native-probe.{m,jsonl,log}`. Temporary evidence is not
committed. Current tests, docs, and the detailed validation report are the durable
handoff. The first sandbox signing attempt and test-fixture compile failure are
recorded separately from final passing results.

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
