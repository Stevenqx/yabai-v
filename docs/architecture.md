# Architecture boundaries

The fork preserves yabai's CLI client, Unix socket protocol, event loop, query
handlers, window manager, Space manager, and BSP/stack/float layout code.
`src/manifest.m` textually includes implementations in a single Objective-C
translation unit; the makefile also links upstream's embedded scripting-addition
payloads. Building those bytes does not install or load the scripting addition.

The new `vspace` domain is dispatched through the same daemon message interface.
Command handling belongs in `src/vspace.c`, included after the shared token
helpers are available. Milestone 0's exact `--ping` response is preserved.
Milestone 1 adds `--list`, `--current`, and `--focus N`; see the
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
- Virtual workspace mapping/focusing is implemented. Dynamic allocation,
  persistence, GUI, hotkeys, and display/window reassignment remain deferred.
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
regardless of tiling or float flags. Missing or unknown AX roles and untracked native
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
