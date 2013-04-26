
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
#include "AMDILDevices.h"
#include "AMDGPUSubtarget.h"

using namespace llvm;
namespace llvm {
namespace AMDGPUDeviceInfo {

AMDGPUDevice* getDeviceFromName(const std::string &deviceName,
                                AMDGPUSubtarget *ptr,
                                bool is64bit, bool is64on32bit) {
  if (deviceName.c_str()[2] == '7') {
    switch (deviceName.c_str()[3]) {
    case '1':
      return new AMDGPU710Device(ptr);
    case '7':
      return new AMDGPU770Device(ptr);
    default:
      return new AMDGPU7XXDevice(ptr);
    }
  } else if (deviceName == "cypress") {
#if DEBUG
    assert(!is64bit && "This device does not support 64bit pointers!");
    assert(!is64on32bit && "This device does not support 64bit"
          " on 32bit pointers!");
#endif
    return new AMDGPUCypressDevice(ptr);
  } else if (deviceName == "juniper") {
#if DEBUG
    assert(!is64bit && "This device does not support 64bit pointers!");
    assert(!is64on32bit && "This device does not support 64bit"
          " on 32bit pointers!");
#endif
    return new AMDGPUEvergreenDevice(ptr);
  } else if (deviceName == "redwood") {
#if DEBUG
    assert(!is64bit && "This device does not support 64bit pointers!");
    assert(!is64on32bit && "This device does not support 64bit"
          " on 32bit pointers!");
#endif
    return new AMDGPURedwoodDevice(ptr);
  } else if (deviceName == "cedar") {
#if DEBUG
    assert(!is64bit && "This device does not support 64bit pointers!");
    assert(!is64on32bit && "This device does not support 64bit"
          " on 32bit pointers!");
#endif
    return new AMDGPUCedarDevice(ptr);
  } else if (deviceName == "barts" || deviceName == "turks") {
#if DEBUG
    assert(!is64bit && "This device does not support 64bit pointers!");
    assert(!is64on32bit && "This device does not support 64bit"
          " on 32bit pointers!");
#endif
    return new AMDGPUNIDevice(ptr);
  } else if (deviceName == "cayman") {
#if DEBUG
    assert(!is64bit && "This device does not support 64bit pointers!");
    assert(!is64on32bit && "This device does not support 64bit"
          " on 32bit pointers!");
#endif
    return new AMDGPUCaymanDevice(ptr);
  } else if (deviceName == "caicos") {
#if DEBUG
    assert(!is64bit && "This device does not support 64bit pointers!");
    assert(!is64on32bit && "This device does not support 64bit"
          " on 32bit pointers!");
#endif
    return new AMDGPUNIDevice(ptr);
  } else if (deviceName == "SI") {
    return new AMDGPUSIDevice(ptr);
  } else {
#if DEBUG
    assert(!is64bit && "This device does not support 64bit pointers!");
    assert(!is64on32bit && "This device does not support 64bit"
          " on 32bit pointers!");
#endif
    return new AMDGPU7XXDevice(ptr);
  }
}
} // End namespace AMDGPUDeviceInfo
} // End namespace llvm