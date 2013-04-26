
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
#include "bootstrap.h"
#include "font.h"
#include "lv1call.h"
#include "ps3.h"

#define FONT_SIZE 14
#define FONT dflt_font_14
#define XMARGIN 40
#define YMARGIN 30
#define BG_COLOR 0x00000000
#define FG_COLOR 0xffffffff

#define FB_SIZE	(16*1024*1024)
uint64_t fb_paddr = 0;
uint32_t *fb_vaddr;

int fb_width, fb_height;
int x, y;

static void ps3cons_probe(struct console *cp);
static int ps3cons_init(int arg);
static void ps3cons_putchar(int c);
static int ps3cons_getchar();
static int ps3cons_poll();

struct console ps3console = {
	"ps3",
	"Playstation 3 Framebuffer",
	0,
	ps3cons_probe,
	ps3cons_init,
	ps3cons_putchar,
	ps3cons_getchar,
	ps3cons_poll,
};

static void
ps3cons_probe(struct console *cp)
{
	/* XXX: Get from HV */
	fb_width = 720;
	fb_height = 480;

	cp->c_flags |= C_PRESENTIN|C_PRESENTOUT;
}

static int
ps3cons_init(int arg)
{
	uint64_t fbhandle, fbcontext;
	int i;

	lv1_gpu_open(0);
	lv1_gpu_context_attribute(0, L1GPU_CONTEXT_ATTRIBUTE_DISPLAY_MODE_SET,
	    0,0,0,0);
	lv1_gpu_context_attribute(0, L1GPU_CONTEXT_ATTRIBUTE_DISPLAY_MODE_SET,
	    0,0,1,0);
	lv1_gpu_context_attribute(0, L1GPU_CONTEXT_ATTRIBUTE_DISPLAY_SYNC,
	    0,L1GPU_DISPLAY_SYNC_VSYNC,0,0);
	lv1_gpu_context_attribute(0, L1GPU_CONTEXT_ATTRIBUTE_DISPLAY_SYNC,
	    1,L1GPU_DISPLAY_SYNC_VSYNC,0,0);
	lv1_gpu_memory_allocate(FB_SIZE, 0, 0, 0, 0, &fbhandle, &fb_paddr);
	lv1_gpu_context_allocate(fbhandle, 0, &fbcontext);

	lv1_gpu_context_attribute(fbcontext,
	    L1GPU_CONTEXT_ATTRIBUTE_DISPLAY_FLIP, 0, 0, 0, 0);
	lv1_gpu_context_attribute(fbcontext,
	    L1GPU_CONTEXT_ATTRIBUTE_DISPLAY_FLIP, 1, 0, 0, 0);

	fb_vaddr = ps3mmu_mapdev(fb_paddr, FB_SIZE);

	x = y = 0;

	/* Blank console */
	for (i = 0; i < fb_width*fb_height; i++)
		fb_vaddr[i] = BG_COLOR;

	return (0);
}

static void
ps3cons_putchar(int c)
{
	uint32_t fg, bg;
	uint32_t *addr;
	int i, j, k;
	u_char *p;

	fg = FG_COLOR;
	bg = BG_COLOR;

	switch (c) {
	case '\0':
		break;
	case '\r':
		x = 0;
		break;
	case '\n':
		y += FONT_SIZE;
		break;
	case '\b':
		x = max(0, x - 8);
		break;
	default:
		/* Wrap long lines */
		if (x + XMARGIN + FONT_SIZE > fb_width - XMARGIN) {
			y += FONT_SIZE;
			x = 0;
		}

		if (y + YMARGIN + FONT_SIZE > fb_height - YMARGIN)
			y = 0;
		
		addr = fb_vaddr + (y + YMARGIN)*fb_width + (x + XMARGIN);
		p = FONT + c*FONT_SIZE;

		for (i = 0; i < FONT_SIZE; i++) {
			for (j = 0, k = 7; j < 8; j++, k--) {
				if ((p[i] & (1 << k)) == 0)
					*(addr + j) = bg;
				else
					*(addr + j) = fg;
			}

			addr += fb_width;
		}

		x += 8;
		break;
	}
}

static int
ps3cons_getchar()
{
	return (-1);
}

static int
ps3cons_poll()
{
	return (0);
}