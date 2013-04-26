
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
#include <sys/socket.h>

#include <security/audit/audit.h>

#include <bsm/audit_record.h>
#include <bsm/audit_socket_type.h>

struct bsm_socket_type {
	u_short	bst_bsm_socket_type;
	int	bst_local_socket_type;
};

#define	ST_NO_LOCAL_MAPPING	-600

static const struct bsm_socket_type bsm_socket_types[] = {
	{ BSM_SOCK_DGRAM, SOCK_DGRAM },
	{ BSM_SOCK_STREAM, SOCK_STREAM },
	{ BSM_SOCK_RAW, SOCK_RAW },
	{ BSM_SOCK_RDM, SOCK_RDM },
	{ BSM_SOCK_SEQPACKET, SOCK_SEQPACKET },
};
static const int bsm_socket_types_count = sizeof(bsm_socket_types) /
	    sizeof(bsm_socket_types[0]);

static const struct bsm_socket_type *
bsm_lookup_local_socket_type(int local_socket_type)
{
	int i;

	for (i = 0; i < bsm_socket_types_count; i++) {
		if (bsm_socket_types[i].bst_local_socket_type ==
		    local_socket_type)
			return (&bsm_socket_types[i]);
	}
	return (NULL);
}

u_short
au_socket_type_to_bsm(int local_socket_type)
{
	const struct bsm_socket_type *bstp;

	bstp = bsm_lookup_local_socket_type(local_socket_type);
	if (bstp == NULL)
		return (BSM_SOCK_UNKNOWN);
	return (bstp->bst_bsm_socket_type);
}

static const struct bsm_socket_type *
bsm_lookup_bsm_socket_type(u_short bsm_socket_type)
{
	int i;

	for (i = 0; i < bsm_socket_types_count; i++) {
		if (bsm_socket_types[i].bst_bsm_socket_type ==
		    bsm_socket_type)
			return (&bsm_socket_types[i]);
	}
	return (NULL);
}

int
au_bsm_to_socket_type(u_short bsm_socket_type, int *local_socket_typep)
{
	const struct bsm_socket_type *bstp;

	bstp = bsm_lookup_bsm_socket_type(bsm_socket_type);
	if (bstp == NULL || bstp->bst_local_socket_type)
		return (-1);
	*local_socket_typep = bstp->bst_local_socket_type;
	return (0);
}