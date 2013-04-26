
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

#include "DiffLog.h"
#include "DiffConsumer.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Instructions.h"

using namespace llvm;

LogBuilder::~LogBuilder() {
  consumer.logf(*this);
}

StringRef LogBuilder::getFormat() const { return Format; }

unsigned LogBuilder::getNumArguments() const { return Arguments.size(); }
Value *LogBuilder::getArgument(unsigned I) const { return Arguments[I]; }

DiffLogBuilder::~DiffLogBuilder() { consumer.logd(*this); }

void DiffLogBuilder::addMatch(Instruction *L, Instruction *R) {
  Diff.push_back(DiffRecord(L, R));
}
void DiffLogBuilder::addLeft(Instruction *L) {
  // HACK: VS 2010 has a bug in the stdlib that requires this.
  Diff.push_back(DiffRecord(L, DiffRecord::second_type(0)));
}
void DiffLogBuilder::addRight(Instruction *R) {
  // HACK: VS 2010 has a bug in the stdlib that requires this.
  Diff.push_back(DiffRecord(DiffRecord::first_type(0), R));
}

unsigned DiffLogBuilder::getNumLines() const { return Diff.size(); }

DiffChange DiffLogBuilder::getLineKind(unsigned I) const {
  return (Diff[I].first ? (Diff[I].second ? DC_match : DC_left)
                        : DC_right);
}
Instruction *DiffLogBuilder::getLeft(unsigned I) const { return Diff[I].first; }
Instruction *DiffLogBuilder::getRight(unsigned I) const { return Diff[I].second; }