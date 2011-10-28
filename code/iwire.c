/* Copyright(c) Andre Caron <andre.l.caron@gmail.com>, 2011
**
** This document is covered by the an Open Source Initiative approved license. A
** copy of the license should have been provided alongside this software package
** (see "LICENSE.txt"). If not, terms of the license are available online at
** "http://www.opensource.org/licenses/mit". */

/*!
 * @file iwire.c
 * @author Andre Caron <andre.l.caron@gmail.com>
 * @brief Parser for FastCGI records.
 */

#include "iwire.h"
#include <string.h>

typedef size_t(*fcgi_request_handler)(fcgi_iwire*,const char*,size_t);
static const fcgi_request_handler fcgi_request_handlers[10];

static const char * fcgi_iwire_error_messages[] =
{
    "no error, parser ok",
};

static size_t _fcgi_iwire_min ( size_t a, size_t b )
{
    return ((a < b)? a : b);
}

const char * fcgi_iwire_error_message ( fcgi_iwire_error error )
{
    return (fcgi_iwire_error_messages[error]);
}

static size_t _fcgi_iwire_copy ( char * lhs, const char * rhs, size_t n )
{
    size_t i = 0;
    while ( i < n ) {
        lhs[i] = rhs[i]; ++i;
    }
    return (i);
}

static size_t fcgi_stage_buffer
    ( fcgi_iwire * stream, const char * data, size_t size )
{
      /* copy to staging area until header is complete. */
    size_t used = _fcgi_iwire_copy(
        stream->staging+stream->staged,
        data, _fcgi_iwire_min(8-stream->staged, size));
      /* update cursors. */
    stream->staged += used;
    return (used);
}

static size_t fcgi_parse_header
    ( fcgi_iwire * stream, const char * data, size_t size )
{
      /* copy to staging area until header is complete. */
    size_t used = fcgi_stage_buffer(stream, data, size);
      /* process body, if complete. */
    if ( stream->staged == 8 )
    {
          /* parse fields. */
        int version =
            (int)(unsigned char)stream->staging[0];
        int reqtype =
            (int)(unsigned char)stream->staging[1];
        int request =
            (int)(unsigned char)stream->staging[2] << 8|
            (int)(unsigned char)stream->staging[3] << 0;
        stream->size =
            (int)(unsigned char)stream->staging[4] << 8|
            (int)(unsigned char)stream->staging[5] << 0;
        stream->skip =
            (int)(unsigned char)stream->staging[6];
          /* new parser state depends on record type. */
        if ((reqtype < 0) || (reqtype > 10)) {
            stream->state = fcgi_iwire_record_fail; return (used);
        }
          /* forward fields. */
        stream->accept_record(stream, version, request, stream->size);
          /* ditch staged data. */
        stream->staged = 0;
          /* for stream records with empty payload, signal end of stream. */
        if ((reqtype >= fcgi_iwire_record_meta) &&
            (reqtype <= fcgi_iwire_record_data) && (stream->size == 0))
        {
            fcgi_request_handlers[reqtype-1](stream, 0, 0);
        }
          /* pass on to new state if there is a payload. */
        if ( stream->size == 0 ) {
            stream->state = fcgi_iwire_record_skip;
        } else {
            stream->state = (fcgi_iwire_state)reqtype;
        }
    }
    return (used);
}

static size_t fcgi_skip_padding
    ( fcgi_iwire * stream, const char * data, size_t size )
{
      /* consume as much data as possible. */
    size_t used = _fcgi_iwire_min(stream->skip, size);
    stream->skip -= used;
      /* signal end of record and reset. */
    if ( stream->skip == 0 ) {
        stream->finish_record(stream);
        stream->skip = 0;
        stream->size = 0;
        stream->staged = 0;
        stream->state = fcgi_iwire_record_idle;
    }
    return (used);
}

typedef void(*accept_stuff)(fcgi_iwire*, const char *, size_t);
typedef void(*accept_ended)(fcgi_iwire*);

static size_t fcgi_forward_stuff_name ( fcgi_iwire * stream,
    const char * data, size_t size, accept_stuff accept_name )
{
       /* note: 'stream->size' is updated by caller. */
    size_t used = _fcgi_iwire_min(stream->ksize, size);
    accept_name(stream, data, used);
    stream->ksize -= used;
    return (used);
}

static size_t fcgi_forward_stuff_data ( fcgi_iwire * stream,
    const char * data, size_t size, accept_stuff accept_data )
{
       /* note: 'stream->size' is updated by caller. */
    size_t used = _fcgi_iwire_min(stream->vsize, size);
    accept_data(stream, data, used);
    stream->vsize -= used;
    return (used);
}

