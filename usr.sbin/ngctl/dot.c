
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

#include <err.h>
#include <inttypes.h>
#include <netgraph.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "ngctl.h"

#define UNNAMED		"\\<unnamed\\>"

static int DotCmd(int ac, char **av);

const struct ngcmd dot_cmd = {
	DotCmd,
	"dot [outputfile]",
	"Produce a GraphViz (.dot) of the entire netgraph.",
	"If no outputfile is specified, stdout will be assumed.",
	{ "graphviz", "confdot" }
};

static int
DotCmd(int ac, char **av)
{
	struct ng_mesg *nlresp;
	struct namelist *nlist;
	FILE *f = stdout;
	int ch;
	u_int i;

	/* Get options */
	optind = 1;
	while ((ch = getopt(ac, av, "")) != -1) {
		switch (ch) {
		case '?':
		default:
			return (CMDRTN_USAGE);
			break;
		}
	}
	ac -= optind;
	av += optind;

	/* Get arguments */
	switch (ac) {
	case 1:
		f = fopen(av[0], "w");
		if (f == NULL) {
			warn("Could not open %s for writing", av[0]);
			return (CMDRTN_ERROR);
		}
	case 0:
		break;
	default:
		if (f != stdout)
			(void)fclose(f);
		return (CMDRTN_USAGE);
	}

	/* Get list of nodes */
	if (NgSendMsg(csock, ".", NGM_GENERIC_COOKIE, NGM_LISTNODES, NULL,
	    0) < 0) {
		warn("send listnodes msg");
		goto error;
	}
	if (NgAllocRecvMsg(csock, &nlresp, NULL) < 0) {
		warn("recv listnodes msg");
		goto error;
	}

	nlist = (struct namelist *)nlresp->data;
	fprintf(f, "graph netgraph {\n");
	/* TODO: implement rank = same or subgraphs at some point */
	fprintf(f, "\tedge [ weight = 1.0 ];\n");
	fprintf(f, "\tnode [ shape = record, fontsize = 12 ] {\n");
	for (i = 0; i < nlist->numnames; i++)
		fprintf(f, "\t\t\"%jx\" [ label = \"{%s:|{%s|[%jx]:}}\" ];\n",
		    (uintmax_t)nlist->nodeinfo[i].id,
		    nlist->nodeinfo[i].name[0] != '\0' ?
		    nlist->nodeinfo[i].name : UNNAMED,
		    nlist->nodeinfo[i].type, (uintmax_t)nlist->nodeinfo[i].id);
	fprintf(f, "\t};\n");

	fprintf(f, "\tsubgraph cluster_disconnected {\n");
	fprintf(f, "\t\tbgcolor = pink;\n");
	for (i = 0; i < nlist->numnames; i++)
		if (nlist->nodeinfo[i].hooks == 0)
			fprintf(f, "\t\t\"%jx\";\n",
			    (uintmax_t)nlist->nodeinfo[i].id);
	fprintf(f, "\t};\n");

	for (i = 0; i < nlist->numnames; i++) {
		struct ng_mesg *hlresp;
		struct hooklist *hlist;
		struct nodeinfo *ninfo;
		char path[NG_PATHSIZ];
		u_int j;

		(void)snprintf(path, sizeof(path), "[%jx]:",
		    (uintmax_t)nlist->nodeinfo[i].id);

		/* Get node info and hook list */
		if (NgSendMsg(csock, path, NGM_GENERIC_COOKIE, NGM_LISTHOOKS,
		    NULL, 0) < 0) {
			free(nlresp);
			warn("send listhooks msg");
			goto error;
		}
		if (NgAllocRecvMsg(csock, &hlresp, NULL) < 0) {
			free(nlresp);
			warn("recv listhooks msg");
			goto error;
		}

		hlist = (struct hooklist *)hlresp->data;
		ninfo = &hlist->nodeinfo;
		if (ninfo->hooks == 0) {
			free(hlresp);
			continue;
		}

		fprintf(f, "\tnode [ shape = octagon, fontsize = 10 ] {\n");
		for (j = 0; j < ninfo->hooks; j++)
			fprintf(f, "\t\t\"%jx.%s\" [ label = \"%s\" ];\n",
			    (uintmax_t)nlist->nodeinfo[i].id,
			    hlist->link[j].ourhook, hlist->link[j].ourhook);
		fprintf(f, "\t};\n");

		fprintf(f, "\t{\n\t\tedge [ weight = 2.0, style = bold ];\n");
		for (j = 0; j < ninfo->hooks; j++)
			fprintf(f, "\t\t\"%jx\" -- \"%jx.%s\";\n",
			    (uintmax_t)nlist->nodeinfo[i].id,
			    (uintmax_t)nlist->nodeinfo[i].id,
			    hlist->link[j].ourhook);
		fprintf(f, "\t};\n");

		for (j = 0; j < ninfo->hooks; j++) {
			/* Only print the edges going in one direction. */
			if (hlist->link[j].nodeinfo.id > nlist->nodeinfo[i].id)
				continue;
			fprintf(f, "\t\"%jx.%s\" -- \"%jx.%s\";\n",
			    (uintmax_t)nlist->nodeinfo[i].id,
			    hlist->link[j].ourhook,
			    (uintmax_t)hlist->link[j].nodeinfo.id,
			    hlist->link[j].peerhook);
		}
		free(hlresp);
	}

	fprintf(f, "};\n");

	free(nlresp);
	if (f != stdout)
		(void)fclose(f);
	return (CMDRTN_OK);
error:
	if (f != stdout)
		(void)fclose(f);
	return (CMDRTN_ERROR);
}