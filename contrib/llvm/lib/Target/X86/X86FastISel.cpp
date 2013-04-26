
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

#include "X86.h"
#include "X86ISelLowering.h"
#include "X86InstrBuilder.h"
#include "X86RegisterInfo.h"
#include "X86Subtarget.h"
#include "X86TargetMachine.h"
#include "llvm/CodeGen/Analysis.h"
#include "llvm/CodeGen/FastISel.h"
#include "llvm/CodeGen/FunctionLoweringInfo.h"
#include "llvm/CodeGen/MachineConstantPool.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/IR/CallingConv.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/GlobalAlias.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Operator.h"
#include "llvm/Support/CallSite.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/GetElementPtrTypeIterator.h"
#include "llvm/Target/TargetOptions.h"
using namespace llvm;

namespace {

class X86FastISel : public FastISel {
  /// Subtarget - Keep a pointer to the X86Subtarget around so that we can
  /// make the right decision when generating code for different targets.
  const X86Subtarget *Subtarget;

  /// RegInfo - X86 register info.
  ///
  const X86RegisterInfo *RegInfo;

  /// X86ScalarSSEf32, X86ScalarSSEf64 - Select between SSE or x87
  /// floating point ops.
  /// When SSE is available, use it for f32 operations.
  /// When SSE2 is available, use it for f64 operations.
  bool X86ScalarSSEf64;
  bool X86ScalarSSEf32;

public:
  explicit X86FastISel(FunctionLoweringInfo &funcInfo,
                       const TargetLibraryInfo *libInfo)
    : FastISel(funcInfo, libInfo) {
    Subtarget = &TM.getSubtarget<X86Subtarget>();
    X86ScalarSSEf64 = Subtarget->hasSSE2();
    X86ScalarSSEf32 = Subtarget->hasSSE1();
    RegInfo = static_cast<const X86RegisterInfo*>(TM.getRegisterInfo());
  }

  virtual bool TargetSelectInstruction(const Instruction *I);

  /// TryToFoldLoad - The specified machine instr operand is a vreg, and that
  /// vreg is being provided by the specified load instruction.  If possible,
  /// try to fold the load as an operand to the instruction, returning true if
  /// possible.
  virtual bool TryToFoldLoad(MachineInstr *MI, unsigned OpNo,
                             const LoadInst *LI);

  virtual bool FastLowerArguments();

#include "X86GenFastISel.inc"

private:
  bool X86FastEmitCompare(const Value *LHS, const Value *RHS, EVT VT);

  bool X86FastEmitLoad(EVT VT, const X86AddressMode &AM, unsigned &RR);

  bool X86FastEmitStore(EVT VT, const Value *Val, const X86AddressMode &AM);
  bool X86FastEmitStore(EVT VT, unsigned Val, const X86AddressMode &AM);

  bool X86FastEmitExtend(ISD::NodeType Opc, EVT DstVT, unsigned Src, EVT SrcVT,
                         unsigned &ResultReg);

  bool X86SelectAddress(const Value *V, X86AddressMode &AM);
  bool X86SelectCallAddress(const Value *V, X86AddressMode &AM);

  bool X86SelectLoad(const Instruction *I);

  bool X86SelectStore(const Instruction *I);

  bool X86SelectRet(const Instruction *I);

  bool X86SelectCmp(const Instruction *I);

  bool X86SelectZExt(const Instruction *I);

  bool X86SelectBranch(const Instruction *I);

  bool X86SelectShift(const Instruction *I);

  bool X86SelectSelect(const Instruction *I);

  bool X86SelectTrunc(const Instruction *I);

  bool X86SelectFPExt(const Instruction *I);
  bool X86SelectFPTrunc(const Instruction *I);

  bool X86VisitIntrinsicCall(const IntrinsicInst &I);
  bool X86SelectCall(const Instruction *I);

  bool DoSelectCall(const Instruction *I, const char *MemIntName);

  const X86InstrInfo *getInstrInfo() const {
    return getTargetMachine()->getInstrInfo();
  }
  const X86TargetMachine *getTargetMachine() const {
    return static_cast<const X86TargetMachine *>(&TM);
  }

  unsigned TargetMaterializeConstant(const Constant *C);

  unsigned TargetMaterializeAlloca(const AllocaInst *C);

  unsigned TargetMaterializeFloatZero(const ConstantFP *CF);

  /// isScalarFPTypeInSSEReg - Return true if the specified scalar FP type is
  /// computed in an SSE register, not on the X87 floating point stack.
  bool isScalarFPTypeInSSEReg(EVT VT) const {
    return (VT == MVT::f64 && X86ScalarSSEf64) || // f64 is when SSE2
      (VT == MVT::f32 && X86ScalarSSEf32);   // f32 is when SSE1
  }

  bool isTypeLegal(Type *Ty, MVT &VT, bool AllowI1 = false);

  bool IsMemcpySmall(uint64_t Len);

  bool TryEmitSmallMemcpy(X86AddressMode DestAM,
                          X86AddressMode SrcAM, uint64_t Len);
};

} // end anonymous namespace.

bool X86FastISel::isTypeLegal(Type *Ty, MVT &VT, bool AllowI1) {
  EVT evt = TLI.getValueType(Ty, /*HandleUnknown=*/true);
  if (evt == MVT::Other || !evt.isSimple())
    // Unhandled type. Halt "fast" selection and bail.
    return false;

  VT = evt.getSimpleVT();
  // For now, require SSE/SSE2 for performing floating-point operations,
  // since x87 requires additional work.
  if (VT == MVT::f64 && !X86ScalarSSEf64)
    return false;
  if (VT == MVT::f32 && !X86ScalarSSEf32)
    return false;
  // Similarly, no f80 support yet.
  if (VT == MVT::f80)
    return false;
  // We only handle legal types. For example, on x86-32 the instruction
  // selector contains all of the 64-bit instructions from x86-64,
  // under the assumption that i64 won't be used if the target doesn't
  // support it.
  return (AllowI1 && VT == MVT::i1) || TLI.isTypeLegal(VT);
}

#include "X86GenCallingConv.inc"

/// X86FastEmitLoad - Emit a machine instruction to load a value of type VT.
/// The address is either pre-computed, i.e. Ptr, or a GlobalAddress, i.e. GV.
/// Return true and the result register by reference if it is possible.
bool X86FastISel::X86FastEmitLoad(EVT VT, const X86AddressMode &AM,
                                  unsigned &ResultReg) {
  // Get opcode and regclass of the output for the given load instruction.
  unsigned Opc = 0;
  const TargetRegisterClass *RC = NULL;
  switch (VT.getSimpleVT().SimpleTy) {
  default: return false;
  case MVT::i1:
  case MVT::i8:
    Opc = X86::MOV8rm;
    RC  = &X86::GR8RegClass;
    break;
  case MVT::i16:
    Opc = X86::MOV16rm;
    RC  = &X86::GR16RegClass;
    break;
  case MVT::i32:
    Opc = X86::MOV32rm;
    RC  = &X86::GR32RegClass;
    break;
  case MVT::i64:
    // Must be in x86-64 mode.
    Opc = X86::MOV64rm;
    RC  = &X86::GR64RegClass;
    break;
  case MVT::f32:
    if (X86ScalarSSEf32) {
      Opc = Subtarget->hasAVX() ? X86::VMOVSSrm : X86::MOVSSrm;
      RC  = &X86::FR32RegClass;
    } else {
      Opc = X86::LD_Fp32m;
      RC  = &X86::RFP32RegClass;
    }
    break;
  case MVT::f64:
    if (X86ScalarSSEf64) {
      Opc = Subtarget->hasAVX() ? X86::VMOVSDrm : X86::MOVSDrm;
      RC  = &X86::FR64RegClass;
    } else {
      Opc = X86::LD_Fp64m;
      RC  = &X86::RFP64RegClass;
    }
    break;
  case MVT::f80:
    // No f80 support yet.
    return false;
  }

  ResultReg = createResultReg(RC);
  addFullAddress(BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt,
                         DL, TII.get(Opc), ResultReg), AM);
  return true;
}

/// X86FastEmitStore - Emit a machine instruction to store a value Val of
/// type VT. The address is either pre-computed, consisted of a base ptr, Ptr
/// and a displacement offset, or a GlobalAddress,
/// i.e. V. Return true if it is possible.
bool
X86FastISel::X86FastEmitStore(EVT VT, unsigned Val, const X86AddressMode &AM) {
  // Get opcode and regclass of the output for the given store instruction.
  unsigned Opc = 0;
  switch (VT.getSimpleVT().SimpleTy) {
  case MVT::f80: // No f80 support yet.
  default: return false;
  case MVT::i1: {
    // Mask out all but lowest bit.
    unsigned AndResult = createResultReg(&X86::GR8RegClass);
    BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL,
            TII.get(X86::AND8ri), AndResult).addReg(Val).addImm(1);
    Val = AndResult;
  }
  // FALLTHROUGH, handling i1 as i8.
  case MVT::i8:  Opc = X86::MOV8mr;  break;
  case MVT::i16: Opc = X86::MOV16mr; break;
  case MVT::i32: Opc = X86::MOV32mr; break;
  case MVT::i64: Opc = X86::MOV64mr; break; // Must be in x86-64 mode.
  case MVT::f32:
    Opc = X86ScalarSSEf32 ?
          (Subtarget->hasAVX() ? X86::VMOVSSmr : X86::MOVSSmr) : X86::ST_Fp32m;
    break;
  case MVT::f64:
    Opc = X86ScalarSSEf64 ?
          (Subtarget->hasAVX() ? X86::VMOVSDmr : X86::MOVSDmr) : X86::ST_Fp64m;
    break;
  case MVT::v4f32:
    Opc = X86::MOVAPSmr;
    break;
  case MVT::v2f64:
    Opc = X86::MOVAPDmr;
    break;
  case MVT::v4i32:
  case MVT::v2i64:
  case MVT::v8i16:
  case MVT::v16i8:
    Opc = X86::MOVDQAmr;
    break;
  }

  addFullAddress(BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt,
                         DL, TII.get(Opc)), AM).addReg(Val);
  return true;
}

bool X86FastISel::X86FastEmitStore(EVT VT, const Value *Val,
                                   const X86AddressMode &AM) {
  // Handle 'null' like i32/i64 0.
  if (isa<ConstantPointerNull>(Val))
    Val = Constant::getNullValue(TD.getIntPtrType(Val->getContext()));

  // If this is a store of a simple constant, fold the constant into the store.
  if (const ConstantInt *CI = dyn_cast<ConstantInt>(Val)) {
    unsigned Opc = 0;
    bool Signed = true;
    switch (VT.getSimpleVT().SimpleTy) {
    default: break;
    case MVT::i1:  Signed = false;     // FALLTHROUGH to handle as i8.
    case MVT::i8:  Opc = X86::MOV8mi;  break;
    case MVT::i16: Opc = X86::MOV16mi; break;
    case MVT::i32: Opc = X86::MOV32mi; break;
    case MVT::i64:
      // Must be a 32-bit sign extended value.
      if (isInt<32>(CI->getSExtValue()))
        Opc = X86::MOV64mi32;
      break;
    }

    if (Opc) {
      addFullAddress(BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt,
                             DL, TII.get(Opc)), AM)
                             .addImm(Signed ? (uint64_t) CI->getSExtValue() :
                                              CI->getZExtValue());
      return true;
    }
  }

  unsigned ValReg = getRegForValue(Val);
  if (ValReg == 0)
    return false;

  return X86FastEmitStore(VT, ValReg, AM);
}

