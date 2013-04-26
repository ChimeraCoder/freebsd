
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

atf_test_case atf_run_warnings
atf_run_warnings_head()
{
    # The fact that this test case is in this test program is an abuse.
    atf_set "descr" "Tests that the test case prints a warning because" \
                    "it is not being run by atf-run"
}
atf_run_warnings_body()
{
    unset __RUNNING_INSIDE_ATF_RUN
    srcdir="$(atf_get_srcdir)"
    for h in $(get_helpers); do
        atf_check -s eq:0 -o match:"passed" -e match:"WARNING.*atf-run" \
            "${h}" -s "${srcdir}" result_pass
    done
}

atf_test_case result_on_stdout
result_on_stdout_head()
{
    atf_set "descr" "Tests that the test case result is printed on stdout" \
                    "by default"
}
result_on_stdout_body()
{
    srcdir="$(atf_get_srcdir)"
    for h in $(get_helpers); do
        atf_check -s eq:0 -o match:"passed" -o match:"msg" \
            -e ignore "${h}" -s "${srcdir}" result_pass
        atf_check -s eq:1 -o match:"failed: Failure reason" -o match:"msg" \
            -e ignore "${h}" -s "${srcdir}" result_fail
        atf_check -s eq:0 -o match:"skipped: Skipped reason" -o match:"msg" \
            -e ignore "${h}" -s "${srcdir}" result_skip
    done
}

atf_test_case result_to_file
result_to_file_head()
{
    atf_set "descr" "Tests that the test case result is sent to a file if -r" \
                    "is used"
}
result_to_file_body()
{
    srcdir="$(atf_get_srcdir)"
    for h in $(get_helpers); do
        atf_check -s eq:0 -o inline:"msg\n" -e ignore "${h}" -s "${srcdir}" \
            -r resfile result_pass
        atf_check -o inline:"passed\n" cat resfile

        atf_check -s eq:1 -o inline:"msg\n" -e ignore "${h}" -s "${srcdir}" \
            -r resfile result_fail
        atf_check -o inline:"failed: Failure reason\n" cat resfile

        atf_check -s eq:0 -o inline:"msg\n" -e ignore "${h}" -s "${srcdir}" \
            -r resfile result_skip
        atf_check -o inline:"skipped: Skipped reason\n" cat resfile
    done
}

atf_test_case result_to_file_fail
result_to_file_fail_head()
{
    atf_set "descr" "Tests controlled failure if the test program fails to" \
        "create the results file"
    atf_set "require.user" "unprivileged"
}
result_to_file_fail_body()
{
    mkdir dir
    chmod 444 dir

    srcdir="$(atf_get_srcdir)"

    for h in $(get_helpers c_helpers cpp_helpers); do
        atf_check -s signal -o ignore \
            -e match:"FATAL ERROR: Cannot create.*'dir/resfile'" \
            "${h}" -s "${srcdir}" -r dir/resfile result_pass
    done

    for h in $(get_helpers sh_helpers); do
        atf_check -s exit -o ignore \
            -e match:"ERROR: Cannot create.*'dir/resfile'" \
            "${h}" -s "${srcdir}" -r dir/resfile result_pass
    done
}

atf_test_case result_exception
result_exception_head()
{
    atf_set "descr" "Tests that an unhandled exception is correctly captured"
}
result_exception_body()
{
    for h in $(get_helpers cpp_helpers); do
        atf_check -s exit:1 -o match:'failed: .*This is unhandled' \
            "${h}" -s "${srcdir}" result_exception
    done
}

atf_init_test_cases()
{
    atf_add_test_case atf_run_warnings
    atf_add_test_case result_on_stdout
    atf_add_test_case result_to_file
    atf_add_test_case result_to_file_fail
    atf_add_test_case result_exception
}

# vim: syntax=sh:expandtab:shiftwidth=4:softtabstop=4