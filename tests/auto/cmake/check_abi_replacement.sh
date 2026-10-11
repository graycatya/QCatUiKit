#!/usr/bin/env bash
# SPDX-License-Identifier: Apache-2.0
set -euo pipefail
if (( $# < 3 )); then
    echo "Usage: $0 BASELINE_PREFIX CANDIDATE_PREFIX QT_PREFIX [WORK_DIR]" >&2
    exit 2
fi
qcat_baseline=$(realpath "$1")
qcat_candidate=$(realpath "$2")
qcat_qt=$(realpath "$3")
qcat_work=${4:-$(mktemp -d)}
qcat_source=$(cd "$(dirname "$0")/consumer" && pwd)
cmake -S "$qcat_source" -B "$qcat_work" -DCMAKE_PREFIX_PATH="$qcat_baseline;$qcat_qt"
cmake --build "$qcat_work"
qcat_library=$(find "$qcat_candidate" -name libQCatUiKit.so.1 -print -quit)
if [[ -z "$qcat_library" ]]; then echo "Candidate SONAME 1 library missing" >&2; exit 1; fi
qcat_libdir=$(dirname "$qcat_library")
qcat_output=$(LD_LIBRARY_PATH="$qcat_libdir:$qcat_qt/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}" "$qcat_work/consumer" 2>&1)
printf '%s\n' "$qcat_output"
if [[ "$qcat_output" != *"$qcat_libdir/"* ]]; then
    echo "Consumer did not load the candidate library" >&2
    exit 1
fi
