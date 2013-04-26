
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

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "log.h"
#include "protocol.h"

/*
 * Initializes the comm_element with any given type of data
 */
void
init_comm_element(struct comm_element *element, enum comm_element_t type)
{

	TRACE_IN(init_comm_element);
	memset(element, 0, sizeof(struct comm_element));

	switch (type) {
	case CET_WRITE_REQUEST:
		init_cache_write_request(&element->c_write_request);
		break;
	case CET_WRITE_RESPONSE:
		init_cache_write_response(&element->c_write_response);
		break;
	case CET_READ_REQUEST:
		init_cache_read_request(&element->c_read_request);
		break;
	case CET_READ_RESPONSE:
		init_cache_read_response(&element->c_read_response);
		break;
	case CET_TRANSFORM_REQUEST:
		init_cache_transform_request(&element->c_transform_request);
		break;
	case CET_TRANSFORM_RESPONSE:
		init_cache_transform_response(&element->c_transform_response);
		break;
	case CET_MP_WRITE_SESSION_REQUEST:
		init_cache_mp_write_session_request(&element->c_mp_ws_request);
		break;
	case CET_MP_WRITE_SESSION_RESPONSE:
		init_cache_mp_write_session_response(&element->c_mp_ws_response);
		break;
	case CET_MP_WRITE_SESSION_WRITE_REQUEST:
		init_cache_mp_write_session_write_request(
			&element->c_mp_ws_write_request);
		break;
	case CET_MP_WRITE_SESSION_WRITE_RESPONSE:
		init_cache_mp_write_session_write_response(
			&element->c_mp_ws_write_response);
		break;
	case CET_MP_READ_SESSION_REQUEST:
		init_cache_mp_read_session_request(&element->c_mp_rs_request);
		break;
	case CET_MP_READ_SESSION_RESPONSE:
		init_cache_mp_read_session_response(&element->c_mp_rs_response);
		break;
	case CET_MP_READ_SESSION_READ_RESPONSE:
		init_cache_mp_read_session_read_response(
			&element->c_mp_rs_read_response);
		break;
	case CET_UNDEFINED:
		break;
	default:
		LOG_ERR_2("init_comm_element", "invalid communication element");
		TRACE_OUT(init_comm_element);
	return;
	}

	element->type = type;
	TRACE_OUT(init_comm_element);
}

void
finalize_comm_element(struct comm_element *element)
{

	TRACE_IN(finalize_comm_element);
	switch (element->type) {
	case CET_WRITE_REQUEST:
		finalize_cache_write_request(&element->c_write_request);
		break;
	case CET_WRITE_RESPONSE:
		finalize_cache_write_response(&element->c_write_response);
		break;
	case CET_READ_REQUEST:
		finalize_cache_read_request(&element->c_read_request);
		break;
	case CET_READ_RESPONSE:
		finalize_cache_read_response(&element->c_read_response);
		break;
	case CET_TRANSFORM_REQUEST:
		finalize_cache_transform_request(&element->c_transform_request);
		break;
	case CET_TRANSFORM_RESPONSE:
		finalize_cache_transform_response(
			&element->c_transform_response);
		break;
	case CET_MP_WRITE_SESSION_REQUEST:
		finalize_cache_mp_write_session_request(
			&element->c_mp_ws_request);
		break;
	case CET_MP_WRITE_SESSION_RESPONSE:
		finalize_cache_mp_write_session_response(
			&element->c_mp_ws_response);
		break;
	case CET_MP_WRITE_SESSION_WRITE_REQUEST:
		finalize_cache_mp_write_session_write_request(
			&element->c_mp_ws_write_request);
		break;
	case CET_MP_WRITE_SESSION_WRITE_RESPONSE:
		finalize_cache_mp_write_session_write_response(
			&element->c_mp_ws_write_response);
		break;
	case CET_MP_READ_SESSION_REQUEST:
		finalize_cache_mp_read_session_request(
			&element->c_mp_rs_request);
		break;
	case CET_MP_READ_SESSION_RESPONSE:
		finalize_cache_mp_read_session_response(
			&element->c_mp_rs_response);
		break;
	case CET_MP_READ_SESSION_READ_RESPONSE:
		finalize_cache_mp_read_session_read_response(
			&element->c_mp_rs_read_response);
		break;
	case CET_UNDEFINED:
		break;
	default:
	break;
	}

	element->type = CET_UNDEFINED;
	TRACE_OUT(finalize_comm_element);
}

