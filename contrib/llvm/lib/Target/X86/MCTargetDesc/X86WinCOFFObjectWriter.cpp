
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

#include "MCTargetDesc/X86FixupKinds.h"
#include "MCTargetDesc/X86MCTargetDesc.h"
#include "llvm/MC/MCWinCOFFObjectWriter.h"
#include "llvm/Support/COFF.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

namespace llvm {
  class MCObjectWriter;
}

namespace {
  class X86WinCOFFObjectWriter : public MCWinCOFFObjectTargetWriter {
    const bool Is64Bit;

  public:
    X86WinCOFFObjectWriter(bool Is64Bit_);
    ~X86WinCOFFObjectWriter();

    virtual unsigned getRelocType(unsigned FixupKind) const;
  };
}

X86WinCOFFObjectWriter::X86WinCOFFObjectWriter(bool Is64Bit_)
  : MCWinCOFFObjectTargetWriter(Is64Bit_ ? COFF::IMAGE_FILE_MACHINE_AMD64 :
                                COFF::IMAGE_FILE_MACHINE_I386),
    Is64Bit(Is64Bit_) {}

X86WinCOFFObjectWriter::~X86WinCOFFObjectWriter() {}

unsigned X86WinCOFFObjectWriter::getRelocType(unsigned FixupKind) const {
  switch (FixupKind) {
  case FK_PCRel_4:
  case X86::reloc_riprel_4byte:
  case X86::reloc_riprel_4byte_movq_load:
    return Is64Bit ? COFF::IMAGE_REL_AMD64_REL32 : COFF::IMAGE_REL_I386_REL32;
  case FK_Data_4:
  case X86::reloc_signed_4byte:
    return Is64Bit ? COFF::IMAGE_REL_AMD64_ADDR32 : COFF::IMAGE_REL_I386_DIR32;
  case FK_Data_8:
    if (Is64Bit)
      return COFF::IMAGE_REL_AMD64_ADDR64;
    llvm_unreachable("unsupported relocation type");
  case FK_SecRel_4:
    return Is64Bit ? COFF::IMAGE_REL_AMD64_SECREL : COFF::IMAGE_REL_I386_SECREL;
  default:
    llvm_unreachable("unsupported relocation type");
  }
}

MCObjectWriter *llvm::createX86WinCOFFObjectWriter(raw_ostream &OS,
                                                   bool Is64Bit) {
  MCWinCOFFObjectTargetWriter *MOTW = new X86WinCOFFObjectWriter(Is64Bit);
  return createWinCOFFObjectWriter(MOTW, OS);
}