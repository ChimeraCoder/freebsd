
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

#include "llvm/Analysis/Passes.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
using namespace llvm;

// A handy option for disabling TBAA functionality. The same effect can also be
// achieved by stripping the !tbaa tags from IR, but this option is sometimes
// more convenient.
static cl::opt<bool> EnableTBAA("enable-tbaa", cl::init(true));

namespace {
  /// TBAANode - This is a simple wrapper around an MDNode which provides a
  /// higher-level interface by hiding the details of how alias analysis
  /// information is encoded in its operands.
  class TBAANode {
    const MDNode *Node;

  public:
    TBAANode() : Node(0) {}
    explicit TBAANode(const MDNode *N) : Node(N) {}

    /// getNode - Get the MDNode for this TBAANode.
    const MDNode *getNode() const { return Node; }

    /// getParent - Get this TBAANode's Alias tree parent.
    TBAANode getParent() const {
      if (Node->getNumOperands() < 2)
        return TBAANode();
      MDNode *P = dyn_cast_or_null<MDNode>(Node->getOperand(1));
      if (!P)
        return TBAANode();
      // Ok, this node has a valid parent. Return it.
      return TBAANode(P);
    }

    /// TypeIsImmutable - Test if this TBAANode represents a type for objects
    /// which are not modified (by any means) in the context where this
    /// AliasAnalysis is relevant.
    bool TypeIsImmutable() const {
      if (Node->getNumOperands() < 3)
        return false;
      ConstantInt *CI = dyn_cast<ConstantInt>(Node->getOperand(2));
      if (!CI)
        return false;
      return CI->getValue()[0];
    }
  };
}

namespace {
  /// TypeBasedAliasAnalysis - This is a simple alias analysis
  /// implementation that uses TypeBased to answer queries.
  class TypeBasedAliasAnalysis : public ImmutablePass,
                                 public AliasAnalysis {
  public:
    static char ID; // Class identification, replacement for typeinfo
    TypeBasedAliasAnalysis() : ImmutablePass(ID) {
      initializeTypeBasedAliasAnalysisPass(*PassRegistry::getPassRegistry());
    }

    virtual void initializePass() {
      InitializeAliasAnalysis(this);
    }

    /// getAdjustedAnalysisPointer - This method is used when a pass implements
    /// an analysis interface through multiple inheritance.  If needed, it
    /// should override this to adjust the this pointer as needed for the
    /// specified pass info.
    virtual void *getAdjustedAnalysisPointer(const void *PI) {
      if (PI == &AliasAnalysis::ID)
        return (AliasAnalysis*)this;
      return this;
    }

    bool Aliases(const MDNode *A, const MDNode *B) const;

  private:
    virtual void getAnalysisUsage(AnalysisUsage &AU) const;
    virtual AliasResult alias(const Location &LocA, const Location &LocB);
    virtual bool pointsToConstantMemory(const Location &Loc, bool OrLocal);
    virtual ModRefBehavior getModRefBehavior(ImmutableCallSite CS);
    virtual ModRefBehavior getModRefBehavior(const Function *F);
    virtual ModRefResult getModRefInfo(ImmutableCallSite CS,
                                       const Location &Loc);
    virtual ModRefResult getModRefInfo(ImmutableCallSite CS1,
                                       ImmutableCallSite CS2);
  };
}  // End of anonymous namespace

// Register this pass...
char TypeBasedAliasAnalysis::ID = 0;
INITIALIZE_AG_PASS(TypeBasedAliasAnalysis, AliasAnalysis, "tbaa",
                   "Type-Based Alias Analysis", false, true, false)

ImmutablePass *llvm::createTypeBasedAliasAnalysisPass() {
  return new TypeBasedAliasAnalysis();
}

void
TypeBasedAliasAnalysis::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AliasAnalysis::getAnalysisUsage(AU);
}

