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

## 2026-11-03 — Require agents to maintain resume instructions

Scope: the user requested the resume-instruction practice in `AGENTS.md` so other
agents maintain the same handoff standard. Starting HEAD: `f92ea9d`, clean
`feat/vspace-smoke` working tree.

Required agents to read the resume instructions at task start and refresh them
before every handoff, including local/published checkpoints, validation, blockers,
next steps, and authorization or validation gates. Explicitly require sufficient
context to continue without chat history, disclosure of unpublished records, and
rechecking historical host identifiers. Mirrored the maintenance rule in the
progress index and recorded accepted decision D006.

Validation: all 36 local Markdown links, including anchors across eight
documents, and `git diff --check` passed.
Documentation only; no executable or host state changed. The next development
step remains macOS 26 interactive runtime validation when a host is available,
or an explicit gate revision before a separately authorized design milestone.
No publication or merge is included in this task.

## 2026-11-03 — Start Milestone 1 and align roadmap numbering

Scope: user-authorized virtual workspace mapping and focusing. All previous
work is Milestone 0; earlier journal labels are historical, not new roadmap IDs.
Starting HEAD: `93a59ed`, clean tree on `feat/vspace-smoke` (three unpublished
documentation commits beyond `origin/feat/vspace-smoke` at `6359db7`). Inspected
all prior source/test changes, build/signing/CI infrastructure, development and
validation records, and native display/Space/window/focus APIs. Created focused
branch `feat/vspace-mapping` without rewriting history.

Updated agent scope, milestone checklists, progress overview, and decision D007.
The user's new instruction supersedes the former implementation gate; macOS 26
GUI evidence remains unavailable and is not claimed. No installed service,
configuration, or binary was changed. Validation: local Markdown links and
`git diff --check` passed before this documentation checkpoint. No executable
changes; build/runtime checks are unnecessary for this checkpoint.

Next: implement and test the independent manager and vspace commands. New CI
publication and interactive service changes require separate approval.

## 2026-11-03 — Implement the independent mapping manager

Scope: Milestone 1 manager foundation, following scope checkpoint `21a6e6f`.
Added native adapters and headless-testable mapping/focus policy, atomic startup
snapshot construction, conservative ordinary-window occupancy, global indices,
separate internal IDs, duplicate prevention, and session stability. Focus reuses
upstream and checks native activation. No upstream domain or layout code changed;
CLI/startup integration remains the next coherent unit.

Validation: `make` passed without warnings; 13/13 headless tests passed with the
five existing upstream CoreVideo warnings. Local Markdown links and whitespace
checks passed. Read-only inspection confirms SIP enabled, one original Homebrew
daemon (PID 9357), and only the earlier Milestone 0 CI run. Updated architecture,
validation, decision D008, milestones, and resume notes. No new daemon or service
change, publication, or GUI test occurred.

Next: wire startup and the strict vspace commands, extend dispatcher regression
tests, sign/verify the final binary, then prepare concrete CI/interactive approval.

## 2026-11-03 — Integrate virtual workspace commands and validate locally

Scope: finish Milestone 1 implementation after manager checkpoint `36e9ac8`.
Initialized the independent manager after native managers and before message/config
processing, with a nonfatal warning on failed enumeration. Added strict list,
current, and focus commands with documented JSON and established daemon failures;
retained exact ping and original domains. Review tightened unknown AX/untracked
window classification, including unusual levels, and added the all-empty case. A
read-only native probe then identified system UI on empty Spaces; applying
upstream’s ordinary-window tag predicate excludes it without dropping unknown
ordinary candidates.
Updated the command guide, architecture, development procedure, milestone status,
validation report, and resume instructions. No Milestone 2 work was started.

