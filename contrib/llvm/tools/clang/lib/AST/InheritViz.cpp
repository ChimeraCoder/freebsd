
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
#include "clang/AST/Decl.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/TypeOrdering.h"
#include "llvm/Support/GraphWriter.h"
#include "llvm/Support/raw_ostream.h"
#include <map>

using namespace llvm;

namespace clang {

/// InheritanceHierarchyWriter - Helper class that writes out a
/// GraphViz file that diagrams the inheritance hierarchy starting at
/// a given C++ class type. Note that we do not use LLVM's
/// GraphWriter, because the interface does not permit us to properly
/// differentiate between uses of types as virtual bases
/// vs. non-virtual bases.
class InheritanceHierarchyWriter {
  ASTContext& Context;
  raw_ostream &Out;
  std::map<QualType, int, QualTypeOrdering> DirectBaseCount;
  std::set<QualType, QualTypeOrdering> KnownVirtualBases;

public:
  InheritanceHierarchyWriter(ASTContext& Context, raw_ostream& Out)
    : Context(Context), Out(Out) { }

  void WriteGraph(QualType Type) {
    Out << "digraph \"" << DOT::EscapeString(Type.getAsString()) << "\" {\n";
    WriteNode(Type, false);
    Out << "}\n";
  }

protected:
  /// WriteNode - Write out the description of node in the inheritance
  /// diagram, which may be a base class or it may be the root node.
  void WriteNode(QualType Type, bool FromVirtual);

  /// WriteNodeReference - Write out a reference to the given node,
  /// using a unique identifier for each direct base and for the
  /// (only) virtual base.
  raw_ostream& WriteNodeReference(QualType Type, bool FromVirtual);
};

void InheritanceHierarchyWriter::WriteNode(QualType Type, bool FromVirtual) {
  QualType CanonType = Context.getCanonicalType(Type);

  if (FromVirtual) {
    if (KnownVirtualBases.find(CanonType) != KnownVirtualBases.end())
      return;

    // We haven't seen this virtual base before, so display it and
    // its bases.
    KnownVirtualBases.insert(CanonType);
  }

  // Declare the node itself.
  Out << "  ";
  WriteNodeReference(Type, FromVirtual);

  // Give the node a label based on the name of the class.
  std::string TypeName = Type.getAsString();
  Out << " [ shape=\"box\", label=\"" << DOT::EscapeString(TypeName);

  // If the name of the class was a typedef or something different
  // from the "real" class name, show the real class name in
  // parentheses so we don't confuse ourselves.
  if (TypeName != CanonType.getAsString()) {
    Out << "\\n(" << CanonType.getAsString() << ")";
  }

  // Finished describing the node.
  Out << " \"];\n";

  // Display the base classes.
  const CXXRecordDecl *Decl
    = static_cast<const CXXRecordDecl *>(Type->getAs<RecordType>()->getDecl());
  for (CXXRecordDecl::base_class_const_iterator Base = Decl->bases_begin();
       Base != Decl->bases_end(); ++Base) {
    QualType CanonBaseType = Context.getCanonicalType(Base->getType());

    // If this is not virtual inheritance, bump the direct base
    // count for the type.
    if (!Base->isVirtual())
      ++DirectBaseCount[CanonBaseType];

    // Write out the node (if we need to).
    WriteNode(Base->getType(), Base->isVirtual());

    // Write out the edge.
    Out << "  ";
    WriteNodeReference(Type, FromVirtual);
    Out << " -> ";
    WriteNodeReference(Base->getType(), Base->isVirtual());

    // Write out edge attributes to show the kind of inheritance.
    if (Base->isVirtual()) {
      Out << " [ style=\"dashed\" ]";
    }
    Out << ";";
  }
}

/// WriteNodeReference - Write out a reference to the given node,
/// using a unique identifier for each direct base and for the
/// (only) virtual base.
raw_ostream&
InheritanceHierarchyWriter::WriteNodeReference(QualType Type,
                                               bool FromVirtual) {
  QualType CanonType = Context.getCanonicalType(Type);

  Out << "Class_" << CanonType.getAsOpaquePtr();
  if (!FromVirtual)
    Out << "_" << DirectBaseCount[CanonType];
  return Out;
}

/// viewInheritance - Display the inheritance hierarchy of this C++
/// class using GraphViz.
void CXXRecordDecl::viewInheritance(ASTContext& Context) const {
  QualType Self = Context.getTypeDeclType(this);
  std::string ErrMsg;
  sys::Path Filename = sys::Path::GetTemporaryDirectory(&ErrMsg);
  if (Filename.isEmpty()) {
    llvm::errs() << "Error: " << ErrMsg << "\n";
    return;
  }
  Filename.appendComponent(Self.getAsString() + ".dot");
  if (Filename.makeUnique(true,&ErrMsg)) {
    llvm::errs() << "Error: " << ErrMsg << "\n";
    return;
  }

  llvm::errs() << "Writing '" << Filename.c_str() << "'... ";

  llvm::raw_fd_ostream O(Filename.c_str(), ErrMsg);

  if (ErrMsg.empty()) {
    InheritanceHierarchyWriter Writer(Context, O);
    Writer.WriteGraph(Self);
    llvm::errs() << " done. \n";

    O.close();

    // Display the graph
    DisplayGraph(Filename);
  } else {
    llvm::errs() << "error opening file for writing!\n";
  }
}

}