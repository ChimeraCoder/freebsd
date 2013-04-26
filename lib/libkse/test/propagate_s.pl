#!/usr/bin/perl -w
# You may redistribute this program and/or modify it under the terms of
# the GNU General Public License as published by the Free Software Foundation,
# either version 3 of the License, or (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

@CPOINTS = ("aio_suspend", "close", "creat", "fcntl", "fsync", "mq_receive",
	    "mq_send", "msync", "nanosleep", "open", "pause",
	    "pthread_cond_timedwait", "pthread_cond_wait", "pthread_join",
	    "pthread_testcancel", "read", "sem_wait", "sigsuspend",
	    "sigtimedwait", "sigwait", "sigwaitinfo", "sleep", "system",
	    "tcdrain", "wait", "waitpid", "write");

print "1..1\n";

$cpoints = join '\|', @CPOINTS;
$regexp = "\" U \\(" . $cpoints . "\\\)\$\"";

`nm -a /usr/lib/libc.a |grep $regexp >propagate_s.out`;
if (!open (NMOUT, "<./propagate_s.out"))
{
    print "not ok 1\n";
}
else
{
    $propagations = 0; 

    while (<NMOUT>)
    {
	$propagations++;
	print "$_\n";
    }
    if ($propagations != 0)
    {
	print "$propagations propagation(s)\n";
	print "not ok 1\n";
    }
    else
    {
	print "ok 1\n";
    }
    close NMOUT;
    unlink "propagate_s.out";
}