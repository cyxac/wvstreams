/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * WvStream-based TCP connection class.
 */
#include <netdb.h>
#include "wvstreamlist.h"
#include "wvtcp.h"
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <errno.h>


WvTCPConn::WvTCPConn(const WvIPPortAddr &_remaddr)
{
    remaddr = _remaddr;
    resolved = true;
    connected = false;
    
    do_connect();
}


WvTCPConn::WvTCPConn(int _fd, const WvIPPortAddr &_remaddr)
	: WvStream(_fd)
{
    remaddr = _remaddr;
    resolved = true;
    connected = true;
    
    nice_tcpopts();

    if (getfd() < 0)
	seterr(errno);
}


WvTCPConn::WvTCPConn(WvStringParm _hostname, __u16 _port)
	: hostname(_hostname)
{
    struct servent* serv;
    char *hnstr = hostname.edit(), *cptr;
    
    cptr = strchr(hnstr, ':');
    if (!cptr)
	cptr = strchr(hnstr, '\t');
    if (!cptr)
	cptr = strchr(hnstr, ' ');
    if (cptr)
    {
	*cptr++ = 0;
	serv = getservbyname(cptr, NULL);
	remaddr.port = serv ? ntohs(serv->s_port) : atoi(cptr);
    }
    
    if (_port)
	remaddr.port = _port;
    
    resolved = connected = false;
    
    WvIPAddr x(hostname);
    if (x != WvIPAddr())
    {
	remaddr = WvIPPortAddr(x, remaddr.port);
	resolved = true;
	do_connect();
    }
    else
	dns.findaddr(0, hostname, NULL);
}


WvTCPConn::~WvTCPConn()
{
    // nothing to do
}


// Set a few "nice" options on our socket... (read/write, non-blocking, 
// keepalive)
void WvTCPConn::nice_tcpopts()
{
    fcntl(getfd(), F_SETFD, FD_CLOEXEC);
    fcntl(getfd(), F_SETFL, O_RDWR|O_NONBLOCK);

    int value = 1;
    setsockopt(getfd(), SOL_SOCKET, SO_KEEPALIVE, &value, sizeof(value));
}


void WvTCPConn::low_delay()
{
    int value;
    
    value = 1;
    setsockopt(getfd(), SOL_TCP, TCP_NODELAY, &value, sizeof(value));
    
    value = IPTOS_LOWDELAY;
    setsockopt(getfd(), SOL_IP, IP_TOS, &value, sizeof(value));
}


void WvTCPConn::do_connect()
{
    sockaddr *sa;

    rwfd = socket(PF_INET, SOCK_STREAM, 0);
    if (rwfd < 0)
    {
	seterr(errno);
	return;
    }
    
    nice_tcpopts();
    
    sa = remaddr.sockaddr();
    if (connect(getfd(), sa, remaddr.sockaddr_len()) < 0
	&& errno != EINPROGRESS)
    {
	seterr(errno);
	delete sa;
	return;
    }
    
    delete sa;
}


void WvTCPConn::check_resolver()
{
    const WvIPAddr *ipr;
    int dnsres = dns.findaddr(0, hostname, &ipr);
    
    if (dnsres == 0)
    {
	// error resolving!
	resolved = true;
	seterr(WvString("Unknown host \"%s\"", hostname));
    }
    else if (dnsres > 0)
    {
	remaddr = WvIPPortAddr(*ipr, remaddr.port);
	resolved = true;
	do_connect();
    }
}

#ifndef SO_ORIGINAL_DST
# define SO_ORIGINAL_DST 80
#endif

WvIPPortAddr WvTCPConn::localaddr()
{
    sockaddr_in sin;
    socklen_t sl = sizeof(sin);
    
    if (!isok())
	return WvIPPortAddr();
    
    if (getsockopt(getfd(), SOL_IP, SO_ORIGINAL_DST, (char*)&sin, &sl) < 0
	&& getsockname(getfd(), (sockaddr *)&sin, &sl))
    {
	return WvIPPortAddr();
    }
    
    return WvIPPortAddr(&sin);
}