void
init_cache_write_request(struct cache_write_request *write_request)
{

	TRACE_IN(init_cache_write_request);
	memset(write_request, 0, sizeof(struct cache_write_request));
	TRACE_OUT(init_cache_write_request);
}

void
finalize_cache_write_request(struct cache_write_request *write_request)
{

	TRACE_IN(finalize_cache_write_request);
	free(write_request->entry);
	free(write_request->cache_key);
	free(write_request->data);
	TRACE_OUT(finalize_cache_write_request);
}

struct cache_write_request *
get_cache_write_request(struct comm_element *element)
{

	TRACE_IN(get_cache_write_request);
	assert(element->type == CET_WRITE_REQUEST);
	TRACE_OUT(get_cache_write_request);
	return (&element->c_write_request);
}

void
init_cache_write_response(struct cache_write_response *write_response)
{

	TRACE_IN(init_cache_write_response);
	memset(write_response, 0, sizeof(struct cache_write_response));
	TRACE_OUT(init_cache_write_response);
}

void
finalize_cache_write_response(struct cache_write_response *write_response)
{

	TRACE_IN(finalize_cache_write_response);
	TRACE_OUT(finalize_cache_write_response);
}

struct cache_write_response *
get_cache_write_response(struct comm_element *element)
{

	TRACE_IN(get_cache_write_response);
	assert(element->type == CET_WRITE_RESPONSE);
	TRACE_OUT(get_cache_write_response);
	return (&element->c_write_response);
}

void
init_cache_read_request(struct cache_read_request *read_request)
{

	TRACE_IN(init_cache_read_request);
	memset(read_request, 0, sizeof(struct cache_read_request));
	TRACE_OUT(init_cache_read_request);
}

void
finalize_cache_read_request(struct cache_read_request *read_request)
{

	TRACE_IN(finalize_cache_read_request);
	free(read_request->entry);
	free(read_request->cache_key);
	TRACE_OUT(finalize_cache_read_request);
}

struct cache_read_request *
get_cache_read_request(struct comm_element *element)
{

	TRACE_IN(get_cache_read_request);
	assert(element->type == CET_READ_REQUEST);
	TRACE_OUT(get_cache_read_request);
	return (&element->c_read_request);
}

void
init_cache_read_response(struct cache_read_response *read_response)
{

	TRACE_IN(init_cache_read_response);
	memset(read_response, 0, sizeof(struct cache_read_response));
	TRACE_OUT(init_cache_read_response);
}

void
finalize_cache_read_response(struct cache_read_response *read_response)
{

	TRACE_IN(finalize_cache_read_response);
	free(read_response->data);
	TRACE_OUT(finalize_cache_read_response);
}

struct cache_read_response *
get_cache_read_response(struct comm_element *element)
{

	TRACE_IN(get_cache_read_response);
	assert(element->type == CET_READ_RESPONSE);
	TRACE_OUT(get_cache_read_response);
	return (&element->c_read_response);
}

void
init_cache_transform_request(struct cache_transform_request *transform_request)
{

	TRACE_IN(init_cache_transform_request);
	memset(transform_request, 0, sizeof(struct cache_transform_request));
	TRACE_OUT(init_cache_transform_request);
}

void
finalize_cache_transform_request(
	struct cache_transform_request *transform_request)
{

	TRACE_IN(finalize_cache_transform_request);
	free(transform_request->entry);
	TRACE_OUT(finalize_cache_transform_request);
}

