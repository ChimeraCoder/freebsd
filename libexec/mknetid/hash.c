
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "hash.h"

#ifndef lint
static const char rcsid[] =
  "$FreeBSD$";
#endif /* not lint */

/*
 * This hash function is stolen directly from the
 * Berkeley DB package. It already exists inside libc, but
 * it's declared static which prevents us from calling it
 * from here.
 */
/*
 * OZ's original sdbm hash
 */
u_int32_t
hash(const void *keyarg, size_t len)
{
	const u_char *key;
	size_t loop;
	u_int32_t h;

#define HASHC   h = *key++ + 65599 * h

	h = 0;
	key = keyarg;
	if (len > 0) {
		loop = (len + 8 - 1) >> 3;

		switch (len & (8 - 1)) {
		case 0:
			do {
				HASHC;
				/* FALLTHROUGH */
		case 7:
				HASHC;
				/* FALLTHROUGH */
		case 6:
				HASHC;
				/* FALLTHROUGH */
		case 5:
				HASHC;
				/* FALLTHROUGH */
		case 4:
				HASHC;
				/* FALLTHROUGH */
		case 3:
				HASHC;
				/* FALLTHROUGH */
		case 2:
				HASHC;
				/* FALLTHROUGH */
		case 1:
				HASHC;
			} while (--loop);
		}
	}
	return (h);
}

/*
 * Generate a hash value for a given key (character string).
 * We mask off all but the lower 8 bits since our table array
 * can only hole 256 elements.
 */
u_int32_t hashkey(char *key)
{

	if (key == NULL)
		return (-1);
	return(hash((void *)key, strlen(key)) & HASH_MASK);
}

/* Find an entry in the hash table (may be hanging off a linked list). */
struct grouplist *lookup(struct member_entry *table[], char *key)
{
	struct member_entry *cur;

	cur = table[hashkey(key)];

	while (cur) {
		if (!strcmp(cur->key, key))
			return(cur->groups);
		cur = cur->next;
	}

	return(NULL);
}

struct grouplist dummy = { 99999, NULL };

/*
 * Store a group member entry and/or update its grouplist.
 */
void mstore (struct member_entry *table[], char *key, int gid, int dup)
{
	struct member_entry *cur, *new;
	struct grouplist *tmp;
	u_int32_t i;

	i = hashkey(key);
	cur = table[i];

	if (!dup) {
		tmp = (struct grouplist *)malloc(sizeof(struct grouplist));
		tmp->groupid = gid;
		tmp->next = NULL;
	}

		/* Check if all we have to do is insert a new groupname. */
	while (cur) {
		if (!dup && !strcmp(cur->key, key)) {
			tmp->next = cur->groups;
			cur->groups = tmp;
			return;
		}
		cur = cur->next;
	}

	/* Didn't find a match -- add the whole mess to the table. */
	new = (struct member_entry *)malloc(sizeof(struct member_entry));
	new->key = strdup(key);
	if (!dup)
		new->groups = tmp;
	else
		new->groups = (struct grouplist *)&dummy;
	new->next = table[i];
	table[i] = new;

	return;
}