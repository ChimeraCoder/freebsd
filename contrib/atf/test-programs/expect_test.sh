
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

check_result() {
    file="${1}"; shift

    atf_check -s eq:0 -o match:"${*}" -e empty cat "${file}"
    rm "${file}"
}

atf_test_case expect_pass
expect_pass_body() {
    for h in $(get_helpers); do
        atf_check -s eq:0 -e ignore "${h}" -r result expect_pass_and_pass
        check_result result "passed"

        atf_check -s eq:1 -e ignore "${h}" -r result \
            expect_pass_but_fail_requirement
        check_result result "failed: Some reason"

        # atf-sh does not support non-fatal failures yet; skip checks for
        # such conditions.
        case "${h}" in *sh_helpers*) continue ;; esac

        atf_check -s eq:1 -o empty -e match:"Some reason" \
            "${h}" -r result expect_pass_but_fail_check
        check_result result "failed: 1 checks failed"
    done
}

atf_test_case expect_fail
expect_fail_body() {
    for h in $(get_helpers c_helpers cpp_helpers); do
        atf_check -s eq:0 "${h}" -r result expect_fail_and_fail_requirement
        check_result result "expected_failure: Fail reason: The failure"

        atf_check -s eq:1 -e match:"Expected check failure: Fail first: abc" \
            -e not-match:"And fail again" "${h}" -r result expect_fail_but_pass
        check_result result "failed: .*expecting a failure"

        # atf-sh does not support non-fatal failures yet; skip checks for
        # such conditions.
        case "${h}" in *sh_helpers*) continue ;; esac

        atf_check -s eq:0 -e match:"Expected check failure: Fail first: abc" \
            -e match:"Expected check failure: And fail again: def" \
            "${h}" -r result expect_fail_and_fail_check
        check_result result "expected_failure: And fail again: 2 checks" \
            "failed as expected"
    done

    # atf-sh does not support non-fatal failures yet; skip checks for
    # such conditions.
    for h in $(get_helpers sh_helpers); do
        atf_check -s eq:0 -e ignore "${h}" -r result \
            expect_fail_and_fail_requirement
        check_result result "expected_failure: Fail reason: The failure"

        atf_check -s eq:1 -e ignore "${h}" -r result expect_fail_but_pass
        check_result result "failed: .*expecting a failure"
    done
}

atf_test_case expect_exit
expect_exit_body() {
    for h in $(get_helpers); do
        atf_check -s eq:0 -e ignore "${h}" -r result expect_exit_any_and_exit
        check_result result "expected_exit: Call will exit"

        atf_check -s eq:123 -e ignore "${h}" -r result expect_exit_code_and_exit
        check_result result "expected_exit\(123\): Call will exit"

        atf_check -s eq:1 -e ignore "${h}" -r result expect_exit_but_pass
        check_result result "failed: .*expected to exit"
    done
}

atf_test_case expect_signal
expect_signal_body() {
    for h in $(get_helpers); do
        atf_check -s signal:9 -e ignore "${h}" -r result \
            expect_signal_any_and_signal
        check_result result "expected_signal: Call will signal"

        atf_check -s signal:hup -e ignore "${h}" -r result \
            expect_signal_no_and_signal
        check_result result "expected_signal\(1\): Call will signal"

        atf_check -s eq:1 -e ignore "${h}" -r result \
            expect_signal_but_pass
        check_result result "failed: .*termination signal"
    done
}

atf_test_case expect_death
expect_death_body() {
    for h in $(get_helpers); do
        atf_check -s eq:123 -e ignore "${h}" -r result expect_death_and_exit
        check_result result "expected_death: Exit case"

        atf_check -s signal:kill -e ignore "${h}" -r result \
            expect_death_and_signal
        check_result result "expected_death: Signal case"

        atf_check -s eq:1 -e ignore "${h}" -r result expect_death_but_pass
        check_result result "failed: .*terminate abruptly"
    done
}

atf_test_case expect_timeout
expect_timeout_body() {
    for h in $(get_helpers); do
        atf_check -s eq:1 -e ignore "${h}" -r result expect_timeout_but_pass
        check_result result "failed: Test case was expected to hang but it" \
            "continued execution"
    done
}

atf_init_test_cases()
{
    atf_add_test_case expect_pass
    atf_add_test_case expect_fail
    atf_add_test_case expect_exit
    atf_add_test_case expect_signal
    atf_add_test_case expect_death
    atf_add_test_case expect_timeout
}

# vim: syntax=sh:expandtab:shiftwidth=4:softtabstop=4