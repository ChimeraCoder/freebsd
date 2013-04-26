
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


#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>


int prepareFile(char* filename,int* fdp);
int mapBuffer(char** bufferp,int fd1,int fd2);
int startIO(int fd,char *buffer);

int pagesize;

#define FILESIZE (32*1024)
char wbuffer[FILESIZE];

/* Create a FILESIZE sized file - then remove file data from the cache*/
int prepareFile(char* filename,int* fdp)
{
  int fd;
  int len;
  int status;
  void *addr;

  fd = open(filename,O_CREAT | O_TRUNC | O_RDWR,S_IRWXU);
  if (fd == -1)
    {
      perror("Creating file");
      return fd;
    }
  
  len = write(fd,wbuffer,FILESIZE);
  if (len < 0)
    {
      perror("Write failed");
      return 1;
    }

  status = fsync(fd);
   if (status != 0)
    {
        perror("fsync failed");
	return 1;
    }

  addr = mmap(NULL,FILESIZE, PROT_READ | PROT_WRITE , MAP_SHARED, fd, 0);
  if (addr == MAP_FAILED)
    {
      perror("Mmap failed");
      return 1;
    }

  status = msync(addr,FILESIZE,MS_INVALIDATE | MS_SYNC);
  if (status != 0)
    {
        perror("Msync failed");
	return 1;
    }

  munmap(addr,FILESIZE);

  *fdp = fd;
  return 0;
}


/* mmap a 2 page buffer - first page is from fd1, second page from fd2 */
int mapBuffer(char** bufferp,int fd1,int fd2)
{
  void* addr;
  char *buffer;

  addr = mmap(NULL,pagesize*2, PROT_READ | PROT_WRITE , MAP_SHARED, fd1, 0);
  if (addr == MAP_FAILED)
    {
      perror("Mmap failed");
      return 1;
    }
 
  buffer = addr;
  addr = mmap(buffer + pagesize,pagesize, PROT_READ | PROT_WRITE , MAP_FIXED | 
MAP_SHARED, fd2, 0);
 
  if (addr == MAP_FAILED)
    {
      perror("Mmap2 failed");
      return 1;
    }
  *bufferp = buffer;
  return 0;
}


int startIO(int fd,char *buffer)
{
  ssize_t len;
  len = write(fd,buffer,2*pagesize);
  if (len == -1) 
    {
      perror("write failed");
      return 1;
    }
  return 0;
}


int main(int argc,char *argv[],char *envp[])
{

  int fdA,fdB,fdDelayA,fdDelayB;
  int status;
  char *bufferA,*bufferB;
  pid_t pid;

  pagesize = getpagesize();

  if ((prepareFile("A",&fdA))
      || (prepareFile("B",&fdB))
      || (prepareFile("DelayA",&fdDelayA))
      || (prepareFile("DelayB",&fdDelayB))
      || (mapBuffer(&bufferA,fdDelayA,fdB))
      || (mapBuffer(&bufferB,fdDelayB,fdA)))
    exit(1);
  
  pid = fork();

  if (pid == 0)
    {
      status = startIO(fdA,bufferA);
      exit(status);
    }

  if (pid == -1)
    {
      exit(1);
    }
  status = startIO(fdB,bufferB);
  exit(status);

}