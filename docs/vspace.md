# Virtual workspace mapping and focusing

Milestone 1 adds a daemon-owned VirtualWorkspaceManager; M2-A adds explicit
spare allocation and native focus results; M2-B adds explicit evacuation/release;
M2-C adds manual refresh and global index compaction.
Each allocated virtual
workspace maps to exactly one native macOS Space, identified by its native ID,
and records its native display ID. It does not position windows or choose a
layout. Existing `space`, `window`, and `query` commands keep their native meaning.

## Commands

```sh
yabai -m vspace --ping
yabai -m vspace --list
yabai -m vspace --current
yabai -m vspace --create
yabai -m vspace --focus 2
yabai -m vspace --destroy 3
yabai -m vspace --destroy 3 --to 2
yabai -m vspace --refresh
```

Use `./bin/yabai` for the checkout client. During an approved development daemon
session, prefix commands with `USER=yabai-fork-dev` as described in the
[development guide](development.md#manual-daemon-smoke-test). A new client cannot
add these commands to an older running daemon.

`--ping` preserves the exact Milestone 0 response, including the trailing newline:

```json
{"status":"ok","component":"vspace"}
```

`--list` returns a JSON array in virtual-index order, followed by a newline:

```json
[
  {"index":1,"id":1,"space-id":3,"display-id":1,"has-focus":true},
  {"index":2,"id":2,"space-id":395,"display-id":2,"has-focus":false}
]
```

| Field | Meaning |
| --- | --- |
| `index` | Positive, one-based user-visible index, unique across all displays; the argument to `--focus` |
| `id` | Opaque internal virtual ID, separate from index and native ID; stable only within this daemon session |
| `space-id` | Native 64-bit Space ID, never a Mission Control index |
| `display-id` | Native display ID associated with this mapping at allocation, matching `query --displays` IDs |
| `has-focus` | Whether this is the globally focused native Space at query time; distinct from merely visible on a display |

IDs are JSON integers. Clients must preserve 64-bit integer precision. Internal
IDs and visible indices initially both count from one; their equality is not an
identity contract. Display IDs and Space IDs are not persistent virtual IDs.

`--current` returns one object with the same fields and `has-focus: true`. It
fails with `current native space is not mapped to a virtual workspace.` when the
user has switched natively to an unallocated spare or newly created Space, or
native focus cannot be determined. In that case `--list` has no focused entry.

`--create` appends one virtual workspace mapped to the first eligible existing
native desktop in enumeration order **on the currently focused display**. It
requires an unmapped, nonvisible desktop confirmed empty using current native
window state. It excludes fullscreen/non-desktop Spaces, visible unmapped Spaces,
floating/hidden/minimized ordinary windows, and unknown occupancy. It never
falls back to another display or treats a missing layout node as empty.

The command re-enumerates topology, visibility, desktop type, occupancy, and
mapping eligibility before committing, and rechecks the focused display. Native
state changes during selection fail without allocation; retry explicitly. A
missing focused display, unavailable topology/visibility, no eligible spare on
that display, and exhausted virtual IDs/indices also fail without mutation.
Commands/events use the existing serial daemon loop; macOS can still change
external state after the last native observation.

Success has no output and exits zero. Find the new mapping in `--list`, then use
its `index` with `--focus`. The appended workspace receives a fresh session-local
ID and the index after the highest existing index. All existing IDs, indices,
and relative order remain unchanged. Allocation changes no focus, window,
layout, or native Space order. Even while empty, it stays mapped for this session
until explicit destruction or refresh releases it.

`--destroy SOURCE [--to TARGET]` evacuates the source's ordinary windows, then
releases only its virtual mapping. Applications/windows and the native Space stay
open. Indices use the same strict positive-decimal format as `--focus`.
An explicit target must be a different surviving mapping to a connected native
desktop. Without `--to`, select the nearest valid preceding workspace on the
source display in global virtual order, otherwise the nearest valid following
one on that display. Interleaved other-display workspaces are skipped. Without
a usable local destination, specify `--to`; cross-display evacuation is explicit.
The last virtual workspace cannot be destroyed, even when empty.

A source visible on any display needs a surviving mapped desktop on that same
display before it can be released. For a local target, show that target. For a
cross-display target, show the preceding-then-following local replacement. If no
local replacement exists, a visible source cannot be destroyed even with `--to`;
preflight rejects it before any move. A hidden source may use a cross-display
target without a local replacement, but becoming visible during evacuation then
blocks release. Fullscreen/non-desktop sources and destinations are unsupported.

Preflight enumerates native windows with minimized/application-hidden windows
included, using the same conservative occupancy classification as allocation.
Root standard, floating, and dialog windows are eligible independently of tiling,
float/minimize flags, or application visibility. Child/attached windows,
sticky/all-Spaces windows, native fullscreen windows, unfamiliar roles, untracked
ordinary candidates, incomplete membership/enumeration, and unavailable AX state
block evacuation. Known menu roles and native desktop/nonordinary auxiliary
windows remain excluded by the established classifier. No BSP tree proves emptiness.

The existing `window_manager_send_window_to_space` path handles movement and
float/BSP/stack bookkeeping, including cross-display native placement. No custom
geometry, minimization, unhide, or fullscreen transition is introduced. macOS may
restrict native moves or alter placement; a sent request is never proof of a move.
The daemon revalidates retained AX/window/PID identities and exact native Space
membership before each move and observes asynchronous completion on later event
turns. Closed or independently departed windows need no repeated move. New
arrivals or unknown occupancy retain the mapping. Observed membership reconciles
the native move path's optimistic untile/retile bookkeeping, including failures.

The globally focused source finishes focused on the destination. A background
source restores the original global Space and surviving focused window where
native APIs allow; closed/reused window identities are never focused. Visibility
and focus must be observed, with up to 100 deferred 50 ms observations per pending
move/handoff (daemon load/native queries may lengthen this). Two empty observations
on separate event turns and a final topology/occupancy/visibility check precede
release. `--focus` itself retains M2-A's native result semantics without this wait.
While destroy is pending, other vspace destroy/create/focus/refresh requests fail busy;
list/current/ping, upstream commands, and native events continue processing.

Success has no output and exits zero after release. Preflight failure leaves
windows/mappings untouched. A later failure retains the source mapping and exits
nonzero, reporting stable source/target IDs and indices, native IDs, any pending
window, windows observed at the destination, and the remaining failure. Moved
windows stay where observed; focus/visibility or frames may already have changed.
Uncertain layout membership is not asserted as repaired. Inspect native queries
and retry; the retry gathers only remaining live source windows. This is not an
atomic transaction with macOS: external changes after the last observation remain
possible, and native requests can complete after a reported failure.

Every destroy survivor keeps its ID, index, and relative order. Index gaps remain
until explicit refresh. Later `--create` can reuse a released, still-hidden empty
desktop with a fresh ID and the index after the highest surviving index.

`--refresh` releases only valid mapped native desktops confirmed empty and hidden
on **every connected display**. It protects the globally focused workspace and
each display's visible Space, including displays without keyboard focus. Floating,
minimized and application-hidden windows count through the same authoritative
occupancy scanner as allocation/destroy; layout trees never establish emptiness.
Unknown occupancy, unsupported classifications, stale/unavailable native mappings,
non-desktop Spaces and disconnected-display mappings are retained. The last mapping
is retained even if every native visible desktop is unmapped.

Refresh filters the existing global order and numbers survivors `1..N`, preserving
each survivor's ID, native mapping, display association and relative order. For
example, interleaved `A1, B1, A2, B2, A3, B3` becomes `A1, B2, A3, B3` if only
hidden empty B1/A2 are released. Gaps left by destroy compact the same way. The next
creation appends at `N+1` with a fresh ID; it still rechecks native emptiness.

The manager prepares a separate candidate, revalidates release candidates against
fresh topology, visibility, identity and occupancy, and retains candidates that
became occupied, visible, focused or uncertain. Missing/inconsistent required
display/Space enumeration, visibility or focus aborts with no mapping change.
Topology changes during the operation also abort. Refresh fails busy while a
destroy is pending, preserving its source, target and replacement identities and
indices. Releases and survivor numbering publish together on the daemon loop.
External native changes after the final observations remain possible.

Success returns one JSON object and a newline, exits zero, and reports only this
operation. All arrays follow the prior global order:

```json
{
  "released": [{"index":2,"id":202,"space-id":400,"display-id":12}],
  "reindexed": [{"id":303,"old-index":3,"index":2}],
  "retained": [
    {"id":101,"index":1,"reason":"focused"},
    {"id":303,"index":2,"reason":"occupancy-unknown"}
  ]
}
```

`released` identifies removed mappings with their old indices; `reindexed` contains
only changed indices; `retained` lists every survivor with its resulting index
and retention reason. Reasons are `focused`, `visible`, `occupied`,
`occupancy-unknown`, `display-unavailable`, `mapping-unavailable`, or
`last-workspace`. Uncertain mappings remain identifiable even when compaction
changes their indices. `--list` fields and meanings are unchanged. Repeating
refresh after native state settles leaves IDs/mappings/indices unchanged and
returns empty `released`/`reindexed` arrays.

Refresh is manual, including for newly created empty workspaces. It changes only
virtual metadata: no native Space/window move, reorder, layout or focus call, and
no synthetic focus event or recency update. Other commands and timers never
trigger refresh. Broader display reconciliation and explicit reordering remain
future work.

`--focus N` accepts a positive decimal integer up to `2147483647`; leading zeros
are accepted. It resolves the virtual index to a native Space ID and calls the
existing native Space-focus implementation. Success produces no output and exits
zero. Missing, malformed, overflowing, or unmapped indices fail. Focusing the
already focused workspace fails, matching upstream's native focus convention.

Other failures include a missing or reassociated native Space/display, an active
Mission Control session, display animation, and native focus errors. M2-A returns
the native `space --focus` result directly after virtual lookup/stale validation.
It adds no polling or timeout. Under SIP, native gesture dispatch can succeed
without a completed visible switch; query and visually verify the final state.

All errors use yabai's existing daemon failure marker: the CLI prints the message
to stderr and exits unsuccessfully. If startup enumeration failed, mapping
commands report `virtual workspace mapping is not initialized.` and ping and
upstream domains remain available. Each vspace request accepts exactly one
command and its documented arguments; extra arguments fail before any mutation.

## Startup policy and session stability

After native Space and window managers initialize, the manager traverses yabai's
native active-display list and each display's native Space list in their existing
order. It registers Spaces with ordinary windows and always registers the
**globally active Space**, even when empty. Other confirmed-empty Spaces,
including empty Spaces visible on unfocused displays, remain unallocated spares.
The snapshot includes enumerated fullscreen Spaces when occupied or active;
virtual workspaces do not change native fullscreen behavior.

Occupancy comes from native window enumeration with minimized/hidden windows
included, then tracked window roles. Standard, floating, and dialog windows
count regardless of BSP/stack/float layout, management rules, or float flags.
The BSP tree is never an occupancy source. Missing, unknown, or custom AX roles
or subroles, untracked ordinary-window candidates, failed queries, and incomplete native iteration are
uncertain and conservatively register the Space. This may allocate a visually
empty Space containing unidentified system UI; it avoids treating uncertain
occupied Spaces as spare. Confirmed desktop/background windows alone do not
allocate an inactive Space.

The startup snapshot is atomic. Duplicate native IDs produce no duplicate
workspace, and conflicting display ownership or incomplete Space enumeration
leaves mapping commands unavailable instead of exposing a partial map.

Existing IDs and associations stay fixed for the current daemon session;
indices change only through explicit refresh, and lifecycle operations can release IDs.
Closing all windows does not automatically remove a mapping; opening windows on a spare does
not allocate it. Native reordering does not renumber virtual indices. New native
Spaces and displays are not mapped automatically; `--create` can allocate an
eligible spare on the currently focused connected display. Stale or reassociated
focus targets fail. Rebuilding at daemon startup is the current recovery path;
changing an installed service still requires approval.

## Limits and validation

No independent vspace move-window command, reordering, window positioning, new layouts,
persistence across restarts, GUI, or hotkeys are implemented. Follow-on designs
are recorded in [architecture](architecture.md#follow-on-proposals).
Fork focus works through upstream's existing gesture fallback with SIP
fully enabled and does not require the scripting addition or root. Upstream's
privileged operations retain their existing restrictions.

Build/headless tests do not establish interactive compatibility. See the
[M2-C GUI validation](validation.md#milestone-2-c-approved-gui-validation--2026-11-05)
for available two-display runtime checks, native-focus limits and remaining
platform/scenario coverage; [local validation](validation.md#milestone-2-c-refresh-and-local-validation--2026-11-05)
records the build/headless checks. The earlier
[M2-B GUI validation](validation.md#milestone-2-b-approved-gui-validation--2026-11-05)
applies to that source. Run the
[standard test script](development.md#standard-vspace-test-script) for automatic
and interactive checks on your own Mac. Ongoing validation focuses on macOS 15;
macOS 26 testing is deferred to a later run on a suitable host.
