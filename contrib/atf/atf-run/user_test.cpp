
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
#include <sys/param.h>
#include <sys/types.h>
#include <limits.h>
#include <unistd.h>
}

#include <iostream>
#include <set>

#include "../atf-c++/macros.hpp"

#include "user.hpp"

// ------------------------------------------------------------------------
// Test cases for the free functions.
// ------------------------------------------------------------------------

ATF_TEST_CASE(euid);
ATF_TEST_CASE_HEAD(euid)
{
    set_md_var("descr", "Tests the euid function");
}
ATF_TEST_CASE_BODY(euid)
{
    using atf::atf_run::euid;

    ATF_REQUIRE_EQ(euid(), ::geteuid());
}

ATF_TEST_CASE(is_member_of_group);
ATF_TEST_CASE_HEAD(is_member_of_group)
{
    set_md_var("descr", "Tests the is_member_of_group function");
}
ATF_TEST_CASE_BODY(is_member_of_group)
{
    using atf::atf_run::is_member_of_group;

    std::set< gid_t > groups;
    gid_t maxgid = 0;
    {
        gid_t gids[NGROUPS_MAX];
        int ngids = ::getgroups(NGROUPS_MAX, gids);
        if (ngids == -1)
            ATF_FAIL("Call to ::getgroups failed");
        for (int i = 0; i < ngids; i++) {
            groups.insert(gids[i]);
            if (gids[i] > maxgid)
                maxgid = gids[i];
        }
        std::cout << "User belongs to " << ngids << " groups\n";
        std::cout << "Last GID is " << maxgid << "\n";
    }

    const gid_t maxgid_limit = 1 << 16;
    if (maxgid > maxgid_limit) {
        std::cout << "Test truncated from " << maxgid << " groups to "
                  << maxgid_limit << " to keep the run time reasonable "
            "enough\n";
        maxgid = maxgid_limit;
    }

    for (gid_t g = 0; g <= maxgid; g++) {
        if (groups.find(g) == groups.end()) {
            std::cout << "Checking if user does not belong to group "
                      << g << "\n";
            ATF_REQUIRE(!is_member_of_group(g));
        } else {
            std::cout << "Checking if user belongs to group " << g << "\n";
            ATF_REQUIRE(is_member_of_group(g));
        }
    }
}

ATF_TEST_CASE(is_root);
ATF_TEST_CASE_HEAD(is_root)
{
    set_md_var("descr", "Tests the is_root function");
}
ATF_TEST_CASE_BODY(is_root)
{
    using atf::atf_run::is_root;

    if (::geteuid() == 0) {
        ATF_REQUIRE(is_root());
    } else {
        ATF_REQUIRE(!is_root());
    }
}

ATF_TEST_CASE(is_unprivileged);
ATF_TEST_CASE_HEAD(is_unprivileged)
{
    set_md_var("descr", "Tests the is_unprivileged function");
}
ATF_TEST_CASE_BODY(is_unprivileged)
{
    using atf::atf_run::is_unprivileged;

    if (::geteuid() != 0) {
        ATF_REQUIRE(is_unprivileged());
    } else {
        ATF_REQUIRE(!is_unprivileged());
    }
}

// ------------------------------------------------------------------------
// Main.
// ------------------------------------------------------------------------

ATF_INIT_TEST_CASES(tcs)
{
    // Add the tests for the free functions.
    ATF_ADD_TEST_CASE(tcs, euid);
    ATF_ADD_TEST_CASE(tcs, is_member_of_group);
    ATF_ADD_TEST_CASE(tcs, is_root);
    ATF_ADD_TEST_CASE(tcs, is_unprivileged);
}