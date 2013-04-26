
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

#include <string.h>
#include <uuid.h>

/* A macro used to improve the readability of uuid_compare(). */
#define DIFF_RETURN(a, b, field)	do {			\
	if ((a)->field != (b)->field)				\
		return (((a)->field < (b)->field) ? -1 : 1);	\
} while (0)

/*
 * uuid_compare() - compare two UUIDs.
 * See also:
 *	http://www.opengroup.org/onlinepubs/009629399/uuid_compare.htm
 *
 * NOTE: Either UUID can be NULL, meaning a nil UUID. nil UUIDs are smaller
 *	 than any non-nil UUID.
 */
int32_t
uuid_compare(const uuid_t *a, const uuid_t *b, uint32_t *status)
{
	int	res;

	if (status != NULL)
		*status = uuid_s_ok;

	/* Deal with NULL or equal pointers. */
	if (a == b)
		return (0);
	if (a == NULL)
		return ((uuid_is_nil(b, NULL)) ? 0 : -1);
	if (b == NULL)
		return ((uuid_is_nil(a, NULL)) ? 0 : 1);

	/* We have to compare the hard way. */
	DIFF_RETURN(a, b, time_low);
	DIFF_RETURN(a, b, time_mid);
	DIFF_RETURN(a, b, time_hi_and_version);
	DIFF_RETURN(a, b, clock_seq_hi_and_reserved);
	DIFF_RETURN(a, b, clock_seq_low);

	res = memcmp(a->node, b->node, sizeof(a->node));
	if (res)
		return ((res < 0) ? -1 : 1);
	return (0);
}

#undef DIFF_RETURN