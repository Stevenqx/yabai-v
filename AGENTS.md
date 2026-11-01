# Development conventions

This is a long-term fork of asmvik/yabai. Keep upstream Git history and make
small changes that can be rebased. Use Conventional Commits with focused commits.

## Scope and compatibility

- Preserve existing CLI behavior and the BSP, stack, and float layouts.
- Put fork commands under `yabai -m vspace`. The only implemented command in
  this milestone is `--ping`.
- Support macOS 15 and 26 with SIP fully enabled. Never require the scripting
  addition, root access, or changes to SIP for fork functionality.
- Retain upstream scripting-addition code for compatibility. Its privileged
  operations remain unavailable with SIP enabled; do not promise otherwise.
- Do not implement native Space creation/deletion, virtual workspace logic,
  a GUI project, or a hotkey engine in this milestone.

## Code and architecture

- Follow surrounding upstream C/Objective-C style: four spaces, snake_case,
  static helpers, and existing token parsing and daemon failure conventions.
- Keep new vspace command handling isolated from existing domain handlers.
- Respect the textual inclusion model in `src/manifest.m`. Do not compile an
  included implementation as a separate translation unit.
- Keep the existing build system and avoid new dependencies or broad formatting.
- Keep future workspace state in the daemon; GUI and hotkey clients should
  communicate through the message interface. Those clients do not exist yet.

## Workflow and validation

- Read `docs/development.md`, `docs/architecture.md`, and the latest validation
  report before changing the development workflow.
- Inspect the build system and build an unmodified upstream base before making
  initial fork changes. Record the exact base commit and observed results.
- Build with `make`, run `make -C tests`, and check `git diff --check`.
- Use a persistent local signing identity for GUI sessions and verify the
  resulting signature. CI ad-hoc signatures do not establish a stable TCC identity.
- Report headless tests separately from a real daemon/GUI session. A build or
  parser test does not establish runtime compatibility on another macOS version.
- Rebase only unpublished work. Fetch upstream and record the new base when
  updating it. Never force-push without explicit authorization.

## Host safety

- Do not stop, restart, replace, install, or uninstall an existing yabai service
  without explicit approval. Do not overwrite user configuration or binaries.
- Do not start a second window-managing daemon alongside an existing daemon.
  Separate IPC sockets do not isolate their window-management effects.
- Request manual Accessibility and Keychain steps when macOS requires them.
  Do not reset TCC permissions or modify keychain trust automatically.
- Do not run `--load-sa`, request sudo, or change SIP.
- Keep development binaries in the checkout. Preserve the original installation
  and its service definition so it remains recoverable.
- Commit locally when requested; do not push unreviewed changes without approval.
