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

## Git and GitHub workflow

- Automatically create local Git commits for each completed, validated unit of
  work, including documentation changes, unless the user requests otherwise.
  Do not leave completed work uncommitted merely because committing was not
  separately requested. If Git writes are blocked, report the blocker and what
  remains to be committed.
- Inspect the branch, working tree, and existing diff before starting. Use a
  descriptive task branch such as `feat/vspace-ping` or `docs/development-records`;
  reuse the current task branch when the work is related. Do not commit directly
  to `main` or `master`, and preserve upstream history.
- Make each commit one coherent change that can be reviewed and reverted on its
  own. Include related implementation, tests, and development-record updates
  together; separate unrelated work and avoid incomplete or broken checkpoints.
- Stage only the task's files or hunks. Review `git diff --cached` and run
  `git diff --cached --check` before committing. Preserve unrelated user changes;
  do not stage the entire working tree blindly, generated binaries, or secrets.
- Use Conventional Commits: `<type>(<scope>): <imperative summary>`; scope is
  optional. Choose an appropriate type such as `feat`, `fix`, `docs`, `refactor`,
  `test`, `build`, `ci`, or `chore`. Keep subjects concise, aiming for 50 characters.
  Example: `docs(workflow): require progress commits`.
- Use a body when needed to explain rationale, tradeoffs, or validation. Reference
  relevant issues when available. Mark breaking changes with `!` or a
  `BREAKING CHANGE:` footer according to the specification.
- Run the checks appropriate to the change before committing. Do not bypass
  commit hooks or configured commit signing to force a commit through.
- Keep the progress records consistent with the committed work. Report new
  commit IDs and validation results at handoff; record references to earlier
  commits in the journal when useful, without requiring a commit to contain its own ID.
- Rebase only unpublished work. Fetch upstream and record the new base when
  updating it. Do not amend another contributor's commits or rewrite published
  history or force-push without explicit authorization. Prefer follow-up commits
  for changes to published work.
- Review the diff before publishing. Push to the fork's `origin` only when
  authorized; local commit creation does not authorize a push or a merge.
- Use a focused pull request for integration when requested. Explain
  the problem, changes, validation, and remaining limitations; link relevant
  milestones or issues. Obtain review and pass applicable CI checks before merge,
  including the macOS 15/26 build matrix. Do not bypass required reviews or checks.

References: [Conventional Commits 1.0.0](https://www.conventionalcommits.org/en/v1.0.0/)
defines the message format; [GitHub flow](https://docs.github.com/en/get-started/using-github/github-flow)
describes the branch and pull request workflow.

## Development records

- Before each development task, read `docs/progress/README.md` and
  `docs/progress/milestones.md`, then the decision and journal entries relevant
  to the task. Follow the [resume instructions](docs/progress/README.md#resuming-work),
  verifying their branch and checkpoint against the checkout. Read the linked
  technical documentation when its subject is affected.
- Record the active task and its scope in the milestone document before changing
  implementation. Future milestones are proposals until explicitly authorized.
- Keep the main progress document current as work advances. Update the relevant
  milestone checklists, follow-ups, and limitations when their status changes.
- Append an entry to `docs/progress/journal.md` for each task, including the date,
  changes, checks actually performed and their results, blockers, and next step.
- Record significant design or workflow decisions in `docs/progress/decisions.md`
  with their rationale and status. Distinguish proposals from accepted decisions.
- Before handing off work, update these records and any affected technical docs.
  Link to detailed validation reports; preserve earlier journal entries and
  distinguish passed, failed, skipped, and unrun checks.
- Before every handoff, refresh the `Resuming work` section in
  `docs/progress/README.md` so another agent can continue without the chat history.
  Record the current task branch, relevant local and published checkpoints,
  completed validation, remaining blockers, next concrete steps, and authorization
  or validation gates. State when local documentation commits are not published.
  Treat recorded PIDs, window IDs, and host/session state as historical and require
  rechecking them before use. Do not present proposed work as authorized.
- For documentation-only changes, check Markdown links and `git diff --check`;
  rebuilding or running the daemon is unnecessary unless executable behavior changes.

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
- Create local commits according to the workflow above; pushes and merges require
  explicit user authorization.
