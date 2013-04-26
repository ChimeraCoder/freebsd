
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

#include "NVPTXSubtarget.h"
#define GET_SUBTARGETINFO_ENUM
#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "NVPTXGenSubtargetInfo.inc"

using namespace llvm;

// Select Driver Interface
#include "llvm/Support/CommandLine.h"
namespace {
cl::opt<NVPTX::DrvInterface> DriverInterface(
    cl::desc("Choose driver interface:"),
    cl::values(clEnumValN(NVPTX::NVCL, "drvnvcl", "Nvidia OpenCL driver"),
               clEnumValN(NVPTX::CUDA, "drvcuda", "Nvidia CUDA driver"),
               clEnumValN(NVPTX::TEST, "drvtest", "Plain Test"), clEnumValEnd),
    cl::init(NVPTX::NVCL));
}

NVPTXSubtarget::NVPTXSubtarget(const std::string &TT, const std::string &CPU,
                               const std::string &FS, bool is64Bit)
    : NVPTXGenSubtargetInfo(TT, CPU, FS), Is64Bit(is64Bit), PTXVersion(0),
      SmVersion(20) {

  drvInterface = DriverInterface;

  // Provide the default CPU if none
  std::string defCPU = "sm_20";

  ParseSubtargetFeatures((CPU.empty() ? defCPU : CPU), FS);

  // Get the TargetName from the FS if available
  if (FS.empty() && CPU.empty())
    TargetName = defCPU;
  else if (!CPU.empty())
    TargetName = CPU;
  else
    llvm_unreachable("we are not using FeatureStr");

  // We default to PTX 3.1, but we cannot just default to it in the initializer
  // since the attribute parser checks if the given option is >= the default.
  // So if we set ptx31 as the default, the ptx30 attribute would never match.
  // Instead, we use 0 as the default and manually set 31 if the default is
  // used.
  if (PTXVersion == 0) {
    PTXVersion = 31;
  }
}