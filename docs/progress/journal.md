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

## 2026-11-02 — Publish CI and complete macOS 15 GUI checks

Scope: the user approved pushing the reviewed `feat/vspace-smoke` branch at
`6359db7` and running CI, plus temporary service pauses for focus and existing-Space
tests with original service, Space, and focus recovery. No merge was authorized
or performed; no product implementation changed.

Published `6359db75df35aa875b780a4e83fc89ab533b4015` to the fork's `origin`.
[Actions run 41403274379](https://github.com/Stevenqx/yabai-v/actions/runs/41403274379)
passed both jobs: macOS 15.8.2 and macOS 26.6.2 arm64 hosts each built and verified
the universal x86_64/arm64 binary, passed all six headless tests, and passed
whitespace/shell-syntax checks. Both test builds emitted the same five upstream
CoreVideo deprecation warnings. CI is build/headless evidence, not a macOS 26
interactive session.

The signed local binary passed verification, live ping, invalid-command, query,
and float-config checks. Computer Use blocked Terminal access for safety and a
Finder inspection timed out; the user manually prepared the disposable windows.
After an initial query-confirmed sequence, the user requested a repeat and
confirmed both focus changes (7966 → 7980 → 7966) and the existing-Space round trip
(Space ID 3 → 7 → 3 on display 1) worked visibly. Each repeated state was held for
five seconds; queries confirmed focus and visibility as well. No native Space
was created/deleted and no scripting addition was installed or loaded.

All sessions restored the original service. An initial final-state check caught
focus changing at original-service startup; restoration through that daemon
corrected it. Updated the [development workflow](../development.md#manual-daemon-smoke-test)
to require verifying focus after startup too. Automatic review held the last
session's completion until visual confirmation, then recovery succeeded. Final
verification found exactly one Homebrew daemon (PID 9357), original visible
Spaces and focus restored, all query domains working, original file hashes
unchanged, and SIP enabled. The two repeat-test windows remain open as promised.

Updated the validation report, main progress index, and M1 checklists. Detailed
evidence and local log paths are in the
[latest validation update](../validation.md#publication-and-gui-validation--2026-11-02).
All 33 local Markdown links, including anchors across eight documents, and
`git diff --check` passed. No new executable changes were made, so additional
local builds or test runs are unnecessary.

Remaining limitation: the user confirmed no macOS 26 GUI host/session is
available. Its interactive runtime check remains unrun; detailed design and
implementation remain deferred. Next: obtain that host/session and record its
runtime evidence before closing M1 or explicitly revise the validation gate.

## 2026-11-03 — Verify the repository handoff

Scope: confirm that another agent can reconstruct current progress and continue
from the repository. Starting HEAD: `dbdc0c0` on `feat/vspace-smoke`, with a clean
working tree and one local commit beyond the published tracking branch.

Checked the agent instructions, progress index, milestones, decisions, journal,
validation report, architecture boundaries, and development guide. They record
the implemented ping command, passing macOS 15/26 CI at `6359db7`, visually
confirmed macOS 15 GUI checks, recovery evidence, and the unavailable macOS 26
runtime host. Added explicit resume instructions and the distinction between
local validation records and the published, CI-tested commit to the progress
index. Historical desktop identifiers must be rechecked before reuse.

Validation: all 33 local Markdown links, including anchors across eight
documents, and `git diff --check` passed.
No executable or host state changed, so builds and runtime tests are unnecessary.
Next: resume the remaining macOS 26 runtime check when a host is supplied, or
obtain an explicit revision of the validation gate before a separately
authorized design milestone. No push or merge was performed for this audit.
