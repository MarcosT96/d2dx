#!/bin/bash
# Build and run the portable d2dx unit tests in a container (podman or docker).
# Works the same on Linux and macOS. Always uses linux/amd64 because the code
# under test uses SSE2 intrinsics (emulated on Apple Silicon).
set -eu

repo="$(cd "$(dirname "$0")/../.." && pwd)"
image=d2dx-tests-linux

if command -v podman >/dev/null 2>&1; then
	engine=podman
elif command -v docker >/dev/null 2>&1; then
	engine=docker
else
	echo "error: need podman or docker installed" >&2
	exit 1
fi

# :Z relabels the bind mount for SELinux (Fedora); harmless elsewhere with podman,
# but docker on macOS doesn't understand it, so only add it for podman.
mount="$repo:/src"
if [ "$engine" = podman ]; then
	mount="$mount:Z"
fi

"$engine" build --platform linux/amd64 -t "$image" -f "$repo/tests/linux/Containerfile" "$repo/tests/linux"

"$engine" run --rm --platform linux/amd64 -v "$mount" "$image" bash -c '
	set -eu
	rm -rf build/linux-tests   # fresh configure: avoids a stale cached compiler
	cmake -S tests/linux -B build/linux-tests -G Ninja -DCMAKE_BUILD_TYPE=Debug
	cmake --build build/linux-tests
	ctest --test-dir build/linux-tests --output-on-failure -V
'
