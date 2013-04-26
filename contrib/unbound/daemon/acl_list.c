
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
 * This file helps the server keep out queries from outside sources, that
 * should not be answered.
 */
#include "config.h"
#include "daemon/acl_list.h"
#include "util/regional.h"
#include "util/log.h"
#include "util/config_file.h"
#include "util/net_help.h"

struct acl_list* 
acl_list_create(void)
{
	struct acl_list* acl = (struct acl_list*)calloc(1,
		sizeof(struct acl_list));
	if(!acl)
		return NULL;
	acl->region = regional_create();
	if(!acl->region) {
		acl_list_delete(acl);
		return NULL;
	}
	return acl;
}

void 
acl_list_delete(struct acl_list* acl)
{
	if(!acl) 
		return;
	regional_destroy(acl->region);
	free(acl);
}

/** insert new address into acl_list structure */
static int
acl_list_insert(struct acl_list* acl, struct sockaddr_storage* addr, 
	socklen_t addrlen, int net, enum acl_access control, 
	int complain_duplicates)
{
	struct acl_addr* node = regional_alloc(acl->region,
		sizeof(struct acl_addr));
	if(!node)
		return 0;
	node->control = control;
	if(!addr_tree_insert(&acl->tree, &node->node, addr, addrlen, net)) {
		if(complain_duplicates)
			verbose(VERB_QUERY, "duplicate acl address ignored.");
	}
	return 1;
}

/** apply acl_list string */
static int
acl_list_str_cfg(struct acl_list* acl, const char* str, const char* s2,
	int complain_duplicates)
{
	struct sockaddr_storage addr;
	int net;
	socklen_t addrlen;
	enum acl_access control;
	if(strcmp(s2, "allow") == 0)
		control = acl_allow;
	else if(strcmp(s2, "deny") == 0)
		control = acl_deny;
	else if(strcmp(s2, "refuse") == 0)
		control = acl_refuse;
	else if(strcmp(s2, "allow_snoop") == 0)
		control = acl_allow_snoop;
	else {
		log_err("access control type %s unknown", str);
		return 0;
	}
	if(!netblockstrtoaddr(str, UNBOUND_DNS_PORT, &addr, &addrlen, &net)) {
		log_err("cannot parse access control: %s %s", str, s2);
		return 0;
	}
	if(!acl_list_insert(acl, &addr, addrlen, net, control, 
		complain_duplicates)) {
		log_err("out of memory");
		return 0;
	}
	return 1;
}

/** read acl_list config */
static int 
read_acl_list(struct acl_list* acl, struct config_file* cfg)
{
	struct config_str2list* p;
	for(p = cfg->acls; p; p = p->next) {
		log_assert(p->str && p->str2);
		if(!acl_list_str_cfg(acl, p->str, p->str2, 1))
			return 0;
	}
	return 1;
}

int 
acl_list_apply_cfg(struct acl_list* acl, struct config_file* cfg)
{
	regional_free_all(acl->region);
	addr_tree_init(&acl->tree);
	if(!read_acl_list(acl, cfg))
		return 0;
	/* insert defaults, with '0' to ignore them if they are duplicates */
	if(!acl_list_str_cfg(acl, "0.0.0.0/0", "refuse", 0))
		return 0;
	if(!acl_list_str_cfg(acl, "127.0.0.0/8", "allow", 0))
		return 0;
	if(cfg->do_ip6) {
		if(!acl_list_str_cfg(acl, "::0/0", "refuse", 0))
			return 0;
		if(!acl_list_str_cfg(acl, "::1", "allow", 0))
			return 0;
		if(!acl_list_str_cfg(acl, "::ffff:127.0.0.1", "allow", 0))
			return 0;
	}
	addr_tree_init_parents(&acl->tree);
	return 1;
}

enum acl_access 
acl_list_lookup(struct acl_list* acl, struct sockaddr_storage* addr,
        socklen_t addrlen)
{
	struct acl_addr* r = (struct acl_addr*)addr_tree_lookup(&acl->tree,
		addr, addrlen);
	if(r) return r->control;
	return acl_deny;
}

size_t 
acl_list_get_mem(struct acl_list* acl)
{
	if(!acl) return 0;
	return sizeof(*acl) + regional_get_mem(acl->region);
}