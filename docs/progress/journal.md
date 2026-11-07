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

## 2026-11-04 — Run approved M2-A live allocation; defer focus while locked

Scope: the user approved the planned Homebrew pause, sole signed-checkout daemon
session, and original-service/desktop recovery. Started clean at `5b199fb` on
`feat/vspace-mapping`; no source or installed file changed. Recorded active
validation scope, verified persistent signature and original hashes, and captured
fresh two-display topology and recovery state. No publication was authorized.

Real daemon/IPC checks passed ping/list/current, native queries, invalid indices,
extra arguments, and allocation on focused display 2: mapped 592 as ID/index 3,
then failed without mutation while display 1 retained spare 7. Existing mapping
bytes/order, focus, visibility, native order, frames, and layouts were unchanged.
The native cross-display focus control returned zero but was not observed; the
helper stopped before virtual comparison/second-display allocation/layout smoke.
Both restarted daemons lacked AX window/focus metadata. Console inspection then
established that the screen was locked; this cannot validate visible focus or
establish a fork regression. Added a lock-state guard to the temporary helper.

The development daemon stopped and original service restarted. Independent
recovery audit passed original executable, one daemon, native Space visibility/
focus/order, all original window frames/associations/layouts, all three original
hashes, and SIP. Window focus is unverified while locked (expected 5943, queried
0). Manual unlock was requested; existing live-session authorization remains in
force. No automated unlock, TCC reset, trust change, SA, or SIP operation ran.

