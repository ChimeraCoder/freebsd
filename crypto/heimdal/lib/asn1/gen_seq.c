
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

#include "gen_locl.h"

RCSID("$Id$");

void
generate_type_seq (const Symbol *s)
{
    char *subname;
    Type *type;

    if (!seq_type(s->name))
	return;
    type = s->type;
    while(type->type == TTag)
	type = type->subtype;

    if (type->type != TSequenceOf && type->type != TSetOf) {
	fprintf(stderr, "%s not seq of %d\n", s->name, (int)type->type);
	return;
    }

    /*
     * Require the subtype to be a type so we can name it and use
     * copy_/free_
     */

    if (type->subtype->type != TType) {
	fprintf(stderr, "%s subtype is not a type, can't generate "
	       "sequence code for this case: %d\n",
		s->name, (int)type->subtype->type);
	exit(1);
    }

    subname = type->subtype->symbol->gen_name;

    fprintf (headerfile,
	     "ASN1EXP int   ASN1CALL add_%s  (%s *, const %s *);\n"
	     "ASN1EXP int   ASN1CALL remove_%s  (%s *, unsigned int);\n",
	     s->gen_name, s->gen_name, subname,
	     s->gen_name, s->gen_name);

    fprintf (codefile, "int ASN1CALL\n"
	     "add_%s(%s *data, const %s *element)\n"
	     "{\n",
	     s->gen_name, s->gen_name, subname);

    fprintf (codefile,
	     "int ret;\n"
	     "void *ptr;\n"
	     "\n"
	     "ptr = realloc(data->val, \n"
	     "\t(data->len + 1) * sizeof(data->val[0]));\n"
	     "if (ptr == NULL) return ENOMEM;\n"
	     "data->val = ptr;\n\n"
	     "ret = copy_%s(element, &data->val[data->len]);\n"
	     "if (ret) return ret;\n"
	     "data->len++;\n"
	     "return 0;\n",
	     subname);

    fprintf (codefile, "}\n\n");

    fprintf (codefile, "int ASN1CALL\n"
	     "remove_%s(%s *data, unsigned int element)\n"
	     "{\n",
	     s->gen_name, s->gen_name);

    fprintf (codefile,
	     "void *ptr;\n"
	     "\n"
	     "if (data->len == 0 || element >= data->len)\n"
	     "\treturn ASN1_OVERRUN;\n"
	     "free_%s(&data->val[element]);\n"
	     "data->len--;\n"
	     /* don't move if its the last element */
	     "if (element < data->len)\n"
	     "\tmemmove(&data->val[element], &data->val[element + 1], \n"
	     "\t\tsizeof(data->val[0]) * (data->len - element));\n"
	     /* resize but don't care about failures since it doesn't matter */
	     "ptr = realloc(data->val, data->len * sizeof(data->val[0]));\n"
	     "if (ptr != NULL || data->len == 0) data->val = ptr;\n"
	     "return 0;\n",
	     subname);

    fprintf (codefile, "}\n\n");
}