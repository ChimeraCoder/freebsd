
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

#include "llvm/Option/Arg.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/Twine.h"
#include "llvm/Option/ArgList.h"
#include "llvm/Option/Option.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;
using namespace llvm::opt;

Arg::Arg(const Option _Opt, StringRef S, unsigned _Index, const Arg *_BaseArg)
  : Opt(_Opt), BaseArg(_BaseArg), Spelling(S), Index(_Index),
    Claimed(false), OwnsValues(false) {
}

Arg::Arg(const Option _Opt, StringRef S, unsigned _Index,
         const char *Value0, const Arg *_BaseArg)
  : Opt(_Opt), BaseArg(_BaseArg), Spelling(S), Index(_Index),
    Claimed(false), OwnsValues(false) {
  Values.push_back(Value0);
}

Arg::Arg(const Option _Opt, StringRef S, unsigned _Index,
         const char *Value0, const char *Value1, const Arg *_BaseArg)
  : Opt(_Opt), BaseArg(_BaseArg), Spelling(S), Index(_Index),
    Claimed(false), OwnsValues(false) {
  Values.push_back(Value0);
  Values.push_back(Value1);
}

Arg::~Arg() {
  if (OwnsValues) {
    for (unsigned i = 0, e = Values.size(); i != e; ++i)
      delete[] Values[i];
  }
}

void Arg::dump() const {
  llvm::errs() << "<";

  llvm::errs() << " Opt:";
  Opt.dump();

  llvm::errs() << " Index:" << Index;

  llvm::errs() << " Values: [";
  for (unsigned i = 0, e = Values.size(); i != e; ++i) {
    if (i) llvm::errs() << ", ";
    llvm::errs() << "'" << Values[i] << "'";
  }

  llvm::errs() << "]>\n";
}

std::string Arg::getAsString(const ArgList &Args) const {
  SmallString<256> Res;
  llvm::raw_svector_ostream OS(Res);

  ArgStringList ASL;
  render(Args, ASL);
  for (ArgStringList::iterator
         it = ASL.begin(), ie = ASL.end(); it != ie; ++it) {
    if (it != ASL.begin())
      OS << ' ';
    OS << *it;
  }

  return OS.str();
}

void Arg::renderAsInput(const ArgList &Args, ArgStringList &Output) const {
  if (!getOption().hasNoOptAsInput()) {
    render(Args, Output);
    return;
  }

  for (unsigned i = 0, e = getNumValues(); i != e; ++i)
    Output.push_back(getValue(i));
}

void Arg::render(const ArgList &Args, ArgStringList &Output) const {
  switch (getOption().getRenderStyle()) {
  case Option::RenderValuesStyle:
    for (unsigned i = 0, e = getNumValues(); i != e; ++i)
      Output.push_back(getValue(i));
    break;

  case Option::RenderCommaJoinedStyle: {
    SmallString<256> Res;
    llvm::raw_svector_ostream OS(Res);
    OS << getSpelling();
    for (unsigned i = 0, e = getNumValues(); i != e; ++i) {
      if (i) OS << ',';
      OS << getValue(i);
    }
    Output.push_back(Args.MakeArgString(OS.str()));
    break;
  }

 case Option::RenderJoinedStyle:
    Output.push_back(Args.GetOrMakeJoinedArgString(
                       getIndex(), getSpelling(), getValue(0)));
    for (unsigned i = 1, e = getNumValues(); i != e; ++i)
      Output.push_back(getValue(i));
    break;

  case Option::RenderSeparateStyle:
    Output.push_back(Args.MakeArgString(getSpelling()));
    for (unsigned i = 0, e = getNumValues(); i != e; ++i)
      Output.push_back(getValue(i));
    break;
  }
}