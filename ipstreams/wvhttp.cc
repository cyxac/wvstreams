/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997, 1998 Worldvisions Computer Technology, Inc.
 * 
 * WvHTTPStream connects to an HTTP server and allows the requested file
 * to be retrieved using the usual WvStream-style calls.
 */
#include "wvhttp.h"
#include "strutils.h"
#include <assert.h>


//////////////////////////////////////// WvURL


WvURL::WvURL(const WvString &url) : err("No error")
{
    char *cptr;
    
    port = 0; // error condition by default
    addr = NULL;
    resolving = true;
    
    if (strncmp(url.str, "http://", 7)) // NOT equal
    {
	err = "WvURL can only handle HTTP URLs.";
	return;
    }
    hostname = url.str + 7;
    
    cptr = strchr(hostname.str, '/');
    if (!cptr) // no path given
	file = "/";
    else
    {
	file = cptr;
	*cptr = 0;
    }
    
    cptr = strchr(hostname.str, ':');
    if (!cptr)
	port = 80;
    else
    {
	port = atoi(cptr+1);
	*cptr = 0;
    }
    
    resolve();
}


WvURL::~WvURL()
{
    if (addr) delete addr;
}


bool WvURL::resolve()
{
    const WvIPAddr *ip;
    int numaddrs;
    
    numaddrs = dns.findaddr(0, hostname, &ip);
    if (!numaddrs) // error condition
    {
	err = WvString("Host %s could not be found.", hostname);
	resolving = false;
	return false;
    }
    else if (numaddrs < 0) // still waiting
    {
	resolving = true;
	return false;
    }
    else // got at least one address
    {
	resolving = false;
	if (addr) delete addr;
	addr = new WvIPPortAddr(*ip, port);
	return true;
    }
}


WvURL::operator WvString () const
{
    if (!isok())
	return WvString("(Invalid URL: %s)", err);
    
    WvString portstr("");
    if (port && port != 80)
	portstr = WvString(":%s", port);
    if (hostname.str)
	return WvString("http://%s%s%s", hostname, portstr, file);
    else if (addr)
	return WvString("http://%s%s%s", *addr, portstr, file);
    else
    {
	assert(0);
	return WvString("(Invalid URL)");
    }
}



//////////////////////////////////////// WvHTTPStream



WvHTTPStream::WvHTTPStream(WvURL &_url)
	: WvStreamClone((WvStream **)&http), headers(7), url(_url)
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
	return url.errstr().str;
    else
	return "Unknown error!";
}


bool WvHTTPStream::select_setup(fd_set &r, fd_set &w, fd_set &x, int &max_fd,
			      bool readable, bool writable, bool isexception)
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

	// otherwise, we just finished connecting:  start transfer.
	state = ReadHeader1;
	print("GET %s HTTP/1.0\n\n", url.getfile());
	
	// FALL THROUGH!
	
    default:
	return WvStreamClone::isok()
	    && WvStreamClone::select_setup(r, w, x, max_fd,
					   readable, writable, isexception);
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
    }
    
    return 0;
}
