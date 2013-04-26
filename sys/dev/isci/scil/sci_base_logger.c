
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
 * @brief This file contains the implementation for the base logger object.
 *        It provides the functionality necessary to enabling, disabling,
 *        constructing, etc. logger objects.
 */


#include <dev/isci/scil/sci_base_logger.h>


#ifdef SCI_LOGGING

//******************************************************************************
//* P R I V A T E   M E T H O D S
//******************************************************************************

#define SCI_LOGGER_GET_OBJECT_MASK(logger, verbosity_level) \
   (((SCI_BASE_LOGGER_T *)(logger))->object_mask[(verbosity_level)])

/**
 * @brief This method is private to this object.  It attempts to enable the
 *        supplied log objects for the supplied verbosity levels.
 *
 * @param[in]  logger This parameter specifies the logger object for which
 *             to attempt to enable log object and verbosity levels.
 * @param[in]  log_object_mask This parameter specifies the log objects to
 *             attempt to enable.
 * @param[in]  verbosity_mask This parameter specifies the verbosity levels
 *             that are allowed to be enabled.
 * @param[in]  verbosity This parameter specifies the specific verbosity level
 *             to attempt to enable.
 *
 * @return none
 */
static
void sci_base_logger_enable_log_object(
   SCI_LOGGER_HANDLE_T  logger,
   U32                  log_object_mask,
   U8                   verbosity_mask,
   U8                   verbosity
)
{
   // Enable the log objects for the error verbosity if errs are enabled.
   if ( (1<<verbosity) & verbosity_mask)
   {
      SCI_LOGGER_GET_OBJECT_MASK(logger, verbosity) |= log_object_mask;
      (((SCI_BASE_LOGGER_T *)(logger))->verbosity_mask |= (1<<verbosity) );
   }
}

/**
 * @brief This method is private to this object.  It attempts to disable the
 *        supplied log objects for the supplied verbosity levels.
 *
 * @param[in]  logger This parameter specifies the logger object for which
 *             to attempt to disable log object and verbosity levels.
 * @param[in]  log_object_mask This parameter specifies the log objects to
 *             attempt to disable.
 * @param[in]  verbosity_mask This parameter specifies the verbosity levels
 *             that are allowed to be disabled.
 * @param[in]  verbosity This parameter specifies the specific verbosity level
 *             to attempt to disable.
 *
 * @return none
 */
static
void sci_base_logger_disable_log_object(
   SCI_LOGGER_HANDLE_T  logger,
   U32                  log_object_mask,
   U8                   verbosity_mask,
   U8                   verbosity
)
{
   if ( (1<<verbosity) & verbosity_mask)
   {
      SCI_LOGGER_GET_OBJECT_MASK(logger, verbosity) &= ~log_object_mask;

      // If all of the objects in the object mask are disabled for this
      // verbosity, then disable the verbosity as well.
      if (SCI_LOGGER_GET_OBJECT_MASK(logger, verbosity) == 0)
         (((SCI_BASE_LOGGER_T *)(logger))->verbosity_mask &= ~(1<<verbosity) );
   }
}

//******************************************************************************
//* P U B L I C   M E T H O D S
//******************************************************************************

U8 sci_logger_get_verbosity_mask(
   SCI_LOGGER_HANDLE_T  logger,
   U32                  log_object
)
{
   U8 verbosity_mask = 0;
   SCI_BASE_LOGGER_T * base_logger = (SCI_BASE_LOGGER_T *)logger;

   if ( base_logger->object_mask[SCI_LOG_VERBOSITY_ERROR] & log_object )
      verbosity_mask |= 1<<SCI_LOG_VERBOSITY_ERROR;

   if ( base_logger->object_mask[SCI_LOG_VERBOSITY_WARNING] & log_object )
      verbosity_mask |= 1<<SCI_LOG_VERBOSITY_WARNING;

   if ( base_logger->object_mask[SCI_LOG_VERBOSITY_INFO] & log_object )
      verbosity_mask |= 1<<SCI_LOG_VERBOSITY_INFO;

   if ( base_logger->object_mask[SCI_LOG_VERBOSITY_TRACE] & log_object )
      verbosity_mask |= 1<<SCI_LOG_VERBOSITY_TRACE;

   if ( base_logger->object_mask[SCI_LOG_VERBOSITY_STATES] & log_object )
      verbosity_mask |= 1<<SCI_LOG_VERBOSITY_TRACE;

   return verbosity_mask;
}

