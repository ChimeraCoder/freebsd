
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

#include <cstdlib>
#include <cstring>

#include "../macros.hpp"

#include "process.hpp"
#include "test_helpers.hpp"

// TODO: Testing the fork function is a huge task and I'm afraid of
// copy/pasting tons of stuff from the C version.  I'd rather not do that
// until some code can be shared, which cannot happen until the C++ binding
// is cleaned by a fair amount.  Instead... just rely (at the moment) on
// the system tests for the tools using this module.

// ------------------------------------------------------------------------
// Auxiliary functions.
// ------------------------------------------------------------------------

static
std::size_t
array_size(const char* const* array)
{
    std::size_t size = 0;

    for (const char* const* ptr = array; *ptr != NULL; ptr++)
        size++;

    return size;
}

static
atf::process::status
exec_process_helpers(const atf::tests::tc& tc, const char* helper_name)
{
    using atf::process::exec;

    std::vector< std::string > argv;
    argv.push_back(get_process_helpers_path(tc).leaf_name());
    argv.push_back(helper_name);

    return exec(get_process_helpers_path(tc),
                atf::process::argv_array(argv),
                atf::process::stream_inherit(),
                atf::process::stream_inherit());
}

// ------------------------------------------------------------------------
// Tests for the "argv_array" type.
// ------------------------------------------------------------------------

ATF_TEST_CASE(argv_array_init_carray);
ATF_TEST_CASE_HEAD(argv_array_init_carray)
{
    set_md_var("descr", "Tests that argv_array is correctly constructed "
               "from a C-style array of strings");
}
ATF_TEST_CASE_BODY(argv_array_init_carray)
{
    {
        const char* const carray[] = { NULL };
        atf::process::argv_array argv(carray);

        ATF_REQUIRE_EQ(argv.size(), 0);
    }

    {
        const char* const carray[] = { "arg0", NULL };
        atf::process::argv_array argv(carray);

        ATF_REQUIRE_EQ(argv.size(), 1);
        ATF_REQUIRE(std::strcmp(argv[0], carray[0]) == 0);
    }

    {
        const char* const carray[] = { "arg0", "arg1", "arg2", NULL };
        atf::process::argv_array argv(carray);

        ATF_REQUIRE_EQ(argv.size(), 3);
        ATF_REQUIRE(std::strcmp(argv[0], carray[0]) == 0);
        ATF_REQUIRE(std::strcmp(argv[1], carray[1]) == 0);
        ATF_REQUIRE(std::strcmp(argv[2], carray[2]) == 0);
    }
}

ATF_TEST_CASE(argv_array_init_col);
ATF_TEST_CASE_HEAD(argv_array_init_col)
{
    set_md_var("descr", "Tests that argv_array is correctly constructed "
               "from a string collection");
}
ATF_TEST_CASE_BODY(argv_array_init_col)
{
    {
        std::vector< std::string > col;
        atf::process::argv_array argv(col);

        ATF_REQUIRE_EQ(argv.size(), 0);
    }

    {
        std::vector< std::string > col;
        col.push_back("arg0");
        atf::process::argv_array argv(col);

        ATF_REQUIRE_EQ(argv.size(), 1);
        ATF_REQUIRE_EQ(argv[0], col[0]);
    }

    {
        std::vector< std::string > col;
        col.push_back("arg0");
        col.push_back("arg1");
        col.push_back("arg2");
        atf::process::argv_array argv(col);

        ATF_REQUIRE_EQ(argv.size(), 3);
        ATF_REQUIRE_EQ(argv[0], col[0]);
        ATF_REQUIRE_EQ(argv[1], col[1]);
        ATF_REQUIRE_EQ(argv[2], col[2]);
    }
}

ATF_TEST_CASE(argv_array_init_empty);
ATF_TEST_CASE_HEAD(argv_array_init_empty)
{
    set_md_var("descr", "Tests that argv_array is correctly constructed "
               "by the default constructor");
}
ATF_TEST_CASE_BODY(argv_array_init_empty)
{
    atf::process::argv_array argv;

    ATF_REQUIRE_EQ(argv.size(), 0);
}

ATF_TEST_CASE(argv_array_init_varargs);
ATF_TEST_CASE_HEAD(argv_array_init_varargs)
{
    set_md_var("descr", "Tests that argv_array is correctly constructed "
               "from a variable list of arguments");
}
ATF_TEST_CASE_BODY(argv_array_init_varargs)
{
    {
        atf::process::argv_array argv("arg0", NULL);

        ATF_REQUIRE_EQ(argv.size(), 1);
        ATF_REQUIRE_EQ(argv[0], std::string("arg0"));
    }

    {
        atf::process::argv_array argv("arg0", "arg1", "arg2", NULL);

        ATF_REQUIRE_EQ(argv.size(), 3);
        ATF_REQUIRE_EQ(argv[0], std::string("arg0"));
        ATF_REQUIRE_EQ(argv[1], std::string("arg1"));
        ATF_REQUIRE_EQ(argv[2], std::string("arg2"));
    }
}

ATF_TEST_CASE(argv_array_assign);
ATF_TEST_CASE_HEAD(argv_array_assign)
{
    set_md_var("descr", "Tests that assigning an argv_array works");
}
ATF_TEST_CASE_BODY(argv_array_assign)
{
    using atf::process::argv_array;

    const char* const carray1[] = { "arg1", NULL };
    const char* const carray2[] = { "arg1", "arg2", NULL };

    std::auto_ptr< argv_array > argv1(new argv_array(carray1));
    std::auto_ptr< argv_array > argv2(new argv_array(carray2));

    *argv2 = *argv1;
    ATF_REQUIRE_EQ(argv2->size(), argv1->size());
    ATF_REQUIRE(std::strcmp((*argv2)[0], (*argv1)[0]) == 0);

    ATF_REQUIRE(argv2->exec_argv() != argv1->exec_argv());
    argv1.release();
    {
        const char* const* eargv2 = argv2->exec_argv();
        ATF_REQUIRE(std::strcmp(eargv2[0], carray1[0]) == 0);
        ATF_REQUIRE_EQ(eargv2[1], static_cast< const char* >(NULL));
    }

    argv2.release();
}

