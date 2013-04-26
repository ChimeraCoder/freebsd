
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

#include <db.h>
#include <mpool.h>

void *__mpool_new__44bsd(MPOOL *, pgno_t *);

void *
__mpool_new__44bsd(MPOOL *mp, pgno_t *pgnoaddr)
{

	return (mpool_new(mp, pgnoaddr, MPOOL_PAGE_NEXT));
}

__sym_compat(mpool_new, __mpool_new_44bsd, FBSD_1.0);