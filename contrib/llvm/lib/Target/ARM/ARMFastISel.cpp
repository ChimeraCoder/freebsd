
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

#include "ARM.h"
#include "ARMBaseInstrInfo.h"
#include "ARMCallingConv.h"
#include "ARMConstantPoolValue.h"
#include "ARMSubtarget.h"
#include "ARMTargetMachine.h"
#include "MCTargetDesc/ARMAddressingModes.h"
#include "llvm/CodeGen/Analysis.h"
#include "llvm/CodeGen/FastISel.h"
#include "llvm/CodeGen/FunctionLoweringInfo.h"
#include "llvm/CodeGen/MachineConstantPool.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineMemOperand.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/IR/CallingConv.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Operator.h"
#include "llvm/Support/CallSite.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/GetElementPtrTypeIterator.h"
#include "llvm/Target/TargetInstrInfo.h"
#include "llvm/Target/TargetLowering.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
using namespace llvm;

extern cl::opt<bool> EnableARMLongCalls;

namespace {

  // All possible address modes, plus some.
  typedef struct Address {
    enum {
      RegBase,
      FrameIndexBase
    } BaseType;

    union {
      unsigned Reg;
      int FI;
    } Base;

    int Offset;

    // Innocuous defaults for our address.
    Address()
     : BaseType(RegBase), Offset(0) {
       Base.Reg = 0;
     }
  } Address;

class ARMFastISel : public FastISel {

  /// Subtarget - Keep a pointer to the ARMSubtarget around so that we can
  /// make the right decision when generating code for different targets.
  const ARMSubtarget *Subtarget;
  const TargetMachine &TM;
  const TargetInstrInfo &TII;
  const TargetLowering &TLI;
  ARMFunctionInfo *AFI;

  // Convenience variables to avoid some queries.
  bool isThumb2;
  LLVMContext *Context;

  public:
    explicit ARMFastISel(FunctionLoweringInfo &funcInfo,
                         const TargetLibraryInfo *libInfo)
    : FastISel(funcInfo, libInfo),
      TM(funcInfo.MF->getTarget()),
      TII(*TM.getInstrInfo()),
      TLI(*TM.getTargetLowering()) {
      Subtarget = &TM.getSubtarget<ARMSubtarget>();
      AFI = funcInfo.MF->getInfo<ARMFunctionInfo>();
      isThumb2 = AFI->isThumbFunction();
      Context = &funcInfo.Fn->getContext();
    }

    // Code from FastISel.cpp.
  private:
    unsigned FastEmitInst_(unsigned MachineInstOpcode,
                           const TargetRegisterClass *RC);
    unsigned FastEmitInst_r(unsigned MachineInstOpcode,
                            const TargetRegisterClass *RC,
                            unsigned Op0, bool Op0IsKill);
    unsigned FastEmitInst_rr(unsigned MachineInstOpcode,
                             const TargetRegisterClass *RC,
                             unsigned Op0, bool Op0IsKill,
                             unsigned Op1, bool Op1IsKill);
    unsigned FastEmitInst_rrr(unsigned MachineInstOpcode,
                              const TargetRegisterClass *RC,
                              unsigned Op0, bool Op0IsKill,
                              unsigned Op1, bool Op1IsKill,
                              unsigned Op2, bool Op2IsKill);
    unsigned FastEmitInst_ri(unsigned MachineInstOpcode,
                             const TargetRegisterClass *RC,
                             unsigned Op0, bool Op0IsKill,
                             uint64_t Imm);
    unsigned FastEmitInst_rf(unsigned MachineInstOpcode,
                             const TargetRegisterClass *RC,
                             unsigned Op0, bool Op0IsKill,
                             const ConstantFP *FPImm);
    unsigned FastEmitInst_rri(unsigned MachineInstOpcode,
                              const TargetRegisterClass *RC,
                              unsigned Op0, bool Op0IsKill,
                              unsigned Op1, bool Op1IsKill,
                              uint64_t Imm);
    unsigned FastEmitInst_i(unsigned MachineInstOpcode,
                            const TargetRegisterClass *RC,
                            uint64_t Imm);
    unsigned FastEmitInst_ii(unsigned MachineInstOpcode,
                             const TargetRegisterClass *RC,
                             uint64_t Imm1, uint64_t Imm2);

    unsigned FastEmitInst_extractsubreg(MVT RetVT,
                                        unsigned Op0, bool Op0IsKill,
                                        uint32_t Idx);

    // Backend specific FastISel code.
  private:
    virtual bool TargetSelectInstruction(const Instruction *I);
    virtual unsigned TargetMaterializeConstant(const Constant *C);
    virtual unsigned TargetMaterializeAlloca(const AllocaInst *AI);
    virtual bool TryToFoldLoad(MachineInstr *MI, unsigned OpNo,
                               const LoadInst *LI);
    virtual bool FastLowerArguments();
  private:
  #include "ARMGenFastISel.inc"

    // Instruction selection routines.
  private:
    bool SelectLoad(const Instruction *I);
    bool SelectStore(const Instruction *I);
    bool SelectBranch(const Instruction *I);
    bool SelectIndirectBr(const Instruction *I);
    bool SelectCmp(const Instruction *I);
    bool SelectFPExt(const Instruction *I);
    bool SelectFPTrunc(const Instruction *I);
    bool SelectBinaryIntOp(const Instruction *I, unsigned ISDOpcode);
    bool SelectBinaryFPOp(const Instruction *I, unsigned ISDOpcode);
    bool SelectIToFP(const Instruction *I, bool isSigned);
    bool SelectFPToI(const Instruction *I, bool isSigned);
    bool SelectDiv(const Instruction *I, bool isSigned);
    bool SelectRem(const Instruction *I, bool isSigned);
    bool SelectCall(const Instruction *I, const char *IntrMemName);
    bool SelectIntrinsicCall(const IntrinsicInst &I);
    bool SelectSelect(const Instruction *I);
    bool SelectRet(const Instruction *I);
    bool SelectTrunc(const Instruction *I);
    bool SelectIntExt(const Instruction *I);
    bool SelectShift(const Instruction *I, ARM_AM::ShiftOpc ShiftTy);

    // Utility routines.
  private:
    bool isTypeLegal(Type *Ty, MVT &VT);
    bool isLoadTypeLegal(Type *Ty, MVT &VT);
    bool ARMEmitCmp(const Value *Src1Value, const Value *Src2Value,
                    bool isZExt);
    bool ARMEmitLoad(MVT VT, unsigned &ResultReg, Address &Addr,
                     unsigned Alignment = 0, bool isZExt = true,
                     bool allocReg = true);
    bool ARMEmitStore(MVT VT, unsigned SrcReg, Address &Addr,
                      unsigned Alignment = 0);
    bool ARMComputeAddress(const Value *Obj, Address &Addr);
    void ARMSimplifyAddress(Address &Addr, MVT VT, bool useAM3);
    bool ARMIsMemCpySmall(uint64_t Len);
    bool ARMTryEmitSmallMemCpy(Address Dest, Address Src, uint64_t Len,
                               unsigned Alignment);
    unsigned ARMEmitIntExt(MVT SrcVT, unsigned SrcReg, MVT DestVT, bool isZExt);
    unsigned ARMMaterializeFP(const ConstantFP *CFP, MVT VT);
    unsigned ARMMaterializeInt(const Constant *C, MVT VT);
    unsigned ARMMaterializeGV(const GlobalValue *GV, MVT VT);
    unsigned ARMMoveToFPReg(MVT VT, unsigned SrcReg);
    unsigned ARMMoveToIntReg(MVT VT, unsigned SrcReg);
    unsigned ARMSelectCallOp(bool UseReg);
    unsigned ARMLowerPICELF(const GlobalValue *GV, unsigned Align, MVT VT);

    // Call handling routines.
  private:
    CCAssignFn *CCAssignFnForCall(CallingConv::ID CC,
                                  bool Return,
                                  bool isVarArg);
    bool ProcessCallArgs(SmallVectorImpl<Value*> &Args,
                         SmallVectorImpl<unsigned> &ArgRegs,
                         SmallVectorImpl<MVT> &ArgVTs,
                         SmallVectorImpl<ISD::ArgFlagsTy> &ArgFlags,
                         SmallVectorImpl<unsigned> &RegArgs,
                         CallingConv::ID CC,
                         unsigned &NumBytes,
                         bool isVarArg);
    unsigned getLibcallReg(const Twine &Name);
    bool FinishCall(MVT RetVT, SmallVectorImpl<unsigned> &UsedRegs,
                    const Instruction *I, CallingConv::ID CC,
                    unsigned &NumBytes, bool isVarArg);
    bool ARMEmitLibcall(const Instruction *I, RTLIB::Libcall Call);

    // OptionalDef handling routines.
  private:
    bool isARMNEONPred(const MachineInstr *MI);
    bool DefinesOptionalPredicate(MachineInstr *MI, bool *CPSR);
    const MachineInstrBuilder &AddOptionalDefs(const MachineInstrBuilder &MIB);
    void AddLoadStoreOperands(MVT VT, Address &Addr,
                              const MachineInstrBuilder &MIB,
                              unsigned Flags, bool useAM3);
};

} // end anonymous namespace

#include "ARMGenCallingConv.inc"

// DefinesOptionalPredicate - This is different from DefinesPredicate in that
// we don't care about implicit defs here, just places we'll need to add a
// default CCReg argument. Sets CPSR if we're setting CPSR instead of CCR.
bool ARMFastISel::DefinesOptionalPredicate(MachineInstr *MI, bool *CPSR) {
  if (!MI->hasOptionalDef())
    return false;

  // Look to see if our OptionalDef is defining CPSR or CCR.
  for (unsigned i = 0, e = MI->getNumOperands(); i != e; ++i) {
    const MachineOperand &MO = MI->getOperand(i);
    if (!MO.isReg() || !MO.isDef()) continue;
    if (MO.getReg() == ARM::CPSR)
      *CPSR = true;
  }
  return true;
}

bool ARMFastISel::isARMNEONPred(const MachineInstr *MI) {
  const MCInstrDesc &MCID = MI->getDesc();

  // If we're a thumb2 or not NEON function we were handled via isPredicable.
  if ((MCID.TSFlags & ARMII::DomainMask) != ARMII::DomainNEON ||
       AFI->isThumb2Function())
    return false;

  for (unsigned i = 0, e = MCID.getNumOperands(); i != e; ++i)
    if (MCID.OpInfo[i].isPredicate())
      return true;

  return false;
}

// If the machine is predicable go ahead and add the predicate operands, if
// it needs default CC operands add those.
// TODO: If we want to support thumb1 then we'll need to deal with optional
// CPSR defs that need to be added before the remaining operands. See s_cc_out
// for descriptions why.
const MachineInstrBuilder &
ARMFastISel::AddOptionalDefs(const MachineInstrBuilder &MIB) {
  MachineInstr *MI = &*MIB;

  // Do we use a predicate? or...
  // Are we NEON in ARM mode and have a predicate operand? If so, I know
  // we're not predicable but add it anyways.
  if (TII.isPredicable(MI) || isARMNEONPred(MI))
    AddDefaultPred(MIB);

  // Do we optionally set a predicate?  Preds is size > 0 iff the predicate
  // defines CPSR. All other OptionalDefines in ARM are the CCR register.
  bool CPSR = false;
  if (DefinesOptionalPredicate(MI, &CPSR)) {
    if (CPSR)
      AddDefaultT1CC(MIB);
    else
      AddDefaultCC(MIB);
  }
  return MIB;
}

unsigned ARMFastISel::FastEmitInst_(unsigned MachineInstOpcode,
                                    const TargetRegisterClass* RC) {
  unsigned ResultReg = createResultReg(RC);
  const MCInstrDesc &II = TII.get(MachineInstOpcode);

  AddOptionalDefs(BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, II, ResultReg));
  return ResultReg;
}

unsigned ARMFastISel::FastEmitInst_r(unsigned MachineInstOpcode,
                                     const TargetRegisterClass *RC,
                                     unsigned Op0, bool Op0IsKill) {
  unsigned ResultReg = createResultReg(RC);
  const MCInstrDesc &II = TII.get(MachineInstOpcode);

  if (II.getNumDefs() >= 1) {
    AddOptionalDefs(BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, II, ResultReg)
                   .addReg(Op0, Op0IsKill * RegState::Kill));
  } else {
    AddOptionalDefs(BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, II)
                   .addReg(Op0, Op0IsKill * RegState::Kill));
    AddOptionalDefs(BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL,
                   TII.get(TargetOpcode::COPY), ResultReg)
                   .addReg(II.ImplicitDefs[0]));
  }
  return ResultReg;
}

unsigned ARMFastISel::FastEmitInst_rr(unsigned MachineInstOpcode,
                                      const TargetRegisterClass *RC,
                                      unsigned Op0, bool Op0IsKill,
                                      unsigned Op1, bool Op1IsKill) {
  unsigned ResultReg = createResultReg(RC);
  const MCInstrDesc &II = TII.get(MachineInstOpcode);

  if (II.getNumDefs() >= 1) {
    AddOptionalDefs(BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, II, ResultReg)
                   .addReg(Op0, Op0IsKill * RegState::Kill)
                   .addReg(Op1, Op1IsKill * RegState::Kill));
  } else {
    AddOptionalDefs(BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, II)
                   .addReg(Op0, Op0IsKill * RegState::Kill)
                   .addReg(Op1, Op1IsKill * RegState::Kill));
    AddOptionalDefs(BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL,
                           TII.get(TargetOpcode::COPY), ResultReg)
                   .addReg(II.ImplicitDefs[0]));
  }
  return ResultReg;
}

unsigned ARMFastISel::FastEmitInst_rrr(unsigned MachineInstOpcode,
                                       const TargetRegisterClass *RC,
                                       unsigned Op0, bool Op0IsKill,
                                       unsigned Op1, bool Op1IsKill,
                                       unsigned Op2, bool Op2IsKill) {
  unsigned ResultReg = createResultReg(RC);
  const MCInstrDesc &II = TII.get(MachineInstOpcode);

  if (II.getNumDefs() >= 1) {
    AddOptionalDefs(BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, II, ResultReg)
                   .addReg(Op0, Op0IsKill * RegState::Kill)
                   .addReg(Op1, Op1IsKill * RegState::Kill)
                   .addReg(Op2, Op2IsKill * RegState::Kill));
  } else {
    AddOptionalDefs(BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, II)
                   .addReg(Op0, Op0IsKill * RegState::Kill)
                   .addReg(Op1, Op1IsKill * RegState::Kill)
                   .addReg(Op2, Op2IsKill * RegState::Kill));
    AddOptionalDefs(BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL,
                           TII.get(TargetOpcode::COPY), ResultReg)
                   .addReg(II.ImplicitDefs[0]));
  }
  return ResultReg;
}

