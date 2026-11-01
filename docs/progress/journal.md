# Development journal

Append an entry for every task. Include date, scope, changes, validation results,
open issues, next step, and commit references when available. Earlier entries
describe what was known at the time; record later corrections in a new entry.

## 2026-11-01 — Environment and minimal vspace smoke test

Scope: first fork milestone, M1.

Established the complete-history fork, local build/signing workflow, agent
conventions, technical documentation, and macOS 15/26 CI configuration. Added
the isolated `vspace --ping` handler and protocol tests while preserving existing
domain handlers and layout implementations.

Validation: upstream build and two original tests passed. The modified signed
universal build, six headless tests, static checks, live ping, and original and
development daemon queries passed on macOS 15.6 arm64. The original service was
restored after approved tests, its file hashes matched, and SIP stayed enabled.

Open issues: GUI focus/Space-switch tests were skipped by the safe-test conditions;
macOS 26 runtime and GitHub CI runs remain unverified. Initial Accessibility
permission was granted manually. Full evidence is in the
[validation report](../validation.md).

Commits: `271fb07`, `e1f1968`, `c8c4a93`, `3ec8c49`.
Next: review and obtain push authorization, run CI, and finish outstanding
validation before a separately authorized workspace-design milestone.

## 2026-11-01 — Development process records

Scope: documentation and agent instructions, M2.

Created [the progress index](README.md), [milestones](milestones.md), and
[decision log](decisions.md), and recorded the initial setup in this journal.
Updated [AGENTS.md](../../AGENTS.md) to require reading and maintaining these
records throughout development, and linked the index from the development guide.

Validation: all 21 local Markdown links and whitespace checks across six documents
passed, including `git diff --check`. The agent instructions reference both the
main progress index and required task journal. Executable behavior is unchanged,
so no build or daemon test was needed for this task.

Open issues: the M1 validation follow-ups remain open.
Next: use these records for the next development task and maintain the outstanding
validation follow-ups in M1.
