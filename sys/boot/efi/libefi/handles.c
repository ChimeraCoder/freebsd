
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

#include <efi.h>
#include <efilib.h>

struct entry {
	EFI_HANDLE handle;
	struct devsw *dev;
	int unit;
};

struct entry *entry;
int nentries;

int
efi_register_handles(struct devsw *sw, EFI_HANDLE *handles, int count)
{
	size_t sz;
	int idx, unit;

	idx = nentries;
	nentries += count;
	sz = nentries * sizeof(struct entry);
	entry = (entry == NULL) ? malloc(sz) : realloc(entry, sz);
	for (unit = 0; idx < nentries; idx++, unit++) {
		entry[idx].handle = handles[unit];
		entry[idx].dev = sw;
		entry[idx].unit = unit;
	}
	return (0);
}

EFI_HANDLE
efi_find_handle(struct devsw *dev, int unit)
{
	int idx;

	for (idx = 0; idx < nentries; idx++) {
		if (entry[idx].dev != dev)
			continue;
		if (entry[idx].unit != unit)
			continue;
		return (entry[idx].handle);
	}
	return (NULL);
}

int
efi_handle_lookup(EFI_HANDLE h, struct devsw **dev, int *unit)
{
	int idx;

	for (idx = 0; idx < nentries; idx++) {
		if (entry[idx].handle != h)
			continue;
		if (dev != NULL)
			*dev = entry[idx].dev;
		if (unit != NULL)
			*unit = entry[idx].unit;
		return (0);
	}
	return (ENOENT);
}