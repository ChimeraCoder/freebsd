
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
#include "term.h"

static	const enum termfont fontmap[EQNFONT__MAX] = {
	TERMFONT_NONE, /* EQNFONT_NONE */
	TERMFONT_NONE, /* EQNFONT_ROMAN */
	TERMFONT_BOLD, /* EQNFONT_BOLD */
	TERMFONT_BOLD, /* EQNFONT_FAT */
	TERMFONT_UNDER /* EQNFONT_ITALIC */
};

static void	eqn_box(struct termp *, const struct eqn_box *);

void
term_eqn(struct termp *p, const struct eqn *ep)
{

	p->flags |= TERMP_NONOSPACE;
	eqn_box(p, ep->root);
	term_word(p, " ");
	p->flags &= ~TERMP_NONOSPACE;
}

static void
eqn_box(struct termp *p, const struct eqn_box *bp)
{

	if (EQNFONT_NONE != bp->font)
		term_fontpush(p, fontmap[(int)bp->font]);
	if (bp->left)
		term_word(p, bp->left);
	if (EQN_SUBEXPR == bp->type)
		term_word(p, "(");

	if (bp->text)
		term_word(p, bp->text);

	if (bp->first)
		eqn_box(p, bp->first);

	if (EQN_SUBEXPR == bp->type)
		term_word(p, ")");
	if (bp->right)
		term_word(p, bp->right);
	if (EQNFONT_NONE != bp->font) 
		term_fontpop(p);

	if (bp->next)
		eqn_box(p, bp->next);
}