unsigned ARMFastISel::FastEmitInst_ri(unsigned MachineInstOpcode,
                                      const TargetRegisterClass *RC,
                                      unsigned Op0, bool Op0IsKill,
                                      uint64_t Imm) {
  unsigned ResultReg = createResultReg(RC);
  const MCInstrDesc &II = TII.get(MachineInstOpcode);

  if (II.getNumDefs() >= 1) {
    AddOptionalDefs(BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, II, ResultReg)
                   .addReg(Op0, Op0IsKill * RegState::Kill)
                   .addImm(Imm));
  } else {
    AddOptionalDefs(BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, II)
                   .addReg(Op0, Op0IsKill * RegState::Kill)
                   .addImm(Imm));
    AddOptionalDefs(BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL,
                           TII.get(TargetOpcode::COPY), ResultReg)
                   .addReg(II.ImplicitDefs[0]));
  }
  return ResultReg;
}

unsigned ARMFastISel::FastEmitInst_rf(unsigned MachineInstOpcode,
                                      const TargetRegisterClass *RC,
                                      unsigned Op0, bool Op0IsKill,
                                      const ConstantFP *FPImm) {
  unsigned ResultReg = createResultReg(RC);
  const MCInstrDesc &II = TII.get(MachineInstOpcode);

  if (II.getNumDefs() >= 1) {
    AddOptionalDefs(BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, II, ResultReg)
                   .addReg(Op0, Op0IsKill * RegState::Kill)
                   .addFPImm(FPImm));
  } else {
    AddOptionalDefs(BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, II)
                   .addReg(Op0, Op0IsKill * RegState::Kill)
                   .addFPImm(FPImm));
    AddOptionalDefs(BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL,
                           TII.get(TargetOpcode::COPY), ResultReg)
                   .addReg(II.ImplicitDefs[0]));
  }
  return ResultReg;
}

unsigned ARMFastISel::FastEmitInst_rri(unsigned MachineInstOpcode,
                                       const TargetRegisterClass *RC,
                                       unsigned Op0, bool Op0IsKill,
                                       unsigned Op1, bool Op1IsKill,
                                       uint64_t Imm) {
  unsigned ResultReg = createResultReg(RC);
  const MCInstrDesc &II = TII.get(MachineInstOpcode);

  if (II.getNumDefs() >= 1) {
    AddOptionalDefs(BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, II, ResultReg)
                   .addReg(Op0, Op0IsKill * RegState::Kill)
                   .addReg(Op1, Op1IsKill * RegState::Kill)
                   .addImm(Imm));
  } else {
    AddOptionalDefs(BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, II)
                   .addReg(Op0, Op0IsKill * RegState::Kill)
                   .addReg(Op1, Op1IsKill * RegState::Kill)
                   .addImm(Imm));
    AddOptionalDefs(BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL,
                           TII.get(TargetOpcode::COPY), ResultReg)
                   .addReg(II.ImplicitDefs[0]));
  }
  return ResultReg;
}

unsigned ARMFastISel::FastEmitInst_i(unsigned MachineInstOpcode,
                                     const TargetRegisterClass *RC,
                                     uint64_t Imm) {
  unsigned ResultReg = createResultReg(RC);
  const MCInstrDesc &II = TII.get(MachineInstOpcode);

  if (II.getNumDefs() >= 1) {
    AddOptionalDefs(BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, II, ResultReg)
                   .addImm(Imm));
  } else {
    AddOptionalDefs(BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, II)
                   .addImm(Imm));
    AddOptionalDefs(BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL,
                           TII.get(TargetOpcode::COPY), ResultReg)
                   .addReg(II.ImplicitDefs[0]));
  }
  return ResultReg;
}

unsigned ARMFastISel::FastEmitInst_ii(unsigned MachineInstOpcode,
                                      const TargetRegisterClass *RC,
                                      uint64_t Imm1, uint64_t Imm2) {
  unsigned ResultReg = createResultReg(RC);
  const MCInstrDesc &II = TII.get(MachineInstOpcode);

  if (II.getNumDefs() >= 1) {
    AddOptionalDefs(BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, II, ResultReg)
                    .addImm(Imm1).addImm(Imm2));
  } else {
    AddOptionalDefs(BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, II)
                    .addImm(Imm1).addImm(Imm2));
    AddOptionalDefs(BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL,
                            TII.get(TargetOpcode::COPY),
                            ResultReg)
                    .addReg(II.ImplicitDefs[0]));
  }
  return ResultReg;
}

unsigned ARMFastISel::FastEmitInst_extractsubreg(MVT RetVT,
                                                 unsigned Op0, bool Op0IsKill,
                                                 uint32_t Idx) {
  unsigned ResultReg = createResultReg(TLI.getRegClassFor(RetVT));
  assert(TargetRegisterInfo::isVirtualRegister(Op0) &&
         "Cannot yet extract from physregs");

  AddOptionalDefs(BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt,
                          DL, TII.get(TargetOpcode::COPY), ResultReg)
                  .addReg(Op0, getKillRegState(Op0IsKill), Idx));
  return ResultReg;
}

// TODO: Don't worry about 64-bit now, but when this is fixed remove the
// checks from the various callers.
unsigned ARMFastISel::ARMMoveToFPReg(MVT VT, unsigned SrcReg) {
  if (VT == MVT::f64) return 0;

  unsigned MoveReg = createResultReg(TLI.getRegClassFor(VT));
  AddOptionalDefs(BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL,
                          TII.get(ARM::VMOVSR), MoveReg)
                  .addReg(SrcReg));
  return MoveReg;
}

unsigned ARMFastISel::ARMMoveToIntReg(MVT VT, unsigned SrcReg) {
  if (VT == MVT::i64) return 0;

  unsigned MoveReg = createResultReg(TLI.getRegClassFor(VT));
  AddOptionalDefs(BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL,
                          TII.get(ARM::VMOVRS), MoveReg)
                  .addReg(SrcReg));
  return MoveReg;
}

// For double width floating point we need to materialize two constants
// (the high and the low) into integer registers then use a move to get
// the combined constant into an FP reg.
unsigned ARMFastISel::ARMMaterializeFP(const ConstantFP *CFP, MVT VT) {
  const APFloat Val = CFP->getValueAPF();
  bool is64bit = VT == MVT::f64;

  // This checks to see if we can use VFP3 instructions to materialize
  // a constant, otherwise we have to go through the constant pool.
  if (TLI.isFPImmLegal(Val, VT)) {
    int Imm;
    unsigned Opc;
    if (is64bit) {
      Imm = ARM_AM::getFP64Imm(Val);
      Opc = ARM::FCONSTD;
    } else {
      Imm = ARM_AM::getFP32Imm(Val);
      Opc = ARM::FCONSTS;
    }
    unsigned DestReg = createResultReg(TLI.getRegClassFor(VT));
    AddOptionalDefs(BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, TII.get(Opc),
                            DestReg)
                    .addImm(Imm));
    return DestReg;
  }

  // Require VFP2 for loading fp constants.
  if (!Subtarget->hasVFP2()) return false;

  // MachineConstantPool wants an explicit alignment.
  unsigned Align = TD.getPrefTypeAlignment(CFP->getType());
  if (Align == 0) {
    // TODO: Figure out if this is correct.
    Align = TD.getTypeAllocSize(CFP->getType());
  }
  unsigned Idx = MCP.getConstantPoolIndex(cast<Constant>(CFP), Align);
  unsigned DestReg = createResultReg(TLI.getRegClassFor(VT));
  unsigned Opc = is64bit ? ARM::VLDRD : ARM::VLDRS;

  // The extra reg is for addrmode5.
  AddOptionalDefs(BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, TII.get(Opc),
                          DestReg)
                  .addConstantPoolIndex(Idx)
                  .addReg(0));
  return DestReg;
}

unsigned ARMFastISel::ARMMaterializeInt(const Constant *C, MVT VT) {

  if (VT != MVT::i32 && VT != MVT::i16 && VT != MVT::i8 && VT != MVT::i1)
    return false;

  // If we can do this in a single instruction without a constant pool entry
  // do so now.
  const ConstantInt *CI = cast<ConstantInt>(C);
  if (Subtarget->hasV6T2Ops() && isUInt<16>(CI->getZExtValue())) {
    unsigned Opc = isThumb2 ? ARM::t2MOVi16 : ARM::MOVi16;
    const TargetRegisterClass *RC = isThumb2 ? &ARM::rGPRRegClass :
      &ARM::GPRRegClass;
    unsigned ImmReg = createResultReg(RC);
    AddOptionalDefs(BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL,
                            TII.get(Opc), ImmReg)
                    .addImm(CI->getZExtValue()));
    return ImmReg;
  }

  // Use MVN to emit negative constants.
  if (VT == MVT::i32 && Subtarget->hasV6T2Ops() && CI->isNegative()) {
    unsigned Imm = (unsigned)~(CI->getSExtValue());
    bool UseImm = isThumb2 ? (ARM_AM::getT2SOImmVal(Imm) != -1) :
      (ARM_AM::getSOImmVal(Imm) != -1);
    if (UseImm) {
      unsigned Opc = isThumb2 ? ARM::t2MVNi : ARM::MVNi;
      unsigned ImmReg = createResultReg(TLI.getRegClassFor(MVT::i32));
      AddOptionalDefs(BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL,
                              TII.get(Opc), ImmReg)
                      .addImm(Imm));
      return ImmReg;
    }
  }

  // Load from constant pool.  For now 32-bit only.
  if (VT != MVT::i32)
    return false;

  unsigned DestReg = createResultReg(TLI.getRegClassFor(VT));

  // MachineConstantPool wants an explicit alignment.
  unsigned Align = TD.getPrefTypeAlignment(C->getType());
  if (Align == 0) {
    // TODO: Figure out if this is correct.
    Align = TD.getTypeAllocSize(C->getType());
  }
  unsigned Idx = MCP.getConstantPoolIndex(C, Align);

  if (isThumb2)
    AddOptionalDefs(BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL,
                            TII.get(ARM::t2LDRpci), DestReg)
                    .addConstantPoolIndex(Idx));
  else
    // The extra immediate is for addrmode2.
    AddOptionalDefs(BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL,
                            TII.get(ARM::LDRcp), DestReg)
                    .addConstantPoolIndex(Idx)
                    .addImm(0));

  return DestReg;
}

unsigned ARMFastISel::ARMMaterializeGV(const GlobalValue *GV, MVT VT) {
  // For now 32-bit only.
  if (VT != MVT::i32) return 0;

  Reloc::Model RelocM = TM.getRelocationModel();
  bool IsIndirect = Subtarget->GVIsIndirectSymbol(GV, RelocM);
  const TargetRegisterClass *RC = isThumb2 ?
    (const TargetRegisterClass*)&ARM::rGPRRegClass :
    (const TargetRegisterClass*)&ARM::GPRRegClass;
  unsigned DestReg = createResultReg(RC);

  // Use movw+movt when possible, it avoids constant pool entries.
  // Darwin targets don't support movt with Reloc::Static, see
  // ARMTargetLowering::LowerGlobalAddressDarwin.  Other targets only support
  // static movt relocations.
  if (Subtarget->useMovt() &&
      Subtarget->isTargetDarwin() == (RelocM != Reloc::Static)) {
    unsigned Opc;
    switch (RelocM) {
    case Reloc::PIC_:
      Opc = isThumb2 ? ARM::t2MOV_ga_pcrel : ARM::MOV_ga_pcrel;
      break;
    case Reloc::DynamicNoPIC:
      Opc = isThumb2 ? ARM::t2MOV_ga_dyn : ARM::MOV_ga_dyn;
      break;
    default:
      Opc = isThumb2 ? ARM::t2MOVi32imm : ARM::MOVi32imm;
      break;
    }
    AddOptionalDefs(BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, TII.get(Opc),
                            DestReg).addGlobalAddress(GV));
  } else {
    // MachineConstantPool wants an explicit alignment.
    unsigned Align = TD.getPrefTypeAlignment(GV->getType());
    if (Align == 0) {
      // TODO: Figure out if this is correct.
      Align = TD.getTypeAllocSize(GV->getType());
    }

    if (Subtarget->isTargetELF() && RelocM == Reloc::PIC_)
      return ARMLowerPICELF(GV, Align, VT);

    // Grab index.
    unsigned PCAdj = (RelocM != Reloc::PIC_) ? 0 :
      (Subtarget->isThumb() ? 4 : 8);
    unsigned Id = AFI->createPICLabelUId();
    ARMConstantPoolValue *CPV = ARMConstantPoolConstant::Create(GV, Id,
                                                                ARMCP::CPValue,
                                                                PCAdj);
    unsigned Idx = MCP.getConstantPoolIndex(CPV, Align);

    // Load value.
    MachineInstrBuilder MIB;
    if (isThumb2) {
      unsigned Opc = (RelocM!=Reloc::PIC_) ? ARM::t2LDRpci : ARM::t2LDRpci_pic;
      MIB = BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, TII.get(Opc), DestReg)
        .addConstantPoolIndex(Idx);
      if (RelocM == Reloc::PIC_)
        MIB.addImm(Id);
      AddOptionalDefs(MIB);
    } else {
      // The extra immediate is for addrmode2.
      MIB = BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, TII.get(ARM::LDRcp),
                    DestReg)
        .addConstantPoolIndex(Idx)
        .addImm(0);
      AddOptionalDefs(MIB);

      if (RelocM == Reloc::PIC_) {
        unsigned Opc = IsIndirect ? ARM::PICLDR : ARM::PICADD;
        unsigned NewDestReg = createResultReg(TLI.getRegClassFor(VT));

        MachineInstrBuilder MIB = BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt,
                                          DL, TII.get(Opc), NewDestReg)
                                  .addReg(DestReg)
                                  .addImm(Id);
        AddOptionalDefs(MIB);
        return NewDestReg;
      }
    }
  }

  if (IsIndirect) {
    MachineInstrBuilder MIB;
    unsigned NewDestReg = createResultReg(TLI.getRegClassFor(VT));
    if (isThumb2)
      MIB = BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL,
                    TII.get(ARM::t2LDRi12), NewDestReg)
            .addReg(DestReg)
            .addImm(0);
    else
      MIB = BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, TII.get(ARM::LDRi12),
                    NewDestReg)
            .addReg(DestReg)
            .addImm(0);
    DestReg = NewDestReg;
    AddOptionalDefs(MIB);
  }

  return DestReg;
}

