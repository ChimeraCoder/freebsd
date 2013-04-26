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
PATH=/bin:/sbin:/usr/bin:/usr/sbin; export PATH

#
# purge cat? directories
#
do_purge()
{
	echo "purge $@" 2>&1
	echo "not implemented yet\n" 2>&1
}


#
# Uncompress one page
#
uncompress_page()
{
	local	pname
	local	fname
	local	sect
	local	ext

	# break up file name
	pname=$1
	IFS='.' ; set $pname
	# less than 3 fields - don't know what to do with this
	if [ $# -lt 3 ] ; then 
		IFS=" 	" ; echo ignoring $pname 1>&2 ; return 0 ; 
	fi
	# construct name and section
	fname=$1 ; shift
	while [ $# -gt 2 ] ; do
		fname=$fname.$1
		shift
	done
	sect=$1
	ext=$2

	IFS=" 	"
	case "$ext" in
	gz|Z) 	{ 
		IFS=" 	" ; set `file $pname`
		if [ $2 != "gzip" ] ; then 
			echo moving hard link $pname 1>&2
			mv $pname $fname.$ext	# link
		else
			if [ $2 != "symbolic" ] ; then
				echo gunzipping page $pname 1>&2
				temp=`mktemp -t manager` || exit 1
				gunzip -c $pname > $temp
				chmod u+w $pname
				cp $temp $pname
				chmod 444 $pname
				mv $pname $fname.$sect
				rm -f $temp
			else
				# skip symlinks - this can be
				# a program like expn, which is
				# its own man page !
				echo skipping symlink $pname 1>&2
			fi
		fi };;
	*)	{
		IFS=" 	"
		echo skipping file $pname 1>&2
		} ;;
	esac
	# reset IFS - this is important!
	IFS=" 	"
}


#
# Uncompress manpages in paths
#
do_uncompress()
{
	local	i
	local	dir
	local	workdir

	workdir=`pwd`
	while [ $# != 0 ] ; do
		if [ -d $1 ] ; then
			dir=$1
			cd $dir
			for i in * ; do
				case $i in
				*cat?)	;; # ignore cat directories
				*)	{
					if [ -d $i ] ; then 
						do_uncompress $i
					else
						if [ -e $i ] ; then
							uncompress_page $i
						fi
					fi } ;;
				esac
			done
			cd $workdir
		else
			echo "directory $1 not found" 1>&2
		fi
		shift
	done
}

#
# Remove .so's from one file
#
so_purge_page()
{
 	local	so_entries
	local	lines
	local	fname

	so_entries=`grep "^\.so" $1 | wc -l`
	if [ $so_entries -eq 0 ] ; then return 0 ; fi

	# we have a page with a .so in it
	echo $1 contains a .so entry 2>&1
	
	# now check how many lines in the file
	lines=`wc -l < $1`

	# if the file is only one line long, we can replace it
	# with a hard link!
	if [ $lines -eq 1 ] ; then
		fname=$1;
		echo replacing $fname with a hard link
		set `cat $fname`;
		rm -f $fname
		ln ../$2 $fname
	else
		echo inlining page $fname 1>&2
		temp=`mktemp -t manager` || exit 1
		cat $fname | \
		(cd .. ; soelim ) > $temp
		chmod u+w $fname
		cp $temp $fname
		chmod 444 $fname
		rm -f $temp
	fi
}

