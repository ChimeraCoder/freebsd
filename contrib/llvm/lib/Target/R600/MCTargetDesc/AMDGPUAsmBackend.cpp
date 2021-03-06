
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

#include "MCTargetDesc/AMDGPUMCTargetDesc.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCValue.h"
#include "llvm/Support/TargetRegistry.h"

using namespace llvm;

namespace {

class AMDGPUMCObjectWriter : public MCObjectWriter {
public:
  AMDGPUMCObjectWriter(raw_ostream &OS) : MCObjectWriter(OS, true) { }
  virtual void ExecutePostLayoutBinding(MCAssembler &Asm,
                                        const MCAsmLayout &Layout) {
    //XXX: Implement if necessary.
  }
  virtual void RecordRelocation(const MCAssembler &Asm,
                                const MCAsmLayout &Layout,
                                const MCFragment *Fragment,
                                const MCFixup &Fixup,
                                MCValue Target, uint64_t &FixedValue) {
    assert(!"Not implemented");
  }

  virtual void WriteObject(MCAssembler &Asm, const MCAsmLayout &Layout);

};

class AMDGPUAsmBackend : public MCAsmBackend {
public:
  AMDGPUAsmBackend(const Target &T)
    : MCAsmBackend() {}

  virtual AMDGPUMCObjectWriter *createObjectWriter(raw_ostream &OS) const;
  virtual unsigned getNumFixupKinds() const { return 0; };
  virtual void applyFixup(const MCFixup &Fixup, char *Data, unsigned DataSize,
                          uint64_t Value) const;
  virtual bool fixupNeedsRelaxation(const MCFixup &Fixup, uint64_t Value,
                                    const MCRelaxableFragment *DF,
                                    const MCAsmLayout &Layout) const {
    return false;
  }
  virtual void relaxInstruction(const MCInst &Inst, MCInst &Res) const {
    assert(!"Not implemented");
  }
  virtual bool mayNeedRelaxation(const MCInst &Inst) const { return false; }
  virtual bool writeNopData(uint64_t Count, MCObjectWriter *OW) const {
    return true;
  }
};

} //End anonymous namespace

void AMDGPUMCObjectWriter::WriteObject(MCAssembler &Asm,
                                       const MCAsmLayout &Layout) {
  for (MCAssembler::iterator I = Asm.begin(), E = Asm.end(); I != E; ++I) {
    Asm.writeSectionData(I, Layout);
  }
}

MCAsmBackend *llvm::createAMDGPUAsmBackend(const Target &T, StringRef TT,
                                           StringRef CPU) {
  return new AMDGPUAsmBackend(T);
}

AMDGPUMCObjectWriter * AMDGPUAsmBackend::createObjectWriter(
                                                        raw_ostream &OS) const {
  return new AMDGPUMCObjectWriter(OS);
}

void AMDGPUAsmBackend::applyFixup(const MCFixup &Fixup, char *Data,
                                  unsigned DataSize, uint64_t Value) const {

  uint16_t *Dst = (uint16_t*)(Data + Fixup.getOffset());
  assert(Fixup.getKind() == FK_PCRel_4);
  *Dst = (Value - 4) / 4;
}