unsigned ARMFastISel::TargetMaterializeConstant(const Constant *C) {
  EVT CEVT = TLI.getValueType(C->getType(), true);

  // Only handle simple types.
  if (!CEVT.isSimple()) return 0;
  MVT VT = CEVT.getSimpleVT();

  if (const ConstantFP *CFP = dyn_cast<ConstantFP>(C))
    return ARMMaterializeFP(CFP, VT);
  else if (const GlobalValue *GV = dyn_cast<GlobalValue>(C))
    return ARMMaterializeGV(GV, VT);
  else if (isa<ConstantInt>(C))
    return ARMMaterializeInt(C, VT);

  return 0;
}

// TODO: unsigned ARMFastISel::TargetMaterializeFloatZero(const ConstantFP *CF);

unsigned ARMFastISel::TargetMaterializeAlloca(const AllocaInst *AI) {
  // Don't handle dynamic allocas.
  if (!FuncInfo.StaticAllocaMap.count(AI)) return 0;

  MVT VT;
  if (!isLoadTypeLegal(AI->getType(), VT)) return 0;

  DenseMap<const AllocaInst*, int>::iterator SI =
    FuncInfo.StaticAllocaMap.find(AI);

  // This will get lowered later into the correct offsets and registers
  // via rewriteXFrameIndex.
  if (SI != FuncInfo.StaticAllocaMap.end()) {
    const TargetRegisterClass* RC = TLI.getRegClassFor(VT);
    unsigned ResultReg = createResultReg(RC);
    unsigned Opc = isThumb2 ? ARM::t2ADDri : ARM::ADDri;
    AddOptionalDefs(BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL,
                            TII.get(Opc), ResultReg)
                            .addFrameIndex(SI->second)
                            .addImm(0));
    return ResultReg;
  }

  return 0;
}

bool ARMFastISel::isTypeLegal(Type *Ty, MVT &VT) {
  EVT evt = TLI.getValueType(Ty, true);

  // Only handle simple types.
  if (evt == MVT::Other || !evt.isSimple()) return false;
  VT = evt.getSimpleVT();

  // Handle all legal types, i.e. a register that will directly hold this
  // value.
  return TLI.isTypeLegal(VT);
}

bool ARMFastISel::isLoadTypeLegal(Type *Ty, MVT &VT) {
  if (isTypeLegal(Ty, VT)) return true;

  // If this is a type than can be sign or zero-extended to a basic operation
  // go ahead and accept it now.
  if (VT == MVT::i1 || VT == MVT::i8 || VT == MVT::i16)
    return true;

  return false;
}

// Computes the address to get to an object.
bool ARMFastISel::ARMComputeAddress(const Value *Obj, Address &Addr) {
  // Some boilerplate from the X86 FastISel.
  const User *U = NULL;
  unsigned Opcode = Instruction::UserOp1;
  if (const Instruction *I = dyn_cast<Instruction>(Obj)) {
    // Don't walk into other basic blocks unless the object is an alloca from
    // another block, otherwise it may not have a virtual register assigned.
    if (FuncInfo.StaticAllocaMap.count(static_cast<const AllocaInst *>(Obj)) ||
        FuncInfo.MBBMap[I->getParent()] == FuncInfo.MBB) {
      Opcode = I->getOpcode();
      U = I;
    }
  } else if (const ConstantExpr *C = dyn_cast<ConstantExpr>(Obj)) {
    Opcode = C->getOpcode();
    U = C;
  }

  if (PointerType *Ty = dyn_cast<PointerType>(Obj->getType()))
    if (Ty->getAddressSpace() > 255)
      // Fast instruction selection doesn't support the special
      // address spaces.
      return false;

  switch (Opcode) {
    default:
    break;
    case Instruction::BitCast: {
      // Look through bitcasts.
      return ARMComputeAddress(U->getOperand(0), Addr);
    }
    case Instruction::IntToPtr: {
      // Look past no-op inttoptrs.
      if (TLI.getValueType(U->getOperand(0)->getType()) == TLI.getPointerTy())
        return ARMComputeAddress(U->getOperand(0), Addr);
      break;
    }
    case Instruction::PtrToInt: {
      // Look past no-op ptrtoints.
      if (TLI.getValueType(U->getType()) == TLI.getPointerTy())
        return ARMComputeAddress(U->getOperand(0), Addr);
      break;
    }
    case Instruction::GetElementPtr: {
      Address SavedAddr = Addr;
      int TmpOffset = Addr.Offset;

      // Iterate through the GEP folding the constants into offsets where
      // we can.
      gep_type_iterator GTI = gep_type_begin(U);
      for (User::const_op_iterator i = U->op_begin() + 1, e = U->op_end();
           i != e; ++i, ++GTI) {
        const Value *Op = *i;
        if (StructType *STy = dyn_cast<StructType>(*GTI)) {
          const StructLayout *SL = TD.getStructLayout(STy);
          unsigned Idx = cast<ConstantInt>(Op)->getZExtValue();
          TmpOffset += SL->getElementOffset(Idx);
        } else {
          uint64_t S = TD.getTypeAllocSize(GTI.getIndexedType());
          for (;;) {
            if (const ConstantInt *CI = dyn_cast<ConstantInt>(Op)) {
              // Constant-offset addressing.
              TmpOffset += CI->getSExtValue() * S;
              break;
            }
            if (isa<AddOperator>(Op) &&
                (!isa<Instruction>(Op) ||
                 FuncInfo.MBBMap[cast<Instruction>(Op)->getParent()]
                 == FuncInfo.MBB) &&
                isa<ConstantInt>(cast<AddOperator>(Op)->getOperand(1))) {
              // An add (in the same block) with a constant operand. Fold the
              // constant.
              ConstantInt *CI =
              cast<ConstantInt>(cast<AddOperator>(Op)->getOperand(1));
              TmpOffset += CI->getSExtValue() * S;
              // Iterate on the other operand.
              Op = cast<AddOperator>(Op)->getOperand(0);
              continue;
            }
            // Unsupported
            goto unsupported_gep;
          }
        }
      }

      // Try to grab the base operand now.
      Addr.Offset = TmpOffset;
      if (ARMComputeAddress(U->getOperand(0), Addr)) return true;

      // We failed, restore everything and try the other options.
      Addr = SavedAddr;

      unsupported_gep:
      break;
    }
    case Instruction::Alloca: {
      const AllocaInst *AI = cast<AllocaInst>(Obj);
      DenseMap<const AllocaInst*, int>::iterator SI =
        FuncInfo.StaticAllocaMap.find(AI);
      if (SI != FuncInfo.StaticAllocaMap.end()) {
        Addr.BaseType = Address::FrameIndexBase;
        Addr.Base.FI = SI->second;
        return true;
      }
      break;
    }
  }

  // Try to get this in a register if nothing else has worked.
  if (Addr.Base.Reg == 0) Addr.Base.Reg = getRegForValue(Obj);
  return Addr.Base.Reg != 0;
}

void ARMFastISel::ARMSimplifyAddress(Address &Addr, MVT VT, bool useAM3) {
  bool needsLowering = false;
  switch (VT.SimpleTy) {
    default: llvm_unreachable("Unhandled load/store type!");
    case MVT::i1:
    case MVT::i8:
    case MVT::i16:
    case MVT::i32:
      if (!useAM3) {
        // Integer loads/stores handle 12-bit offsets.
        needsLowering = ((Addr.Offset & 0xfff) != Addr.Offset);
        // Handle negative offsets.
        if (needsLowering && isThumb2)
          needsLowering = !(Subtarget->hasV6T2Ops() && Addr.Offset < 0 &&
                            Addr.Offset > -256);
      } else {
        // ARM halfword load/stores and signed byte loads use +/-imm8 offsets.
        needsLowering = (Addr.Offset > 255 || Addr.Offset < -255);
      }
      break;
    case MVT::f32:
    case MVT::f64:
      // Floating point operands handle 8-bit offsets.
      needsLowering = ((Addr.Offset & 0xff) != Addr.Offset);
      break;
  }

  // If this is a stack pointer and the offset needs to be simplified then
  // put the alloca address into a register, set the base type back to
  // register and continue. This should almost never happen.
  if (needsLowering && Addr.BaseType == Address::FrameIndexBase) {
    const TargetRegisterClass *RC = isThumb2 ?
      (const TargetRegisterClass*)&ARM::tGPRRegClass :
      (const TargetRegisterClass*)&ARM::GPRRegClass;
    unsigned ResultReg = createResultReg(RC);
    unsigned Opc = isThumb2 ? ARM::t2ADDri : ARM::ADDri;
    AddOptionalDefs(BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL,
                            TII.get(Opc), ResultReg)
                            .addFrameIndex(Addr.Base.FI)
                            .addImm(0));
    Addr.Base.Reg = ResultReg;
    Addr.BaseType = Address::RegBase;
  }

  // Since the offset is too large for the load/store instruction
  // get the reg+offset into a register.
  if (needsLowering) {
    Addr.Base.Reg = FastEmit_ri_(MVT::i32, ISD::ADD, Addr.Base.Reg,
                                 /*Op0IsKill*/false, Addr.Offset, MVT::i32);
    Addr.Offset = 0;
  }
}

void ARMFastISel::AddLoadStoreOperands(MVT VT, Address &Addr,
                                       const MachineInstrBuilder &MIB,
                                       unsigned Flags, bool useAM3) {
  // addrmode5 output depends on the selection dag addressing dividing the
  // offset by 4 that it then later multiplies. Do this here as well.
  if (VT.SimpleTy == MVT::f32 || VT.SimpleTy == MVT::f64)
    Addr.Offset /= 4;

  // Frame base works a bit differently. Handle it separately.
  if (Addr.BaseType == Address::FrameIndexBase) {
    int FI = Addr.Base.FI;
    int Offset = Addr.Offset;
    MachineMemOperand *MMO =
          FuncInfo.MF->getMachineMemOperand(
                                  MachinePointerInfo::getFixedStack(FI, Offset),
                                  Flags,
                                  MFI.getObjectSize(FI),
                                  MFI.getObjectAlignment(FI));
    // Now add the rest of the operands.
    MIB.addFrameIndex(FI);

    // ARM halfword load/stores and signed byte loads need an additional
    // operand.
    if (useAM3) {
      signed Imm = (Addr.Offset < 0) ? (0x100 | -Addr.Offset) : Addr.Offset;
      MIB.addReg(0);
      MIB.addImm(Imm);
    } else {
      MIB.addImm(Addr.Offset);
    }
    MIB.addMemOperand(MMO);
  } else {
    // Now add the rest of the operands.
    MIB.addReg(Addr.Base.Reg);

    // ARM halfword load/stores and signed byte loads need an additional
    // operand.
    if (useAM3) {
      signed Imm = (Addr.Offset < 0) ? (0x100 | -Addr.Offset) : Addr.Offset;
      MIB.addReg(0);
      MIB.addImm(Imm);
    } else {
      MIB.addImm(Addr.Offset);
    }
  }
  AddOptionalDefs(MIB);
}

bool ARMFastISel::ARMEmitLoad(MVT VT, unsigned &ResultReg, Address &Addr,
                              unsigned Alignment, bool isZExt, bool allocReg) {
  unsigned Opc;
  bool useAM3 = false;
  bool needVMOV = false;
  const TargetRegisterClass *RC;
  switch (VT.SimpleTy) {
    // This is mostly going to be Neon/vector support.
    default: return false;
    case MVT::i1:
    case MVT::i8:
      if (isThumb2) {
        if (Addr.Offset < 0 && Addr.Offset > -256 && Subtarget->hasV6T2Ops())
          Opc = isZExt ? ARM::t2LDRBi8 : ARM::t2LDRSBi8;
        else
          Opc = isZExt ? ARM::t2LDRBi12 : ARM::t2LDRSBi12;
      } else {
        if (isZExt) {
          Opc = ARM::LDRBi12;
        } else {
          Opc = ARM::LDRSB;
          useAM3 = true;
        }
      }
      RC = &ARM::GPRRegClass;
      break;
    case MVT::i16:
      if (Alignment && Alignment < 2 && !Subtarget->allowsUnalignedMem())
        return false;

      if (isThumb2) {
        if (Addr.Offset < 0 && Addr.Offset > -256 && Subtarget->hasV6T2Ops())
          Opc = isZExt ? ARM::t2LDRHi8 : ARM::t2LDRSHi8;
        else
          Opc = isZExt ? ARM::t2LDRHi12 : ARM::t2LDRSHi12;
      } else {
        Opc = isZExt ? ARM::LDRH : ARM::LDRSH;
        useAM3 = true;
      }
      RC = &ARM::GPRRegClass;
      break;
    case MVT::i32:
      if (Alignment && Alignment < 4 && !Subtarget->allowsUnalignedMem())
        return false;

      if (isThumb2) {
        if (Addr.Offset < 0 && Addr.Offset > -256 && Subtarget->hasV6T2Ops())
          Opc = ARM::t2LDRi8;
        else
          Opc = ARM::t2LDRi12;
      } else {
        Opc = ARM::LDRi12;
      }
      RC = &ARM::GPRRegClass;
      break;
    case MVT::f32:
      if (!Subtarget->hasVFP2()) return false;
      // Unaligned loads need special handling. Floats require word-alignment.
      if (Alignment && Alignment < 4) {
        needVMOV = true;
        VT = MVT::i32;
        Opc = isThumb2 ? ARM::t2LDRi12 : ARM::LDRi12;
        RC = &ARM::GPRRegClass;
      } else {
        Opc = ARM::VLDRS;
        RC = TLI.getRegClassFor(VT);
      }
      break;
    case MVT::f64:
      if (!Subtarget->hasVFP2()) return false;
      // FIXME: Unaligned loads need special handling.  Doublewords require
      // word-alignment.
      if (Alignment && Alignment < 4)
        return false;

      Opc = ARM::VLDRD;
      RC = TLI.getRegClassFor(VT);
      break;
  }
  // Simplify this down to something we can handle.
  ARMSimplifyAddress(Addr, VT, useAM3);

  // Create the base instruction, then add the operands.
  if (allocReg)
    ResultReg = createResultReg(RC);
  assert (ResultReg > 255 && "Expected an allocated virtual register.");
  MachineInstrBuilder MIB = BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL,
                                    TII.get(Opc), ResultReg);
  AddLoadStoreOperands(VT, Addr, MIB, MachineMemOperand::MOLoad, useAM3);

  // If we had an unaligned load of a float we've converted it to an regular
  // load.  Now we must move from the GRP to the FP register.
  if (needVMOV) {
    unsigned MoveReg = createResultReg(TLI.getRegClassFor(MVT::f32));
    AddOptionalDefs(BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL,
                            TII.get(ARM::VMOVSR), MoveReg)
                    .addReg(ResultReg));
    ResultReg = MoveReg;
  }
  return true;
}

