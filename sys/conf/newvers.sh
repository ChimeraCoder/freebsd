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

TYPE="FreeBSD"
REVISION="10.0"
BRANCH="CURRENT"
if [ "X${BRANCH_OVERRIDE}" != "X" ]; then
	BRANCH=${BRANCH_OVERRIDE}
fi
RELEASE="${REVISION}-${BRANCH}"
VERSION="${TYPE} ${RELEASE}"
SYSDIR=$(dirname $0)/..

if [ "X${PARAMFILE}" != "X" ]; then
	RELDATE=$(awk '/__FreeBSD_version.*propagated to newvers/ {print $3}' \
		${PARAMFILE})
else
	RELDATE=$(awk '/__FreeBSD_version.*propagated to newvers/ {print $3}' \
		${SYSDIR}/sys/param.h)
fi

b=share/examples/etc/bsd-style-copyright
year=`date '+%Y'`
# look for copyright template
for bsd_copyright in ../$b ../../$b ../../../$b /usr/src/$b /usr/$b
do
	if [ -r "$bsd_copyright" ]; then
		COPYRIGHT=`sed \
		    -e "s/\[year\]/1992-$year/" \
		    -e 's/\[your name here\]\.* /The FreeBSD Project./' \
		    -e 's/\[your name\]\.*/The FreeBSD Project./' \
		    -e '/\[id for your version control system, if any\]/d' \
		    $bsd_copyright` 
		break
	fi
done

# no copyright found, use a dummy
if [ X"$COPYRIGHT" = X ]; then
	COPYRIGHT="/*-
 * Copyright (c) 1992-$year The FreeBSD Project.
 * All rights reserved.
 *
 */"
fi

# add newline
COPYRIGHT="$COPYRIGHT
"

LC_ALL=C; export LC_ALL
if [ ! -r version ]
then
	echo 0 > version
fi

touch version
v=`cat version` u=${USER:-root} d=`pwd` h=${HOSTNAME:-`hostname`} t=`date`
i=`${MAKE:-make} -V KERN_IDENT`
compiler_v=$($(${MAKE:-make} -V CC) -v 2>&1 | grep 'version')

for dir in /bin /usr/bin /usr/local/bin; do
	if [ -x "${dir}/svnversion" ] && [ -z ${svnversion} ] ; then
		svnversion=${dir}/svnversion
	fi
	if [ -x "${dir}/p4" ] && [ -z ${p4_cmd} ] ; then
		p4_cmd=${dir}/p4
	fi
done
if [ -d "${SYSDIR}/../.git" ] ; then
	for dir in /bin /usr/bin /usr/local/bin; do
		if [ -x "${dir}/git" ] ; then
			git_cmd="${dir}/git --git-dir=${SYSDIR}/../.git"
			break
		fi
	done
fi

if [ -n "$svnversion" ] ; then
	svn=`cd ${SYSDIR} && $svnversion`
	case "$svn" in
	[0-9]*)	svn=" r${svn}" ;;
	*)	unset svn ;;
	esac
fi

if [ -n "$git_cmd" ] ; then
	git=`$git_cmd rev-parse --verify --short HEAD 2>/dev/null`
	svn=`$git_cmd svn find-rev $git 2>/dev/null`
	if [ -n "$svn" ] ; then
		svn=" r${svn}"
		git="=${git}"
	else
		svn=`$git_cmd log | fgrep 'git-svn-id:' | head -1 | \
		     sed -n 's/^.*@\([0-9][0-9]*\).*$/\1/p'`
		if [ -n $svn ] ; then
			svn=" r${svn}"
			git="+${git}"
		else
			git=" ${git}"
		fi
	fi
	if $git_cmd --work-tree=${SYSDIR}/.. diff-index \
	    --name-only HEAD | read dummy; then
		git="${git}-dirty"
	fi
fi

if [ -n "$p4_cmd" ] ; then
	p4version=`cd ${SYSDIR} && $p4_cmd changes -m1 "./...#have" 2>&1 | \
		awk '{ print $2 }'`
	case "$p4version" in
	[0-9]*)
		p4version=" ${p4version}"
		p4opened=`cd ${SYSDIR} && $p4_cmd opened ./... 2>&1`
		case "$p4opened" in
		File*) ;;
		//*)	p4version="${p4version}+edit" ;;
		esac
		;;
	*)	unset p4version ;;
	esac
fi
	

cat << EOF > vers.c
$COPYRIGHT
#define SCCSSTR "@(#)${VERSION} #${v}${svn}${git}${p4version}: ${t}"
#define VERSTR "${VERSION} #${v}${svn}${git}${p4version}: ${t}\\n    ${u}@${h}:${d}\\n"
#define RELSTR "${RELEASE}"

char sccs[sizeof(SCCSSTR) > 128 ? sizeof(SCCSSTR) : 128] = SCCSSTR;
char version[sizeof(VERSTR) > 256 ? sizeof(VERSTR) : 256] = VERSTR;
char compiler_version[] = "${compiler_v}";
char ostype[] = "${TYPE}";
char osrelease[sizeof(RELSTR) > 32 ? sizeof(RELSTR) : 32] = RELSTR;
int osreldate = ${RELDATE};
char kern_ident[] = "${i}";
EOF

echo $((v + 1)) > version