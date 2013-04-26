
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

/* hash.c --
 *
 * 	This module contains routines to manipulate a hash table.
 * 	See hash.h for a definition of the structure of the hash
 * 	table.  Hash tables grow automatically as the amount of
 * 	information increases.
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "hash.h"
#include "util.h"

/*
 * Forward references to local procedures that are used before they're
 * defined:
 */
static void RebuildTable(Hash_Table *);

/*
 * The following defines the ratio of # entries to # buckets
 * at which we rebuild the table to make it larger.
 */

#define	rebuildLimit 8

/*
 *---------------------------------------------------------
 *
 * Hash_InitTable --
 *
 * 	Set up the hash table t with a given number of buckets, or a
 * 	reasonable default if the number requested is less than or
 * 	equal to zero.  Hash tables will grow in size as needed.
 *
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	Memory is allocated for the initial bucket area.
 *
 *---------------------------------------------------------
 */
void
Hash_InitTable(Hash_Table *t, int numBuckets)
{
	int i;
	struct Hash_Entry **hp;

	/*
	 * Round up the size to a power of two.
	 */
	if (numBuckets <= 0)
		i = 16;
	else {
		for (i = 2; i < numBuckets; i <<= 1)
			 continue;
	}
	t->numEntries = 0;
	t->size = i;
	t->mask = i - 1;
	t->bucketPtr = hp = emalloc(sizeof(*hp) * i);
	while (--i >= 0)
		*hp++ = NULL;
}

/*
 *---------------------------------------------------------
 *
 * Hash_DeleteTable --
 *
 *	This routine removes everything from a hash table
 *	and frees up the memory space it occupied (except for
 *	the space in the Hash_Table structure).
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	Lots of memory is freed up.
 *
 *---------------------------------------------------------
 */
void
Hash_DeleteTable(Hash_Table *t)
{
	struct Hash_Entry **hp, *h, *nexth = NULL;
	int i;

	for (hp = t->bucketPtr, i = t->size; --i >= 0;) {
		for (h = *hp++; h != NULL; h = nexth) {
			nexth = h->next;
			free(h);
		}
	}
	free(t->bucketPtr);

	/*
	 * Set up the hash table to cause memory faults on any future access
	 * attempts until re-initialization.
	 */
	t->bucketPtr = NULL;
}

/*
 *---------------------------------------------------------
 *
 * Hash_FindEntry --
 *
 * 	Searches a hash table for an entry corresponding to key.
 *
 * Results:
 *	The return value is a pointer to the entry for key,
 *	if key was present in the table.  If key was not
 *	present, NULL is returned.
 *
 * Side Effects:
 *	None.
 *
 *---------------------------------------------------------
 */
Hash_Entry *
Hash_FindEntry(const Hash_Table *t, const char *key)
{
	Hash_Entry *e;
	unsigned h;
	const char *p;

	for (h = 0, p = key; *p;)
		h = (h << 5) - h + *p++;
	p = key;
	for (e = t->bucketPtr[h & t->mask]; e != NULL; e = e->next)
		if (e->namehash == h && strcmp(e->name, p) == 0)
			return (e);
	return (NULL);
}

/*
 *---------------------------------------------------------
 *
 * Hash_CreateEntry --
 *
 *	Searches a hash table for an entry corresponding to
 *	key.  If no entry is found, then one is created.
 *
 * Results:
 *	The return value is a pointer to the entry.  If *newPtr
 *	isn't NULL, then *newPtr is filled in with TRUE if a
 *	new entry was created, and FALSE if an entry already existed
 *	with the given key.
 *
 * Side Effects:
 *	Memory may be allocated, and the hash buckets may be modified.
 *---------------------------------------------------------
 */
Hash_Entry *
Hash_CreateEntry(Hash_Table *t, const char *key, Boolean *newPtr)
{
	Hash_Entry *e;
	unsigned int h;
	const char *p;
	int keylen;
	struct Hash_Entry **hp;

	/*
	 * Hash the key.  As a side effect, save the length (strlen) of the
	 * key in case we need to create the entry.
	 */
	for (h = 0, p = key; *p;)
		h = (h << 5) - h + *p++;
	keylen = p - key;
	p = key;
	for (e = t->bucketPtr[h & t->mask]; e != NULL; e = e->next) {
		if (e->namehash == h && strcmp(e->name, p) == 0) {
			if (newPtr != NULL)
				*newPtr = FALSE;
			return (e);
		}
	}

	/*
	 * The desired entry isn't there.  Before allocating a new entry,
	 * expand the table if necessary (and this changes the resulting
	 * bucket chain).
	 */
	if (t->numEntries >= rebuildLimit * t->size)
		RebuildTable(t);
	e = emalloc(sizeof(*e) + keylen);
	hp = &t->bucketPtr[h & t->mask];
	e->next = *hp;
	*hp = e;
	e->clientData = NULL;
	e->namehash = h;
	strcpy(e->name, p);
	t->numEntries++;

	if (newPtr != NULL)
		*newPtr = TRUE;
	return (e);
}

