
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

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <stand.h>
#include <btxv86.h>
#include <machine/cpufunc.h>
#include "bootstrap.h"
#include "libi386.h"

static int	bios_seconds(void);

/*
 * Return the BIOS time-of-day value.
 *
 * XXX uses undocumented BCD support from libstand.
 */
static int
bios_seconds(void)
{
    int			hr, minute, sec;
    unsigned char	bios_time[6];
    
    v86.ctl = 0;
    v86.addr = 0x1c;		/* int 0x1c, function 0 */
    v86.eax = 0x0000;
    v86.es  = VTOPSEG(bios_time);
    v86.ebx = VTOPOFF(bios_time);
    v86int();

    hr = bcd2bin(bios_time[3]);
    minute = bcd2bin(bios_time[4]);
    sec = bcd2bin(bios_time[5]);
    
    return (hr * 3600 + minute * 60 + sec);
}

/*
 * Return the time in seconds since the beginning of the day.
 */
time_t
time(time_t *t)
{
    static time_t lasttime;
    time_t now;

    now = bios_seconds();

    if (now < lasttime)
	now += 24 * 3600;
    lasttime = now;
    
    if (t != NULL)
	*t = now;
    return(now);
}

/*
 * Use the BIOS Wait function to pause for (period) microseconds.
 *
 * Resolution of this function is variable, but typically around
 * 1ms.
 */
void
delay(int period)
{
    int i;

    period = (period + 500) / 1000;
    for( ; period != 0 ; period--)
	for(i=800;i != 0; i--)
	    outb(0x5f,0);       /* wait 600ns */
}