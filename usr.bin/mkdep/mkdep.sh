#!/bin/sh -
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

PATH=/bin:/usr/bin:/usr/ucb:/usr/old/bin
export PATH

D=.depend			# default dependency file is .depend
append=0

while :
	do case "$1" in
		# -a appends to the depend file
		-a)
			append=1
			shift ;;

		# -f allows you to select a makefile name
		-f)
			D=$2
			shift; shift ;;

		# the -p flag produces "program: program.c" style dependencies
		# so .o's don't get produced
		-p)
			SED='s;\.o ; ;'
			shift ;;
		*)
			break ;;
	esac
done

if [ $# = 0 ] ; then
	echo 'usage: mkdep [-p] [-f depend_file] [cc_flags] file ...'
	exit 1
fi

TMP=/tmp/mkdep$$

trap 'rm -f $TMP ; trap 2 ; kill -2 $$' 1 2 3 13 15

cc -M $* |
sed "
	s; \./; ;g
	/\.c:$/d
	$SED" |
awk '{
	if ($1 != prev) {
		if (rec != "")
			print rec;
		rec = $0;
		prev = $1;
	}
	else {
		if (length(rec $2) > 78) {
			print rec;
			rec = $0;
		}
		else
			rec = rec " " $2
	}
}
END {
	print rec
}' > $TMP

if [ $? != 0 ]; then
	echo 'mkdep: compile failed.'
	rm -f $TMP
	exit 1
fi

if [ $append = 1 ]; then
	cat $TMP >> $D
	rm -f $TMP
else
	mv $TMP $D
fi
exit 0