The preflight helper initially read only signature stderr and exited without
service changes; corrected combined stdout/stderr check passed. Two temporary
console-parser attempts were corrected for pipe/root-dictionary handling before
the successful read-only lock observation. Full results and limits are retained
in [approved GUI validation](../validation.md#milestone-2-a-approved-gui-validation--2026-11-04).
Docs links/anchors and whitespace passed; no build/headless rerun is necessary
for these documentation-only records. Next: after manual unlock, re-audit focus
recovery and finish the approved GUI sequence. New CI/publication remains gated.

## 2026-11-04 — Complete approved unlocked M2-A GUI validation

Scope: continue under the user's existing service-pause approval after manual
unlock and a subsequent idle-session confirmation. Checkout `157bae2` was clean;
production source remains `5b199fb`, preserving native-focus checkpoint `86dabb5`.
The host's display/Space/window state had changed externally since the locked
attempt, so rechecked AX metadata and captured a fresh recovery baseline. The
first unlocked attempt was interrupted by focus/window lifecycle changes and is
retained separately; no closed/transient windows were recreated for recovery.

Controlled evidence `iaus8mt4` passed CLI regressions, creation of hidden 7 then
592, display-2 exhaustion despite other-display spares, unchanged mappings/focus/
frames/layout/order on allocation, native/virtual same-display and visible
cross-display focus, and matching already-focused errors. A native hidden-target
call returned zero and made the target visible without global focus; the helper's
extra activation assertion was too strong for the requested native semantics.
Focused completion `chsprfea` passed result-parity and empty-view float/BSP/stack
checks. The subsequent virtual call globally focused the already-visible empty
target; target visibility differed between those two controls and is explicitly
recorded. The user confirmed visible switching and return. No native-path change
was made to compensate for its observed SIP focus behavior.

Both development sessions stopped their sole daemon and restored the original
service. Startup of the original service changed two surviving floating frames;
restored their captured positions/sizes using native commands after PID/association
checks. One constrained width needed a second pass after position restoration.
The final independent audit in `chsprfea` passed all conditions against the first
idle baseline: one original Homebrew daemon, native topology/order/visible and
active Spaces, original Orca focus, every original frame/association, layouts,
hashes, and SIP. Helper failures and the initial frame audit remain retained.

Updated validation, milestone state, development guidance, and resume notes.
Markdown links/anchors and whitespace passed. No executable behavior changed;
the existing signed universal build and 30/30 headless results still apply, and
no rebuild was necessary for this documentation unit. New hosted macOS 15/26 CI
needs publication approval; macOS 26 GUI, a standalone single-display GUI repeat,
and populated layout smoke remain unrun. No push, merge, installed-file overwrite,
TCC reset, SA, SIP change, or follow-on implementation occurred. Next: review
local commits and publish/run the exact-SHA matrix only when separately approved.

## 2026-11-04 — Maintain a portable vspace validation script

Scope: the user requested a maintained automatic/interactive test script for
their own Macs and other OS versions later, then chose to focus ongoing checks
on macOS 15 and defer macOS 26. Started clean at `4f637d7` on
`feat/vspace-mapping`, ahead of `origin/feat/vspace-mapping` by four commits.
Read agent/development/architecture/validation/progress records and inspected
the build, tests, CI, and temporary M2-A GUI helper before implementing.

Added `scripts/test-vspace.py`: default headless build/sign/checks, read-only
daemon contract/query checks, and typed opt-in interactive allocation/focus/empty
layout stages. Reused all 30 production fixtures and promoted the available GUI
procedures without host-specific IDs or service management. Reports contain OS/
SDK/source/binary metadata, individual results, commands, and live recovery
snapshots. Focus comparisons restore all displays' visibility and global focus
between controls; native return status and observed/visual activation remain
separate. Unknown public-query occupancy remains a candidate observation, not
proof of emptiness. New mappings persist for the daemon session. Recovery covers
the running development daemon; original-service preparation/recovery stays manual.

Added 14 focused runner regressions, including full fake-daemon orchestration,
append/exhaustion, identical focus setup, already-focused with one mapping,
interruption recovery, changed window lifetimes, layout-command failure recovery,
and nonterminal refusal. Updated development maintenance/run instructions,
compatibility notes, decision D012, and resume/milestone records. Default CI now
runs this same script on macOS 15, with macOS 26 opt-in for a later manual run;
updated the earlier AGENTS.md dual-OS gate to match the user's latest instruction.
No production C/Objective-C source changed after `5b199fb`.

Passed on SIP-enabled macOS 15.6 arm64: full persistent-signed universal build
through the runner, signature/version/help checks, 30/30 production tests and the
initial runner suite. After runner refinements, final `headless --skip-build`
passed all 30 production and 14 runner tests, strict signature/architectures and
static checks. The full production build had no warnings; the test compile kept
five existing CoreVideo deprecations. All 14 runner tests also passed with Apple
Python 3.9.6 (default Python 3.15). Help, invalid infinite observation budget,
workflow YAML/wiring, Markdown links/anchors, and whitespace passed. Initial
sandbox signing/trust failures are retained separately; retries under the
established signing workflow passed with no trust/TCC changes. Detailed reports
and exact temporary paths are in [runner validation](../validation.md#standard-vspace-test-runner--2026-11-04).

Not run: new runner query/interactive modes against a real daemon, hosted CI, or
macOS 26. Fake-daemon results are not live GUI evidence. Earlier M2-A runtime and
recovery evidence remains separate. No service, desktop, user configuration,
installed binary, SIP, or scripting-addition state changed. Create the focused
local checkpoint; no publication/merge is authorized. Next: use this runner for
future macOS 15 work and prepared interactive sessions; use the same script on
other OS versions later when supplied, without treating deferred tests as passes.

## 2026-11-05 — Implement explicit M2-B evacuation and release

Scope: implement `tmp/yabai-v-m2b-codex-prompt.md` from clean `cf20e36` on
`feat/vspace-mapping`. Verified completed M2-A/runner ancestry, existing diff,
agent/progress/development/architecture/validation records and actual native
move/focus/event paths. Recorded active M2-B authorization before implementation;
M2-C/D/MRU, publication and installed-service changes remain outside scope.

Added manager-owned destroy with frozen source/target/replacement identities,
conservative native window preflight, existing native move/layout dispatch,
observed membership/layout reconciliation, and deferred event-loop completion.
Retain the source on partial failure or uncertainty and report stable mappings,
observed moved windows and remaining failure. Native visible-Space handoff and
final global focus must be observed before release; background window focus uses
retained lifetime checks. Survivor IDs/indices/order and fresh-ID append creation
are preserved. Ordinary `--focus` keeps native result semantics when no destroy
is pending. CLI/help, classification/limitations and accepted decision D013
record the lifecycle contract and future persistence release seam.

Extended production regressions to 43 and runner regressions to 16. Tests cover
partial/retry and late failures, arrival/disconnect/lifetime races, sparse and
interleaved ordering, visible/focused cross-display policy, supported native
window classes and real deferred socket ownership. Maintained runner query
rejections and added opt-in hidden-empty allocation release, with a prepared
manual populated-window validation procedure. Fixture macro compile errors and
four test-only format warnings were corrected during development.

Passed persistent-signed universal build, strict signature/architecture/help,
43/43 production and 16/16 runner tests, shell/whitespace checks on SIP-enabled
macOS 15.6 arm64/SDK 15.5. Arm64 analysis had zero diagnostics. A guarded read-only
native probe passed uncertain-occupancy rejection without mutations; native
empty release/reuse was skipped for lack of an eligible focused-display spare.
Detailed reports and final verification are in
[M2-B validation](../validation.md#milestone-2-b-evacuation-and-release--2026-11-05).

The original Homebrew daemon was inspected read-only and remains untouched.
No second daemon, service operation, existing-user-window movement, TCC/trust/SIP
change, SA load or publication ran. M2-B live moves/populated layouts/minimized/
hidden windows and physical visibility/focus handoff remain unrun; M2-A GUI
results remain separate. New hosted macOS 15 CI and macOS 26 are unrun/deferred.
The coherent implementation/tests/records form the next local checkpoint.
Next: prepare disposable Spaces/windows and obtain explicit service-pause approval
before interactive validation.

Final reconciliation found `origin/feat/vspace-mapping` already at `cf20e36`,
with local reflog `update by push`; corrected the stale resume claim that the
M2-A/runner baseline remained unpublished. No network refresh/push was performed,
and no baseline/new hosted CI status is inferred. Final full rebuild/sign and
43/16 tests passed after AX identity hardening; test compile has only the five
existing upstream warnings. Apple Python 3.9.6 runner tests, zero-finding analysis,
Markdown links/anchors and whitespace also passed. New M2-B work remains local.

## 2026-11-05 — Complete approved M2-B GUI validation

Scope: the user approved pausing Homebrew yabai, running the persistently signed
checkout as the sole daemon, testing prepared disposable windows/existing Spaces,
and restoring/auditing the original session. Started at clean production
`ca48ac8`; only milestone/progress documentation changed. Read progress,
architecture/development, decisions and current validation, then checked fresh
service/topology/console/signature state. The interruption occurred before the
first pause; rechecked processes and native state after the user's continuation.
The user manually supplied empty desktop Spaces and an idle unlocked session.

Prepared temporary AppKit fixture windows, a raw WindowServer membership/owner/
bounds probe and a recovery harness outside Git. The first `oxsvv6qz` attempt
stopped before lifecycle commands because cached window frames disagreed after
initialization. Original service/focus/visibility/order/associations/layouts/
hashes/SIP recovered; cached frames did not match. A managed-window absolute
resize request was rejected, with no resize performed. No fixture was launched.

At the user's request, investigated padding without changing it. Global/per-Space
values stayed 10 pixels. Native work-area calculation exactly yields Zotero's
current `(10,45,1332,823)` frame from the 1352×878 display and 35-pixel menu bar;
its older cached size matches the other display's padded dimensions. Chrome's
native height differed from the intended padded target, with no proven cause.
Tracked-window JSON uses an event-updated frame cache. The first attempt had no
pre-pause native bounds, so it cannot prove physical changes. The user explicitly
accepted an independently measured baseline for the repeat; recorded D014 and
updated validation guidance instead of forcing potentially stale cached frames.

Measured repeat `chyw7zi8` passed empty release/reuse and mixed BSP evacuation,
then stopped at the harness's incorrect expected BSP stack indices. The retained
snapshot showed correct BSP split nodes/geometry/native membership; fixed only
the harness assertion (BSP singleton stack indices are zero). Every recovery
condition, including independent native frames, passed. Production source stayed
unchanged. Both initial attempts remain separate from the complete pass.

Complete `mhi73ua9` run passed maintained-runner query/negative CLI mode; empty
hidden release/reuse with survivor gap 4/fresh IDs; same-target rejection; mixed
tiled/floating/minimized window evacuation into BSP, stack and float; populated
layout bookkeeping; application-hidden windows; other-display visibility handoff
with original global Space/window focus; and explicit populated cross-display
movement with a surviving mapped source-display replacement. Four disposable
windows stayed open through each operation, then closed only at teardown. All
original native window frames/owners/memberships were invariant throughout.

Focused `pqaohqqa` follow-up passed same-display and visible cross-display
`vspace --focus` round trips, current/already-focused outcomes and unchanged
mappings/native geometry. A separate final audit against `mhi73ua9`'s pre-pause
native baseline passed every condition: one original service (historical PID
12255), executable, topology/native order, visibility/focus, all original window
lifetimes/native frames/memberships, matching cached frames, original stack
layouts/all 10-pixel padding/gaps, original binary/config/plist hashes, SIP and
no development fixture. The user confirmed switching and return looked correct.

No production code changed, so the existing signed universal build and 43/43
production/16/16 runner tests still apply; no rebuild or test rerun was necessary
for this documentation unit. Markdown links/anchors and whitespace checked
before the local documentation commit. Full temporary evidence and explicit
limits are in [M2-B GUI validation](../validation.md#milestone-2-b-approved-gui-validation--2026-11-05).
No TCC/Keychain trust, installed binary/config/service definition, SA or SIP change,
push or merge occurred. User-created native desktops remain. Physical single-
display checks, unsafe fault injection and macOS 26 remain unrun/headless-only;
new hosted macOS 15 CI requires separate publication approval. Next: hand off the
validated local M2-B work; later milestone implementation remains unauthorized.

## 2026-11-05 — Implement M2-C manual refresh

The user invoked `tmp/yabai-v-m2c-codex-prompt.md`. Verified clean
`feat/vspace-mapping` at `e23bd17`, descendant of completed M2-A/M2-B; cached
published checkpoint remains `cf20e36`. Read development/progress/architecture,
D009–D014 and the latest M2-B GUI evidence. Recorded the authorized task before
implementation; M2-D/MRU and broader display reconciliation remain proposals.

Added manager-owned explicit refresh with complete display/Space/visibility/focus
snapshots, conservative occupancy/identity checks and candidate revalidation.
Protect every display's visible Space/global focus, retain uncertain/disconnected/
stale mappings with reasons, preserve at least one mapping, and reject pending
destroy before mutation. A separate candidate array filters authoritative global
order and publishes removals/`1..N` numbering together; survivor IDs/native mappings/
display association/relative order and `next_id` survive. Native state, layouts,
focus and recency paths are unchanged. Recorded accepted D015 and preserved
future observed-native-focus MRU guidance.

Extended isolated CLI handling/help with `--refresh` and JSON release/index-change/
retention results, keeping list semantics. Added deterministic safety regressions
and standard-runner `interactive --refresh-mappings` opt-in with report/invariant/
repeat checks. No default command or timer refreshes metadata. Updated CLI,
architecture, development, milestones and resume records together.

Passed on macOS 15.6 arm64/SDK 15.5/SIP enabled: persistent-signed universal `make`
build, strict signature, 52/52 production tests, 19/19 runner regressions on Python
3.15 and Apple's 3.9.6, zero-diagnostic arm64 analysis, shell/Markdown link/anchor
and whitespace checks. Source build has no warnings; tests retain five upstream
CoreVideo deprecations. Initial fixture macro/shadowing/release-order errors were
corrected before the final complete pass. Sandbox signing/verification limitations
were resolved by approved execution with the existing trusted identity, without
changing trust/TCC. Final report: `/private/tmp/yabai-v-m2c-validated/report.json`.

Read-only native-query probe with isolated hypothetical mappings on two physical
displays/six native Spaces released two hidden empty local mappings, retained
uncertain/visible/non-desktop mappings, passed idempotence, and preserved native
topology/order/visibility/global focus with zero guarded mutation calls. No daemon
or tracked AX window manager was initialized. Original-service read-only native
query regressions passed with the new client. These are distinct from M2-C real
daemon/IPC/GUI evidence; see [full M2-C validation](../validation.md#milestone-2-c-refresh-and-local-validation--2026-11-05).

The original Homebrew service remains running; no pause/second daemon, installed
file/configuration/service definition, TCC/trust, scripting addition or SIP change,
push or merge occurred. Real M2-C GUI and physical single-display checks are unrun;
new service approval is needed, and earlier completed M2-B approval is historical.
Hosted macOS 15 CI awaits authorized publication; macOS 26 remains deferred under
D012. The coherent change is recorded in the local feature commit containing this
entry, with only this task's implementation/tests/docs staged.
Next: obtain applicable approval for a measured sole-daemon refresh GUI session,
verify release/compaction/create/focus, then independently audit original recovery.

## 2026-11-05 — Validate M2-C in the approved GUI session

The user approved the prepared Homebrew pause/sole signed checkout daemon/test/
restore plan. Verified source `31b3035`, signature/hash, unlocked macOS 15.6/SIP
session, original executable/service and current two-display native topology.
Recorded the active follow-up before testing. Captured independent WindowServer
bounds, owners and exact native memberships for all five original windows before
the first pause, alongside cached frames, configuration/layout/padding/gaps and
installation hashes. Daemon source and signed binary stayed unchanged throughout.

The first `xxa84kya` attempt exposed a runner preflight bug before refresh: native
`focus_follows_mouse` queries return `disabled`, although the setter accepts `off`.
Corrected only the Python runner's expected value; mouse-follows-focus remains
`off`, and enabled mouse-focus modes remain rejected. Added realistic preflight
regressions and updated the guide. All 21 runner tests passed with Python 3.15 and
Apple's 3.9.6. The standard headless `--skip-build` run passed the unchanged signed
universal binary, 52 production and 21 runner tests, help/syntax/whitespace. The
previous full build and zero-finding analysis remain applicable to unchanged C.
Report: `/private/tmp/yabai-v-m2c-gui-runner-fix-headless/report.json`.

Core `jungx6qq` and repeated `9otq8z8c` checks passed maintained read-only query and
real interactive refresh modes, empty destroy/recreate gaps, release of only
hidden empty ID 5/native 871, preservation of background-visible ID 6/native 867,
index 6 → 4 compaction with stable identity/order/mapping, diagnostics for real
uncertain occupancy at 848, idempotence, fresh-ID appended create, compact/current/
fresh focus and native/virtual already-focused parity. Per-refresh native state
and original independent window bounds/memberships stayed invariant.

Those two attempts stopped later at fixture preconditions: AppKit initially
placed its window on the primary display; after explicit fixture-only placement,
the source remained visible on the other display and correctly returned `visible`
rather than the harness's expected `occupied`. Recorded both partial attempts
without relabeling them. The focused `5tf4x6i3` follow-up hid the source correctly,
then passed ordinary/floating/minimized/application-hidden occupancy one live
window at a time, exact native membership and preserved flags/frames. Each case
returned `occupied`; closing the last fixture window permitted explicit release,
and repeat refresh was unchanged. This completion had no test/recovery errors.

The user had not watched and explicitly requested another visual repeat. Primary
Space 3 → 7 → 3 switching passed in `bga1388z`; its secondary activation was
unobserved despite dispatch success, so that attempt stopped/recovered. The
focused `r63fdz3z` repeat logged first-dispatch visibility of 867 while global
focus remained at 3, then a separate request focused the now-visible target and
completed return through 848 to 3. Eight-/six-second viewing pauses were used.
The user confirmed: “Yes, switching and return looked correct.” Recorded that
feedback separately from the maintained runner's earlier `unobserved` answer.
No stronger native SIP focus guarantee or daemon-code fix is claimed.

Every original-service recovery passed, including original stack layouts and all
10-pixel padding/gaps. After the watched repeat, an independent read-only audit
against the **very first pre-pause native baseline** passed every service/topology/
visibility/global/window-focus/lifetime/native-owner/membership/state/frame/hash/
SIP condition, with matching cached frames and no fixture/development daemon.
Evidence: `/private/tmp/yabai-v-m2c-gui.r63fdz3z/independent-final-recovery-audit.json`
and `user-visual-confirmation.json`; the final original PID 48377 is historical.
The original installation/configuration/plist/TCC/trust/SA/SIP were preserved.

Updated validation, CLI/development guidance, milestones and resume notes. Checked
Markdown links/anchors and staged whitespace before the focused local follow-up
commit. No push/merge was authorized or performed. Physical single-display GUI,
simultaneously empty visible Spaces on both displays, unsafe live fault injection,
new hosted CI and macOS 26 remain headless-only/unrun/deferred as specified in the
[full M2-C GUI report](../validation.md#milestone-2-c-approved-gui-validation--2026-11-05).
No more service action is pending; next work needs its applicable scope/publication
or service authorization. Persistence, native MRU and broader topology work remain
proposals, not newly authorized implementation.

## 2026-11-07 — Implement M2-D persistence and validated restoration

Scope: the user's `tmp/yabai-v-m2d-codex-prompt.md`, from clean `dbe2cde` on
`feat/vspace-mapping`. Read M2-C handoff, development/architecture/validation,
D009–D015, lifecycle release seams and actual UUID/configuration/startup/lock code.
Recorded the active milestone before implementation; accepted D016 for this scope.

Added an isolated bounded version-1 JSON state module, textually included in the
existing manifest, reusing Cocoa JSON facilities. Decimal strings preserve uint64
IDs/generation/allocator state; global array order and strict positive indices
preserve destroy gaps. State lives under XDG state home or `~/.local/state/yabai-v`,
separate from configuration/socket names. Private lifetime `flock` ownership
serializes writers; malformed supported state is preserved with original bytes,
while future/read/lock/preservation/allocation/startup-save failures use diagnosed
session-only fallback without replacing protected state. Checked candidate
allocation prevents partial publication; abandoned private temp files are removed
only after exclusive ownership.

Restore temporary manager state using unique native Space/display UUIDs plus
current desktop ownership/type. Runtime numbers are hints, not lifetime proof.
Unmatched/ambiguous records remain unresolved and reserve logical positions/IDs
without owning native mappings. Every display's visible empty desktop is included;
new required native desktops append fresh IDs after all verified saved records.
Focus/current/spare/destroy/refresh paths respect availability, and unresolved
records cannot substitute for the last usable native mapping. No native/layout
handler or MRU observation path changed.

Create, refresh and final observed destroy release persist complete candidates
before publishing. Synced temp plus atomic replacement defines the commit point;
a later directory-sync failure publishes that generation and diagnoses uncertainty.
Pre-replacement destroy save failures retain source metadata and report already
moved windows for retry. Focus alone never writes. Added availability/persistence
fields to existing queries/help and updated CLI, architecture, development, state
schema/recovery, milestone and resume records.

Passed: persistent-signed universal build and strict all-architecture verification,
68 production tests, 24 runner tests on Python 3.15 and Apple's 3.9.6, zero-finding
arm64 analysis, shell syntax, Markdown links/anchors and whitespace. Full final
report `/private/tmp/yabai-v-m2d-final/report.json`; post-review runner filtering
was rechecked with the unchanged binary/explicit skipped rebuild in
`/private/tmp/yabai-v-m2d-final-checked/report.json`. Initial sandbox signing and
native-query restrictions were resolved through approved execution outside the
sandbox with the existing identity; trust/TCC stayed unchanged. Early fixture
syntax/visibility assumptions were corrected before the full passes. Final review
fixed repeated unresolved-hint appends under native ambiguity and runner focus
selection when unresolved/available records share a numeric native hint.

Fixtures cover real temporary files, restart/order/gaps/high-water behavior,
synthetic display/Space identity changes, malformed/oversized/future recovery,
writer contention, allocation/permission/partial ENOSPC/pre/post replacement
failures, forked crash boundaries, stale-temp cleanup and mid-evacuation restart/
retry. Native read-only production UUID probes passed on one physical display;
original-service native display/Space/window CLI queries passed and one yabai
process was observed. These are separate from actual M2-D daemon/restart/GUI or
hardware/reboot validation; see [full evidence](../validation.md#milestone-2-d-persistence-and-local-validation--2026-11-07).

No service/installed file/configuration, TCC/trust, scripting addition or SIP
change, competing daemon, push or merge occurred. Real M2-D restart/GUI needs a
new explicitly approved sole-daemon session with D014's independent recovery
baseline. Physical reconnect/Dock/recreation/reboot, actual full-volume/power-loss,
hosted macOS 15 CI and deferred macOS 26 remain unrun. Local implementation/tests/
records are committed together in the feature commit containing this entry;
publication remains unauthorized. Next: prepare the isolated-state measured GUI
restart checks for review/approval, then test and audit original-session recovery.
Native MRU and later product/native lifecycle work remain proposals.
