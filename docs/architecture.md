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
- Native Space creation/deletion and programmatic Space focus through upstream's
  scripting addition are outside the supported fork workflow. Use Mission
  Control or existing system shortcuts for manual Space-switching validation.
- Window focusing and tiling require Accessibility permission and a GUI session.
  Screen Recording is only relevant to optional upstream animation features.
- Virtual workspaces, workspace persistence, GUI overlays, hotkey handling, and
  display/window reassignment policies are not implemented.
- A separate `USER` value changes upstream socket/lock names but does not isolate
  two daemons from the same WindowServer session. Never use it as justification
  to run competing window managers.

Keep upstream changes small: add a command domain and isolated implementation,
extend the existing tests, and leave existing domain handlers and layouts intact.
