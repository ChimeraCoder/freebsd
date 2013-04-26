
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

#include "clang/Driver/CC1AsOptions.h"
#include "clang/Driver/OptTable.h"
#include "clang/Driver/Option.h"
using namespace clang;
using namespace clang::driver;
using namespace clang::driver::options;
using namespace clang::driver::cc1asoptions;

#define PREFIX(NAME, VALUE) const char *const NAME[] = VALUE;
#define OPTION(PREFIX, NAME, ID, KIND, GROUP, ALIAS, FLAGS, PARAM, \
               HELPTEXT, METAVAR)
#include "clang/Driver/CC1AsOptions.inc"
#undef OPTION
#undef PREFIX

static const OptTable::Info CC1AsInfoTable[] = {
#define PREFIX(NAME, VALUE)
#define OPTION(PREFIX, NAME, ID, KIND, GROUP, ALIAS, FLAGS, PARAM, \
               HELPTEXT, METAVAR)   \
  { PREFIX, NAME, HELPTEXT, METAVAR, OPT_##ID, Option::KIND##Class, PARAM, \
    FLAGS, OPT_##GROUP, OPT_##ALIAS },
#include "clang/Driver/CC1AsOptions.inc"
};

namespace {

class CC1AsOptTable : public OptTable {
public:
  CC1AsOptTable()
    : OptTable(CC1AsInfoTable,
               sizeof(CC1AsInfoTable) / sizeof(CC1AsInfoTable[0])) {}
};

}

OptTable *clang::driver::createCC1AsOptTable() {
  return new CC1AsOptTable();
}