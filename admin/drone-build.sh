#!/bin/sh
# Drone build for the live server: run from the source clone after `git pull`.
#
# Full path (autotools + configure + fresh moc sources) only when the build
# system itself changed since the last successful build, or the generated
# build files have drifted. Otherwise plain `make`: object dependencies
# already track .cpp and headers, and skipping autotools keeps Makefile mtimes
# stable, so make no longer rebuilds every moc source and the relinks behind
# them.
#
# Force a full build: put [full-build] in a commit message, or delete
# $OBJS/.drone-last-built.
set -e

SRC=$(pwd)
OBJS=${OBJS:-/home/dreamland/objs}
PREFIX=${PREFIX:-/home/dreamland/runtime}
STAMP=$OBJS/.drone-last-built
HEAD=$(git rev-parse HEAD)

# A Makefile.in older than its Makefile.am or the shared Makefile.inc would
# make automake regenerate it inside `make -j 2`, across ~100 directories at
# once (the am--refresh storm of 2026-10-04). A hand checkout can bump those
# mtimes without changing content.
drifted() {
    [ ! -f configure ] && return 0
    for f in configure.ac aclocal.m4 acinclude.m4; do
        [ "$f" -nt configure ] && return 0
    done
    [ configure -nt "$OBJS/config.status" ] && return 0
    for am in $(git ls-files '*Makefile.am'); do
        in=${am%.am}.in
        [ ! -f "$in" ] || [ "$am" -nt "$in" ] && return 0
        [ plug-ins/Makefile.inc -nt "$in" ] || [ src/Makefile.inc -nt "$in" ] && return 0
    done
    return 1
}

why=""
if [ ! -f "$STAMP" ] || [ ! -f "$OBJS/config.status" ]; then
    why="no previous build stamp or config.status"
elif ! grep -q "^prefix = $PREFIX\$" "$OBJS/Makefile"; then
    why="objs was configured with another prefix"
else
    prev=$(cat "$STAMP")
    if ! git cat-file -e "$prev^{commit}" 2>/dev/null; then
        why="last built commit $prev not in history"
    elif git log --format=%B "$prev..HEAD" | grep -qF '[full-build]'; then
        why="[full-build] in a commit message"
    elif drifted; then
        why="generated build files older than their sources"
    else
        # Inputs of Makefile.git and configure: automake/autoconf sources, the
        # _MOC lists moc_edit.pl reads from Makefile.am, and the moc compiler.
        # No pipefail: the status is tr's, so a grep with no match is fine.
        changed=$(git diff --name-only "$prev" HEAD \
            | grep -E '(^|/)(Makefile\.am|Makefile\.inc|Makefile\.git|configure\.ac|[^/]*\.m4|[^/]*\.m4\.in)$|^admin/|^src/moc/' \
            | head -5 | tr '\n' ' ')
        [ -z "$changed" ] || why="build files changed: $changed"
    fi
fi

# moc bakes every ancestor's XML nodes into each derived class's generated
# source, but the make rule only depends on the library's own headers. A base
# class in another library gaining or losing a node would leave its
# descendants stale, so any XML_ line change in a header regenerates all moc
# output. A dropped _MOC header also leaves a stale source behind, hence the
# wipe on every full build. Blind spot: a multi-line XML_VARIABLE whose
# continuation line alone changes; use [full-build].
wipe_moc=""
if [ -n "$why" ]; then
    wipe_moc=1
elif git diff -U0 "$prev" HEAD -- '*.h' | grep -qE '^[-+].*\bXML_(VARIABLE|OBJECT)\b'; then
    wipe_moc=1
fi

if [ -n "$why" ]; then
    echo "*** FULL build: $why"
    make -f Makefile.git
    cd "$OBJS"
    "$SRC/configure" --prefix="$PREFIX"
else
    echo "*** INCREMENTAL build: no build-system change since $prev"
    cd "$OBJS"
fi

if [ -n "$wipe_moc" ]; then
    echo "*** regenerating all moc sources"
    find "$OBJS" -name '*_moc_xml.cpp' -delete
fi

make -j 2
make install
echo "$HEAD" > "$STAMP"
