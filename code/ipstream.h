#ifndef _fcgi_ipstream_h__
#define _fcgi_ipstream_h__

/* Copyright(c) Andre Caron (andre.l.caron@gmail.com), 2011
**
** This document is covered by the an Open Source Initiative approved license. A
** copy of the license should have been provided alongside this software package
** (see "LICENSE.txt"). If not, terms of the license are available online at
** "http://www.opensource.org/licenses/mit". */

/*!
 * @file ipstream.h
 * @author Andre Caron (andre.l.caron@gmail.com)
 * @brief Incremental parser for the FastCGI parameter stream.
 */

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct fcgi_pstream_t;

typedef size_t(*fcgi_ipstream_state)
    (struct fcgi_ipstream_t*,const char*,size_t);

typedef struct fcgi_ipstream_t
{
    void * object;

    void(*accept)(struct fcgi_ipstream_t*,size_t,size_t);
    void(*accept_name)(struct fcgi_ipstream_t*,const char *,size_t);
    void(*finish_name)(struct fcgi_ipstream_t*);
    void(*accept_data)(struct fcgi_ipstream_t*,const char *,size_t);
    void(*finish_data)(struct fcgi_ipstream_t*);
    void(*finish)(struct fcgi_ipstream_t*);

    fcgi_ipstream_state state;

    size_t nsize;
    size_t dsize;
    size_t npass;
    size_t dpass;

    size_t staged;
    char staging[4];

} fcgi_ipstream;

  /*!
   * @brief Initialize a new name-value pstream parser.
   */
void fcgi_ipstream_init ( fcgi_ipstream * stream );

  /*!
   * @brief Clear errors and reset the parser state.
   *
   * This function does not clear the @c object and callback fields.
   */
void fcgi_ipstream_clear ( fcgi_ipstream * stream );

  /*!
   * @brief Feed data to the parser.
   * @param stream Name-value pstream parser.
   * @param data Pointer to first byte of data.
   * @param size Size of @a data, in bytes.
   * @return Number of bytes consumed.  Normally, this value is equal to
   *  @a size.  However, the parser may choose to interrupt parser early or stop
   *  processing data because of an error.
   */
size_t fcgi_ipstream_feed ( fcgi_ipstream * stream, const char * data, size_t size );

#ifdef __cplusplus
}
#endif


#endif /* _fcgi_ipstream_h__ */
