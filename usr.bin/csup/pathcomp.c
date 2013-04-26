
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

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "misc.h"
#include "pathcomp.h"

struct pathcomp {
	char *target;
	size_t targetlen;
	char *trashed;
	char *prev;
	size_t prevlen;
	size_t goal;
	size_t curlen;
};

struct pathcomp	*
pathcomp_new(void)
{
	struct pathcomp *pc;

	pc = xmalloc(sizeof(struct pathcomp));
	pc->curlen = 0;
	pc->target = NULL;
	pc->targetlen = 0;
	pc->trashed = NULL;
	pc->prev = NULL;
	pc->prevlen = 0;
	return (pc);
}

int
pathcomp_put(struct pathcomp *pc, int type, char *path)
{
	char *cp;

	assert(pc->target == NULL);
	if (*path == '/')
		return (-1);

	switch (type) {
	case PC_DIRDOWN:
		pc->target = path;
		pc->targetlen = strlen(path);
		break;
	case PC_FILE:
	case PC_DIRUP:
		cp = strrchr(path, '/');
		pc->target = path;
		if (cp != NULL)
			pc->targetlen = cp - path;
		else
			pc->targetlen = 0;
		break;
	}
	if (pc->prev != NULL)
		pc->goal = commonpathlength(pc->prev, pc->prevlen, pc->target,
		    pc->targetlen);
	else
		pc->goal = 0;
	if (pc->curlen == pc->goal)	/* No need to go up. */
		pc->goal = pc->targetlen;
	return (0);
}

int
pathcomp_get(struct pathcomp *pc, int *type, char **name)
{
	char *cp;
	size_t slashpos, start;

	if (pc->curlen > pc->goal) {		/* Going up. */
		assert(pc->prev != NULL);
		pc->prev[pc->curlen] = '\0';
		cp = pc->prev + pc->curlen - 1;
		while (cp >= pc->prev) {
			if (*cp == '/')
				break;
			cp--;
		}
		if (cp >= pc->prev)
			slashpos = cp - pc->prev;
		else
			slashpos = 0;
		pc->curlen = slashpos;
		if (pc->curlen <= pc->goal) {	/* Done going up. */
			assert(pc->curlen == pc->goal);
			pc->goal = pc->targetlen;
		}
		*type = PC_DIRUP;
		*name = pc->prev;
		return (1);
	} else if (pc->curlen < pc->goal) {	/* Going down. */
		/* Restore the previously overwritten '/' character. */
		if (pc->trashed != NULL) {
			*pc->trashed = '/';
			pc->trashed = NULL;
		}
		if (pc->curlen == 0)
			start = pc->curlen;
		else
			start = pc->curlen + 1;
		slashpos = start;
		while (slashpos < pc->goal) {
			if (pc->target[slashpos] == '/')
				break;
			slashpos++;
		}
		if (pc->target[slashpos] != '\0') {
			assert(pc->target[slashpos] == '/');
			pc->trashed = pc->target + slashpos;
			pc->target[slashpos] = '\0';
		}
		pc->curlen = slashpos;
		*type = PC_DIRDOWN;
		*name = pc->target;
		return (1);
	} else {	/* Done. */
		if (pc->target != NULL) {
			if (pc->trashed != NULL) {
				*pc->trashed = '/';
				pc->trashed = NULL;
			}
			if (pc->prev != NULL)
				free(pc->prev);
			pc->prev = xmalloc(pc->targetlen + 1);
			memcpy(pc->prev, pc->target, pc->targetlen);
			pc->prev[pc->targetlen] = '\0';
			pc->prevlen = pc->targetlen;
			pc->target = NULL;
			pc->targetlen = 0;
		}
		return (0);
	}
}

void
pathcomp_finish(struct pathcomp *pc)
{

	pc->target = NULL;
	pc->targetlen = 0;
	pc->goal = 0;
}

void
pathcomp_free(struct pathcomp *pc)
{

	if (pc->prev != NULL)
		free(pc->prev);
	free(pc);
}