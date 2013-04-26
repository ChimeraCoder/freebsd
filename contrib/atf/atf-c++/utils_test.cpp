
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

#include <iostream>

#include "atf-c/defs.h"

#include "macros.hpp"
#include "utils.hpp"

#include "detail/test_helpers.hpp"

// ------------------------------------------------------------------------
// Tests for the "auto_array" class.
// ------------------------------------------------------------------------

class test_array {
public:
    int m_value;

    static ssize_t m_nblocks;

    static
    atf::utils::auto_array< test_array >
    do_copy(atf::utils::auto_array< test_array >& ta)
    {
        return atf::utils::auto_array< test_array >(ta);
    }

    void* operator new(size_t size ATF_DEFS_ATTRIBUTE_UNUSED)
    {
        ATF_FAIL("New called but should have been new[]");
        return new int(5);
    }

    void* operator new[](size_t size)
    {
        m_nblocks++;
        void* mem = ::operator new(size);
        std::cout << "Allocated 'test_array' object " << mem << "\n";
        return mem;
    }

    void operator delete(void* mem ATF_DEFS_ATTRIBUTE_UNUSED)
    {
        ATF_FAIL("Delete called but should have been delete[]");
    }

    void operator delete[](void* mem)
    {
        std::cout << "Releasing 'test_array' object " << mem << "\n";
        if (m_nblocks == 0)
            ATF_FAIL("Unbalanced delete[]");
        m_nblocks--;
        ::operator delete(mem);
    }
};

ssize_t test_array::m_nblocks = 0;

ATF_TEST_CASE(auto_array_scope);
ATF_TEST_CASE_HEAD(auto_array_scope)
{
    set_md_var("descr", "Tests the automatic scope handling in the "
               "auto_array smart pointer class");
}
ATF_TEST_CASE_BODY(auto_array_scope)
{
    using atf::utils::auto_array;

    ATF_REQUIRE_EQ(test_array::m_nblocks, 0);
    {
        auto_array< test_array > t(new test_array[10]);
        ATF_REQUIRE_EQ(test_array::m_nblocks, 1);
    }
    ATF_REQUIRE_EQ(test_array::m_nblocks, 0);
}

ATF_TEST_CASE(auto_array_copy);
ATF_TEST_CASE_HEAD(auto_array_copy)
{
    set_md_var("descr", "Tests the auto_array smart pointer class' copy "
               "constructor");
}
ATF_TEST_CASE_BODY(auto_array_copy)
{
    using atf::utils::auto_array;

    ATF_REQUIRE_EQ(test_array::m_nblocks, 0);
    {
        auto_array< test_array > t1(new test_array[10]);
        ATF_REQUIRE_EQ(test_array::m_nblocks, 1);

        {
            auto_array< test_array > t2(t1);
            ATF_REQUIRE_EQ(test_array::m_nblocks, 1);
        }
        ATF_REQUIRE_EQ(test_array::m_nblocks, 0);
    }
    ATF_REQUIRE_EQ(test_array::m_nblocks, 0);
}

ATF_TEST_CASE(auto_array_copy_ref);
ATF_TEST_CASE_HEAD(auto_array_copy_ref)
{
    set_md_var("descr", "Tests the auto_array smart pointer class' copy "
               "constructor through the auxiliary auto_array_ref object");
}
ATF_TEST_CASE_BODY(auto_array_copy_ref)
{
    using atf::utils::auto_array;

    ATF_REQUIRE_EQ(test_array::m_nblocks, 0);
    {
        auto_array< test_array > t1(new test_array[10]);
        ATF_REQUIRE_EQ(test_array::m_nblocks, 1);

        {
            auto_array< test_array > t2 = test_array::do_copy(t1);
            ATF_REQUIRE_EQ(test_array::m_nblocks, 1);
        }
        ATF_REQUIRE_EQ(test_array::m_nblocks, 0);
    }
    ATF_REQUIRE_EQ(test_array::m_nblocks, 0);
}

ATF_TEST_CASE(auto_array_get);
ATF_TEST_CASE_HEAD(auto_array_get)
{
    set_md_var("descr", "Tests the auto_array smart pointer class' get "
               "method");
}
ATF_TEST_CASE_BODY(auto_array_get)
{
    using atf::utils::auto_array;

    test_array* ta = new test_array[10];
    auto_array< test_array > t(ta);
    ATF_REQUIRE_EQ(t.get(), ta);
}

ATF_TEST_CASE(auto_array_release);
ATF_TEST_CASE_HEAD(auto_array_release)
{
    set_md_var("descr", "Tests the auto_array smart pointer class' release "
               "method");
}
ATF_TEST_CASE_BODY(auto_array_release)
{
    using atf::utils::auto_array;

    test_array* ta1 = new test_array[10];
    {
        auto_array< test_array > t(ta1);
        ATF_REQUIRE_EQ(test_array::m_nblocks, 1);
        test_array* ta2 = t.release();
        ATF_REQUIRE_EQ(ta2, ta1);
        ATF_REQUIRE_EQ(test_array::m_nblocks, 1);
    }
    ATF_REQUIRE_EQ(test_array::m_nblocks, 1);
    delete [] ta1;
}

