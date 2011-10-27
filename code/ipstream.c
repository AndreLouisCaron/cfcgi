/* Copyright(c) Andre Caron (andre.l.caron@gmail.com), 2011
**
** This document is covered by the an Open Source Initiative approved license. A
** copy of the license should have been provided alongside this software package
** (see "LICENSE.txt"). If not, terms of the license are available online at
** "http://www.opensource.org/licenses/mit". */

/*!
 * @file ipstream.c
 * @author Andre Caron (andre.l.caron@gmail.com)
 * @brief Incremental parser for the FastCGI parameter stream.
 */

#include "ipstream.h"

static size_t _fcgi_ipstream_min ( size_t a, size_t b )
{
    return ((a < b)? a : b);
}

static size_t _fcgi_ipstream_nsize(fcgi_ipstream*,const char*,size_t);
static size_t _fcgi_ipstream_dsize(fcgi_ipstream*,const char*,size_t);
static size_t _fcgi_ipstream_ndata(fcgi_ipstream*,const char*,size_t);
static size_t _fcgi_ipstream_ddata(fcgi_ipstream*,const char*,size_t);

static size_t _fcgi_ipstream_nsize
    ( fcgi_ipstream * stream, const char * data, size_t size )
{
    int n = 0;
    size_t used = 0;
    while ((size > 0) && (stream->staged < 4))
    {
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
    }
    if ( stream->staged == 4 )
    {
        stream->nsize = stream->npass =
            (uint32_t)(unsigned char)stream->staging[0] <<  0|
            (uint32_t)(unsigned char)stream->staging[1] <<  8|
            (uint32_t)(unsigned char)stream->staging[2] << 16|
            (uint32_t)(unsigned char)stream->staging[3] << 24;
        stream->staged = 0;
        stream->state = &_fcgi_ipstream_dsize;
    }
    return (used);
}

static size_t _fcgi_ipstream_dsize
    ( fcgi_ipstream * stream, const char * data, size_t size )
{
    int n = 0;
    size_t used = 0;
    while ((size > 0) && (stream->staged < 4))
    {
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
    }
    if ( stream->staged == 4 )
    {
        stream->dsize = stream->dpass =
            (uint32_t)(unsigned char)stream->staging[0] <<  0|
            (uint32_t)(unsigned char)stream->staging[1] <<  8|
            (uint32_t)(unsigned char)stream->staging[2] << 16|
            (uint32_t)(unsigned char)stream->staging[3] << 24;
        stream->staged = 0;
        if ( stream->accept ) {
            stream->accept(stream, stream->nsize, stream->dsize);
        }
        stream->state = &_fcgi_ipstream_ndata;
    }
    return (used);
}

static size_t _fcgi_ipstream_ndata
    ( fcgi_ipstream * stream, const char * data, size_t size )
{
    size_t used = _fcgi_ipstream_min(stream->npass, size);
    if  ( stream->accept_name ) {
        stream->accept_name(stream, data, used);
    }
    stream->npass -= used;
    if ( stream->npass == 0 )
    {
        if ( stream->finish_name ) {
            stream->finish_name(stream);
        }
        stream->state = &_fcgi_ipstream_ddata;
    }
    return (used);
}

static size_t _fcgi_ipstream_ddata
    ( fcgi_ipstream * stream, const char * data, size_t size )
{
    size_t used = _fcgi_ipstream_min(stream->dpass, size);
    if  ( stream->accept_data ) {
        stream->accept_data(stream, data, used);
    }
    stream->dpass -= used;
    if ( stream->dpass == 0 )
    {
        if ( stream->finish_data ) {
            stream->finish_data(stream);
        }
        if ( stream->finish ) {
            stream->finish(stream);
        }
        stream->nsize = 0;
        stream->dsize = 0;
        stream->state = &_fcgi_ipstream_nsize;
    }
    return (used);
}

void fcgi_ipstream_init ( fcgi_ipstream * stream )
{
    stream->object = 0;
    stream->accept = 0;
    stream->accept_name = 0;
    stream->accept_data = 0;
    stream->finish_name = 0;
    stream->finish_data = 0;
    stream->finish = 0;
    
    stream->nsize = 0;
    stream->dsize = 0;
    stream->npass = 0;
    stream->dpass = 0;
    
    stream->staged = 0;
    stream->state = &_fcgi_ipstream_nsize;
}

void fcgi_ipstream_clear ( fcgi_ipstream * stream )
{
    stream->nsize = 0;
    stream->dsize = 0;
    stream->npass = 0;
    stream->dpass = 0;
}

size_t fcgi_ipstream_feed ( fcgi_ipstream * stream, const char * data, size_t size )
{
    size_t used = 0;
    while ( used < size ) {
        used += (*stream->state)(stream, data+used, size-used);
    }
    return (used);
}
