
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

import os
import signal
import subprocess
import time

if os.path.exists('results.pcap'):
    os.remove('results.pcap')
tcpdump = subprocess.Popen('tcpdump -n -i lo0 -w results.pcap icmp', shell=True)
time.sleep(1) # Give tcpdump time to start

os.system('sysctl net.inet.icmp.icmplim=0')
os.system('ping -q -i .001 -c 100000 127.0.0.1')

time.sleep(3) # Give tcpdump time to catch up
os.kill(tcpdump.pid, signal.SIGTERM)

os.system('tcpdump -n -v -r results.pcap > results.txt')

id_lastseen = {}
id_minperiod = {}

count = 0
for line in open('results.txt').readlines():
    id = int(line.split(' id ')[1].split(',')[0])
    if id in id_lastseen:
        period = count - id_lastseen[id]
        if id not in id_minperiod or period < id_minperiod[id]:
            id_minperiod[id] = period
    id_lastseen[id] = count
    count += 1

sorted_minperiod = list(zip(*reversed(list(zip(*list(id_minperiod.items()))))))
sorted_minperiod.sort()

print("Lowest 10 ID periods detected:")
x = 0
while x < 10:
    id_tuple = sorted_minperiod.pop(0)
    print("id: %d period: %d" % (id_tuple[1], id_tuple[0]))
    x += 1

print("Highest 10 ID periods detected:")
x = 0
while x < 10:
    id_tuple = sorted_minperiod.pop()
    print("id: %d period: %d" % (id_tuple[1], id_tuple[0]))
    x += 1