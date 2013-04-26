
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

atf_test_case default_status
default_status_head()
{
    atf_set "descr" "Verifies that test cases get the correct default" \
                    "status if they did not provide any"
}
default_status_body()
{
    h="$(atf_get_srcdir)/misc_helpers -s $(atf_get_srcdir)"
    atf_check -s eq:0 -o ignore -e ignore ${h} tc_pass_true
    atf_check -s eq:1 -o ignore -e ignore ${h} tc_pass_false
    atf_check -s eq:1 -o match:'failed:.*body.*non-ok exit code' -e ignore \
        ${h} tc_pass_return_error
    atf_check -s eq:1 -o ignore -e match:'An error' ${h} tc_fail
}

atf_test_case missing_body
missing_body_head()
{
    atf_set "descr" "Verifies that test cases without a body are reported" \
                    "as failed"
}
missing_body_body()
{
    h="$(atf_get_srcdir)/misc_helpers -s $(atf_get_srcdir)"
    atf_check -s eq:1 -o ignore -e ignore ${h} tc_missing_body
}

atf_init_test_cases()
{
    atf_add_test_case default_status
    atf_add_test_case missing_body
}

# vim: syntax=sh:expandtab:shiftwidth=4:softtabstop=4