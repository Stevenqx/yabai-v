# Development progress

This is the main development record for `Stevenqx/yabai-v`. Start here before
working on the fork, and keep this overview aligned with the supporting documents.

## Current position

The development environment and minimal `vspace --ping` extension are implemented.
The signed universal binary, six headless tests, live ping, and existing query
commands passed validation on macOS 15.6 arm64. Window focusing and switching
existing Spaces also passed queries and user visual confirmation on 2026-11-02.
The original yabai service, visible Spaces, and focus were restored; SIP remained
enabled and the original installation files are unchanged.

The upstream base is `dd845723416f5fe92af49fad5ebab00369e07edd`.
The setup work is recorded in four Conventional Commits ending at `3ec8c49`.
All six commits through `de6a849` have now been reviewed with no blocking
findings, and all six headless tests passed again. The reviewed task branch was
published at `6359db7`; [both macOS 15/26 CI jobs passed](https://github.com/Stevenqx/yabai-v/actions/runs/41403274379)
for that commit, including universal builds, ad-hoc signature verification, and
six tests per host. Local GUI sessions used the persistent-certificate binary.
macOS 26 interactive runtime remains unverified because no GUI host/session is
available. See the [validation report](../validation.md) for evidence and limits.

The progress folder and agent maintenance rules are established in M2 and
committed as `724ba46`. M3, committed as `de6a849`, adds a
[version-control workflow](../../AGENTS.md#git-and-github-workflow) that requires
local Conventional Commits for validated work and review before GitHub integration.
Virtual workspace management, the native GUI, and the hotkey engine are future
work and require a separately authorized milestone.

The completed publication and macOS 15 GUI checks are recorded in the
[latest validation update](../validation.md#publication-and-gui-validation--2026-11-02).
The remaining M1 follow-up is macOS 26 interactive runtime validation; detailed
design and implementation remain deferred. At the 2026-11-02 handoff, the two
disposable terminal windows from the confirmed repeat were left open as promised.

## Resuming work

Handoff checked on 2026-11-03. Use this checkout on `feat/vspace-smoke`, which
includes the validation-record commit `dbdc0c0`. The published, CI-tested commit
is `6359db7`; the later validation records are local, so a fresh GitHub clone
needs those documentation commits before it has the same handoff context.

1. Inspect the branch and working tree, then read the milestones, decisions,
   latest journal entry, and linked technical guides. Treat recorded PIDs,
   window IDs, and desktop state as historical; recheck them before use.
2. Resume M1 with macOS 26 interactive runtime validation when a GUI host/session
   becomes available. Follow the signed-build, GUI, and recovery procedure in
   the development guide; keep the remaining check unrun until evidence exists.
3. After that evidence or an explicit user revision of the validation gate,
   define a separately authorized design milestone for daemon-owned workspace
   state and SIP-compatible window visibility/assignment. Detailed design,
   workspace implementation, GUI, and hotkey work are not currently authorized.

## Supporting documents

| Document | Purpose |
| --- | --- |
| [Milestones](milestones.md) | Completed work, active scope, and remaining follow-ups |
| [Decisions](decisions.md) | Accepted decisions, their rationale, and future proposals |
| [Development journal](journal.md) | Dated task history, validation outcomes, and handoffs |

Technical references: [development workflow](../development.md),
[architecture boundaries](../architecture.md), [validation report](../validation.md),
and [agent conventions](../../AGENTS.md).

## Maintaining the record

At task start, read this overview and the milestones, then relevant decisions,
journal entries, and technical references. Identify the authorized scope and
record an active milestone task before implementation changes.

As work progresses, update milestone status and record significant decisions.
Before handing off, refresh this overview and append a journal entry with the
date, changes, checks and results, unresolved issues, and next step. Include
commit references when available. Preserve previous entries; explain corrections
or superseded decisions in a later entry.

Link to detailed technical documentation and validation evidence. Keep unrun or
skipped checks explicit, and treat future plans as proposals rather than approval
to implement them. Do not store credentials or signing keys in these documents.