#
# Remove .so entries from man pages
#	If a page consists of just one line with a .so,
#	replace it with a hard link
#
remove_so()
{
	local	pname
	local	fname
	local	sect

	# break up file name
	pname=$1
	IFS='.' ; set $pname
	if [ $# -lt 2 ] ; then 
		IFS=" 	" ; echo ignoring $pname 1>&2 ; return 0 ; 
	fi
	# construct name and section
	fname=$1 ; shift
	while [ $# -gt 1 ] ; do
		fname=$fname.$1
		shift
	done
	sect=$1

	IFS=" 	"
	case "$sect" in
	gz) 	{ echo file $pname already gzipped 1>&2 ; } ;;
	Z)	{ echo file $pname already compressed 1>&2 ; } ;;
	[12345678ln]*){
		IFS=" 	" ; set `file $pname`
		if [ $2 = "gzip" ] ; then 
			echo moving hard link $pname 1>&2
			mv $pname $pname.gz	# link
		else
			if [ $2 != "symbolic" ] ; then
				echo "removing .so's in  page $pname" 1>&2
				so_purge_page $pname
			else
				# skip symlink - this can be
				# a program like expn, which is
				# its own man page !
				echo skipping symlink $pname 1>&2
			fi
		fi };;
	*)	{
		IFS=" 	"
		echo skipping file $pname 1>&2
		} ;;
	esac
	# reset IFS - this is important!
	IFS=" 	"
}


#
# compress one page
#	We need to watch out for hard links here.
#
compress_page()
{
	local	pname
	local	fname
	local	sect

	# break up file name
	pname=$1
	IFS='.' ; set $pname
	if [ $# -lt 2 ] ; then 
		IFS=" 	" ; echo ignoring $pname 1>&2 ; return 0 ; 
	fi
	# construct name and section
	fname=$1 ; shift
	while [ $# -gt 1 ] ; do
		fname=$fname.$1
		shift
	done
	sect=$1

	IFS=" 	"
	case "$sect" in
	gz) 	{ echo file $pname already gzipped 1>&2 ; } ;;
	Z)	{ echo file $pname already compressed 1>&2 ; } ;;
	[12345678ln]*){
		IFS=" 	" ; set `file $pname`
		if [ $2 = "gzip" ] ; then 
			echo moving hard link $pname 1>&2
			mv $pname $pname.gz	# link
		else
			if [ $2 != "symbolic" ] ; then
				echo gzipping page $pname 1>&2
				temp=`mktemp -t manager` || exit 1
				cat $pname | \
				(cd .. ; soelim )| gzip -c -- > $temp
				chmod u+w $pname
				cp $temp $pname
				chmod 444 $pname
				mv $pname $pname.gz
				rm -f $temp
			else
				# skip symlink - this can be
				# a program like expn, which is
				# its own man page !
				echo skipping symlink $pname 1>&2
			fi
		fi };;
	*)	{
		IFS=" 	"
		echo skipping file $pname 1>&2
		} ;;
	esac
	# reset IFS - this is important!
	IFS=" 	"
}

#
# Compress man pages in paths
#
do_compress_so()
{
	local	i
	local	dir
	local	workdir
	local	what

	what=$1
	shift
	workdir=`pwd`
	while [ $# != 0 ] ; do
		if [ -d $1 ] ; then
			dir=$1
			cd $dir
			for i in * ; do
				case $i in
				*cat?)	;; # ignore cat directories
				*)	{
					if [ -d $i ] ; then 
						do_compress_so $what $i
					else 
						if [ -e $i ] ; then
							$what $i
						fi
					fi } ;;
				esac
			done
			cd $workdir
		else
			echo "directory $1 not found" 1>&2
		fi
		shift
	done
}

#
# Display a usage message
#
ctl_usage()
{
	echo "usage: $1 -compress <path> ... " 1>&2
	echo "       $1 -uncompress <path> ... " 1>&2
	echo "       $1 -purge <days> <path> ... " 1>&2
	echo "       $1 -purge expire <path> ... " 1>&2
	exit 1
}

#
# remove .so's and do compress
#
do_compress()
{
	# First remove all so's from the pages to be compressed
	do_compress_so remove_so "$@"
	# now do ahead and compress the pages
	do_compress_so compress_page "$@"
}

#
# dispatch options
#
if [ $# -lt 2 ] ; then ctl_usage $0 ; fi ;

case "$1" in
	-compress)	shift ; do_compress "$@" ;;
	-uncompress)	shift ; do_uncompress "$@" ;;
	-purge)		shift ; do_purge "$@" ;;
	*)		ctl_usage $0 ;;
esac