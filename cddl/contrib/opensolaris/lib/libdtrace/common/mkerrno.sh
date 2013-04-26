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

echo "\
/*\n\
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.\n\
 * Use is subject to license terms.\n\
 */\n\
\n\
#pragma ident\t\"%Z%%M%\t%I%\t%E% SMI\"\n"

pattern='^#define[	 ]\(E[A-Z0-9]*\)[	 ]*\([A-Z0-9]*\).*$'
replace='inline int \1 = \2;@#pragma D binding "1.0" \1'

sed -n "s/$pattern/$replace/p" | tr '@' '\n'