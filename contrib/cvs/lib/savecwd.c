
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
/* Record the location of the current working directory in CWD so that   the program may change to other directories and later use restore_cwd
   to return to the recorded location.  This function may allocate
   space using malloc (via xgetwd) or leave a file descriptor open;
   use free_cwd to perform the necessary free or close.  Upon failure,
   no memory is allocated, any locally opened file descriptors are
   closed;  return non-zero -- in that case, free_cwd need not be
   called, but doing so is ok.  Otherwise, return zero.  */

int
save_cwd (cwd)
     struct saved_cwd *cwd;
{
  static int have_working_fchdir = 1;

  cwd->desc = -1;
  cwd->name = NULL;

  if (have_working_fchdir)
    {
#ifdef HAVE_FCHDIR
      cwd->desc = open (".", O_RDONLY);
      if (cwd->desc < 0)
	{
	  error (0, errno, "cannot open current directory");
	  return 1;
	}

# if __sun__ || sun
      /* On SunOS 4, fchdir returns EINVAL if accounting is enabled,
	 so we have to fall back to chdir.  */
      if (fchdir (cwd->desc))
	{
	  if (errno == EINVAL)
	    {
	      close (cwd->desc);
	      cwd->desc = -1;
	      have_working_fchdir = 0;
	    }
	  else
	    {
	      error (0, errno, "current directory");
	      close (cwd->desc);
	      cwd->desc = -1;
	      return 1;
	    }
	}
# endif /* __sun__ || sun */
#else
#define fchdir(x) (abort (), 0)
      have_working_fchdir = 0;
#endif
    }

  if (!have_working_fchdir)
    {
      cwd->name = xgetwd ();
      if (cwd->name == NULL)
	{
	  error (0, errno, "cannot get current directory");
	  return 1;
	}
    }
  return 0;
}

/* Change to recorded location, CWD, in directory hierarchy.
   If "saved working directory", NULL))
   */

int
restore_cwd (cwd, dest)
     const struct saved_cwd *cwd;
     const char *dest;
{
  int fail = 0;
  if (cwd->desc >= 0)
    {
      if (fchdir (cwd->desc))
	{
	  error (0, errno, "cannot return to %s",
		 (dest ? dest : "saved working directory"));
	  fail = 1;
	}
    }
  else if (chdir (cwd->name) < 0)
    {
      error (0, errno, "%s", cwd->name);
      fail = 1;
    }
  return fail;
}

void
free_cwd (cwd)
     struct saved_cwd *cwd;
{
  if (cwd->desc >= 0)
    close (cwd->desc);
  if (cwd->name)
    free (cwd->name);
}