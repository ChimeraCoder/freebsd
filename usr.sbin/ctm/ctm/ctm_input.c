
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

/*---------------------------------------------------------------------------*/
void
Fatal_(int ln, char *fn, char *kind)
{
    if(Verbose > 2)
	fprintf(stderr,"Fatal error. (%s:%d)\n",fn,ln);
    fprintf(stderr,"%s Fatal error: %s\n",FileName, kind);
}
#define Fatal(foo) Fatal_(__LINE__,__FILE__,foo)
#define Assert() Fatal_(__LINE__,__FILE__,"Assert failed.")

/*---------------------------------------------------------------------------*/
/* get next field, check that the terminating whitespace is what we expect */
u_char *
Ffield(FILE *fd, MD5_CTX *ctx,u_char term)
{
    static u_char buf[BUFSIZ];
    int i,l;

    for(l=0;;) {
	if((i=getc(fd)) == EOF) {
	    Fatal("Truncated patch.");
	    return 0;
	}
	buf[l++] = i;
	if(isspace(i))
	    break;
	if(l >= sizeof buf) {
	    Fatal("Corrupt patch.");
	    printf("Token is too long.\n");
	    return 0;
	}
    }
    buf[l] = '\0';
    MD5Update(ctx,buf,l);
    if(buf[l-1] != term) {
        Fatal("Corrupt patch.");
	fprintf(stderr,"Expected \"%s\" but didn't find it {%02x}.\n",
	    term == '\n' ? "\\n" : " ",buf[l-1]);
	if(Verbose > 4)
	    fprintf(stderr,"{%s}\n",buf);
	return 0;
    }
    buf[--l] = '\0';
    if(Verbose > 4)
        fprintf(stderr,"<%s>\n",buf);
    return buf;
}

int
Fbytecnt(FILE *fd, MD5_CTX *ctx, u_char term)
{
    u_char *p,*q;
    int u_chars=0;

    p = Ffield(fd,ctx,term);
    if(!p) return -1;
    for(q=p;*q;q++) {
	if(!isdigit(*q)) {
	    Fatal("Bytecount contains non-digit.");
	    return -1;
	}
	u_chars *= 10;
	u_chars += (*q - '0');
    }
    return u_chars;
}

u_char *
Fdata(FILE *fd, int u_chars, MD5_CTX *ctx)
{
    u_char *p = Malloc(u_chars+1);

    if(u_chars+1 != fread(p,1,u_chars+1,fd)) {
	Fatal("Truncated patch.");
	return 0;
    }
    MD5Update(ctx,p,u_chars+1);
    if(p[u_chars] != '\n') {
	if(Verbose > 3)
	    printf("FileData wasn't followed by a newline.\n");
        Fatal("Corrupt patch.");
	return 0;
    }
    p[u_chars] = '\0';
    return p;
}

/*---------------------------------------------------------------------------*/
/* get the filename in the next field, prepend BaseDir and give back the result
   strings. The sustitute filename is return (the one with the suffix SUBSUFF) 
   if it exists and the qualifier contains CTM_Q_Name_Subst
   NOTA: Buffer is already initialize with BaseDir, CatPtr is the insertion
   point on this buffer + the length test in Ffield() is enough for Fname() */

u_char *
Fname(FILE *fd, MD5_CTX *ctx,u_char term,int qual, int verbose)
{
    u_char * p;
    struct stat st;

    if ((p = Ffield(fd,ctx,term)) == NULL) return(NULL);

    strcpy(CatPtr, p);

    if (!(qual & CTM_Q_Name_Subst)) return(Buffer);

    p = Buffer + strlen(Buffer);

    strcat(Buffer, SUBSUFF);

    if ( -1 == stat(Buffer, &st) ) {
	*p = '\0';
    } else {
	if(verbose > 2)
	    fprintf(stderr,"Using %s as substitute file\n", Buffer);
    }

    return (Buffer);
}