/// X86FastEmitExtend - Emit a machine instruction to extend a value Src of
/// type SrcVT to type DstVT using the specified extension opcode Opc (e.g.
/// ISD::SIGN_EXTEND).
bool X86FastISel::X86FastEmitExtend(ISD::NodeType Opc, EVT DstVT,
                                    unsigned Src, EVT SrcVT,
                                    unsigned &ResultReg) {
  unsigned RR = FastEmit_r(SrcVT.getSimpleVT(), DstVT.getSimpleVT(), Opc,
                           Src, /*TODO: Kill=*/false);
  if (RR == 0)
    return false;

  ResultReg = RR;
  return true;
}

/// X86SelectAddress - Attempt to fill in an address from the given value.
///
bool X86FastISel::X86SelectAddress(const Value *V, X86AddressMode &AM) {
  const User *U = NULL;
  unsigned Opcode = Instruction::UserOp1;
  if (const Instruction *I = dyn_cast<Instruction>(V)) {
    // Don't walk into other basic blocks; it's possible we haven't
    // visited them yet, so the instructions may not yet be assigned
    // virtual registers.
    if (FuncInfo.StaticAllocaMap.count(static_cast<const AllocaInst *>(V)) ||
        FuncInfo.MBBMap[I->getParent()] == FuncInfo.MBB) {
      Opcode = I->getOpcode();
      U = I;
    }
  } else if (const ConstantExpr *C = dyn_cast<ConstantExpr>(V)) {
    Opcode = C->getOpcode();
    U = C;
  }

  if (PointerType *Ty = dyn_cast<PointerType>(V->getType()))
    if (Ty->getAddressSpace() > 255)
      // Fast instruction selection doesn't support the special
      // address spaces.
      return false;

  switch (Opcode) {
  default: break;
  case Instruction::BitCast:
    // Look past bitcasts.
    return X86SelectAddress(U->getOperand(0), AM);

  case Instruction::IntToPtr:
    // Look past no-op inttoptrs.
    if (TLI.getValueType(U->getOperand(0)->getType()) == TLI.getPointerTy())
      return X86SelectAddress(U->getOperand(0), AM);
    break;

  case Instruction::PtrToInt:
    // Look past no-op ptrtoints.
    if (TLI.getValueType(U->getType()) == TLI.getPointerTy())
      return X86SelectAddress(U->getOperand(0), AM);
    break;

  case Instruction::Alloca: {
    // Do static allocas.
    const AllocaInst *A = cast<AllocaInst>(V);
    DenseMap<const AllocaInst*, int>::iterator SI =
      FuncInfo.StaticAllocaMap.find(A);
    if (SI != FuncInfo.StaticAllocaMap.end()) {
      AM.BaseType = X86AddressMode::FrameIndexBase;
      AM.Base.FrameIndex = SI->second;
      return true;
    }
    break;
  }

  case Instruction::Add: {
    // Adds of constants are common and easy enough.
    if (const ConstantInt *CI = dyn_cast<ConstantInt>(U->getOperand(1))) {
      uint64_t Disp = (int32_t)AM.Disp + (uint64_t)CI->getSExtValue();
      // They have to fit in the 32-bit signed displacement field though.
      if (isInt<32>(Disp)) {
        AM.Disp = (uint32_t)Disp;
        return X86SelectAddress(U->getOperand(0), AM);
      }
    }
    break;
  }

  case Instruction::GetElementPtr: {
    X86AddressMode SavedAM = AM;

    // Pattern-match simple GEPs.
    uint64_t Disp = (int32_t)AM.Disp;
    unsigned IndexReg = AM.IndexReg;
    unsigned Scale = AM.Scale;
    gep_type_iterator GTI = gep_type_begin(U);
    // Iterate through the indices, folding what we can. Constants can be
    // folded, and one dynamic index can be handled, if the scale is supported.
    for (User::const_op_iterator i = U->op_begin() + 1, e = U->op_end();
         i != e; ++i, ++GTI) {
      const Value *Op = *i;
      if (StructType *STy = dyn_cast<StructType>(*GTI)) {
        const StructLayout *SL = TD.getStructLayout(STy);
        Disp += SL->getElementOffset(cast<ConstantInt>(Op)->getZExtValue());
        continue;
      }

      // A array/variable index is always of the form i*S where S is the
      // constant scale size.  See if we can push the scale into immediates.
      uint64_t S = TD.getTypeAllocSize(GTI.getIndexedType());
      for (;;) {
        if (const ConstantInt *CI = dyn_cast<ConstantInt>(Op)) {
          // Constant-offset addressing.
          Disp += CI->getSExtValue() * S;
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
          Disp += CI->getSExtValue() * S;
          // Iterate on the other operand.
          Op = cast<AddOperator>(Op)->getOperand(0);
          continue;
        }
        if (IndexReg == 0 &&
            (!AM.GV || !Subtarget->isPICStyleRIPRel()) &&
            (S == 1 || S == 2 || S == 4 || S == 8)) {
          // Scaled-index addressing.
          Scale = S;
          IndexReg = getRegForGEPIndex(Op).first;
          if (IndexReg == 0)
            return false;
          break;
        }
        // Unsupported.
        goto unsupported_gep;
      }
    }
    // Check for displacement overflow.
    if (!isInt<32>(Disp))
      break;
    // Ok, the GEP indices were covered by constant-offset and scaled-index
    // addressing. Update the address state and move on to examining the base.
    AM.IndexReg = IndexReg;
    AM.Scale = Scale;
    AM.Disp = (uint32_t)Disp;
    if (X86SelectAddress(U->getOperand(0), AM))
      return true;

    // If we couldn't merge the gep value into this addr mode, revert back to
    // our address and just match the value instead of completely failing.
    AM = SavedAM;
    break;
  unsupported_gep:
    // Ok, the GEP indices weren't all covered.
    break;
  }
  }

  // Handle constant address.
  if (const GlobalValue *GV = dyn_cast<GlobalValue>(V)) {
    // Can't handle alternate code models yet.
    if (TM.getCodeModel() != CodeModel::Small)
      return false;

    // Can't handle TLS yet.
    if (const GlobalVariable *GVar = dyn_cast<GlobalVariable>(GV))
      if (GVar->isThreadLocal())
        return false;

    // Can't handle TLS yet, part 2 (this is slightly crazy, but this is how
    // it works...).
    if (const GlobalAlias *GA = dyn_cast<GlobalAlias>(GV))
      if (const GlobalVariable *GVar =
            dyn_cast_or_null<GlobalVariable>(GA->resolveAliasedGlobal(false)))
        if (GVar->isThreadLocal())
          return false;

    // RIP-relative addresses can't have additional register operands, so if
    // we've already folded stuff into the addressing mode, just force the
    // global value into its own register, which we can use as the basereg.
    if (!Subtarget->isPICStyleRIPRel() ||
        (AM.Base.Reg == 0 && AM.IndexReg == 0)) {
      // Okay, we've committed to selecting this global. Set up the address.
      AM.GV = GV;

      // Allow the subtarget to classify the global.
      unsigned char GVFlags = Subtarget->ClassifyGlobalReference(GV, TM);

      // If this reference is relative to the pic base, set it now.
      if (isGlobalRelativeToPICBase(GVFlags)) {
        // FIXME: How do we know Base.Reg is free??
        AM.Base.Reg = getInstrInfo()->getGlobalBaseReg(FuncInfo.MF);
      }

      // Unless the ABI requires an extra load, return a direct reference to
      // the global.
      if (!isGlobalStubReference(GVFlags)) {
        if (Subtarget->isPICStyleRIPRel()) {
          // Use rip-relative addressing if we can.  Above we verified that the
          // base and index registers are unused.
          assert(AM.Base.Reg == 0 && AM.IndexReg == 0);
          AM.Base.Reg = X86::RIP;
        }
        AM.GVOpFlags = GVFlags;
        return true;
      }

      // Ok, we need to do a load from a stub.  If we've already loaded from
      // this stub, reuse the loaded pointer, otherwise emit the load now.
      DenseMap<const Value*, unsigned>::iterator I = LocalValueMap.find(V);
      unsigned LoadReg;
      if (I != LocalValueMap.end() && I->second != 0) {
        LoadReg = I->second;
      } else {
        // Issue load from stub.
        unsigned Opc = 0;
        const TargetRegisterClass *RC = NULL;
        X86AddressMode StubAM;
        StubAM.Base.Reg = AM.Base.Reg;
        StubAM.GV = GV;
        StubAM.GVOpFlags = GVFlags;

        // Prepare for inserting code in the local-value area.
        SavePoint SaveInsertPt = enterLocalValueArea();

        if (TLI.getPointerTy() == MVT::i64) {
          Opc = X86::MOV64rm;
          RC  = &X86::GR64RegClass;

          if (Subtarget->isPICStyleRIPRel())
            StubAM.Base.Reg = X86::RIP;
        } else {
          Opc = X86::MOV32rm;
          RC  = &X86::GR32RegClass;
        }

        LoadReg = createResultReg(RC);
        MachineInstrBuilder LoadMI =
          BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, TII.get(Opc), LoadReg);
        addFullAddress(LoadMI, StubAM);

        // Ok, back to normal mode.
        leaveLocalValueArea(SaveInsertPt);

        // Prevent loading GV stub multiple times in same MBB.
        LocalValueMap[V] = LoadReg;
      }

      // Now construct the final address. Note that the Disp, Scale,
      // and Index values may already be set here.
      AM.Base.Reg = LoadReg;
      AM.GV = 0;
      return true;
    }
  }

  // If all else fails, try to materialize the value in a register.
  if (!AM.GV || !Subtarget->isPICStyleRIPRel()) {
    if (AM.Base.Reg == 0) {
      AM.Base.Reg = getRegForValue(V);
      return AM.Base.Reg != 0;
    }
    if (AM.IndexReg == 0) {
      assert(AM.Scale == 1 && "Scale with no index!");
      AM.IndexReg = getRegForValue(V);
      return AM.IndexReg != 0;
    }
  }

  return false;
}

