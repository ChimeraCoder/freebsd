
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

/* $Id$ */

/*! \file */

#include <config.h>

#include <string.h>

#include <isc/atomic.h>
#include <isc/buffer.h>
#include <isc/magic.h>
#include <isc/mem.h>
#include <isc/platform.h>
#include <isc/print.h>
#include <isc/rwlock.h>
#include <isc/stats.h>
#include <isc/util.h>

#define ISC_STATS_MAGIC			ISC_MAGIC('S', 't', 'a', 't')
#define ISC_STATS_VALID(x)		ISC_MAGIC_VALID(x, ISC_STATS_MAGIC)

#ifndef ISC_STATS_USEMULTIFIELDS
#if defined(ISC_RWLOCK_USEATOMIC) && defined(ISC_PLATFORM_HAVEXADD) && !defined(ISC_PLATFORM_HAVEXADDQ)
#define ISC_STATS_USEMULTIFIELDS 1
#else
#define ISC_STATS_USEMULTIFIELDS 0
#endif
#endif	/* ISC_STATS_USEMULTIFIELDS */

#if ISC_STATS_USEMULTIFIELDS
typedef struct {
	isc_uint32_t hi;
	isc_uint32_t lo;
} isc_stat_t;
#else
typedef isc_uint64_t isc_stat_t;
#endif

struct isc_stats {
	/*% Unlocked */
	unsigned int	magic;
	isc_mem_t	*mctx;
	int		ncounters;

	isc_mutex_t	lock;
	unsigned int	references; /* locked by lock */

	/*%
	 * Locked by counterlock or unlocked if efficient rwlock is not
	 * available.
	 */
#ifdef ISC_RWLOCK_USEATOMIC
	isc_rwlock_t	counterlock;
#endif
	isc_stat_t	*counters;

	/*%
	 * We don't want to lock the counters while we are dumping, so we first
	 * copy the current counter values into a local array.  This buffer
	 * will be used as the copy destination.  It's allocated on creation
	 * of the stats structure so that the dump operation won't fail due
	 * to memory allocation failure.
	 * XXX: this approach is weird for non-threaded build because the
	 * additional memory and the copy overhead could be avoided.  We prefer
	 * simplicity here, however, under the assumption that this function
	 * should be only rarely called.
	 */
	isc_uint64_t	*copiedcounters;
};

static isc_result_t
create_stats(isc_mem_t *mctx, int ncounters, isc_stats_t **statsp) {
	isc_stats_t *stats;
	isc_result_t result = ISC_R_SUCCESS;

	REQUIRE(statsp != NULL && *statsp == NULL);

	stats = isc_mem_get(mctx, sizeof(*stats));
	if (stats == NULL)
		return (ISC_R_NOMEMORY);

	result = isc_mutex_init(&stats->lock);
	if (result != ISC_R_SUCCESS)
		goto clean_stats;

	stats->counters = isc_mem_get(mctx, sizeof(isc_stat_t) * ncounters);
	if (stats->counters == NULL) {
		result = ISC_R_NOMEMORY;
		goto clean_mutex;
	}
	stats->copiedcounters = isc_mem_get(mctx,
					    sizeof(isc_uint64_t) * ncounters);
	if (stats->copiedcounters == NULL) {
		result = ISC_R_NOMEMORY;
		goto clean_counters;
	}

#ifdef ISC_RWLOCK_USEATOMIC
	result = isc_rwlock_init(&stats->counterlock, 0, 0);
	if (result != ISC_R_SUCCESS)
		goto clean_copiedcounters;
#endif

	stats->references = 1;
	memset(stats->counters, 0, sizeof(isc_stat_t) * ncounters);
	stats->mctx = NULL;
	isc_mem_attach(mctx, &stats->mctx);
	stats->ncounters = ncounters;
	stats->magic = ISC_STATS_MAGIC;

	*statsp = stats;

	return (result);

clean_counters:
	isc_mem_put(mctx, stats->counters, sizeof(isc_stat_t) * ncounters);

#ifdef ISC_RWLOCK_USEATOMIC
clean_copiedcounters:
	isc_mem_put(mctx, stats->copiedcounters,
		    sizeof(isc_stat_t) * ncounters);
#endif

clean_mutex:
	DESTROYLOCK(&stats->lock);

clean_stats:
	isc_mem_put(mctx, stats, sizeof(*stats));

	return (result);
}

void
isc_stats_attach(isc_stats_t *stats, isc_stats_t **statsp) {
	REQUIRE(ISC_STATS_VALID(stats));
	REQUIRE(statsp != NULL && *statsp == NULL);

	LOCK(&stats->lock);
	stats->references++;
	UNLOCK(&stats->lock);

	*statsp = stats;
}

void
isc_stats_detach(isc_stats_t **statsp) {
	isc_stats_t *stats;

	REQUIRE(statsp != NULL && ISC_STATS_VALID(*statsp));

	stats = *statsp;
	*statsp = NULL;

	LOCK(&stats->lock);
	stats->references--;
	UNLOCK(&stats->lock);

	if (stats->references == 0) {
		isc_mem_put(stats->mctx, stats->copiedcounters,
			    sizeof(isc_stat_t) * stats->ncounters);
		isc_mem_put(stats->mctx, stats->counters,
			    sizeof(isc_stat_t) * stats->ncounters);
		DESTROYLOCK(&stats->lock);
#ifdef ISC_RWLOCK_USEATOMIC
		isc_rwlock_destroy(&stats->counterlock);
#endif
		isc_mem_putanddetach(&stats->mctx, stats, sizeof(*stats));
	}
}

