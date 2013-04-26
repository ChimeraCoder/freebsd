
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

#include "clang/Analysis/Analyses/PostOrderCFGView.h"

using namespace clang;

void PostOrderCFGView::anchor() { }

PostOrderCFGView::PostOrderCFGView(const CFG *cfg) {
  Blocks.reserve(cfg->getNumBlockIDs());
  CFGBlockSet BSet(cfg);
    
  for (po_iterator I = po_iterator::begin(cfg, BSet),
                   E = po_iterator::end(cfg, BSet); I != E; ++I) {
    BlockOrder[*I] = Blocks.size() + 1;
    Blocks.push_back(*I);      
  }
}

PostOrderCFGView *PostOrderCFGView::create(AnalysisDeclContext &ctx) {
  const CFG *cfg = ctx.getCFG();
  if (!cfg)
    return 0;
  return new PostOrderCFGView(cfg);
}

const void *PostOrderCFGView::getTag() { static int x; return &x; }

bool PostOrderCFGView::BlockOrderCompare::operator()(const CFGBlock *b1,
                                                     const CFGBlock *b2) const {
  PostOrderCFGView::BlockOrderTy::const_iterator b1It = POV.BlockOrder.find(b1);
  PostOrderCFGView::BlockOrderTy::const_iterator b2It = POV.BlockOrder.find(b2);
    
  unsigned b1V = (b1It == POV.BlockOrder.end()) ? 0 : b1It->second;
  unsigned b2V = (b2It == POV.BlockOrder.end()) ? 0 : b2It->second;
  return b1V > b2V;
}