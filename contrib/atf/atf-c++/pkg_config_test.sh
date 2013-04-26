
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

# The following tests assume that the atfc++.pc file is installed in a
# directory that is known by pkg-config.  Otherwise they will fail,
# and you will be required to adjust PKG_CONFIG_PATH accordingly.
#
# It would be possible to bypass this requirement by setting the path
# explicitly during the tests, but then this would not do a real check
# to ensure that the installation is working.

require_pc()
{
    pkg-config ${1} || atf_fail "pkg-config could not locate ${1}.pc;" \
                                "maybe need to set PKG_CONFIG_PATH?"
}

check_version()
{
    atf_check -s eq:0 -o save:stdout -e empty -x \
              "atf-version | head -n 1 | cut -d ' ' -f 4"
    ver1=$(cat stdout)
    echo "Version reported by atf-version: ${ver1}"

    atf_check -s eq:0 -o save:stdout -e empty pkg-config --modversion "${1}"
    ver2=$(cat stdout)
    echo "Version reported by pkg-config: ${ver2}"

    atf_check_equal ${ver1} ${ver2}
}

atf_test_case version
version_head()
{
    atf_set "descr" "Checks that the version in atf-c++ is correct"
    atf_set "require.progs" "pkg-config"
}
version_body()
{
    require_pc "atf-c++"

    check_version "atf-c++"
}

atf_test_case build
build_head()
{
    atf_set "descr" "Checks that a test program can be built against" \
                    "the C++ library based on the pkg-config information"
    atf_set "require.progs" "pkg-config"
}
build_body()
{
    require_pc "atf-c++"

    atf_check -s eq:0 -o save:stdout -e empty \
              pkg-config --variable=cxx atf-c++
    cxx=$(cat stdout)
    echo "Compiler is: ${cxx}"
    atf_require_prog ${cxx}

    cat >tp.cpp <<EOF
#include <iostream>

#include <atf-c++.hpp>

ATF_TEST_CASE(tc);
ATF_TEST_CASE_HEAD(tc) {
    set_md_var("descr", "A test case");
}
ATF_TEST_CASE_BODY(tc) {
    std::cout << "Running\n";
}

ATF_INIT_TEST_CASES(tcs) {
    ATF_ADD_TEST_CASE(tcs, tc);
}
EOF

    atf_check -s eq:0 -o save:stdout -e empty pkg-config --cflags atf-c++
    cxxflags=$(cat stdout)
    echo "CXXFLAGS are: ${cxxflags}"

    atf_check -s eq:0 -o save:stdout -e empty \
        pkg-config --libs-only-L --libs-only-other atf-c++
    ldflags=$(cat stdout)
    atf_check -s eq:0 -o save:stdout -e empty \
              pkg-config --libs-only-l atf-c++
    libs=$(cat stdout)
    echo "LDFLAGS are: ${ldflags}"
    echo "LIBS are: ${libs}"

    atf_check -s eq:0 -o empty -e empty ${cxx} ${cxxflags} -o tp.o -c tp.cpp
    atf_check -s eq:0 -o empty -e empty ${cxx} ${ldflags} -o tp tp.o ${libs}

    libpath=
    for f in ${ldflags}; do
        case ${f} in
            -L*)
                dir=$(echo ${f} | sed -e 's,^-L,,')
                if [ -z "${libpath}" ]; then
                    libpath="${dir}"
                else
                    libpath="${libpath}:${dir}"
                fi
                ;;
            *)
                ;;
        esac
    done

    atf_check -s eq:0 -o empty -e empty test -x tp
    atf_check -s eq:0 -o match:'Running' -e empty -x \
              "LD_LIBRARY_PATH=${libpath} ./tp tc"
}

atf_init_test_cases()
{
    atf_add_test_case version
    atf_add_test_case build
}

# vim: syntax=sh:expandtab:shiftwidth=4:softtabstop=4