struct cache_transform_request *
get_cache_transform_request(struct comm_element *element)
{

	TRACE_IN(get_cache_transform_request);
	assert(element->type == CET_TRANSFORM_REQUEST);
	TRACE_OUT(get_cache_transform_request);
	return (&element->c_transform_request);
}

void
init_cache_transform_response(
	struct cache_transform_response *transform_response)
{

	TRACE_IN(init_cache_transform_request);
	memset(transform_response, 0, sizeof(struct cache_transform_response));
	TRACE_OUT(init_cache_transform_request);
}

void
finalize_cache_transform_response(
	struct cache_transform_response *transform_response)
{

	TRACE_IN(finalize_cache_transform_response);
	TRACE_OUT(finalize_cache_transform_response);
}

struct cache_transform_response *
get_cache_transform_response(struct comm_element *element)
{

	TRACE_IN(get_cache_transform_response);
	assert(element->type == CET_TRANSFORM_RESPONSE);
	TRACE_OUT(get_cache_transform_response);
	return (&element->c_transform_response);
}


void
init_cache_mp_write_session_request(
	struct cache_mp_write_session_request *mp_ws_request)
{

	TRACE_IN(init_cache_mp_write_session_request);
	memset(mp_ws_request, 0,
    		sizeof(struct cache_mp_write_session_request));
	TRACE_OUT(init_cache_mp_write_session_request);
}

void
finalize_cache_mp_write_session_request(
	struct cache_mp_write_session_request *mp_ws_request)
{

	TRACE_IN(finalize_cache_mp_write_session_request);
	free(mp_ws_request->entry);
	TRACE_OUT(finalize_cache_mp_write_session_request);
}

struct cache_mp_write_session_request *
get_cache_mp_write_session_request(struct comm_element *element)
{

	TRACE_IN(get_cache_mp_write_session_request);
	assert(element->type == CET_MP_WRITE_SESSION_REQUEST);
	TRACE_OUT(get_cache_mp_write_session_request);
	return (&element->c_mp_ws_request);
}

void
init_cache_mp_write_session_response(
	struct cache_mp_write_session_response *mp_ws_response)
{

	TRACE_IN(init_cache_mp_write_session_response);
	memset(mp_ws_response, 0,
    		sizeof(struct cache_mp_write_session_response));
	TRACE_OUT(init_cache_mp_write_session_response);
}

void
finalize_cache_mp_write_session_response(
	struct cache_mp_write_session_response *mp_ws_response)
{

	TRACE_IN(finalize_cache_mp_write_session_response);
	TRACE_OUT(finalize_cache_mp_write_session_response);
}

struct cache_mp_write_session_response *
get_cache_mp_write_session_response(struct comm_element *element)
{

	TRACE_IN(get_cache_mp_write_session_response);
	assert(element->type == CET_MP_WRITE_SESSION_RESPONSE);
	TRACE_OUT(get_cache_mp_write_session_response);
	return (&element->c_mp_ws_response);
}

void
init_cache_mp_write_session_write_request(
	struct cache_mp_write_session_write_request *mp_ws_write_request)
{

	TRACE_IN(init_cache_mp_write_session_write_request);
	memset(mp_ws_write_request, 0,
		sizeof(struct cache_mp_write_session_write_request));
	TRACE_OUT(init_cache_mp_write_session_write_response);
}

void
finalize_cache_mp_write_session_write_request(
	struct cache_mp_write_session_write_request *mp_ws_write_request)
{

	TRACE_IN(finalize_cache_mp_write_session_write_request);
	free(mp_ws_write_request->data);
	TRACE_OUT(finalize_cache_mp_write_session_write_request);
}

struct cache_mp_write_session_write_request *
get_cache_mp_write_session_write_request(struct comm_element *element)
{

	TRACE_IN(get_cache_mp_write_session_write_request);
	assert(element->type == CET_MP_WRITE_SESSION_WRITE_REQUEST);
	TRACE_OUT(get_cache_mp_write_session_write_request);
	return (&element->c_mp_ws_write_request);
}

