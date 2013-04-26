
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
  /* initialize */  if (unreaped == 0)
    {
      unreaped = ualloc (unreaped, 1);
      unreaped[0].pid = 0;
      n = 1;
    }

  for (i = 0; unreaped[i].pid; i++)
    if (unreaped[i].pid == pid)
      {
	*status = unreaped[i].status;
	while (unreaped[i].pid)
	  {
	    unreaped[i] = unreaped[i+1];
	    i++;
	  }
	n--;
	return pid;
      }

  while (1)
    {
#ifdef HAVE_WAIT3
      pid_t p = wait3 (status, options, (struct rusage *) 0);
#else
      pid_t p = wait (status);
#endif

      if (p == 0 || p == -1 || p == pid)
	return p;

      n++;
      unreaped = ualloc (unreaped, n);
      unreaped[n-1].pid = p;
      unreaped[n-1].status = *status;
    }
}