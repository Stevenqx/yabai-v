# Development workflow

The GitHub fork is `Stevenqx/yabai-v`. The working checkout for this setup is
`/Users/steven/Workspace/Github/yabai-v`; the originally preferred location is
`~/Developer/yabai-fork`. Build and run binaries from the checkout; do not replace
a Homebrew binary or install a development service. See `architecture.md` for
scope and `validation.md` for observed results.

Read the [development progress index](progress/README.md) for current milestones,
decisions, and the task journal. Maintain those records as described in `AGENTS.md`.

## Repository setup

With Git and the GitHub CLI authenticated, create the fork and clone its complete
history (no `--depth` or source archives):

```sh
gh repo fork asmvik/yabai --fork-name yabai-v --clone=false
mkdir -p ~/Developer
git clone https://github.com/YOUR_ACCOUNT/yabai-v.git ~/Developer/yabai-fork
cd ~/Developer/yabai-fork
git remote add upstream https://github.com/asmvik/yabai.git
git fetch upstream --tags
git remote -v
git rev-parse --is-shallow-repository  # false
git switch -c feat/vspace-smoke upstream/master
git rev-parse upstream/master        # record the full base SHA
```

If the fork already exists, inspect it instead of repeating creation. If the
checkout was initially cloned from upstream, rename that remote to `upstream`
and add the fork URL as `origin`. This does not publish local commits. Push only
after reviewing the diff and receiving explicit confirmation.

To relocate this existing checkout after finishing the current workspace session,
first import its prepared commits and stop any foreground development daemon.
Run the following manually only if the destination does not already exist:

```sh
if test ! -e ~/Developer/yabai-fork; then
    mkdir -p ~/Developer &&
        mv /Users/steven/Workspace/Github/yabai-v ~/Developer/yabai-fork
fi
```

Move the complete directory, including `.git`, to retain history and commits.
Use the new path for later builds and re-enable Accessibility for that binary if
macOS requests it. This move does not relocate the Homebrew installation.

For later updates, start with a clean working tree, fetch `upstream`, and rebase
unpublished feature commits onto `upstream/master`. Coordinate published history
instead of rewriting it automatically. Use small Conventional Commits, such as
`feat(vspace): add ping command` and `ci: check macOS 15 and 26 builds`.

## Toolchain and upstream baseline

```sh
sw_vers
uname -m
xcode-select -p
xcrun --find clang
xcrun clang --version
xcrun --sdk macosx --show-sdk-version
csrutil status
```

Xcode Command Line Tools are sufficient for the command-line build. Full Xcode
is optional; the Swift GUI project is deliberately deferred. If tools are
missing, install them manually with `xcode-select --install`.

Before modifying upstream, inspect `makefile`, `src/manifest.m`, and
`tests/makefile`, then run `make` and record the result and base SHA. The default
build is a universal x86_64/arm64 debug binary at `bin/yabai`, with `-g -O0`,
warnings, and the existing macOS deployment target. It uses Apple frameworks and
the existing embedded payload sources. No scripting addition is installed or
loaded by the build. Do not run `make clean` unnecessarily: upstream's target
also removes generated payload sources. The ordinary `make` already cleans the
binary directory and recompiles the single manifest translation unit.

## Persistent local signing

Create one local certificate manually in Keychain Access: Certificate Assistant
→ Create a Certificate → name `yabai-fork Development`, identity type
`Self Signed Root`, certificate type `Code Signing`. In that certificate's trust
settings, trust it for code signing. Use the login keychain and retain the same
certificate and private key across builds; do not export them into the repository.

```sh
security find-identity -v -p codesigning
./scripts/build-dev.sh
codesign --verify --strict --verbose=2 ./bin/yabai
codesign -d --verbose=4 -r- ./bin/yabai
./bin/yabai --version
```

The helper uses a fixed development identifier and defaults to this certificate.
Set `YABAI_FORK_SIGN_IDENTITY` to its SHA-1 identity if duplicate names exist.
Keep the checkout path, certificate, and identifier stable so Accessibility
authorization can survive rebuilds. Move to the preferred checkout location
before granting permissions. Recreate the certificate only deliberately.

For build-only CI, `YABAI_FORK_SIGN_IDENTITY=- ./scripts/build-dev.sh` explicitly
selects ad-hoc signing. That verifies executable integrity, but it does not
establish persistent local signing or a stable Accessibility identity.

## Tests and static checks

```sh
make -C tests
git diff --check
```

The vspace tests exercise the production message dispatcher without initializing
WindowServer managers: exact ping JSON and error handling for unsupported input.
The existing upstream tests run alongside them. These checks require neither
Accessibility permissions nor a running daemon. GitHub Actions repeats the build,
ad-hoc signature verification, and tests on `macos-15` and `macos-26`; hosted
runners do not validate an interactive window-management session. Runner labels
are documented in the [GitHub runner images catalog](https://github.com/actions/runner-images).

## Manual daemon smoke test

First inspect the existing service and confirm its executable, configuration,
and current state. Never stop or replace it without explicit approval. If it is
running, defer the real daemon test until an approved test session is available.
Do not run two window-managing daemons against the same desktop, even if their
sockets differ. Prefer a separate macOS login session for manual testing.

Sign the development binary and grant that exact binary Accessibility permission
in System Settings → Privacy & Security → Accessibility. Keep SIP enabled and
do not load the scripting addition. Screen Recording is unnecessary for this
test because the development config disables animations.

Once no competing daemon is running, launch in a terminal from the checkout:

```sh
USER=yabai-fork-dev ./bin/yabai --verbose --config "$PWD/examples/yabairc.dev"
```

This selects dedicated socket/lock names and an explicit development config.
It does not install a service. In another terminal in the same checkout:

```sh
USER=yabai-fork-dev ./bin/yabai -m vspace --ping
# {"status":"ok","component":"vspace"}
USER=yabai-fork-dev ./bin/yabai -m query --displays
USER=yabai-fork-dev ./bin/yabai -m query --spaces
USER=yabai-fork-dev ./bin/yabai -m query --windows
```

Check that the queries return JSON arrays. Check that `vspace`,
`vspace --unknown`, and `vspace --ping extra` exit unsuccessfully. If a disposable
window and a safe GUI session are available, query its ID and use
`USER=yabai-fork-dev ./bin/yabai -m window --focus ID`. Replace `ID` with the
integer `id` from the query output before running the command; verify focus
visually and restore the original focus afterward.
Switch between existing Spaces with Mission Control or the system shortcuts and
query `--spaces` again. Optionally test `space --focus INDEX` on an existing
Space: this upstream base has a gesture fallback that does not require the
scripting addition. Verify the visible result and query it again, then restore
the original Space. Do not create/delete native Spaces or load the scripting
addition; do not substitute `space --switch`, whose implementation differs.

Stop only the foreground development process with Ctrl-C. An existing Homebrew
binary, service plist, and user configuration must remain intact. If the original
service was stopped during an approved test, restore its prior state explicitly
and verify its queries; do not leave recovery to an automated cleanup guess.

## Debugging

Use the signed `bin/yabai` and its default debug symbols:

```sh
USER=yabai-fork-dev lldb -- ./bin/yabai --verbose --config "$PWD/examples/yabairc.dev"
# (lldb) breakpoint set --name handle_domain_vspace
# (lldb) run
```

The same no-competing-daemon rule applies under LLDB. macOS may require a manual
Developer Tools authorization or prevent debugging a particular signed process;
use foreground verbose logs if access is denied. Never change SIP to debug.
After the ordinary tests pass, `make asan` is available for targeted memory-error
investigation; it rebuilds `bin`, so sign the result again before a GUI session.
