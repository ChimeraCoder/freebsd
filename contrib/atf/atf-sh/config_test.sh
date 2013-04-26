
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

atf_test_case has
has_head()
{
    atf_set "descr" "Verifies that atf_config_has works"
}
has_body()
{
    h="$(atf_get_srcdir)/misc_helpers -s $(atf_get_srcdir)"

    atf_check -s eq:0 -o match:'foo not found' -e ignore -x \
              "TEST_VARIABLE=foo ${h} config_has"

    atf_check -s eq:0 -o match:'foo found' -e ignore -x \
              "TEST_VARIABLE=foo ${h} -v foo=bar config_has"

    echo "Checking for deprecated variables"
    atf_check -s eq:0 -o match:'workdir not found' -e ignore -x \
              "TEST_VARIABLE=workdir ${h} config_has"
}

atf_test_case get
get_head()
{
    atf_set "descr" "Verifies that atf_config_get works"
}
get_body()
{
    h="$(atf_get_srcdir)/misc_helpers -s $(atf_get_srcdir)"

    echo "Querying an undefined variable"
    ( atf_config_get "undefined" ) >out 2>err && \
        atf_fail "Getting an undefined variable succeeded"
    grep 'not find' err || \
        atf_fail "Getting an undefined variable did not report an error"

    echo "Querying an undefined variable using a default value"
    v=$(atf_config_get "undefined" "the default value")
    [ "${v}" = "the default value" ] || \
        atf_fail "Default value does not work"

    atf_check -s eq:0 -o match:'foo = bar' -e ignore -x \
              "TEST_VARIABLE=foo ${h} -v foo=bar config_get"

    atf_check -s eq:0 -o match:'foo = baz' -e ignore -x \
              "TEST_VARIABLE=foo ${h} -v foo=baz config_get"
}

atf_init_test_cases()
{
    atf_add_test_case has
    atf_add_test_case get
}

# vim: syntax=sh:expandtab:shiftwidth=4:softtabstop=4