const WvIPPortAddr *WvTCPConn::src() const
{
    return &remaddr;
}


bool WvTCPConn::pre_select(SelectInfo &si)
{
    if (!resolved)
    {
	if (dns.pre_select(hostname, si))
	    check_resolver();
    }

    if (resolved && isok()) // name might be resolved now.
    {
	bool oldw = si.wants.writable, retval;
	if (!isconnected()) si.wants.writable = true;
	retval = WvStream::pre_select(si);
	si.wants.writable = oldw;
	return retval;
    }
    else
	return false;
}
			  

bool WvTCPConn::post_select(SelectInfo &si)
{
    bool result = false;

    if (!resolved)
	check_resolver();
    else
    {
	result = WvStream::post_select(si);

	if (result && !connected)
	{
	    sockaddr *sa = remaddr.sockaddr();
	    int retval = connect(getfd(), sa, remaddr.sockaddr_len());
	    
	    if (!retval || (retval < 0 && errno == EISCONN))
		connected = result = true;
	    else if (retval < 0 && errno != EINPROGRESS)
	    {
		seterr(errno);
		result = true;
	    }
	    else
		result = false;
	    
	    delete sa;
	}
    }
    
    return result;
}


bool WvTCPConn::isok() const
{
    return !resolved || WvStream::isok();
}


size_t WvTCPConn::uwrite(const void *buf, size_t count)
{
    if (connected)
	return WvStream::uwrite(buf, count);
    else
	return 0; // can't write yet; let them enqueue it instead
}




WvTCPListener::WvTCPListener(const WvIPPortAddr &_listenport)
	: listenport(_listenport), auto_callback(NULL)
{
    listenport = _listenport;
    auto_list = NULL;
    auto_userdata = NULL;
    
    sockaddr *sa = listenport.sockaddr();
    
    int x = 1;

    rwfd = socket(PF_INET, SOCK_STREAM, 0);
    if (rwfd < 0
	|| setsockopt(getfd(), SOL_SOCKET, SO_REUSEADDR, &x, sizeof(x))
	|| fcntl(getfd(), F_SETFD, 1)
	|| bind(getfd(), sa, listenport.sockaddr_len())
	|| listen(getfd(), 5))
    {
	seterr(errno);
    }
    
    if (listenport.port == 0) // auto-select a port number
    {
	socklen_t namelen = listenport.sockaddr_len();
	
	if (getsockname(getfd(), sa, &namelen) != 0)
	    seterr(errno);
	else
	    listenport = WvIPPortAddr((sockaddr_in *)sa);
    }
    
    delete sa;
}


WvTCPListener::~WvTCPListener()
{
    close();
}


//#include <wvlog.h>
void WvTCPListener::close()
{
    WvStream::close();
/*    WvLog log("ZAP!");
    
    log("Closing TCP LISTENER at %s!!\n", listenport);
    abort();*/
}


WvTCPConn *WvTCPListener::accept()
{
    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);
    int newfd;
    WvTCPConn *ret;

    newfd = ::accept(getfd(), (struct sockaddr *)&sin, &len);
    ret = new WvTCPConn(newfd, WvIPPortAddr(&sin));
    return ret;
}


void WvTCPListener::auto_accept(WvStreamList *list,
				WvStreamCallback callfunc, void *userdata)
{
    auto_list = list;
    auto_callback = callfunc;
    auto_userdata = userdata;
    setcallback(accept_callback, this);
}


void WvTCPListener::accept_callback(WvStream &, void *userdata)
{
    WvTCPListener &l = *(WvTCPListener *)userdata;

    WvTCPConn *connection = l.accept();
    connection->setcallback(l.auto_callback, l.auto_userdata);
    l.auto_list->append(connection, true);
}


size_t WvTCPListener::uread(void *, size_t)
{
    return 0;
}


size_t WvTCPListener::uwrite(const void *, size_t)
{
    return 0;
}


const WvIPPortAddr *WvTCPListener::src() const
{
    return &listenport;
}

