# Virtual workspace mapping and focusing

Milestone 1 adds a daemon-owned VirtualWorkspaceManager. Each allocated virtual
workspace maps to exactly one native macOS Space, identified by its native ID,
and records its native display ID. It does not position windows or choose a
layout. Existing `space`, `window`, and `query` commands keep their native meaning.

## Commands

```sh
yabai -m vspace --ping
yabai -m vspace --list
yabai -m vspace --current
yabai -m vspace --focus 2
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
| `display-id` | Native display ID associated with this mapping at initialization, matching `query --displays` IDs |
| `has-focus` | Whether this is the globally focused native Space at query time; distinct from merely visible on a display |

IDs are JSON integers. Clients must preserve 64-bit integer precision. Internal
IDs and visible indices initially both count from one; their equality is not an
identity contract. Display IDs and Space IDs are not persistent virtual IDs.

`--current` returns one object with the same fields and `has-focus: true`. It
fails with `current native space is not mapped to a virtual workspace.` when the
user has switched natively to an unallocated spare or newly created Space, or
native focus cannot be determined. In that case `--list` has no focused entry.

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
command and its documented arguments; extra arguments fail before any focus.

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
The BSP tree is never an occupancy source. Missing/unknown roles, untracked
ordinary-window candidates, failed queries, and incomplete native iteration are
uncertain and conservatively register the Space. This may allocate a visually
empty Space containing unidentified system UI; it avoids treating uncertain
occupied Spaces as spare. Confirmed desktop/background windows alone do not
allocate an inactive Space.

The startup snapshot is atomic. Duplicate native IDs produce no duplicate
workspace, and conflicting display ownership or incomplete Space enumeration
leaves mapping commands unavailable instead of exposing a partial map.

IDs, indices, and associations stay fixed for the current daemon session.
Closing all windows does not remove a mapping; opening windows on a spare does
not allocate it. Native reordering does not renumber virtual indices. Newly
created native Spaces and new displays remain unmapped; stale or reassociated
focus targets fail. Rebuilding at daemon startup is the current recovery path;
changing an installed service still requires approval.

## Limits and validation

No create/destroy, move-window, reordering, window positioning, new layouts,
persistence across restarts, GUI, or hotkeys are implemented. Milestone 2 has not
begun. Fork focus works through upstream's existing gesture fallback with SIP
fully enabled and does not require the scripting addition or root. Upstream's
privileged operations retain their existing restrictions.

Build/headless tests do not establish interactive compatibility. See the
[latest Milestone 1 validation](validation.md#milestone-1-publication-and-interactive-validation--2026-11-04)
for checks actually performed and remaining approval/runtime gates.