ATF_TEST_CASE(auto_array_reset);
ATF_TEST_CASE_HEAD(auto_array_reset)
{
    set_md_var("descr", "Tests the auto_array smart pointer class' reset "
               "method");
}
ATF_TEST_CASE_BODY(auto_array_reset)
{
    using atf::utils::auto_array;

    test_array* ta1 = new test_array[10];
    test_array* ta2 = new test_array[10];
    ATF_REQUIRE_EQ(test_array::m_nblocks, 2);

    {
        auto_array< test_array > t(ta1);
        ATF_REQUIRE_EQ(test_array::m_nblocks, 2);
        t.reset(ta2);
        ATF_REQUIRE_EQ(test_array::m_nblocks, 1);
        t.reset();
        ATF_REQUIRE_EQ(test_array::m_nblocks, 0);
    }
    ATF_REQUIRE_EQ(test_array::m_nblocks, 0);
}

ATF_TEST_CASE(auto_array_assign);
ATF_TEST_CASE_HEAD(auto_array_assign)
{
    set_md_var("descr", "Tests the auto_array smart pointer class' "
               "assignment operator");
}
ATF_TEST_CASE_BODY(auto_array_assign)
{
    using atf::utils::auto_array;

    ATF_REQUIRE_EQ(test_array::m_nblocks, 0);
    {
        auto_array< test_array > t1(new test_array[10]);
        ATF_REQUIRE_EQ(test_array::m_nblocks, 1);

        {
            auto_array< test_array > t2;
            t2 = t1;
            ATF_REQUIRE_EQ(test_array::m_nblocks, 1);
        }
        ATF_REQUIRE_EQ(test_array::m_nblocks, 0);
    }
    ATF_REQUIRE_EQ(test_array::m_nblocks, 0);
}

ATF_TEST_CASE(auto_array_assign_ref);
ATF_TEST_CASE_HEAD(auto_array_assign_ref)
{
    set_md_var("descr", "Tests the auto_array smart pointer class' "
               "assignment operator through the auxiliary auto_array_ref "
               "object");
}
ATF_TEST_CASE_BODY(auto_array_assign_ref)
{
    using atf::utils::auto_array;

    ATF_REQUIRE_EQ(test_array::m_nblocks, 0);
    {
        auto_array< test_array > t1(new test_array[10]);
        ATF_REQUIRE_EQ(test_array::m_nblocks, 1);

        {
            auto_array< test_array > t2;
            t2 = test_array::do_copy(t1);
            ATF_REQUIRE_EQ(test_array::m_nblocks, 1);
        }
        ATF_REQUIRE_EQ(test_array::m_nblocks, 0);
    }
    ATF_REQUIRE_EQ(test_array::m_nblocks, 0);
}

ATF_TEST_CASE(auto_array_access);
ATF_TEST_CASE_HEAD(auto_array_access)
{
    set_md_var("descr", "Tests the auto_array smart pointer class' access "
               "operator");
}
ATF_TEST_CASE_BODY(auto_array_access)
{
    using atf::utils::auto_array;

    auto_array< test_array > t(new test_array[10]);

    for (int i = 0; i < 10; i++)
        t[i].m_value = i * 2;

    for (int i = 0; i < 10; i++)
        ATF_REQUIRE_EQ(t[i].m_value, i * 2);
}

// ------------------------------------------------------------------------
// Tests cases for the header file.
// ------------------------------------------------------------------------

HEADER_TC(include, "atf-c++/utils.hpp");

// ------------------------------------------------------------------------
// Main.
// ------------------------------------------------------------------------

ATF_INIT_TEST_CASES(tcs)
{
    // Add the test for the "auto_array" class.
    ATF_ADD_TEST_CASE(tcs, auto_array_scope);
    ATF_ADD_TEST_CASE(tcs, auto_array_copy);
    ATF_ADD_TEST_CASE(tcs, auto_array_copy_ref);
    ATF_ADD_TEST_CASE(tcs, auto_array_get);
    ATF_ADD_TEST_CASE(tcs, auto_array_release);
    ATF_ADD_TEST_CASE(tcs, auto_array_reset);
    ATF_ADD_TEST_CASE(tcs, auto_array_assign);
    ATF_ADD_TEST_CASE(tcs, auto_array_assign_ref);
    ATF_ADD_TEST_CASE(tcs, auto_array_access);

    // Add the test cases for the header file.
    ATF_ADD_TEST_CASE(tcs, include);
}