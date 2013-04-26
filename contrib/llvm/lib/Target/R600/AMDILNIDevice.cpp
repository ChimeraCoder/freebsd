
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
#include "AMDILNIDevice.h"
#include "AMDGPUSubtarget.h"
#include "AMDILEvergreenDevice.h"

using namespace llvm;

AMDGPUNIDevice::AMDGPUNIDevice(AMDGPUSubtarget *ST)
  : AMDGPUEvergreenDevice(ST) {
  std::string name = ST->getDeviceName();
  if (name == "caicos") {
    DeviceFlag = OCL_DEVICE_CAICOS;
  } else if (name == "turks") {
    DeviceFlag = OCL_DEVICE_TURKS;
  } else if (name == "cayman") {
    DeviceFlag = OCL_DEVICE_CAYMAN;
  } else {
    DeviceFlag = OCL_DEVICE_BARTS;
  }
}
AMDGPUNIDevice::~AMDGPUNIDevice() {
}

size_t
AMDGPUNIDevice::getMaxLDSSize() const {
  if (usesHardware(AMDGPUDeviceInfo::LocalMem)) {
    return MAX_LDS_SIZE_900;
  } else {
    return 0;
  }
}

uint32_t
AMDGPUNIDevice::getGeneration() const {
  return AMDGPUDeviceInfo::HD6XXX;
}


AMDGPUCaymanDevice::AMDGPUCaymanDevice(AMDGPUSubtarget *ST)
  : AMDGPUNIDevice(ST) {
  setCaps();
}

AMDGPUCaymanDevice::~AMDGPUCaymanDevice() {
}

void
AMDGPUCaymanDevice::setCaps() {
  if (mSTM->isOverride(AMDGPUDeviceInfo::DoubleOps)) {
    mHWBits.set(AMDGPUDeviceInfo::DoubleOps);
    mHWBits.set(AMDGPUDeviceInfo::FMA);
  }
  mHWBits.set(AMDGPUDeviceInfo::Signed24BitOps);
  mSWBits.reset(AMDGPUDeviceInfo::Signed24BitOps);
  mSWBits.set(AMDGPUDeviceInfo::ArenaSegment);
}