#!/bin/sh
set -eu

cd "$(dirname "$0")/.."

identity=${YABAI_FORK_SIGN_IDENTITY:-yabai-fork Development}

make
codesign --force --sign "$identity" --identifier com.yabai-fork.development \
    --timestamp=none ./bin/yabai
codesign --verify --strict --all-architectures --verbose=2 ./bin/yabai
