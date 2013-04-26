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
CAPS="${1-Caps}"
cat <<EOF
/*
 * parametrized.h --- is a termcap capability parametrized?
 *
 * Note: this file is generated using MKparametrized.sh, do not edit by hand.
 * A value of -1 in the table means suppress both pad and % translations.
 * A value of 0 in the table means do pad but not % translations.
 * A value of 1 in the table means do both pad and % translations.
 */

static short const parametrized[] = {
EOF

# We detect whether % translations should be done by looking for #[0-9] in the
# description field.  We presently suppress padding translation only for the
# XENIX acs_* capabilities.  Maybe someday we'll dedicate a flag field for
# this, that would be cleaner....

${AWK-awk} <$CAPS '
$3 != "str"	{next;}
$1 ~ /^acs_/	{print "-1,\t/* ", $2, " */"; count++; next;}
$0 ~ /#[0-9]/	{print "1,\t/* ", $2, " */"; count++; next;}
		{print "0,\t/* ", $2, " */"; count++;}
END		{printf("} /* %d entries */;\n\n", count);}
'