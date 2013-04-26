
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
		*line = '\0';		FD_ZERO(&rd);
		FD_SET(0,&rd);
		FD_SET(afd,&rd);
		now = time(NULL);
		if (expire >= now)
		    {
			tv2.tv_usec = 0;
			tv2.tv_sec = expire - now;
			nfd = select(FD_SETSIZE, &rd, NULL, NULL, &tv2);
		    }
		else
			nfd = select(FD_SETSIZE, &rd, NULL, NULL, NULL);

		if (FD_ISSET(0, &rd))
		{
			if (!fgets(line, sizeof(line) - 1, stdin))
				exit(0);
			if (s = index(line, '\n'))
				*s = '\0';
		}

		if (isalpha(*line))
		{
			(void)printf("Asking about [%s] #%d.\n",line, ++seq);
			(void)ar_gethostbyname(line, (char *)&seq,
					       sizeof(seq));
			lookup++;
		}
		else if (isdigit(*line))
		{
			(void)printf("Asking about IP#[%s] #%d.\n",
				line, ++seq);
			adr.s_addr = inet_addr(line);
			(void)ar_gethostbyaddr(&adr, (char *)&seq,
					       sizeof(seq));
			lookup++;
		}
		if (lookup)
			(void)printf("Waiting for answer:\n");
		if (FD_ISSET(afd, &rd))
			(void)waitonlookup(afd);
		del = 0;
		expire = ar_timeout(time(NULL), &del, sizeof(del));
		if (del)
		{
			(void)fprintf(stderr,"#%d failed\n", del);
			lookup--;
		}
	}
}

printhostent(hp)
struct hostent *hp;
{
	struct in_addr ip;
	int i;

	(void)printf("hname = %s\n", hp->h_name);
	for (i = 0; hp->h_aliases[i]; i++)
		(void)printf("alias %d = %s\n", i+1, hp->h_aliases[i]);
	for (i = 0; hp->h_addr_list[i]; i++)
	{
		bcopy(hp->h_addr_list[i], (char *)&ip, sizeof(ip));
		(void)printf("IP# %d = %s\n", i+1, inet_ntoa(ip));
	}
}

int	waitonlookup(afd)
int	afd;
{
	struct	timeval delay;
	struct	hostent	*hp;
	fd_set	rd;
	long	now;
	int	nfd, del;

waitloop:
	FD_ZERO(&rd);
	now = time(NULL);
	if (expire >= now)
		delay.tv_sec = expire - now;
	else
		delay.tv_sec = 1;
	delay.tv_usec = 0;
	FD_SET(afd, &rd);
	FD_SET(0, &rd);

	nfd = select(FD_SETSIZE, &rd, 0, 0, &delay);
	if (nfd == 0)
		return 0;
	else if (FD_ISSET(afd, &rd))
	{
		del = 0;
		hp = ar_answer(&del, sizeof(del));

		(void)printf("hp=%x seq=%d\n",hp,del);
		if (hp)
		    {
			(void)printhostent(hp);
			if (!--lookup)
				return 1;
		    }
	}
	if (FD_ISSET(0, &rd))
		return 2;
	return 0;
}