
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

#include <sys/queue.h>

#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

#include <pjdlog.h>

#include "rangelock.h"

#ifndef	PJDLOG_ASSERT
#include <assert.h>
#define	PJDLOG_ASSERT(...)	assert(__VA_ARGS__)
#endif

#define	RANGELOCKS_MAGIC	0x94310c
struct rangelocks {
	int	 rls_magic;		/* Magic value. */
	TAILQ_HEAD(, rlock) rls_locks;	/* List of locked ranges. */
};

struct rlock {
	off_t	rl_start;
	off_t	rl_end;
	TAILQ_ENTRY(rlock) rl_next;
};

int
rangelock_init(struct rangelocks **rlsp)
{
	struct rangelocks *rls;

	PJDLOG_ASSERT(rlsp != NULL);

	rls = malloc(sizeof(*rls));
	if (rls == NULL)
		return (-1);

	TAILQ_INIT(&rls->rls_locks);

	rls->rls_magic = RANGELOCKS_MAGIC;
	*rlsp = rls;

	return (0);
}

void
rangelock_free(struct rangelocks *rls)
{
	struct rlock *rl;

	PJDLOG_ASSERT(rls->rls_magic == RANGELOCKS_MAGIC);

	rls->rls_magic = 0;

	while ((rl = TAILQ_FIRST(&rls->rls_locks)) != NULL) {
		TAILQ_REMOVE(&rls->rls_locks, rl, rl_next);
		free(rl);
	}
	free(rls);
}

int
rangelock_add(struct rangelocks *rls, off_t offset, off_t length)
{
	struct rlock *rl;

	PJDLOG_ASSERT(rls->rls_magic == RANGELOCKS_MAGIC);

	rl = malloc(sizeof(*rl));
	if (rl == NULL)
		return (-1);
	rl->rl_start = offset;
	rl->rl_end = offset + length;
	TAILQ_INSERT_TAIL(&rls->rls_locks, rl, rl_next);
	return (0);
}

void
rangelock_del(struct rangelocks *rls, off_t offset, off_t length)
{
	struct rlock *rl;

	PJDLOG_ASSERT(rls->rls_magic == RANGELOCKS_MAGIC);

	TAILQ_FOREACH(rl, &rls->rls_locks, rl_next) {
		if (rl->rl_start == offset && rl->rl_end == offset + length)
			break;
	}
	PJDLOG_ASSERT(rl != NULL);
	TAILQ_REMOVE(&rls->rls_locks, rl, rl_next);
	free(rl);
}

bool
rangelock_islocked(struct rangelocks *rls, off_t offset, off_t length)
{
	struct rlock *rl;
	off_t end;

	PJDLOG_ASSERT(rls->rls_magic == RANGELOCKS_MAGIC);

	end = offset + length;
	TAILQ_FOREACH(rl, &rls->rls_locks, rl_next) {
		if (rl->rl_start < end && rl->rl_end > offset)
			break;
	}
	return (rl != NULL);
}