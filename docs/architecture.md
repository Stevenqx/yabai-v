# Architecture boundaries

The fork preserves yabai's CLI client, Unix socket protocol, event loop, query
handlers, window manager, Space manager, and BSP/stack/float layout code.
`src/manifest.m` textually includes implementations in a single Objective-C
translation unit; the makefile also links upstream's embedded scripting-addition
payloads. Building those bytes does not install or load the scripting addition.

The new `vspace` domain is dispatched through the same daemon message interface.
Its implementation belongs in `src/vspace.c`, included after the shared token
helpers are available. For this milestone it has no state and no macOS API calls.

```
./bin/yabai -m vspace --ping
{"status":"ok","component":"vspace"}
```

The response proves that the request reached a daemon containing the extension.
It does not report Accessibility permission, scripting-addition availability,
layout health, or virtual workspace support. Missing/unknown commands and extra
arguments fail using the existing CLI error protocol.

Future virtual workspace state should remain separate from native macOS Spaces
and layout algorithms. The daemon will own that state; any future native overlay
or integrated hotkey engine will use explicit interfaces to request changes.
This milestone adds neither subsystem nor a Swift project.

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
- Virtual workspaces, workspace persistence, GUI overlays, hotkey handling, and
  display/window reassignment policies are not implemented.
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
The CLI/startup connection follows in the next implementation checkpoint.

Initialization traverses native display order, then each display's native Space
order. It assigns separate sequential internal IDs and global one-based visible
indices to occupied, uncertain, and globally active Spaces. Confirmed-empty
inactive Spaces are spare. Duplicate native IDs cannot produce two mappings;
conflicting display ownership or incomplete enumeration fails atomically.
Repeated initialization preserves the original session snapshot.

Occupancy uses the native window list with hidden/minimized windows included,
then tracked AX roles. Ordinary standard, floating, and dialog windows count
regardless of tiling or float flags. Missing AX information and untracked native
candidates at upstream's ordinary window levels are unknown, hence allocated.
Failed queries and incomplete native iteration are also unknown. Known desktop
and nonordinary windows do not alone allocate a Space. No BSP tree is consulted.

Focus resolves only a virtual index, checks the native display association, and
calls `space_manager_focus_space` unchanged. It translates native failures and
checks native active state for approximately one second, because upstream's
gesture fallback can report success without switching. No new scripting-addition
requirement is introduced. External Space/display changes do not reassign IDs;
stale targets fail and startup rebuilding remains the recovery mechanism.
