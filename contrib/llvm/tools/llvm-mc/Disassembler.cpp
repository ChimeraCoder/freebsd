
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

#include "Disassembler.h"
#include "llvm/ADT/OwningPtr.h"
#include "llvm/ADT/Triple.h"
#include "llvm/MC/MCDisassembler.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/MemoryObject.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

typedef std::vector<std::pair<unsigned char, const char*> > ByteArrayTy;

namespace {
class VectorMemoryObject : public MemoryObject {
private:
  const ByteArrayTy &Bytes;
public:
  VectorMemoryObject(const ByteArrayTy &bytes) : Bytes(bytes) {}

  uint64_t getBase() const { return 0; }
  uint64_t getExtent() const { return Bytes.size(); }

  int readByte(uint64_t Addr, uint8_t *Byte) const {
    if (Addr >= getExtent())
      return -1;
    *Byte = Bytes[Addr].first;
    return 0;
  }
};
}

static bool PrintInsts(const MCDisassembler &DisAsm,
                       const ByteArrayTy &Bytes,
                       SourceMgr &SM, raw_ostream &Out,
                       MCStreamer &Streamer) {
  // Wrap the vector in a MemoryObject.
  VectorMemoryObject memoryObject(Bytes);

  // Disassemble it to strings.
  uint64_t Size;
  uint64_t Index;

  for (Index = 0; Index < Bytes.size(); Index += Size) {
    MCInst Inst;

    MCDisassembler::DecodeStatus S;
    S = DisAsm.getInstruction(Inst, Size, memoryObject, Index,
                              /*REMOVE*/ nulls(), nulls());
    switch (S) {
    case MCDisassembler::Fail:
      SM.PrintMessage(SMLoc::getFromPointer(Bytes[Index].second),
                      SourceMgr::DK_Warning,
                      "invalid instruction encoding");
      if (Size == 0)
        Size = 1; // skip illegible bytes
      break;

    case MCDisassembler::SoftFail:
      SM.PrintMessage(SMLoc::getFromPointer(Bytes[Index].second),
                      SourceMgr::DK_Warning,
                      "potentially undefined instruction encoding");
      // Fall through

    case MCDisassembler::Success:
      Streamer.EmitInstruction(Inst);
      break;
    }
  }

  return false;
}

static bool ByteArrayFromString(ByteArrayTy &ByteArray,
                                StringRef &Str,
                                SourceMgr &SM) {
  while (!Str.empty()) {
    // Strip horizontal whitespace.
    if (size_t Pos = Str.find_first_not_of(" \t\r")) {
      Str = Str.substr(Pos);
      continue;
    }

    // If this is the end of a line or start of a comment, remove the rest of
    // the line.
    if (Str[0] == '\n' || Str[0] == '#') {
      // Strip to the end of line if we already processed any bytes on this
      // line.  This strips the comment and/or the \n.
      if (Str[0] == '\n') {
        Str = Str.substr(1);
      } else {
        Str = Str.substr(Str.find_first_of('\n'));
        if (!Str.empty())
          Str = Str.substr(1);
      }
      continue;
    }

    // Get the current token.
    size_t Next = Str.find_first_of(" \t\n\r#");
    StringRef Value = Str.substr(0, Next);

    // Convert to a byte and add to the byte vector.
    unsigned ByteVal;
    if (Value.getAsInteger(0, ByteVal) || ByteVal > 255) {
      // If we have an error, print it and skip to the end of line.
      SM.PrintMessage(SMLoc::getFromPointer(Value.data()), SourceMgr::DK_Error,
                      "invalid input token");
      Str = Str.substr(Str.find('\n'));
      ByteArray.clear();
      continue;
    }

    ByteArray.push_back(std::make_pair((unsigned char)ByteVal, Value.data()));
    Str = Str.substr(Next);
  }

  return false;
}

int Disassembler::disassemble(const Target &T,
                              const std::string &Triple,
                              MCSubtargetInfo &STI,
                              MCStreamer &Streamer,
                              MemoryBuffer &Buffer,
                              SourceMgr &SM,
                              raw_ostream &Out) {
  OwningPtr<const MCDisassembler> DisAsm(T.createMCDisassembler(STI));
  if (!DisAsm) {
    errs() << "error: no disassembler for target " << Triple << "\n";
    return -1;
  }

  // Set up initial section manually here
  Streamer.InitSections();

  bool ErrorOccurred = false;

  // Convert the input to a vector for disassembly.
  ByteArrayTy ByteArray;
  StringRef Str = Buffer.getBuffer();

  ErrorOccurred |= ByteArrayFromString(ByteArray, Str, SM);

  if (!ByteArray.empty())
    ErrorOccurred |= PrintInsts(*DisAsm, ByteArray, SM, Out, Streamer);

  return ErrorOccurred;
}