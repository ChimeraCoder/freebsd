
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

#include "clang/AST/CommentCommandTraits.h"
#include "llvm/ADT/STLExtras.h"

namespace clang {
namespace comments {

#include "clang/AST/CommentCommandInfo.inc"

CommandTraits::CommandTraits(llvm::BumpPtrAllocator &Allocator,
                             const CommentOptions &CommentOptions) :
    NextID(llvm::array_lengthof(Commands)), Allocator(Allocator) {
  registerCommentOptions(CommentOptions);
}

void CommandTraits::registerCommentOptions(
    const CommentOptions &CommentOptions) {
  for (CommentOptions::BlockCommandNamesTy::const_iterator
           I = CommentOptions.BlockCommandNames.begin(),
           E = CommentOptions.BlockCommandNames.end();
       I != E; I++) {
    registerBlockCommand(*I);
  }
}

const CommandInfo *CommandTraits::getCommandInfoOrNULL(StringRef Name) const {
  if (const CommandInfo *Info = getBuiltinCommandInfo(Name))
    return Info;
  return getRegisteredCommandInfo(Name);
}

const CommandInfo *CommandTraits::getCommandInfo(unsigned CommandID) const {
  if (const CommandInfo *Info = getBuiltinCommandInfo(CommandID))
    return Info;
  return getRegisteredCommandInfo(CommandID);
}

CommandInfo *CommandTraits::createCommandInfoWithName(StringRef CommandName) {
  char *Name = Allocator.Allocate<char>(CommandName.size() + 1);
  memcpy(Name, CommandName.data(), CommandName.size());
  Name[CommandName.size()] = '\0';

  // Value-initialize (=zero-initialize in this case) a new CommandInfo.
  CommandInfo *Info = new (Allocator) CommandInfo();
  Info->Name = Name;
  Info->ID = NextID++;

  RegisteredCommands.push_back(Info);

  return Info;
}

const CommandInfo *CommandTraits::registerUnknownCommand(
                                                  StringRef CommandName) {
  CommandInfo *Info = createCommandInfoWithName(CommandName);
  Info->IsUnknownCommand = true;
  return Info;
}

const CommandInfo *CommandTraits::registerBlockCommand(StringRef CommandName) {
  CommandInfo *Info = createCommandInfoWithName(CommandName);
  Info->IsBlockCommand = true;
  return Info;
}

const CommandInfo *CommandTraits::getBuiltinCommandInfo(
                                                  unsigned CommandID) {
  if (CommandID < llvm::array_lengthof(Commands))
    return &Commands[CommandID];
  return NULL;
}

const CommandInfo *CommandTraits::getRegisteredCommandInfo(
                                                  StringRef Name) const {
  for (unsigned i = 0, e = RegisteredCommands.size(); i != e; ++i) {
    if (RegisteredCommands[i]->Name == Name)
      return RegisteredCommands[i];
  }
  return NULL;
}

const CommandInfo *CommandTraits::getRegisteredCommandInfo(
                                                  unsigned CommandID) const {
  return RegisteredCommands[CommandID - llvm::array_lengthof(Commands)];
}

} // end namespace comments
} // end namespace clang