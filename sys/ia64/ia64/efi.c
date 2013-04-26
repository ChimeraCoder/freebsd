
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
#include <machine/bootinfo.h>
#include <machine/efi.h>
#include <machine/md_var.h>
#include <machine/sal.h>
#include <vm/vm.h>
#include <vm/pmap.h>

static struct efi_systbl *efi_systbl;
static struct efi_cfgtbl *efi_cfgtbl;
static struct efi_rt *efi_runtime;

static int efi_status2err[25] = {
	0,		/* EFI_SUCCESS */
	ENOEXEC,	/* EFI_LOAD_ERROR */
	EINVAL,		/* EFI_INVALID_PARAMETER */
	ENOSYS,		/* EFI_UNSUPPORTED */
	EMSGSIZE, 	/* EFI_BAD_BUFFER_SIZE */
	EOVERFLOW,	/* EFI_BUFFER_TOO_SMALL */
	EBUSY,		/* EFI_NOT_READY */
	EIO,		/* EFI_DEVICE_ERROR */
	EROFS,		/* EFI_WRITE_PROTECTED */
	EAGAIN,		/* EFI_OUT_OF_RESOURCES */
	EIO,		/* EFI_VOLUME_CORRUPTED */
	ENOSPC,		/* EFI_VOLUME_FULL */
	ENXIO,		/* EFI_NO_MEDIA */
	ESTALE,		/* EFI_MEDIA_CHANGED */
	ENOENT,		/* EFI_NOT_FOUND */
	EACCES,		/* EFI_ACCESS_DENIED */
	ETIMEDOUT,	/* EFI_NO_RESPONSE */
	EADDRNOTAVAIL,	/* EFI_NO_MAPPING */
	ETIMEDOUT,	/* EFI_TIMEOUT */
	EDOOFUS,	/* EFI_NOT_STARTED */
	EALREADY,	/* EFI_ALREADY_STARTED */
	ECANCELED,	/* EFI_ABORTED */
	EPROTO,		/* EFI_ICMP_ERROR */
	EPROTO,		/* EFI_TFTP_ERROR */
	EPROTO		/* EFI_PROTOCOL_ERROR */
};

static int
efi_status_to_errno(efi_status status)
{
	u_long code;
	int error;

	code = status & 0x3ffffffffffffffful;
	error = (code < 25) ? efi_status2err[code] : EDOOFUS;
	return (error);
}

void
efi_boot_finish(void)
{
}

/*
 * Collect the entry points for PAL and SAL. Be extra careful about NULL
 * pointer values. We're running pre-console, so it's better to return
 * error values than to cause panics, machine checks and other traps and
 * faults. Keep this minimal...
 */
int
efi_boot_minimal(uint64_t systbl)
{
	ia64_efi_f setvirt;
	struct efi_md *md;
	efi_status status;

	if (systbl == 0)
		return (EINVAL);
	efi_systbl = (struct efi_systbl *)IA64_PHYS_TO_RR7(systbl);
	if (efi_systbl->st_hdr.th_sig != EFI_SYSTBL_SIG) {
		efi_systbl = NULL;
		return (EFAULT);
	}
	efi_cfgtbl = (efi_systbl->st_cfgtbl == 0) ? NULL :
	    (struct efi_cfgtbl *)IA64_PHYS_TO_RR7(efi_systbl->st_cfgtbl);
	if (efi_cfgtbl == NULL)
		return (ENOENT);
	efi_runtime = (efi_systbl->st_rt == 0) ? NULL :
	    (struct efi_rt *)IA64_PHYS_TO_RR7(efi_systbl->st_rt);
	if (efi_runtime == NULL)
		return (ENOENT);

	/*
	 * Relocate runtime memory segments for firmware.
	 */
	md = efi_md_first();
	while (md != NULL) {
		if (md->md_attr & EFI_MD_ATTR_RT) {
			md->md_virt = (md->md_attr & EFI_MD_ATTR_WB) ?
			    (void *)IA64_PHYS_TO_RR7(md->md_phys) :
			    (void *)IA64_PHYS_TO_RR6(md->md_phys);
		}
		md = efi_md_next(md);
	}
	setvirt = (void *)IA64_PHYS_TO_RR7((u_long)efi_runtime->rt_setvirtual);
	status = ia64_efi_physical(setvirt, bootinfo->bi_memmap_size,
	    bootinfo->bi_memdesc_size, bootinfo->bi_memdesc_version,
	    ia64_tpa(bootinfo->bi_memmap));
	return ((status < 0) ? EFAULT : 0);
}

