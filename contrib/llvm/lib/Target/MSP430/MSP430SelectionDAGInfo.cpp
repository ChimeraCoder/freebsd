
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

#define DEBUG_TYPE "msp430-selectiondag-info"
#include "MSP430TargetMachine.h"
using namespace llvm;

MSP430SelectionDAGInfo::MSP430SelectionDAGInfo(const MSP430TargetMachine &TM)
  : TargetSelectionDAGInfo(TM) {
}

MSP430SelectionDAGInfo::~MSP430SelectionDAGInfo() {
}