/// Aliases - Test whether the type represented by A may alias the
/// type represented by B.
bool
TypeBasedAliasAnalysis::Aliases(const MDNode *A,
                                const MDNode *B) const {
  // Keep track of the root node for A and B.
  TBAANode RootA, RootB;

  // Climb the tree from A to see if we reach B.
  for (TBAANode T(A); ; ) {
    if (T.getNode() == B)
      // B is an ancestor of A.
      return true;

    RootA = T;
    T = T.getParent();
    if (!T.getNode())
      break;
  }

  // Climb the tree from B to see if we reach A.
  for (TBAANode T(B); ; ) {
    if (T.getNode() == A)
      // A is an ancestor of B.
      return true;

    RootB = T;
    T = T.getParent();
    if (!T.getNode())
      break;
  }

  // Neither node is an ancestor of the other.
  
  // If they have different roots, they're part of different potentially
  // unrelated type systems, so we must be conservative.
  if (RootA.getNode() != RootB.getNode())
    return true;

  // If they have the same root, then we've proved there's no alias.
  return false;
}

AliasAnalysis::AliasResult
TypeBasedAliasAnalysis::alias(const Location &LocA,
                              const Location &LocB) {
  if (!EnableTBAA)
    return AliasAnalysis::alias(LocA, LocB);

  // Get the attached MDNodes. If either value lacks a tbaa MDNode, we must
  // be conservative.
  const MDNode *AM = LocA.TBAATag;
  if (!AM) return AliasAnalysis::alias(LocA, LocB);
  const MDNode *BM = LocB.TBAATag;
  if (!BM) return AliasAnalysis::alias(LocA, LocB);

  // If they may alias, chain to the next AliasAnalysis.
  if (Aliases(AM, BM))
    return AliasAnalysis::alias(LocA, LocB);

  // Otherwise return a definitive result.
  return NoAlias;
}

bool TypeBasedAliasAnalysis::pointsToConstantMemory(const Location &Loc,
                                                    bool OrLocal) {
  if (!EnableTBAA)
    return AliasAnalysis::pointsToConstantMemory(Loc, OrLocal);

  const MDNode *M = Loc.TBAATag;
  if (!M) return AliasAnalysis::pointsToConstantMemory(Loc, OrLocal);

  // If this is an "immutable" type, we can assume the pointer is pointing
  // to constant memory.
  if (TBAANode(M).TypeIsImmutable())
    return true;

  return AliasAnalysis::pointsToConstantMemory(Loc, OrLocal);
}

AliasAnalysis::ModRefBehavior
TypeBasedAliasAnalysis::getModRefBehavior(ImmutableCallSite CS) {
  if (!EnableTBAA)
    return AliasAnalysis::getModRefBehavior(CS);

  ModRefBehavior Min = UnknownModRefBehavior;

  // If this is an "immutable" type, we can assume the call doesn't write
  // to memory.
  if (const MDNode *M = CS.getInstruction()->getMetadata(LLVMContext::MD_tbaa))
    if (TBAANode(M).TypeIsImmutable())
      Min = OnlyReadsMemory;

  return ModRefBehavior(AliasAnalysis::getModRefBehavior(CS) & Min);
}

AliasAnalysis::ModRefBehavior
TypeBasedAliasAnalysis::getModRefBehavior(const Function *F) {
  // Functions don't have metadata. Just chain to the next implementation.
  return AliasAnalysis::getModRefBehavior(F);
}

AliasAnalysis::ModRefResult
TypeBasedAliasAnalysis::getModRefInfo(ImmutableCallSite CS,
                                      const Location &Loc) {
  if (!EnableTBAA)
    return AliasAnalysis::getModRefInfo(CS, Loc);

  if (const MDNode *L = Loc.TBAATag)
    if (const MDNode *M =
          CS.getInstruction()->getMetadata(LLVMContext::MD_tbaa))
      if (!Aliases(L, M))
        return NoModRef;

  return AliasAnalysis::getModRefInfo(CS, Loc);
}

AliasAnalysis::ModRefResult
TypeBasedAliasAnalysis::getModRefInfo(ImmutableCallSite CS1,
                                      ImmutableCallSite CS2) {
  if (!EnableTBAA)
    return AliasAnalysis::getModRefInfo(CS1, CS2);

  if (const MDNode *M1 =
        CS1.getInstruction()->getMetadata(LLVMContext::MD_tbaa))
    if (const MDNode *M2 =
          CS2.getInstruction()->getMetadata(LLVMContext::MD_tbaa))
      if (!Aliases(M1, M2))
        return NoModRef;

  return AliasAnalysis::getModRefInfo(CS1, CS2);
}