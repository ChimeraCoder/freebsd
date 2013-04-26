
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

atf_test_case no_descr
no_descr_head()
{
    atf_set "descr" "Tests that the description may not be present"
}
no_descr_body()
{
    for h in $(get_helpers); do
        atf_check -s eq:0 -o ignore -e ignore ${h} -s $(atf_get_srcdir) -l
        atf_check -s eq:0 -o match:passed -e ignore ${h} -s $(atf_get_srcdir) \
            metadata_no_descr
    done
}

atf_test_case no_head
no_head_head()
{
    atf_set "descr" "Tests that the head may not be present"
}
no_head_body()
{
    for h in $(get_helpers); do
        atf_check -s eq:0 -o ignore -e ignore ${h} -s $(atf_get_srcdir) -l
        atf_check -s eq:0 -o match:passed -e ignore ${h} -s $(atf_get_srcdir) \
            metadata_no_head
    done
}

atf_init_test_cases()
{
    atf_add_test_case no_descr
    atf_add_test_case no_head
}

# vim: syntax=sh:expandtab:shiftwidth=4:softtabstop=4