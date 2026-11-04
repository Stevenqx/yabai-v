# Architecture boundaries

The fork preserves yabai's CLI client, Unix socket protocol, event loop, query
handlers, window manager, Space manager, and BSP/stack/float layout code.
`src/manifest.m` textually includes implementations in a single Objective-C
translation unit; the makefile also links upstream's embedded scripting-addition
payloads. Building those bytes does not install or load the scripting addition.

The new `vspace` domain is dispatched through the same daemon message interface.
Command handling belongs in `src/vspace.c`, included after the shared token
helpers are available. Milestone 0's exact `--ping` response is preserved.
Milestone 1 adds `--list`, `--current`, and `--focus N`; M2-A adds `--create`
and native focus result parity. See the
[command and JSON contract](vspace.md). Existing native selectors and domain
handlers are unchanged.

The daemon owns a separate `virtual_workspace_manager`, initialized after the
native Space and window managers and before the message loop/configuration.
It owns only mapping state, never window positions or layout state. Future GUI
and hotkey clients must use the message interface; they do not exist yet.

## Compatibility and limitations

- macOS 15 and 26 are the target platforms. CI checks builds and headless tests;
  actual GUI behavior requires separate tests on both operating systems.
- SIP stays fully enabled. Fork functionality must work without a scripting
  addition. Upstream's CLI commands remain present, including commands whose
  privileged behavior cannot work under this restriction.
- Native Space creation/deletion and other scripting-addition-only operations
  are outside the supported fork workflow. This base already has a gesture
  fallback for `space --focus` when the scripting addition is unavailable;
  preserve it. Verify actual switching visually and through queries because
  a successful CLI exit does not establish that a synthesized gesture worked.
- Window focusing and tiling require Accessibility permission and a GUI session.
  Screen Recording is only relevant to optional upstream animation features.
- Virtual mapping, explicit spare allocation, and focusing are implemented.
  Destroy, refresh, persistence, GUI, hotkeys, and display/window reassignment
  remain deferred.
- A separate `USER` value changes upstream socket/lock names but does not isolate
  two daemons from the same WindowServer session. Never use it as justification
  to run competing window managers.

Keep upstream changes small: add a command domain and isolated implementation,
extend the existing tests, and leave existing domain handlers and layouts intact.

## Milestone 1 manager foundation

`src/virtual_workspace_manager.h` and `.c` define an independent daemon-owned
mapping module, textually included by the manifest. The native adapter uses the
existing active-display and display-Space lists and WindowServer queries. A
query-source interface allows the same mapping/focus policy to run headlessly.
Startup initializes the mapping before requests arrive. Enumeration failure
logs a warning and leaves the new mapping commands unavailable; the original
daemon commands and ping continue to work.

Initialization traverses native display order, then each display's native Space
order. It assigns separate sequential internal IDs and global one-based visible
indices to occupied, uncertain, and globally active Spaces. Confirmed-empty
inactive Spaces are spare. Duplicate native IDs cannot produce two mappings;
conflicting display ownership or incomplete enumeration fails atomically.
Repeated initialization preserves the original session snapshot.

Occupancy uses the native window list with hidden/minimized windows included,
then tracked AX roles. Ordinary standard, floating, and dialog windows count
regardless of tiling or float flags. Missing, unknown, or custom AX roles/subroles and untracked native
ordinary-window candidates are unknown, hence allocated. Native desktop levels
and the existing ordinary-window tag predicate exclude system UI without AX.
Failed queries and incomplete native iteration are also unknown. Known desktop
and nonordinary windows do not alone allocate a Space. No BSP tree is consulted.

Focus resolves only a virtual index, checks the native display association, and
calls `space_manager_focus_space` unchanged. M2-A translates its results directly,
including already-focused and scripting-addition errors, without M1's extra
polling or timeout. Native gesture dispatch success does not establish observed
activation under SIP. External Space/display changes do not reassign IDs; stale
targets fail and startup rebuilding remains the recovery mechanism.

## Milestone 2-A allocation

