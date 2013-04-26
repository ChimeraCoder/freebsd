
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

#include <sys/queue.h>
#include <bluetooth.h>
#include <string.h>
#include <stdlib.h>
#include "profile.h"
#include "provider.h"

static TAILQ_HEAD(, provider)	providers = TAILQ_HEAD_INITIALIZER(providers);
static uint32_t			change_state = 0;		
static uint32_t			handle = 0;

/*
 * Register Service Discovery provider.
 * Should not be called more the once.
 */

int32_t
provider_register_sd(int32_t fd)
{
	extern profile_t	sd_profile_descriptor;
	extern profile_t	bgd_profile_descriptor;

	provider_p		sd = calloc(1, sizeof(*sd));
	provider_p		bgd = calloc(1, sizeof(*bgd));

	if (sd == NULL || bgd == NULL) {
		if (sd != NULL)
			free(sd);

		if (bgd != NULL)
			free(bgd);

		return (-1);
	}

	sd->profile = &sd_profile_descriptor;
	bgd->handle = 0;
	sd->fd = fd;
	TAILQ_INSERT_HEAD(&providers, sd, provider_next);

	bgd->profile = &bgd_profile_descriptor;
	bgd->handle = 1;
	sd->fd = fd;
	TAILQ_INSERT_AFTER(&providers, sd, bgd, provider_next);
	
	change_state ++;

	return (0);
}

/*
 * Register new provider for a given profile, bdaddr and session.
 */

provider_p
provider_register(profile_p const profile, bdaddr_p const bdaddr, int32_t fd,
	uint8_t const *data, uint32_t datalen)
{
	provider_p	provider = calloc(1, sizeof(*provider));

	if (provider != NULL) {
		provider->data = malloc(datalen);
		if (provider->data != NULL) {
			provider->profile = profile;
			memcpy(provider->data, data, datalen);

			/*
			 * Record handles 0x0 and 0x1 are reserved
			 * for SDP itself
			 */

			if (++ handle <= 1)
				handle = 2;

			provider->handle = handle;

			memcpy(&provider->bdaddr, bdaddr,
				sizeof(provider->bdaddr));
			provider->fd = fd;

			TAILQ_INSERT_TAIL(&providers, provider, provider_next);
			change_state ++;
		} else {
			free(provider);
			provider = NULL;
		}
	}

	return (provider);
}

/*
 * Unregister provider
 */

void
provider_unregister(provider_p provider)
{
	TAILQ_REMOVE(&providers, provider, provider_next);
	if (provider->data != NULL)
		free(provider->data);
	free(provider);
	change_state ++;
}

/*
 * Update provider data
 */

int32_t
provider_update(provider_p provider, uint8_t const *data, uint32_t datalen)
{
	uint8_t	*new_data = (uint8_t *) realloc(provider->data, datalen);

	if (new_data == NULL)
		return (-1);

	memcpy(new_data, data, datalen);
	provider->data = new_data;

	return (0);
}

/*
 * Get a provider for given record handle
 */

provider_p
provider_by_handle(uint32_t handle)
{
	provider_p	provider = NULL;

	TAILQ_FOREACH(provider, &providers, provider_next)
		if (provider->handle == handle)
			break;

	return (provider);
}

/*
 * Cursor access
 */

provider_p
provider_get_first(void)
{
	return (TAILQ_FIRST(&providers));
}

provider_p
provider_get_next(provider_p provider)
{
	return (TAILQ_NEXT(provider, provider_next));
}

/*
 * Return change state
 */

uint32_t
provider_get_change_state(void)
{
	return (change_state);
}