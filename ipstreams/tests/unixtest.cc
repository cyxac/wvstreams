/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * WvTCPConn test.  Telnets to your local SMTP port, or any other port given
 * on the command line.
 */
#include "wvunixsocket.h"
#include "wvstreamlist.h"
#include "wvlog.h"


int main(int argc, char **argv)
{
    WvLog err("unixtest", WvLog::Error);
    WvUnixConn sock(argc==2 ? argv[1] : "/tmp/fuzzy");
    
    wvcon->autoforward(sock);
    sock.autoforward(*wvcon);
    
    WvStreamList l;
    l.add_after(l.tail, wvcon, false);
    l.add_after(l.tail, &sock, false);
    
    while (wvcon->isok() && sock.isok())
    {
	if (l.select(-1))
	    l.callback();
    }
    
    if (!wvcon->isok() && wvcon->geterr())
	err("stdin: %s\n", wvcon->errstr());
    else if (!sock.isok() && sock.geterr())
	err("socket: %s\n", sock.errstr());
    
    return 0;
}
