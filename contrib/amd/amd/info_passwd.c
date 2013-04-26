
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
 * Get info from password "file"
 *
 * This is experimental and probably doesn't do what you expect.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* HAVE_CONFIG_H */
#include <am_defs.h>
#include <amd.h>

#define	PASSWD_MAP	"/etc/passwd"

/* forward declarations */
int passwd_init(mnt_map *m, char *map, time_t *tp);
int passwd_search(mnt_map *m, char *map, char *key, char **pval, time_t *tp);


/*
 * Nothing to probe - check the map name is PASSWD_MAP.
 */
int
passwd_init(mnt_map *m, char *map, time_t *tp)
{
  *tp = 0;

  /*
   * Recognize the old format "PASSWD_MAP"
   * Uses default return string
   * "type:=nfs;rfs:=/${var0}/${var1};rhost:=${var1};sublink:=${var2};fs:=${autodir}${var3}"
   */
  if (STREQ(map, PASSWD_MAP))
    return 0;
  /*
   * Recognize the new format "PASSWD_MAP:pval-format"
   */
  if (!NSTREQ(map, PASSWD_MAP, sizeof(PASSWD_MAP) - 1))
    return ENOENT;
  if (map[sizeof(PASSWD_MAP)-1] != ':')
    return ENOENT;

  return 0;
}


/*
 * Grab the entry via the getpwname routine
 * Modify time is ignored by passwd - XXX
 */
int
passwd_search(mnt_map *m, char *map, char *key, char **pval, time_t *tp)
{
  char *dir = 0;
  struct passwd *pw;

  if (STREQ(key, "/defaults")) {
    *pval = strdup("type:=nfs");
    return 0;
  }
  pw = getpwnam(key);

  if (pw) {
    /*
     * We chop the home directory up as follows:
     * /anydir/dom1/dom2/dom3/user
     *
     * and return
     * rfs:=/anydir/dom3;rhost:=dom3.dom2.dom1;sublink:=user
     * and now have
     * var0:=pw-prefix:=anydir
     * var1:=pw-rhost:=dom3.dom2.dom1
     * var2:=pw-user:=user
     * var3:=pw-home:=/anydir/dom1/dom2/dom3/user
     *
     * This allows cross-domain entries in your passwd file.
     * ... but forget about security!
     */
    char *user;
    char *p, *q;
    char val[MAXPATHLEN];
    char rhost[MAXHOSTNAMELEN];
    dir = strdup(pw->pw_dir);

    /*
     * Find user name.  If no / then Invalid...
     */
    user = strrchr(dir, '/');
    if (!user)
      goto enoent;
    *user++ = '\0';

    /*
     * Find start of host "path".  If no / then Invalid...
     */
    p = strchr(dir + 1, '/');
    if (!p)
      goto enoent;
    *p++ = '\0';

    /*
     * At this point, p is dom1/dom2/dom3
     * Copy, backwards, into rhost replacing
     * / with .
     */
    rhost[0] = '\0';
    do {
      q = strrchr(p, '/');
      if (q) {
	xstrlcat(rhost, q + 1, sizeof(rhost));
	xstrlcat(rhost, ".", sizeof(rhost));
	*q = '\0';
      } else {
	xstrlcat(rhost, p, sizeof(rhost));
      }
    } while (q);

    /*
     * Sanity check
     */
    if (*rhost == '\0' || *user == '\0' || *dir == '\0')
      goto enoent;

    /*
     * Make up return string
     */
    q = strchr(rhost, '.');
    if (q)
      *q = '\0';
    p = strchr(map, ':');
    if (p)
      p++;
    else
      p = "type:=nfs;rfs:=/${var0}/${var1};rhost:=${var1};sublink:=${var2};fs:=${autodir}${var3}";
    xsnprintf(val, sizeof(val), "var0:=%s;var1:=%s;var2:=%s;var3:=%s;%s",
	      dir+1, rhost, user, pw->pw_dir, p);
    dlog("passwd_search: map=%s key=%s -> %s", map, key, val);
    if (q)
      *q = '.';
    *pval = strdup(val);
    return 0;
  }

enoent:
  if (dir)
    XFREE(dir);

  return ENOENT;
}