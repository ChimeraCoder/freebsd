#! /bin/sh
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

#
# A utility to sanity check the coding style of all source files in the
# project tree.
#

Prog_Name=${0##*/}

#
# err message
#
err() {
    echo "${Prog_Name}: ${@}" 1>&2
    exit 1
}

#
# warn message
#
warn() {
    echo "${Prog_Name}: ${@}" 1>&2
}

#
# guess_topdir
#
# Locates the project's top directory and prints its path.  The caller
# must verify if the result is correct or not.
#
guess_topdir() {
    olddir=$(pwd)
    topdir=$(pwd)
    while [ ${topdir} != / ]; do
        if [ -f ./atf-c.h ]; then
            break
        else
            cd ..
            topdir=$(pwd)
        fi
    done
    cd ${olddir}
    echo ${topdir}
}

#
# find_sources
#
# Locates all source files within the project, relative to the current
# directory, and prints their paths.
#
find_sources() {
    find . \( -name "AUTHORS" -o \
              -name "COPYING" -o \
              -name "ChangeLog" -o \
              -name "NEWS" -o \
              -name "README" -o \
              -name "TODO" -o \
              -name "*.[0-9]" -o \
              -name "*.ac" -o \
              -name "*.at" -o \
              -name "*.awk" -o \
              -name "*.c" -o \
              -name "*.cpp" -o \
              -name "*.h" -o \
              -name "*.h.in" -o \
              -name "*.hpp" -o \
              -name "*.m4" -o \
              -name "*.sh" \
           \) -a \( \
              \! -path "*/atf-[0-9]*" -a \
              \! -path "*autom4te*" -a \
              -type f -a \
              \! -name "aclocal.m4" \
              \! -name "bconfig.h" \
              \! -name "defs.h" \
              \! -name "libtool.m4" \
              \! -name "ltoptions.m4" \
              \! -name "ltsugar.m4" \
              \! -name "lt~obsolete.m4" \
              \! -name "*.so.*" \
           \)
}

#
# guess_formats file
#
# Guesses the formats applicable to the given file and prints the resulting
# list.
#
guess_formats() {
    case ${1} in
        */ltmain.sh)
            ;;
        *.[0-9])
            echo common man
            ;;
        *.c|*.h)
            echo common c
            ;;
        *.cpp|*.hpp)
            echo common cpp
            ;;
        *.sh)
            echo common shell
            ;;
        *)
            echo common
            ;;
    esac
}

#
# check_file file
#
# Checks the validity of the given file.
#
check_file() {
    err=0
    for format in $(guess_formats ${1}); do
        awk -f ${topdir}/admin/check-style-${format}.awk ${1} || err=1
    done

    return ${err}
}

#
# main [file list]
#
# Entry point.
#
main() {
    topdir=$(guess_topdir)
    if [ ! -f ${topdir}/atf-c.h ]; then
        err "Could not locate the project's top directory"
    fi

    if [ ${#} -gt 0 ]; then
        sources=${@}
    else
        cd ${topdir}
        sources=$(find_sources)
    fi

    ok=0
    for file in ${sources}; do
        file=$(echo ${file} | sed -e "s,\\./,,")

        if [ ! -f ${file} ]; then
            err "Could not open ${file}"
        else
            check_file ${file} || ok=1
        fi
    done

    return ${ok}
}

main "${@}"

# vim: syntax=sh:expandtab:shiftwidth=4:softtabstop=4