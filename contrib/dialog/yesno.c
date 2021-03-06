
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

#include <dialog.h>
#include <dlg_keys.h>

/*
 * Display a dialog box with two buttons - Yes and No.
 */
int
dialog_yesno(const char *title, const char *cprompt, int height, int width)
{
    /* *INDENT-OFF* */
    static DLG_KEYS_BINDING binding[] = {
	HELPKEY_BINDINGS,
	ENTERKEY_BINDINGS,
	DLG_KEYS_DATA( DLGK_ENTER,	' ' ),
	DLG_KEYS_DATA( DLGK_FIELD_NEXT,	KEY_DOWN ),
	DLG_KEYS_DATA( DLGK_FIELD_NEXT, KEY_RIGHT ),
	DLG_KEYS_DATA( DLGK_FIELD_NEXT, TAB ),
	DLG_KEYS_DATA( DLGK_FIELD_PREV,	KEY_UP ),
	DLG_KEYS_DATA( DLGK_FIELD_PREV, KEY_BTAB ),
	DLG_KEYS_DATA( DLGK_FIELD_PREV, KEY_LEFT ),
	SCROLLKEY_BINDINGS,
	END_KEYS_BINDING
    };
    /* *INDENT-ON* */

    int x, y;
    int key = 0, fkey;
    int code;
    int button = dlg_defaultno_button();
    WINDOW *dialog = 0;
    int result = DLG_EXIT_UNKNOWN;
    char *prompt = dlg_strclone(cprompt);
    const char **buttons = dlg_yes_labels();
    int min_width = 25;
    bool show = TRUE;
    int page, last = 0, offset = 0;

#ifdef KEY_RESIZE
    int req_high = height;
    int req_wide = width;
  restart:
#endif

    dlg_tab_correct_str(prompt);
    dlg_button_layout(buttons, &min_width);
    dlg_auto_size(title, prompt, &height, &width, 2, min_width);
    dlg_print_size(height, width);
    dlg_ctl_size(height, width);

    x = dlg_box_x_ordinate(width);
    y = dlg_box_y_ordinate(height);

#ifdef KEY_RESIZE
    if (dialog != 0)
	dlg_move_window(dialog, height, width, y, x);
    else
#endif
    {
	dialog = dlg_new_window(height, width, y, x);
	dlg_register_window(dialog, "yesno", binding);
	dlg_register_buttons(dialog, "yesno", buttons);
    }

    dlg_draw_box(dialog, 0, 0, height, width, dialog_attr, border_attr);
    dlg_draw_bottom_box(dialog);
    dlg_draw_title(dialog, title);
    dlg_draw_helpline(dialog, FALSE);

    wattrset(dialog, dialog_attr);

    page = height - (1 + 3 * MARGIN);
    dlg_draw_buttons(dialog,
		     height - 2 * MARGIN, 0,
		     buttons, button, FALSE, width);

    while (result == DLG_EXIT_UNKNOWN) {
	if (show) {
	    last = dlg_print_scrolled(dialog, prompt, offset,
				      page, width, TRUE);
	    show = FALSE;
	}
	key = dlg_mouse_wgetch(dialog, &fkey);
	if (dlg_result_key(key, fkey, &result))
	    break;
	if ((code = dlg_char_to_button(key, buttons)) >= 0) {
	    result = dlg_ok_buttoncode(code);
	    break;
	}
	/* handle function keys */
	if (fkey) {
	    switch (key) {
	    case DLGK_FIELD_NEXT:
		button = dlg_next_button(buttons, button);
		if (button < 0)
		    button = 0;
		dlg_draw_buttons(dialog,
				 height - 2, 0,
				 buttons, button,
				 FALSE, width);
		break;
	    case DLGK_FIELD_PREV:
		button = dlg_prev_button(buttons, button);
		if (button < 0)
		    button = 0;
		dlg_draw_buttons(dialog,
				 height - 2, 0,
				 buttons, button,
				 FALSE, width);
		break;
	    case DLGK_ENTER:
		result = dlg_yes_buttoncode(button);
		break;
#ifdef KEY_RESIZE
	    case KEY_RESIZE:
		dlg_clear();
		height = req_high;
		width = req_wide;
		goto restart;
#endif
	    default:
		if (is_DLGK_MOUSE(key)) {
		    result = dlg_yes_buttoncode(key - M_EVENT);
		    if (result < 0)
			result = DLG_EXIT_OK;
		} else if (dlg_check_scrolled(key, last, page,
					      &show, &offset) != 0) {
		    beep();
		}
		break;
	    }
	} else {
	    beep();
	}
    }

    dlg_del_window(dialog);
    dlg_mouse_free_regions();
    free(prompt);
    return result;
}