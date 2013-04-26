
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
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include "support.h"

#ifndef HAVE_ERR_H

void
warnx(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	fprintf(stderr, "warning: ");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	va_end(ap);
}

void
warn(const char *fmt, ...)
{
	va_list ap;
	int e = errno;

	va_start(ap, fmt);
	fprintf(stderr, "warning: ");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, ": %s\n", strerror(e));
	va_end(ap);
}

void
errx(int code, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	fprintf(stderr, "error: ");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	va_end(ap);
	exit(code);
}

void
err(int code, const char *fmt, ...)
{
	va_list ap;
	int e = errno;

	va_start(ap, fmt);
	fprintf(stderr, "error: ");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, ": %s\n", strerror(e));
	va_end(ap);
	exit(code);
}

#endif

#ifndef HAVE_STRLCPY

size_t
strlcpy(char *dst, const char *src, size_t len)
{
	size_t ret = strlen(dst);

	while (len > 1) {
		*dst++ = *src++;
		len--;
	}
	if (len > 0)
		*dst = '\0';
	return (ret);
}

#endif

#ifndef HAVE_GETADDRINFO

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

extern int h_nerr;
extern int h_errno;
extern const char *h_errlist[];

/*
 * VERY poor man's implementation
 */
int
getaddrinfo(const char *host, const char *port, const struct addrinfo *hints,
    struct addrinfo **res)
{
	struct hostent *hent;
	struct sockaddr_in *s;
	struct servent *sent;

	if ((hent = gethostbyname(host)) == NULL)
		return (h_errno);
	if (hent->h_addrtype != hints->ai_family)
		return (HOST_NOT_FOUND);
	if (hent->h_addrtype != AF_INET)
		return (HOST_NOT_FOUND);

	if ((*res = malloc(sizeof(**res))) == NULL)
		return (HOST_NOT_FOUND);

	(*res)->ai_flags = hints->ai_flags;
	(*res)->ai_family = hints->ai_family;
	(*res)->ai_socktype = hints->ai_socktype;
	(*res)->ai_protocol = hints->ai_protocol;
	(*res)->ai_next = NULL;

	if (((*res)->ai_addr = malloc(sizeof(struct sockaddr_in))) == NULL) {
		freeaddrinfo(*res);
		return (HOST_NOT_FOUND);
	}
	(*res)->ai_addrlen = sizeof(struct sockaddr_in);
	s = (struct sockaddr_in *)(*res)->ai_addr;
	s->sin_family = hints->ai_family;
	s->sin_len = sizeof(*s);
	memcpy(&s->sin_addr, hent->h_addr, 4);

	if ((sent = getservbyname(port, NULL)) == NULL) {
		freeaddrinfo(*res);
		return (HOST_NOT_FOUND);
	}
	s->sin_port = sent->s_port;

	return (0);
}

const char *
gai_strerror(int e)
{

	if (e < 0 || e >= h_nerr)
		return ("unknown error");
	return (h_errlist[e]);
}

void
freeaddrinfo(struct addrinfo *p)
{
	struct addrinfo *next;

	while (p != NULL) {
		next = p->ai_next;
		if (p->ai_addr != NULL)
			free(p->ai_addr);
		free(p);
		p = next;
	}
}

#endif