/// X86SelectCallAddress - Attempt to fill in an address from the given value.
///
bool X86FastISel::X86SelectCallAddress(const Value *V, X86AddressMode &AM) {
  const User *U = NULL;
  unsigned Opcode = Instruction::UserOp1;
  if (const Instruction *I = dyn_cast<Instruction>(V)) {
    Opcode = I->getOpcode();
    U = I;
  } else if (const ConstantExpr *C = dyn_cast<ConstantExpr>(V)) {
    Opcode = C->getOpcode();
    U = C;
  }

  switch (Opcode) {
  default: break;
  case Instruction::BitCast:
    // Look past bitcasts.
    return X86SelectCallAddress(U->getOperand(0), AM);

  case Instruction::IntToPtr:
    // Look past no-op inttoptrs.
    if (TLI.getValueType(U->getOperand(0)->getType()) == TLI.getPointerTy())
      return X86SelectCallAddress(U->getOperand(0), AM);
    break;

  case Instruction::PtrToInt:
    // Look past no-op ptrtoints.
    if (TLI.getValueType(U->getType()) == TLI.getPointerTy())
      return X86SelectCallAddress(U->getOperand(0), AM);
    break;
  }

  // Handle constant address.
  if (const GlobalValue *GV = dyn_cast<GlobalValue>(V)) {
    // Can't handle alternate code models yet.
    if (TM.getCodeModel() != CodeModel::Small)
      return false;

    // RIP-relative addresses can't have additional register operands.
    if (Subtarget->isPICStyleRIPRel() &&
        (AM.Base.Reg != 0 || AM.IndexReg != 0))
      return false;

    // Can't handle DLLImport.
    if (GV->hasDLLImportLinkage())
      return false;

    // Can't handle TLS.
    if (const GlobalVariable *GVar = dyn_cast<GlobalVariable>(GV))
      if (GVar->isThreadLocal())
        return false;

    // Okay, we've committed to selecting this global. Set up the basic address.
    AM.GV = GV;

    // No ABI requires an extra load for anything other than DLLImport, which
    // we rejected above. Return a direct reference to the global.
    if (Subtarget->isPICStyleRIPRel()) {
      // Use rip-relative addressing if we can.  Above we verified that the
      // base and index registers are unused.
      assert(AM.Base.Reg == 0 && AM.IndexReg == 0);
      AM.Base.Reg = X86::RIP;
    } else if (Subtarget->isPICStyleStubPIC()) {
      AM.GVOpFlags = X86II::MO_PIC_BASE_OFFSET;
    } else if (Subtarget->isPICStyleGOT()) {
      AM.GVOpFlags = X86II::MO_GOTOFF;
    }

    return true;
  }

  // If all else fails, try to materialize the value in a register.
  if (!AM.GV || !Subtarget->isPICStyleRIPRel()) {
    if (AM.Base.Reg == 0) {
      AM.Base.Reg = getRegForValue(V);
      return AM.Base.Reg != 0;
    }
    if (AM.IndexReg == 0) {
      assert(AM.Scale == 1 && "Scale with no index!");
      AM.IndexReg = getRegForValue(V);
      return AM.IndexReg != 0;
    }
  }

  return false;
}


/// X86SelectStore - Select and emit code to implement store instructions.
bool X86FastISel::X86SelectStore(const Instruction *I) {
  // Atomic stores need special handling.
  const StoreInst *S = cast<StoreInst>(I);

  if (S->isAtomic())
    return false;

  unsigned SABIAlignment =
    TD.getABITypeAlignment(S->getValueOperand()->getType());
  if (S->getAlignment() != 0 && S->getAlignment() < SABIAlignment)
    return false;

  MVT VT;
  if (!isTypeLegal(I->getOperand(0)->getType(), VT, /*AllowI1=*/true))
    return false;

  X86AddressMode AM;
  if (!X86SelectAddress(I->getOperand(1), AM))
    return false;

  return X86FastEmitStore(VT, I->getOperand(0), AM);
}

/// X86SelectRet - Select and emit code to implement ret instructions.
bool X86FastISel::X86SelectRet(const Instruction *I) {
  const ReturnInst *Ret = cast<ReturnInst>(I);
  const Function &F = *I->getParent()->getParent();
  const X86MachineFunctionInfo *X86MFInfo =
      FuncInfo.MF->getInfo<X86MachineFunctionInfo>();

  if (!FuncInfo.CanLowerReturn)
    return false;

  CallingConv::ID CC = F.getCallingConv();
  if (CC != CallingConv::C &&
      CC != CallingConv::Fast &&
      CC != CallingConv::X86_FastCall)
    return false;

  if (Subtarget->isTargetWin64())
    return false;

  // Don't handle popping bytes on return for now.
  if (X86MFInfo->getBytesToPopOnReturn() != 0)
    return false;

  // fastcc with -tailcallopt is intended to provide a guaranteed
  // tail call optimization. Fastisel doesn't know how to do that.
  if (CC == CallingConv::Fast && TM.Options.GuaranteedTailCallOpt)
    return false;

  // Let SDISel handle vararg functions.
  if (F.isVarArg())
    return false;

  // Build a list of return value registers.
  SmallVector<unsigned, 4> RetRegs;

  if (Ret->getNumOperands() > 0) {
    SmallVector<ISD::OutputArg, 4> Outs;
    GetReturnInfo(F.getReturnType(), F.getAttributes(), Outs, TLI);

    // Analyze operands of the call, assigning locations to each operand.
    SmallVector<CCValAssign, 16> ValLocs;
    CCState CCInfo(CC, F.isVarArg(), *FuncInfo.MF, TM, ValLocs,
                   I->getContext());
    CCInfo.AnalyzeReturn(Outs, RetCC_X86);

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

    // The calling-convention tables for x87 returns don't tell
    // the whole story.
    if (VA.getLocReg() == X86::ST0 || VA.getLocReg() == X86::ST1)
      return false;

    unsigned SrcReg = Reg + VA.getValNo();
    EVT SrcVT = TLI.getValueType(RV->getType());
    EVT DstVT = VA.getValVT();
    // Special handling for extended integers.
    if (SrcVT != DstVT) {
      if (SrcVT != MVT::i1 && SrcVT != MVT::i8 && SrcVT != MVT::i16)
        return false;

      if (!Outs[0].Flags.isZExt() && !Outs[0].Flags.isSExt())
        return false;

      assert(DstVT == MVT::i32 && "X86 should always ext to i32");

      if (SrcVT == MVT::i1) {
        if (Outs[0].Flags.isSExt())
          return false;
        SrcReg = FastEmitZExtFromI1(MVT::i8, SrcReg, /*TODO: Kill=*/false);
        SrcVT = MVT::i8;
      }
      unsigned Op = Outs[0].Flags.isZExt() ? ISD::ZERO_EXTEND :
                                             ISD::SIGN_EXTEND;
      SrcReg = FastEmit_r(SrcVT.getSimpleVT(), DstVT.getSimpleVT(), Op,
                          SrcReg, /*TODO: Kill=*/false);
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

  // The x86-64 ABI for returning structs by value requires that we copy
  // the sret argument into %rax for the return. We saved the argument into
  // a virtual register in the entry block, so now we copy the value out
  // and into %rax. We also do the same with %eax for Win32.
  if (F.hasStructRetAttr() &&
      (Subtarget->is64Bit() || Subtarget->isTargetWindows())) {
    unsigned Reg = X86MFInfo->getSRetReturnReg();
    assert(Reg &&
           "SRetReturnReg should have been set in LowerFormalArguments()!");
    unsigned RetReg = Subtarget->is64Bit() ? X86::RAX : X86::EAX;
    BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, TII.get(TargetOpcode::COPY),
            RetReg).addReg(Reg);
    RetRegs.push_back(RetReg);
  }

  // Now emit the RET.
  MachineInstrBuilder MIB =
    BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, TII.get(X86::RET));
  for (unsigned i = 0, e = RetRegs.size(); i != e; ++i)
    MIB.addReg(RetRegs[i], RegState::Implicit);
  return true;
}

/// X86SelectLoad - Select and emit code to implement load instructions.
///
bool X86FastISel::X86SelectLoad(const Instruction *I)  {
  // Atomic loads need special handling.
  if (cast<LoadInst>(I)->isAtomic())
    return false;

  MVT VT;
  if (!isTypeLegal(I->getType(), VT, /*AllowI1=*/true))
    return false;

  X86AddressMode AM;
  if (!X86SelectAddress(I->getOperand(0), AM))
    return false;

  unsigned ResultReg = 0;
  if (X86FastEmitLoad(VT, AM, ResultReg)) {
    UpdateValueMap(I, ResultReg);
    return true;
  }
  return false;
}

static unsigned X86ChooseCmpOpcode(EVT VT, const X86Subtarget *Subtarget) {
  bool HasAVX = Subtarget->hasAVX();
  bool X86ScalarSSEf32 = Subtarget->hasSSE1();
  bool X86ScalarSSEf64 = Subtarget->hasSSE2();

  switch (VT.getSimpleVT().SimpleTy) {
  default:       return 0;
  case MVT::i8:  return X86::CMP8rr;
  case MVT::i16: return X86::CMP16rr;
  case MVT::i32: return X86::CMP32rr;
  case MVT::i64: return X86::CMP64rr;
  case MVT::f32:
    return X86ScalarSSEf32 ? (HasAVX ? X86::VUCOMISSrr : X86::UCOMISSrr) : 0;
  case MVT::f64:
    return X86ScalarSSEf64 ? (HasAVX ? X86::VUCOMISDrr : X86::UCOMISDrr) : 0;
  }
}

/// X86ChooseCmpImmediateOpcode - If we have a comparison with RHS as the RHS
/// of the comparison, return an opcode that works for the compare (e.g.
/// CMP32ri) otherwise return 0.
static unsigned X86ChooseCmpImmediateOpcode(EVT VT, const ConstantInt *RHSC) {
  switch (VT.getSimpleVT().SimpleTy) {
  // Otherwise, we can't fold the immediate into this comparison.
  default: return 0;
  case MVT::i8: return X86::CMP8ri;
  case MVT::i16: return X86::CMP16ri;
  case MVT::i32: return X86::CMP32ri;
  case MVT::i64:
    // 64-bit comparisons are only valid if the immediate fits in a 32-bit sext
    // field.
    if ((int)RHSC->getSExtValue() == RHSC->getSExtValue())
      return X86::CMP64ri32;
    return 0;
  }
}

bool X86FastISel::X86FastEmitCompare(const Value *Op0, const Value *Op1,
                                     EVT VT) {
  unsigned Op0Reg = getRegForValue(Op0);
  if (Op0Reg == 0) return false;

  // Handle 'null' like i32/i64 0.
  if (isa<ConstantPointerNull>(Op1))
    Op1 = Constant::getNullValue(TD.getIntPtrType(Op0->getContext()));

  // We have two options: compare with register or immediate.  If the RHS of
  // the compare is an immediate that we can fold into this compare, use
  // CMPri, otherwise use CMPrr.
  if (const ConstantInt *Op1C = dyn_cast<ConstantInt>(Op1)) {
    if (unsigned CompareImmOpc = X86ChooseCmpImmediateOpcode(VT, Op1C)) {
      BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, TII.get(CompareImmOpc))
        .addReg(Op0Reg)
        .addImm(Op1C->getSExtValue());
      return true;
    }
  }

  unsigned CompareOpc = X86ChooseCmpOpcode(VT, Subtarget);
  if (CompareOpc == 0) return false;

  unsigned Op1Reg = getRegForValue(Op1);
  if (Op1Reg == 0) return false;
  BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, TII.get(CompareOpc))
    .addReg(Op0Reg)
    .addReg(Op1Reg);

  return true;
}

