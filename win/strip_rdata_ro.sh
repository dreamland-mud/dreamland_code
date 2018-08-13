#!/bin/sh

# This script is used to fetch dlls from .libs/ and place 'em
# together, while striping debugging symbols and `ro' bit from `.rdata'
# section, making dynamic pseudo relocations in wine (and probably other
# platforms) possible.

if test $# != 2; then
    echo "Usage: $0 <src_start_dir> <dst_dir>"
    exit 1
fi

find $1 -name '*.dll' -or -name '*.exe' |
while read i; do
    j="$2/`basename "$i"`"
    echo "Processing $i -> $j"
    i586-pc-mingw32-objcopy -S --rename-section .rdata=.rrdata,CONTENTS,ALLOC,LOAD,DATA $i $j || exit 2
done

