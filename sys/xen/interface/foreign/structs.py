
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
# configuration: what needs translation
unions  = [ "vcpu_cr_regs",
            "vcpu_ar_regs" ];

structs = [ "start_info",
            "trap_info",
            "pt_fpreg",
            "cpu_user_regs",
            "xen_ia64_boot_param",
            "ia64_tr_entry",
            "vcpu_tr_regs",
            "vcpu_guest_context_regs",
            "vcpu_guest_context",
            "arch_vcpu_info",
            "vcpu_time_info",
            "vcpu_info",
            "arch_shared_info",
            "shared_info" ];

defines = [ "__i386__",
            "__x86_64__",

            "FLAT_RING1_CS",
            "FLAT_RING1_DS",
            "FLAT_RING1_SS",

            "FLAT_RING3_CS64",
            "FLAT_RING3_DS64",
            "FLAT_RING3_SS64",
            "FLAT_KERNEL_CS64",
            "FLAT_KERNEL_DS64",
            "FLAT_KERNEL_SS64",

            "FLAT_KERNEL_CS",
            "FLAT_KERNEL_DS",
            "FLAT_KERNEL_SS",

            # x86_{32,64}
            "_VGCF_i387_valid",
            "VGCF_i387_valid",
            "_VGCF_in_kernel",
            "VGCF_in_kernel",
            "_VGCF_failsafe_disables_events",
            "VGCF_failsafe_disables_events",
            "_VGCF_syscall_disables_events",
            "VGCF_syscall_disables_events",
            "_VGCF_online",
            "VGCF_online",

            # ia64
            "VGCF_EXTRA_REGS",

            # all archs
            "xen_pfn_to_cr3",
            "MAX_VIRT_CPUS",
            "MAX_GUEST_CMDLINE" ];