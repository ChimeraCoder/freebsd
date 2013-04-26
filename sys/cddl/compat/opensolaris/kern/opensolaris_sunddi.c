
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
#include <sys/jail.h>
#include <sys/kernel.h>
#include <sys/libkern.h>
#include <sys/limits.h>
#include <sys/misc.h>
#include <sys/sunddi.h>
#include <sys/sysctl.h>

int
ddi_strtol(const char *str, char **nptr, int base, long *result)
{

	*result = strtol(str, nptr, base);
	if (*result == 0)
		return (EINVAL);
	else if (*result == LONG_MIN || *result == LONG_MAX)
		return (ERANGE);
	return (0);
}

int
ddi_strtoul(const char *str, char **nptr, int base, unsigned long *result)
{

	if (str == hw_serial) {
		*result = prison0.pr_hostid;
		return (0);
	}

	*result = strtoul(str, nptr, base);
	if (*result == 0)
		return (EINVAL);
	else if (*result == ULONG_MAX)
		return (ERANGE);
	return (0);
}

int
ddi_strtoull(const char *str, char **nptr, int base, unsigned long long *result)
{

	*result = (unsigned long long)strtouq(str, nptr, base);
	if (*result == 0)
		return (EINVAL);
	else if (*result == ULLONG_MAX)
		return (ERANGE);
	return (0);
}

struct ddi_soft_state_item {
	int	 ssi_item;
	void	*ssi_data;
	LIST_ENTRY(ddi_soft_state_item) ssi_next;
};

struct ddi_soft_state {
	size_t		ss_size;
	kmutex_t	ss_lock;
	LIST_HEAD(, ddi_soft_state_item) ss_list;
};

static void *
ddi_get_soft_state_locked(struct ddi_soft_state *ss, int item)
{
	struct ddi_soft_state_item *itemp;

	ASSERT(MUTEX_HELD(&ss->ss_lock));

	LIST_FOREACH(itemp, &ss->ss_list, ssi_next) {
		if (itemp->ssi_item == item)
			return (itemp->ssi_data);
	}
	return (NULL);
}

void *
ddi_get_soft_state(void *state, int item)
{
	struct ddi_soft_state *ss = state;
	void *data;

	mutex_enter(&ss->ss_lock);
	data = ddi_get_soft_state_locked(ss, item);
	mutex_exit(&ss->ss_lock);
	return (data);
}

int
ddi_soft_state_zalloc(void *state, int item)
{
	struct ddi_soft_state *ss = state;
	struct ddi_soft_state_item *itemp;

	itemp = kmem_alloc(sizeof(*itemp), KM_SLEEP);
	itemp->ssi_item = item;
	itemp->ssi_data = kmem_zalloc(ss->ss_size, KM_SLEEP);

	mutex_enter(&ss->ss_lock);
	if (ddi_get_soft_state_locked(ss, item) != NULL) {
		mutex_exit(&ss->ss_lock);
		kmem_free(itemp->ssi_data, ss->ss_size);
		kmem_free(itemp, sizeof(*itemp));
		return (DDI_FAILURE);
	}
	LIST_INSERT_HEAD(&ss->ss_list, itemp, ssi_next);
	mutex_exit(&ss->ss_lock);
	return (DDI_SUCCESS);
}

static void
ddi_soft_state_free_locked(struct ddi_soft_state *ss, int item)
{
	struct ddi_soft_state_item *itemp;

	ASSERT(MUTEX_HELD(&ss->ss_lock));

	LIST_FOREACH(itemp, &ss->ss_list, ssi_next) {
		if (itemp->ssi_item == item)
			break;
	}
	if (itemp != NULL) {
		LIST_REMOVE(itemp, ssi_next);
		kmem_free(itemp->ssi_data, ss->ss_size);
		kmem_free(itemp, sizeof(*itemp));
	}
}

void
ddi_soft_state_free(void *state, int item)
{
	struct ddi_soft_state *ss = state;

	mutex_enter(&ss->ss_lock);
	ddi_soft_state_free_locked(ss, item);
	mutex_exit(&ss->ss_lock);
}

int
ddi_soft_state_init(void **statep, size_t size, size_t nitems __unused)
{
	struct ddi_soft_state *ss;

	ss = kmem_alloc(sizeof(*ss), KM_SLEEP);
	mutex_init(&ss->ss_lock, NULL, MUTEX_DEFAULT, NULL);
	ss->ss_size = size;
	LIST_INIT(&ss->ss_list);
	*statep = ss;
	return (0);
}

void
ddi_soft_state_fini(void **statep)
{
	struct ddi_soft_state *ss = *statep;
	struct ddi_soft_state_item *itemp;
	int item;

	mutex_enter(&ss->ss_lock);
	while ((itemp = LIST_FIRST(&ss->ss_list)) != NULL) {
		item = itemp->ssi_item;
		ddi_soft_state_free_locked(ss, item);
	}
	mutex_exit(&ss->ss_lock);
	mutex_destroy(&ss->ss_lock);
	kmem_free(ss, sizeof(*ss));

	*statep = NULL;
}