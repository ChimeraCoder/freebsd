
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

#include <dev/isci/isci.h>

#include <dev/isci/scil/scif_user_callback.h>
#include <dev/isci/scil/scic_user_callback.h>
#include <dev/isci/scil/sci_logger.h>

#include <machine/stdarg.h>
#include <sys/time.h>

#define ERROR_LEVEL	0
#define WARNING_LEVEL	1
#define TRACE_LEVEL	2
#define INFO_LEVEL	3

void
isci_log_message(uint32_t verbosity, char *log_message_prefix,
    char *log_message, ...)
{
	va_list argp;
	char buffer[512];
	struct timeval tv;

	if (verbosity > g_isci_debug_level)
		return;

	va_start (argp, log_message);
	vsnprintf(buffer, sizeof(buffer)-1, log_message, argp);
	va_end(argp);
	microtime(&tv);

	printf("isci: %d:%06d %s %s", (int)tv.tv_sec, (int)tv.tv_usec,
	    log_message_prefix, buffer);
}


#ifdef SCI_LOGGING
#define SCI_ENABLE_LOGGING_ERROR	1
#define SCI_ENABLE_LOGGING_WARNING	1
#define SCI_ENABLE_LOGGING_INFO		1
#define SCI_ENABLE_LOGGING_TRACE	1
#define SCI_ENABLE_LOGGING_STATES	1

#define ISCI_LOG_MESSAGE(			\
	logger_object,				\
	log_object_mask,			\
	log_message,				\
	verbosity,				\
	log_message_prefix			\
)						\
{						\
	va_list argp;				\
	char buffer[512];			\
						\
	if (!sci_logger_is_enabled(logger_object, log_object_mask, verbosity)) \
		return;				\
						\
	va_start (argp, log_message);		\
	vsnprintf(buffer, sizeof(buffer)-1, log_message, argp); \
	va_end(argp);				\
						\
	/* prepend the "object:verbosity_level:" */ \
	isci_log_message(verbosity, log_message_prefix, buffer); \
}
#endif /* SCI_LOGGING */


#ifdef SCI_ENABLE_LOGGING_ERROR
/**
 * @brief In this method the user is expected to log the supplied
 *        error information.  The user must be capable of handling variable
 *        length argument lists and should consider prepending the fact
 *        that this is an error from the framework.
 *
 * @param[in]  logger_object This parameter specifies the logger object
 *             associated with this message.
 * @param[in]  log_object_mask This parameter specifies the log objects
 *             for which this message is being generated.
 * @param[in]  log_message This parameter specifies the message to be logged.
 *
 * @return none
 */
void scif_cb_logger_log_error(SCI_LOGGER_HANDLE_T logger_object,
    uint32_t log_object_mask, char *log_message, ...)
{

	ISCI_LOG_MESSAGE(logger_object, log_object_mask, log_message,
	    SCI_LOG_VERBOSITY_ERROR, "FRAMEWORK: ERROR: ");
}
#endif

#ifdef SCI_ENABLE_LOGGING_WARNING
/**
 * @brief In this method the user is expected to log the supplied warning
 *        information.  The user must be capable of handling variable
 *        length argument lists and should consider prepending the fact
 *        that this is a warning from the framework.
 *
 * @param[in]  logger_object This parameter specifies the logger object
 *             associated with this message.
 * @param[in]  log_object_mask This parameter specifies the log objects
 *             for which this message is being generated.
 * @param[in]  log_message This parameter specifies the message to be logged.
 *
 * @return none
 */
void
scif_cb_logger_log_warning(SCI_LOGGER_HANDLE_T logger_object,
    uint32_t log_object_mask, char *log_message, ...)
{

	ISCI_LOG_MESSAGE(logger_object, log_object_mask, log_message,
	    SCI_LOG_VERBOSITY_WARNING, "FRAMEWORK: WARNING: ");
}
#endif

#ifdef SCI_ENABLE_LOGGING_INFO
/**
 * @brief In this method the user is expected to log the supplied debug
 *        information.  The user must be capable of handling variable
 *        length argument lists and should consider prepending the fact
 *        that this is a debug message from the framework.
 *
 * @param[in]  logger_object This parameter specifies the logger object
 *             associated with this message.
 * @param[in]  log_object_mask This parameter specifies the log objects
 *             for which this message is being generated.
 * @param[in]  log_message This parameter specifies the message to be logged.
 *
 * @return none
 */
void
scif_cb_logger_log_info(SCI_LOGGER_HANDLE_T logger_object,
    uint32_t log_object_mask, char *log_message, ...)
{

	ISCI_LOG_MESSAGE(logger_object, log_object_mask, log_message,
	    SCI_LOG_VERBOSITY_INFO, "FRAMEWORK: INFO: ");
}
#endif

#ifdef SCI_ENABLE_LOGGING_TRACE
/**
 * @brief In this method the user is expected to log the supplied function
 *        trace information.  The user must be capable of handling variable
 *        length argument lists and should consider prepending the fact
 *        that this is a function trace (i.e. entry/exit) message from the
 *        framework.
 *
 * @param[in]  logger_object This parameter specifies the logger object
 *             associated with this message.
 * @param[in]  log_object_mask This parameter specifies the log objects
 *             for which this message is being generated.
 * @param[in]  log_message This parameter specifies the message to be logged.
 *
 * @return none
 */
void
scif_cb_logger_log_trace(SCI_LOGGER_HANDLE_T logger_object,
    uint32_t log_object_mask, char *log_message, ...)
{

	ISCI_LOG_MESSAGE(logger_object, log_object_mask, log_message,
	    SCI_LOG_VERBOSITY_TRACE, "FRAMEWORK: TRACE: ");
}
#endif

