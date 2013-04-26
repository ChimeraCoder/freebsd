#!/bin/sh -e
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

export PATH=/bin:/usr/bin

FS=': '				# internal field separator
LIBDEPENDS=./_libdeps		# intermediate output file
USRSRC=${1:-/usr/src}		# source root
LIBS="
	lib
	gnu/lib
	kerberos5/lib
	secure/lib
	usr.bin/lex/lib
	cddl/lib
"				# where to scan for libraries

# This sed(1) filter is used to convert -lfoo to path/to/libfoo.
#
SED_FILTER="
sed -E
    -e's; ;! ;g'
    -e's;$;!;'
    -e's;-lbsdxml!;lib/libexpat;g'
    -e's;-lbsdyml!;lib/libyaml;g'
    -e's;-lpthread!;lib/libthr;g'
    -e's;-lm!;lib/msun;g'
    -e's;-l(ncurses|termcap)!;lib/ncurses/ncurses;g'
    -e's;-l(gcc)!;gnu/lib/lib\1;g'
    -e's;-lssp_nonshared!;gnu/lib/libssp/libssp_nonshared;g'
    -e's;-l(asn1|hdb|kdc|heimbase|heimntlm|heimsqlite|hx509|krb5|roken|wind)!;kerberos5/lib/lib\1;g'
    -e's;-l(crypto|ssh|ssl)!;secure/lib/lib\1;g'
    -e's;-l([^!]+)!;lib/lib\1;g'
"

# Generate interdependencies between libraries.
#
genlibdepends()
{
	(
		cd ${USRSRC}
		find ${LIBS} -mindepth 1 -name Makefile |
		xargs grep -l 'bsd\.lib\.mk' |
		while read makefile; do
			libdir=$(dirname ${makefile})
			deps=$(
				cd ${libdir}
				make -m ${USRSRC}/share/mk -V LDADD
			)
			if [ "${deps}" ]; then
				echo ${libdir}"${FS}"$(
					echo ${deps} |
					eval ${SED_FILTER}
				)
			fi
		done
	)
}

main()
{
	if [ ! -f ${LIBDEPENDS} ]; then
		genlibdepends >${LIBDEPENDS}
	fi

	prebuild_libs=$(
		awk -F"${FS}" '{ print $2 }' ${LIBDEPENDS} |rs 0 1 |sort -u
	)
	echo "Libraries with dependents:"
	echo
	echo ${prebuild_libs} |
	rs 0 1
	echo

	echo "List of interdependencies:"
	echo
	for lib in ${prebuild_libs}; do
		grep "^${lib}${FS}" ${LIBDEPENDS} || true
	done |
	awk -F"${FS}" '{
		if ($2 in dependents)
			dependents[$2]=dependents[$2]" "$1
		else
			dependents[$2]=$1
	}
	END {
		for (lib in dependents)
			print dependents[lib]": " lib
	}' |
	sort

	exit 0
}

main