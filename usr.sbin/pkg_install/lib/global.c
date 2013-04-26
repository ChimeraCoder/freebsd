
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

 * 18 July 1993
 *
 * Semi-convenient place to stick some needed globals.
 *
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "lib.h"

/* These are global for all utils */
Boolean	Quiet		= FALSE;
Boolean	Fake		= FALSE;
Boolean	Force		= FALSE;
int AutoAnswer		= FALSE;
int	Verbose			= 0;		/* Allow multiple levels of verbose. */