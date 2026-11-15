#!/bin/sh
# Formats the C and C++ sources with clang-format, using .clang-format
#   ./format-files.sh                  format src/ and include/
#   ./format-files.sh src/camera       format only these paths
#   ./format-files.sh --check [paths]  list the files that would change, change nothing
# Set CLANG_FORMAT to pick the binary; otherwise clang-format on PATH, then ./clang-format.exe

check=0
if [ "$1" = "--check" ]; then
	check=1
	shift
fi
if [ $# -eq 0 ]; then
	set -- src include
fi

CLANG_FORMAT=${CLANG_FORMAT:-clang-format}
if ! command -v "$CLANG_FORMAT" >/dev/null 2>&1; then
	CLANG_FORMAT=./clang-format.exe
fi
if [ ! -x "$CLANG_FORMAT" ] && ! command -v "$CLANG_FORMAT" >/dev/null 2>&1; then
	echo "clang-format not found; put it on PATH, set CLANG_FORMAT, or place clang-format.exe here" >&2
	exit 1
fi

# Files listed in .clang-format-ignore are skipped here too, since clang-format before 18 ignores that file
ignored=$(mktemp)
trap 'rm -f "$ignored"' EXIT
if [ -f .clang-format-ignore ]; then
	grep -v -e '^#' -e '^[[:space:]]*$' .clang-format-ignore | tr -d '\r' >"$ignored"
fi
files() {
	find "$@" -type f \( -iname "*.h" -o -iname "*.hpp" -o -iname "*.c" -o -iname "*.cp" -o -iname "*.cpp" -o -iname "*.inc" \) |
		sed 's|^\./||' | grep -vxF -f "$ignored"
}

if [ $check -eq 1 ]; then
	bad=0
	for f in $(files "$@"); do
		if ! "$CLANG_FORMAT" --dry-run -Werror "$f" >/dev/null 2>&1; then
			echo "$f"
			bad=$((bad + 1))
		fi
	done
	echo "$bad file(s) need formatting" >&2
	[ $bad -eq 0 ]
	exit
fi

files "$@" | xargs "$CLANG_FORMAT" -i
