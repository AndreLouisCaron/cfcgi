=====================================================
  `cfcgi` --- Streaming FastCGI record parser for C
=====================================================
:authors:
   André Caron
:contact: andre.l.caron@gmail.com

Description
===========

This library provides a parser for `FastCGI`_ records.  The parser is
implemented as a finite state machine (FSM) for use in streaming applications
(i.e. data arrives at an unpredictable rate and the parser must be
interruptible).  As such, the parser itself does not buffer any received data.
It just forwards it to registered callbacks.  It requires little overhead and is
well suited for being used in an object-oriented wrapper.

.. _`FastCGI`: http://www.fastcgi.com/drupal/

Demonstration
=============

There is a pre-fork server (*NIX systems only) implementing a FastCGI
authorizer in the ``demo/nix`` folder.  It also comes with a minimal LigHTTPd
configuration to test it against an existing FastCGI implementation known to
work!
