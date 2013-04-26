
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

#include "ctm.h"

int
ctm_edit(u_char *script, int length, char *filein, char *fileout)
{
    u_char *ep, cmd;
    int ln, ln2, iln, ret=0, c;
    FILE *fi=0,*fo=0;

    fi = fopen(filein,"r");
    if(!fi) {
	warn("%s", filein);
	return 8;
    }

    fo = fopen(fileout,"w");
    if(!fo) {
	warn("%s", fileout);
	fclose(fi);
	return 4;
    }
    iln = 1;
    for(ep=script;ep < script+length;) {
	cmd = *ep++;
	if(cmd != 'a' && cmd != 'd') { ret = 1; goto bye; }
	ln = 0;
	while(isdigit(*ep)) {
	    ln *= 10;
	    ln += (*ep++ - '0');
	}
	if(*ep++ != ' ') { ret = 1; goto bye; }
	ln2 = 0;
	while(isdigit(*ep)) {
	    ln2 *= 10;
	    ln2 += (*ep++ - '0');
	}
	if(*ep++ != '\n') { ret = 1; goto bye; }


	if(cmd == 'd') {
	    while(iln < ln) {
		c = getc(fi);
		if(c == EOF) { ret = 1; goto bye; }
		putc(c,fo);
		if(c == '\n')
		    iln++;
	    }
	    while(ln2) {
		c = getc(fi);
		if(c == EOF) { ret = 1; goto bye; }
		if(c != '\n')
		    continue;
		ln2--;
		iln++;
	    }
	    continue;
	}
	if(cmd == 'a') {
	    while(iln <= ln) {
		c = getc(fi);
		if(c == EOF) { ret = 1; goto bye; }
		putc(c,fo);
		if(c == '\n')
		    iln++;
	    }
	    while(ln2) {
		c = *ep++;
		putc(c,fo);
		if(c != '\n')
		    continue;
		ln2--;
	    }
	    continue;
	}
	ret = 1;
	goto bye;
    }
    while(1) {
	c = getc(fi);
	if(c == EOF) break;
	putc(c,fo);
    }
    ret = 0;
bye:
    if(fi) {
	if(fclose(fi) != 0) {
	    warn("%s", filein);
	    ret = 1;
	}
    }
    if(fo) {
     	if(fflush(fo) != 0) {
	    warn("%s", fileout);
	    ret = 1;
     	}
     	if(fclose(fo) != 0) {
	    warn("%s", fileout);
	    ret = 1;
     	}
    }
    return ret;
}