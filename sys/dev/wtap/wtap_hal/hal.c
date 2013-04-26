
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
#include "hal.h"
#include "../if_medium.h"
#include "handler.h"

static void
hal_tx_proc(void *arg, int npending)
{
	struct wtap_hal *hal = (struct wtap_hal *)arg;
	struct packet *p;

#if 0
	DWTAP_PRINTF("%s\n", __func__);
#endif

	hal = (struct wtap_hal *)arg;
	for(;;){
		p = medium_get_next_packet(hal->hal_md);
		if(p == NULL)
		return;

		hal->plugin->work(hal->plugin, p);

#if 0
		DWTAP_PRINTF("[%d] freeing m=%p\n", p->id, p->m);
#endif
		m_free(p->m);
		free(p, M_WTAP_PACKET);
	}
}

void
init_hal(struct wtap_hal *hal)
{

	DWTAP_PRINTF("%s\n", __func__);
	mtx_init(&hal->hal_mtx, "wtap_hal mtx", NULL, MTX_DEF | MTX_RECURSE);

	hal->hal_md = (struct wtap_medium *)malloc(sizeof(struct wtap_medium),
	    M_WTAP, M_NOWAIT | M_ZERO);
	bzero(hal->hal_md, sizeof(struct wtap_medium));

	init_medium(hal->hal_md);
	/* register event handler for packets */
	TASK_INIT(&hal->hal_md->tx_handler->proc, 0, hal_tx_proc, hal);
}

void
register_plugin(struct wtap_hal *hal, struct wtap_plugin *plugin)
{

	plugin->init(plugin);
	hal->plugin = plugin;
}

void
deregister_plugin(struct wtap_hal *hal)
{

	hal->plugin->deinit(hal->plugin);
	hal->plugin = NULL; /* catch illegal usages */
}

void
deinit_hal(struct wtap_hal *hal)
{

	DWTAP_PRINTF("%s\n", __func__);
	deinit_medium(hal->hal_md);
	free(hal->hal_md, M_WTAP);
	mtx_destroy(&hal->hal_mtx);
}

int32_t
new_wtap(struct wtap_hal *hal, int32_t id)
{
	static const uint8_t mac_pool[64][IEEE80211_ADDR_LEN] = {
	    {0,152,154,152,150,151},
	    {0,152,154,152,150,152},
	    {0,152,154,152,150,153},
	    {0,152,154,152,150,154},
	    {0,152,154,152,150,155},
	    {0,152,154,152,150,156},
	    {0,152,154,152,150,157},
	    {0,152,154,152,150,158},
	    {0,152,154,152,151,151},
	    {0,152,154,152,151,152},
	    {0,152,154,152,151,153},
	    {0,152,154,152,151,154},
	    {0,152,154,152,151,155},
	    {0,152,154,152,151,156},
	    {0,152,154,152,151,157},
	    {0,152,154,152,151,158},
	    {0,152,154,152,152,151},
	    {0,152,154,152,152,152},
	    {0,152,154,152,152,153},
	    {0,152,154,152,152,154},
	    {0,152,154,152,152,155},
	    {0,152,154,152,152,156},
	    {0,152,154,152,152,157},
	    {0,152,154,152,152,158},
	    {0,152,154,152,153,151},
	    {0,152,154,152,153,152},
	    {0,152,154,152,153,153},
	    {0,152,154,152,153,154},
	    {0,152,154,152,153,155},
	    {0,152,154,152,153,156},
	    {0,152,154,152,153,157},
	    {0,152,154,152,153,158},
	    {0,152,154,152,154,151},
	    {0,152,154,152,154,152},
	    {0,152,154,152,154,153},
	    {0,152,154,152,154,154},
	    {0,152,154,152,154,155},
	    {0,152,154,152,154,156},
	    {0,152,154,152,154,157},
	    {0,152,154,152,154,158},
	    {0,152,154,152,155,151},
	    {0,152,154,152,155,152},
	    {0,152,154,152,155,153},
	    {0,152,154,152,155,154},
	    {0,152,154,152,155,155},
	    {0,152,154,152,155,156},
	    {0,152,154,152,155,157},
	    {0,152,154,152,155,158},
	    {0,152,154,152,156,151},
	    {0,152,154,152,156,152},
	    {0,152,154,152,156,153},
	    {0,152,154,152,156,154},
	    {0,152,154,152,156,155},
	    {0,152,154,152,156,156},
	    {0,152,154,152,156,157},
	    {0,152,154,152,156,158},
	    {0,152,154,152,157,151},
	    {0,152,154,152,157,152},
	    {0,152,154,152,157,153},
	    {0,152,154,152,157,154},
	    {0,152,154,152,157,155},
	    {0,152,154,152,157,156},
	    {0,152,154,152,157,157},
	    {0,152,154,152,157,158}
	    };

	DWTAP_PRINTF("%s\n", __func__);
	uint8_t const *macaddr = mac_pool[id];
	if(hal->hal_devs[id] != NULL){
		printf("error, wtap_id=%d already created\n", id);
		return -1;
	}

	hal->hal_devs[id] = (struct wtap_softc *)malloc(
	    sizeof(struct wtap_softc), M_WTAP, M_NOWAIT | M_ZERO);
	bzero(hal->hal_devs[id], sizeof(struct wtap_softc));
	hal->hal_devs[id]->sc_md = hal->hal_md;
	hal->hal_devs[id]->id = id;
	mtx_init(&hal->hal_devs[id]->sc_mtx, "wtap_softc mtx", NULL,
	    MTX_DEF | MTX_RECURSE);

	if(wtap_attach(hal->hal_devs[id], macaddr)){
		printf("%s, cant alloc new wtap\n", __func__);
		return -1;
	}

	return 0;
}

int32_t
free_wtap(struct wtap_hal *hal, int32_t id)
{

	DWTAP_PRINTF("%s\n", __func__);
	if(hal->hal_devs[id] == NULL){
		printf("error, wtap_id=%d never created\n", id);
		return -1;
	}

	if(wtap_detach(hal->hal_devs[id]))
		printf("%s, cant alloc new wtap\n", __func__);
	mtx_destroy(&hal->hal_devs[id]->sc_mtx);
	free(hal->hal_devs[id], M_WTAP);
	hal->hal_devs[id] = NULL;
	return 0;
}