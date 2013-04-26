
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

/*
 * Dictionary support
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* HAVE_CONFIG_H */
#include <am_defs.h>
#include <fsi_data.h>
#include <fsinfo.h>


static int
dict_hash(char *k)
{
  u_int h;

  for (h = 0; *k; h += *k++) ;
  return h % DICTHASH;
}


dict *
new_dict(void)
{
  dict *dp = CALLOC(struct dict);

  return dp;
}


static void
dict_add_data(dict_ent *de, char *v)
{
  dict_data *dd = CALLOC(struct dict_data);

  dd->dd_data = v;
  ins_que(&dd->dd_q, de->de_q.q_back);
  de->de_count++;
}


static dict_ent *
new_dict_ent(char *k)
{
  dict_ent *de = CALLOC(struct dict_ent);

  de->de_key = k;
  init_que(&de->de_q);
  return de;
}


dict_ent *
dict_locate(dict *dp, char *k)
{
  dict_ent *de = dp->de[dict_hash(k)];

  while (de && !STREQ(de->de_key, k))
    de = de->de_next;
  return de;
}


void
dict_add(dict *dp, char *k, char *v)
{
  dict_ent *de = dict_locate(dp, k);

  if (!de) {
    dict_ent **dep = &dp->de[dict_hash(k)];
    de = new_dict_ent(k);
    de->de_next = *dep;
    *dep = de;
  }
  dict_add_data(de, v);
}


int
dict_iter(dict *dp, int (*fn) (qelem *))
{
  int i;
  int errors = 0;

  for (i = 0; i < DICTHASH; i++) {
    dict_ent *de = dp->de[i];
    while (de) {
      errors += (*fn) (&de->de_q);
      de = de->de_next;
    }
  }
  return errors;
}