The native source extends the existing query seam with focused-display lookup,
per-display visible Space, and `space_is_user` (native type zero). The guarded
focused-display adapter reuses `display_manager_active_display_uuid` and
`display_id`; it returns unavailable instead of the convenience accessor's NULL
assertion. No fallback display is selected.

`virtual_workspace_manager_create` reads connected displays and the focused
display's native Space order. It verifies display ownership and a valid visible
Space before considering unmapped hidden desktops. It reuses the native occupancy
adapter, not the filtered `space_window_list` result: that result alone cannot
distinguish an empty Space from failed/incomplete iteration or untracked ordinary
windows. Unfamiliar tracked AX roles/subroles are unknown; only recognized menu
roles and the existing native desktop/nonordinary filters establish exclusions.

Before appending, repeat candidate selection with fresh native queries and
require the same candidate and focused display. Reject changed/uncertain state
without touching mapping state. This runs inside `DAEMON_MESSAGE` on the serial
event loop, with no new worker, lock, polling, or event handler. Queries observe
macOS state; no transaction can prevent subsequent external native changes.

A daemon-owned monotonic `next_id` advances only on successful allocation.
The visible index is separately computed as the highest existing index plus one;
overflow is rejected. Append without rewriting survivors, switching focus,
moving windows, touching layout trees, or reordering native Spaces. Empty
allocations persist for the session. Startup policy is otherwise unchanged.

## Follow-on proposals

These are design directions, not authorization to implement later milestones.

### M2-B — Destroy

Evacuate a source workspace's windows into another active virtual workspace using
`window_manager_send_window_to_space`, `space_manager_move_window_to_space`, and
the existing untile/retile/layout paths. Reject destruction of the last virtual
workspace. Prefer an adjacent workspace on the same display by default; crossing
displays requires an explicit destination. A focused source uses the destination
as its focus target; a background source preserves focus where possible.

Release the source mapping only after confirming evacuation using current native
state, including floating/hidden/minimized windows. If movement fails partway,
retain the source mapping and report failure plus the windows already moved.
Define the final adjacency/target-selection and partial-failure policy during
M2-B implementation. Existing movement paths need postcondition checks; their
invocation alone does not establish evacuation. Keep SIP enabled and do not
promise unsupported native privileged operations.

### M2-C — Manual refresh

An explicit `vspace --refresh` releases only mapped Spaces confirmed empty and
hidden on **all connected displays**. Preserve visible Spaces on unfocused
displays, disconnected-display mappings, and every unavailable/uncertain state.
Retain at least one virtual workspace. Compact survivor indices globally to
`1..N`, preserving relative order, internal IDs, and native mappings, including
interleaved workspaces from different displays. Update only virtual state.
Broader display reconciliation and explicit reordering may extend this later.

### M2-D — Persistence

M2-A remains session-local. Persist logical identity and order later, and
revalidate native mappings at startup before restoring them. Neither native
Space IDs nor display IDs alone are persistent logical workspace identities.

### Separate feature — Native Space and window MRU

Current `parse_space_selector` in `src/message.c` resolves `recent` through
`g_space_manager.last_space_id`. Window `recent` uses
`window_manager_find_recent_managed_window`/`last_window_id`; stack recency uses
`window_manager_find_recent_window_in_stack` and node `window_order`.

Likely observation points are `SPACE_CHANGED` and `DISPLAY_CHANGED` in
`src/event_loop.c`, plus `window_did_receive_focus`, its application-front-switch
and recovered/lost-event callers, and `WINDOW_FOCUSED`. History must follow
observed focus, including Mission Control, mouse, system shortcuts, and native
CLI changes outside `vspace`. Command dispatch success alone is insufficient.

Use native Space/window identities with current enumeration/ownership and window
lifetime checks (including destroyed/reused IDs and application lifetime).
Resolve a virtual view of native history through current mappings; refreshing
virtual indices must never rewrite recency. Defer history storage, public
commands, and global versus per-display policy. M2-A needs no history seam, so
none is added. Review display removal, Space removal, and window destruction when
implementing invalidation.