static size_t fcgi_accept_stuff (
    fcgi_iwire * stream, const char * data, size_t size,
    accept_ended complete, accept_stuff accept_name, accept_stuff accept_data )
{
    int n = 0;
    size_t used = 0;
      /* read prefixed lengths. */
    while ((size > 0) && (stream->staged < 8))
    {
          /* when length < 127, length is only one byte. */
        if ( stream->staged == 0 )
        {
            n = (int)data[used++];
            stream->staging[0] = n;
            if ( n <= 127 ) {
                stream->staging[1] = 0;
                stream->staging[2] = 0;
                stream->staging[3] = 0;
                stream->staged = 4;
            }
        }
        if ( stream->staged < 4 ) {
            stream->staging[stream->staged++] = data[used++];
        }
          /* when length < 127, length is only one byte. */
        if ( stream->staged == 4 )
        {
            n = (int)data[used++];
            stream->staging[4] = n;
            if ( n <= 127 ) {
                stream->staging[5] = 0;
                stream->staging[6] = 0;
                stream->staging[7] = 0;
                stream->staged = 8;
            }
        }
        if ( stream->staged < 8 ) {
            stream->staging[stream->staged++] = data[used++];
        }
    }
      /* after reading lengths, interpret content. */
    if ((stream->staged == 8) && (stream->ksize == 0) && (stream->vsize == 0))
    {
        stream->ksize =
            (uint32_t)(unsigned char)stream->staging[0] <<  0|
            (uint32_t)(unsigned char)stream->staging[1] <<  8|
            (uint32_t)(unsigned char)stream->staging[2] << 16|
            (uint32_t)(unsigned char)stream->staging[3] << 24;
        stream->vsize =
            (uint32_t)(unsigned char)stream->staging[4] <<  0|
            (uint32_t)(unsigned char)stream->staging[5] <<  8|
            (uint32_t)(unsigned char)stream->staging[6] << 16|
            (uint32_t)(unsigned char)stream->staging[7] << 24;
    }
      /* forward trailing data. */
    while ((stream->staged == 8) && (used < size) &&
        ((stream->ksize > 0) || (stream->vsize > 0)))
    {
        if (stream->ksize > 0) {
            used += fcgi_forward_stuff_name(
                stream, data+used, size-used, accept_name);
        }
        else if (stream->vsize > 0) {
            used += fcgi_forward_stuff_data(
                stream, data+used, size-used, accept_data);
        }
    }
      /* update parser state. */
    stream->size -= used;
    if ( stream->size == 0 ) {
        complete(stream);
        stream->state = fcgi_iwire_record_skip;
    }
    return (used);
}

static size_t FCGI_BEGIN_REQUEST
    ( fcgi_iwire * stream, const char * data, size_t size )
{
      /* copy to staging area until header is complete. */
    size_t used = fcgi_stage_buffer(stream, data, size);
      /* process body, if complete. */
    if ( stream->staged == 8 )
    {
          /* parse fields. */
        int role =
            (int)(unsigned char)stream->staging[0] << 8|
            (int)(unsigned char)stream->staging[1] << 0;
        int flags =
            (int)(unsigned char)stream->staging[2];
          /* consume staging area. */
        stream->staged = 0;
          /* forward request. */
        stream->accept_request(stream, role, flags);
          /* start skipping padding. */
        stream->state = fcgi_iwire_record_skip;
    }
    return (used);
}

static size_t FCGI_ABORT_REQUEST
    ( fcgi_iwire * stream, const char * data, size_t size )
{
      /* signal abortion. */
    stream->cancel_request(stream);
      /* start skipping padding. */
    stream->state = fcgi_iwire_record_skip;
    return (0);
}

static size_t FCGI_END_REQUEST
    ( fcgi_iwire * stream, const char * data, size_t size )
{
      /* copy to staging area until header is complete. */
    size_t used = fcgi_stage_buffer(stream, data, size);
      /* process body, if complete. */
    if ( stream->staged == 8 )
    {
          /* parse fields. */
        int astatus =
            (int)(unsigned char)stream->staging[0] << 24|
            (int)(unsigned char)stream->staging[1] << 16|
            (int)(unsigned char)stream->staging[2] <<  8|
            (int)(unsigned char)stream->staging[3] <<  0;
        int pstatus =
            (int)(unsigned char)stream->staging[4];
          /* consume staging area. */
        stream->staged = 0;
          /* forward request. */
        stream->finish_request(stream, astatus, pstatus);
          /* start skipping padding. */
        stream->state = fcgi_iwire_record_skip;
    }
    return (used);
}

static size_t FCGI_PARAMS
    ( fcgi_iwire * stream, const char * data, size_t size )
{
      /* consume as much data as possible. */
    size_t used = _fcgi_iwire_min(stream->size, size);
      /* finish parsing if we catch an empty payload. */
    if ((stream->size == 0) && stream->finish_headers ) {
        stream->finish_headers(stream);
    }
      /* forward data as usual. */
    if ( stream->accept_headers ) {
        stream->accept_headers(stream, data, size);
    }
    stream->size -= used;
    if ( stream->size == 0 ) {
        stream->state = fcgi_iwire_record_skip;
    }
    return (used);
}

static size_t FCGI_GET_VALUES
    ( fcgi_iwire * stream, const char * data, size_t size )
{
    return (fcgi_accept_stuff(stream, data, size, stream->accept_query,
        stream->accept_query_name, stream->accept_query_data));
}

