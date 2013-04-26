
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

/**
 * \file
 *
 * This file contains functions to assist the iterator module.
 * The donotqueryaddresses are stored and looked up. These addresses
 * (like 127.0.0.1) must not be used to send queries to, and can be
 * discarded immediately from the server selection.
 */
#include "config.h"
#include "iterator/iter_donotq.h"
#include "util/regional.h"
#include "util/log.h"
#include "util/config_file.h"
#include "util/net_help.h"

struct iter_donotq* 
donotq_create(void)
{
	struct iter_donotq* dq = (struct iter_donotq*)calloc(1,
		sizeof(struct iter_donotq));
	if(!dq)
		return NULL;
	dq->region = regional_create();
	if(!dq->region) {
		donotq_delete(dq);
		return NULL;
	}
	return dq;
}

void 
donotq_delete(struct iter_donotq* dq)
{
	if(!dq) 
		return;
	regional_destroy(dq->region);
	free(dq);
}

/** insert new address into donotq structure */
static int
donotq_insert(struct iter_donotq* dq, struct sockaddr_storage* addr, 
	socklen_t addrlen, int net)
{
	struct addr_tree_node* node = (struct addr_tree_node*)regional_alloc(
		dq->region, sizeof(*node));
	if(!node)
		return 0;
	if(!addr_tree_insert(&dq->tree, node, addr, addrlen, net)) {
		verbose(VERB_QUERY, "duplicate donotquery address ignored.");
	}
	return 1;
}

/** apply donotq string */
static int
donotq_str_cfg(struct iter_donotq* dq, const char* str)
{
	struct sockaddr_storage addr;
	int net;
	socklen_t addrlen;
	verbose(VERB_ALGO, "donotq: %s", str);
	if(!netblockstrtoaddr(str, UNBOUND_DNS_PORT, &addr, &addrlen, &net)) {
		log_err("cannot parse donotquery netblock: %s", str);
		return 0;
	}
	if(!donotq_insert(dq, &addr, addrlen, net)) {
		log_err("out of memory");
		return 0;
	}
	return 1;
}

/** read donotq config */
static int 
read_donotq(struct iter_donotq* dq, struct config_file* cfg)
{
	struct config_strlist* p;
	for(p = cfg->donotqueryaddrs; p; p = p->next) {
		log_assert(p->str);
		if(!donotq_str_cfg(dq, p->str))
			return 0;
	}
	return 1;
}

int 
donotq_apply_cfg(struct iter_donotq* dq, struct config_file* cfg)
{
	regional_free_all(dq->region);
	addr_tree_init(&dq->tree);
	if(!read_donotq(dq, cfg))
		return 0;
	if(cfg->donotquery_localhost) {
		if(!donotq_str_cfg(dq, "127.0.0.0/8"))
			return 0;
		if(cfg->do_ip6) {
			if(!donotq_str_cfg(dq, "::1"))
				return 0;
		}
	}
	addr_tree_init_parents(&dq->tree);
	return 1;
}

int 
donotq_lookup(struct iter_donotq* donotq, struct sockaddr_storage* addr,
        socklen_t addrlen)
{
	return addr_tree_lookup(&donotq->tree, addr, addrlen) != NULL;
}

size_t 
donotq_get_mem(struct iter_donotq* donotq)
{
	if(!donotq) return 0;
	return sizeof(*donotq) + regional_get_mem(donotq->region);
}