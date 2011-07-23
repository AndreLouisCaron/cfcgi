/* Copyright(c) Andre Caron <andre.l.caron@gmail.com>, 2011
**
** This document is covered by the an Open Source Initiative approved license. A
** copy of the license should have been provided alongside this software package
** (see "LICENSE.txt"). If not, terms of the license are available online at
** "http://www.opensource.org/licenses/mit". */

/*!
 * @file fcgi.c
 * @author Andre Caron <andre.l.caron@gmail.com>
 * @brief Parser for FastCGI records.
 */

#include "fcgi.h"
#include <string.h>

static const char * fcgi_error_messages[] =
{
    "no error, parser ok",
};

static size_t fcgi_min ( size_t a, size_t b )
{
    return ((a < b)? a : b);
}

const char * fcgi_error_message ( enum fcgi_parser_error error )
{
    return (fcgi_error_messages[error]);
}

static size_t fcgi_copy ( char * lhs, const char * rhs, size_t n )
{
    size_t i = 0;
    while ( i < n ) {
        lhs[i] = rhs[i]; ++i;
    }
    return (i);
}

static size_t fcgi_stage_buffer
    ( struct fcgi_parser * parser, const char * data, size_t size )
{
      /* copy to staging area until header is complete. */
    size_t used = fcgi_copy(
        parser->staging+parser->staged,
        data, fcgi_min(8-parser->staged, size));
      /* update cursors. */
    parser->staged += used;
    return (used);
}

static size_t fcgi_parse_header
    ( struct fcgi_parser * parser, const char * data, size_t size )
{
      /* copy to staging area until header is complete. */
    size_t used = fcgi_stage_buffer(parser, data, size);
      /* process body, if complete. */
    if ( parser->staged == 8 )
    {
          /* parse fields. */
        int version =
            (int)(unsigned char)parser->staging[0];
        int reqtype =
            (int)(unsigned char)parser->staging[1];
        int request =
            (int)(unsigned char)parser->staging[2] << 8|
            (int)(unsigned char)parser->staging[3] << 0;
        parser->size =
            (int)(unsigned char)parser->staging[4] << 8|
            (int)(unsigned char)parser->staging[5] << 0;
        parser->skip =
            (int)(unsigned char)parser->staging[6];
          /* new parser state depends on record type. */
        if ((reqtype < 0) || (reqtype > 10)) {
            parser->state = fcgi_parse_record_fail; return (used);
        }
          /* forward fields. */
        parser->accept_record(parser, version, request, parser->size);
          /* ditch staged data. */
        parser->staged = 0;
          /* pass on to new state. */
        parser->state = (enum fcgi_parser_state)reqtype;
    }
    return (used);
}

static size_t fcgi_skip_padding
    ( struct fcgi_parser * parser, const char * data, size_t size )
{
      /* consume as much data as possible. */
    size_t used = fcgi_min(parser->skip, size);
    parser->skip -= used;
      /* signal end of record and reset. */
    if ( parser->skip == 0 ) {
        parser->finish_record(parser);
        parser->skip = 0;
        parser->size = 0;
        parser->staged = 0;
        parser->state = fcgi_parse_record_idle;
    }
    return (used);
}

typedef void(*accept_stuff)(struct fcgi_parser*, const char *, size_t);

static size_t fcgi_forward_stuff_name ( struct fcgi_parser * parser,
    const char * data, size_t size, accept_stuff accept_name )
{
       /* note: 'parser->size' is updated by caller. */
    size_t used = fcgi_min(parser->ksize, size);
    accept_name(parser, data, used);
    parser->ksize -= used;
    return (used);
}

static size_t fcgi_forward_stuff_data ( struct fcgi_parser * parser,
    const char * data, size_t size, accept_stuff accept_data )
{
       /* note: 'parser->size' is updated by caller. */
    size_t used = fcgi_min(parser->vsize, size);
    accept_data(parser, data, used);
    parser->vsize -= used;
    return (used);
}