static size_t FCGI_GET_VALUES_RESULT
    ( fcgi_iwire * stream, const char * data, size_t size )
{
    return (fcgi_accept_stuff(stream, data, size, stream->accept_reply,
        stream->accept_reply_name, stream->accept_reply_data));
}

static size_t FCGI_STDIN
    ( fcgi_iwire * stream, const char * data, size_t size )
{
      /* consume as much data as possible. */
    size_t used = _fcgi_iwire_min(stream->size, size);
      /* don't forward empty record until we actually have none left. */
    if ((stream->size > 0) && (used == 0)) {
        return (used);
    }
    stream->accept_content_stdi(stream, data, used);
      /* adjust parser state. */
    stream->size -= used;
    if ( stream->size == 0 ) {
        stream->state = fcgi_iwire_record_skip;
    }
    return (used);
}

static size_t FCGI_STDOUT
    ( fcgi_iwire * stream, const char * data, size_t size )
{
      /* consume as much data as possible. */
    size_t used = _fcgi_iwire_min(stream->size, size);
      /* don't forward empty record until we actually have none left. */
    if ((stream->size > 0) && (used == 0)) {
        return (used);
    }
    stream->accept_content_stdo(stream, data, size);
      /* adjust parser state. */
    stream->size -= used;
    if ( stream->size == 0 ) {
        stream->state = fcgi_iwire_record_skip;
    }
    return (used);
}

static size_t FCGI_STDERR
    ( fcgi_iwire * stream, const char * data, size_t size )
{
      /* consume as much data as possible. */
    size_t used = _fcgi_iwire_min(stream->size, size);
      /* don't forward empty record until we actually have none left. */
    if ((stream->size > 0) && (used == 0)) {
        return (used);
    }
    stream->accept_content_stde(stream, data, size);
      /* adjust parser state. */
    stream->size -= used;
    if ( stream->size == 0 ) {
        stream->state = fcgi_iwire_record_skip;
    }
    return (used);
}

static size_t FCGI_DATA
    ( fcgi_iwire * stream, const char * data, size_t size )
{
      /* consume as much data as possible. */
    size_t used = _fcgi_iwire_min(stream->size, size);
      /* signal end of headers when possible. */
    if ( stream->size == 0 )
    {
        if ( stream->finish_headers ) {
            stream->finish_headers(stream);
        }
        stream->state = fcgi_iwire_record_skip;
        return (used);
    }
    stream->accept_content_data(stream, data, size);
      /* adjust parser state. */
    stream->size -= used;
    if ( stream->size == 0 ) {
        stream->state = fcgi_iwire_record_skip;
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

void fcgi_iwire_init
    ( const fcgi_iwire_settings * settings, fcgi_iwire * stream )
{
      /* public members. */
    stream->state = fcgi_iwire_record_idle;
    stream->error = fcgi_iwire_error_none;
    stream->object = 0;
      /* callbacks. */
    stream->accept_record = 0;
    stream->finish_record = 0;
    stream->accept_request = 0;
    stream->accept_request = 0;
    stream->cancel_request = 0;
    stream->finish_request = 0;
    stream->accept_headers = 0;
    stream->finish_headers = 0;
    stream->accept_query_name = 0;
    stream->accept_query_data = 0;
    stream->accept_reply_name = 0;
    stream->accept_reply_data = 0;
    stream->accept_content_stdi = 0;
    stream->accept_content_stdo = 0;
    stream->accept_content_stde = 0;
    stream->accept_content_data = 0;
      /* secret members. */
    stream->settings = settings;
    stream->size = 0;
    stream->skip = 0;
    stream->staged = 0;
    stream->ksize = 0;
    stream->vsize = 0;
}

void fcgi_iwire_clear ( fcgi_iwire * stream )
{
      /* public members. */
    stream->state = fcgi_iwire_record_idle;
    stream->error = fcgi_iwire_error_none;
      /* secret members. */
    stream->size = 0;
    stream->skip = 0;
    stream->staged = 0;
    stream->ksize = 0;
    stream->vsize = 0;
}

size_t fcgi_iwire_feed ( fcgi_iwire * stream, const char * data, size_t size )
{
    size_t used = 0;
      /* data might contain more than one request. */
    while ((used < size) && (stream->state != fcgi_iwire_record_fail))
    {
          /* record head. */
        if ( stream->state == fcgi_iwire_record_idle )
        {
            used += fcgi_parse_header(stream, data+used, size-used);
        }
          /* record body. */
        if ((stream->state >= fcgi_iwire_record_head) &&
            (stream->state <= fcgi_iwire_record_push))
        {
            used += fcgi_request_handlers
                [stream->state-1](stream, data+used, size-used);
        }
           /* record pads. */
        if ( stream->state == fcgi_iwire_record_skip )
        {
            used += fcgi_skip_padding(stream, data+used, size-used);
        }
    }
    return (used);
}
