
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
 * Get info from NIS+ (version 3) map
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* HAVE_CONFIG_H */
#include <am_defs.h>
#include <amd.h>

#define NISPLUS_KEY "key="
#define NISPLUS_ORGDIR ".org_dir"

struct nis_callback_data {
  mnt_map *ncd_m;
  char *ncd_map;
  void (*ncd_fn)();
};

struct nisplus_search_callback_data {
  nis_name key;
  char *value;
};


static int
nisplus_callback(const nis_name key, const nis_object *value, voidp opaquedata)
{
  char *kp = strnsave(ENTRY_VAL(value, 0), ENTRY_LEN(value, 0));
  char *vp = strnsave(ENTRY_VAL(value, 1), ENTRY_LEN(value, 1));
  struct nis_callback_data *data = (struct nis_callback_data *) opaquedata;

  dlog("NISplus callback for <%s,%s>", kp, vp);

  (*data->ncd_fn) (data->ncd_m, kp, vp);

  /*
   * We want more ...
   */
  return FALSE;
}


int
nisplus_reload(mnt_map *m, char *map, void (*fn) ())
{
  int error = 0;
  struct nis_callback_data data;
  nis_result *result;
  char *org;		/* if map does not have ".org_dir" then append it */
  nis_name map_name;
  size_t l;

  org = strstr(map, NISPLUS_ORGDIR);
  if (org == NULL)
    org = NISPLUS_ORGDIR;
  else
    org = "";

  /* make some room for the NIS map_name */
  l = strlen(map) + sizeof(NISPLUS_ORGDIR);
  map_name = xmalloc(l);
  if (map_name == NULL) {
    plog(XLOG_ERROR, "Unable to create map_name %s: %s",
	 map, strerror(ENOMEM));
    return ENOMEM;
  }
  xsnprintf(map_name, l, "%s%s", map, org);

  data.ncd_m = m;
  data.ncd_map = map_name;
  data.ncd_fn = fn;

  dlog("NISplus reload for %s", map);

  result = nis_list(map_name,
		    EXPAND_NAME | FOLLOW_LINKS | FOLLOW_PATH,
		    (int (*)()) nisplus_callback,
		    &data);

  /* free off the NIS map_name */
  XFREE(map_name);

  if (result->status != NIS_SUCCESS && result->status != NIS_CBRESULTS)
    error = 1;

  if (error)
    plog(XLOG_ERROR, "error grabbing nisplus map of %s: %s",
	 map,
	 nis_sperrno(result->status));

  nis_freeresult(result);
  return error;
}


static int
nisplus_search_callback(const nis_name key, const nis_object *value, voidp opaquedata)
{
  struct nisplus_search_callback_data *data = (struct nisplus_search_callback_data *) opaquedata;

  dlog("NISplus search callback for <%s>", ENTRY_VAL(value, 0));
  dlog("NISplus search callback value <%s>", ENTRY_VAL(value, 1));

  data->value = strnsave(ENTRY_VAL(value, 1), ENTRY_LEN(value, 1));
  return TRUE;
}


/*
 * Try to locate a key using NIS+.
 */
int
nisplus_search(mnt_map *m, char *map, char *key, char **val, time_t *tp)
{
  nis_result *result;
  int error = 0;
  struct nisplus_search_callback_data data;
  nis_name index;
  char *org;		/* if map does not have ".org_dir" then append it */
  size_t l;

  org = strstr(map, NISPLUS_ORGDIR);
  if (org == NULL)
    org = NISPLUS_ORGDIR;
  else
    org = "";

  /* make some room for the NIS index */
  l = sizeof('[')		/* for opening selection criteria */
    + sizeof(NISPLUS_KEY)
    + strlen(key)
    + sizeof(']')		/* for closing selection criteria */
    + sizeof(',')		/* + 1 for , separator */
    + strlen(map)
    + sizeof(NISPLUS_ORGDIR);
  index = xmalloc(l);
  if (index == NULL) {
    plog(XLOG_ERROR,
	 "Unable to create index %s: %s",
	 map,
	 strerror(ENOMEM));
    return ENOMEM;
  }
  xsnprintf(index, l, "[%s%s],%s%s", NISPLUS_KEY, key, map, org);

  data.key = key;
  data.value = NULL;

  dlog("NISplus search for %s", index);

  result = nis_list(index,
		    EXPAND_NAME | FOLLOW_LINKS | FOLLOW_PATH,
		    (int (*)()) nisplus_search_callback,
		    &data);

  /* free off the NIS index */
  XFREE(index);

  if (result == NULL) {
    plog(XLOG_ERROR, "nisplus_search: %s: %s", map, strerror(ENOMEM));
    return ENOMEM;
  }

  /*
   * Do something interesting with the return code
   */
  switch (result->status) {
  case NIS_SUCCESS:
  case NIS_CBRESULTS:

    if (data.value == NULL) {
      nis_object *value = result->objects.objects_val;
      dlog("NISplus search found <nothing>");
      dlog("NISplus search for %s: %s(%d)",
	   map, nis_sperrno(result->status), result->status);

      if (value != NULL)
	data.value = strnsave(ENTRY_VAL(value, 1), ENTRY_LEN(value, 1));
    }
    *val = data.value;

    if (*val) {
      error = 0;
      dlog("NISplus search found %s", *val);
    } else {
      error = ENOENT;
      dlog("NISplus search found nothing");
    }

    *tp = 0;
    break;

  case NIS_NOSUCHNAME:
    dlog("NISplus search returned %d", result->status);
    error = ENOENT;
    break;

  default:
    plog(XLOG_ERROR, "nisplus_search: %s: %s", map, nis_sperrno(result->status));
    error = EIO;
    break;
  }
  nis_freeresult(result);

  return error;
}


int
nisplus_init(mnt_map *m, char *map, time_t *tp)
{
  nis_result *result;
  char *org;		/* if map does not have ".org_dir" then append it */
  nis_name map_name;
  int error = 0;
  size_t l;

  org = strstr(map, NISPLUS_ORGDIR);
  if (org == NULL)
    org = NISPLUS_ORGDIR;
  else
    org = "";

  /* make some room for the NIS map_name */
  l = strlen(map) + sizeof(NISPLUS_ORGDIR);
  map_name = xmalloc(l);
  if (map_name == NULL) {
    plog(XLOG_ERROR,
	 "Unable to create map_name %s: %s",
	 map,
	 strerror(ENOMEM));
    return ENOMEM;
  }
  xsnprintf(map_name, l, "%s%s", map, org);

  result = nis_lookup(map_name, (EXPAND_NAME | FOLLOW_LINKS | FOLLOW_PATH));

  /* free off the NIS map_name */
  XFREE(map_name);

  if (result == NULL) {
    plog(XLOG_ERROR, "NISplus init <%s>: %s", map, strerror(ENOMEM));
    return ENOMEM;
  }

  if (result->status != NIS_SUCCESS) {
    dlog("NISplus init <%s>: %s (%d)",
	 map, nis_sperrno(result->status), result->status);

    error = ENOENT;
  }

  *tp = 0;			/* no time */
  nis_freeresult(result);
  return error;
}


int
nisplus_mtime(mnt_map *m, char *map, time_t *tp)
{
  return nisplus_init(m,map, tp);
}