void *
efi_get_table(struct uuid *uuid)
{
	struct efi_cfgtbl *ct;
	u_long count;

	if (efi_cfgtbl == NULL)
		return (NULL);
	count = efi_systbl->st_entries;
	ct = efi_cfgtbl;
	while (count--) {
		if (!bcmp(&ct->ct_uuid, uuid, sizeof(*uuid)))
			return ((void *)IA64_PHYS_TO_RR7(ct->ct_data));
		ct++;
	}
	return (NULL);
}

void
efi_get_time(struct efi_tm *tm)
{

	efi_runtime->rt_gettime(tm, NULL);
}

struct efi_md *
efi_md_first(void)
{
	struct efi_md *md;

	if (bootinfo->bi_memmap == 0)
		return (NULL);
	md = (struct efi_md *)bootinfo->bi_memmap;
	return (md);
}

struct efi_md *
efi_md_last(void)
{
	struct efi_md *md;

	if (bootinfo->bi_memmap == 0)
		return (NULL);
	md = (struct efi_md *)(bootinfo->bi_memmap + bootinfo->bi_memmap_size -
	    bootinfo->bi_memdesc_size);
	return (md);
}

struct efi_md *
efi_md_next(struct efi_md *md)
{
	struct efi_md *lim;

	lim = efi_md_last();
	md = (struct efi_md *)((uintptr_t)md + bootinfo->bi_memdesc_size);
	return ((md > lim) ? NULL : md);
}

struct efi_md *
efi_md_prev(struct efi_md *md)
{
	struct efi_md *lim;

	lim = efi_md_first();
	md = (struct efi_md *)((uintptr_t)md - bootinfo->bi_memdesc_size);
	return ((md < lim) ? NULL : md);
}

struct efi_md *
efi_md_find(vm_paddr_t pa)
{
	static struct efi_md *last = NULL;
	struct efi_md *md, *p0, *p1;

	md = (last != NULL) ? last : efi_md_first();
	p1 = p0 = NULL;
	while (md != NULL && md != p1) {
		if (pa >= md->md_phys &&
		    pa < md->md_phys + md->md_pages * EFI_PAGE_SIZE) {
			last = md;
			return (md);
		}

		p1 = p0;
		p0 = md;
		md = (pa < md->md_phys) ? efi_md_prev(md) : efi_md_next(md);
	}

	return (NULL);
}

void
efi_reset_system(void)
{

	if (efi_runtime != NULL)
		efi_runtime->rt_reset(EFI_RESET_WARM, 0, 0, NULL);
	panic("%s: unable to reset the machine", __func__);
}

int
efi_set_time(struct efi_tm *tm)
{

	return (efi_status_to_errno(efi_runtime->rt_settime(tm)));
}

int
efi_var_get(efi_char *name, struct uuid *vendor, uint32_t *attrib,
    size_t *datasize, void *data)
{
	efi_status status;

	status = efi_runtime->rt_getvar(name, vendor, attrib, datasize, data);
	return (efi_status_to_errno(status));
}

int
efi_var_nextname(size_t *namesize, efi_char *name, struct uuid *vendor)
{
	efi_status status;

	status = efi_runtime->rt_scanvar(namesize, name, vendor);
	return (efi_status_to_errno(status));
}
 
int
efi_var_set(efi_char *name, struct uuid *vendor, uint32_t attrib,
    size_t datasize, void *data)
{
	efi_status status;
 
	status = efi_runtime->rt_setvar(name, vendor, attrib, datasize, data);
	return (efi_status_to_errno(status));
}