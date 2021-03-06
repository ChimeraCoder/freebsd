
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
#include "gprof.h"
#include "search_list.h"
#include "source.h"
#include "symtab.h"
#include "cg_arcs.h"
#include "corefile.h"
#include "hist.h"

static Sym indirect_child;

void mips_find_call (Sym *, bfd_vma, bfd_vma);

void
mips_find_call (Sym *parent, bfd_vma p_lowpc, bfd_vma p_highpc)
{
  bfd_vma pc, dest_pc;
  unsigned int op;
  int offset;
  Sym *child;
  static bfd_boolean inited = FALSE;

  if (!inited)
    {
      inited = TRUE;
      sym_init (&indirect_child);
      indirect_child.name = _("<indirect child>");
      indirect_child.cg.prop.fract = 1.0;
      indirect_child.cg.cyc.head = &indirect_child;
    }

  DBG (CALLDEBUG, printf (_("[find_call] %s: 0x%lx to 0x%lx\n"),
			  parent->name, (unsigned long) p_lowpc,
			  (unsigned long) p_highpc));
  for (pc = p_lowpc; pc < p_highpc; pc += 4)
    {
      op = bfd_get_32 (core_bfd, ((unsigned char *)core_text_space
                                 + pc - core_text_sect->vma));
      if ((op & 0xfc000000) == 0x0c000000)
	{
	  /* This is a "jal" instruction.  Check that the destination
	     is the address of a function.  */
	  DBG (CALLDEBUG,
	       printf (_("[find_call] 0x%lx: jal"), (unsigned long) pc));
          offset = (op & 0x03ffffff) << 2;
	  dest_pc = (pc & ~(bfd_vma) 0xfffffff) | offset;
	  if (hist_check_address (dest_pc))
	    {
	      child = sym_lookup (&symtab, dest_pc);
	      DBG (CALLDEBUG,
		   printf (" 0x%lx\t; name=%s, addr=0x%lx",
			   (unsigned long) dest_pc, child->name,
			   (unsigned long) child->addr));
	      if (child->addr == dest_pc)
		{
		  DBG (CALLDEBUG, printf ("\n"));
		  /* a hit:  */
		  arc_add (parent, child, (unsigned long) 0);
		  continue;
		}
	    }
	  /* Something funny going on.  */
	  DBG (CALLDEBUG, printf ("\tbut it's a botch\n"));
	}
      else if ((op & 0xfc00f83f) == 0x0000f809)
	{
	  /* This is a "jalr" instruction (indirect call).  */
	  DBG (CALLDEBUG,
	       printf (_("[find_call] 0x%lx: jalr\n"), (unsigned long) pc));
	  arc_add (parent, &indirect_child, (unsigned long) 0);
	}
    }
}