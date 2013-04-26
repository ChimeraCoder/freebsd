
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

#define DEBUG_TYPE "spiller"

#include "Spiller.h"
#include "llvm/CodeGen/LiveIntervalAnalysis.h"
#include "llvm/CodeGen/LiveRangeEdit.h"
#include "llvm/CodeGen/LiveStackAnalysis.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineLoopInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/VirtRegMap.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetInstrInfo.h"
#include "llvm/Target/TargetMachine.h"

using namespace llvm;

namespace {
  enum SpillerName { trivial, inline_ };
}

static cl::opt<SpillerName>
spillerOpt("spiller",
           cl::desc("Spiller to use: (default: standard)"),
           cl::Prefix,
           cl::values(clEnumVal(trivial,   "trivial spiller"),
                      clEnumValN(inline_,  "inline", "inline spiller"),
                      clEnumValEnd),
           cl::init(trivial));

// Spiller virtual destructor implementation.
Spiller::~Spiller() {}

namespace {

/// Utility class for spillers.
class SpillerBase : public Spiller {
protected:
  MachineFunctionPass *pass;
  MachineFunction *mf;
  VirtRegMap *vrm;
  LiveIntervals *lis;
  MachineFrameInfo *mfi;
  MachineRegisterInfo *mri;
  const TargetInstrInfo *tii;
  const TargetRegisterInfo *tri;

  /// Construct a spiller base.
  SpillerBase(MachineFunctionPass &pass, MachineFunction &mf, VirtRegMap &vrm)
    : pass(&pass), mf(&mf), vrm(&vrm)
  {
    lis = &pass.getAnalysis<LiveIntervals>();
    mfi = mf.getFrameInfo();
    mri = &mf.getRegInfo();
    tii = mf.getTarget().getInstrInfo();
    tri = mf.getTarget().getRegisterInfo();
  }

  /// Add spill ranges for every use/def of the live interval, inserting loads
  /// immediately before each use, and stores after each def. No folding or
  /// remat is attempted.
  void trivialSpillEverywhere(LiveRangeEdit& LRE) {
    LiveInterval* li = &LRE.getParent();

    DEBUG(dbgs() << "Spilling everywhere " << *li << "\n");

    assert(li->weight != HUGE_VALF &&
           "Attempting to spill already spilled value.");

    assert(!TargetRegisterInfo::isStackSlot(li->reg) &&
           "Trying to spill a stack slot.");

    DEBUG(dbgs() << "Trivial spill everywhere of reg" << li->reg << "\n");

    const TargetRegisterClass *trc = mri->getRegClass(li->reg);
    unsigned ss = vrm->assignVirt2StackSlot(li->reg);

    // Iterate over reg uses/defs.
    for (MachineRegisterInfo::reg_iterator
         regItr = mri->reg_begin(li->reg); regItr != mri->reg_end();) {

      // Grab the use/def instr.
      MachineInstr *mi = &*regItr;

      DEBUG(dbgs() << "  Processing " << *mi);

      // Step regItr to the next use/def instr.
      do {
        ++regItr;
      } while (regItr != mri->reg_end() && (&*regItr == mi));

      // Collect uses & defs for this instr.
      SmallVector<unsigned, 2> indices;
      bool hasUse = false;
      bool hasDef = false;
      for (unsigned i = 0; i != mi->getNumOperands(); ++i) {
        MachineOperand &op = mi->getOperand(i);
        if (!op.isReg() || op.getReg() != li->reg)
          continue;
        hasUse |= mi->getOperand(i).isUse();
        hasDef |= mi->getOperand(i).isDef();
        indices.push_back(i);
      }

      // Create a new vreg & interval for this instr.
      LiveInterval *newLI = &LRE.create();
      newLI->weight = HUGE_VALF;

      // Update the reg operands & kill flags.
      for (unsigned i = 0; i < indices.size(); ++i) {
        unsigned mopIdx = indices[i];
        MachineOperand &mop = mi->getOperand(mopIdx);
        mop.setReg(newLI->reg);
        if (mop.isUse() && !mi->isRegTiedToDefOperand(mopIdx)) {
          mop.setIsKill(true);
        }
      }
      assert(hasUse || hasDef);

      // Insert reload if necessary.
      MachineBasicBlock::iterator miItr(mi);
      if (hasUse) {
        tii->loadRegFromStackSlot(*mi->getParent(), miItr, newLI->reg, ss, trc,
                                  tri);
        MachineInstr *loadInstr(prior(miItr));
        SlotIndex loadIndex =
          lis->InsertMachineInstrInMaps(loadInstr).getRegSlot();
        SlotIndex endIndex = loadIndex.getNextIndex();
        VNInfo *loadVNI =
          newLI->getNextValue(loadIndex, lis->getVNInfoAllocator());
        newLI->addRange(LiveRange(loadIndex, endIndex, loadVNI));
      }

      // Insert store if necessary.
      if (hasDef) {
        tii->storeRegToStackSlot(*mi->getParent(), llvm::next(miItr),newLI->reg,
                                 true, ss, trc, tri);
        MachineInstr *storeInstr(llvm::next(miItr));
        SlotIndex storeIndex =
          lis->InsertMachineInstrInMaps(storeInstr).getRegSlot();
        SlotIndex beginIndex = storeIndex.getPrevIndex();
        VNInfo *storeVNI =
          newLI->getNextValue(beginIndex, lis->getVNInfoAllocator());
        newLI->addRange(LiveRange(beginIndex, storeIndex, storeVNI));
      }
    }
  }
};

} // end anonymous namespace

namespace {

/// Spills any live range using the spill-everywhere method with no attempt at
/// folding.
class TrivialSpiller : public SpillerBase {
public:

  TrivialSpiller(MachineFunctionPass &pass, MachineFunction &mf,
                 VirtRegMap &vrm)
    : SpillerBase(pass, mf, vrm) {}

  void spill(LiveRangeEdit &LRE) {
    // Ignore spillIs - we don't use it.
    trivialSpillEverywhere(LRE);
  }
};

} // end anonymous namespace

void Spiller::anchor() { }

llvm::Spiller* llvm::createSpiller(MachineFunctionPass &pass,
                                   MachineFunction &mf,
                                   VirtRegMap &vrm) {
  switch (spillerOpt) {
  case trivial: return new TrivialSpiller(pass, mf, vrm);
  case inline_: return createInlineSpiller(pass, mf, vrm);
  }
  llvm_unreachable("Invalid spiller optimization");
}