// ---------------------------------------------------------------------------

U32 sci_logger_get_object_mask(
   SCI_LOGGER_HANDLE_T  logger,
   U8                   verbosity
)
{
   // Ensure that a supported verbosity level was supplied.
   if (  (SCI_LOG_VERBOSITY_ERROR == verbosity)
      || (SCI_LOG_VERBOSITY_WARNING == verbosity)
      || (SCI_LOG_VERBOSITY_INFO == verbosity)
      || (SCI_LOG_VERBOSITY_TRACE == verbosity)
      || (SCI_LOG_VERBOSITY_STATES == verbosity) )
   {
      return SCI_LOGGER_GET_OBJECT_MASK(logger, verbosity);
   }

   // An unsupported verbosity level was supplied.  Simply return an empty
   // log object mask.
   return 0;
}

// ---------------------------------------------------------------------------

void sci_logger_enable(
   SCI_LOGGER_HANDLE_T  logger,
   U32                  log_object_mask,
   U8                   verbosity_mask
)
{
   sci_base_logger_enable_log_object(
      logger, log_object_mask, verbosity_mask, SCI_LOG_VERBOSITY_ERROR
   );

   sci_base_logger_enable_log_object(
      logger, log_object_mask, verbosity_mask, SCI_LOG_VERBOSITY_WARNING
   );

   sci_base_logger_enable_log_object(
      logger, log_object_mask, verbosity_mask, SCI_LOG_VERBOSITY_INFO
   );

   sci_base_logger_enable_log_object(
      logger, log_object_mask, verbosity_mask, SCI_LOG_VERBOSITY_TRACE
   );

   sci_base_logger_enable_log_object(
      logger, log_object_mask, verbosity_mask, SCI_LOG_VERBOSITY_STATES
   );
}

// ---------------------------------------------------------------------------

void sci_logger_disable(
   SCI_LOGGER_HANDLE_T  logger,
   U32                  log_object_mask,
   U8                   verbosity_mask
)
{
   sci_base_logger_disable_log_object(
      logger, log_object_mask, verbosity_mask, SCI_LOG_VERBOSITY_ERROR
   );

   sci_base_logger_disable_log_object(
      logger, log_object_mask, verbosity_mask, SCI_LOG_VERBOSITY_WARNING
   );

   sci_base_logger_disable_log_object(
      logger, log_object_mask, verbosity_mask, SCI_LOG_VERBOSITY_INFO
   );

   sci_base_logger_disable_log_object(
      logger, log_object_mask, verbosity_mask, SCI_LOG_VERBOSITY_TRACE
   );

   sci_base_logger_disable_log_object(
      logger, log_object_mask, verbosity_mask, SCI_LOG_VERBOSITY_STATES
   );
}

// ---------------------------------------------------------------------------

BOOL sci_logger_is_enabled(
   SCI_LOGGER_HANDLE_T  logger,
   U32                  log_object_mask,
   U8                   verbosity_level
)
{
   SCI_BASE_LOGGER_T * base_logger = (SCI_BASE_LOGGER_T*) logger;

   if (  (base_logger->verbosity_mask & (1<<verbosity_level))
      && (base_logger->object_mask[verbosity_level] & log_object_mask) )
      return TRUE;

   return FALSE;
}

//******************************************************************************
//* P R O T E C T E D   M E T H O D S
//******************************************************************************

void sci_base_logger_construct(
   SCI_BASE_LOGGER_T *this_logger
)
{
   int index;

   sci_base_object_construct(
      &this_logger->parent, this_logger
   );

   this_logger->verbosity_mask = 0;

   for (index = 0; index < SCI_BASE_LOGGER_MAX_VERBOSITY_LEVELS; index++)
   {
      this_logger->object_mask[index] = 0;
   }
}

#endif // SCI_LOGGING