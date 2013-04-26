
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

# TODO: This test program is about checking the test case's "environment"
# (not the variables).  Should be named something else than t_fork.

atf_test_case stop
stop_head()
{
    atf_set "descr" "Tests that sending a stop signal to a test case does" \
                    "not report it as failed"
}
stop_body()
{
    for h in $(get_helpers); do
        ${h} -s $(atf_get_srcdir) -v pidfile=$(pwd)/pid \
             -v donefile=$(pwd)/done -r resfile fork_stop &
        ppid=${!}
        echo "Waiting for pid file for test program ${ppid}"
        while test ! -f pid; do sleep 1; done
        pid=$(cat pid)
        echo "Test case's pid is ${pid}"
        kill -STOP ${pid}
        touch done
        echo "Wrote done file"
        kill -CONT ${pid}
        wait ${ppid}
        atf_check -s eq:0 -o ignore -e empty grep '^passed$' resfile
        rm -f pid done
    done
}

atf_init_test_cases()
{
    atf_add_test_case stop
}

# vim: syntax=sh:expandtab:shiftwidth=4:softtabstop=4