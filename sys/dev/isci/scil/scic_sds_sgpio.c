
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

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

/**
* @file
*
* @brief This file contains the implementation of the SGPIO register inteface
*        methods.
*/

#include <dev/isci/scil/scic_sgpio.h>
#include <dev/isci/scil/scic_sds_controller_registers.h>
#include <dev/isci/scil/scic_user_callback.h>

/**
 * @brief Function writes Value to the
 *        SGPIO Output Data Select Register (SGODSR) for phys specified by
 *        phy_mask paremeter
 *
 * @param[in] SCIC_SDS_CONTROLLER_T controller
 * @param[in] phy_mask - This field is a bit mask that specifies the phys
 *                       to be updated.
 * @param[in] value - Value for write
 *
 * @return none
 */
static
void scic_sgpio_write_SGODSR_register(
   SCIC_SDS_CONTROLLER_T *controller,
   U32 phy_mask,
   U32 value
)
{
   U8 phy_index;

   for (phy_index = 0; phy_index < SCI_MAX_PHYS; phy_index++)
   {
      if (phy_mask >> phy_index & 1)
      {
          scu_sgpio_peg0_register_write(
             controller, output_data_select[phy_index], value
          );
      }
   }
}

void scic_sgpio_set_vendor_code(
   SCI_CONTROLLER_HANDLE_T controller,
   U8 vendor_specific_sequence
)
{
   SCIC_SDS_CONTROLLER_T * core_controller = (SCIC_SDS_CONTROLLER_T *) controller;

   scu_sgpio_peg0_register_write(
      core_controller, vendor_specific_code, vendor_specific_sequence);
}

void scic_sgpio_set_blink_patterns(
   SCI_CONTROLLER_HANDLE_T controller,
   U8 pattern_a_low,
   U8 pattern_a_high,
   U8 pattern_b_low,
   U8 pattern_b_high
)
{
   U32 value;
   SCIC_SDS_CONTROLLER_T * core_controller = (SCIC_SDS_CONTROLLER_T *) controller;

   value = (pattern_b_high << 12) + (pattern_b_low << 8) + (pattern_a_high << 4) + pattern_a_low;

   scu_sgpio_peg0_register_write(
      core_controller, blink_rate, value);
}

void scic_sgpio_set_functionality(
   SCI_CONTROLLER_HANDLE_T controller,
   BOOL sgpio_mode
)
{
   U32 value = DISABLE_SGPIO_FUNCTIONALITY;
   SCIC_SDS_CONTROLLER_T * core_controller = (SCIC_SDS_CONTROLLER_T *) controller;

   if(sgpio_mode)
   {
      value = ENABLE_SGPIO_FUNCTIONALITY;
   }

   scu_sgpio_peg0_register_write(
      core_controller, interface_control, value);
}

void scic_sgpio_apply_led_blink_pattern(
   SCI_CONTROLLER_HANDLE_T controller,
   U32 phy_mask,
   BOOL error,
   BOOL locate,
   BOOL activity,
   U8 pattern_selection
)
{
   U32 output_value = 0;

   SCIC_SDS_CONTROLLER_T * core_controller = (SCIC_SDS_CONTROLLER_T *) controller;

   // Start with all LEDs turned off
   output_value = (SGODSR_INVERT_BIT <<  SGODSR_ERROR_LED_SHIFT)
                     | (SGODSR_INVERT_BIT <<  SGODSR_LOCATE_LED_SHIFT)
                     | (SGODSR_INVERT_BIT << SGODSR_ACTIVITY_LED_SHIFT);

   if(error)
   {  //apply pattern to error LED
      output_value |= pattern_selection << SGODSR_ERROR_LED_SHIFT;
      output_value &= ~(SGODSR_INVERT_BIT <<  SGODSR_ERROR_LED_SHIFT);
   }
   if(locate)
   {  //apply pattern to locate LED
      output_value |= pattern_selection << SGODSR_LOCATE_LED_SHIFT;
      output_value &= ~(SGODSR_INVERT_BIT <<  SGODSR_LOCATE_LED_SHIFT);
   }
   if(activity)
   {  //apply pattern to activity LED
      output_value |= pattern_selection << SGODSR_ACTIVITY_LED_SHIFT;
      output_value &= ~(SGODSR_INVERT_BIT << SGODSR_ACTIVITY_LED_SHIFT);
   }

   scic_sgpio_write_SGODSR_register(core_controller, phy_mask, output_value);
}