bool X86FastISel::X86SelectCmp(const Instruction *I) {
  const CmpInst *CI = cast<CmpInst>(I);

  MVT VT;
  if (!isTypeLegal(I->getOperand(0)->getType(), VT))
    return false;

  unsigned ResultReg = createResultReg(&X86::GR8RegClass);
  unsigned SetCCOpc;
  bool SwapArgs;  // false -> compare Op0, Op1.  true -> compare Op1, Op0.
  switch (CI->getPredicate()) {
  case CmpInst::FCMP_OEQ: {
    if (!X86FastEmitCompare(CI->getOperand(0), CI->getOperand(1), VT))
      return false;

    unsigned EReg = createResultReg(&X86::GR8RegClass);
    unsigned NPReg = createResultReg(&X86::GR8RegClass);
    BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, TII.get(X86::SETEr), EReg);
    BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL,
            TII.get(X86::SETNPr), NPReg);
    BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL,
            TII.get(X86::AND8rr), ResultReg).addReg(NPReg).addReg(EReg);
    UpdateValueMap(I, ResultReg);
    return true;
  }
  case CmpInst::FCMP_UNE: {
    if (!X86FastEmitCompare(CI->getOperand(0), CI->getOperand(1), VT))
      return false;

    unsigned NEReg = createResultReg(&X86::GR8RegClass);
    unsigned PReg = createResultReg(&X86::GR8RegClass);
    BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, TII.get(X86::SETNEr), NEReg);
    BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, TII.get(X86::SETPr), PReg);
    BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, TII.get(X86::OR8rr),ResultReg)
      .addReg(PReg).addReg(NEReg);
    UpdateValueMap(I, ResultReg);
    return true;
  }
  case CmpInst::FCMP_OGT: SwapArgs = false; SetCCOpc = X86::SETAr;  break;
  case CmpInst::FCMP_OGE: SwapArgs = false; SetCCOpc = X86::SETAEr; break;
  case CmpInst::FCMP_OLT: SwapArgs = true;  SetCCOpc = X86::SETAr;  break;
  case CmpInst::FCMP_OLE: SwapArgs = true;  SetCCOpc = X86::SETAEr; break;
  case CmpInst::FCMP_ONE: SwapArgs = false; SetCCOpc = X86::SETNEr; break;
  case CmpInst::FCMP_ORD: SwapArgs = false; SetCCOpc = X86::SETNPr; break;
  case CmpInst::FCMP_UNO: SwapArgs = false; SetCCOpc = X86::SETPr;  break;
  case CmpInst::FCMP_UEQ: SwapArgs = false; SetCCOpc = X86::SETEr;  break;
  case CmpInst::FCMP_UGT: SwapArgs = true;  SetCCOpc = X86::SETBr;  break;
  case CmpInst::FCMP_UGE: SwapArgs = true;  SetCCOpc = X86::SETBEr; break;
  case CmpInst::FCMP_ULT: SwapArgs = false; SetCCOpc = X86::SETBr;  break;
  case CmpInst::FCMP_ULE: SwapArgs = false; SetCCOpc = X86::SETBEr; break;

  case CmpInst::ICMP_EQ:  SwapArgs = false; SetCCOpc = X86::SETEr;  break;
  case CmpInst::ICMP_NE:  SwapArgs = false; SetCCOpc = X86::SETNEr; break;
  case CmpInst::ICMP_UGT: SwapArgs = false; SetCCOpc = X86::SETAr;  break;
  case CmpInst::ICMP_UGE: SwapArgs = false; SetCCOpc = X86::SETAEr; break;
  case CmpInst::ICMP_ULT: SwapArgs = false; SetCCOpc = X86::SETBr;  break;
  case CmpInst::ICMP_ULE: SwapArgs = false; SetCCOpc = X86::SETBEr; break;
  case CmpInst::ICMP_SGT: SwapArgs = false; SetCCOpc = X86::SETGr;  break;
  case CmpInst::ICMP_SGE: SwapArgs = false; SetCCOpc = X86::SETGEr; break;
  case CmpInst::ICMP_SLT: SwapArgs = false; SetCCOpc = X86::SETLr;  break;
  case CmpInst::ICMP_SLE: SwapArgs = false; SetCCOpc = X86::SETLEr; break;
  default:
    return false;
  }

  const Value *Op0 = CI->getOperand(0), *Op1 = CI->getOperand(1);
  if (SwapArgs)
    std::swap(Op0, Op1);

  // Emit a compare of Op0/Op1.
  if (!X86FastEmitCompare(Op0, Op1, VT))
    return false;

  BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, TII.get(SetCCOpc), ResultReg);
  UpdateValueMap(I, ResultReg);
  return true;
}

bool X86FastISel::X86SelectZExt(const Instruction *I) {
  // Handle zero-extension from i1 to i8, which is common.
  if (!I->getOperand(0)->getType()->isIntegerTy(1))
    return false;

  EVT DstVT = TLI.getValueType(I->getType());
  if (!TLI.isTypeLegal(DstVT))
    return false;

  unsigned ResultReg = getRegForValue(I->getOperand(0));
  if (ResultReg == 0)
    return false;

  // Set the high bits to zero.
  ResultReg = FastEmitZExtFromI1(MVT::i8, ResultReg, /*TODO: Kill=*/false);
  if (ResultReg == 0)
    return false;

  if (DstVT != MVT::i8) {
    ResultReg = FastEmit_r(MVT::i8, DstVT.getSimpleVT(), ISD::ZERO_EXTEND,
                           ResultReg, /*Kill=*/true);
    if (ResultReg == 0)
      return false;
  }

  UpdateValueMap(I, ResultReg);
  return true;
}


bool X86FastISel::X86SelectBranch(const Instruction *I) {
  // Unconditional branches are selected by tablegen-generated code.
  // Handle a conditional branch.
  const BranchInst *BI = cast<BranchInst>(I);
  MachineBasicBlock *TrueMBB = FuncInfo.MBBMap[BI->getSuccessor(0)];
  MachineBasicBlock *FalseMBB = FuncInfo.MBBMap[BI->getSuccessor(1)];

  // Fold the common case of a conditional branch with a comparison
  // in the same block (values defined on other blocks may not have
  // initialized registers).
  if (const CmpInst *CI = dyn_cast<CmpInst>(BI->getCondition())) {
    if (CI->hasOneUse() && CI->getParent() == I->getParent()) {
      EVT VT = TLI.getValueType(CI->getOperand(0)->getType());

      // Try to take advantage of fallthrough opportunities.
      CmpInst::Predicate Predicate = CI->getPredicate();
      if (FuncInfo.MBB->isLayoutSuccessor(TrueMBB)) {
        std::swap(TrueMBB, FalseMBB);
        Predicate = CmpInst::getInversePredicate(Predicate);
      }

      bool SwapArgs;  // false -> compare Op0, Op1.  true -> compare Op1, Op0.
      unsigned BranchOpc; // Opcode to jump on, e.g. "X86::JA"

      switch (Predicate) {
      case CmpInst::FCMP_OEQ:
        std::swap(TrueMBB, FalseMBB);
        Predicate = CmpInst::FCMP_UNE;
        // FALL THROUGH
      case CmpInst::FCMP_UNE: SwapArgs = false; BranchOpc = X86::JNE_4; break;
      case CmpInst::FCMP_OGT: SwapArgs = false; BranchOpc = X86::JA_4;  break;
      case CmpInst::FCMP_OGE: SwapArgs = false; BranchOpc = X86::JAE_4; break;
      case CmpInst::FCMP_OLT: SwapArgs = true;  BranchOpc = X86::JA_4;  break;
      case CmpInst::FCMP_OLE: SwapArgs = true;  BranchOpc = X86::JAE_4; break;
      case CmpInst::FCMP_ONE: SwapArgs = false; BranchOpc = X86::JNE_4; break;
      case CmpInst::FCMP_ORD: SwapArgs = false; BranchOpc = X86::JNP_4; break;
      case CmpInst::FCMP_UNO: SwapArgs = false; BranchOpc = X86::JP_4;  break;
      case CmpInst::FCMP_UEQ: SwapArgs = false; BranchOpc = X86::JE_4;  break;
      case CmpInst::FCMP_UGT: SwapArgs = true;  BranchOpc = X86::JB_4;  break;
      case CmpInst::FCMP_UGE: SwapArgs = true;  BranchOpc = X86::JBE_4; break;
      case CmpInst::FCMP_ULT: SwapArgs = false; BranchOpc = X86::JB_4;  break;
      case CmpInst::FCMP_ULE: SwapArgs = false; BranchOpc = X86::JBE_4; break;

      case CmpInst::ICMP_EQ:  SwapArgs = false; BranchOpc = X86::JE_4;  break;
      case CmpInst::ICMP_NE:  SwapArgs = false; BranchOpc = X86::JNE_4; break;
      case CmpInst::ICMP_UGT: SwapArgs = false; BranchOpc = X86::JA_4;  break;
      case CmpInst::ICMP_UGE: SwapArgs = false; BranchOpc = X86::JAE_4; break;
      case CmpInst::ICMP_ULT: SwapArgs = false; BranchOpc = X86::JB_4;  break;
      case CmpInst::ICMP_ULE: SwapArgs = false; BranchOpc = X86::JBE_4; break;
      case CmpInst::ICMP_SGT: SwapArgs = false; BranchOpc = X86::JG_4;  break;
      case CmpInst::ICMP_SGE: SwapArgs = false; BranchOpc = X86::JGE_4; break;
      case CmpInst::ICMP_SLT: SwapArgs = false; BranchOpc = X86::JL_4;  break;
      case CmpInst::ICMP_SLE: SwapArgs = false; BranchOpc = X86::JLE_4; break;
      default:
        return false;
      }

      const Value *Op0 = CI->getOperand(0), *Op1 = CI->getOperand(1);
      if (SwapArgs)
        std::swap(Op0, Op1);

      // Emit a compare of the LHS and RHS, setting the flags.
      if (!X86FastEmitCompare(Op0, Op1, VT))
        return false;

      BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, TII.get(BranchOpc))
        .addMBB(TrueMBB);

      if (Predicate == CmpInst::FCMP_UNE) {
        // X86 requires a second branch to handle UNE (and OEQ,
        // which is mapped to UNE above).
        BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, TII.get(X86::JP_4))
          .addMBB(TrueMBB);
      }

      FastEmitBranch(FalseMBB, DL);
      FuncInfo.MBB->addSuccessor(TrueMBB);
      return true;
    }
  } else if (TruncInst *TI = dyn_cast<TruncInst>(BI->getCondition())) {
    // Handle things like "%cond = trunc i32 %X to i1 / br i1 %cond", which
    // typically happen for _Bool and C++ bools.
    MVT SourceVT;
    if (TI->hasOneUse() && TI->getParent() == I->getParent() &&
        isTypeLegal(TI->getOperand(0)->getType(), SourceVT)) {
      unsigned TestOpc = 0;
      switch (SourceVT.SimpleTy) {
      default: break;
      case MVT::i8:  TestOpc = X86::TEST8ri; break;
      case MVT::i16: TestOpc = X86::TEST16ri; break;
      case MVT::i32: TestOpc = X86::TEST32ri; break;
      case MVT::i64: TestOpc = X86::TEST64ri32; break;
      }
      if (TestOpc) {
        unsigned OpReg = getRegForValue(TI->getOperand(0));
        if (OpReg == 0) return false;
        BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, TII.get(TestOpc))
          .addReg(OpReg).addImm(1);

        unsigned JmpOpc = X86::JNE_4;
        if (FuncInfo.MBB->isLayoutSuccessor(TrueMBB)) {
          std::swap(TrueMBB, FalseMBB);
          JmpOpc = X86::JE_4;
        }

        BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, TII.get(JmpOpc))
          .addMBB(TrueMBB);
        FastEmitBranch(FalseMBB, DL);
        FuncInfo.MBB->addSuccessor(TrueMBB);
        return true;
      }
    }
  }

  // Otherwise do a clumsy setcc and re-test it.
  // Note that i1 essentially gets ANY_EXTEND'ed to i8 where it isn't used
  // in an explicit cast, so make sure to handle that correctly.
  unsigned OpReg = getRegForValue(BI->getCondition());
  if (OpReg == 0) return false;

  BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, TII.get(X86::TEST8ri))
    .addReg(OpReg).addImm(1);
  BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, TII.get(X86::JNE_4))
    .addMBB(TrueMBB);
  FastEmitBranch(FalseMBB, DL);
  FuncInfo.MBB->addSuccessor(TrueMBB);
  return true;
}

