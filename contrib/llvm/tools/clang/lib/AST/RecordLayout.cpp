
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

#include "clang/AST/ASTContext.h"
#include "clang/AST/RecordLayout.h"
#include "clang/Basic/TargetInfo.h"

using namespace clang;

void ASTRecordLayout::Destroy(ASTContext &Ctx) {
  if (FieldOffsets)
    Ctx.Deallocate(FieldOffsets);
  if (CXXInfo) {
    Ctx.Deallocate(CXXInfo);
    CXXInfo->~CXXRecordLayoutInfo();
  }
  this->~ASTRecordLayout();
  Ctx.Deallocate(this);
}

ASTRecordLayout::ASTRecordLayout(const ASTContext &Ctx, CharUnits size,
                                 CharUnits alignment, CharUnits datasize,
                                 const uint64_t *fieldoffsets,
                                 unsigned fieldcount)
  : Size(size), DataSize(datasize), Alignment(alignment), FieldOffsets(0),
    FieldCount(fieldcount), CXXInfo(0) {
  if (FieldCount > 0)  {
    FieldOffsets = new (Ctx) uint64_t[FieldCount];
    memcpy(FieldOffsets, fieldoffsets, FieldCount * sizeof(*FieldOffsets));
  }
}

// Constructor for C++ records.
ASTRecordLayout::ASTRecordLayout(const ASTContext &Ctx,
                                 CharUnits size, CharUnits alignment,
                                 bool hasOwnVFPtr, CharUnits vbptroffset,
                                 CharUnits datasize,
                                 const uint64_t *fieldoffsets,
                                 unsigned fieldcount,
                                 CharUnits nonvirtualsize,
                                 CharUnits nonvirtualalign,
                                 CharUnits SizeOfLargestEmptySubobject,
                                 const CXXRecordDecl *PrimaryBase,
                                 bool IsPrimaryBaseVirtual,
                                 const BaseOffsetsMapTy& BaseOffsets,
                                 const VBaseOffsetsMapTy& VBaseOffsets)
  : Size(size), DataSize(datasize), Alignment(alignment), FieldOffsets(0),
    FieldCount(fieldcount), CXXInfo(new (Ctx) CXXRecordLayoutInfo)
{
  if (FieldCount > 0)  {
    FieldOffsets = new (Ctx) uint64_t[FieldCount];
    memcpy(FieldOffsets, fieldoffsets, FieldCount * sizeof(*FieldOffsets));
  }

  CXXInfo->PrimaryBase.setPointer(PrimaryBase);
  CXXInfo->PrimaryBase.setInt(IsPrimaryBaseVirtual);
  CXXInfo->NonVirtualSize = nonvirtualsize;
  CXXInfo->NonVirtualAlign = nonvirtualalign;
  CXXInfo->SizeOfLargestEmptySubobject = SizeOfLargestEmptySubobject;
  CXXInfo->BaseOffsets = BaseOffsets;
  CXXInfo->VBaseOffsets = VBaseOffsets;
  CXXInfo->HasOwnVFPtr = hasOwnVFPtr;
  CXXInfo->VBPtrOffset = vbptroffset;

#ifndef NDEBUG
    if (const CXXRecordDecl *PrimaryBase = getPrimaryBase()) {
      if (isPrimaryBaseVirtual()) {
        if (Ctx.getTargetInfo().getCXXABI().hasPrimaryVBases()) {
          assert(getVBaseClassOffset(PrimaryBase).isZero() &&
                 "Primary virtual base must be at offset 0!");
        }
      } else {
        assert(getBaseClassOffset(PrimaryBase).isZero() &&
               "Primary base must be at offset 0!");
      }
    }
#endif        
}