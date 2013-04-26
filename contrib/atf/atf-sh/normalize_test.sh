
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

atf_test_case main
main_head()
{
    atf_set "descr" "Verifies that variable names with symbols not" \
                    "allowed as part of shell variable names work"
}
main_body()
{
    h="$(atf_get_srcdir)/misc_helpers -s $(atf_get_srcdir)"
    atf_check -s eq:0 -o match:'a.b: test value 1' \
        -o match:'c-d: test value 2' -e ignore ${h} normalize
}

atf_init_test_cases()
{
    atf_add_test_case main
}

# vim: syntax=sh:expandtab:shiftwidth=4:softtabstop=4