bool X86FastISel::X86SelectShift(const Instruction *I) {
  unsigned CReg = 0, OpReg = 0;
  const TargetRegisterClass *RC = NULL;
  if (I->getType()->isIntegerTy(8)) {
    CReg = X86::CL;
    RC = &X86::GR8RegClass;
    switch (I->getOpcode()) {
    case Instruction::LShr: OpReg = X86::SHR8rCL; break;
    case Instruction::AShr: OpReg = X86::SAR8rCL; break;
    case Instruction::Shl:  OpReg = X86::SHL8rCL; break;
    default: return false;
    }
  } else if (I->getType()->isIntegerTy(16)) {
    CReg = X86::CX;
    RC = &X86::GR16RegClass;
    switch (I->getOpcode()) {
    case Instruction::LShr: OpReg = X86::SHR16rCL; break;
    case Instruction::AShr: OpReg = X86::SAR16rCL; break;
    case Instruction::Shl:  OpReg = X86::SHL16rCL; break;
    default: return false;
    }
  } else if (I->getType()->isIntegerTy(32)) {
    CReg = X86::ECX;
    RC = &X86::GR32RegClass;
    switch (I->getOpcode()) {
    case Instruction::LShr: OpReg = X86::SHR32rCL; break;
    case Instruction::AShr: OpReg = X86::SAR32rCL; break;
    case Instruction::Shl:  OpReg = X86::SHL32rCL; break;
    default: return false;
    }
  } else if (I->getType()->isIntegerTy(64)) {
    CReg = X86::RCX;
    RC = &X86::GR64RegClass;
    switch (I->getOpcode()) {
    case Instruction::LShr: OpReg = X86::SHR64rCL; break;
    case Instruction::AShr: OpReg = X86::SAR64rCL; break;
    case Instruction::Shl:  OpReg = X86::SHL64rCL; break;
    default: return false;
    }
  } else {
    return false;
  }

  MVT VT;
  if (!isTypeLegal(I->getType(), VT))
    return false;

  unsigned Op0Reg = getRegForValue(I->getOperand(0));
  if (Op0Reg == 0) return false;

  unsigned Op1Reg = getRegForValue(I->getOperand(1));
  if (Op1Reg == 0) return false;
  BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, TII.get(TargetOpcode::COPY),
          CReg).addReg(Op1Reg);

  // The shift instruction uses X86::CL. If we defined a super-register
  // of X86::CL, emit a subreg KILL to precisely describe what we're doing here.
  if (CReg != X86::CL)
    BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL,
            TII.get(TargetOpcode::KILL), X86::CL)
      .addReg(CReg, RegState::Kill);

  unsigned ResultReg = createResultReg(RC);
  BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, TII.get(OpReg), ResultReg)
    .addReg(Op0Reg);
  UpdateValueMap(I, ResultReg);
  return true;
}

bool X86FastISel::X86SelectSelect(const Instruction *I) {
  MVT VT;
  if (!isTypeLegal(I->getType(), VT))
    return false;

  // We only use cmov here, if we don't have a cmov instruction bail.
  if (!Subtarget->hasCMov()) return false;

  unsigned Opc = 0;
  const TargetRegisterClass *RC = NULL;
  if (VT == MVT::i16) {
    Opc = X86::CMOVE16rr;
    RC = &X86::GR16RegClass;
  } else if (VT == MVT::i32) {
    Opc = X86::CMOVE32rr;
    RC = &X86::GR32RegClass;
  } else if (VT == MVT::i64) {
    Opc = X86::CMOVE64rr;
    RC = &X86::GR64RegClass;
  } else {
    return false;
  }

  unsigned Op0Reg = getRegForValue(I->getOperand(0));
  if (Op0Reg == 0) return false;
  unsigned Op1Reg = getRegForValue(I->getOperand(1));
  if (Op1Reg == 0) return false;
  unsigned Op2Reg = getRegForValue(I->getOperand(2));
  if (Op2Reg == 0) return false;

  BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, TII.get(X86::TEST8rr))
    .addReg(Op0Reg).addReg(Op0Reg);
  unsigned ResultReg = createResultReg(RC);
  BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, TII.get(Opc), ResultReg)
    .addReg(Op1Reg).addReg(Op2Reg);
  UpdateValueMap(I, ResultReg);
  return true;
}

bool X86FastISel::X86SelectFPExt(const Instruction *I) {
  // fpext from float to double.
  if (X86ScalarSSEf64 &&
      I->getType()->isDoubleTy()) {
    const Value *V = I->getOperand(0);
    if (V->getType()->isFloatTy()) {
      unsigned OpReg = getRegForValue(V);
      if (OpReg == 0) return false;
      unsigned ResultReg = createResultReg(&X86::FR64RegClass);
      BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL,
              TII.get(X86::CVTSS2SDrr), ResultReg)
        .addReg(OpReg);
      UpdateValueMap(I, ResultReg);
      return true;
    }
  }

  return false;
}

bool X86FastISel::X86SelectFPTrunc(const Instruction *I) {
  if (X86ScalarSSEf64) {
    if (I->getType()->isFloatTy()) {
      const Value *V = I->getOperand(0);
      if (V->getType()->isDoubleTy()) {
        unsigned OpReg = getRegForValue(V);
        if (OpReg == 0) return false;
        unsigned ResultReg = createResultReg(&X86::FR32RegClass);
        BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL,
                TII.get(X86::CVTSD2SSrr), ResultReg)
          .addReg(OpReg);
        UpdateValueMap(I, ResultReg);
        return true;
      }
    }
  }

  return false;
}

bool X86FastISel::X86SelectTrunc(const Instruction *I) {
  EVT SrcVT = TLI.getValueType(I->getOperand(0)->getType());
  EVT DstVT = TLI.getValueType(I->getType());

  // This code only handles truncation to byte.
  if (DstVT != MVT::i8 && DstVT != MVT::i1)
    return false;
  if (!TLI.isTypeLegal(SrcVT))
    return false;

  unsigned InputReg = getRegForValue(I->getOperand(0));
  if (!InputReg)
    // Unhandled operand.  Halt "fast" selection and bail.
    return false;

  if (SrcVT == MVT::i8) {
    // Truncate from i8 to i1; no code needed.
    UpdateValueMap(I, InputReg);
    return true;
  }

  if (!Subtarget->is64Bit()) {
    // If we're on x86-32; we can't extract an i8 from a general register.
    // First issue a copy to GR16_ABCD or GR32_ABCD.
    const TargetRegisterClass *CopyRC = (SrcVT == MVT::i16) ?
      (const TargetRegisterClass*)&X86::GR16_ABCDRegClass :
      (const TargetRegisterClass*)&X86::GR32_ABCDRegClass;
    unsigned CopyReg = createResultReg(CopyRC);
    BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, TII.get(TargetOpcode::COPY),
            CopyReg).addReg(InputReg);
    InputReg = CopyReg;
  }

  // Issue an extract_subreg.
  unsigned ResultReg = FastEmitInst_extractsubreg(MVT::i8,
                                                  InputReg, /*Kill=*/true,
                                                  X86::sub_8bit);
  if (!ResultReg)
    return false;

  UpdateValueMap(I, ResultReg);
  return true;
}

bool X86FastISel::IsMemcpySmall(uint64_t Len) {
  return Len <= (Subtarget->is64Bit() ? 32 : 16);
}

bool X86FastISel::TryEmitSmallMemcpy(X86AddressMode DestAM,
                                     X86AddressMode SrcAM, uint64_t Len) {

  // Make sure we don't bloat code by inlining very large memcpy's.
  if (!IsMemcpySmall(Len))
    return false;

  bool i64Legal = Subtarget->is64Bit();

  // We don't care about alignment here since we just emit integer accesses.
  while (Len) {
    MVT VT;
    if (Len >= 8 && i64Legal)
      VT = MVT::i64;
    else if (Len >= 4)
      VT = MVT::i32;
    else if (Len >= 2)
      VT = MVT::i16;
    else {
      VT = MVT::i8;
    }

    unsigned Reg;
    bool RV = X86FastEmitLoad(VT, SrcAM, Reg);
    RV &= X86FastEmitStore(VT, Reg, DestAM);
    assert(RV && "Failed to emit load or store??");

    unsigned Size = VT.getSizeInBits()/8;
    Len -= Size;
    DestAM.Disp += Size;
    SrcAM.Disp += Size;
  }

  return true;
}