static size_t fcgi_accept_stuff (
    struct fcgi_parser * parser, const char * data, size_t size,
    accept_stuff accept_name, accept_stuff accept_data )
{
    int n = 0;
    size_t used = 0;
      /* read prefixed lengths. */
    while ((size > 0) && (parser->staged < 8))
    {
          /* when length < 127, length is only one byte. */
        if ( parser->staged == 0 )
        {
            n = (int)data[used++];
            parser->staging[0] = n;
            if ( n <= 127 ) {
                parser->staging[1] = 0;
                parser->staging[2] = 0;
                parser->staging[3] = 0;
                parser->staged = 4;
            }
        }
        if ( parser->staged < 4 ) {
            parser->staging[parser->staged++] = data[used++];
        }
          /* when length < 127, length is only one byte. */
        if ( parser->staged == 4 )
        {
            n = (int)data[used++];
            parser->staging[4] = n;
            if ( n <= 127 ) {
                parser->staging[5] = 0;
                parser->staging[6] = 0;
                parser->staging[7] = 0;
                parser->staged = 8;
            }
        }
        if ( parser->staged < 8 ) {
            parser->staging[parser->staged++] = data[used++];
        }
    }
      /* after reading lengths, interpret content. */
    if ((parser->staged == 8) && (parser->ksize == 0) && (parser->vsize == 0))
    {
        parser->ksize =
            (uint32_t)(unsigned char)parser->staging[0] <<  0|
            (uint32_t)(unsigned char)parser->staging[1] <<  8|
            (uint32_t)(unsigned char)parser->staging[2] << 16|
            (uint32_t)(unsigned char)parser->staging[3] << 24;
        parser->vsize =
            (uint32_t)(unsigned char)parser->staging[4] <<  0|
            (uint32_t)(unsigned char)parser->staging[5] <<  8|
            (uint32_t)(unsigned char)parser->staging[6] << 16|
            (uint32_t)(unsigned char)parser->staging[7] << 24;
    }
      /* forward trailing data. */
    while ((parser->staged == 8) && (used < size) &&
        ((parser->ksize > 0) || (parser->vsize > 0)))
    {
        if (parser->ksize > 0) {
            used += fcgi_forward_stuff_name(
                parser, data+used, size-used, accept_name);
        }
        else if (parser->vsize > 0) {
            used += fcgi_forward_stuff_data(
                parser, data+used, size-used, accept_data);
        }
    }
      /* update parser state. */
    parser->size -= used;
    if ( parser->size == 0 ) {
        parser->state = fcgi_parse_record_skip;
    }
    return (used);
}

typedef size_t(*fcgi_request_handler)(struct fcgi_parser*,const char*,size_t);

static size_t FCGI_BEGIN_REQUEST
    ( struct fcgi_parser * parser, const char * data, size_t size )
{
      /* copy to staging area until header is complete. */
    size_t used = fcgi_stage_buffer(parser, data, size);
      /* process body, if complete. */
    if ( parser->staged == 8 )
    {
          /* parse fields. */
        int role =
            (int)(unsigned char)parser->staging[0] << 8|
            (int)(unsigned char)parser->staging[1] << 0;
        int flags =
            (int)(unsigned char)parser->staging[2];
          /* consume staging area. */
        parser->staged = 0;
          /* forward request. */
        parser->accept_request(parser, role, flags);
          /* start skipping padding. */
        parser->state = fcgi_parse_record_skip;
    }
    return (used);
}

static size_t FCGI_ABORT_REQUEST
    ( struct fcgi_parser * parser, const char * data, size_t size )
{
      /* signal abortion. */
    parser->cancel_request(parser);
      /* start skipping padding. */
    parser->state = fcgi_parse_record_skip;
    return (0);
}

static size_t FCGI_END_REQUEST
    ( struct fcgi_parser * parser, const char * data, size_t size )
{
      /* copy to staging area until header is complete. */
    size_t used = fcgi_stage_buffer(parser, data, size);
      /* process body, if complete. */
    if ( parser->staged == 8 )
    {
          /* parse fields. */
        int astatus =
            (int)(unsigned char)parser->staging[0] << 24|
            (int)(unsigned char)parser->staging[1] << 16|
            (int)(unsigned char)parser->staging[2] <<  8|
            (int)(unsigned char)parser->staging[3] <<  0;
        int pstatus =
            (int)(unsigned char)parser->staging[4];
          /* consume staging area. */
        parser->staged = 0;
          /* forward request. */
        parser->finish_request(parser, astatus, pstatus);
          /* start skipping padding. */
        parser->state = fcgi_parse_record_skip;
    }
    return (used);
}

static size_t FCGI_PARAMS
    ( struct fcgi_parser * parser, const char * data, size_t size )
{
    return (fcgi_accept_stuff(parser, data, size,
        parser->accept_param_name, parser->accept_param_data));
}

static size_t FCGI_GET_VALUES
    ( struct fcgi_parser * parser, const char * data, size_t size )
{
    return (fcgi_accept_stuff(parser, data, size,
        parser->accept_query_name, parser->accept_query_data));
}

static size_t FCGI_GET_VALUES_RESULT
    ( struct fcgi_parser * parser, const char * data, size_t size )
{
    return (fcgi_accept_stuff(parser, data, size,
        parser->accept_reply_name, parser->accept_reply_data));
}

