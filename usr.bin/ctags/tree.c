
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

#if 0
#ifndef lint
static char sccsid[] = "@(#)tree.c	8.3 (Berkeley) 4/2/94";
#endif
#endif

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <err.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ctags.h"

static void	add_node(NODE *, NODE *);
static void	free_tree(NODE *);

/*
 * pfnote --
 *	enter a new node in the tree
 */
void
pfnote(const char *name, int ln)
{
	NODE	*np;
	char	*fp;
	char	nbuf[MAXTOKEN];

	/*NOSTRICT*/
	if (!(np = (NODE *)malloc(sizeof(NODE)))) {
		warnx("too many entries to sort");
		put_entries(head);
		free_tree(head);
		/*NOSTRICT*/
		if (!(head = np = (NODE *)malloc(sizeof(NODE))))
			errx(1, "out of space");
	}
	if (!xflag && !strcmp(name, "main")) {
		if (!(fp = strrchr(curfile, '/')))
			fp = curfile;
		else
			++fp;
		(void)snprintf(nbuf, sizeof(nbuf), "M%s", fp);
		fp = strrchr(nbuf, '.');
		if (fp && !fp[2])
			*fp = EOS;
		name = nbuf;
	}
	if (!(np->entry = strdup(name)))
		err(1, NULL);
	np->file = curfile;
	np->lno = ln;
	np->left = np->right = 0;
	if (!(np->pat = strdup(lbuf)))
		err(1, NULL);
	if (!head)
		head = np;
	else
		add_node(np, head);
}

static void
add_node(NODE *node, NODE *cur_node)
{
	int	dif;

	dif = strcoll(node->entry, cur_node->entry);
	if (!dif) {
		if (node->file == cur_node->file) {
			if (!wflag)
				fprintf(stderr, "Duplicate entry in file %s, line %d: %s\nSecond entry ignored\n", node->file, lineno, node->entry);
			return;
		}
		if (!cur_node->been_warned)
			if (!wflag)
				fprintf(stderr, "Duplicate entry in files %s and %s: %s (Warning only)\n", node->file, cur_node->file, node->entry);
		cur_node->been_warned = YES;
	}
	else if (dif < 0)
		if (cur_node->left)
			add_node(node, cur_node->left);
		else
			cur_node->left = node;
	else if (cur_node->right)
		add_node(node, cur_node->right);
	else
		cur_node->right = node;
}

static void
free_tree(NODE *node)
{
	NODE *node_next;
	while (node) {
		if (node->right)
			free_tree(node->right);
		node_next = node->left;
		free(node);
		node = node_next;
	}
}