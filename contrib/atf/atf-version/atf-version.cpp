
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

#if defined(HAVE_CONFIG_H)
#include "bconfig.h"
#endif

#include <cstdlib>
#include <iostream>

#include "atf-c++/detail/application.hpp"
#include "atf-c++/detail/ui.hpp"

#include "revision.h"

class atf_version : public atf::application::app {
    static const char* m_description;

public:
    atf_version(void);

    int main(void);
};

const char* atf_version::m_description =
    "atf-version is a tool that shows information about the currently "
    "installed version of ATF.";

atf_version::atf_version(void) :
    app(m_description, "atf-version(1)", "atf(7)")
{
}

int
atf_version::main(void)
{
    using atf::ui::format_text;
    using atf::ui::format_text_with_tag;

    std::cout << PACKAGE_STRING " (" PACKAGE_TARNAME "-" PACKAGE_VERSION
                 ")\n" PACKAGE_COPYRIGHT "\n\n";

#if defined(PACKAGE_REVISION_TYPE_DIST)
    std::cout << format_text("Built from a distribution file; no revision "
        "information available.") << "\n";
#elif defined(PACKAGE_REVISION_TYPE_GIT)
    std::cout << format_text_with_tag(PACKAGE_REVISION_BRANCH, "Branch: ",
                                      false) << "\n";
    std::cout << format_text_with_tag(PACKAGE_REVISION_BASE
#   if PACKAGE_REVISION_MODIFIED
        " (locally modified)"
#   endif
        " " PACKAGE_REVISION_DATE,
        "Base revision: ", false) << "\n";
#else
#   error "Unknown PACKAGE_REVISION_TYPE value"
#endif

    return EXIT_SUCCESS;
}

int
main(int argc, char* const* argv)
{
    return atf_version().run(argc, argv);
}