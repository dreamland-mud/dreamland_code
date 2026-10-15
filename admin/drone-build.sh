#!/bin/sh
# Drone build for the live server: run from the source clone after `git pull`.
#
# Full path (autotools + configure + fresh moc sources) only when the build
# system itself changed since the last successful build. Otherwise plain
# `make`: object and moc dependencies already track .cpp and _MOC headers.
# Skipping autotools keeps Makefile mtimes stable, which is what lets make
# rebuild only what changed instead of every moc source and the relinks
# behind them.
#
# Force a full build: put [full-build] in a commit message, or delete
# $OBJS/.drone-last-built.
set -e

SRC=$(pwd)
OBJS=${OBJS:-/home/dreamland/objs}
PREFIX=${PREFIX:-/home/dreamland/runtime}
STAMP=$OBJS/.drone-last-built
HEAD=$(git rev-parse HEAD)

why=""
if [ ! -f "$STAMP" ] || [ ! -f "$OBJS/config.status" ]; then
    why="no previous build stamp or config.status"
else
    prev=$(cat "$STAMP")
    if ! git cat-file -e "$prev^{commit}" 2>/dev/null; then
        why="last built commit $prev not in history"
    elif git log --format=%B "$prev..HEAD" | grep -qF '[full-build]'; then
        why="[full-build] in a commit message"
    else
        # Inputs of Makefile.git and configure: automake/autoconf sources, the
        # _MOC lists moc_edit.pl reads from Makefile.am, and the moc compiler.
        changed=$(git diff --name-only "$prev" HEAD \
            | grep -E '(^|/)(Makefile\.am|Makefile\.inc|Makefile\.git|configure\.ac|[^/]*\.m4|[^/]*\.m4\.in)$|^admin/|^src/moc/' \
            | head -5 | tr '\n' ' ')
        [ -z "$changed" ] || why="build files changed: $changed"
    fi
fi

if [ -n "$why" ]; then
    echo "*** FULL build: $why"
    make -f Makefile.git
    cd "$OBJS"
    "$SRC/configure" --prefix="$PREFIX"
    # A header dropped from a _MOC list leaves its generated source behind, and
    # that source does not depend on the Makefile. Start moc output clean.
    rm -f plug-ins/*/lib*cpp plug-ins/*/*/lib*cpp
else
    echo "*** INCREMENTAL build: no build-system change since $prev"
    cd "$OBJS"
fi

make -j 2
make install
echo "$HEAD" > "$STAMP"
