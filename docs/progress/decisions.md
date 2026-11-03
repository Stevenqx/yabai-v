# Decisions

Record significant decisions with a stable ID, date, status, rationale, and links.
Use proposed, accepted, or superseded status. Add a dated superseding decision
when a direction changes, and retain the history.

## D001 — Extend the CLI through vspace

Date: 2026-11-01. Status: accepted for the smoke-test milestone.

Keep fork commands under `yabai -m vspace` and isolate their handling from existing
domains. The current extension implements only `--ping`. This preserves existing
CLI and layout behavior while keeping the patch small for upstream rebases.

Reference: [architecture boundaries](../architecture.md).

## D002 — Keep SIP enabled and preserve the installed service

Date: 2026-11-01. Status: accepted project constraint.

Fork functionality must work without the scripting addition or root privileges.
Retain upstream compatibility code while documenting privileged-operation limits.
Native Space creation/deletion is outside fork scope. Development uses a signed
checkout binary and explicit config; changes to the installed service require
explicit authorization, and test sessions must restore its prior state.

References: [agent conventions](../../AGENTS.md),
[development workflow](../development.md), [recovery results](../validation.md).

## D003 — Preserve upstream architecture and defer new subsystems

Date: 2026-11-01. Status: accepted boundary; future design is pending.

Keep the existing build system, C/Objective-C style, textual inclusion model, and
BSP/stack/float layouts. Future workspace state belongs in the daemon, with GUI
and hotkey clients using explicit interfaces. Their detailed design and
implementation remain future work; no Swift GUI project has been added.

Reference: [architecture boundaries](../architecture.md).

## D004 — Maintain development records alongside changes

Date: 2026-11-01. Status: accepted by the documentation task.

Use a main progress index, milestone checklist, decision log, and dated journal.
Agents read the index and relevant records before development and update them
before handoff. Technical guides and validation reports retain detailed evidence;
the progress documents link to those references to keep status and history clear.

Reference: [maintenance rules](README.md#maintaining-the-record).

## D005 — Commit validated work as development checkpoints

Date: 2026-11-01. Status: accepted by the version-control workflow task.

Agents create focused local Conventional Commits for completed, validated work,
including documentation and its development records. This makes task history
traceable and allows individual changes to be reviewed or reverted. Stage only
related changes and preserve upstream and published history. Local commits do
not authorize publication; pushes and merges still need user authorization,
with review and applicable CI checks before integration.

Reference: [Git and GitHub workflow](../../AGENTS.md#git-and-github-workflow).

## D006 — Keep resume instructions current at every handoff

Date: 2026-11-03. Status: accepted by the user's agent-instruction request.

Agents read the progress index's `Resuming work` section at task start and refresh
it before every handoff. Include local/published checkpoints, validation, blockers, next steps,
and authorization or validation gates so another agent can continue without chat
history. Recheck historical host identifiers before reuse, and keep proposed work
distinct from authorized scope.

Reference: [development records rules](../../AGENTS.md#development-records).

## D007 — Align the roadmap and authorize mapping/focusing

Date: 2026-11-03. Status: accepted by the user's Milestone 1 request.

All earlier setup and documentation tasks belong to Milestone 0. Historical
entries keep their original labels for traceability. The user authorizes
Milestone 1 mapping/focusing now, revising the previous gate that deferred
implementation until macOS 26 GUI validation. That runtime check remains unrun.
Only this milestone is authorized; lifecycle, movement, persistence, GUI, and
hotkeys remain deferred. Publication and service changes still require approval.

Reference: [Milestone 1 scope](milestones.md#milestone-1--virtual-workspace-mapping-and-focusing).
