/* Copyright(c) Andre Caron (andre.l.caron@gmail.com), 2011
**
** This document is covered by the an Open Source Initiative approved license. A
** copy of the license should have been provided alongside this software package
** (see "LICENSE.txt"). If not, terms of the license are available online at
** "http://www.opensource.org/licenses/mit". */

/*!
 * @file owire.c
 * @author Andre Caron (andre.l.caron@gmail.com)
 * @brief Writer for FastCGI records.
 */

#include "owire.h"
#include <string.h>

#define MAXIMUM_CONTENT_LENGTH ((1 << 16)-1)

static const char * fcgi_owire_error_messages[] =
{
    "no error, writer ok",
};

static size_t _fcgi_owire_min ( size_t a, size_t b )
{
    return ((a < b)? a : b);
}

static size_t _fcgi_owire_send ( fcgi_owire * stream,
    uint16_t rqid, int type, const char * body, size_t size )
{
    const char head[8] = {
        1,                             // version        : FCGI_VERSION_1
        type,                          // record type    : FCGI_STD*
        ((rqid>>8)&0xff), (rqid&0xff), // request id     : ...
        ((size>>8)&0xff), (size&0xff), // content length : ...
        0,                             // padding        : 0
        0,                             // reserved       : ...
    };
    stream->write_stream(stream, head,    8);
    stream->write_stream(stream, body, size);
    if ( stream->flush_stream ) {
        stream->flush_stream(stream);
    }
    return (size);
}

void fcgi_owire_init
    ( const fcgi_owire_settings * settings, fcgi_owire * stream )
{
    stream->error = fcgi_owire_error_none;
    stream->settings = settings;
    stream->object = 0;
    stream->write_stream = 0;
    stream->flush_stream = 0;
}

size_t fcgi_owire_new_request
    ( fcgi_owire * stream, uint16_t request, uint16_t role )
{
    const char body[8] = {
        ((role>>8)&0xff), (role&0xff), // request id
        0,                             // flags
        0, 0, 0, 0, 0,                 // reserved
    };
    _fcgi_owire_send(stream, request, 1, body, 8); return (0);
}

size_t fcgi_owire_bad_request ( fcgi_owire * stream, uint16_t request )
{
    _fcgi_owire_send(stream, request, 2, 0, 0); return (0);
}

size_t fcgi_owire_end_request
    ( fcgi_owire * stream, uint16_t request, uint32_t astatus, uint8_t pstatus )
{
    const char body[8] = {
        ((astatus>>24)&0xff), ((astatus>>16)&0xff), // application status
        ((astatus>> 8)&0xff), ((astatus>> 0)&0xff), // (continued)
        pstatus,                                    // protocol status
        0, 0, 0,                                    // reserved
    };
    _fcgi_owire_send(stream, request, 3, body, 8); return (0);
}

size_t fcgi_owire_param
    ( fcgi_owire * stream, uint16_t request, const char * data, size_t size )
{
    size_t used = 0;
    do {
        used += _fcgi_owire_send(stream, request, 4,
            data+used, _fcgi_owire_min(size-used, MAXIMUM_CONTENT_LENGTH));
    } while ( used < size );
    return (used);
}

size_t fcgi_owire_stdi
    ( fcgi_owire * stream, uint16_t request, const char * data, size_t size )
{
    size_t used = 0;
    do {
        used += _fcgi_owire_send(stream, request, 5,
            data+used, _fcgi_owire_min(size-used, MAXIMUM_CONTENT_LENGTH));
    } while ( used < size );
    return (used);
}

size_t fcgi_owire_stdo
    ( fcgi_owire * stream, uint16_t request, const char * data, size_t size )
{
    size_t used = 0;
    do {
        used += _fcgi_owire_send(stream, request, 6,
            data+used, _fcgi_owire_min(size-used, MAXIMUM_CONTENT_LENGTH));
    }
    while ( used < size );
    return (used);
}

size_t fcgi_owire_stde
    ( fcgi_owire * stream, uint16_t request, const char * data, size_t size )
{
    size_t used = 0;
    do {
        used += _fcgi_owire_send(stream, request, 7,
            data+used, _fcgi_owire_min(size-used, MAXIMUM_CONTENT_LENGTH));
    }
    while ( used < size );
    return (used);
}

size_t fcgi_owire_extra
    ( fcgi_owire * stream, uint16_t request, const char * data, size_t size )
{
    size_t used = 0;
    do {
        used += _fcgi_owire_send(stream, request, 8,
            data+used, _fcgi_owire_min(size-used, MAXIMUM_CONTENT_LENGTH));
    }
    while ( used < size );
    return (used);
}

size_t fcgi_owire_query
    ( fcgi_owire * stream, const char * data, uint16_t size )
{
    _fcgi_owire_send(stream, 0, 9, data, size); return (size);
}

size_t fcgi_owire_reply
    ( fcgi_owire * stream, const char * data, uint16_t size )
{
    _fcgi_owire_send(stream, 0, 10, data, size); return (size);
}
