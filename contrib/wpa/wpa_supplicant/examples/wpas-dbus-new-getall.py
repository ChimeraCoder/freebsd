#!/usr/bin/python
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

import dbus
import sys, os
import time
import gobject

def main():
	bus = dbus.SystemBus()
	wpas_obj = bus.get_object("fi.w1.wpa_supplicant1",
				  "/fi/w1/wpa_supplicant1")
	props = wpas_obj.GetAll("fi.w1.wpa_supplicant1",
				dbus_interface=dbus.PROPERTIES_IFACE)
	print "GetAll(fi.w1.wpa_supplicant1, /fi/w1/wpa_supplicant1):"
	print props

	if len(sys.argv) != 2:
		os._exit(1)

	ifname = sys.argv[1]

	wpas = dbus.Interface(wpas_obj, "fi.w1.wpa_supplicant1")
	path = wpas.GetInterface(ifname)
	if_obj = bus.get_object("fi.w1.wpa_supplicant1", path)
	props = if_obj.GetAll("fi.w1.wpa_supplicant1.Interface",
			      dbus_interface=dbus.PROPERTIES_IFACE)
	print
	print "GetAll(fi.w1.wpa_supplicant1.Interface, %s):" % (path)
	print props

	props = if_obj.GetAll("fi.w1.wpa_supplicant1.Interface.WPS",
			      dbus_interface=dbus.PROPERTIES_IFACE)
	print
	print "GetAll(fi.w1.wpa_supplicant1.Interface.WPS, %s):" % (path)
	print props

	res = if_obj.Get("fi.w1.wpa_supplicant1.Interface", 'BSSs',
			 dbus_interface=dbus.PROPERTIES_IFACE)
	if len(res) > 0:
		bss_obj = bus.get_object("fi.w1.wpa_supplicant1", res[0])
		props = bss_obj.GetAll("fi.w1.wpa_supplicant1.BSS",
				       dbus_interface=dbus.PROPERTIES_IFACE)
		print
		print "GetAll(fi.w1.wpa_supplicant1.BSS, %s):" % (res[0])
		print props

	res = if_obj.Get("fi.w1.wpa_supplicant1.Interface", 'Networks',
			 dbus_interface=dbus.PROPERTIES_IFACE)
	if len(res) > 0:
		net_obj = bus.get_object("fi.w1.wpa_supplicant1", res[0])
		props = net_obj.GetAll("fi.w1.wpa_supplicant1.Network",
				       dbus_interface=dbus.PROPERTIES_IFACE)
		print
		print "GetAll(fi.w1.wpa_supplicant1.Network, %s):" % (res[0])
		print props

if __name__ == "__main__":
	main()