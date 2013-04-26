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

D=.depend			# default dependency file is .depend
append=0
pflag=

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
			pflag=p
			shift ;;
		*)
			break ;;
	esac
done

case $# in 0) 
	echo 'usage: mkdep [-ap] [-f file] [flags] file ...' >&2
	exit 1;;
esac

TMP=_mkdep$$
trap 'rm -f $TMP ; trap 2 ; kill -2 $$' 1 2 3 13 15
trap 'rm -f $TMP' 0

# For C sources, mkdep must use exactly the same cpp and predefined flags
# as the compiler would.  This is easily arranged by letting the compiler
# pick the cpp.  mkdep must be told the cpp to use for exceptional cases.
CC=${CC-"cc"}
MKDEP_CPP=${MKDEP_CPP-"${CC} -E"}
MKDEP_CPP_OPTS=${MKDEP_CPP_OPTS-"-M"};

echo "# $@" > $TMP	# store arguments for debugging

if $MKDEP_CPP $MKDEP_CPP_OPTS "$@" >> $TMP; then :
else
	echo 'mkdep: compile failed' >&2
	exit 1
fi

case x$pflag in
	x) case $append in 
		0) sed -e 's; \./; ;g' < $TMP >  $D;;
		*) sed -e 's; \./; ;g' < $TMP >> $D;;
	   esac
	;;	
	*) case $append in 
		0) sed -e 's;\.o:;:;' -e 's; \./; ;g' < $TMP >  $D;;
		*) sed -e 's;\.o:;:;' -e 's; \./; ;g' < $TMP >> $D;;
	   esac
	;;
esac

exit $?