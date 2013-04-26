
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

/******************************* GLOBALS *************************************/

/********************** PRIVATE FUNCTIONS/DATA/DEFINES ***********************/

#define u32 unsigned
#define u16 unsigned short
#define u8  unsigned char

// #include "/usr/src/arm/linux/include/asm/setup.h"
#include <linux/asm/setup.h>
#include "tag_list.h"

#define PAGE_SIZE 	0x1000
#define MEM_SIZE	0x2000000
#define PHYS_OFFSET	0x20000000

/*************************** GLOBAL FUNCTIONS ********************************/

/*
 * .KB_C_FN_DEFINITION_START
 * void InitTagList(char*, void *)
 *  This global function populates a linux-boot style tag list from the
 * string passed in the pointer at the location specified.
 * .KB_C_FN_DEFINITION_END
 */
void InitTagList(char *parms, void *output) {

	char *src, *dst;
	struct tag *tagList = (struct tag*)output;

	tagList->hdr.size  = tag_size(tag_core);
	tagList->hdr.tag   = ATAG_CORE;
	tagList->u.core.flags    = 1;
	tagList->u.core.pagesize = PAGE_SIZE;
	tagList->u.core.rootdev  = 0xff;
	tagList = tag_next(tagList);

	tagList->hdr.size  = tag_size(tag_mem32);
	tagList->hdr.tag   = ATAG_MEM;
	tagList->u.mem.size  = MEM_SIZE;
	tagList->u.mem.start = PHYS_OFFSET;
	tagList = tag_next(tagList);

	tagList->hdr.size  = tag_size(tag_cmdline);
	tagList->hdr.tag   = ATAG_CMDLINE;

	src = parms;
	dst = tagList->u.cmdline.cmdline;
	while (*src) {
		*dst++ = *src++;
	}
	*dst = 0;

	tagList->hdr.size += ((unsigned)(src - parms) + 1) / sizeof(unsigned);
	tagList = tag_next(tagList);

	tagList->hdr.size  = 0;
	tagList->hdr.tag   = ATAG_NONE;
}