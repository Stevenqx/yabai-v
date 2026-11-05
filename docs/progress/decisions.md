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

## D008 — Freeze a conservative native mapping for each session

Date: 2026-11-03. Status: accepted for Milestone 1.

Keep VirtualWorkspaceManager independent of native layout state. Use native
Space IDs and display IDs, separate internal virtual IDs and global visible
indices, and native enumeration order. Allocate occupied/uncertain Spaces and
the globally active Space; confirmed-empty inactive Spaces remain spare.
Incomplete window metadata must not establish emptiness. Include floating,
hidden, and minimized ordinary windows without consulting the BSP tree.

Build the snapshot atomically after native managers are ready. Freeze it until
restart; dynamic allocation, reassociation, and persistence are future work.
Reject stale native/display associations at focus time. Reuse upstream native
focus unchanged and verify observed activation to catch silent gesture failure.
A small query-source interface supports deterministic headless policy tests.

Reference: [manager architecture](../architecture.md#milestone-1-manager-foundation).

## D009 — Return native focus semantics

Date: 2026-11-04. Status: accepted for the authorized M2-A task.

Supersedes D008's observed-activation polling requirement. After virtual index
lookup and stale association validation, use the existing native focus path and
return its result. Preserve already-focused, Mission Control, animation, and
scripting-addition failures. Remove the extra one-second polling timeout:
dispatch success has the same guarantee as `space --focus`, including its SIP
gesture fallback limits. Observe actual switching separately in GUI validation.

The same user request authorizes spare mapping creation in M2-A and recording
follow-on designs, without authorizing later implementation, publication, or
installed service changes.

## D010 — Allocate only a revalidated focused-display spare

Date: 2026-11-04. Status: accepted for M2-A.

Extend the session snapshot by explicit `--create` only. Select the first
unmapped hidden desktop confirmed empty on the focused display using existing
native queries. Unknown state is ineligible, including unfamiliar AX roles.
Repeat topology/visibility/type/occupancy/mapping selection and recheck focus
before mutation. Fail without allocation if state changed; never fall back to
another display. Append a fresh monotonic virtual ID and highest-index-plus-one
without rewriting survivors or changing the native desktop. Empty allocations
stay mapped. This supersedes D008's frozen-snapshot restriction only for explicit
creation; native focus uses D009. External macOS changes remain observable races.

## D011 — Record lifecycle, persistence, and native MRU proposals

Date: 2026-11-04. Status: proposed for future implementation.

The user's follow-on design constraints are recorded under
[architecture proposals](../architecture.md#follow-on-proposals): evacuate before
destroying mappings with explicit cross-display targeting and retained mappings
on partial failure; manual refresh conservatively releases hidden empty mappings
and globally compacts indices; later persistence restores logical identity/order
after native revalidation. Native Space/window MRU is a separate observed-focus
feature, independent of virtual numbering. Final destroy policies, MRU storage,
commands, and selection scope remain deferred. No MRU seam is needed by M2-A.

## D012 — Maintain portable validation; focus current checks on macOS 15

Date: 2026-11-04. Status: accepted by the user's standard-test-script request.

Maintain `scripts/test-vspace.py` as the shared automatic/read-only/interactive
entry point, reusing production safety fixtures and the M2-A GUI procedures.
Record OS/SDK/architecture/source/binary and each passed/failed/skipped/unobserved
outcome. Restore all displays' visibility and focus between focus controls;
record native results separately from queried/visual activation. Avoid host IDs,
new package dependencies, daemon/service management, and unsafe live-state
injection. New commands must extend the applicable tests and guide.

The user explicitly deferred macOS 26 testing and chose macOS 15 for ongoing
development. Default CI now checks macOS 15 via the runner; optional manual CI
can include macOS 26 later. This revises the earlier dual-OS validation gate,
including the corresponding AGENTS.md workflow instruction. Support targets
remain 15/26; unrun platforms remain unverified. Cross-version runtime testing
will use the same script when suitable hosts are available. No publication,
service operation, or later milestone implementation is authorized by this task.

Reference: [standard runner and validation policy](../development.md#standard-vspace-test-script).

## D013 — Explicit evacuation with observed release

Date: 2026-11-05. Status: accepted for the user's M2-B implementation request in
`tmp/yabai-v-m2b-codex-prompt.md`. Supersedes D011's proposed destroy policy only;
refresh, persistence and MRU remain proposals.

Resolve source, destination and local visibility replacement to stable virtual
identities/native mappings at entry. Default to the nearest valid preceding
same-display workspace in global order, otherwise following; cross-display
evacuation requires `--to`. A visible source needs a surviving local mapped
desktop, even with a cross-display target. Always require a usable surviving
destination and retain at least one mapping. Preserve all survivor indices/IDs/
order; only future explicit M2-C refresh may compact gaps.

Reuse authoritative native occupancy and the existing window move/layout path.
Root ordinary/floating/dialog windows include minimized and application-hidden
ones. Child, sticky/all-Spaces, fullscreen and unknown occupancy block evacuation.
Retain AX/PID identities, revalidate native ownership/membership before moving,
and reconcile optimistic layout bookkeeping to observed membership. A partial
failure retains the source, reports observed moves, and allows a fresh retry.
Native move/geometry/state restrictions remain observable platform limits.

Use one pending operation on the daemon event loop with a monotonic token and
50 ms deferred continuation events. Hold the response stream until completion,
reject other vspace mutations while pending, and keep native events/read queries
processing. Wait for observed evacuation and required visibility/global focus,
then require two empty observations and a final fresh native-state check. Keep
ordinary `--focus` on D009's unchanged native path. Reserve the isolated mapping
release step for M2-D candidate persistence before publication. macOS state can
change after observation; do not claim a native transaction or guaranteed rollback.

The existing service is outside implementation authorization. The earlier M2-A
GUI approval covered finished sessions; another service pause needs explicit
approval under `AGENTS.md`. macOS 26 and hosted CI remain deferred/unpublished.
Native MRU stays separate and must follow observed focus, including external
changes; allocation/release/index gaps do not create history.

## D014 — Audit physical GUI frames independently of daemon caches

Date: 2026-11-05. Status: accepted for the approved M2-B GUI session and measured
baseline repeat; explicitly confirmed by the user after investigating padding.

Tracked-window JSON serializes the daemon's frame cache. An initial GUI attempt
stopped when a fresh daemon disagreed with that cache, but lacked independent
pre-pause WindowServer bounds; it cannot establish a physical resize. Padding
remained 10 pixels. The current Zotero frame exactly matches display 1's menu-bar/
padding calculation, while the older cached dimensions match display 2's padded
area despite a display-1 mapping. Record that evidence without declaring a proven
historical cause or rewriting the failed/inconclusive attempt as a pass.

Capture independent native bounds, ownership and exact Space membership before
service pause and during/after tests. Treat those measurements as physical-frame
evidence and keep cached-query comparisons separate. Preserve the original
configuration and validate its padding/layout after recovery. Do not resize user
windows merely to reproduce potentially stale cache values. The user approved
using the measured native baseline; the final independent audit matched both
native and cached frames, all original window identities/associations, topology,
visibility/focus, layouts/padding, file hashes and SIP.

This decision changes validation evidence, not CLI behavior or native geometry
code. The production source remains `ca48ac8`; no code change was needed for the
successful GUI checks. macOS 26 and publication remain outside this session.