bool ARMFastISel::SelectLoad(const Instruction *I) {
  // Atomic loads need special handling.
  if (cast<LoadInst>(I)->isAtomic())
    return false;

  // Verify we have a legal type before going any further.
  MVT VT;
  if (!isLoadTypeLegal(I->getType(), VT))
    return false;

  // See if we can handle this address.
  Address Addr;
  if (!ARMComputeAddress(I->getOperand(0), Addr)) return false;

  unsigned ResultReg;
  if (!ARMEmitLoad(VT, ResultReg, Addr, cast<LoadInst>(I)->getAlignment()))
    return false;
  UpdateValueMap(I, ResultReg);
  return true;
}

bool ARMFastISel::ARMEmitStore(MVT VT, unsigned SrcReg, Address &Addr,
                               unsigned Alignment) {
  unsigned StrOpc;
  bool useAM3 = false;
  switch (VT.SimpleTy) {
    // This is mostly going to be Neon/vector support.
    default: return false;
    case MVT::i1: {
      unsigned Res = createResultReg(isThumb2 ?
        (const TargetRegisterClass*)&ARM::tGPRRegClass :
        (const TargetRegisterClass*)&ARM::GPRRegClass);
      unsigned Opc = isThumb2 ? ARM::t2ANDri : ARM::ANDri;
      AddOptionalDefs(BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL,
                              TII.get(Opc), Res)
                      .addReg(SrcReg).addImm(1));
      SrcReg = Res;
    } // Fallthrough here.
    case MVT::i8:
      if (isThumb2) {
        if (Addr.Offset < 0 && Addr.Offset > -256 && Subtarget->hasV6T2Ops())
          StrOpc = ARM::t2STRBi8;
        else
          StrOpc = ARM::t2STRBi12;
      } else {
        StrOpc = ARM::STRBi12;
      }
      break;
    case MVT::i16:
      if (Alignment && Alignment < 2 && !Subtarget->allowsUnalignedMem())
        return false;

      if (isThumb2) {
        if (Addr.Offset < 0 && Addr.Offset > -256 && Subtarget->hasV6T2Ops())
          StrOpc = ARM::t2STRHi8;
        else
          StrOpc = ARM::t2STRHi12;
      } else {
        StrOpc = ARM::STRH;
        useAM3 = true;
      }
      break;
    case MVT::i32:
      if (Alignment && Alignment < 4 && !Subtarget->allowsUnalignedMem())
        return false;

      if (isThumb2) {
        if (Addr.Offset < 0 && Addr.Offset > -256 && Subtarget->hasV6T2Ops())
          StrOpc = ARM::t2STRi8;
        else
          StrOpc = ARM::t2STRi12;
      } else {
        StrOpc = ARM::STRi12;
      }
      break;
    case MVT::f32:
      if (!Subtarget->hasVFP2()) return false;
      // Unaligned stores need special handling. Floats require word-alignment.
      if (Alignment && Alignment < 4) {
        unsigned MoveReg = createResultReg(TLI.getRegClassFor(MVT::i32));
        AddOptionalDefs(BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL,
                                TII.get(ARM::VMOVRS), MoveReg)
                        .addReg(SrcReg));
        SrcReg = MoveReg;
        VT = MVT::i32;
        StrOpc = isThumb2 ? ARM::t2STRi12 : ARM::STRi12;
      } else {
        StrOpc = ARM::VSTRS;
      }
      break;
    case MVT::f64:
      if (!Subtarget->hasVFP2()) return false;
      // FIXME: Unaligned stores need special handling.  Doublewords require
      // word-alignment.
      if (Alignment && Alignment < 4)
          return false;

      StrOpc = ARM::VSTRD;
      break;
  }
  // Simplify this down to something we can handle.
  ARMSimplifyAddress(Addr, VT, useAM3);

  // Create the base instruction, then add the operands.
  MachineInstrBuilder MIB = BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL,
                                    TII.get(StrOpc))
                            .addReg(SrcReg);
  AddLoadStoreOperands(VT, Addr, MIB, MachineMemOperand::MOStore, useAM3);
  return true;
}

bool ARMFastISel::SelectStore(const Instruction *I) {
  Value *Op0 = I->getOperand(0);
  unsigned SrcReg = 0;

  // Atomic stores need special handling.
  if (cast<StoreInst>(I)->isAtomic())
    return false;

  // Verify we have a legal type before going any further.
  MVT VT;
  if (!isLoadTypeLegal(I->getOperand(0)->getType(), VT))
    return false;

  // Get the value to be stored into a register.
  SrcReg = getRegForValue(Op0);
  if (SrcReg == 0) return false;

  // See if we can handle this address.
  Address Addr;
  if (!ARMComputeAddress(I->getOperand(1), Addr))
    return false;

  if (!ARMEmitStore(VT, SrcReg, Addr, cast<StoreInst>(I)->getAlignment()))
    return false;
  return true;
}

static ARMCC::CondCodes getComparePred(CmpInst::Predicate Pred) {
  switch (Pred) {
    // Needs two compares...
    case CmpInst::FCMP_ONE:
    case CmpInst::FCMP_UEQ:
    default:
      // AL is our "false" for now. The other two need more compares.
      return ARMCC::AL;
    case CmpInst::ICMP_EQ:
    case CmpInst::FCMP_OEQ:
      return ARMCC::EQ;
    case CmpInst::ICMP_SGT:
    case CmpInst::FCMP_OGT:
      return ARMCC::GT;
    case CmpInst::ICMP_SGE:
    case CmpInst::FCMP_OGE:
      return ARMCC::GE;
    case CmpInst::ICMP_UGT:
    case CmpInst::FCMP_UGT:
      return ARMCC::HI;
    case CmpInst::FCMP_OLT:
      return ARMCC::MI;
    case CmpInst::ICMP_ULE:
    case CmpInst::FCMP_OLE:
      return ARMCC::LS;
    case CmpInst::FCMP_ORD:
      return ARMCC::VC;
    case CmpInst::FCMP_UNO:
      return ARMCC::VS;
    case CmpInst::FCMP_UGE:
      return ARMCC::PL;
    case CmpInst::ICMP_SLT:
    case CmpInst::FCMP_ULT:
      return ARMCC::LT;
    case CmpInst::ICMP_SLE:
    case CmpInst::FCMP_ULE:
      return ARMCC::LE;
    case CmpInst::FCMP_UNE:
    case CmpInst::ICMP_NE:
      return ARMCC::NE;
    case CmpInst::ICMP_UGE:
      return ARMCC::HS;
    case CmpInst::ICMP_ULT:
      return ARMCC::LO;
  }
}

bool ARMFastISel::SelectBranch(const Instruction *I) {
  const BranchInst *BI = cast<BranchInst>(I);
  MachineBasicBlock *TBB = FuncInfo.MBBMap[BI->getSuccessor(0)];
  MachineBasicBlock *FBB = FuncInfo.MBBMap[BI->getSuccessor(1)];

  // Simple branch support.

  // If we can, avoid recomputing the compare - redoing it could lead to wonky
  // behavior.
  if (const CmpInst *CI = dyn_cast<CmpInst>(BI->getCondition())) {
    if (CI->hasOneUse() && (CI->getParent() == I->getParent())) {

      // Get the compare predicate.
      // Try to take advantage of fallthrough opportunities.
      CmpInst::Predicate Predicate = CI->getPredicate();
      if (FuncInfo.MBB->isLayoutSuccessor(TBB)) {
        std::swap(TBB, FBB);
        Predicate = CmpInst::getInversePredicate(Predicate);
      }

      ARMCC::CondCodes ARMPred = getComparePred(Predicate);

      // We may not handle every CC for now.
      if (ARMPred == ARMCC::AL) return false;

      // Emit the compare.
      if (!ARMEmitCmp(CI->getOperand(0), CI->getOperand(1), CI->isUnsigned()))
        return false;

      unsigned BrOpc = isThumb2 ? ARM::t2Bcc : ARM::Bcc;
      BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, TII.get(BrOpc))
      .addMBB(TBB).addImm(ARMPred).addReg(ARM::CPSR);
      FastEmitBranch(FBB, DL);
      FuncInfo.MBB->addSuccessor(TBB);
      return true;
    }
  } else if (TruncInst *TI = dyn_cast<TruncInst>(BI->getCondition())) {
    MVT SourceVT;
    if (TI->hasOneUse() && TI->getParent() == I->getParent() &&
        (isLoadTypeLegal(TI->getOperand(0)->getType(), SourceVT))) {
      unsigned TstOpc = isThumb2 ? ARM::t2TSTri : ARM::TSTri;
      unsigned OpReg = getRegForValue(TI->getOperand(0));
      AddOptionalDefs(BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL,
                              TII.get(TstOpc))
                      .addReg(OpReg).addImm(1));

      unsigned CCMode = ARMCC::NE;
      if (FuncInfo.MBB->isLayoutSuccessor(TBB)) {
        std::swap(TBB, FBB);
        CCMode = ARMCC::EQ;
      }

      unsigned BrOpc = isThumb2 ? ARM::t2Bcc : ARM::Bcc;
      BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, TII.get(BrOpc))
      .addMBB(TBB).addImm(CCMode).addReg(ARM::CPSR);

      FastEmitBranch(FBB, DL);
      FuncInfo.MBB->addSuccessor(TBB);
      return true;
    }
  } else if (const ConstantInt *CI =
             dyn_cast<ConstantInt>(BI->getCondition())) {
    uint64_t Imm = CI->getZExtValue();
    MachineBasicBlock *Target = (Imm == 0) ? FBB : TBB;
    FastEmitBranch(Target, DL);
    return true;
  }

  unsigned CmpReg = getRegForValue(BI->getCondition());
  if (CmpReg == 0) return false;

  // We've been divorced from our compare!  Our block was split, and
  // now our compare lives in a predecessor block.  We musn't
  // re-compare here, as the children of the compare aren't guaranteed
  // live across the block boundary (we *could* check for this).
  // Regardless, the compare has been done in the predecessor block,
  // and it left a value for us in a virtual register.  Ergo, we test
  // the one-bit value left in the virtual register.
  unsigned TstOpc = isThumb2 ? ARM::t2TSTri : ARM::TSTri;
  AddOptionalDefs(BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, TII.get(TstOpc))
                  .addReg(CmpReg).addImm(1));

  unsigned CCMode = ARMCC::NE;
  if (FuncInfo.MBB->isLayoutSuccessor(TBB)) {
    std::swap(TBB, FBB);
    CCMode = ARMCC::EQ;
  }

  unsigned BrOpc = isThumb2 ? ARM::t2Bcc : ARM::Bcc;
  BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, TII.get(BrOpc))
                  .addMBB(TBB).addImm(CCMode).addReg(ARM::CPSR);
  FastEmitBranch(FBB, DL);
  FuncInfo.MBB->addSuccessor(TBB);
  return true;
}

bool ARMFastISel::SelectIndirectBr(const Instruction *I) {
  unsigned AddrReg = getRegForValue(I->getOperand(0));
  if (AddrReg == 0) return false;

  unsigned Opc = isThumb2 ? ARM::tBRIND : ARM::BX;
  AddOptionalDefs(BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, TII.get(Opc))
                  .addReg(AddrReg));

  const IndirectBrInst *IB = cast<IndirectBrInst>(I);
  for (unsigned i = 0, e = IB->getNumSuccessors(); i != e; ++i)
    FuncInfo.MBB->addSuccessor(FuncInfo.MBBMap[IB->getSuccessor(i)]);

  return true;
}

bool ARMFastISel::ARMEmitCmp(const Value *Src1Value, const Value *Src2Value,
                             bool isZExt) {
  Type *Ty = Src1Value->getType();
  EVT SrcEVT = TLI.getValueType(Ty, true);
  if (!SrcEVT.isSimple()) return false;
  MVT SrcVT = SrcEVT.getSimpleVT();

  bool isFloat = (Ty->isFloatTy() || Ty->isDoubleTy());
  if (isFloat && !Subtarget->hasVFP2())
    return false;

  // Check to see if the 2nd operand is a constant that we can encode directly
  // in the compare.
  int Imm = 0;
  bool UseImm = false;
  bool isNegativeImm = false;
  // FIXME: At -O0 we don't have anything that canonicalizes operand order.
  // Thus, Src1Value may be a ConstantInt, but we're missing it.
  if (const ConstantInt *ConstInt = dyn_cast<ConstantInt>(Src2Value)) {
    if (SrcVT == MVT::i32 || SrcVT == MVT::i16 || SrcVT == MVT::i8 ||
        SrcVT == MVT::i1) {
      const APInt &CIVal = ConstInt->getValue();
      Imm = (isZExt) ? (int)CIVal.getZExtValue() : (int)CIVal.getSExtValue();
      // For INT_MIN/LONG_MIN (i.e., 0x80000000) we need to use a cmp, rather
      // then a cmn, because there is no way to represent 2147483648 as a 
      // signed 32-bit int.
      if (Imm < 0 && Imm != (int)0x80000000) {
        isNegativeImm = true;
        Imm = -Imm;
      }
      UseImm = isThumb2 ? (ARM_AM::getT2SOImmVal(Imm) != -1) :
        (ARM_AM::getSOImmVal(Imm) != -1);
    }
  } else if (const ConstantFP *ConstFP = dyn_cast<ConstantFP>(Src2Value)) {
    if (SrcVT == MVT::f32 || SrcVT == MVT::f64)
      if (ConstFP->isZero() && !ConstFP->isNegative())
        UseImm = true;
  }

  unsigned CmpOpc;
  bool isICmp = true;
  bool needsExt = false;
  switch (SrcVT.SimpleTy) {
    default: return false;
    // TODO: Verify compares.
    case MVT::f32:
      isICmp = false;
      CmpOpc = UseImm ? ARM::VCMPEZS : ARM::VCMPES;
      break;
    case MVT::f64:
      isICmp = false;
      CmpOpc = UseImm ? ARM::VCMPEZD : ARM::VCMPED;
      break;
    case MVT::i1:
    case MVT::i8:
    case MVT::i16:
      needsExt = true;
    // Intentional fall-through.
    case MVT::i32:
      if (isThumb2) {
        if (!UseImm)
          CmpOpc = ARM::t2CMPrr;
        else
          CmpOpc = isNegativeImm ? ARM::t2CMNri : ARM::t2CMPri;
      } else {
        if (!UseImm)
          CmpOpc = ARM::CMPrr;
        else
          CmpOpc = isNegativeImm ? ARM::CMNri : ARM::CMPri;
      }
      break;
  }

  unsigned SrcReg1 = getRegForValue(Src1Value);
  if (SrcReg1 == 0) return false;

  unsigned SrcReg2 = 0;
  if (!UseImm) {
    SrcReg2 = getRegForValue(Src2Value);
    if (SrcReg2 == 0) return false;
  }

  // We have i1, i8, or i16, we need to either zero extend or sign extend.
  if (needsExt) {
    SrcReg1 = ARMEmitIntExt(SrcVT, SrcReg1, MVT::i32, isZExt);
    if (SrcReg1 == 0) return false;
    if (!UseImm) {
      SrcReg2 = ARMEmitIntExt(SrcVT, SrcReg2, MVT::i32, isZExt);
      if (SrcReg2 == 0) return false;
    }
  }

  if (!UseImm) {
    AddOptionalDefs(BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL,
                            TII.get(CmpOpc))
                    .addReg(SrcReg1).addReg(SrcReg2));
  } else {
    MachineInstrBuilder MIB;
    MIB = BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, TII.get(CmpOpc))
      .addReg(SrcReg1);

    // Only add immediate for icmp as the immediate for fcmp is an implicit 0.0.
    if (isICmp)
      MIB.addImm(Imm);
    AddOptionalDefs(MIB);
  }

  // For floating point we need to move the result to a comparison register
  // that we can then use for branches.
  if (Ty->isFloatTy() || Ty->isDoubleTy())
    AddOptionalDefs(BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL,
                            TII.get(ARM::FMSTAT)));
  return true;
}

