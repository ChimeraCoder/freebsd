
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

atf_test_case srcdir
srcdir_head()
{
    atf_set "descr" "Verifies that the source directory can be queried" \
                    "from the initialization function"
}
srcdir_body()
{
    mkdir work
    atf_check -s eq:0 -o empty -e empty cp "$(atf_get_srcdir)/misc_helpers" work
    cat >work/subrs <<EOF
helper_subr() {
    echo "This is a helper subroutine"
}
EOF

    atf_check -s eq:0 -o match:'Calling helper' \
        -o match:'This is a helper subroutine' -e ignore ./work/misc_helpers \
        -s "$(pwd)"/work tp_srcdir
}

atf_init_test_cases()
{
    atf_add_test_case srcdir
}

# vim: syntax=sh:expandtab:shiftwidth=4:softtabstop=4