
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

atf_test_case vflag
vflag_head()
{
    atf_set "descr" "Tests that the -v flag works correctly to set" \
                    "configuration variables"
}
vflag_body()
{
    for h in $(get_helpers); do
        atf_check -s eq:0 -o ignore -e ignore ${h} -s $(atf_get_srcdir) \
            -r resfile config_unset
        atf_check -s eq:0 -o ignore -e empty grep 'passed' resfile

        atf_check -s eq:0 -o ignore -e ignore ${h} -s $(atf_get_srcdir) \
            -r resfile -v 'test=' config_empty
        atf_check -s eq:0 -o ignore -e empty grep 'passed' resfile

        atf_check -s eq:0 -o ignore -e ignore ${h} -s $(atf_get_srcdir) \
            -r resfile -v 'test=foo' config_value
        atf_check -s eq:0 -o ignore -e empty grep 'passed' resfile

        atf_check -s eq:0 -o ignore -e ignore -x ${h} -s $(atf_get_srcdir) \
            -r resfile -v \'test=foo bar\' config_multi_value
        atf_check -s eq:0 -o ignore -e empty grep 'passed' resfile
    done
}

atf_init_test_cases()
{
    atf_add_test_case vflag
}

# vim: syntax=sh:expandtab:shiftwidth=4:softtabstop=4