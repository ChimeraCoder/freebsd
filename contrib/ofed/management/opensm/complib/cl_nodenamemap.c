
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

#define _GNU_SOURCE
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <complib/cl_nodenamemap.h>

static int map_name(void *cxt, uint64_t guid, char *p)
{
	cl_qmap_t *map = cxt;
	name_map_item_t *item;

	p = strtok(p, "\"#");
	if (!p)
		return 0;

	item = malloc(sizeof(*item));
	if (!item)
		return -1;
	item->guid = guid;
	item->name = strdup(p);
	cl_qmap_insert(map, item->guid, (cl_map_item_t *)item);
	return 0;
}

nn_map_t *
open_node_name_map(char *node_name_map)
{
	nn_map_t *map;

	if (!node_name_map) {
#ifdef HAVE_DEFAULT_NODENAME_MAP
		struct stat buf;
		node_name_map = HAVE_DEFAULT_NODENAME_MAP;
		if (stat(node_name_map, &buf))
			return NULL;
#else
		return NULL;
#endif /* HAVE_DEFAULT_NODENAME_MAP */
	}

	map = malloc(sizeof(*map));
	if (!map)
		return NULL;
	cl_qmap_init(map);

	if (parse_node_map(node_name_map, map_name, map)) {
		fprintf(stderr,
			"WARNING failed to open node name map \"%s\" (%s)\n",
			node_name_map, strerror(errno));
			close_node_name_map(map);
			return NULL;
	}

	return map;
}

void
close_node_name_map(nn_map_t *map)
{
	name_map_item_t *item = NULL;

	if (!map)
		return;

	item = (name_map_item_t *)cl_qmap_head(map);
	while (item != (name_map_item_t *)cl_qmap_end(map)) {
		item = (name_map_item_t *)cl_qmap_remove(map, item->guid);
		free(item->name);
		free(item);
		item = (name_map_item_t *)cl_qmap_head(map);
	}
	free(map);
}

char *
remap_node_name(nn_map_t *map, uint64_t target_guid, char *nodedesc)
{
	char *rc = NULL;
	name_map_item_t *item = NULL;

	if (!map)
		goto done;

	item = (name_map_item_t *)cl_qmap_get(map, target_guid);
	if (item != (name_map_item_t *)cl_qmap_end(map))
		rc = strdup(item->name);

done:
	if (rc == NULL)
		rc = strdup(clean_nodedesc(nodedesc));
	return (rc);
}

char *
clean_nodedesc(char *nodedesc)
{
	int i = 0;

	nodedesc[63] = '\0';
	while (nodedesc[i]) {
		if (!isprint(nodedesc[i]))
			nodedesc[i] = ' ';
		i++;
	}

	return (nodedesc);
}

int parse_node_map(const char *file_name,
		   int (*create)(void *, uint64_t, char *), void *cxt)
{
	char line[256];
	FILE *f;

	if (!(f = fopen(file_name, "r")))
		return -1;

	while (fgets(line, sizeof(line), f)) {
		uint64_t guid;
		char *p, *e;

		p = line;
		while (isspace(*p))
			p++;
		if (*p == '\0' || *p == '\n' || *p == '#')
			continue;

		guid = strtoull(p, &e, 0);
		if (e == p || (!isspace(*e) && *e != '#' && *e != '\0')) {
			fclose(f);
			return -1;
		}

		p = e;
		while (isspace(*p))
			p++;

		e = strpbrk(p, "\n");
		if (e)
			*e = '\0';

		if (create(cxt, guid, p)) {
			fclose(f);
			return -1;
		}
	}

	fclose(f);
	return 0;
}