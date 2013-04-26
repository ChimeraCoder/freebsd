
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

#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchersInternal.h"

namespace clang {
namespace ast_matchers {
namespace internal {

void BoundNodesMap::copyTo(BoundNodesTreeBuilder *Builder) const {
  for (IDToNodeMap::const_iterator It = NodeMap.begin();
       It != NodeMap.end();
       ++It) {
    Builder->setBinding(It->first, It->second);
  }
}

void BoundNodesMap::copyTo(BoundNodesMap *Other) const {
  for (IDToNodeMap::const_iterator I = NodeMap.begin(),
                                   E = NodeMap.end();
       I != E; ++I) {
    Other->NodeMap[I->first] = I->second;
  }
}

BoundNodesTree::BoundNodesTree() {}

BoundNodesTree::BoundNodesTree(
  const BoundNodesMap& Bindings,
  const std::vector<BoundNodesTree> RecursiveBindings)
  : Bindings(Bindings),
    RecursiveBindings(RecursiveBindings) {}

void BoundNodesTree::copyTo(BoundNodesTreeBuilder* Builder) const {
  Bindings.copyTo(Builder);
  for (std::vector<BoundNodesTree>::const_iterator
         I = RecursiveBindings.begin(),
         E = RecursiveBindings.end();
       I != E; ++I) {
    Builder->addMatch(*I);
  }
}

void BoundNodesTree::visitMatches(Visitor* ResultVisitor) {
  BoundNodesMap AggregatedBindings;
  visitMatchesRecursively(ResultVisitor, AggregatedBindings);
}

void BoundNodesTree::
visitMatchesRecursively(Visitor* ResultVisitor,
                        const BoundNodesMap& AggregatedBindings) {
  BoundNodesMap CombinedBindings(AggregatedBindings);
  Bindings.copyTo(&CombinedBindings);
  if (RecursiveBindings.empty()) {
    ResultVisitor->visitMatch(BoundNodes(CombinedBindings));
  } else {
    for (unsigned I = 0; I < RecursiveBindings.size(); ++I) {
      RecursiveBindings[I].visitMatchesRecursively(ResultVisitor,
                                                   CombinedBindings);
    }
  }
}

BoundNodesTreeBuilder::BoundNodesTreeBuilder() {}

void BoundNodesTreeBuilder::addMatch(const BoundNodesTree& Bindings) {
  RecursiveBindings.push_back(Bindings);
}

BoundNodesTree BoundNodesTreeBuilder::build() const {
  return BoundNodesTree(Bindings, RecursiveBindings);
}

} // end namespace internal
} // end namespace ast_matchers
} // end namespace clang