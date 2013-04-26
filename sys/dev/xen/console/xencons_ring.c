
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
	/* XXX needs locking */	while (cons != prod) {
		xencons_rx(intf->in + MASK_XENCONS_IDX(cons, intf->in), 1);
		cons++;
	}

	mb();
	intf->in_cons = cons;

	CN_LOCK(cn_mtx);
	notify_remote_via_evtchn(xen_start_info->console_evtchn);

	xencons_tx();
	CN_UNLOCK(cn_mtx);
}

void 
xencons_ring_register_receiver(xencons_receiver_func *f)
{
	xencons_receiver = f;
}

int
xencons_ring_init(void)
{
	int err;

	if (!xen_start_info->console_evtchn)
		return 0;

	err = bind_caller_port_to_irqhandler(xen_start_info->console_evtchn,
		"xencons", xencons_handle_input, NULL,
		INTR_TYPE_MISC | INTR_MPSAFE, &console_irq);
	if (err) {
		return err;
	}

	return 0;
}

extern void xencons_suspend(void);
extern void xencons_resume(void);

void 
xencons_suspend(void)
{

	if (!xen_start_info->console_evtchn)
		return;

	unbind_from_irqhandler(console_irq);
}

void 
xencons_resume(void)
{

	(void)xencons_ring_init();
}

/*
 * Local variables:
 * mode: C
 * c-set-style: "BSD"
 * c-basic-offset: 8
 * tab-width: 4
 * indent-tabs-mode: t
 * End:
 */