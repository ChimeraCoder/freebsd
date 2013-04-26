
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

#include "MCTargetDesc/MBlazeMCTargetDesc.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCFixup.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

namespace {
  class MBlazeELFObjectWriter : public MCELFObjectTargetWriter {
  public:
    MBlazeELFObjectWriter(uint8_t OSABI);

    virtual ~MBlazeELFObjectWriter();
  protected:
    virtual unsigned GetRelocType(const MCValue &Target, const MCFixup &Fixup,
                                  bool IsPCRel, bool IsRelocWithSymbol,
                                  int64_t Addend) const;
  };
}

MBlazeELFObjectWriter::MBlazeELFObjectWriter(uint8_t OSABI)
  : MCELFObjectTargetWriter(/*Is64Bit*/ false, OSABI, ELF::EM_MBLAZE,
                            /*HasRelocationAddend*/ false) {}

MBlazeELFObjectWriter::~MBlazeELFObjectWriter() {
}

unsigned MBlazeELFObjectWriter::GetRelocType(const MCValue &Target,
                                             const MCFixup &Fixup,
                                             bool IsPCRel,
                                             bool IsRelocWithSymbol,
                                             int64_t Addend) const {
  // determine the type of the relocation
  unsigned Type;
  if (IsPCRel) {
    switch ((unsigned)Fixup.getKind()) {
    default:
      llvm_unreachable("Unimplemented");
    case FK_PCRel_4:
      Type = ELF::R_MICROBLAZE_64_PCREL;
      break;
    case FK_PCRel_2:
      Type = ELF::R_MICROBLAZE_32_PCREL;
      break;
    }
  } else {
    switch ((unsigned)Fixup.getKind()) {
    default: llvm_unreachable("invalid fixup kind!");
    case FK_Data_4:
      Type = ((IsRelocWithSymbol || Addend !=0)
              ? ELF::R_MICROBLAZE_32
              : ELF::R_MICROBLAZE_64);
      break;
    case FK_Data_2:
      Type = ELF::R_MICROBLAZE_32;
      break;
    }
  }
  return Type;
}



MCObjectWriter *llvm::createMBlazeELFObjectWriter(raw_ostream &OS,
                                                  uint8_t OSABI) {
  MCELFObjectTargetWriter *MOTW = new MBlazeELFObjectWriter(OSABI);
  return createELFObjectWriter(MOTW, OS,  /*IsLittleEndian=*/ false);
}