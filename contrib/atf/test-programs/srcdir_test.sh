
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

create_files()
{
    mkdir tmp
    touch tmp/datafile
}

atf_test_case default
default_head()
{
    atf_set "descr" "Checks that the program can find its files if" \
                    "executed from the same directory"
}
default_body()
{
    create_files

    for hp in $(get_helpers); do
        h=${hp##*/}
        cp ${hp} tmp
        atf_check -s eq:0 -o ignore -e ignore -x \
                  "cd tmp && ./${h} srcdir_exists"
        atf_check -s eq:1 -o empty -e ignore "${hp}" -r res srcdir_exists
        atf_check -s eq:0 -o ignore -e empty grep "Cannot find datafile" res
    done
}

atf_test_case libtool
libtool_head()
{
    atf_set "descr" "Checks that the program can find its files if" \
                    "executed from the source directory and if it" \
                    "was built with libtool"
}
libtool_body()
{
    create_files
    mkdir tmp/.libs

    for hp in $(get_helpers c_helpers cpp_helpers); do
        h=${hp##*/}
        cp ${hp} tmp
        cp ${hp} tmp/.libs
        atf_check -s eq:0 -o ignore -e ignore -x \
                  "cd tmp && ./.libs/${h} srcdir_exists"
        atf_check -s eq:1 -o empty -e ignore "${hp}" -r res srcdir_exists
        atf_check -s eq:0 -o ignore -e empty grep "Cannot find datafile" res
    done

    for hp in $(get_helpers c_helpers cpp_helpers); do
        h=${hp##*/}
        cp ${hp} tmp
        cp ${hp} tmp/.libs/lt-${h}
        atf_check -s eq:0 -o ignore -e ignore -x \
                  "cd tmp && ./.libs/lt-${h} srcdir_exists"
        atf_check -s eq:1 -o empty -e ignore "${hp}" -r res srcdir_exists
        atf_check -s eq:0 -o ignore -e empty grep "Cannot find datafile" res
    done
}

atf_test_case sflag
sflag_head()
{
    atf_set "descr" "Checks that the program can find its files when" \
                    "using the -s flag"
}
sflag_body()
{
    create_files

    for hp in $(get_helpers); do
        h=${hp##*/}
        cp ${hp} tmp
        atf_check -s eq:0 -o ignore -e ignore -x \
                  "cd tmp && ./${h} -s $(pwd)/tmp \
                   srcdir_exists"
        atf_check -s eq:1 -o empty -e save:stderr "${hp}" -r res srcdir_exists
        atf_check -s eq:0 -o ignore -e empty grep "Cannot find datafile" res
        atf_check -s eq:0 -o ignore -e ignore \
                  "${hp}" -s "$(pwd)"/tmp srcdir_exists
    done
}

atf_test_case relative
relative_head()
{
    atf_set "descr" "Checks that passing a relative path through -s" \
                    "works"
}
relative_body()
{
    create_files

    for hp in $(get_helpers); do
        h=${hp##*/}
        cp ${hp} tmp

        for p in tmp tmp/. ./tmp; do
            echo "Helper is: ${h}"
            echo "Using source directory: ${p}"

            atf_check -s eq:0 -o ignore -e ignore \
                      "./tmp/${h}" -s "${p}" srcdir_exists
            atf_check -s eq:1 -o empty -e save:stderr "${hp}" -r res \
                srcdir_exists
            atf_check -s eq:0 -o ignore -e empty grep "Cannot find datafile" res
            atf_check -s eq:0 -o ignore -e ignore \
                      "${hp}" -s "${p}" srcdir_exists
        done
    done
}

atf_init_test_cases()
{
    atf_add_test_case default
    atf_add_test_case libtool
    atf_add_test_case sflag
    atf_add_test_case relative
}

# vim: syntax=sh:expandtab:shiftwidth=4:softtabstop=4