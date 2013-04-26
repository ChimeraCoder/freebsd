
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

#include <sm/gen.h>
SM_RCSID("@(#)$Id: xtrap.c,v 1.5 2001/09/11 04:04:49 gshapiro Exp $")

#include <sm/xtrap.h>

SM_ATOMIC_UINT_T SmXtrapCount;

SM_DEBUG_T SmXtrapDebug = SM_DEBUG_INITIALIZER("sm_xtrap",
	"@(#)$Debug: sm_xtrap - raise exception at N'th xtrap point $");

SM_DEBUG_T SmXtrapReport = SM_DEBUG_INITIALIZER("sm_xtrap_report",
	"@(#)$Debug: sm_xtrap_report - report xtrap count on exit $");