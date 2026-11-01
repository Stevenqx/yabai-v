# Development progress

This is the main development record for `Stevenqx/yabai-v`. Start here before
working on the fork, and keep this overview aligned with the supporting documents.

## Current position

The development environment and minimal `vspace --ping` extension are implemented.
The signed universal binary, six headless tests, live ping, and existing query
commands passed validation on macOS 15.6 arm64. The original yabai service was
restored, and SIP remained enabled.

The upstream base is `dd845723416f5fe92af49fad5ebab00369e07edd`.
The setup work is recorded in four Conventional Commits ending at `3ec8c49`.
Build checks for macOS 15 and 26 are configured but have not run on GitHub.
Window focusing, switching existing Spaces, and macOS 26 runtime behavior remain
unverified. See the [validation report](../validation.md) for evidence and limits.

The progress folder and agent maintenance rules are established in M2 and
committed as `724ba46`. M3 adds a
[version-control workflow](../../AGENTS.md#git-and-github-workflow) that requires
local Conventional Commits for validated work and review before GitHub integration.
Virtual workspace management, the native GUI, and the hotkey engine are future
work and require a separately authorized milestone.

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