#ifdef SCI_ENABLE_LOGGING_STATES
/**
 * @brief In this method the user is expected to log the supplied function
 *        state transition information.  The user must be capable of handling
 *        variable length argument lists and should consider prepending the
 *        fact that this is a function trace (i.e. entry/exit) message from
 *        the framework.
 *
 * @param[in]  logger_object This parameter specifies the logger object
 *             associated with this message.
 * @param[in]  log_object_mask This parameter specifies the log objects
 *             for which this message is being generated.
 * @param[in]  log_message This parameter specifies the message to be logged.
 *
 * @return none
 */
void
scif_cb_logger_log_states(SCI_LOGGER_HANDLE_T logger_object,
    uint32_t log_object_mask, char *log_message, ...)
{

	ISCI_LOG_MESSAGE(logger_object, log_object_mask, log_message,
	    SCI_LOG_VERBOSITY_STATES, "FRAMEWORK: STATE TRANSITION: ");
}
#endif

#ifdef SCI_ENABLE_LOGGING_ERROR
/**
 * @brief In this method the user is expected to log the supplied
 *        error information.  The user must be capable of handling variable
 *        length argument lists and should consider prepending the fact
 *        that this is an error from the core.
 *
 * @param[in]  logger_object This parameter specifies the logger object
 *             associated with this message.
 * @param[in]  log_object_mask This parameter specifies the log objects
 *             for which this message is being generated.
 * @param[in]  log_message This parameter specifies the message to be logged.
 *
 * @return none
 */
void
scic_cb_logger_log_error(SCI_LOGGER_HANDLE_T logger_object,
    uint32_t log_object_mask, char *log_message, ...)
{

	ISCI_LOG_MESSAGE(logger_object, log_object_mask, log_message,
	    SCI_LOG_VERBOSITY_ERROR, "CORE: ERROR: ");
}
#endif

#ifdef SCI_ENABLE_LOGGING_WARNING
/**
 * @brief In this method the user is expected to log the supplied warning
 *        information.  The user must be capable of handling variable
 *        length argument lists and should consider prepending the fact
 *        that this is a warning from the core.
 *
 * @param[in]  logger_object This parameter specifies the logger object
 *             associated with this message.
 * @param[in]  log_object_mask This parameter specifies the log objects
 *             for which this message is being generated.
 * @param[in]  log_message This parameter specifies the message to be logged.
 *
 * @return none
 */
void
scic_cb_logger_log_warning(SCI_LOGGER_HANDLE_T logger_object,
    uint32_t log_object_mask, char *log_message, ...)
{

	ISCI_LOG_MESSAGE(logger_object, log_object_mask, log_message,
	    SCI_LOG_VERBOSITY_WARNING, "CORE: WARNING: ");
}
#endif

#ifdef SCI_ENABLE_LOGGING_INFO
/**
 * @brief In this method the user is expected to log the supplied debug
 *        information.  The user must be capable of handling variable
 *        length argument lists and should consider prepending the fact
 *        that this is a debug message from the core.
 *
 * @param[in]  logger_object This parameter specifies the logger object
 *             associated with this message.
 * @param[in]  log_object_mask This parameter specifies the log objects
 *             for which this message is being generated.
 * @param[in]  log_message This parameter specifies the message to be logged.
 *
 * @return none
 */
void
scic_cb_logger_log_info(SCI_LOGGER_HANDLE_T logger_object,
    uint32_t log_object_mask, char *log_message, ...)
{

	ISCI_LOG_MESSAGE(logger_object, log_object_mask, log_message,
	    SCI_LOG_VERBOSITY_INFO, "CORE: INFO: ");
}
#endif

#ifdef SCI_ENABLE_LOGGING_TRACE
/**
 * @brief In this method the user is expected to log the supplied function
 *        trace information.  The user must be capable of handling variable
 *        length argument lists and should consider prepending the fact
 *        that this is a function trace (i.e. entry/exit) message from the
 *        core.
 *
 * @param[in]  logger_object This parameter specifies the logger object
 *             associated with this message.
 * @param[in]  log_object_mask This parameter specifies the log objects
 *             for which this message is being generated.
 * @param[in]  log_message This parameter specifies the message to be logged.
 *
 * @return none
 */
void
scic_cb_logger_log_trace(SCI_LOGGER_HANDLE_T logger_object,
    uint32_t log_object_mask, char *log_message, ...)
{

	ISCI_LOG_MESSAGE(logger_object, log_object_mask, log_message,
	    SCI_LOG_VERBOSITY_TRACE, "CORE: TRACE: ");
}
#endif

#ifdef SCI_ENABLE_LOGGING_STATES
/**
 * @brief In this method the user is expected to log the supplied function
 *        state transition information.  The user must be capable of handling
 *        variable length argument lists and should consider prepending the
 *        fact that this is a function trace (i.e. entry/exit) message from
 *        the core.
 *
 * @param[in]  logger_object This parameter specifies the logger object
 *             associated with this message.
 * @param[in]  log_object_mask This parameter specifies the log objects
 *             for which this message is being generated.
 * @param[in]  log_message This parameter specifies the message to be logged.
 *
 * @return none
 */
void
scic_cb_logger_log_states(SCI_LOGGER_HANDLE_T logger_object,
    uint32_t log_object_mask, char *log_message, ...)
{

	ISCI_LOG_MESSAGE(logger_object, log_object_mask, log_message,
	    SCI_LOG_VERBOSITY_STATES, "CORE: STATE TRANSITION: ");
}
#endif