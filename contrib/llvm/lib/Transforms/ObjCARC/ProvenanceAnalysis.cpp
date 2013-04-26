
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

#include "ObjCARC.h"
#include "ProvenanceAnalysis.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallPtrSet.h"

using namespace llvm;
using namespace llvm::objcarc;

bool ProvenanceAnalysis::relatedSelect(const SelectInst *A,
                                       const Value *B) {
  // If the values are Selects with the same condition, we can do a more precise
  // check: just check for relations between the values on corresponding arms.
  if (const SelectInst *SB = dyn_cast<SelectInst>(B))
    if (A->getCondition() == SB->getCondition())
      return related(A->getTrueValue(), SB->getTrueValue()) ||
             related(A->getFalseValue(), SB->getFalseValue());

  // Check both arms of the Select node individually.
  return related(A->getTrueValue(), B) ||
         related(A->getFalseValue(), B);
}

bool ProvenanceAnalysis::relatedPHI(const PHINode *A,
                                    const Value *B) {
  // If the values are PHIs in the same block, we can do a more precise as well
  // as efficient check: just check for relations between the values on
  // corresponding edges.
  if (const PHINode *PNB = dyn_cast<PHINode>(B))
    if (PNB->getParent() == A->getParent()) {
      for (unsigned i = 0, e = A->getNumIncomingValues(); i != e; ++i)
        if (related(A->getIncomingValue(i),
                    PNB->getIncomingValueForBlock(A->getIncomingBlock(i))))
          return true;
      return false;
    }

  // Check each unique source of the PHI node against B.
  SmallPtrSet<const Value *, 4> UniqueSrc;
  for (unsigned i = 0, e = A->getNumIncomingValues(); i != e; ++i) {
    const Value *PV1 = A->getIncomingValue(i);
    if (UniqueSrc.insert(PV1) && related(PV1, B))
      return true;
  }

  // All of the arms checked out.
  return false;
}

/// Test if the value of P, or any value covered by its provenance, is ever
/// stored within the function (not counting callees).
static bool IsStoredObjCPointer(const Value *P) {
  SmallPtrSet<const Value *, 8> Visited;
  SmallVector<const Value *, 8> Worklist;
  Worklist.push_back(P);
  Visited.insert(P);
  do {
    P = Worklist.pop_back_val();
    for (Value::const_use_iterator UI = P->use_begin(), UE = P->use_end();
         UI != UE; ++UI) {
      const User *Ur = *UI;
      if (isa<StoreInst>(Ur)) {
        if (UI.getOperandNo() == 0)
          // The pointer is stored.
          return true;
        // The pointed is stored through.
        continue;
      }
      if (isa<CallInst>(Ur))
        // The pointer is passed as an argument, ignore this.
        continue;
      if (isa<PtrToIntInst>(P))
        // Assume the worst.
        return true;
      if (Visited.insert(Ur))
        Worklist.push_back(Ur);
    }
  } while (!Worklist.empty());

  // Everything checked out.
  return false;
}

bool ProvenanceAnalysis::relatedCheck(const Value *A,
                                      const Value *B) {
  // Skip past provenance pass-throughs.
  A = GetUnderlyingObjCPtr(A);
  B = GetUnderlyingObjCPtr(B);

  // Quick check.
  if (A == B)
    return true;

  // Ask regular AliasAnalysis, for a first approximation.
  switch (AA->alias(A, B)) {
  case AliasAnalysis::NoAlias:
    return false;
  case AliasAnalysis::MustAlias:
  case AliasAnalysis::PartialAlias:
    return true;
  case AliasAnalysis::MayAlias:
    break;
  }

  bool AIsIdentified = IsObjCIdentifiedObject(A);
  bool BIsIdentified = IsObjCIdentifiedObject(B);

  // An ObjC-Identified object can't alias a load if it is never locally stored.
  if (AIsIdentified) {
    // Check for an obvious escape.
    if (isa<LoadInst>(B))
      return IsStoredObjCPointer(A);
    if (BIsIdentified) {
      // Check for an obvious escape.
      if (isa<LoadInst>(A))
        return IsStoredObjCPointer(B);
      // Both pointers are identified and escapes aren't an evident problem.
      return false;
    }
  } else if (BIsIdentified) {
    // Check for an obvious escape.
    if (isa<LoadInst>(A))
      return IsStoredObjCPointer(B);
  }

   // Special handling for PHI and Select.
  if (const PHINode *PN = dyn_cast<PHINode>(A))
    return relatedPHI(PN, B);
  if (const PHINode *PN = dyn_cast<PHINode>(B))
    return relatedPHI(PN, A);
  if (const SelectInst *S = dyn_cast<SelectInst>(A))
    return relatedSelect(S, B);
  if (const SelectInst *S = dyn_cast<SelectInst>(B))
    return relatedSelect(S, A);

  // Conservative.
  return true;
}

bool ProvenanceAnalysis::related(const Value *A,
                                 const Value *B) {
  // Begin by inserting a conservative value into the map. If the insertion
  // fails, we have the answer already. If it succeeds, leave it there until we
  // compute the real answer to guard against recursive queries.
  if (A > B) std::swap(A, B);
  std::pair<CachedResultsTy::iterator, bool> Pair =
    CachedResults.insert(std::make_pair(ValuePairTy(A, B), true));
  if (!Pair.second)
    return Pair.first->second;

  bool Result = relatedCheck(A, B);
  CachedResults[ValuePairTy(A, B)] = Result;
  return Result;
}