static size_t FCGI_STDIN
    ( struct fcgi_parser * parser, const char * data, size_t size )
{
      /* consume as much data as possible. */
    size_t used = fcgi_min(parser->size, size);
    parser->accept_content_stdi(parser, data, size);
      /* adjust parser state. */
    parser->size -= used;
    if ( parser->size == 0 ) {
        parser->state = fcgi_parse_record_skip;
    }
    return (used);
}

static size_t FCGI_STDOUT
    ( struct fcgi_parser * parser, const char * data, size_t size )
{
      /* consume as much data as possible. */
    size_t used = fcgi_min(parser->size, size);
    parser->accept_content_stdo(parser, data, size);
      /* adjust parser state. */
    parser->size -= used;
    if ( parser->size == 0 ) {
        parser->state = fcgi_parse_record_skip;
    }
    return (used);
}

static size_t FCGI_STDERR
    ( struct fcgi_parser * parser, const char * data, size_t size )
{
      /* consume as much data as possible. */
    size_t used = fcgi_min(parser->size, size);
    parser->accept_content_stde(parser, data, size);
      /* adjust parser state. */
    parser->size -= used;
    if ( parser->size == 0 ) {
        parser->state = fcgi_parse_record_skip;
    }
    return (used);
}

static size_t FCGI_DATA
    ( struct fcgi_parser * parser, const char * data, size_t size )
{
      /* consume as much data as possible. */
    size_t used = fcgi_min(parser->size, size);
    parser->accept_content_data(parser, data, size);
      /* adjust parser state. */
    parser->size -= used;
    if ( parser->size == 0 ) {
        parser->state = fcgi_parse_record_skip;
    }
    return (used);
}

/* function pointers to handle internal state transitions.  order must match
 * state numbers, which must match protocol record types. */
static const fcgi_request_handler fcgi_request_handlers[] =
{
      /* note: off by one. */
    FCGI_BEGIN_REQUEST,
    FCGI_ABORT_REQUEST,
    FCGI_END_REQUEST,
    FCGI_PARAMS,
    FCGI_STDIN,
    FCGI_STDOUT,
    FCGI_STDERR,
    FCGI_DATA,
    FCGI_GET_VALUES,
    FCGI_GET_VALUES_RESULT,
};

void fcgi_setup
    ( struct fcgi_limits * limits, struct fcgi_parser * parser )
{
      /* public members. */
    parser->state = fcgi_parse_record_idle;
    parser->error = fcgi_parse_error_none;
    parser->object = 0;
      /* callbacks. */
    parser->accept_record = 0;
    parser->finish_record = 0;
    parser->accept_request = 0;
    parser->accept_request = 0;
    parser->cancel_request = 0;
    parser->finish_request = 0;
    parser->accept_param_name = 0;
    parser->accept_param_data = 0;
    parser->accept_query_name = 0;
    parser->accept_query_data = 0;
    parser->accept_reply_name = 0;
    parser->accept_reply_data = 0;
    parser->accept_content_stdi = 0;
    parser->accept_content_stdo = 0;
    parser->accept_content_stde = 0;
    parser->accept_content_data = 0;
      /* secret members. */
    parser->size = 0;
    parser->skip = 0;
    parser->staged = 0;
    parser->ksize = 0;
    parser->vsize = 0;
}

void fcgi_clear ( struct fcgi_parser * parser )
{
      /* public members. */
    parser->state = fcgi_parse_record_idle;
    parser->error = fcgi_parse_error_none;
      /* secret members. */
    parser->size = 0;
    parser->skip = 0;
    parser->staged = 0;
    parser->ksize = 0;
    parser->vsize = 0;
}

size_t fcgi_consume ( const struct fcgi_limits * limits,
    struct fcgi_parser * parser, const char * data, size_t size )
{
    size_t used = 0;
      /* data might contain more than one request. */
    while ((used < size) && (parser->state != fcgi_parse_record_fail))
    {
          /* record head. */
        if ( parser->state == fcgi_parse_record_idle )
        {
            used += fcgi_parse_header(parser, data+used, size-used);
        }
          /* record body. */
        if ((parser->state >= fcgi_parse_record_head) &&
            (parser->state <= fcgi_parse_record_push))
        {
            used += fcgi_request_handlers
                [parser->state-1](parser, data+used, size-used);
        }
           /* record pads. */
        if ( parser->state == fcgi_parse_record_skip )
        {
            used += fcgi_skip_padding(parser, data+used, size-used);
        }
    }
    return (used);
}