bool ARMFastISel::SelectCmp(const Instruction *I) {
  const CmpInst *CI = cast<CmpInst>(I);

  // Get the compare predicate.
  ARMCC::CondCodes ARMPred = getComparePred(CI->getPredicate());

  // We may not handle every CC for now.
  if (ARMPred == ARMCC::AL) return false;

  // Emit the compare.
  if (!ARMEmitCmp(CI->getOperand(0), CI->getOperand(1), CI->isUnsigned()))
    return false;

  // Now set a register based on the comparison. Explicitly set the predicates
  // here.
  unsigned MovCCOpc = isThumb2 ? ARM::t2MOVCCi : ARM::MOVCCi;
  const TargetRegisterClass *RC = isThumb2 ?
    (const TargetRegisterClass*)&ARM::rGPRRegClass :
    (const TargetRegisterClass*)&ARM::GPRRegClass;
  unsigned DestReg = createResultReg(RC);
  Constant *Zero = ConstantInt::get(Type::getInt32Ty(*Context), 0);
  unsigned ZeroReg = TargetMaterializeConstant(Zero);
  // ARMEmitCmp emits a FMSTAT when necessary, so it's always safe to use CPSR.
  BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, TII.get(MovCCOpc), DestReg)
          .addReg(ZeroReg).addImm(1)
          .addImm(ARMPred).addReg(ARM::CPSR);

  UpdateValueMap(I, DestReg);
  return true;
}

bool ARMFastISel::SelectFPExt(const Instruction *I) {
  // Make sure we have VFP and that we're extending float to double.
  if (!Subtarget->hasVFP2()) return false;

  Value *V = I->getOperand(0);
  if (!I->getType()->isDoubleTy() ||
      !V->getType()->isFloatTy()) return false;

  unsigned Op = getRegForValue(V);
  if (Op == 0) return false;

  unsigned Result = createResultReg(&ARM::DPRRegClass);
  AddOptionalDefs(BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL,
                          TII.get(ARM::VCVTDS), Result)
                  .addReg(Op));
  UpdateValueMap(I, Result);
  return true;
}

bool ARMFastISel::SelectFPTrunc(const Instruction *I) {
  // Make sure we have VFP and that we're truncating double to float.
  if (!Subtarget->hasVFP2()) return false;

  Value *V = I->getOperand(0);
  if (!(I->getType()->isFloatTy() &&
        V->getType()->isDoubleTy())) return false;

  unsigned Op = getRegForValue(V);
  if (Op == 0) return false;

  unsigned Result = createResultReg(&ARM::SPRRegClass);
  AddOptionalDefs(BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL,
                          TII.get(ARM::VCVTSD), Result)
                  .addReg(Op));
  UpdateValueMap(I, Result);
  return true;
}

bool ARMFastISel::SelectIToFP(const Instruction *I, bool isSigned) {
  // Make sure we have VFP.
  if (!Subtarget->hasVFP2()) return false;

  MVT DstVT;
  Type *Ty = I->getType();
  if (!isTypeLegal(Ty, DstVT))
    return false;

  Value *Src = I->getOperand(0);
  EVT SrcEVT = TLI.getValueType(Src->getType(), true);
  if (!SrcEVT.isSimple())
    return false;
  MVT SrcVT = SrcEVT.getSimpleVT();
  if (SrcVT != MVT::i32 && SrcVT != MVT::i16 && SrcVT != MVT::i8)
    return false;

  unsigned SrcReg = getRegForValue(Src);
  if (SrcReg == 0) return false;

  // Handle sign-extension.
  if (SrcVT == MVT::i16 || SrcVT == MVT::i8) {
    SrcReg = ARMEmitIntExt(SrcVT, SrcReg, MVT::i32,
                                       /*isZExt*/!isSigned);
    if (SrcReg == 0) return false;
  }

  // The conversion routine works on fp-reg to fp-reg and the operand above
  // was an integer, move it to the fp registers if possible.
  unsigned FP = ARMMoveToFPReg(MVT::f32, SrcReg);
  if (FP == 0) return false;

  unsigned Opc;
  if (Ty->isFloatTy()) Opc = isSigned ? ARM::VSITOS : ARM::VUITOS;
  else if (Ty->isDoubleTy()) Opc = isSigned ? ARM::VSITOD : ARM::VUITOD;
  else return false;

  unsigned ResultReg = createResultReg(TLI.getRegClassFor(DstVT));
  AddOptionalDefs(BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, TII.get(Opc),
                          ResultReg)
                  .addReg(FP));
  UpdateValueMap(I, ResultReg);
  return true;
}

bool ARMFastISel::SelectFPToI(const Instruction *I, bool isSigned) {
  // Make sure we have VFP.
  if (!Subtarget->hasVFP2()) return false;

  MVT DstVT;
  Type *RetTy = I->getType();
  if (!isTypeLegal(RetTy, DstVT))
    return false;

  unsigned Op = getRegForValue(I->getOperand(0));
  if (Op == 0) return false;

  unsigned Opc;
  Type *OpTy = I->getOperand(0)->getType();
  if (OpTy->isFloatTy()) Opc = isSigned ? ARM::VTOSIZS : ARM::VTOUIZS;
  else if (OpTy->isDoubleTy()) Opc = isSigned ? ARM::VTOSIZD : ARM::VTOUIZD;
  else return false;

  // f64->s32/u32 or f32->s32/u32 both need an intermediate f32 reg.
  unsigned ResultReg = createResultReg(TLI.getRegClassFor(MVT::f32));
  AddOptionalDefs(BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, TII.get(Opc),
                          ResultReg)
                  .addReg(Op));

  // This result needs to be in an integer register, but the conversion only
  // takes place in fp-regs.
  unsigned IntReg = ARMMoveToIntReg(DstVT, ResultReg);
  if (IntReg == 0) return false;

  UpdateValueMap(I, IntReg);
  return true;
}

bool ARMFastISel::SelectSelect(const Instruction *I) {
  MVT VT;
  if (!isTypeLegal(I->getType(), VT))
    return false;

  // Things need to be register sized for register moves.
  if (VT != MVT::i32) return false;

  unsigned CondReg = getRegForValue(I->getOperand(0));
  if (CondReg == 0) return false;
  unsigned Op1Reg = getRegForValue(I->getOperand(1));
  if (Op1Reg == 0) return false;

  // Check to see if we can use an immediate in the conditional move.
  int Imm = 0;
  bool UseImm = false;
  bool isNegativeImm = false;
  if (const ConstantInt *ConstInt = dyn_cast<ConstantInt>(I->getOperand(2))) {
    assert (VT == MVT::i32 && "Expecting an i32.");
    Imm = (int)ConstInt->getValue().getZExtValue();
    if (Imm < 0) {
      isNegativeImm = true;
      Imm = ~Imm;
    }
    UseImm = isThumb2 ? (ARM_AM::getT2SOImmVal(Imm) != -1) :
      (ARM_AM::getSOImmVal(Imm) != -1);
  }

  unsigned Op2Reg = 0;
  if (!UseImm) {
    Op2Reg = getRegForValue(I->getOperand(2));
    if (Op2Reg == 0) return false;
  }

  unsigned CmpOpc = isThumb2 ? ARM::t2CMPri : ARM::CMPri;
  AddOptionalDefs(BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, TII.get(CmpOpc))
                  .addReg(CondReg).addImm(0));

  unsigned MovCCOpc;
  const TargetRegisterClass *RC;
  if (!UseImm) {
    RC = isThumb2 ? &ARM::tGPRRegClass : &ARM::GPRRegClass;
    MovCCOpc = isThumb2 ? ARM::t2MOVCCr : ARM::MOVCCr;
  } else {
    RC = isThumb2 ? &ARM::rGPRRegClass : &ARM::GPRRegClass;
    if (!isNegativeImm)
      MovCCOpc = isThumb2 ? ARM::t2MOVCCi : ARM::MOVCCi;
    else
      MovCCOpc = isThumb2 ? ARM::t2MVNCCi : ARM::MVNCCi;
  }
  unsigned ResultReg = createResultReg(RC);
  if (!UseImm)
    BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, TII.get(MovCCOpc), ResultReg)
    .addReg(Op2Reg).addReg(Op1Reg).addImm(ARMCC::NE).addReg(ARM::CPSR);
  else
    BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, TII.get(MovCCOpc), ResultReg)
    .addReg(Op1Reg).addImm(Imm).addImm(ARMCC::EQ).addReg(ARM::CPSR);
  UpdateValueMap(I, ResultReg);
  return true;
}

bool ARMFastISel::SelectDiv(const Instruction *I, bool isSigned) {
  MVT VT;
  Type *Ty = I->getType();
  if (!isTypeLegal(Ty, VT))
    return false;

  // If we have integer div support we should have selected this automagically.
  // In case we have a real miss go ahead and return false and we'll pick
  // it up later.
  if (Subtarget->hasDivide()) return false;

  // Otherwise emit a libcall.
  RTLIB::Libcall LC = RTLIB::UNKNOWN_LIBCALL;
  if (VT == MVT::i8)
    LC = isSigned ? RTLIB::SDIV_I8 : RTLIB::UDIV_I8;
  else if (VT == MVT::i16)
    LC = isSigned ? RTLIB::SDIV_I16 : RTLIB::UDIV_I16;
  else if (VT == MVT::i32)
    LC = isSigned ? RTLIB::SDIV_I32 : RTLIB::UDIV_I32;
  else if (VT == MVT::i64)
    LC = isSigned ? RTLIB::SDIV_I64 : RTLIB::UDIV_I64;
  else if (VT == MVT::i128)
    LC = isSigned ? RTLIB::SDIV_I128 : RTLIB::UDIV_I128;
  assert(LC != RTLIB::UNKNOWN_LIBCALL && "Unsupported SDIV!");

  return ARMEmitLibcall(I, LC);
}

bool ARMFastISel::SelectRem(const Instruction *I, bool isSigned) {
  MVT VT;
  Type *Ty = I->getType();
  if (!isTypeLegal(Ty, VT))
    return false;

  RTLIB::Libcall LC = RTLIB::UNKNOWN_LIBCALL;
  if (VT == MVT::i8)
    LC = isSigned ? RTLIB::SREM_I8 : RTLIB::UREM_I8;
  else if (VT == MVT::i16)
    LC = isSigned ? RTLIB::SREM_I16 : RTLIB::UREM_I16;
  else if (VT == MVT::i32)
    LC = isSigned ? RTLIB::SREM_I32 : RTLIB::UREM_I32;
  else if (VT == MVT::i64)
    LC = isSigned ? RTLIB::SREM_I64 : RTLIB::UREM_I64;
  else if (VT == MVT::i128)
    LC = isSigned ? RTLIB::SREM_I128 : RTLIB::UREM_I128;
  assert(LC != RTLIB::UNKNOWN_LIBCALL && "Unsupported SREM!");

  return ARMEmitLibcall(I, LC);
}

bool ARMFastISel::SelectBinaryIntOp(const Instruction *I, unsigned ISDOpcode) {
  EVT DestVT  = TLI.getValueType(I->getType(), true);

  // We can get here in the case when we have a binary operation on a non-legal
  // type and the target independent selector doesn't know how to handle it.
  if (DestVT != MVT::i16 && DestVT != MVT::i8 && DestVT != MVT::i1)
    return false;

  unsigned Opc;
  switch (ISDOpcode) {
    default: return false;
    case ISD::ADD:
      Opc = isThumb2 ? ARM::t2ADDrr : ARM::ADDrr;
      break;
    case ISD::OR:
      Opc = isThumb2 ? ARM::t2ORRrr : ARM::ORRrr;
      break;
    case ISD::SUB:
      Opc = isThumb2 ? ARM::t2SUBrr : ARM::SUBrr;
      break;
  }

  unsigned SrcReg1 = getRegForValue(I->getOperand(0));
  if (SrcReg1 == 0) return false;

  // TODO: Often the 2nd operand is an immediate, which can be encoded directly
  // in the instruction, rather then materializing the value in a register.
  unsigned SrcReg2 = getRegForValue(I->getOperand(1));
  if (SrcReg2 == 0) return false;

  unsigned ResultReg = createResultReg(TLI.getRegClassFor(MVT::i32));
  AddOptionalDefs(BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL,
                          TII.get(Opc), ResultReg)
                  .addReg(SrcReg1).addReg(SrcReg2));
  UpdateValueMap(I, ResultReg);
  return true;
}

bool ARMFastISel::SelectBinaryFPOp(const Instruction *I, unsigned ISDOpcode) {
  EVT FPVT = TLI.getValueType(I->getType(), true);
  if (!FPVT.isSimple()) return false;
  MVT VT = FPVT.getSimpleVT();

  // We can get here in the case when we want to use NEON for our fp
  // operations, but can't figure out how to. Just use the vfp instructions
  // if we have them.
  // FIXME: It'd be nice to use NEON instructions.
  Type *Ty = I->getType();
  bool isFloat = (Ty->isDoubleTy() || Ty->isFloatTy());
  if (isFloat && !Subtarget->hasVFP2())
    return false;

  unsigned Opc;
  bool is64bit = VT == MVT::f64 || VT == MVT::i64;
  switch (ISDOpcode) {
    default: return false;
    case ISD::FADD:
      Opc = is64bit ? ARM::VADDD : ARM::VADDS;
      break;
    case ISD::FSUB:
      Opc = is64bit ? ARM::VSUBD : ARM::VSUBS;
      break;
    case ISD::FMUL:
      Opc = is64bit ? ARM::VMULD : ARM::VMULS;
      break;
  }
  unsigned Op1 = getRegForValue(I->getOperand(0));
  if (Op1 == 0) return false;

  unsigned Op2 = getRegForValue(I->getOperand(1));
  if (Op2 == 0) return false;

  unsigned ResultReg = createResultReg(TLI.getRegClassFor(VT.SimpleTy));
  AddOptionalDefs(BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL,
                          TII.get(Opc), ResultReg)
                  .addReg(Op1).addReg(Op2));
  UpdateValueMap(I, ResultReg);
  return true;
}

