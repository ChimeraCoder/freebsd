#!/bin/sh
# You may redistribute this program and/or modify it under the terms of
# the GNU General Public License as published by the Free Software Foundation,
# either version 3 of the License, or (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

BSDECHO=-e

echo ${BSDECHO} "\
/*\n\
 * Copyright 2005 Sun Microsystems, Inc.  All rights reserved.\n\
 * Use is subject to license terms.\n\
 */\n\
\n\
#pragma ident\t\"%Z%%M%\t%I%\t%E% SMI\"\n\
\n\
#include <dtrace.h>\n\
\n\
/*ARGSUSED*/
const char *\n\
dtrace_subrstr(dtrace_hdl_t *dtp, int subr)\n\
{\n\
	switch (subr) {"

nawk '
/^#define[ 	]*DIF_SUBR_/ && $2 != "DIF_SUBR_MAX" {
	printf("\tcase %s: return (\"%s\");\n", $2, tolower(substr($2, 10)));
}'

echo ${BSDECHO} "\
	default: return (\"unknown\");\n\
	}\n\
}"