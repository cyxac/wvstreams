/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * WvHTTPStream connects to an HTTP server and allows the requested file
 * to be retrieved using the usual WvStream-style calls.
 */
#include "wvhttp.h"
#include "strutils.h"
#include <assert.h>



WvHTTPStream::WvHTTPStream(const WvURL &_url)
	: WvStreamClone((WvStream **)&http), headers(7), client_headers(7),
          url(_url)
{
    state = Resolving;
    http = NULL;
    num_received = 0;
    
    // we need this: if the URL tried to dns-resolve before, but failed,
    // this might make isok() true again if the name has turned up.
    url.resolve();
}


WvHTTPStream::~WvHTTPStream()
{
    if (http) delete http;
}


bool WvHTTPStream::isok() const
{
    if (http)
	return WvStreamClone::isok();
    else
	return url.isok();
}


int WvHTTPStream::geterr() const
{
    if (http)
	return WvStreamClone::geterr();
    else
	return -1;
}


const char *WvHTTPStream::errstr() const
{
    if (http)
	return WvStreamClone::errstr();
    else if (!url.isok())
	return url.errstr();
    else
	return "Unknown error! (no stream yet)";
}


bool WvHTTPStream::pre_select(SelectInfo &si)
{
    if (!isok()) return false;
    
    switch (state)
    {
    case Resolving:
	if (!url.isok())
	    seterr("Invalid URL");
	else if (url.resolve())
	{
	    state = Connecting;
	    http = new WvTCPConn(url.getaddr());
	}
	return false;

    case Connecting:
	http->select(0, false, true, false);
	if (!http->isconnected())
	    return false;
	if (http->geterr())
	    return false;

	// otherwise, we just finished connecting:  start transfer.
	state = ReadHeader1;
	delay_output(true);
	print("GET %s HTTP/1.0\r\n", url.getfile());
	print("Host: %s:%s\r\n", url.gethost(), url.getport());
        {
            WvHTTPHeaderDict::Iter i(client_headers);
            for (i.rewind(); i.next(); )
            {
                print("%s: %s\r\n", i().name, i().value);
            }
        }
        print("\r\n");
	delay_output(false);
	
	// FALL THROUGH!
	
    default:
	return WvStreamClone::isok()
	    && WvStreamClone::pre_select(si);
    }
}


size_t WvHTTPStream::uread(void *buf, size_t count)
{
    char *line;
    int retval;
    size_t len;
    
    switch (state)
    {
    case Resolving:
    case Connecting:
	break;
	
    case ReadHeader1:
	line = http->getline(0);
	line = trim_string(line);
	if (line) // got response code line
	{
	    if (strncmp(line, "HTTP/", 5))
	    {
		seterr("Invalid HTTP response");
		return 0;
	    }
	    
	    retval = atoi(trim_string(line+9));
	    
	    if (retval / 100 != 2)
	    {
		seterr(WvString("HTTP error: %s", trim_string(line+9)));
		return 0;
	    }
		
	    state = ReadHeader;
	}
	break;
	
    case ReadHeader:
	line = http->getline(0);
	if (line)
	{
	    line = trim_string(line);
	    if (!line[0])
		state = ReadData;	// header is done
	    else
	    {
		char *cptr = strchr(line, ':');
		if (!cptr)
		    headers.add(new WvHTTPHeader(line, ""), true);
		else
		{
		    *cptr++ = 0;
		    line = trim_string(line);
		    cptr = trim_string(cptr);
		    headers.add(new WvHTTPHeader(line, cptr), true);
		}
	    }
	}
	break;
	
    case ReadData:
	len = http->read(buf, count);
	num_received += len;
	return len;
	
    case Done:
	break;
    }
    
    return 0;
}
