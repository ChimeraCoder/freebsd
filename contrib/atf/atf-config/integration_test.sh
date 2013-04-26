
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

all_vars="atf_arch \
          atf_build_cc \
          atf_build_cflags \
          atf_build_cpp \
          atf_build_cppflags \
          atf_build_cxx \
          atf_build_cxxflags \
          atf_confdir \
          atf_includedir \
          atf_libdir \
          atf_libexecdir \
          atf_machine \
          atf_pkgdatadir \
          atf_shell \
          atf_workdir"
all_vars_no=15

atf_test_case list_all
list_all_head()
{
    atf_set "descr" "Tests that at atf-config prints all expected" \
                    "variables, and not more"
}
list_all_body()
{
    atf_check -s eq:0 -o save:stdout -e empty atf-config
    atf_check -s eq:0 -o empty -e empty \
              test "$(wc -l stdout | awk '{ print $1 }')" = "${all_vars_no}"
    for v in ${all_vars}; do
        atf_check -s eq:0 -o ignore -e empty grep "${v}" stdout
    done
}

atf_test_case query_one
query_one_head()
{
    atf_set "descr" "Tests that querying a single variable works"
}
query_one_body()
{
    for v in ${all_vars}; do
        atf_check -s eq:0 -o save:stdout -o match:"${v}" -e empty \
            atf-config "${v}"
        atf_check -s eq:0 -o empty -e empty \
                  test "$(wc -l stdout | awk '{ print $1 }')" = 1
    done
}

atf_test_case query_one_terse
query_one_terse_head()
{
    atf_set "descr" "Tests that querying a single variable in terse mode" \
                    "works"
}
query_one_terse_body()
{
    for v in ${all_vars}; do
        atf_check -s eq:0 -o save:stdout -o match:"${v}" -e empty \
            atf-config "${v}"
        atf_check -s eq:0 -o empty -e empty \
                  test "$(wc -l stdout | awk '{ print $1 }')" = 1
        atf_check -s eq:0 -o save:stdout -e empty cut -d ' ' -f 3- stdout
        atf_check -s eq:0 -o empty -e empty mv stdout expout
        atf_check -s eq:0 -o file:expout -e empty atf-config -t "${v}"
    done
}

atf_test_case query_multiple
query_multiple_head()
{
    atf_set "descr" "Tests that querying multiple variables works"
}
query_multiple_body()
{
    atf_check -s eq:0 -o save:stdout -o match:'atf_libexecdir' \
        -o match:'atf_shell' -e empty atf-config atf_libexecdir atf_shell
    atf_check -s eq:0 -o empty -e empty \
              test "$(wc -l stdout | awk '{ print $1 }')" = 2
}

atf_test_case query_unknown
query_unknown_head()
{
    atf_set "descr" "Tests that querying an unknown variable delivers" \
                    "the correct error"
}
query_unknown_body()
{
    atf_check -s eq:1 -o empty -e match:'Unknown variable.*non_existent' \
        atf-config non_existent
}

atf_test_case query_mixture
query_mixture_head()
{
    atf_set "descr" "Tests that querying a known and an unknown variable" \
                    "delivers the correct error"
}
query_mixture_body()
{
    for v in ${all_vars}; do
        atf_check -s eq:1 -o empty -e match:'Unknown variable.*non_existent' \
                  atf-config "${v}" non_existent
        atf_check -s eq:1 -o empty -e match:'Unknown variable.*non_existent' \
                  atf-config non_existent "${v}"
    done
}

atf_test_case override_env
override_env_head()
{
    atf_set "descr" "Tests that build-time variables can be overriden" \
                    "through their corresponding environment variables"
}
override_env_body()
{
    for v in ${all_vars}; do
        V=$(echo ${v} | tr '[a-z]' '[A-Z]')
        atf_check -s eq:0 -o save:stdout -e empty -x "${V}=testval atf-config"
        atf_check -s eq:0 -o empty -e empty mv stdout all

        atf_check -s eq:0 -o save:stdout -e empty grep "^${v} : " all
        atf_check -s eq:0 -o empty -e empty mv stdout affected
        atf_check -s eq:0 -o save:stdout -e empty grep -v "^${v} : " all
        atf_check -s eq:0 -o empty -e empty mv stdout unaffected

        atf_check -s eq:0 -o empty -e empty \
                  test "$(wc -l affected | awk '{ print $1 }')" = 1
        atf_check -s eq:0 -o empty -e empty \
                  test "$(wc -l unaffected | awk '{ print $1 }')" = \
                   "$((${all_vars_no} -1))"

        atf_check -s eq:0 -o ignore -e empty grep "^${v} : testval$" affected
        atf_check -s eq:1 -o empty -e empty grep ' : testval$' unaffected
    done
}

atf_init_test_cases()
{
    atf_add_test_case list_all

    atf_add_test_case query_one
    atf_add_test_case query_one_terse
    atf_add_test_case query_multiple
    atf_add_test_case query_unknown
    atf_add_test_case query_mixture

    atf_add_test_case override_env
}

# vim: syntax=sh:expandtab:shiftwidth=4:softtabstop=4