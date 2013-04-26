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
# A utility to ensure that INSTALL lists the correct versions of the
# tools used to generate the distfile.
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
# check_tool readme_file prog_name verbose_name
#
#   Executes 'prog_name' to determine its version and checks if the
#   given 'readme_file' contains 'verbose_name <version>' in it.
#
check_tool() {
    readme=${1}
    prog=${2}
    name=${3}

    ver=$(${prog} --version | head -n 1 | cut -d ' ' -f 4)

    if grep "\\* ${name} ${ver}" ${readme} >/dev/null; then
        true
    else
        warn "Incorrect version of ${name}"
        false
    fi
}

#
# main readme_file
#
# Entry point.
#
main() {
    readme=${1}
    ret=0

    check_tool ${readme} autoconf "GNU autoconf" || ret=1
    check_tool ${readme} automake "GNU automake" || ret=1
    check_tool ${readme} libtool "GNU libtool" || ret=1

    return ${ret}
}

main "${@}"

# vim: syntax=sh:expandtab:shiftwidth=4:softtabstop=4