int
isc_stats_ncounters(isc_stats_t *stats) {
	REQUIRE(ISC_STATS_VALID(stats));

	return (stats->ncounters);
}

static inline void
incrementcounter(isc_stats_t *stats, int counter) {
	isc_int32_t prev;

#ifdef ISC_RWLOCK_USEATOMIC
	/*
	 * We use a "read" lock to prevent other threads from reading the
	 * counter while we "writing" a counter field.  The write access itself
	 * is protected by the atomic operation.
	 */
	isc_rwlock_lock(&stats->counterlock, isc_rwlocktype_read);
#endif

#if ISC_STATS_USEMULTIFIELDS
	prev = isc_atomic_xadd((isc_int32_t *)&stats->counters[counter].lo, 1);
	/*
	 * If the lower 32-bit field overflows, increment the higher field.
	 * Note that it's *theoretically* possible that the lower field
	 * overlaps again before the higher field is incremented.  It doesn't
	 * matter, however, because we don't read the value until
	 * isc_stats_copy() is called where the whole process is protected
	 * by the write (exclusive) lock.
	 */
	if (prev == (isc_int32_t)0xffffffff)
		isc_atomic_xadd((isc_int32_t *)&stats->counters[counter].hi, 1);
#elif defined(ISC_PLATFORM_HAVEXADDQ)
	UNUSED(prev);
	isc_atomic_xaddq((isc_int64_t *)&stats->counters[counter], 1);
#else
	UNUSED(prev);
	stats->counters[counter]++;
#endif

#ifdef ISC_RWLOCK_USEATOMIC
	isc_rwlock_unlock(&stats->counterlock, isc_rwlocktype_read);
#endif
}

static inline void
decrementcounter(isc_stats_t *stats, int counter) {
	isc_int32_t prev;

#ifdef ISC_RWLOCK_USEATOMIC
	isc_rwlock_lock(&stats->counterlock, isc_rwlocktype_read);
#endif

#if ISC_STATS_USEMULTIFIELDS
	prev = isc_atomic_xadd((isc_int32_t *)&stats->counters[counter].lo, -1);
	if (prev == 0)
		isc_atomic_xadd((isc_int32_t *)&stats->counters[counter].hi,
				-1);
#elif defined(ISC_PLATFORM_HAVEXADDQ)
	UNUSED(prev);
	isc_atomic_xaddq((isc_int64_t *)&stats->counters[counter], -1);
#else
	UNUSED(prev);
	stats->counters[counter]--;
#endif

#ifdef ISC_RWLOCK_USEATOMIC
	isc_rwlock_unlock(&stats->counterlock, isc_rwlocktype_read);
#endif
}

static void
copy_counters(isc_stats_t *stats) {
	int i;

#ifdef ISC_RWLOCK_USEATOMIC
	/*
	 * We use a "write" lock before "reading" the statistics counters as
	 * an exclusive lock.
	 */
	isc_rwlock_lock(&stats->counterlock, isc_rwlocktype_write);
#endif

#if ISC_STATS_USEMULTIFIELDS
	for (i = 0; i < stats->ncounters; i++) {
		stats->copiedcounters[i] =
				(isc_uint64_t)(stats->counters[i].hi) << 32 |
				stats->counters[i].lo;
	}
#else
	UNUSED(i);
	memcpy(stats->copiedcounters, stats->counters,
	       stats->ncounters * sizeof(isc_stat_t));
#endif

#ifdef ISC_RWLOCK_USEATOMIC
	isc_rwlock_unlock(&stats->counterlock, isc_rwlocktype_write);
#endif
}

isc_result_t
isc_stats_create(isc_mem_t *mctx, isc_stats_t **statsp, int ncounters) {
	REQUIRE(statsp != NULL && *statsp == NULL);

	return (create_stats(mctx, ncounters, statsp));
}

void
isc_stats_increment(isc_stats_t *stats, isc_statscounter_t counter) {
	REQUIRE(ISC_STATS_VALID(stats));
	REQUIRE(counter < stats->ncounters);

	incrementcounter(stats, (int)counter);
}

void
isc_stats_decrement(isc_stats_t *stats, isc_statscounter_t counter) {
	REQUIRE(ISC_STATS_VALID(stats));
	REQUIRE(counter < stats->ncounters);

	decrementcounter(stats, (int)counter);
}

void
isc_stats_dump(isc_stats_t *stats, isc_stats_dumper_t dump_fn,
	       void *arg, unsigned int options)
{
	int i;

	REQUIRE(ISC_STATS_VALID(stats));

	copy_counters(stats);

	for (i = 0; i < stats->ncounters; i++) {
		if ((options & ISC_STATSDUMP_VERBOSE) == 0 &&
		    stats->copiedcounters[i] == 0)
				continue;
		dump_fn((isc_statscounter_t)i, stats->copiedcounters[i], arg);
	}
}