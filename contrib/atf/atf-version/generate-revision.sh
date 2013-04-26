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
# Generates a header file with information about the revision used to
# build ATF.
#

set -e

Prog_Name=${0##*/}

GIT=
ROOT=

#
# err message
#
err() {
    echo "${Prog_Name}: ${@}" 1>&2
    exit 1
}

#
# call_git args
#
call_git() {
    ( cd "${ROOT}" && "${GIT}" "${@}" )
}

#
# generate_from_dist revfile version
#
generate_from_dist() {
    revfile=${1}; shift
    version=${1}; shift

    >${revfile}

    echo "#define PACKAGE_REVISION_TYPE_DIST" >>${revfile}
}

#
# generate_from_git revfile
#
generate_from_git() {
    revfile=${1}

    rev_base_id=$(call_git rev-parse HEAD)
    rev_branch=$(call_git branch | grep '^\* ' | cut -d ' ' -f 2-)
    rev_date=$(call_git log -1 | grep '^Date:' | sed -e 's,^Date:[ \t]*,,')
    if [ -z "$(call_git status -s)" ]; then
        rev_modified=false
    else
        rev_modified=true
    fi

    >${revfile}

    echo "#define PACKAGE_REVISION_TYPE_GIT" >>${revfile}

    echo "#define PACKAGE_REVISION_BRANCH \"${rev_branch}\"" >>${revfile}
    echo "#define PACKAGE_REVISION_BASE \"${rev_base_id}\"" >>${revfile}

    if [ ${rev_modified} = true ]; then
        echo "#define PACKAGE_REVISION_MODIFIED 1" >>${revfile}
    fi

    echo "#define PACKAGE_REVISION_DATE \"${rev_date}\"" >>${revfile}
}

#
# main
#
# Entry point.
#
main() {
    outfile=
    version=
    while getopts :g:r:o:v: arg; do
        case ${arg} in
            g)
                GIT=${OPTARG}
                ;;
            o)
                outfile=${OPTARG}
                ;;
            r)
                ROOT=${OPTARG}
                ;;
            v)
                version=${OPTARG}
                ;;
            *)
                err "Unknown option ${arg}"
                ;;
        esac
    done
    [ -n "${ROOT}" ] || \
        err "Must specify the top-level source directory with -r"
    [ -n "${outfile}" ] || \
        err "Must specify an output file with -o"
    [ -n "${version}" ] || \
        err "Must specify a version number with -v"

    if [ -n "${GIT}" -a -d ${ROOT}/.git ]; then
        generate_from_git ${outfile}
    else
        generate_from_dist ${outfile} ${version}
    fi
}

main "${@}"

# vim: syntax=sh:expandtab:shiftwidth=4:softtabstop=4