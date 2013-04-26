
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
#	$NetBSD: mkconf,v 1.1.1.1 1997/07/24 21:20:12 christos Exp $
# $FreeBSD$
# mkconf
# Generate local configuration parameters for amd
#
if [ -e $1 ]; then
	eval `LC_ALL=C egrep '^[A-Z]+=' $1 | grep -v COPYRIGHT`
	OS=`echo ${TYPE} | LC_ALL=C tr 'A-Z' 'a-z'`
	echo '/* Define name and version of host machine (eg. solaris2.5.1) */'
	echo "#define HOST_OS \"${OS}${REVISION}\""
	echo '/* Define only name of host machine OS (eg. solaris2) */'
	echo "#define HOST_OS_NAME \"${OS}${REVISION}\"" \
		| sed -e 's/\.[-._0-9]*//'
	echo '/* Define only version of host machine (eg. 2.5.1) */'
	echo "#define HOST_OS_VERSION \"${REVISION}\""
else
cat << __NO_newvers_sh

/* Define name and version of host machine (eg. solaris2.5.1) */
#define HOST_OS "`uname -s | LC_ALL=C tr 'A-Z' 'a-z'``uname -r`"

/* Define only name of host machine OS (eg. solaris2) */
#define HOST_OS_NAME "`uname -s | LC_ALL=C tr 'A-Z' 'a-z'``uname -r | sed -e 's/\..*$//'`"

/* Define only version of host machine (eg. 2.5.1) */
#define HOST_OS_VERSION "`uname -r | sed -e 's/[-([:alpha:]].*//'`"

__NO_newvers_sh
fi

cat << __EOF

/* Define name of host */
#define BUILD_HOST "`hostname`"

/* Define user name */
#define BUILD_USER "`whoami`"

/* Define configuration date */
#define BUILD_DATE "`LC_ALL=C date`"

__EOF