Validation: signed universal `make` build and strict/all-architecture verification
passed with the persistent certificate and unchanged identity. All 22 tests pass,
with the same five upstream CoreVideo warnings; arm64 Clang analysis has no
findings. Shell syntax, local Markdown links/anchors, and whitespace checks pass.
Native handlers/layouts and build/test/CI infrastructure are byte-for-byte
unchanged from `93a59ed`. The built CLI's read-only original-daemon queries pass
(2 displays, 4 Spaces, 5 windows); all three original installation hashes match.
The signed read-only native probe confirmed real native ordering, conservative
occupied mapping, empty spare Spaces, and list/current JSON without any daemon
or focus calls. Its first temporary harness omitted lookup-table initialization
and crashed; initializing those tables fixed the harness. Its next run exposed
system-UI false allocation, corrected with the native tag predicate and confirmed
by another probe run and unit cases.
The first baseline helper used an incorrect plist filename; directory inspection
and rerun completed the read-only baseline without changing host files.

No regression was detected within executed checks. No service or desktop changes,
new daemon, push, or merge occurred. New macOS 15/26 CI and interactive smoke
checks remain **unrun** pending explicit publication/service-pause approval.
macOS 26 GUI runtime still lacks a host. Mapping remains a startup snapshot;
unknown occupancy may conservatively allocate a visually empty Space.

