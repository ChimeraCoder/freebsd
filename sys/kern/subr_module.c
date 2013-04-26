
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

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/linker.h>

/*
 * Preloaded module support
 */

vm_offset_t preload_addr_relocate = 0;
caddr_t preload_metadata;

/*
 * Search for the preloaded module (name)
 */
caddr_t
preload_search_by_name(const char *name)
{
    caddr_t	curp;
    uint32_t	*hdr;
    int		next;
    
    if (preload_metadata != NULL) {
	
	curp = preload_metadata;
	for (;;) {
	    hdr = (uint32_t *)curp;
	    if (hdr[0] == 0 && hdr[1] == 0)
		break;

	    /* Search for a MODINFO_NAME field */
	    if ((hdr[0] == MODINFO_NAME) &&
		!strcmp(name, curp + sizeof(uint32_t) * 2))
		return(curp);

	    /* skip to next field */
	    next = sizeof(uint32_t) * 2 + hdr[1];
	    next = roundup(next, sizeof(u_long));
	    curp += next;
	}
    }
    return(NULL);
}

/*
 * Search for the first preloaded module of (type)
 */
caddr_t
preload_search_by_type(const char *type)
{
    caddr_t	curp, lname;
    uint32_t	*hdr;
    int		next;

    if (preload_metadata != NULL) {

	curp = preload_metadata;
	lname = NULL;
	for (;;) {
	    hdr = (uint32_t *)curp;
	    if (hdr[0] == 0 && hdr[1] == 0)
		break;

	    /* remember the start of each record */
	    if (hdr[0] == MODINFO_NAME)
		lname = curp;

	    /* Search for a MODINFO_TYPE field */
	    if ((hdr[0] == MODINFO_TYPE) &&
		!strcmp(type, curp + sizeof(uint32_t) * 2))
		return(lname);

	    /* skip to next field */
	    next = sizeof(uint32_t) * 2 + hdr[1];
	    next = roundup(next, sizeof(u_long));
	    curp += next;
	}
    }
    return(NULL);
}

/*
 * Walk through the preloaded module list
 */
caddr_t
preload_search_next_name(caddr_t base)
{
    caddr_t	curp;
    uint32_t	*hdr;
    int		next;
    
    if (preload_metadata != NULL) {
	
	/* Pick up where we left off last time */
	if (base) {
	    /* skip to next field */
	    curp = base;
	    hdr = (uint32_t *)curp;
	    next = sizeof(uint32_t) * 2 + hdr[1];
	    next = roundup(next, sizeof(u_long));
	    curp += next;
	} else
	    curp = preload_metadata;

	for (;;) {
	    hdr = (uint32_t *)curp;
	    if (hdr[0] == 0 && hdr[1] == 0)
		break;

	    /* Found a new record? */
	    if (hdr[0] == MODINFO_NAME)
		return curp;

	    /* skip to next field */
	    next = sizeof(uint32_t) * 2 + hdr[1];
	    next = roundup(next, sizeof(u_long));
	    curp += next;
	}
    }
    return(NULL);
}

/*
 * Given a preloaded module handle (mod), return a pointer
 * to the data for the attribute (inf).
 */
caddr_t
preload_search_info(caddr_t mod, int inf)
{
    caddr_t	curp;
    uint32_t	*hdr;
    uint32_t	type = 0;
    int		next;

    curp = mod;
    for (;;) {
	hdr = (uint32_t *)curp;
	/* end of module data? */
	if (hdr[0] == 0 && hdr[1] == 0)
	    break;
	/* 
	 * We give up once we've looped back to what we were looking at 
	 * first - this should normally be a MODINFO_NAME field.
	 */
	if (type == 0) {
	    type = hdr[0];
	} else {
	    if (hdr[0] == type)
		break;
	}
	
	/* 
	 * Attribute match? Return pointer to data.
	 * Consumer may safely assume that size value precedes	
	 * data.
	 */
	if (hdr[0] == inf)
	    return(curp + (sizeof(uint32_t) * 2));

	/* skip to next field */
	next = sizeof(uint32_t) * 2 + hdr[1];
	next = roundup(next, sizeof(u_long));
	curp += next;
    }
    return(NULL);
}

/*
 * Delete a preload record by name.
 */
void
preload_delete_name(const char *name)
{
    caddr_t	curp;
    uint32_t	*hdr;
    int		next;
    int		clearing;
    
    if (preload_metadata != NULL) {
	
	clearing = 0;
	curp = preload_metadata;
	for (;;) {
	    hdr = (uint32_t *)curp;
	    if (hdr[0] == 0 && hdr[1] == 0)
		break;

	    /* Search for a MODINFO_NAME field */
	    if (hdr[0] == MODINFO_NAME) {
		if (!strcmp(name, curp + sizeof(uint32_t) * 2))
		    clearing = 1;	/* got it, start clearing */
		else if (clearing)
		    clearing = 0;	/* at next one now.. better stop */
	    }
	    if (clearing)
		hdr[0] = MODINFO_EMPTY;

	    /* skip to next field */
	    next = sizeof(uint32_t) * 2 + hdr[1];
	    next = roundup(next, sizeof(u_long));
	    curp += next;
	}
    }
}

void *
preload_fetch_addr(caddr_t mod)
{
	caddr_t *mdp;

	mdp = (caddr_t *)preload_search_info(mod, MODINFO_ADDR);
	if (mdp == NULL)
		return (NULL);
	return (*mdp + preload_addr_relocate);
}

size_t
preload_fetch_size(caddr_t mod)
{
	size_t *mdp;

	mdp = (size_t *)preload_search_info(mod, MODINFO_SIZE);
	if (mdp == NULL)
		return (0);
	return (*mdp);
}

/* Called from locore on i386.  Convert physical pointers to kvm. Sigh. */
void
preload_bootstrap_relocate(vm_offset_t offset)
{
    caddr_t	curp;
    uint32_t	*hdr;
    vm_offset_t	*ptr;
    int		next;
    
    if (preload_metadata != NULL) {
	
	curp = preload_metadata;
	for (;;) {
	    hdr = (uint32_t *)curp;
	    if (hdr[0] == 0 && hdr[1] == 0)
		break;

	    /* Deal with the ones that we know we have to fix */
	    switch (hdr[0]) {
	    case MODINFO_ADDR:
	    case MODINFO_METADATA|MODINFOMD_SSYM:
	    case MODINFO_METADATA|MODINFOMD_ESYM:
		ptr = (vm_offset_t *)(curp + (sizeof(uint32_t) * 2));
		*ptr += offset;
		break;
	    }
	    /* The rest is beyond us for now */

	    /* skip to next field */
	    next = sizeof(uint32_t) * 2 + hdr[1];
	    next = roundup(next, sizeof(u_long));
	    curp += next;
	}
    }
}