bool X86FastISel::X86VisitIntrinsicCall(const IntrinsicInst &I) {
  // FIXME: Handle more intrinsics.
  switch (I.getIntrinsicID()) {
  default: return false;
  case Intrinsic::memcpy: {
    const MemCpyInst &MCI = cast<MemCpyInst>(I);
    // Don't handle volatile or variable length memcpys.
    if (MCI.isVolatile())
      return false;

    if (isa<ConstantInt>(MCI.getLength())) {
      // Small memcpy's are common enough that we want to do them
      // without a call if possible.
      uint64_t Len = cast<ConstantInt>(MCI.getLength())->getZExtValue();
      if (IsMemcpySmall(Len)) {
        X86AddressMode DestAM, SrcAM;
        if (!X86SelectAddress(MCI.getRawDest(), DestAM) ||
            !X86SelectAddress(MCI.getRawSource(), SrcAM))
          return false;
        TryEmitSmallMemcpy(DestAM, SrcAM, Len);
        return true;
      }
    }

    unsigned SizeWidth = Subtarget->is64Bit() ? 64 : 32;
    if (!MCI.getLength()->getType()->isIntegerTy(SizeWidth))
      return false;

    if (MCI.getSourceAddressSpace() > 255 || MCI.getDestAddressSpace() > 255)
      return false;

    return DoSelectCall(&I, "memcpy");
  }
  case Intrinsic::memset: {
    const MemSetInst &MSI = cast<MemSetInst>(I);

    if (MSI.isVolatile())
      return false;

    unsigned SizeWidth = Subtarget->is64Bit() ? 64 : 32;
    if (!MSI.getLength()->getType()->isIntegerTy(SizeWidth))
      return false;

    if (MSI.getDestAddressSpace() > 255)
      return false;

    return DoSelectCall(&I, "memset");
  }
  case Intrinsic::stackprotector: {
    // Emit code to store the stack guard onto the stack.
    EVT PtrTy = TLI.getPointerTy();

    const Value *Op1 = I.getArgOperand(0); // The guard's value.
    const AllocaInst *Slot = cast<AllocaInst>(I.getArgOperand(1));

    // Grab the frame index.
    X86AddressMode AM;
    if (!X86SelectAddress(Slot, AM)) return false;
    if (!X86FastEmitStore(PtrTy, Op1, AM)) return false;
    return true;
  }
  case Intrinsic::dbg_declare: {
    const DbgDeclareInst *DI = cast<DbgDeclareInst>(&I);
    X86AddressMode AM;
    assert(DI->getAddress() && "Null address should be checked earlier!");
    if (!X86SelectAddress(DI->getAddress(), AM))
      return false;
    const MCInstrDesc &II = TII.get(TargetOpcode::DBG_VALUE);
    // FIXME may need to add RegState::Debug to any registers produced,
    // although ESP/EBP should be the only ones at the moment.
    addFullAddress(BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, II), AM).
      addImm(0).addMetadata(DI->getVariable());
    return true;
  }
  case Intrinsic::trap: {
    BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, TII.get(X86::TRAP));
    return true;
  }
  case Intrinsic::sadd_with_overflow:
  case Intrinsic::uadd_with_overflow: {
    // FIXME: Should fold immediates.

    // Replace "add with overflow" intrinsics with an "add" instruction followed
    // by a seto/setc instruction.
    const Function *Callee = I.getCalledFunction();
    Type *RetTy =
      cast<StructType>(Callee->getReturnType())->getTypeAtIndex(unsigned(0));

    MVT VT;
    if (!isTypeLegal(RetTy, VT))
      return false;

    const Value *Op1 = I.getArgOperand(0);
    const Value *Op2 = I.getArgOperand(1);
    unsigned Reg1 = getRegForValue(Op1);
    unsigned Reg2 = getRegForValue(Op2);

    if (Reg1 == 0 || Reg2 == 0)
      // FIXME: Handle values *not* in registers.
      return false;

    unsigned OpC = 0;
    if (VT == MVT::i32)
      OpC = X86::ADD32rr;
    else if (VT == MVT::i64)
      OpC = X86::ADD64rr;
    else
      return false;

    // The call to CreateRegs builds two sequential registers, to store the
    // both the returned values.
    unsigned ResultReg = FuncInfo.CreateRegs(I.getType());
    BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, TII.get(OpC), ResultReg)
      .addReg(Reg1).addReg(Reg2);

    unsigned Opc = X86::SETBr;
    if (I.getIntrinsicID() == Intrinsic::sadd_with_overflow)
      Opc = X86::SETOr;
    BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, TII.get(Opc), ResultReg+1);

    UpdateValueMap(&I, ResultReg, 2);
    return true;
  }
  }
}

