
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
    
    v86.ctl = 0;
    v86.addr = 0x1a;		/* int 0x1a, function 2 */
    v86.eax = 0x0200;
    v86int();

    hr = bcd2bin((v86.ecx & 0xff00) >> 8);	/* hour in %ch */
    minute = bcd2bin(v86.ecx & 0xff);		/* minute in %cl */
    sec = bcd2bin((v86.edx & 0xff00) >> 8);	/* second in %dh */
    
    return (hr * 3600 + minute * 60 + sec);
}

/*
 * Return the time in seconds since the beginning of the day.
 *
 * Some BIOSes (notably qemu) don't correctly read the RTC
 * registers in an atomic way, sometimes returning bogus values.
 * Therefore we "debounce" the reading by accepting it only when
 * we got 8 identical values in succession.
 *
 * If we pass midnight, don't wrap back to 0.
 */
time_t
time(time_t *t)
{
    static time_t lasttime;
    time_t now, check;
    int same, try;

    same = try = 0;
    check = bios_seconds();
    do {
	now = check;
	check = bios_seconds();
	if (check != now)
	    same = 0;
    } while (++same < 8 && ++try < 1000);

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
    v86.ctl = 0;
    v86.addr = 0x15;		/* int 0x15, function 0x86 */
    v86.eax = 0x8600;
    v86.ecx = period >> 16;
    v86.edx = period & 0xffff;
    v86int();
}