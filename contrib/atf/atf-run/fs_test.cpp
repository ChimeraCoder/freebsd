
/*
 * You may redistribute this program and/or modify it under the terms of
 * the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

extern "C" {
#include <sys/types.h>
#include <sys/stat.h>
}

#include <cerrno>
#include <fstream>

#include "atf-c++/macros.hpp"

#include "atf-c++/detail/exceptions.hpp"
#include "atf-c++/detail/fs.hpp"

#include "fs.hpp"
#include "user.hpp"

// ------------------------------------------------------------------------
// Auxiliary functions.
// ------------------------------------------------------------------------

static
void
create_file(const char *name)
{
    std::ofstream os(name);
    os.close();
}

// ------------------------------------------------------------------------
// Test cases for the "temp_dir" class.
// ------------------------------------------------------------------------

ATF_TEST_CASE(temp_dir_raii);
ATF_TEST_CASE_HEAD(temp_dir_raii)
{
    set_md_var("descr", "Tests the RAII behavior of the temp_dir class");
}
ATF_TEST_CASE_BODY(temp_dir_raii)
{
    using atf::atf_run::temp_dir;

    atf::fs::path t1("non-existent");
    atf::fs::path t2("non-existent");

    {
        atf::fs::path tmpl("testdir.XXXXXX");
        temp_dir td1(tmpl);
        temp_dir td2(tmpl);
        t1 = td1.get_path();
        t2 = td2.get_path();
        ATF_REQUIRE(t1.str().find("XXXXXX") == std::string::npos);
        ATF_REQUIRE(t2.str().find("XXXXXX") == std::string::npos);
        ATF_REQUIRE(t1 != t2);
        ATF_REQUIRE(!atf::fs::exists(tmpl));
        ATF_REQUIRE( atf::fs::exists(t1));
        ATF_REQUIRE( atf::fs::exists(t2));

        atf::fs::file_info fi1(t1);
        ATF_REQUIRE( fi1.is_owner_readable());
        ATF_REQUIRE( fi1.is_owner_writable());
        ATF_REQUIRE( fi1.is_owner_executable());
        ATF_REQUIRE(!fi1.is_group_readable());
        ATF_REQUIRE(!fi1.is_group_writable());
        ATF_REQUIRE(!fi1.is_group_executable());
        ATF_REQUIRE(!fi1.is_other_readable());
        ATF_REQUIRE(!fi1.is_other_writable());
        ATF_REQUIRE(!fi1.is_other_executable());

        atf::fs::file_info fi2(t2);
        ATF_REQUIRE( fi2.is_owner_readable());
        ATF_REQUIRE( fi2.is_owner_writable());
        ATF_REQUIRE( fi2.is_owner_executable());
        ATF_REQUIRE(!fi2.is_group_readable());
        ATF_REQUIRE(!fi2.is_group_writable());
        ATF_REQUIRE(!fi2.is_group_executable());
        ATF_REQUIRE(!fi2.is_other_readable());
        ATF_REQUIRE(!fi2.is_other_writable());
        ATF_REQUIRE(!fi2.is_other_executable());
    }

    ATF_REQUIRE(t1.str() != "non-existent");
    ATF_REQUIRE(!atf::fs::exists(t1));
    ATF_REQUIRE(t2.str() != "non-existent");
    ATF_REQUIRE(!atf::fs::exists(t2));
}


// ------------------------------------------------------------------------
// Test cases for the free functions.
// ------------------------------------------------------------------------

ATF_TEST_CASE(cleanup);
ATF_TEST_CASE_HEAD(cleanup)
{
    set_md_var("descr", "Tests the cleanup function");
}
ATF_TEST_CASE_BODY(cleanup)
{
    using atf::atf_run::cleanup;

    ::mkdir("root", 0755);
    ::mkdir("root/dir", 0755);
    ::mkdir("root/dir/1", 0100);
    ::mkdir("root/dir/2", 0644);
    create_file("root/reg");

    atf::fs::path p("root");
    ATF_REQUIRE(atf::fs::exists(p));
    ATF_REQUIRE(atf::fs::exists(p / "dir"));
    ATF_REQUIRE(atf::fs::exists(p / "dir/1"));
    ATF_REQUIRE(atf::fs::exists(p / "dir/2"));
    ATF_REQUIRE(atf::fs::exists(p / "reg"));
    cleanup(p);
    ATF_REQUIRE(!atf::fs::exists(p));
}

ATF_TEST_CASE(cleanup_eacces_on_root);
ATF_TEST_CASE_HEAD(cleanup_eacces_on_root)
{
    set_md_var("descr", "Tests the cleanup function");
}
ATF_TEST_CASE_BODY(cleanup_eacces_on_root)
{
    using atf::atf_run::cleanup;

    ::mkdir("aux", 0755);
    ::mkdir("aux/root", 0755);
    ATF_REQUIRE(::chmod("aux", 0555) != -1);

    try {
        cleanup(atf::fs::path("aux/root"));
        ATF_REQUIRE(atf::atf_run::is_root());
    } catch (const atf::system_error& e) {
        ATF_REQUIRE(!atf::atf_run::is_root());
        ATF_REQUIRE_EQ(EACCES, e.code());
    }
}

ATF_TEST_CASE(cleanup_eacces_on_subdir);
ATF_TEST_CASE_HEAD(cleanup_eacces_on_subdir)
{
    set_md_var("descr", "Tests the cleanup function");
}
ATF_TEST_CASE_BODY(cleanup_eacces_on_subdir)
{
    using atf::atf_run::cleanup;

    ::mkdir("root", 0755);
    ::mkdir("root/1", 0755);
    ::mkdir("root/1/2", 0755);
    ::mkdir("root/1/2/3", 0755);
    ATF_REQUIRE(::chmod("root/1/2", 0555) != -1);
    ATF_REQUIRE(::chmod("root/1", 0555) != -1);

    const atf::fs::path p("root");
    cleanup(p);
    ATF_REQUIRE(!atf::fs::exists(p));
}

ATF_TEST_CASE(change_directory);
ATF_TEST_CASE_HEAD(change_directory)
{
    set_md_var("descr", "Tests the change_directory function");
}
ATF_TEST_CASE_BODY(change_directory)
{
    using atf::atf_run::change_directory;
    using atf::atf_run::get_current_dir;

    ::mkdir("files", 0755);
    ::mkdir("files/dir", 0755);
    create_file("files/reg");

    const atf::fs::path old = get_current_dir();

    ATF_REQUIRE_THROW(atf::system_error,
                    change_directory(atf::fs::path("files/reg")));
    ATF_REQUIRE(get_current_dir() == old);

    atf::fs::path old2 = change_directory(atf::fs::path("files"));
    ATF_REQUIRE(old2 == old);
    atf::fs::path old3 = change_directory(atf::fs::path("dir"));
    ATF_REQUIRE(old3 == old2 / "files");
    atf::fs::path old4 = change_directory(atf::fs::path("../.."));
    ATF_REQUIRE(old4 == old3 / "dir");
    ATF_REQUIRE(get_current_dir() == old);
}

ATF_TEST_CASE(get_current_dir);
ATF_TEST_CASE_HEAD(get_current_dir)
{
    set_md_var("descr", "Tests the get_current_dir function");
}
ATF_TEST_CASE_BODY(get_current_dir)
{
    using atf::atf_run::change_directory;
    using atf::atf_run::get_current_dir;

    ::mkdir("files", 0755);
    ::mkdir("files/dir", 0755);
    create_file("files/reg");

    atf::fs::path curdir = get_current_dir();
    change_directory(atf::fs::path("."));
    ATF_REQUIRE(get_current_dir() == curdir);
    change_directory(atf::fs::path("files"));
    ATF_REQUIRE(get_current_dir() == curdir / "files");
    change_directory(atf::fs::path("dir"));
    ATF_REQUIRE(get_current_dir() == curdir / "files/dir");
    change_directory(atf::fs::path(".."));
    ATF_REQUIRE(get_current_dir() == curdir / "files");
    change_directory(atf::fs::path(".."));
    ATF_REQUIRE(get_current_dir() == curdir);
}

// ------------------------------------------------------------------------
// Main.
// ------------------------------------------------------------------------

ATF_INIT_TEST_CASES(tcs)
{
    // Add the tests for the "temp_dir" class.
    ATF_ADD_TEST_CASE(tcs, temp_dir_raii);

    // Add the tests for the free functions.
    ATF_ADD_TEST_CASE(tcs, cleanup);
    ATF_ADD_TEST_CASE(tcs, cleanup_eacces_on_root);
    ATF_ADD_TEST_CASE(tcs, cleanup_eacces_on_subdir);
    ATF_ADD_TEST_CASE(tcs, change_directory);
    ATF_ADD_TEST_CASE(tcs, get_current_dir);
}