bool X86FastISel::FastLowerArguments() {
  if (!FuncInfo.CanLowerReturn)
    return false;

  if (Subtarget->isTargetWin64())
    return false;

  const Function *F = FuncInfo.Fn;
  if (F->isVarArg())
    return false;

  CallingConv::ID CC = F->getCallingConv();
  if (CC != CallingConv::C)
    return false;
  
  if (!Subtarget->is64Bit())
    return false;
  
  // Only handle simple cases. i.e. Up to 6 i32/i64 scalar arguments.
  unsigned Idx = 1;
  for (Function::const_arg_iterator I = F->arg_begin(), E = F->arg_end();
       I != E; ++I, ++Idx) {
    if (Idx > 6)
      return false;

    if (F->getAttributes().hasAttribute(Idx, Attribute::ByVal) ||
        F->getAttributes().hasAttribute(Idx, Attribute::InReg) ||
        F->getAttributes().hasAttribute(Idx, Attribute::StructRet) ||
        F->getAttributes().hasAttribute(Idx, Attribute::Nest))
      return false;

    Type *ArgTy = I->getType();
    if (ArgTy->isStructTy() || ArgTy->isArrayTy() || ArgTy->isVectorTy())
      return false;

    EVT ArgVT = TLI.getValueType(ArgTy);
    if (!ArgVT.isSimple()) return false;
    switch (ArgVT.getSimpleVT().SimpleTy) {
    case MVT::i32:
    case MVT::i64:
      break;
    default:
      return false;
    }
  }

  static const uint16_t GPR32ArgRegs[] = {
    X86::EDI, X86::ESI, X86::EDX, X86::ECX, X86::R8D, X86::R9D
  };
  static const uint16_t GPR64ArgRegs[] = {
    X86::RDI, X86::RSI, X86::RDX, X86::RCX, X86::R8 , X86::R9
  };

  Idx = 0;
  const TargetRegisterClass *RC32 = TLI.getRegClassFor(MVT::i32);
  const TargetRegisterClass *RC64 = TLI.getRegClassFor(MVT::i64);
  for (Function::const_arg_iterator I = F->arg_begin(), E = F->arg_end();
       I != E; ++I, ++Idx) {
    if (I->use_empty())
      continue;
    bool is32Bit = TLI.getValueType(I->getType()) == MVT::i32;
    const TargetRegisterClass *RC = is32Bit ? RC32 : RC64;
    unsigned SrcReg = is32Bit ? GPR32ArgRegs[Idx] : GPR64ArgRegs[Idx];
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

bool X86FastISel::X86SelectCall(const Instruction *I) {
  const CallInst *CI = cast<CallInst>(I);
  const Value *Callee = CI->getCalledValue();

  // Can't handle inline asm yet.
  if (isa<InlineAsm>(Callee))
    return false;

  // Handle intrinsic calls.
  if (const IntrinsicInst *II = dyn_cast<IntrinsicInst>(CI))
    return X86VisitIntrinsicCall(*II);

  // Allow SelectionDAG isel to handle tail calls.
  if (cast<CallInst>(I)->isTailCall())
    return false;

  return DoSelectCall(I, 0);
}

static unsigned computeBytesPoppedByCallee(const X86Subtarget &Subtarget,
                                           const ImmutableCallSite &CS) {
  if (Subtarget.is64Bit())
    return 0;
  if (Subtarget.isTargetWindows())
    return 0;
  CallingConv::ID CC = CS.getCallingConv();
  if (CC == CallingConv::Fast || CC == CallingConv::GHC)
    return 0;
  if (!CS.paramHasAttr(1, Attribute::StructRet))
    return 0;
  if (CS.paramHasAttr(1, Attribute::InReg))
    return 0;
  return 4;
}

// Select either a call, or an llvm.memcpy/memmove/memset intrinsic
bool X86FastISel::DoSelectCall(const Instruction *I, const char *MemIntName) {
  const CallInst *CI = cast<CallInst>(I);
  const Value *Callee = CI->getCalledValue();

  // Handle only C and fastcc calling conventions for now.
  ImmutableCallSite CS(CI);
  CallingConv::ID CC = CS.getCallingConv();
  if (CC != CallingConv::C && CC != CallingConv::Fast &&
      CC != CallingConv::X86_FastCall)
    return false;

  // fastcc with -tailcallopt is intended to provide a guaranteed
  // tail call optimization. Fastisel doesn't know how to do that.
  if (CC == CallingConv::Fast && TM.Options.GuaranteedTailCallOpt)
    return false;

  PointerType *PT = cast<PointerType>(CS.getCalledValue()->getType());
  FunctionType *FTy = cast<FunctionType>(PT->getElementType());
  bool isVarArg = FTy->isVarArg();

  // Don't know how to handle Win64 varargs yet.  Nothing special needed for
  // x86-32.  Special handling for x86-64 is implemented.
  if (isVarArg && Subtarget->isTargetWin64())
    return false;

  // Fast-isel doesn't know about callee-pop yet.
  if (X86::isCalleePop(CC, Subtarget->is64Bit(), isVarArg,
                       TM.Options.GuaranteedTailCallOpt))
    return false;

  // Check whether the function can return without sret-demotion.
  SmallVector<ISD::OutputArg, 4> Outs;
  GetReturnInfo(I->getType(), CS.getAttributes(), Outs, TLI);
  bool CanLowerReturn = TLI.CanLowerReturn(CS.getCallingConv(),
                                           *FuncInfo.MF, FTy->isVarArg(),
                                           Outs, FTy->getContext());
  if (!CanLowerReturn)
    return false;

  // Materialize callee address in a register. FIXME: GV address can be
  // handled with a CALLpcrel32 instead.
  X86AddressMode CalleeAM;
  if (!X86SelectCallAddress(Callee, CalleeAM))
    return false;
  unsigned CalleeOp = 0;
  const GlobalValue *GV = 0;
  if (CalleeAM.GV != 0) {
    GV = CalleeAM.GV;
  } else if (CalleeAM.Base.Reg != 0) {
    CalleeOp = CalleeAM.Base.Reg;
  } else
    return false;

  // Deal with call operands first.
  SmallVector<const Value *, 8> ArgVals;
  SmallVector<unsigned, 8> Args;
  SmallVector<MVT, 8> ArgVTs;
  SmallVector<ISD::ArgFlagsTy, 8> ArgFlags;
  unsigned arg_size = CS.arg_size();
  Args.reserve(arg_size);
  ArgVals.reserve(arg_size);
  ArgVTs.reserve(arg_size);
  ArgFlags.reserve(arg_size);
  for (ImmutableCallSite::arg_iterator i = CS.arg_begin(), e = CS.arg_end();
       i != e; ++i) {
    // If we're lowering a mem intrinsic instead of a regular call, skip the
    // last two arguments, which should not passed to the underlying functions.
    if (MemIntName && e-i <= 2)
      break;
    Value *ArgVal = *i;
    ISD::ArgFlagsTy Flags;
    unsigned AttrInd = i - CS.arg_begin() + 1;
    if (CS.paramHasAttr(AttrInd, Attribute::SExt))
      Flags.setSExt();
    if (CS.paramHasAttr(AttrInd, Attribute::ZExt))
      Flags.setZExt();

    if (CS.paramHasAttr(AttrInd, Attribute::ByVal)) {
      PointerType *Ty = cast<PointerType>(ArgVal->getType());
      Type *ElementTy = Ty->getElementType();
      unsigned FrameSize = TD.getTypeAllocSize(ElementTy);
      unsigned FrameAlign = CS.getParamAlignment(AttrInd);
      if (!FrameAlign)
        FrameAlign = TLI.getByValTypeAlignment(ElementTy);
      Flags.setByVal();
      Flags.setByValSize(FrameSize);
      Flags.setByValAlign(FrameAlign);
      if (!IsMemcpySmall(FrameSize))
        return false;
    }

    if (CS.paramHasAttr(AttrInd, Attribute::InReg))
      Flags.setInReg();
    if (CS.paramHasAttr(AttrInd, Attribute::Nest))
      Flags.setNest();

    // If this is an i1/i8/i16 argument, promote to i32 to avoid an extra
    // instruction.  This is safe because it is common to all fastisel supported
    // calling conventions on x86.
    if (ConstantInt *CI = dyn_cast<ConstantInt>(ArgVal)) {
      if (CI->getBitWidth() == 1 || CI->getBitWidth() == 8 ||
          CI->getBitWidth() == 16) {
        if (Flags.isSExt())
          ArgVal = ConstantExpr::getSExt(CI,Type::getInt32Ty(CI->getContext()));
        else
          ArgVal = ConstantExpr::getZExt(CI,Type::getInt32Ty(CI->getContext()));
      }
    }

    unsigned ArgReg;

    // Passing bools around ends up doing a trunc to i1 and passing it.
    // Codegen this as an argument + "and 1".
    if (ArgVal->getType()->isIntegerTy(1) && isa<TruncInst>(ArgVal) &&
        cast<TruncInst>(ArgVal)->getParent() == I->getParent() &&
        ArgVal->hasOneUse()) {
      ArgVal = cast<TruncInst>(ArgVal)->getOperand(0);
      ArgReg = getRegForValue(ArgVal);
      if (ArgReg == 0) return false;

      MVT ArgVT;
      if (!isTypeLegal(ArgVal->getType(), ArgVT)) return false;

      ArgReg = FastEmit_ri(ArgVT, ArgVT, ISD::AND, ArgReg,
                           ArgVal->hasOneUse(), 1);
    } else {
      ArgReg = getRegForValue(ArgVal);
    }

    if (ArgReg == 0) return false;

    Type *ArgTy = ArgVal->getType();
    MVT ArgVT;
    if (!isTypeLegal(ArgTy, ArgVT))
      return false;
    if (ArgVT == MVT::x86mmx)
      return false;
    unsigned OriginalAlignment = TD.getABITypeAlignment(ArgTy);
    Flags.setOrigAlign(OriginalAlignment);

    Args.push_back(ArgReg);
    ArgVals.push_back(ArgVal);
    ArgVTs.push_back(ArgVT);
    ArgFlags.push_back(Flags);
  }

  // Analyze operands of the call, assigning locations to each operand.
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CC, isVarArg, *FuncInfo.MF, TM, ArgLocs,
                 I->getParent()->getContext());

  // Allocate shadow area for Win64
  if (Subtarget->isTargetWin64())
    CCInfo.AllocateStack(32, 8);

  CCInfo.AnalyzeCallOperands(ArgVTs, ArgFlags, CC_X86);

  // Get a count of how many bytes are to be pushed on the stack.
  unsigned NumBytes = CCInfo.getNextStackOffset();

  // Issue CALLSEQ_START
  unsigned AdjStackDown = TII.getCallFrameSetupOpcode();
  BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, TII.get(AdjStackDown))
    .addImm(NumBytes);

  // Process argument: walk the register/memloc assignments, inserting
  // copies / loads.
  SmallVector<unsigned, 4> RegArgs;
  for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) {
    CCValAssign &VA = ArgLocs[i];
    unsigned Arg = Args[VA.getValNo()];
    EVT ArgVT = ArgVTs[VA.getValNo()];

    // Promote the value if needed.
    switch (VA.getLocInfo()) {
    case CCValAssign::Full: break;
    case CCValAssign::SExt: {
      assert(VA.getLocVT().isInteger() && !VA.getLocVT().isVector() &&
             "Unexpected extend");
      bool Emitted = X86FastEmitExtend(ISD::SIGN_EXTEND, VA.getLocVT(),
                                       Arg, ArgVT, Arg);
      assert(Emitted && "Failed to emit a sext!"); (void)Emitted;
      ArgVT = VA.getLocVT();
      break;
    }
    case CCValAssign::ZExt: {
      assert(VA.getLocVT().isInteger() && !VA.getLocVT().isVector() &&
             "Unexpected extend");
      bool Emitted = X86FastEmitExtend(ISD::ZERO_EXTEND, VA.getLocVT(),
                                       Arg, ArgVT, Arg);
      assert(Emitted && "Failed to emit a zext!"); (void)Emitted;
      ArgVT = VA.getLocVT();
      break;
    }
    case CCValAssign::AExt: {
      assert(VA.getLocVT().isInteger() && !VA.getLocVT().isVector() &&
             "Unexpected extend");
      bool Emitted = X86FastEmitExtend(ISD::ANY_EXTEND, VA.getLocVT(),
                                       Arg, ArgVT, Arg);
      if (!Emitted)
        Emitted = X86FastEmitExtend(ISD::ZERO_EXTEND, VA.getLocVT(),
                                    Arg, ArgVT, Arg);
      if (!Emitted)
        Emitted = X86FastEmitExtend(ISD::SIGN_EXTEND, VA.getLocVT(),
                                    Arg, ArgVT, Arg);

      assert(Emitted && "Failed to emit a aext!"); (void)Emitted;
      ArgVT = VA.getLocVT();
      break;
    }
    case CCValAssign::BCvt: {
      unsigned BC = FastEmit_r(ArgVT.getSimpleVT(), VA.getLocVT(),
                               ISD::BITCAST, Arg, /*TODO: Kill=*/false);
      assert(BC != 0 && "Failed to emit a bitcast!");
      Arg = BC;
      ArgVT = VA.getLocVT();
      break;
    }
    case CCValAssign::VExt: 
      // VExt has not been implemented, so this should be impossible to reach
      // for now.  However, fallback to Selection DAG isel once implemented.
      return false;
    case CCValAssign::Indirect:
      // FIXME: Indirect doesn't need extending, but fast-isel doesn't fully
      // support this.
      return false;
    }

    if (VA.isRegLoc()) {
      BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, TII.get(TargetOpcode::COPY),
              VA.getLocReg()).addReg(Arg);
      RegArgs.push_back(VA.getLocReg());
    } else {
      unsigned LocMemOffset = VA.getLocMemOffset();
      X86AddressMode AM;
      AM.Base.Reg = RegInfo->getStackRegister();
      AM.Disp = LocMemOffset;
      const Value *ArgVal = ArgVals[VA.getValNo()];
      ISD::ArgFlagsTy Flags = ArgFlags[VA.getValNo()];

      if (Flags.isByVal()) {
        X86AddressMode SrcAM;
        SrcAM.Base.Reg = Arg;
        bool Res = TryEmitSmallMemcpy(AM, SrcAM, Flags.getByValSize());
        assert(Res && "memcpy length already checked!"); (void)Res;
      } else if (isa<ConstantInt>(ArgVal) || isa<ConstantPointerNull>(ArgVal)) {
        // If this is a really simple value, emit this with the Value* version
        // of X86FastEmitStore.  If it isn't simple, we don't want to do this,
        // as it can cause us to reevaluate the argument.
        if (!X86FastEmitStore(ArgVT, ArgVal, AM))
          return false;
      } else {
        if (!X86FastEmitStore(ArgVT, Arg, AM))
          return false;
      }
    }
  }

  // ELF / PIC requires GOT in the EBX register before function calls via PLT
  // GOT pointer.
  if (Subtarget->isPICStyleGOT()) {
    unsigned Base = getInstrInfo()->getGlobalBaseReg(FuncInfo.MF);
    BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, TII.get(TargetOpcode::COPY),
            X86::EBX).addReg(Base);
  }

  if (Subtarget->is64Bit() && isVarArg && !Subtarget->isTargetWin64()) {
    // Count the number of XMM registers allocated.
    static const uint16_t XMMArgRegs[] = {
      X86::XMM0, X86::XMM1, X86::XMM2, X86::XMM3,
      X86::XMM4, X86::XMM5, X86::XMM6, X86::XMM7
    };
    unsigned NumXMMRegs = CCInfo.getFirstUnallocated(XMMArgRegs, 8);
    BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, TII.get(X86::MOV8ri),
            X86::AL).addImm(NumXMMRegs);
  }

  // Issue the call.
  MachineInstrBuilder MIB;
  if (CalleeOp) {
    // Register-indirect call.
    unsigned CallOpc;
    if (Subtarget->is64Bit())
      CallOpc = X86::CALL64r;
    else
      CallOpc = X86::CALL32r;
    MIB = BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, TII.get(CallOpc))
      .addReg(CalleeOp);

  } else {
    // Direct call.
    assert(GV && "Not a direct call");
    unsigned CallOpc;
    if (Subtarget->is64Bit())
      CallOpc = X86::CALL64pcrel32;
    else
      CallOpc = X86::CALLpcrel32;

    // See if we need any target-specific flags on the GV operand.
    unsigned char OpFlags = 0;

    // On ELF targets, in both X86-64 and X86-32 mode, direct calls to
    // external symbols most go through the PLT in PIC mode.  If the symbol
    // has hidden or protected visibility, or if it is static or local, then
    // we don't need to use the PLT - we can directly call it.
    if (Subtarget->isTargetELF() &&
        TM.getRelocationModel() == Reloc::PIC_ &&
        GV->hasDefaultVisibility() && !GV->hasLocalLinkage()) {
      OpFlags = X86II::MO_PLT;
    } else if (Subtarget->isPICStyleStubAny() &&
               (GV->isDeclaration() || GV->isWeakForLinker()) &&
               (!Subtarget->getTargetTriple().isMacOSX() ||
                Subtarget->getTargetTriple().isMacOSXVersionLT(10, 5))) {
      // PC-relative references to external symbols should go through $stub,
      // unless we're building with the leopard linker or later, which
      // automatically synthesizes these stubs.
      OpFlags = X86II::MO_DARWIN_STUB;
    }


    MIB = BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, TII.get(CallOpc));
    if (MemIntName)
      MIB.addExternalSymbol(MemIntName, OpFlags);
    else
      MIB.addGlobalAddress(GV, 0, OpFlags);
  }

  // Add a register mask with the call-preserved registers.
  // Proper defs for return values will be added by setPhysRegsDeadExcept().
  MIB.addRegMask(TRI.getCallPreservedMask(CS.getCallingConv()));

  // Add an implicit use GOT pointer in EBX.
  if (Subtarget->isPICStyleGOT())
    MIB.addReg(X86::EBX, RegState::Implicit);

  if (Subtarget->is64Bit() && isVarArg && !Subtarget->isTargetWin64())
    MIB.addReg(X86::AL, RegState::Implicit);

  // Add implicit physical register uses to the call.
  for (unsigned i = 0, e = RegArgs.size(); i != e; ++i)
    MIB.addReg(RegArgs[i], RegState::Implicit);

  // Issue CALLSEQ_END
  unsigned AdjStackUp = TII.getCallFrameDestroyOpcode();
  const unsigned NumBytesCallee = computeBytesPoppedByCallee(*Subtarget, CS);
  BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, TII.get(AdjStackUp))
    .addImm(NumBytes).addImm(NumBytesCallee);

  // Build info for return calling conv lowering code.
  // FIXME: This is practically a copy-paste from TargetLowering::LowerCallTo.
  SmallVector<ISD::InputArg, 32> Ins;
  SmallVector<EVT, 4> RetTys;
  ComputeValueVTs(TLI, I->getType(), RetTys);
  for (unsigned i = 0, e = RetTys.size(); i != e; ++i) {
    EVT VT = RetTys[i];
    MVT RegisterVT = TLI.getRegisterType(I->getParent()->getContext(), VT);
    unsigned NumRegs = TLI.getNumRegisters(I->getParent()->getContext(), VT);
    for (unsigned j = 0; j != NumRegs; ++j) {
      ISD::InputArg MyFlags;
      MyFlags.VT = RegisterVT;
      MyFlags.Used = !CS.getInstruction()->use_empty();
      if (CS.paramHasAttr(0, Attribute::SExt))
        MyFlags.Flags.setSExt();
      if (CS.paramHasAttr(0, Attribute::ZExt))
        MyFlags.Flags.setZExt();
      if (CS.paramHasAttr(0, Attribute::InReg))
        MyFlags.Flags.setInReg();
      Ins.push_back(MyFlags);
    }
  }

  // Now handle call return values.
  SmallVector<unsigned, 4> UsedRegs;
  SmallVector<CCValAssign, 16> RVLocs;
  CCState CCRetInfo(CC, false, *FuncInfo.MF, TM, RVLocs,
                    I->getParent()->getContext());
  unsigned ResultReg = FuncInfo.CreateRegs(I->getType());
  CCRetInfo.AnalyzeCallResult(Ins, RetCC_X86);
  for (unsigned i = 0; i != RVLocs.size(); ++i) {
    EVT CopyVT = RVLocs[i].getValVT();
    unsigned CopyReg = ResultReg + i;

    // If this is a call to a function that returns an fp value on the x87 fp
    // stack, but where we prefer to use the value in xmm registers, copy it
    // out as F80 and use a truncate to move it from fp stack reg to xmm reg.
    if ((RVLocs[i].getLocReg() == X86::ST0 ||
         RVLocs[i].getLocReg() == X86::ST1)) {
      if (isScalarFPTypeInSSEReg(RVLocs[i].getValVT())) {
        CopyVT = MVT::f80;
        CopyReg = createResultReg(&X86::RFP80RegClass);
      }
      BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, TII.get(X86::FpPOP_RETVAL),
              CopyReg);
    } else {
      BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, TII.get(TargetOpcode::COPY),
              CopyReg).addReg(RVLocs[i].getLocReg());
      UsedRegs.push_back(RVLocs[i].getLocReg());
    }

    if (CopyVT != RVLocs[i].getValVT()) {
      // Round the F80 the right size, which also moves to the appropriate xmm
      // register. This is accomplished by storing the F80 value in memory and
      // then loading it back. Ewww...
      EVT ResVT = RVLocs[i].getValVT();
      unsigned Opc = ResVT == MVT::f32 ? X86::ST_Fp80m32 : X86::ST_Fp80m64;
      unsigned MemSize = ResVT.getSizeInBits()/8;
      int FI = MFI.CreateStackObject(MemSize, MemSize, false);
      addFrameReference(BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL,
                                TII.get(Opc)), FI)
        .addReg(CopyReg);
      Opc = ResVT == MVT::f32 ? X86::MOVSSrm : X86::MOVSDrm;
      addFrameReference(BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL,
                                TII.get(Opc), ResultReg + i), FI);
    }
  }

  if (RVLocs.size())
    UpdateValueMap(I, ResultReg, RVLocs.size());

  // Set all unused physreg defs as dead.
  static_cast<MachineInstr *>(MIB)->setPhysRegsDeadExcept(UsedRegs, TRI);

  return true;
}


