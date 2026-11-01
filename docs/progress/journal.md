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

## 2026-11-01 — Conventional Commits and GitHub workflow

Scope: documentation and local version control, M3.

Recorded the completed progress-document task as `724ba46`
(`docs: add development progress records`). Updated
[AGENTS.md](../../AGENTS.md#git-and-github-workflow) to require local Conventional
Commits for completed, validated work. Added conventions for focused staging,
task branches, reviewable checkpoints, history preservation, and GitHub pull
requests with review and CI checks. Pushes and merges retain explicit authorization.
Recorded the rationale as [D005](decisions.md#d005--commit-validated-work-as-development-checkpoints).

Validation: the first documentation commit passed 21 local-link checks and staged
whitespace checks. The policy update passed all 25 local Markdown links and
whitespace checks across six documents, including `git diff --check`. The old
opt-in commit rule was replaced, and publishing still requires authorization.
Executable behavior is unchanged; no build or daemon test was needed.

Open issues: the M1 validation follow-ups remain open.
Next: use the committed workflow for subsequent work and complete M1 validation
follow-ups when authorized.

## 2026-11-01 — M1 validation follow-up and review

Scope: continue unfinished validation before detailed design or implementation.
Starting HEAD: `de6a849`, clean `feat/vspace-smoke` working tree and index.

Reviewed all six local commits against upstream
`dd845723416f5fe92af49fad5ebab00369e07edd`; no blocking findings. Confirmed the
minimal production-code scope, textual inclusion, existing failure protocol,
and unchanged upstream handlers/layouts/build system. Corrected the setup-time
import instructions and GitHub-access limitation; imported commits are already
present, and read-only GitHub inspection now succeeds outside the sandbox.
Updated the progress checklist and appended current evidence and a concrete
remaining-test plan to the [validation report](../validation.md#validation-follow-up--2026-11-01).

Validation: all six headless tests passed, with the same five upstream CoreVideo
deprecation warnings. The universal `make` build through `./scripts/build-dev.sh`
passed without warnings; the user handled the Keychain prompt manually.
Certificate signature verification passed outside the sandbox for all
architectures, with the same identifier and certificate leaf SHA-1. Shell syntax,
workflow YAML/macOS 15/26 matrix, upstream history, and baseline-to-HEAD whitespace
checks passed. The one original Homebrew daemon remains running; its read-only
queries passed, its three original file hashes still match, and SIP is enabled.
The rebuilt binary contains x86_64/arm64 and reports `yabai-v7.1.25`. All 29
local Markdown links, including anchors across eight documents, and
`git diff --check` passed.

Open issues: publication and CI remain pending explicit approval; the remote task
branch is absent and Actions has no runs. No service changes or new daemon/GUI
tests were performed. Display 1 now has two ordinary existing Spaces, but the
focused Space has no eligible window pair. An approved GUI session needs
disposable test windows and visible confirmation. macOS 26 runtime still needs
a separate supplied host/session. These remain validation limits, not passed
checks; future workspace design and implementation remain deferred.

Next: obtain publication and temporary service-pause approval, run CI for the
published HEAD and safe GUI tests, then record macOS 26 runtime evidence.
