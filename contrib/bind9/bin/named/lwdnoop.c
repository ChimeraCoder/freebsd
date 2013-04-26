
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

/* $Id: lwdnoop.c,v 1.13 2008/01/22 23:28:04 tbox Exp $ */

/*! \file */

#include <config.h>

#include <isc/socket.h>
#include <isc/util.h>

#include <named/types.h>
#include <named/lwdclient.h>

void
ns_lwdclient_processnoop(ns_lwdclient_t *client, lwres_buffer_t *b) {
	lwres_nooprequest_t *req;
	lwres_noopresponse_t resp;
	isc_result_t result;
	lwres_result_t lwres;
	isc_region_t r;
	lwres_buffer_t lwb;

	REQUIRE(NS_LWDCLIENT_ISRECVDONE(client));
	INSIST(client->byaddr == NULL);

	req = NULL;

	result = lwres_nooprequest_parse(client->clientmgr->lwctx,
					 b, &client->pkt, &req);
	if (result != LWRES_R_SUCCESS)
		goto send_error;

	client->pkt.recvlength = LWRES_RECVLENGTH;
	client->pkt.authtype = 0; /* XXXMLG */
	client->pkt.authlength = 0;
	client->pkt.result = LWRES_R_SUCCESS;

	resp.datalength = req->datalength;
	resp.data = req->data;

	lwres = lwres_noopresponse_render(client->clientmgr->lwctx, &resp,
					  &client->pkt, &lwb);
	if (lwres != LWRES_R_SUCCESS)
		goto cleanup_req;

	r.base = lwb.base;
	r.length = lwb.used;
	client->sendbuf = r.base;
	client->sendlength = r.length;
	result = ns_lwdclient_sendreply(client, &r);
	if (result != ISC_R_SUCCESS)
		goto cleanup_lwb;

	/*
	 * We can now destroy request.
	 */
	lwres_nooprequest_free(client->clientmgr->lwctx, &req);

	NS_LWDCLIENT_SETSEND(client);

	return;

 cleanup_lwb:
	lwres_context_freemem(client->clientmgr->lwctx, lwb.base, lwb.length);

 cleanup_req:
	lwres_nooprequest_free(client->clientmgr->lwctx, &req);

 send_error:
	ns_lwdclient_errorpktsend(client, LWRES_R_FAILURE);
}