bool
X86FastISel::TargetSelectInstruction(const Instruction *I)  {
  switch (I->getOpcode()) {
  default: break;
  case Instruction::Load:
    return X86SelectLoad(I);
  case Instruction::Store:
    return X86SelectStore(I);
  case Instruction::Ret:
    return X86SelectRet(I);
  case Instruction::ICmp:
  case Instruction::FCmp:
    return X86SelectCmp(I);
  case Instruction::ZExt:
    return X86SelectZExt(I);
  case Instruction::Br:
    return X86SelectBranch(I);
  case Instruction::Call:
    return X86SelectCall(I);
  case Instruction::LShr:
  case Instruction::AShr:
  case Instruction::Shl:
    return X86SelectShift(I);
  case Instruction::Select:
    return X86SelectSelect(I);
  case Instruction::Trunc:
    return X86SelectTrunc(I);
  case Instruction::FPExt:
    return X86SelectFPExt(I);
  case Instruction::FPTrunc:
    return X86SelectFPTrunc(I);
  case Instruction::IntToPtr: // Deliberate fall-through.
  case Instruction::PtrToInt: {
    EVT SrcVT = TLI.getValueType(I->getOperand(0)->getType());
    EVT DstVT = TLI.getValueType(I->getType());
    if (DstVT.bitsGT(SrcVT))
      return X86SelectZExt(I);
    if (DstVT.bitsLT(SrcVT))
      return X86SelectTrunc(I);
    unsigned Reg = getRegForValue(I->getOperand(0));
    if (Reg == 0) return false;
    UpdateValueMap(I, Reg);
    return true;
  }
  }

  return false;
}

unsigned X86FastISel::TargetMaterializeConstant(const Constant *C) {
  MVT VT;
  if (!isTypeLegal(C->getType(), VT))
    return 0;

  // Can't handle alternate code models yet.
  if (TM.getCodeModel() != CodeModel::Small)
    return 0;

  // Get opcode and regclass of the output for the given load instruction.
  unsigned Opc = 0;
  const TargetRegisterClass *RC = NULL;
  switch (VT.SimpleTy) {
  default: return 0;
  case MVT::i8:
    Opc = X86::MOV8rm;
    RC  = &X86::GR8RegClass;
    break;
  case MVT::i16:
    Opc = X86::MOV16rm;
    RC  = &X86::GR16RegClass;
    break;
  case MVT::i32:
    Opc = X86::MOV32rm;
    RC  = &X86::GR32RegClass;
    break;
  case MVT::i64:
    // Must be in x86-64 mode.
    Opc = X86::MOV64rm;
    RC  = &X86::GR64RegClass;
    break;
  case MVT::f32:
    if (X86ScalarSSEf32) {
      Opc = Subtarget->hasAVX() ? X86::VMOVSSrm : X86::MOVSSrm;
      RC  = &X86::FR32RegClass;
    } else {
      Opc = X86::LD_Fp32m;
      RC  = &X86::RFP32RegClass;
    }
    break;
  case MVT::f64:
    if (X86ScalarSSEf64) {
      Opc = Subtarget->hasAVX() ? X86::VMOVSDrm : X86::MOVSDrm;
      RC  = &X86::FR64RegClass;
    } else {
      Opc = X86::LD_Fp64m;
      RC  = &X86::RFP64RegClass;
    }
    break;
  case MVT::f80:
    // No f80 support yet.
    return 0;
  }

  // Materialize addresses with LEA instructions.
  if (isa<GlobalValue>(C)) {
    X86AddressMode AM;
    if (X86SelectAddress(C, AM)) {
      // If the expression is just a basereg, then we're done, otherwise we need
      // to emit an LEA.
      if (AM.BaseType == X86AddressMode::RegBase &&
          AM.IndexReg == 0 && AM.Disp == 0 && AM.GV == 0)
        return AM.Base.Reg;

      Opc = TLI.getPointerTy() == MVT::i32 ? X86::LEA32r : X86::LEA64r;
      unsigned ResultReg = createResultReg(RC);
      addFullAddress(BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL,
                             TII.get(Opc), ResultReg), AM);
      return ResultReg;
    }
    return 0;
  }

  // MachineConstantPool wants an explicit alignment.
  unsigned Align = TD.getPrefTypeAlignment(C->getType());
  if (Align == 0) {
    // Alignment of vector types.  FIXME!
    Align = TD.getTypeAllocSize(C->getType());
  }

  // x86-32 PIC requires a PIC base register for constant pools.
  unsigned PICBase = 0;
  unsigned char OpFlag = 0;
  if (Subtarget->isPICStyleStubPIC()) { // Not dynamic-no-pic
    OpFlag = X86II::MO_PIC_BASE_OFFSET;
    PICBase = getInstrInfo()->getGlobalBaseReg(FuncInfo.MF);
  } else if (Subtarget->isPICStyleGOT()) {
    OpFlag = X86II::MO_GOTOFF;
    PICBase = getInstrInfo()->getGlobalBaseReg(FuncInfo.MF);
  } else if (Subtarget->isPICStyleRIPRel() &&
             TM.getCodeModel() == CodeModel::Small) {
    PICBase = X86::RIP;
  }

  // Create the load from the constant pool.
  unsigned MCPOffset = MCP.getConstantPoolIndex(C, Align);
  unsigned ResultReg = createResultReg(RC);
  addConstantPoolReference(BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL,
                                   TII.get(Opc), ResultReg),
                           MCPOffset, PICBase, OpFlag);

  return ResultReg;
}

unsigned X86FastISel::TargetMaterializeAlloca(const AllocaInst *C) {
  // Fail on dynamic allocas. At this point, getRegForValue has already
  // checked its CSE maps, so if we're here trying to handle a dynamic
  // alloca, we're not going to succeed. X86SelectAddress has a
  // check for dynamic allocas, because it's called directly from
  // various places, but TargetMaterializeAlloca also needs a check
  // in order to avoid recursion between getRegForValue,
  // X86SelectAddrss, and TargetMaterializeAlloca.
  if (!FuncInfo.StaticAllocaMap.count(C))
    return 0;

  X86AddressMode AM;
  if (!X86SelectAddress(C, AM))
    return 0;
  unsigned Opc = Subtarget->is64Bit() ? X86::LEA64r : X86::LEA32r;
  const TargetRegisterClass* RC = TLI.getRegClassFor(TLI.getPointerTy());
  unsigned ResultReg = createResultReg(RC);
  addFullAddress(BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL,
                         TII.get(Opc), ResultReg), AM);
  return ResultReg;
}

unsigned X86FastISel::TargetMaterializeFloatZero(const ConstantFP *CF) {
  MVT VT;
  if (!isTypeLegal(CF->getType(), VT))
    return 0;

  // Get opcode and regclass for the given zero.
  unsigned Opc = 0;
  const TargetRegisterClass *RC = NULL;
  switch (VT.SimpleTy) {
  default: return 0;
  case MVT::f32:
    if (X86ScalarSSEf32) {
      Opc = X86::FsFLD0SS;
      RC  = &X86::FR32RegClass;
    } else {
      Opc = X86::LD_Fp032;
      RC  = &X86::RFP32RegClass;
    }
    break;
  case MVT::f64:
    if (X86ScalarSSEf64) {
      Opc = X86::FsFLD0SD;
      RC  = &X86::FR64RegClass;
    } else {
      Opc = X86::LD_Fp064;
      RC  = &X86::RFP64RegClass;
    }
    break;
  case MVT::f80:
    // No f80 support yet.
    return 0;
  }

  unsigned ResultReg = createResultReg(RC);
  BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DL, TII.get(Opc), ResultReg);
  return ResultReg;
}


/// TryToFoldLoad - The specified machine instr operand is a vreg, and that
/// vreg is being provided by the specified load instruction.  If possible,
/// try to fold the load as an operand to the instruction, returning true if
/// possible.
bool X86FastISel::TryToFoldLoad(MachineInstr *MI, unsigned OpNo,
                                const LoadInst *LI) {
  X86AddressMode AM;
  if (!X86SelectAddress(LI->getOperand(0), AM))
    return false;

  const X86InstrInfo &XII = (const X86InstrInfo&)TII;

  unsigned Size = TD.getTypeAllocSize(LI->getType());
  unsigned Alignment = LI->getAlignment();

  SmallVector<MachineOperand, 8> AddrOps;
  AM.getFullAddress(AddrOps);

  MachineInstr *Result =
    XII.foldMemoryOperandImpl(*FuncInfo.MF, MI, OpNo, AddrOps, Size, Alignment);
  if (Result == 0) return false;

  FuncInfo.MBB->insert(FuncInfo.InsertPt, Result);
  MI->eraseFromParent();
  return true;
}


namespace llvm {
  FastISel *X86::createFastISel(FunctionLoweringInfo &funcInfo,
                                const TargetLibraryInfo *libInfo) {
    return new X86FastISel(funcInfo, libInfo);
  }
}