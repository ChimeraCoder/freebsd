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
# $FreeBSD$
usage()
{
	echo "usage: genassym [-o outfile] objfile"
	exit 1
}


work()
{
	${NM:='nm'} "$1" | ${AWK:='awk'} '
	/ C .*sign$/ {
		sign = substr($1, length($1) - 3, 4)
		sub("^0*", "", sign)
		if (sign != "")
			sign = "-"
	}
	/ C .*w0$/ {
		w0 = substr($1, length($1) - 3, 4)
	}
	/ C .*w1$/ {
		w1 = substr($1, length($1) - 3, 4)
	}
	/ C .*w2$/ {
		w2 = substr($1, length($1) - 3, 4)
	}
	/ C .*w3$/ {
		w3 = substr($1, length($1) - 3, 4)
		w = w3 w2 w1 w0
		sub("^0*", "", w)
		if (w == "")
			w = "0"
		sub("w3$", "", $3)
		# This still has minor problems representing INT_MIN, etc. 
		# E.g.,
		# with 32-bit 2''s complement ints, this prints -0x80000000,
		# which has the wrong type (unsigned int).
		printf("#define\t%s\t%s0x%s\n", $3, sign, w)
	} '
}


#
#MAIN PROGGRAM
#
use_outfile="no"
while getopts "o:" option
do
	case "$option" in
	o)	outfile="$OPTARG"
		use_outfile="yes";;
	*)	usage;;
	esac
done
shift $(($OPTIND - 1))
case $# in
1)	;;
*)	usage;;
esac

if [ "$use_outfile" = "yes" ]
then
	work $1  3>"$outfile" >&3 3>&-
else
	work $1
fi