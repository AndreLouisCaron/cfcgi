#ifndef _fcgi_h__
#define _fcgi_h__

/* Copyright(c) Andre Caron <andre.l.caron@gmail.com>, 2011
**
** This document is covered by the an Open Source Initiative approved license. A
** copy of the license should have been provided alongside this software package
** (see "LICENSE.txt"). If not, terms of the license are available online at
** "http://www.opensource.org/licenses/mit". */

/*!
 * @file fcgi.h
 * @author Andre Caron <andre.l.caron@gmail.com>
 * @brief Parser for FastCGI records.
 */

typedef unsigned char uint8_t;
typedef unsigned int uint32_t;

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

  /*!
   * @brief Enumeration of error states the parser may report.
   */
enum fcgi_parser_error
{
    fcgi_parse_error_none=0,
};

  /*!
   * @brief Gets a human-readable description of the error.
   */
const char * fcgi_error_message ( enum fcgi_parser_error error );

  /*!
   * @brief Enumeration of parser states.
   */
enum fcgi_parser_state
{
    fcgi_parse_record_fail=-2,
    fcgi_parse_record_skip=-1,
    fcgi_parse_record_idle= 0,
    fcgi_parse_record_head= 1, /* FCGI_BEGIN_REQUEST */
    fcgi_parse_record_bail= 2, /* FCGI_ABORT_REQUEST */
    fcgi_parse_record_done= 3, /* FCGI_END_REQUEST */
    fcgi_parse_record_meta= 4, /* FCGI_PARAMS */
    fcgi_parse_record_stdi= 5, /* FCGI_STDIN */
    fcgi_parse_record_stdo= 6, /* FCGI_STDOUT */
    fcgi_parse_record_stde= 7, /* FCGI_STDERR */
    fcgi_parse_record_data= 8, /* FCGI_DATA */
    fcgi_parse_record_pull= 9, /* FCGI_GET_VALUES */
    fcgi_parse_record_push=10, /* FCGI_GET_VALUES_RESULT */
};

  /*!
   * @brief Customizable limits for netstrings.
   */
struct fcgi_limits
{
    int dummy;
};

  /*!
   * @brief FastCGI parser state.
   *
   * The parser is implemented as a Finite State Machine (FSM).  By itself, it
   * does not buffer any data.  As soon as the syntax is validated, all content
   * is forwarded to the client code through the callbacks.
   */
struct fcgi_parser
{
      /*! @public
       * @brief Current state of the parser.
       *
       * Client code should check the state after each call to
       * @c fcgi_consume() to check for important state transitions.
       *
       * @warning This field is provided to clients as read-only.  Any attempt
       *  to change it will cause unpredictable output.
       */
    enum fcgi_parser_state state;

      /*! @public
       * @brief Last error reported by the parser.
       *
       * @warning This field should only be interpreted if @c state is set to
       *  @c fcgi_parsing_failed.  Its value is undefined at all other times.
       */
    enum fcgi_parser_error error;

      /*! @public
       * @brief Extra field for client code's use.
       *
       * The contents of this field are not interpreted in any way by the
       * netstring request parser.  It is used for any purpose by client code.
       * Usually, this field serves as link back to the owner object and used by
       * registered callbacks.
       */
    void * object;

    /* generic markers for begin/end of records. */
    void(*accept_record)(struct fcgi_parser*, int, int, int);
      // FastCGI Version, Request ID, Content Length.
    void(*finish_record)(struct fcgi_parser*);

    /* FCGI_BEGIN_REQUEST. */
    void(*accept_request)(struct fcgi_parser*, int, int);
    // Role, Flags.

    /* FCGI_ABORT_REQUEST. */
    void(*cancel_request)(struct fcgi_parser*);

    /* FCGI_END_REQUEST */
    void(*finish_request)(struct fcgi_parser*, uint32_t, int);

    /* FCGI_PARAMS */
    void(*accept_param_name)(struct fcgi_parser*, const char *, size_t);
    void(*accept_param_data)(struct fcgi_parser*, const char *, size_t);

    /* FCGI_GET_VALUES */
    void(*accept_query_name)(struct fcgi_parser*, const char *, size_t);
    void(*accept_query_data)(struct fcgi_parser*, const char *, size_t);

    /* FCGI_GET_VALUES_RESULT */
    void(*accept_reply_name)(struct fcgi_parser*, const char *, size_t);
    void(*accept_reply_data)(struct fcgi_parser*, const char *, size_t);

    /* FCGI_STDIN */
    void(*accept_content_stdi)(struct fcgi_parser*, const char *, size_t);

    /* FCGI_STDOUT */
    void(*accept_content_stdo)(struct fcgi_parser*, const char *, size_t);

    /* FCGI_STDERR */
    void(*accept_content_stde)(struct fcgi_parser*, const char *, size_t);

    /* FCGI_DATA */
    void(*accept_content_data)(struct fcgi_parser*, const char *, size_t);

    /*! @private
     * @brief Number of bytes to skip until start of next record.
     * @invariant in [0, 2^8).
     */
    size_t skip;

    /*! @private
     * @brief Number of bytes of content left to forward.
     * @invariant in [0, 2^16)
     */
    size_t size;

    /*! @private
     * @brief Small staging area for parsing multi-byte values.
     * @invariant bytes [0, size) are used, bytes [size,2^8) are free.
     */
    char staging[8];

    /*! @private
     * @brief Number of valid bytes in @c staging.
     * @invariant in [0, 8).
     */
    size_t staged;

    /*! @private
     * @brief Number of bytes left to forward in parameter name.
     */
    uint32_t ksize;

    /*! @private
     * @brief Number of bytes left to forward in parameter value.
     */
    uint32_t vsize;
};

  /*!
   * @brief Initialize a parser and its limits.
   */
void fcgi_setup
    ( struct fcgi_limits * limits, struct fcgi_parser * parser );

  /*!
   * @brief Clear errors and reset the parser state.
   *
   * This function does not clear the @c object and callback fields.  You may
   * call it to re-use any parsing context, such as allocated buffers for
   * string contents.
   */
void fcgi_clear ( struct fcgi_parser * parser );

  /*!
   * @brief Feed data to the parser.
   * @param data Pointer to first byte of data.
   * @param size Size of @a data, in bytes.
   * @return Number of bytes consumed.  Normally, this value is equal to
   *  @a size.  However, the parser may choose to interrupt parser early or stop
   *  processing data because of an error.
   *
   * You should @e always check the parser state after a call to this method.
   * In particular, all data may be consumed before an error is reported, so
   * a return value equal to @a size is not a reliable indicator of success.
   */
size_t fcgi_consume ( const struct fcgi_limits * limits,
    struct fcgi_parser * parser, const char * data, size_t size );

#ifdef __cplusplus
}
#endif

#endif /* _fcgi_h__ */
