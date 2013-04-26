
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

#include <linux/pci.h>
#include "wc.h"

#if defined(__i386__) || defined(__x86_64__)

pgprot_t pgprot_wc(pgprot_t _prot)
{
	return pgprot_writecombine(_prot);
}

int mlx4_wc_enabled(void)
{
	return 1;
}

#elif defined(CONFIG_PPC64)

pgprot_t pgprot_wc(pgprot_t _prot)
{
	return __pgprot((pgprot_val(_prot) | _PAGE_NO_CACHE) &
				     ~(pgprot_t)_PAGE_GUARDED);
}

int mlx4_wc_enabled(void)
{
	return 1;
}

#else	/* !(defined(__i386__) || defined(__x86_64__)) */

pgprot_t pgprot_wc(pgprot_t _prot)
{
	return pgprot_noncached(_prot);
}

int mlx4_wc_enabled(void)
{
	return 0;
}

#endif