// Call Handling Code

// This is largely taken directly from CCAssignFnForNode
// TODO: We may not support all of this.
CCAssignFn *ARMFastISel::CCAssignFnForCall(CallingConv::ID CC,
                                           bool Return,
                                           bool isVarArg) {
  switch (CC) {
  default:
    llvm_unreachable("Unsupported calling convention");
  case CallingConv::Fast:
    if (Subtarget->hasVFP2() && !isVarArg) {
      if (!Subtarget->isAAPCS_ABI())
        return (Return ? RetFastCC_ARM_APCS : FastCC_ARM_APCS);
      // For AAPCS ABI targets, just use VFP variant of the calling convention.
      return (Return ? RetCC_ARM_AAPCS_VFP : CC_ARM_AAPCS_VFP);
    }
    // Fallthrough
  case CallingConv::C:
    // Use target triple & subtarget features to do actual dispatch.
    if (Subtarget->isAAPCS_ABI()) {
      if (Subtarget->hasVFP2() &&
          TM.Options.FloatABIType == FloatABI::Hard && !isVarArg)
        return (Return ? RetCC_ARM_AAPCS_VFP: CC_ARM_AAPCS_VFP);
      else
        return (Return ? RetCC_ARM_AAPCS: CC_ARM_AAPCS);
    } else
        return (Return ? RetCC_ARM_APCS: CC_ARM_APCS);
  case CallingConv::ARM_AAPCS_VFP:
    if (!isVarArg)
      return (Return ? RetCC_ARM_AAPCS_VFP: CC_ARM_AAPCS_VFP);
    // Fall through to soft float variant, variadic functions don't
    // use hard floating point ABI.
  case CallingConv::ARM_AAPCS:
    return (Return ? RetCC_ARM_AAPCS: CC_ARM_AAPCS);
  case CallingConv::ARM_APCS:
    return (Return ? RetCC_ARM_APCS: CC_ARM_APCS);
  case CallingConv::GHC:
    if (Return)
      llvm_unreachable("Can't return in GHC call convention");
    else
      return CC_ARM_APCS_GHC;
  }
}

bool ARMFastISel::ProcessCallArgs(SmallVectorImpl<Value*> &Args,
                                  SmallVectorImpl<unsigned> &ArgRegs,
                                  SmallVectorImpl<MVT> &ArgVTs,
                                  SmallVectorImpl<ISD::ArgFlagsTy> &ArgFlags,
                                  SmallVectorImpl<unsigned> &RegArgs,
                                  CallingConv::ID CC,
                                  unsigned &NumBytes,
                                  bool isVarArg) {
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CC, isVarArg, *FuncInfo.MF, TM, ArgLocs, *Context);
  CCInfo.AnalyzeCallOperands(ArgVTs, ArgFlags,
                             CCAssignFnForCall(CC, false, isVarArg));

  // Check that we can handle all of the arguments. If we can't, then bail out
  // now before we add code to the MBB.
  for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) {
    CCValAssign &VA = ArgLocs[i];
    MVT ArgVT = ArgVTs[VA.getValNo()];

    // We don't handle NEON/vector parameters yet.
    if (ArgVT.isVector() || ArgVT.getSizeInBits() > 64)
      return false;

    // Now copy/store arg to correct locations.
    if (VA.isRegLoc() && !VA.needsCustom()) {
      continue;
    } else if (VA.needsCustom()) {
      // TODO: We need custom lowering for vector (v2f64) args.
      if (VA.getLocVT() != MVT::f64 ||
          // TODO: Only handle register args for now.
          !VA.isRegLoc() || !ArgLocs[++i].isRegLoc())
        return false;
    } else {
      switch (static_cast<EVT>(ArgVT).getSimpleVT().SimpleTy) {
      default:
        return false;
      case MVT::i1:
      case MVT::i8:
      case MVT::i16:
      case MVT::i32:
        break;
      case MVT::f32:
        if (!Subtarget->hasVFP2())
          return false;
        break;
      case MVT::f64:
        if (!Subtarget->hasVFP2())
          return false;
        break;
      }
    }
  }

  // At the point, we are able to handle the call's arguments in fast isel.

  // Get a count of how many bytes are to be pushed on the stack.
  NumBytes = CCInfo.getNextStackOffset();

  // Issue CALLSEQ_START
  unsigned AdjStackDown = TII.getCallFrameSetupOpcode();
  AddOptionalDefs(BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL,
                          TII.get(AdjStackDown))
                  .addImm(NumBytes));

  // Process the args.
  for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) {
    CCValAssign &VA = ArgLocs[i];
    unsigned Arg = ArgRegs[VA.getValNo()];
    MVT ArgVT = ArgVTs[VA.getValNo()];

    assert((!ArgVT.isVector() && ArgVT.getSizeInBits() <= 64) &&
           "We don't handle NEON/vector parameters yet.");

    // Handle arg promotion, etc.
    switch (VA.getLocInfo()) {
      case CCValAssign::Full: break;
      case CCValAssign::SExt: {
        MVT DestVT = VA.getLocVT();
        Arg = ARMEmitIntExt(ArgVT, Arg, DestVT, /*isZExt*/false);
        assert (Arg != 0 && "Failed to emit a sext");
        ArgVT = DestVT;
        break;
      }
      case CCValAssign::AExt:
        // Intentional fall-through.  Handle AExt and ZExt.
      case CCValAssign::ZExt: {
        MVT DestVT = VA.getLocVT();
        Arg = ARMEmitIntExt(ArgVT, Arg, DestVT, /*isZExt*/true);
        assert (Arg != 0 && "Failed to emit a sext");
        ArgVT = DestVT;
        break;
      }
      case CCValAssign::BCvt: {
        unsigned BC = FastEmit_r(ArgVT, VA.getLocVT(), ISD::BITCAST, Arg,
                                 /*TODO: Kill=*/false);
        assert(BC != 0 && "Failed to emit a bitcast!");
        Arg = BC;
        ArgVT = VA.getLocVT();
        break;
      }
      default: llvm_unreachable("Unknown arg promotion!");
    }

    // Now copy/store arg to correct locations.
    if (VA.isRegLoc() && !VA.needsCustom()) {
      BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, TII.get(TargetOpcode::COPY),
              VA.getLocReg())
        .addReg(Arg);
      RegArgs.push_back(VA.getLocReg());
    } else if (VA.needsCustom()) {
      // TODO: We need custom lowering for vector (v2f64) args.
      assert(VA.getLocVT() == MVT::f64 &&
             "Custom lowering for v2f64 args not available");

      CCValAssign &NextVA = ArgLocs[++i];

      assert(VA.isRegLoc() && NextVA.isRegLoc() &&
             "We only handle register args!");

      AddOptionalDefs(BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL,
                              TII.get(ARM::VMOVRRD), VA.getLocReg())
                      .addReg(NextVA.getLocReg(), RegState::Define)
                      .addReg(Arg));
      RegArgs.push_back(VA.getLocReg());
      RegArgs.push_back(NextVA.getLocReg());
    } else {
      assert(VA.isMemLoc());
      // Need to store on the stack.
      Address Addr;
      Addr.BaseType = Address::RegBase;
      Addr.Base.Reg = ARM::SP;
      Addr.Offset = VA.getLocMemOffset();

      bool EmitRet = ARMEmitStore(ArgVT, Arg, Addr); (void)EmitRet;
      assert(EmitRet && "Could not emit a store for argument!");
    }
  }

  return true;
}

bool ARMFastISel::FinishCall(MVT RetVT, SmallVectorImpl<unsigned> &UsedRegs,
                             const Instruction *I, CallingConv::ID CC,
                             unsigned &NumBytes, bool isVarArg) {
  // Issue CALLSEQ_END
  unsigned AdjStackUp = TII.getCallFrameDestroyOpcode();
  AddOptionalDefs(BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL,
                          TII.get(AdjStackUp))
                  .addImm(NumBytes).addImm(0));

  // Now the return value.
  if (RetVT != MVT::isVoid) {
    SmallVector<CCValAssign, 16> RVLocs;
    CCState CCInfo(CC, isVarArg, *FuncInfo.MF, TM, RVLocs, *Context);
    CCInfo.AnalyzeCallResult(RetVT, CCAssignFnForCall(CC, true, isVarArg));

    // Copy all of the result registers out of their specified physreg.
    if (RVLocs.size() == 2 && RetVT == MVT::f64) {
      // For this move we copy into two registers and then move into the
      // double fp reg we want.
      MVT DestVT = RVLocs[0].getValVT();
      const TargetRegisterClass* DstRC = TLI.getRegClassFor(DestVT);
      unsigned ResultReg = createResultReg(DstRC);
      AddOptionalDefs(BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL,
                              TII.get(ARM::VMOVDRR), ResultReg)
                      .addReg(RVLocs[0].getLocReg())
                      .addReg(RVLocs[1].getLocReg()));

      UsedRegs.push_back(RVLocs[0].getLocReg());
      UsedRegs.push_back(RVLocs[1].getLocReg());

      // Finally update the result.
      UpdateValueMap(I, ResultReg);
    } else {
      assert(RVLocs.size() == 1 &&"Can't handle non-double multi-reg retvals!");
      MVT CopyVT = RVLocs[0].getValVT();

      // Special handling for extended integers.
      if (RetVT == MVT::i1 || RetVT == MVT::i8 || RetVT == MVT::i16)
        CopyVT = MVT::i32;

      const TargetRegisterClass* DstRC = TLI.getRegClassFor(CopyVT);

      unsigned ResultReg = createResultReg(DstRC);
      BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, TII.get(TargetOpcode::COPY),
              ResultReg).addReg(RVLocs[0].getLocReg());
      UsedRegs.push_back(RVLocs[0].getLocReg());

      // Finally update the result.
      UpdateValueMap(I, ResultReg);
    }
  }

  return true;
}

bool ARMFastISel::SelectRet(const Instruction *I) {
  const ReturnInst *Ret = cast<ReturnInst>(I);
  const Function &F = *I->getParent()->getParent();

  if (!FuncInfo.CanLowerReturn)
    return false;

  // Build a list of return value registers.
  SmallVector<unsigned, 4> RetRegs;

  CallingConv::ID CC = F.getCallingConv();
  if (Ret->getNumOperands() > 0) {
    SmallVector<ISD::OutputArg, 4> Outs;
    GetReturnInfo(F.getReturnType(), F.getAttributes(), Outs, TLI);

    // Analyze operands of the call, assigning locations to each operand.
    SmallVector<CCValAssign, 16> ValLocs;
    CCState CCInfo(CC, F.isVarArg(), *FuncInfo.MF, TM, ValLocs,I->getContext());
    CCInfo.AnalyzeReturn(Outs, CCAssignFnForCall(CC, true /* is Ret */,
                                                 F.isVarArg()));

    const Value *RV = Ret->getOperand(0);
    unsigned Reg = getRegForValue(RV);
    if (Reg == 0)
      return false;

    // Only handle a single return value for now.
    if (ValLocs.size() != 1)
      return false;

    CCValAssign &VA = ValLocs[0];

    // Don't bother handling odd stuff for now.
    if (VA.getLocInfo() != CCValAssign::Full)
      return false;
    // Only handle register returns for now.
    if (!VA.isRegLoc())
      return false;

    unsigned SrcReg = Reg + VA.getValNo();
    EVT RVEVT = TLI.getValueType(RV->getType());
    if (!RVEVT.isSimple()) return false;
    MVT RVVT = RVEVT.getSimpleVT();
    MVT DestVT = VA.getValVT();
    // Special handling for extended integers.
    if (RVVT != DestVT) {
      if (RVVT != MVT::i1 && RVVT != MVT::i8 && RVVT != MVT::i16)
        return false;

      assert(DestVT == MVT::i32 && "ARM should always ext to i32");

      // Perform extension if flagged as either zext or sext.  Otherwise, do
      // nothing.
      if (Outs[0].Flags.isZExt() || Outs[0].Flags.isSExt()) {
        SrcReg = ARMEmitIntExt(RVVT, SrcReg, DestVT, Outs[0].Flags.isZExt());
        if (SrcReg == 0) return false;
      }
    }

    // Make the copy.
    unsigned DstReg = VA.getLocReg();
    const TargetRegisterClass* SrcRC = MRI.getRegClass(SrcReg);
    // Avoid a cross-class copy. This is very unlikely.
    if (!SrcRC->contains(DstReg))
      return false;
    BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, TII.get(TargetOpcode::COPY),
            DstReg).addReg(SrcReg);

    // Add register to return instruction.
    RetRegs.push_back(VA.getLocReg());
  }

  unsigned RetOpc = isThumb2 ? ARM::tBX_RET : ARM::BX_RET;
  MachineInstrBuilder MIB = BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL,
                                    TII.get(RetOpc));
  AddOptionalDefs(MIB);
  for (unsigned i = 0, e = RetRegs.size(); i != e; ++i)
    MIB.addReg(RetRegs[i], RegState::Implicit);
  return true;
}

unsigned ARMFastISel::ARMSelectCallOp(bool UseReg) {
  if (UseReg)
    return isThumb2 ? ARM::tBLXr : ARM::BLX;
  else
    return isThumb2 ? ARM::tBL : ARM::BL;
}

unsigned ARMFastISel::getLibcallReg(const Twine &Name) {
  GlobalValue *GV = new GlobalVariable(Type::getInt32Ty(*Context), false,
                                       GlobalValue::ExternalLinkage, 0, Name);
  EVT LCREVT = TLI.getValueType(GV->getType());
  if (!LCREVT.isSimple()) return 0;
  return ARMMaterializeGV(GV, LCREVT.getSimpleVT());
}

