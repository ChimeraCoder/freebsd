
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

#include <stdexcept>

#include "expand.hpp"
#include "text.hpp"

namespace impl = atf::expand;
#define IMPL_NAME "atf::expand"

// ------------------------------------------------------------------------
// Auxiliary functions.
// ------------------------------------------------------------------------

namespace {

std::string
glob_to_regex(const std::string& glob)
{
    std::string regex;
    regex.reserve(glob.length() * 2);

    regex += '^';
    for (std::string::const_iterator iter = glob.begin(); iter != glob.end();
         iter++) {
        switch (*iter) {
        case '*': regex += ".*"; break;
        case '?': regex += "."; break;
        default: regex += *iter;
        }
    }
    regex += '$';

    return regex;
}

} // anonymous namespace

// ------------------------------------------------------------------------
// Free functions.
// ------------------------------------------------------------------------

bool
impl::is_glob(const std::string& glob)
{
    // NOTE: Keep this in sync with glob_to_regex!
    return glob.find_first_of("*?") != std::string::npos;
}

bool
impl::matches_glob(const std::string& glob, const std::string& candidate)
{
    return atf::text::match(candidate, glob_to_regex(glob));
}