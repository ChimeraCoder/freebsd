
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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mandoc.h"
#include "out.h"
#include "html.h"

static	const enum htmltag fontmap[EQNFONT__MAX] = {
	TAG_SPAN, /* EQNFONT_NONE */
	TAG_SPAN, /* EQNFONT_ROMAN */
	TAG_B, /* EQNFONT_BOLD */
	TAG_B, /* EQNFONT_FAT */
	TAG_I /* EQNFONT_ITALIC */
};


static void	eqn_box(struct html *, const struct eqn_box *);

void
print_eqn(struct html *p, const struct eqn *ep)
{
	struct htmlpair	 tag;
	struct tag	*t;

	PAIR_CLASS_INIT(&tag, "eqn");
	t = print_otag(p, TAG_SPAN, 1, &tag);

	p->flags |= HTML_NONOSPACE;
	eqn_box(p, ep->root);
	p->flags &= ~HTML_NONOSPACE;

	print_tagq(p, t);
}

static void
eqn_box(struct html *p, const struct eqn_box *bp)
{
	struct tag	*t;

	t = EQNFONT_NONE == bp->font ? NULL : 
		print_otag(p, fontmap[(int)bp->font], 0, NULL);

	if (bp->left)
		print_text(p, bp->left);
	
	if (bp->text)
		print_text(p, bp->text);

	if (bp->first)
		eqn_box(p, bp->first);

	if (NULL != t)
		print_tagq(p, t);
	if (bp->right)
		print_text(p, bp->right);

	if (bp->next)
		eqn_box(p, bp->next);
}