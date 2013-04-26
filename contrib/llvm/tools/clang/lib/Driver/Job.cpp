
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

#include "clang/Driver/Job.h"
#include "llvm/ADT/STLExtras.h"
#include <cassert>
using namespace clang::driver;

Job::~Job() {}

void Command::anchor() {}

Command::Command(const Action &_Source, const Tool &_Creator,
                 const char *_Executable, const ArgStringList &_Arguments)
  : Job(CommandClass), Source(_Source), Creator(_Creator),
    Executable(_Executable), Arguments(_Arguments)
{
}

JobList::JobList() : Job(JobListClass) {}

JobList::~JobList() {
  for (iterator it = begin(), ie = end(); it != ie; ++it)
    delete *it;
}

void JobList::clear() {
  DeleteContainerPointers(Jobs);
}

void Job::addCommand(Command *C) {
  cast<JobList>(this)->addJob(C);
}