ATF_TEST_CASE(argv_array_copy);
ATF_TEST_CASE_HEAD(argv_array_copy)
{
    set_md_var("descr", "Tests that copying an argv_array constructed from "
               "a C-style array of strings works");
}
ATF_TEST_CASE_BODY(argv_array_copy)
{
    using atf::process::argv_array;

    const char* const carray[] = { "arg0", NULL };

    std::auto_ptr< argv_array > argv1(new argv_array(carray));
    std::auto_ptr< argv_array > argv2(new argv_array(*argv1));

    ATF_REQUIRE_EQ(argv2->size(), argv1->size());
    ATF_REQUIRE(std::strcmp((*argv2)[0], (*argv1)[0]) == 0);

    ATF_REQUIRE(argv2->exec_argv() != argv1->exec_argv());
    argv1.release();
    {
        const char* const* eargv2 = argv2->exec_argv();
        ATF_REQUIRE(std::strcmp(eargv2[0], carray[0]) == 0);
        ATF_REQUIRE_EQ(eargv2[1], static_cast< const char* >(NULL));
    }

    argv2.release();
}

ATF_TEST_CASE(argv_array_exec_argv);
ATF_TEST_CASE_HEAD(argv_array_exec_argv)
{
    set_md_var("descr", "Tests that the exec argv provided by an argv_array "
               "is correct");
}
ATF_TEST_CASE_BODY(argv_array_exec_argv)
{
    using atf::process::argv_array;

    {
        argv_array argv;
        const char* const* eargv = argv.exec_argv();
        ATF_REQUIRE_EQ(array_size(eargv), 0);
        ATF_REQUIRE_EQ(eargv[0], static_cast< const char* >(NULL));
    }

    {
        const char* const carray[] = { "arg0", NULL };
        argv_array argv(carray);
        const char* const* eargv = argv.exec_argv();
        ATF_REQUIRE_EQ(array_size(eargv), 1);
        ATF_REQUIRE(std::strcmp(eargv[0], "arg0") == 0);
        ATF_REQUIRE_EQ(eargv[1], static_cast< const char* >(NULL));
    }

    {
        std::vector< std::string > col;
        col.push_back("arg0");
        argv_array argv(col);
        const char* const* eargv = argv.exec_argv();
        ATF_REQUIRE_EQ(array_size(eargv), 1);
        ATF_REQUIRE(std::strcmp(eargv[0], "arg0") == 0);
        ATF_REQUIRE_EQ(eargv[1], static_cast< const char* >(NULL));
    }
}

ATF_TEST_CASE(argv_array_iter);
ATF_TEST_CASE_HEAD(argv_array_iter)
{
    set_md_var("descr", "Tests that an argv_array can be iterated");
}
ATF_TEST_CASE_BODY(argv_array_iter)
{
    using atf::process::argv_array;

    std::vector< std::string > vector;
    vector.push_back("arg0");
    vector.push_back("arg1");
    vector.push_back("arg2");

    argv_array argv(vector);
    ATF_REQUIRE_EQ(argv.size(), 3);
    std::vector< std::string >::size_type pos = 0;
    for (argv_array::const_iterator iter = argv.begin(); iter != argv.end();
         iter++) {
        ATF_REQUIRE_EQ(*iter, vector[pos]);
        pos++;
    }
}

// ------------------------------------------------------------------------
// Tests cases for the free functions.
// ------------------------------------------------------------------------

ATF_TEST_CASE(exec_failure);
ATF_TEST_CASE_HEAD(exec_failure)
{
    set_md_var("descr", "Tests execing a command that reports failure");
}
ATF_TEST_CASE_BODY(exec_failure)
{
    const atf::process::status s = exec_process_helpers(*this, "exit-failure");
    ATF_REQUIRE(s.exited());
    ATF_REQUIRE_EQ(s.exitstatus(), EXIT_FAILURE);
}

ATF_TEST_CASE(exec_success);
ATF_TEST_CASE_HEAD(exec_success)
{
    set_md_var("descr", "Tests execing a command that reports success");
}
ATF_TEST_CASE_BODY(exec_success)
{
    const atf::process::status s = exec_process_helpers(*this, "exit-success");
    ATF_REQUIRE(s.exited());
    ATF_REQUIRE_EQ(s.exitstatus(), EXIT_SUCCESS);
}

// ------------------------------------------------------------------------
// Main.
// ------------------------------------------------------------------------

ATF_INIT_TEST_CASES(tcs)
{
    // Add the test cases for the "argv_array" type.
    ATF_ADD_TEST_CASE(tcs, argv_array_assign);
    ATF_ADD_TEST_CASE(tcs, argv_array_copy);
    ATF_ADD_TEST_CASE(tcs, argv_array_exec_argv);
    ATF_ADD_TEST_CASE(tcs, argv_array_init_carray);
    ATF_ADD_TEST_CASE(tcs, argv_array_init_col);
    ATF_ADD_TEST_CASE(tcs, argv_array_init_empty);
    ATF_ADD_TEST_CASE(tcs, argv_array_init_varargs);
    ATF_ADD_TEST_CASE(tcs, argv_array_iter);

    // Add the test cases for the free functions.
    ATF_ADD_TEST_CASE(tcs, exec_failure);
    ATF_ADD_TEST_CASE(tcs, exec_success);
}