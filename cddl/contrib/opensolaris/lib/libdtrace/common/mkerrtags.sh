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
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.\n\
 * Use is subject to license terms.\n\
 */\n\
\n\
#pragma ident\t\"%Z%%M%\t%I%\t%E% SMI\"\n\
\n\
#include <dt_errtags.h>
\n\
static const char *const _dt_errtags[] = {"

pattern='^	\(D_[A-Z0-9_]*\),*'
replace='	"\1",'

sed -n "s/$pattern/$replace/p" || exit 1

echo ${BSDECHO} "\
};\n\
\n\
static const int _dt_ntag = sizeof (_dt_errtags) / sizeof (_dt_errtags[0]);\n\
\n\
const char *
dt_errtag(dt_errtag_t tag)
{
	return (_dt_errtags[(tag > 0 && tag < _dt_ntag) ? tag : 0]);
}"

exit 0