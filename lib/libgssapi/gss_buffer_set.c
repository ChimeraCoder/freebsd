
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

#include <gssapi/gssapi.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

/* RCSID("$Id: gss_buffer_set.c 18885 2006-10-24 21:53:02Z lha $"); */

OM_uint32 
gss_create_empty_buffer_set(OM_uint32 * minor_status,
    gss_buffer_set_t *buffer_set)
{
	gss_buffer_set_t set;

	set = (gss_buffer_set_desc *) malloc(sizeof(*set));
	if (set == GSS_C_NO_BUFFER_SET) {
		*minor_status = ENOMEM;
		return (GSS_S_FAILURE);
	}

	set->count = 0;
	set->elements = NULL;

	*buffer_set = set;

	*minor_status = 0;
	return (GSS_S_COMPLETE);
}

OM_uint32
gss_add_buffer_set_member(OM_uint32 * minor_status,
    const gss_buffer_t member_buffer, gss_buffer_set_t *buffer_set)
{
	gss_buffer_set_t set;
	gss_buffer_t p;
	OM_uint32 ret;

	if (*buffer_set == GSS_C_NO_BUFFER_SET) {
		ret = gss_create_empty_buffer_set(minor_status,
		    buffer_set);
		if (ret) {
			return (ret);
		}
	}

	set = *buffer_set;
	set->elements = realloc(set->elements,
	    (set->count + 1) * sizeof(set->elements[0]));
	if (set->elements == NULL) {
		*minor_status = ENOMEM;
		return (GSS_S_FAILURE);
	}

	p = &set->elements[set->count];

	p->value = malloc(member_buffer->length);
	if (p->value == NULL) {
		*minor_status = ENOMEM;
		return (GSS_S_FAILURE);
	}
	memcpy(p->value, member_buffer->value, member_buffer->length);
	p->length = member_buffer->length;

	set->count++;

	*minor_status = 0;
	return (GSS_S_COMPLETE);
}

OM_uint32
gss_release_buffer_set(OM_uint32 * minor_status, gss_buffer_set_t *buffer_set)
{
	size_t i;
	OM_uint32 minor;

	*minor_status = 0;

	if (*buffer_set == GSS_C_NO_BUFFER_SET)
		return (GSS_S_COMPLETE);

	for (i = 0; i < (*buffer_set)->count; i++)
		gss_release_buffer(&minor, &((*buffer_set)->elements[i]));

	free((*buffer_set)->elements);

	(*buffer_set)->elements = NULL;
	(*buffer_set)->count = 0;

	free(*buffer_set);
	*buffer_set = GSS_C_NO_BUFFER_SET;

	return (GSS_S_COMPLETE);
}