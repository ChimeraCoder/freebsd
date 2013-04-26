
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

# TODO: Bring in the checks in the bootstrap testsuite for atf_check.

atf_test_case info_ok
info_ok_head()
{
    atf_set "descr" "Verifies that atf_check prints an informative" \
                    "message even when the command is successful"
}
info_ok_body()
{
    h="$(atf_get_srcdir)/misc_helpers -s $(atf_get_srcdir)"

    atf_check -s eq:0 -o save:stdout -e save:stderr -x \
              "${h} atf_check_info_ok"
    grep 'Executing command.*true' stdout >/dev/null || \
        atf_fail "atf_check does not print an informative message"

    atf_check -s eq:0 -o save:stdout -e save:stderr -x \
              "${h} atf_check_info_fail"
    grep 'Executing command.*false' stdout >/dev/null || \
        atf_fail "atf_check does not print an informative message"
}

atf_test_case expout_mismatch
expout_mismatch_head()
{
    atf_set "descr" "Verifies that atf_check prints a diff of the" \
                    "stdout and the expected stdout of the two do not" \
                    "match"
}
expout_mismatch_body()
{
    h="$(atf_get_srcdir)/misc_helpers -s $(atf_get_srcdir)"

    atf_check -s eq:1 -o save:stdout -e save:stderr -x \
              "${h} atf_check_expout_mismatch"
    grep 'Executing command.*echo bar' stdout >/dev/null || \
        atf_fail "atf_check does not print an informative message"
    grep 'stdout does not match golden output' stderr >/dev/null || \
        atf_fail "atf_check does not print the stdout header"
    grep 'stderr' stderr >/dev/null && \
        atf_fail "atf_check prints the stderr header"
    grep '^-foo' stderr >/dev/null || \
        atf_fail "atf_check does not print the stdout's diff"
    grep '^+bar' stderr >/dev/null || \
        atf_fail "atf_check does not print the stdout's diff"
}

atf_test_case experr_mismatch
experr_mismatch_head()
{
    atf_set "descr" "Verifies that atf_check prints a diff of the" \
                    "stderr and the expected stderr of the two do not" \
                    "match"
}
experr_mismatch_body()
{
    h="$(atf_get_srcdir)/misc_helpers -s $(atf_get_srcdir)"

    atf_check -s eq:1 -o save:stdout -e save:stderr -x \
              "${h} atf_check_experr_mismatch"
    grep 'Executing command.*echo bar' stdout >/dev/null || \
        atf_fail "atf_check does not print an informative message"
    grep 'stdout' stderr >/dev/null && \
        atf_fail "atf_check prints the stdout header"
    grep 'stderr does not match golden output' stderr >/dev/null || \
        atf_fail "atf_check does not print the stderr header"
    grep '^-foo' stderr >/dev/null || \
        atf_fail "atf_check does not print the stderr's diff"
    grep '^+bar' stderr >/dev/null || \
        atf_fail "atf_check does not print the stderr's diff"
}

atf_test_case null_stdout
null_stdout_head()
{
    atf_set "descr" "Verifies that atf_check prints a the stdout it got" \
                    "when it was supposed to be null"
}
null_stdout_body()
{
    h="$(atf_get_srcdir)/misc_helpers -s $(atf_get_srcdir)"

    atf_check -s eq:1 -o save:stdout -e save:stderr -x \
              "${h} atf_check_null_stdout"
    grep 'Executing command.*echo.*These.*contents' stdout >/dev/null || \
        atf_fail "atf_check does not print an informative message"
    grep 'stdout not empty' stderr >/dev/null || \
        atf_fail "atf_check does not print the stdout header"
    grep 'stderr' stderr >/dev/null && \
        atf_fail "atf_check prints the stderr header"
    grep 'These are the contents' stderr >/dev/null || \
        atf_fail "atf_check does not print stdout's contents"
}

atf_test_case null_stderr
null_stderr_head()
{
    atf_set "descr" "Verifies that atf_check prints a the stderr it got" \
                    "when it was supposed to be null"
}
null_stderr_body()
{
    h="$(atf_get_srcdir)/misc_helpers -s $(atf_get_srcdir)"

    atf_check -s eq:1 -o save:stdout -e save:stderr -x \
              "${h} atf_check_null_stderr"
    grep 'Executing command.*echo.*These.*contents' stdout >/dev/null || \
        atf_fail "atf_check does not print an informative message"
    grep 'stdout' stderr >/dev/null && \
        atf_fail "atf_check prints the stdout header"
    grep 'stderr not empty' stderr >/dev/null || \
        atf_fail "atf_check does not print the stderr header"
    grep 'These are the contents' stderr >/dev/null || \
        atf_fail "atf_check does not print stderr's contents"
}

atf_test_case equal
equal_head()
{
    atf_set "descr" "Verifies that atf_check_equal works"
}
equal_body()
{
    h="$(atf_get_srcdir)/misc_helpers -s $(atf_get_srcdir)"

    atf_check -s eq:0 -o ignore -e ignore -x "${h} atf_check_equal_ok"

    atf_check -s eq:1 -o ignore -e ignore -x \
        "${h} -r resfile atf_check_equal_fail"
    atf_check -s eq:0 -o ignore -e empty grep '^failed: a != b (a != b)$' \
        resfile

    atf_check -s eq:0 -o ignore -e ignore -x "${h} atf_check_equal_eval_ok"

    atf_check -s eq:1 -o ignore -e ignore -x \
        "${h} -r resfile atf_check_equal_eval_fail"
    atf_check -s eq:0 -o ignore -e empty \
        grep '^failed: \${x} != \${y} (a != b)$' resfile
}

atf_init_test_cases()
{
    atf_add_test_case info_ok
    atf_add_test_case expout_mismatch
    atf_add_test_case experr_mismatch
    atf_add_test_case null_stdout
    atf_add_test_case null_stderr
    atf_add_test_case equal
}

# vim: syntax=sh:expandtab:shiftwidth=4:softtabstop=4