void
init_cache_mp_write_session_write_response(
	struct cache_mp_write_session_write_response *mp_ws_write_response)
{

	TRACE_IN(init_cache_mp_write_session_write_response);
	memset(mp_ws_write_response, 0,
		sizeof(struct cache_mp_write_session_write_response));
	TRACE_OUT(init_cache_mp_write_session_write_response);
}

void
finalize_cache_mp_write_session_write_response(
	struct cache_mp_write_session_write_response *mp_ws_write_response)
{

	TRACE_IN(finalize_cache_mp_write_session_write_response);
	TRACE_OUT(finalize_cache_mp_write_session_write_response);
}

struct cache_mp_write_session_write_response *
get_cache_mp_write_session_write_response(struct comm_element *element)
{

	TRACE_IN(get_cache_mp_write_session_write_response);
	assert(element->type == CET_MP_WRITE_SESSION_WRITE_RESPONSE);
	TRACE_OUT(get_cache_mp_write_session_write_response);
	return (&element->c_mp_ws_write_response);
}

void
init_cache_mp_read_session_request(
	struct cache_mp_read_session_request *mp_rs_request)
{

	TRACE_IN(init_cache_mp_read_session_request);
	memset(mp_rs_request, 0, sizeof(struct cache_mp_read_session_request));
	TRACE_OUT(init_cache_mp_read_session_request);
}

void
finalize_cache_mp_read_session_request(
	struct cache_mp_read_session_request *mp_rs_request)
{

	TRACE_IN(finalize_cache_mp_read_session_request);
	free(mp_rs_request->entry);
	TRACE_OUT(finalize_cache_mp_read_session_request);
}

struct cache_mp_read_session_request *
get_cache_mp_read_session_request(struct comm_element *element)
{

	TRACE_IN(get_cache_mp_read_session_request);
	assert(element->type == CET_MP_READ_SESSION_REQUEST);
	TRACE_OUT(get_cache_mp_read_session_request);
	return (&element->c_mp_rs_request);
}

void
init_cache_mp_read_session_response(
	struct cache_mp_read_session_response *mp_rs_response)
{

	TRACE_IN(init_cache_mp_read_session_response);
	memset(mp_rs_response, 0,
    		sizeof(struct cache_mp_read_session_response));
	TRACE_OUT(init_cache_mp_read_session_response);
}

void
finalize_cache_mp_read_session_response(
	struct cache_mp_read_session_response *mp_rs_response)
{

	TRACE_IN(finalize_cache_mp_read_session_response);
	TRACE_OUT(finalize_cache_mp_read_session_response);
}

struct cache_mp_read_session_response *
get_cache_mp_read_session_response(struct comm_element *element)
{

	TRACE_IN(get_cache_mp_read_session_response);
	assert(element->type == CET_MP_READ_SESSION_RESPONSE);
	TRACE_OUT(get_cache_mp_read_session_response);
	return (&element->c_mp_rs_response);
}

void
init_cache_mp_read_session_read_response(
	struct cache_mp_read_session_read_response *mp_ws_read_response)
{

	TRACE_IN(init_cache_mp_read_session_read_response);
	memset(mp_ws_read_response, 0,
		sizeof(struct cache_mp_read_session_read_response));
	TRACE_OUT(init_cache_mp_read_session_read_response);
}

void
finalize_cache_mp_read_session_read_response(
	struct cache_mp_read_session_read_response *mp_rs_read_response)
{

	TRACE_IN(finalize_cache_mp_read_session_read_response);
	free(mp_rs_read_response->data);
	TRACE_OUT(finalize_cache_mp_read_session_read_response);
}

struct cache_mp_read_session_read_response *
get_cache_mp_read_session_read_response(struct comm_element *element)
{

	TRACE_IN(get_cache_mp_read_session_read_response);
	assert(element->type == CET_MP_READ_SESSION_READ_RESPONSE);
	TRACE_OUT(get_cache_mp_read_session_read_response);
	return (&element->c_mp_rs_read_response);
}