// A quick function that will emit a call for a named libcall in F with the
// vector of passed arguments for the Instruction in I. We can assume that we
// can emit a call for any libcall we can produce. This is an abridged version
// of the full call infrastructure since we won't need to worry about things
// like computed function pointers or strange arguments at call sites.
// TODO: Try to unify this and the normal call bits for ARM, then try to unify
// with X86.
bool ARMFastISel::ARMEmitLibcall(const Instruction *I, RTLIB::Libcall Call) {
  CallingConv::ID CC = TLI.getLibcallCallingConv(Call);

  // Handle *simple* calls for now.
  Type *RetTy = I->getType();
  MVT RetVT;
  if (RetTy->isVoidTy())
    RetVT = MVT::isVoid;
  else if (!isTypeLegal(RetTy, RetVT))
    return false;

  // Can't handle non-double multi-reg retvals.
  if (RetVT != MVT::isVoid && RetVT != MVT::i32) {
    SmallVector<CCValAssign, 16> RVLocs;
    CCState CCInfo(CC, false, *FuncInfo.MF, TM, RVLocs, *Context);
    CCInfo.AnalyzeCallResult(RetVT, CCAssignFnForCall(CC, true, false));
    if (RVLocs.size() >= 2 && RetVT != MVT::f64)
      return false;
  }

  // Set up the argument vectors.
  SmallVector<Value*, 8> Args;
  SmallVector<unsigned, 8> ArgRegs;
  SmallVector<MVT, 8> ArgVTs;
  SmallVector<ISD::ArgFlagsTy, 8> ArgFlags;
  Args.reserve(I->getNumOperands());
  ArgRegs.reserve(I->getNumOperands());
  ArgVTs.reserve(I->getNumOperands());
  ArgFlags.reserve(I->getNumOperands());
  for (unsigned i = 0; i < I->getNumOperands(); ++i) {
    Value *Op = I->getOperand(i);
    unsigned Arg = getRegForValue(Op);
    if (Arg == 0) return false;

    Type *ArgTy = Op->getType();
    MVT ArgVT;
    if (!isTypeLegal(ArgTy, ArgVT)) return false;

    ISD::ArgFlagsTy Flags;
    unsigned OriginalAlignment = TD.getABITypeAlignment(ArgTy);
    Flags.setOrigAlign(OriginalAlignment);

    Args.push_back(Op);
    ArgRegs.push_back(Arg);
    ArgVTs.push_back(ArgVT);
    ArgFlags.push_back(Flags);
  }

  // Handle the arguments now that we've gotten them.
  SmallVector<unsigned, 4> RegArgs;
  unsigned NumBytes;
  if (!ProcessCallArgs(Args, ArgRegs, ArgVTs, ArgFlags,
                       RegArgs, CC, NumBytes, false))
    return false;

  unsigned CalleeReg = 0;
  if (EnableARMLongCalls) {
    CalleeReg = getLibcallReg(TLI.getLibcallName(Call));
    if (CalleeReg == 0) return false;
  }

  // Issue the call.
  unsigned CallOpc = ARMSelectCallOp(EnableARMLongCalls);
  MachineInstrBuilder MIB = BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt,
                                    DL, TII.get(CallOpc));
  // BL / BLX don't take a predicate, but tBL / tBLX do.
  if (isThumb2)
    AddDefaultPred(MIB);
  if (EnableARMLongCalls)
    MIB.addReg(CalleeReg);
  else
    MIB.addExternalSymbol(TLI.getLibcallName(Call));

  // Add implicit physical register uses to the call.
  for (unsigned i = 0, e = RegArgs.size(); i != e; ++i)
    MIB.addReg(RegArgs[i], RegState::Implicit);

  // Add a register mask with the call-preserved registers.
  // Proper defs for return values will be added by setPhysRegsDeadExcept().
  MIB.addRegMask(TRI.getCallPreservedMask(CC));

  // Finish off the call including any return values.
  SmallVector<unsigned, 4> UsedRegs;
  if (!FinishCall(RetVT, UsedRegs, I, CC, NumBytes, false)) return false;

  // Set all unused physreg defs as dead.
  static_cast<MachineInstr *>(MIB)->setPhysRegsDeadExcept(UsedRegs, TRI);

  return true;
}

bool ARMFastISel::SelectCall(const Instruction *I,
                             const char *IntrMemName = 0) {
  const CallInst *CI = cast<CallInst>(I);
  const Value *Callee = CI->getCalledValue();

  // Can't handle inline asm.
  if (isa<InlineAsm>(Callee)) return false;

  // Allow SelectionDAG isel to handle tail calls.
  if (CI->isTailCall()) return false;

  // Check the calling convention.
  ImmutableCallSite CS(CI);
  CallingConv::ID CC = CS.getCallingConv();

  // TODO: Avoid some calling conventions?

  PointerType *PT = cast<PointerType>(CS.getCalledValue()->getType());
  FunctionType *FTy = cast<FunctionType>(PT->getElementType());
  bool isVarArg = FTy->isVarArg();

  // Handle *simple* calls for now.
  Type *RetTy = I->getType();
  MVT RetVT;
  if (RetTy->isVoidTy())
    RetVT = MVT::isVoid;
  else if (!isTypeLegal(RetTy, RetVT) && RetVT != MVT::i16 &&
           RetVT != MVT::i8  && RetVT != MVT::i1)
    return false;

  // Can't handle non-double multi-reg retvals.
  if (RetVT != MVT::isVoid && RetVT != MVT::i1 && RetVT != MVT::i8 &&
      RetVT != MVT::i16 && RetVT != MVT::i32) {
    SmallVector<CCValAssign, 16> RVLocs;
    CCState CCInfo(CC, isVarArg, *FuncInfo.MF, TM, RVLocs, *Context);
    CCInfo.AnalyzeCallResult(RetVT, CCAssignFnForCall(CC, true, isVarArg));
    if (RVLocs.size() >= 2 && RetVT != MVT::f64)
      return false;
  }

  // Set up the argument vectors.
  SmallVector<Value*, 8> Args;
  SmallVector<unsigned, 8> ArgRegs;
  SmallVector<MVT, 8> ArgVTs;
  SmallVector<ISD::ArgFlagsTy, 8> ArgFlags;
  unsigned arg_size = CS.arg_size();
  Args.reserve(arg_size);
  ArgRegs.reserve(arg_size);
  ArgVTs.reserve(arg_size);
  ArgFlags.reserve(arg_size);
  for (ImmutableCallSite::arg_iterator i = CS.arg_begin(), e = CS.arg_end();
       i != e; ++i) {
    // If we're lowering a memory intrinsic instead of a regular call, skip the
    // last two arguments, which shouldn't be passed to the underlying function.
    if (IntrMemName && e-i <= 2)
      break;

    ISD::ArgFlagsTy Flags;
    unsigned AttrInd = i - CS.arg_begin() + 1;
    if (CS.paramHasAttr(AttrInd, Attribute::SExt))
      Flags.setSExt();
    if (CS.paramHasAttr(AttrInd, Attribute::ZExt))
      Flags.setZExt();

    // FIXME: Only handle *easy* calls for now.
    if (CS.paramHasAttr(AttrInd, Attribute::InReg) ||
        CS.paramHasAttr(AttrInd, Attribute::StructRet) ||
        CS.paramHasAttr(AttrInd, Attribute::Nest) ||
        CS.paramHasAttr(AttrInd, Attribute::ByVal))
      return false;

    Type *ArgTy = (*i)->getType();
    MVT ArgVT;
    if (!isTypeLegal(ArgTy, ArgVT) && ArgVT != MVT::i16 && ArgVT != MVT::i8 &&
        ArgVT != MVT::i1)
      return false;

    unsigned Arg = getRegForValue(*i);
    if (Arg == 0)
      return false;

    unsigned OriginalAlignment = TD.getABITypeAlignment(ArgTy);
    Flags.setOrigAlign(OriginalAlignment);

    Args.push_back(*i);
    ArgRegs.push_back(Arg);
    ArgVTs.push_back(ArgVT);
    ArgFlags.push_back(Flags);
  }

  // Handle the arguments now that we've gotten them.
  SmallVector<unsigned, 4> RegArgs;
  unsigned NumBytes;
  if (!ProcessCallArgs(Args, ArgRegs, ArgVTs, ArgFlags,
                       RegArgs, CC, NumBytes, isVarArg))
    return false;

  bool UseReg = false;
  const GlobalValue *GV = dyn_cast<GlobalValue>(Callee);
  if (!GV || EnableARMLongCalls) UseReg = true;

  unsigned CalleeReg = 0;
  if (UseReg) {
    if (IntrMemName)
      CalleeReg = getLibcallReg(IntrMemName);
    else
      CalleeReg = getRegForValue(Callee);

    if (CalleeReg == 0) return false;
  }

  // Issue the call.
  unsigned CallOpc = ARMSelectCallOp(UseReg);
  MachineInstrBuilder MIB = BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt,
                                    DL, TII.get(CallOpc));

  // ARM calls don't take a predicate, but tBL / tBLX do.
  if(isThumb2)
    AddDefaultPred(MIB);
  if (UseReg)
    MIB.addReg(CalleeReg);
  else if (!IntrMemName)
    MIB.addGlobalAddress(GV, 0, 0);
  else
    MIB.addExternalSymbol(IntrMemName, 0);

  // Add implicit physical register uses to the call.
  for (unsigned i = 0, e = RegArgs.size(); i != e; ++i)
    MIB.addReg(RegArgs[i], RegState::Implicit);

  // Add a register mask with the call-preserved registers.
  // Proper defs for return values will be added by setPhysRegsDeadExcept().
  MIB.addRegMask(TRI.getCallPreservedMask(CC));

  // Finish off the call including any return values.
  SmallVector<unsigned, 4> UsedRegs;
  if (!FinishCall(RetVT, UsedRegs, I, CC, NumBytes, isVarArg))
    return false;

  // Set all unused physreg defs as dead.
  static_cast<MachineInstr *>(MIB)->setPhysRegsDeadExcept(UsedRegs, TRI);

  return true;
}

bool ARMFastISel::ARMIsMemCpySmall(uint64_t Len) {
  return Len <= 16;
}

bool ARMFastISel::ARMTryEmitSmallMemCpy(Address Dest, Address Src,
                                        uint64_t Len, unsigned Alignment) {
  // Make sure we don't bloat code by inlining very large memcpy's.
  if (!ARMIsMemCpySmall(Len))
    return false;

  while (Len) {
    MVT VT;
    if (!Alignment || Alignment >= 4) {
      if (Len >= 4)
        VT = MVT::i32;
      else if (Len >= 2)
        VT = MVT::i16;
      else {
        assert (Len == 1 && "Expected a length of 1!");
        VT = MVT::i8;
      }
    } else {
      // Bound based on alignment.
      if (Len >= 2 && Alignment == 2)
        VT = MVT::i16;
      else {
        VT = MVT::i8;
      }
    }

    bool RV;
    unsigned ResultReg;
    RV = ARMEmitLoad(VT, ResultReg, Src);
    assert (RV == true && "Should be able to handle this load.");
    RV = ARMEmitStore(VT, ResultReg, Dest);
    assert (RV == true && "Should be able to handle this store.");
    (void)RV;

    unsigned Size = VT.getSizeInBits()/8;
    Len -= Size;
    Dest.Offset += Size;
    Src.Offset += Size;
  }

  return true;
}

bool ARMFastISel::SelectIntrinsicCall(const IntrinsicInst &I) {
  // FIXME: Handle more intrinsics.
  switch (I.getIntrinsicID()) {
  default: return false;
  case Intrinsic::frameaddress: {
    MachineFrameInfo *MFI = FuncInfo.MF->getFrameInfo();
    MFI->setFrameAddressIsTaken(true);

    unsigned LdrOpc;
    const TargetRegisterClass *RC;
    if (isThumb2) {
      LdrOpc =  ARM::t2LDRi12;
      RC = (const TargetRegisterClass*)&ARM::tGPRRegClass;
    } else {
      LdrOpc =  ARM::LDRi12;
      RC = (const TargetRegisterClass*)&ARM::GPRRegClass;
    }

    const ARMBaseRegisterInfo *RegInfo =
          static_cast<const ARMBaseRegisterInfo*>(TM.getRegisterInfo());
    unsigned FramePtr = RegInfo->getFrameRegister(*(FuncInfo.MF));
    unsigned SrcReg = FramePtr;

    // Recursively load frame address
    // ldr r0 [fp]
    // ldr r0 [r0]
    // ldr r0 [r0]
    // ...
    unsigned DestReg;
    unsigned Depth = cast<ConstantInt>(I.getOperand(0))->getZExtValue();
    while (Depth--) {
      DestReg = createResultReg(RC);
      AddOptionalDefs(BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL,
                              TII.get(LdrOpc), DestReg)
                      .addReg(SrcReg).addImm(0));
      SrcReg = DestReg;
    }
    UpdateValueMap(&I, SrcReg);
    return true;
  }
  case Intrinsic::memcpy:
  case Intrinsic::memmove: {
    const MemTransferInst &MTI = cast<MemTransferInst>(I);
    // Don't handle volatile.
    if (MTI.isVolatile())
      return false;

    // Disable inlining for memmove before calls to ComputeAddress.  Otherwise,
    // we would emit dead code because we don't currently handle memmoves.
    bool isMemCpy = (I.getIntrinsicID() == Intrinsic::memcpy);
    if (isa<ConstantInt>(MTI.getLength()) && isMemCpy) {
      // Small memcpy's are common enough that we want to do them without a call
      // if possible.
      uint64_t Len = cast<ConstantInt>(MTI.getLength())->getZExtValue();
      if (ARMIsMemCpySmall(Len)) {
        Address Dest, Src;
        if (!ARMComputeAddress(MTI.getRawDest(), Dest) ||
            !ARMComputeAddress(MTI.getRawSource(), Src))
          return false;
        unsigned Alignment = MTI.getAlignment();
        if (ARMTryEmitSmallMemCpy(Dest, Src, Len, Alignment))
          return true;
      }
    }

    if (!MTI.getLength()->getType()->isIntegerTy(32))
      return false;

    if (MTI.getSourceAddressSpace() > 255 || MTI.getDestAddressSpace() > 255)
      return false;

    const char *IntrMemName = isa<MemCpyInst>(I) ? "memcpy" : "memmove";
    return SelectCall(&I, IntrMemName);
  }
  case Intrinsic::memset: {
    const MemSetInst &MSI = cast<MemSetInst>(I);
    // Don't handle volatile.
    if (MSI.isVolatile())
      return false;

    if (!MSI.getLength()->getType()->isIntegerTy(32))
      return false;

    if (MSI.getDestAddressSpace() > 255)
      return false;

    return SelectCall(&I, "memset");
  }
  case Intrinsic::trap: {
    BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, TII.get(
      Subtarget->useNaClTrap() ? ARM::TRAPNaCl : ARM::TRAP));
    return true;
  }
  }
}

