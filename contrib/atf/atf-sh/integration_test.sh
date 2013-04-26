
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

create_test_program() {
    echo '#! /usr/bin/env atf-sh' >"${1}"
    cat >>"${1}"
    chmod +x "${1}"
}

atf_test_case no_args
no_args_body()
{
    cat >experr <<EOF
atf-sh: ERROR: No test program provided
atf-sh: Type \`atf-sh -h' for more details.
EOF
    atf_check -s eq:1 -o ignore -e file:experr atf-sh
}

atf_test_case missing_script
missing_script_body()
{
    cat >experr <<EOF
atf-sh: ERROR: The test program 'non-existent' does not exist
EOF
    atf_check -s eq:1 -o ignore -e file:experr atf-sh non-existent
}

atf_test_case arguments
arguments_body()
{
    create_test_program tp <<EOF
main() {
    echo ">>>\${0}<<<"
    while test \${#} -gt 0; do
        echo ">>>\${1}<<<"
        shift
    done
    true
}
EOF

    cat >expout <<EOF
>>>./tp<<<
>>> a b <<<
>>>foo<<<
EOF
    atf_check -s eq:0 -o file:expout -e empty ./tp ' a b ' foo

    cat >expout <<EOF
>>>tp<<<
>>> hello bye <<<
>>>foo bar<<<
EOF
    atf_check -s eq:0 -o file:expout -e empty atf-sh tp ' hello bye ' 'foo bar'
}

atf_init_test_cases()
{
    atf_add_test_case no_args
    atf_add_test_case missing_script
    atf_add_test_case arguments
}

# vim: syntax=sh:expandtab:shiftwidth=4:softtabstop=4