Evidence and concrete next checks: [Milestone 1 validation](../validation.md#milestone-1-commands-and-local-validation--2026-11-03).
Next: request approval for the completed branch's publication/CI and a brief
interactive virtual-focus round trip with full original-service/focus recovery.

## 2026-11-04 — Correct the development animation value

Scope: repair the existing checkout-only development config discovered during
the approved Milestone 1 smoke test. The published production implementation
remains `6c4a354`. Upstream's parser accepts a float token for animation duration;
the old example supplied integer `0`, which emitted an error and stopped the
config script. Changed that one value to `0.0` and documented the requirement,
without changing native parser/domain behavior or user configuration.

Validation: shell syntax and whitespace checks passed. During the approved
repeat session, the development daemon reported animation duration `0.000000`,
layout `float`, and empty stderr. The virtual-focus round trip passed queries,
frame/layout comparisons, and user visual confirmation. Detailed CI/session and
recovery evidence follows in the validation-record task. The original service,
visible Space, focus, hashes, and SIP were verified after the session.
No executable source changed, so another build/headless run is unnecessary for
this config fix; the production SHA passed both CI jobs with 22 tests each.

This follow-up is local, beyond the explicitly published `6c4a354` checkpoint.
Next: finish the detailed validation report and refresh the handoff records.

## 2026-11-04 — Publish Milestone 1 and complete approved validation

Scope: the user explicitly approved publishing `feat/vspace-mapping` at `6c4a354`,
running CI, and a brief signed-development-daemon focus round trip with full
original-service/desktop recovery. Inspected the clean branch, unchanged reviewed
diff, remotes, current service, signature, SIP, native queries, and original file
hashes. Published only the approved branch; no merge or history rewrite occurred.

[CI run 41779098539](https://github.com/Stevenqx/yabai-v/actions/runs/41779098539)
passed at `6c4a354efb3a75c25906d48a49ad55c224fe175a`: macOS 15.8.2 and 26.6.2
arm64 runners each built/verified the universal binary and passed 22 tests and
static checks, with the same five upstream CoreVideo warnings.

Fresh host state had one display, occupied Space 3, and empty Spaces 7/592.
Focusing existing empty Space 7 before development startup tested empty-active
registration without creating Spaces or moving windows. The first attempt
passed initialization and errors, then native activation failed and Mission
Control was active; the user reported possible manual switching during it.
Restored and audited the original service after the attempt. An unchanged-native
control passed, and the idle repeat used local config fix `b195681` with no
stderr/config errors. Virtual focus 7 → 3 → 7 → 3 passed current/list/native
queries, stable mapping checks, and exact window-frame/association/layout
comparisons. The user confirmed the repeated switches worked visibly.

Both helpers retained generic recovery assertions; their precise cause was not
established. Separate final recovery audits completed the checks explicitly.
The successful repeat's final audit verified exactly one original Homebrew
daemon (PID 13873), visible/active Space 3, Orca window 6369 focused, all original
window frames/associations restored, all three original file hashes matching,
and SIP enabled. No original config/binary/plist was overwritten. A Computer Use
Dock inspection timed out and supplied no visual evidence; the user's repeat
confirmation is recorded distinctly from command/query results.

Updated milestones, overview/resume instructions, development guide, command
validation link, and detailed validation report. All local Markdown links/anchors,
shell syntax, and whitespace checks passed. These are records/procedure updates;
C source remains unchanged after the CI/GUI-tested checkpoint. The config fix and
this validation-record commit remain local/unpublished. Evidence and failed/skipped
conditions are preserved in the [latest report](../validation.md#milestone-1-publication-and-interactive-validation--2026-11-04).

Milestone 1's authorized scope and requested checks are complete. macOS 26 GUI
runtime remains unrun without a host. Next: review local follow-ups and obtain
publication authorization if desired, or validate on a supplied macOS 26 GUI host.
No Milestone 2 implementation, pull request, or merge was started.

## 2026-11-04 — Match native virtual-focus results

Scope: first M2-A unit, authorized by the user's create/native-focus task.
Started clean on `feat/vspace-mapping` at `8c5d0b3`, preserving M1 history.
Recorded active M2-A scope before implementation. Removed the extra focus polling
loop/timeout, retained virtual lookup and stale association checks, and preserved
native focus failure categories including scripting-addition errors. Updated
headless regressions and current command/architecture guidance; D009 supersedes
D008's polling policy. Historical validation reports remain unchanged.

Validation: universal `make` passed without warnings; 22/22 headless tests passed
with five existing CoreVideo warnings. Local Markdown links and whitespace passed.
These checks do not establish GUI switching. No daemon/service, installed files,
TCC, SIP, or remote branch was changed. M2-A allocation remains in progress; next
implement/revalidate focused-display spare allocation and record later proposals.

## 2026-11-04 — Allocate revalidated native spares and record M2 follow-ons

Scope: complete authorized M2-A implementation after focus checkpoint `86dabb5`.
Added strict `vspace --create`, guarded focused-display resolution, native desktop
and visibility checks, existing conservative occupancy reuse, fresh selection
revalidation, and append-only session IDs/global indices. Failures leave mappings
and allocation counters unchanged. Tightened unfamiliar AX role/subrole handling
to unknown. No native focus, placement, layout, or Space order changes occur on
creation. Updated CLI help, commands, architecture, development guide, and D010.
Recorded M2-B destroy/partial failure, M2-C all-display conservative refresh,
M2-D persistence, and native-identity/observed-focus MRU proposals under D011.
Later implementations remain unauthorized; no MRU seam was necessary.

Validation: final signed universal build and strict/all-architecture verification
passed with the persistent identity; 30/30 headless tests passed with five
existing CoreVideo warnings. Arm64 analyzer has no findings. Shell syntax,
local Markdown links/anchors and whitespace passed. Native handlers/layouts,
makefiles, and CI remain unchanged. Native read-only process-local mapping probes
passed one-display exhaustion and, after an external topology change, final-source
two-display focused-pool exhaustion without falling back to another display's
spare. These probes never start a daemon or focus/move anything. Original native
queries and all installation hashes passed; original service stayed running.

An early test-macro fixture compilation error and sandbox signing failure were
corrected; the final builds/tests/signature passed. An audit's nonexistent config
path was corrected from repository records. See
[detailed M2-A evidence](../validation.md#milestone-2-a-allocation-and-local-validation--2026-11-04).
New live daemon/focus/layout sessions and hosted CI are unrun: service-pause and
publication require approval. macOS 26 GUI still needs a supplied session. Next:
review local commits, obtain explicit live-session authorization, execute the
recorded recovery plan against fresh topology, and publish/run CI only if approved.
