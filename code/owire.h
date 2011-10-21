#ifndef _fcgi_owire_h__
#define _fcgi_owire_h__

/* Copyright(c) Andre Caron (andre.l.caron@gmail.com), 2011
**
** This document is covered by the an Open Source Initiative approved license. A
** copy of the license should have been provided alongside this software package
** (see "LICENSE.txt"). If not, terms of the license are available online at
** "http://www.opensource.org/licenses/mit". */

/*!
 * @file owire.h
 * @author Andre Caron (andre.l.caron@gmail.com)
 * @brief Incremental writer for FastCGI out-bound traffic.
 */

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

  /*!
   * @brief Enumeration of error states the writer may report.
   */
typedef enum fcgi_owire_error_t
{
    fcgi_owire_error_none = 0,

} fcgi_owire_error;

  /*!
   * @brief Gets a human-readable description of the error.
   */
const char * fcgi_owire_error_message ( fcgi_owire_error error );

  /*!
   * @brief Customizable limits for FastCGI wire protocol.
   */
typedef struct fcgi_owire_settings_t
{
    int dummy;

} fcgi_owire_settings;

  /*!
   * @brief FastCGI writer state.
   *
   * The writer is implemented as a Finite State Machine (FSM).  By itself, it
   * does not buffer any data.  As soon as the syntax is validated, all content
   * is forwarded to the client code through the callbacks.
   */
typedef struct fcgi_owire_t
{
      /*! @public
       * @brief Last error reported by the writer.
       *
       * @warning This field should only be interpreted if @c state is set to
       *  @c fcgi_parsing_failed.  Its value is undefined at all other times.
       */
    fcgi_owire_error error;

    const fcgi_owire_settings * settings;

      /*! @public
       * @brief Extra field for client code's use.
       *
       * The contents of this field are not interpreted in any way by the
       * netstring request writer.  It is used for any purpose by client code.
       * Usually, this field serves as link back to the owner object and used by
       * registered callbacks.
       */
    void * object;

      /*!
       * @brief Callback used to write data to output stream.
       */
    void(*write_stream)(struct fcgi_owire_t*, const char *, size_t);

      /*!
       * @brief Callback used to flush output stream buffers.
       */
    void(*flush_stream)(struct fcgi_owire_t*);

} fcgi_owire;

  /*!
   * @brief Initialize a new parser.
   */
void fcgi_owire_init
    ( const fcgi_owire_settings * settings, fcgi_owire * stream );

  /*!
   * @group gateway
   * @brief Reserve a request ID.
   */
size_t fcgi_owire_new_request
    ( fcgi_owire * stream, uint16_t request, uint16_t role );

  /*!
   * @ingroup gateway
   * @brief Cancel a request and release its request ID.
   */
size_t fcgi_owire_bad_request
    ( fcgi_owire * stream, uint16_t request );

  /*!
   * @ingroup application
   * @brief Release a request ID.
   */
size_t fcgi_owire_end_request ( fcgi_owire * stream,
    uint16_t request, uint32_t astatus, uint8_t pstatus );

  /*!
   * @ingroup gateway
   * @brief Send headers to the application.
   */
size_t fcgi_owire_param
    ( fcgi_owire * stream, uint16_t request, const char * data, size_t size );

  /*!
   * @ingroup gateway
   * @brief Send the application data it should receive on the standard input.
   */
size_t fcgi_owire_stdi
    ( fcgi_owire * stream, uint16_t request, const char * data, size_t size );

  /*!
   * @ingroup application
   * @brief Send the gateway data it should receive on the standard input.
   */
size_t fcgi_owire_stdo
    ( fcgi_owire * stream, uint16_t request, const char * data, size_t size );

  /*!
   * @ingroup application
   * @brief Send the gateway data it should receive on the standard input.
   */
size_t fcgi_owire_stde
    ( fcgi_owire * stream, uint16_t request, const char * data, size_t size );

  /*!
   * @ingroup gateway
   * @brief Send the application data it should receive on the extra input.
   */
size_t fcgi_owire_extra
    ( fcgi_owire * stream, uint16_t request, const char * data, size_t size );

  /*!
   * @ingroup gateway
   * @brief Request information from the application.
   */
size_t fcgi_owire_query
    ( fcgi_owire * stream, uint16_t request, const char * data, uint16_t size );

  /*!
   * @ingroup application
   * @brief Send gateway a responsea to a request.
   */
size_t fcgi_owire_reply
    ( fcgi_owire * stream, uint16_t request, const char * data, uint16_t size );

#ifdef __cplusplus
}
#endif

#endif /* _fcgi_owire_h__ */