void scic_sgpio_set_led_blink_pattern(
   SCI_CONTROLLER_HANDLE_T controller,
   SCI_PORT_HANDLE_T port_handle,
   BOOL error,
   BOOL locate,
   BOOL activity,
   U8 pattern_selection
)
{
   U32 phy_mask;

   SCIC_SDS_PORT_T * port = (SCIC_SDS_PORT_T *) port_handle;

   phy_mask = scic_sds_port_get_phys(port);

   scic_sgpio_apply_led_blink_pattern(
           controller, phy_mask, error, locate, activity, pattern_selection);
}

void scic_sgpio_update_led_state(
   SCI_CONTROLLER_HANDLE_T controller,
   U32 phy_mask,
   BOOL error,
   BOOL locate,
   BOOL activity
)
{
   U32 output_value;

   SCIC_SDS_CONTROLLER_T * core_controller = (SCIC_SDS_CONTROLLER_T *) controller;

   // Start with all LEDs turned on
   output_value = 0x00000000;

   if(!error)
   {  //turn off error LED
      output_value |= SGODSR_INVERT_BIT <<  SGODSR_ERROR_LED_SHIFT;
   }
   if(!locate)
   {  //turn off locate LED
      output_value |= SGODSR_INVERT_BIT <<  SGODSR_LOCATE_LED_SHIFT;
   }
   if(!activity)
   {  //turn off activity LED
      output_value |= SGODSR_INVERT_BIT <<  SGODSR_ACTIVITY_LED_SHIFT;
   }

   scic_sgpio_write_SGODSR_register(core_controller, phy_mask, output_value);
}

void scic_sgpio_set_led_state(
   SCI_CONTROLLER_HANDLE_T controller,
   SCI_PORT_HANDLE_T port_handle,
   BOOL error,
   BOOL locate,
   BOOL activity
)
{
   U32 phy_mask;

   SCIC_SDS_PORT_T * port = (SCIC_SDS_PORT_T *) port_handle;

   phy_mask = scic_sds_port_get_phys(port);

   scic_sgpio_update_led_state(controller, phy_mask, error, locate, activity);
}

void scic_sgpio_set_to_hardware_control(
   SCI_CONTROLLER_HANDLE_T controller,
   BOOL is_hardware_controlled
)
{
   SCIC_SDS_CONTROLLER_T * core_controller = (SCIC_SDS_CONTROLLER_T *) controller;
   U8 i;
   U32 output_value;

   //turn on hardware control for LED's
   if(is_hardware_controlled)
   {
      output_value = SGPIO_HARDWARE_CONTROL;
   }
   else //turn off hardware control
   {
      output_value = SGPIO_SOFTWARE_CONTROL;
   }

   for(i = 0; i < SCI_MAX_PHYS; i++)
   {
      scu_sgpio_peg0_register_write(
         core_controller, output_data_select[i], output_value);
   }
}

U32 scic_sgpio_read(
   SCI_CONTROLLER_HANDLE_T controller
)
{
   //Not supported in the SCU hardware returning 0xFFFFFFFF
   return 0xffffffff;
}

void scic_sgpio_hardware_initialize(
   SCI_CONTROLLER_HANDLE_T controller
)
{
   scic_sgpio_set_functionality(controller, TRUE);
   scic_sgpio_set_to_hardware_control(controller, TRUE);
   scic_sgpio_set_vendor_code(controller, 0x00);
}

void scic_sgpio_initialize(
   SCI_CONTROLLER_HANDLE_T controller
)
{
   scic_sgpio_set_functionality(controller, TRUE);
   scic_sgpio_set_to_hardware_control(controller, FALSE);
   scic_sgpio_set_vendor_code(controller, 0x00);
}