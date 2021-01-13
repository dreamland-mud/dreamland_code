#!/bin/sh -
##
# $Id: newvers.sh,v 1.1.2.5 2005/09/16 13:10:17 rufina Exp $
#
# rufina, 2004
#
# Based on fbsd,
#
#	@(#)newvers.sh	8.1 (Berkeley) 4/20/94
# $FreeBSD: src/sys/conf/newvers.sh,v 1.44.2.23 2002/05/15 16:03:58 murray Exp $

TYPE="DreamLand"
RELEASE="4.0"
VERSION="${TYPE} ${RELEASE}"

year=`date '+%Y'`

COPYRIGHT="
/* \$Id\$
 *
 * ruffina, $year
 */

"

LC_ALL=C; export LC_ALL
if [ ! -r version ]
then
	echo 0 > version
fi

touch version
v=`cat version` u=${USER-root} d=`pwd` h=`hostname` t=`date`
cat << EOF > version.cpp
$COPYRIGHT
#include "dreamland.h"

const DLString DreamLand::version = "${VERSION} #${v}: ${t}\\r\\n    ${u}@${h}:${d}\\r\\n";
EOF

echo `expr ${v} + 1` > version
