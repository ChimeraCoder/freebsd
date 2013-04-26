
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

#include "llvm/Support/TimeValue.h"
#include "llvm/Config/config.h"

namespace llvm {
using namespace sys;

const TimeValue::SecondsType
  TimeValue::PosixZeroTimeSeconds = -946684800;
const TimeValue::SecondsType
  TimeValue::Win32ZeroTimeSeconds = -12591158400ULL;

const TimeValue TimeValue::MinTime       = TimeValue ( INT64_MIN,0 );
const TimeValue TimeValue::MaxTime       = TimeValue ( INT64_MAX,0 );
const TimeValue TimeValue::ZeroTime      = TimeValue ( 0,0 );
const TimeValue TimeValue::PosixZeroTime = TimeValue ( PosixZeroTimeSeconds,0 );
const TimeValue TimeValue::Win32ZeroTime = TimeValue ( Win32ZeroTimeSeconds,0 );

void
TimeValue::normalize( void ) {
  if ( nanos_ >= NANOSECONDS_PER_SECOND ) {
    do {
      seconds_++;
      nanos_ -= NANOSECONDS_PER_SECOND;
    } while ( nanos_ >= NANOSECONDS_PER_SECOND );
  } else if (nanos_ <= -NANOSECONDS_PER_SECOND ) {
    do {
      seconds_--;
      nanos_ += NANOSECONDS_PER_SECOND;
    } while (nanos_ <= -NANOSECONDS_PER_SECOND);
  }

  if (seconds_ >= 1 && nanos_ < 0) {
    seconds_--;
    nanos_ += NANOSECONDS_PER_SECOND;
  } else if (seconds_ < 0 && nanos_ > 0) {
    seconds_++;
    nanos_ -= NANOSECONDS_PER_SECOND;
  }
}

}

/// Include the platform specific portion of TimeValue class
#ifdef LLVM_ON_UNIX
#include "Unix/TimeValue.inc"
#endif
#ifdef LLVM_ON_WIN32
#include "Windows/TimeValue.inc"
#endif