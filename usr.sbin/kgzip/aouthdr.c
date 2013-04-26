
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

#include <stddef.h>
#include "aouthdr.h"

#define KGZ_FIX_NSIZE	0	/* Run-time fixup */

const struct kgz_aouthdr0 aouthdr0 = {
    /* a.out header */
    {
	MID_I386 << 020 | OMAGIC,			/* a_midmag */
	0,						/* a_text */
	sizeof(struct kgz_hdr) + KGZ_FIX_NSIZE, 	/* a_data */
	0,						/* a_bss */
	sizeof(struct nlist) * KGZ__STNUM,		/* a_syms */
	0,						/* a_entry */
	0,						/* a_trsize */
	0						/* a_drsize */
    }
};

const struct kgz_aouthdr1 aouthdr1 = {
    /* Symbol table */
    {
	{
	    {
		(char *)offsetof(struct kgz__strtab,
				 kgz)			/* n_un */
	    },
	    N_DATA | N_EXT,				/* n_type */
	    AUX_OBJECT, 				/* n_other */
	    0,						/* n_desc */
	    0						/* n_value */
	},
	{
	    {
		(char *)offsetof(struct kgz__strtab,
				 kgz_ndata)		/* n_un */
	    },
	    N_DATA | N_EXT,				/* n_type */
	    AUX_OBJECT, 				/* n_other */
	    0,						/* n_desc */
	    sizeof(struct kgz_hdr)			/* n_value */
	}
    },
    /* String table */
    {
	sizeof(struct kgz__strtab),			/* length */
	KGZ__STR_KGZ,					/* kgz */
	KGZ__STR_KGZ_NDATA				/* kgz_ndata */
    }
};