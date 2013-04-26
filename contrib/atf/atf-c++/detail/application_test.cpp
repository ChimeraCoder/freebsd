
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
#include <unistd.h>
}

#include "application.hpp"

#include "../macros.hpp"

class getopt_app : public atf::application::app {
public:
    getopt_app(void) : app("description", "manpage", "other") {}

    int main(void)
    {
        // Provide an option that is unknown to the application driver and
        // one that is, together with an argument that would be swallowed by
        // the test program option if it were recognized.
        int argc = 4;
        char arg1[] = "progname";
        char arg2[] = "-Z";
        char arg3[] = "-s";
        char arg4[] = "foo";
        char *const argv[] = { arg1, arg2, arg3, arg4, NULL };

        int ch;
        bool zflag;

        // Given that this obviously is an application, and that we used the
        // same driver to start, we can test getopt(3) right here without doing
        // any fancy stuff.
        zflag = false;
        while ((ch = ::getopt(argc, argv, ":Z")) != -1) {
            switch (ch) {
            case 'Z':
                zflag = true;
                break;

            case '?':
            default:
                if (optopt != 's')
                    ATF_FAIL("Unexpected unknown option found");
            }
        }

        ATF_REQUIRE(zflag);
        ATF_REQUIRE_EQ(1, argc - optind);
        ATF_REQUIRE_EQ(std::string("foo"), argv[optind]);

        return 0;
    }
};

ATF_TEST_CASE_WITHOUT_HEAD(getopt);
ATF_TEST_CASE_BODY(getopt)
{
    int argc = 1;
    char arg1[] = "progname";
    char *const argv[] = { arg1, NULL };
    ATF_REQUIRE_EQ(0, getopt_app().run(argc, argv));
}

ATF_INIT_TEST_CASES(tcs)
{
    ATF_ADD_TEST_CASE(tcs, getopt);
}