/*
 *---------------------------------------------------------
 *
 * Hash_DeleteEntry --
 *
 * 	Delete the given hash table entry and free memory associated with
 *	it.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	Hash chain that entry lives in is modified and memory is freed.
 *
 *---------------------------------------------------------
 */
void
Hash_DeleteEntry(Hash_Table *t, Hash_Entry *e)
{
	Hash_Entry **hp, *p;

	if (e == NULL)
		return;
	for (hp = &t->bucketPtr[e->namehash & t->mask];
	     (p = *hp) != NULL; hp = &p->next) {
		if (p == e) {
			*hp = p->next;
			free(p);
			t->numEntries--;
			return;
		}
	}
	write(STDERR_FILENO, "bad call to Hash_DeleteEntry\n", 29);
	abort();
}

/*
 *---------------------------------------------------------
 *
 * Hash_EnumFirst --
 *	This procedure sets things up for a complete search
 *	of all entries recorded in the hash table.
 *
 * Results:
 *	The return value is the address of the first entry in
 *	the hash table, or NULL if the table is empty.
 *
 * Side Effects:
 *	The information in searchPtr is initialized so that successive
 *	calls to Hash_Next will return successive HashEntry's
 *	from the table.
 *
 *---------------------------------------------------------
 */
Hash_Entry *
Hash_EnumFirst(const Hash_Table *t, Hash_Search *searchPtr)
{

	searchPtr->tablePtr = t;
	searchPtr->nextIndex = 0;
	searchPtr->hashEntryPtr = NULL;
	return (Hash_EnumNext(searchPtr));
}

/*
 *---------------------------------------------------------
 *
 * Hash_EnumNext --
 *    This procedure returns successive entries in the hash table.
 *
 * Results:
 *    The return value is a pointer to the next HashEntry
 *    in the table, or NULL when the end of the table is
 *    reached.
 *
 * Side Effects:
 *    The information in searchPtr is modified to advance to the
 *    next entry.
 *
 *---------------------------------------------------------
 */
Hash_Entry *
Hash_EnumNext(Hash_Search *searchPtr)
{
	Hash_Entry *e;
	const Hash_Table *t = searchPtr->tablePtr;

	/*
	 * The hashEntryPtr field points to the most recently returned
	 * entry, or is NULL if we are starting up.  If not NULL, we have
	 * to start at the next one in the chain.
	 */
	e = searchPtr->hashEntryPtr;
	if (e != NULL)
		e = e->next;
	/*
	 * If the chain ran out, or if we are starting up, we need to
	 * find the next nonempty chain.
	 */
	while (e == NULL) {
		if (searchPtr->nextIndex >= t->size)
			return (NULL);
		e = t->bucketPtr[searchPtr->nextIndex++];
	}
	searchPtr->hashEntryPtr = e;
	return (e);
}

/*
 *---------------------------------------------------------
 *
 * RebuildTable --
 *	This local routine makes a new hash table that
 *	is larger than the old one.
 *
 * Results:
 * 	None.
 *
 * Side Effects:
 *	The entire hash table is moved, so any bucket numbers
 *	from the old table are invalid.
 *
 *---------------------------------------------------------
 */
static void
RebuildTable(Hash_Table *t)
{
	Hash_Entry *e, *next = NULL, **hp, **xp;
	int i, mask;
        Hash_Entry **oldhp;
	int oldsize;

	oldhp = t->bucketPtr;
	oldsize = i = t->size;
	i <<= 1;
	t->size = i;
	t->mask = mask = i - 1;
	t->bucketPtr = hp = emalloc(sizeof(*hp) * i);
	while (--i >= 0)
		*hp++ = NULL;
	for (hp = oldhp, i = oldsize; --i >= 0;) {
		for (e = *hp++; e != NULL; e = next) {
			next = e->next;
			xp = &t->bucketPtr[e->namehash & mask];
			e->next = *xp;
			*xp = e;
		}
	}
	free(oldhp);
}