bool ARMFastISel::SelectTrunc(const Instruction *I) {
  // The high bits for a type smaller than the register size are assumed to be
  // undefined.
  Value *Op = I->getOperand(0);

  EVT SrcVT, DestVT;
  SrcVT = TLI.getValueType(Op->getType(), true);
  DestVT = TLI.getValueType(I->getType(), true);

  if (SrcVT != MVT::i32 && SrcVT != MVT::i16 && SrcVT != MVT::i8)
    return false;
  if (DestVT != MVT::i16 && DestVT != MVT::i8 && DestVT != MVT::i1)
    return false;

  unsigned SrcReg = getRegForValue(Op);
  if (!SrcReg) return false;

  // Because the high bits are undefined, a truncate doesn't generate
  // any code.
  UpdateValueMap(I, SrcReg);
  return true;
}

unsigned ARMFastISel::ARMEmitIntExt(MVT SrcVT, unsigned SrcReg, MVT DestVT,
                                    bool isZExt) {
  if (DestVT != MVT::i32 && DestVT != MVT::i16 && DestVT != MVT::i8)
    return 0;

  unsigned Opc;
  bool isBoolZext = false;
  const TargetRegisterClass *RC = TLI.getRegClassFor(MVT::i32);
  switch (SrcVT.SimpleTy) {
  default: return 0;
  case MVT::i16:
    if (!Subtarget->hasV6Ops()) return 0;
    RC = isThumb2 ? &ARM::rGPRRegClass : &ARM::GPRnopcRegClass;
    if (isZExt)
      Opc = isThumb2 ? ARM::t2UXTH : ARM::UXTH;
    else
      Opc = isThumb2 ? ARM::t2SXTH : ARM::SXTH;
    break;
  case MVT::i8:
    if (!Subtarget->hasV6Ops()) return 0;
    RC = isThumb2 ? &ARM::rGPRRegClass : &ARM::GPRnopcRegClass;
    if (isZExt)
      Opc = isThumb2 ? ARM::t2UXTB : ARM::UXTB;
    else
      Opc = isThumb2 ? ARM::t2SXTB : ARM::SXTB;
    break;
  case MVT::i1:
    if (isZExt) {
      RC = isThumb2 ? &ARM::rGPRRegClass : &ARM::GPRRegClass;
      Opc = isThumb2 ? ARM::t2ANDri : ARM::ANDri;
      isBoolZext = true;
      break;
    }
    return 0;
  }

  unsigned ResultReg = createResultReg(RC);
  MachineInstrBuilder MIB;
  MIB = BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, TII.get(Opc), ResultReg)
        .addReg(SrcReg);
  if (isBoolZext)
    MIB.addImm(1);
  else
    MIB.addImm(0);
  AddOptionalDefs(MIB);
  return ResultReg;
}

bool ARMFastISel::SelectIntExt(const Instruction *I) {
  // On ARM, in general, integer casts don't involve legal types; this code
  // handles promotable integers.
  Type *DestTy = I->getType();
  Value *Src = I->getOperand(0);
  Type *SrcTy = Src->getType();

  bool isZExt = isa<ZExtInst>(I);
  unsigned SrcReg = getRegForValue(Src);
  if (!SrcReg) return false;

  EVT SrcEVT, DestEVT;
  SrcEVT = TLI.getValueType(SrcTy, true);
  DestEVT = TLI.getValueType(DestTy, true);
  if (!SrcEVT.isSimple()) return false;
  if (!DestEVT.isSimple()) return false;

  MVT SrcVT = SrcEVT.getSimpleVT();
  MVT DestVT = DestEVT.getSimpleVT();
  unsigned ResultReg = ARMEmitIntExt(SrcVT, SrcReg, DestVT, isZExt);
  if (ResultReg == 0) return false;
  UpdateValueMap(I, ResultReg);
  return true;
}

bool ARMFastISel::SelectShift(const Instruction *I,
                              ARM_AM::ShiftOpc ShiftTy) {
  // We handle thumb2 mode by target independent selector
  // or SelectionDAG ISel.
  if (isThumb2)
    return false;

  // Only handle i32 now.
  EVT DestVT = TLI.getValueType(I->getType(), true);
  if (DestVT != MVT::i32)
    return false;

  unsigned Opc = ARM::MOVsr;
  unsigned ShiftImm;
  Value *Src2Value = I->getOperand(1);
  if (const ConstantInt *CI = dyn_cast<ConstantInt>(Src2Value)) {
    ShiftImm = CI->getZExtValue();

    // Fall back to selection DAG isel if the shift amount
    // is zero or greater than the width of the value type.
    if (ShiftImm == 0 || ShiftImm >=32)
      return false;

    Opc = ARM::MOVsi;
  }

  Value *Src1Value = I->getOperand(0);
  unsigned Reg1 = getRegForValue(Src1Value);
  if (Reg1 == 0) return false;

  unsigned Reg2 = 0;
  if (Opc == ARM::MOVsr) {
    Reg2 = getRegForValue(Src2Value);
    if (Reg2 == 0) return false;
  }

  unsigned ResultReg = createResultReg(TLI.getRegClassFor(MVT::i32));
  if(ResultReg == 0) return false;

  MachineInstrBuilder MIB = BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL,
                                    TII.get(Opc), ResultReg)
                            .addReg(Reg1);

  if (Opc == ARM::MOVsi)
    MIB.addImm(ARM_AM::getSORegOpc(ShiftTy, ShiftImm));
  else if (Opc == ARM::MOVsr) {
    MIB.addReg(Reg2);
    MIB.addImm(ARM_AM::getSORegOpc(ShiftTy, 0));
  }

  AddOptionalDefs(MIB);
  UpdateValueMap(I, ResultReg);
  return true;
}

// TODO: SoftFP support.
bool ARMFastISel::TargetSelectInstruction(const Instruction *I) {

  switch (I->getOpcode()) {
    case Instruction::Load:
      return SelectLoad(I);
    case Instruction::Store:
      return SelectStore(I);
    case Instruction::Br:
      return SelectBranch(I);
    case Instruction::IndirectBr:
      return SelectIndirectBr(I);
    case Instruction::ICmp:
    case Instruction::FCmp:
      return SelectCmp(I);
    case Instruction::FPExt:
      return SelectFPExt(I);
    case Instruction::FPTrunc:
      return SelectFPTrunc(I);
    case Instruction::SIToFP:
      return SelectIToFP(I, /*isSigned*/ true);
    case Instruction::UIToFP:
      return SelectIToFP(I, /*isSigned*/ false);
    case Instruction::FPToSI:
      return SelectFPToI(I, /*isSigned*/ true);
    case Instruction::FPToUI:
      return SelectFPToI(I, /*isSigned*/ false);
    case Instruction::Add:
      return SelectBinaryIntOp(I, ISD::ADD);
    case Instruction::Or:
      return SelectBinaryIntOp(I, ISD::OR);
    case Instruction::Sub:
      return SelectBinaryIntOp(I, ISD::SUB);
    case Instruction::FAdd:
      return SelectBinaryFPOp(I, ISD::FADD);
    case Instruction::FSub:
      return SelectBinaryFPOp(I, ISD::FSUB);
    case Instruction::FMul:
      return SelectBinaryFPOp(I, ISD::FMUL);
    case Instruction::SDiv:
      return SelectDiv(I, /*isSigned*/ true);
    case Instruction::UDiv:
      return SelectDiv(I, /*isSigned*/ false);
    case Instruction::SRem:
      return SelectRem(I, /*isSigned*/ true);
    case Instruction::URem:
      return SelectRem(I, /*isSigned*/ false);
    case Instruction::Call:
      if (const IntrinsicInst *II = dyn_cast<IntrinsicInst>(I))
        return SelectIntrinsicCall(*II);
      return SelectCall(I);
    case Instruction::Select:
      return SelectSelect(I);
    case Instruction::Ret:
      return SelectRet(I);
    case Instruction::Trunc:
      return SelectTrunc(I);
    case Instruction::ZExt:
    case Instruction::SExt:
      return SelectIntExt(I);
    case Instruction::Shl:
      return SelectShift(I, ARM_AM::lsl);
    case Instruction::LShr:
      return SelectShift(I, ARM_AM::lsr);
    case Instruction::AShr:
      return SelectShift(I, ARM_AM::asr);
    default: break;
  }
  return false;
}

/// TryToFoldLoad - The specified machine instr operand is a vreg, and that
/// vreg is being provided by the specified load instruction.  If possible,
/// try to fold the load as an operand to the instruction, returning true if
/// successful.
bool ARMFastISel::TryToFoldLoad(MachineInstr *MI, unsigned OpNo,
                                const LoadInst *LI) {
  // Verify we have a legal type before going any further.
  MVT VT;
  if (!isLoadTypeLegal(LI->getType(), VT))
    return false;

  // Combine load followed by zero- or sign-extend.
  // ldrb r1, [r0]       ldrb r1, [r0]
  // uxtb r2, r1     =>
  // mov  r3, r2         mov  r3, r1
  bool isZExt = true;
  switch(MI->getOpcode()) {
    default: return false;
    case ARM::SXTH:
    case ARM::t2SXTH:
      isZExt = false;
    case ARM::UXTH:
    case ARM::t2UXTH:
      if (VT != MVT::i16)
        return false;
    break;
    case ARM::SXTB:
    case ARM::t2SXTB:
      isZExt = false;
    case ARM::UXTB:
    case ARM::t2UXTB:
      if (VT != MVT::i8)
        return false;
    break;
  }
  // See if we can handle this address.
  Address Addr;
  if (!ARMComputeAddress(LI->getOperand(0), Addr)) return false;

  unsigned ResultReg = MI->getOperand(0).getReg();
  if (!ARMEmitLoad(VT, ResultReg, Addr, LI->getAlignment(), isZExt, false))
    return false;
  MI->eraseFromParent();
  return true;
}

unsigned ARMFastISel::ARMLowerPICELF(const GlobalValue *GV,
                                     unsigned Align, MVT VT) {
  bool UseGOTOFF = GV->hasLocalLinkage() || GV->hasHiddenVisibility();
  ARMConstantPoolConstant *CPV =
    ARMConstantPoolConstant::Create(GV, UseGOTOFF ? ARMCP::GOTOFF : ARMCP::GOT);
  unsigned Idx = MCP.getConstantPoolIndex(CPV, Align);

  unsigned Opc;
  unsigned DestReg1 = createResultReg(TLI.getRegClassFor(VT));
  // Load value.
  if (isThumb2) {
    AddOptionalDefs(BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL,
                            TII.get(ARM::t2LDRpci), DestReg1)
                    .addConstantPoolIndex(Idx));
    Opc = UseGOTOFF ? ARM::t2ADDrr : ARM::t2LDRs;
  } else {
    // The extra immediate is for addrmode2.
    AddOptionalDefs(BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt,
                            DL, TII.get(ARM::LDRcp), DestReg1)
                    .addConstantPoolIndex(Idx).addImm(0));
    Opc = UseGOTOFF ? ARM::ADDrr : ARM::LDRrs;
  }

  unsigned GlobalBaseReg = AFI->getGlobalBaseReg();
  if (GlobalBaseReg == 0) {
    GlobalBaseReg = MRI.createVirtualRegister(TLI.getRegClassFor(VT));
    AFI->setGlobalBaseReg(GlobalBaseReg);
  }

  unsigned DestReg2 = createResultReg(TLI.getRegClassFor(VT));
  MachineInstrBuilder MIB = BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt,
                                    DL, TII.get(Opc), DestReg2)
                            .addReg(DestReg1)
                            .addReg(GlobalBaseReg);
  if (!UseGOTOFF)
    MIB.addImm(0);
  AddOptionalDefs(MIB);

  return DestReg2;
}

bool ARMFastISel::FastLowerArguments() {
  if (!FuncInfo.CanLowerReturn)
    return false;

  const Function *F = FuncInfo.Fn;
  if (F->isVarArg())
    return false;

  CallingConv::ID CC = F->getCallingConv();
  switch (CC) {
  default:
    return false;
  case CallingConv::Fast:
  case CallingConv::C:
  case CallingConv::ARM_AAPCS_VFP:
  case CallingConv::ARM_AAPCS:
  case CallingConv::ARM_APCS:
    break;
  }

  // Only handle simple cases. i.e. Up to 4 i8/i16/i32 scalar arguments
  // which are passed in r0 - r3.
  unsigned Idx = 1;
  for (Function::const_arg_iterator I = F->arg_begin(), E = F->arg_end();
       I != E; ++I, ++Idx) {
    if (Idx > 4)
      return false;

    if (F->getAttributes().hasAttribute(Idx, Attribute::InReg) ||
        F->getAttributes().hasAttribute(Idx, Attribute::StructRet) ||
        F->getAttributes().hasAttribute(Idx, Attribute::ByVal))
      return false;

    Type *ArgTy = I->getType();
    if (ArgTy->isStructTy() || ArgTy->isArrayTy() || ArgTy->isVectorTy())
      return false;

    EVT ArgVT = TLI.getValueType(ArgTy);
    if (!ArgVT.isSimple()) return false;
    switch (ArgVT.getSimpleVT().SimpleTy) {
    case MVT::i8:
    case MVT::i16:
    case MVT::i32:
      break;
    default:
      return false;
    }
  }


  static const uint16_t GPRArgRegs[] = {
    ARM::R0, ARM::R1, ARM::R2, ARM::R3
  };

  const TargetRegisterClass *RC = TLI.getRegClassFor(MVT::i32);
  Idx = 0;
  for (Function::const_arg_iterator I = F->arg_begin(), E = F->arg_end();
       I != E; ++I, ++Idx) {
    if (I->use_empty())
      continue;
    unsigned SrcReg = GPRArgRegs[Idx];
    unsigned DstReg = FuncInfo.MF->addLiveIn(SrcReg, RC);
    // FIXME: Unfortunately it's necessary to emit a copy from the livein copy.
    // Without this, EmitLiveInCopies may eliminate the livein if its only
    // use is a bitcast (which isn't turned into an instruction).
    unsigned ResultReg = createResultReg(RC);
    BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, TII.get(TargetOpcode::COPY),
            ResultReg).addReg(DstReg, getKillRegState(true));
    UpdateValueMap(I, ResultReg);
  }

  return true;
}

namespace llvm {
  FastISel *ARM::createFastISel(FunctionLoweringInfo &funcInfo,
                                const TargetLibraryInfo *libInfo) {
    // Completely untested on non-iOS.
    const TargetMachine &TM = funcInfo.MF->getTarget();

    // Darwin and thumb1 only for now.
    const ARMSubtarget *Subtarget = &TM.getSubtarget<ARMSubtarget>();
    if (Subtarget->isTargetIOS() && !Subtarget->isThumb1Only())
      return new ARMFastISel(funcInfo, libInfo);
    return 0;
  }
}