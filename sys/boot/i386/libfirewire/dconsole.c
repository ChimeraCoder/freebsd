
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
#include <bootstrap.h>
#include <sys/param.h>
#include <btxv86.h>
#include <dev/dcons/dcons.h>

void fw_enable(void);
void fw_poll(void);

static void	dconsole_probe(struct console *cp);
static int	dconsole_init(int arg);
static void	dconsole_putchar(int c);
static int	dconsole_getchar(void);
static int	dconsole_ischar(void);

static int	dcons_started = 0;

#define DCONS_BUF_SIZE (64*1024)
static struct dcons_softc sc[DCONS_NPORT];
uint32_t dcons_paddr;

/* The buffer must be allocated in BSS becase:
 *    - The dcons driver in the kernel is initialized before VM/pmap is
 *	initialized, so that the buffer must be allocate in the region
 *	that is mapped at the very early boot state.
 *    - We expect identiy map only for regions before KERNLOAD
 *	(i386:4MB amd64:1MB).
 *    - It seems that heap in conventional memory(640KB) is not sufficent
 *	and we move it to high address as LOADER_SUPPORT_BZIP2.
 *    - BSS is placed in conventional memory.
 */
static char dcons_buffer[DCONS_BUF_SIZE + PAGE_SIZE];

struct console dconsole = {
    "dcons",
    "dumb console port",
    0,
    dconsole_probe,
    dconsole_init,
    dconsole_putchar,
    dconsole_getchar,
    dconsole_ischar
};

#define DCONSOLE_AS_MULTI_CONSOLE	1

static void
dconsole_probe(struct console *cp)
{
    /* XXX check the BIOS equipment list? */
    cp->c_flags |= (C_PRESENTIN | C_PRESENTOUT);
#if DCONSOLE_AS_MULTI_CONSOLE
    dconsole_init(0);
    cp->c_flags |= (C_ACTIVEIN | C_ACTIVEOUT);
#endif
}

static int
dconsole_init(int arg)
{
    char buf[16], *dbuf;
    int size;

    if (dcons_started && arg == 0)
	return 0;
    dcons_started = 1;

    size = DCONS_BUF_SIZE;
    dbuf = (char *)round_page((vm_offset_t)&dcons_buffer[0]);
    dcons_paddr = VTOP(dbuf);
    sprintf(buf, "0x%08x", dcons_paddr);
    setenv("dcons.addr", buf, 1);

    dcons_init((struct dcons_buf *)dbuf, size, sc);
    sprintf(buf, "%d", size);
    setenv("dcons.size", buf, 1);
    fw_enable();
    return(0);
}

static void
dconsole_putchar(int c)
{
    dcons_putc(&sc[0], c);
}

static int
dconsole_getchar(void)
{
    fw_poll();
    return (dcons_checkc(&sc[0]));
}

static int
dconsole_ischar(void)
{
    fw_poll();
    return (dcons_ischar(&sc[0]));
}