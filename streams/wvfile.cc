/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2004 Net Integration Technologies, Inc.
 * 
 * A simple class to access filesystem files using WvStreams.
 */
#include "wvfile.h"

#ifdef _WIN32
#include <io.h>
#define O_NONBLOCK 0
#define O_LARGEFILE 0
#define fcntl(a,b,c)
#endif

#ifndef _WIN32 // meaningless to do this on win32
/*
 * The Win32 runtime library doesn't provide fcntl so we can't
 * set readable and writable reliably. Use the other constructor.
 */
WvFile::WvFile(int rwfd) : WvFDStream(rwfd)
{
    if (rwfd > -1)
    {
	/* We have to do it this way since O_RDONLY is defined as 0
	    in linux. */
	mode_t xmode = fcntl(rwfd, F_GETFL);
	xmode = xmode & (O_RDONLY | O_WRONLY | O_RDWR);
	readable = (xmode == O_RDONLY) || (xmode == O_RDWR);
	writable = (xmode == O_WRONLY) || (xmode == O_RDWR);
    }
    else
    {
	readable = writable = false;
    }
}
#endif

bool WvFile::open(WvStringParm filename, int mode, int create_mode)
{
    errnum = 0;
    
    /* We have to do it this way since O_RDONLY is defined as 0
       in linux. */
    int xmode = (mode & (O_RDONLY | O_WRONLY | O_RDWR));
    readable = (xmode == O_RDONLY) || (xmode == O_RDWR);
    writable = (xmode == O_WRONLY) || (xmode == O_RDWR);

    skip_select = false;
    
    // don't do the default force_select of read if we're not readable!
    if (!readable)
	undo_force_select(true, false, false);
    
    close();
    #ifndef _WIN32
    int rwfd = ::open(filename, mode | O_NONBLOCK, create_mode);
    #else
    int rwfd = ::_open(filename, mode | O_NONBLOCK, create_mode);
    #endif
    if (rwfd < 0)
    {
	seterr(errno);
	return false;
    }
    setfd(rwfd);
    fcntl(rwfd, F_SETFD, 1);
    return true;
}


// files not open for read are never readable; files not open for write
// are never writable.
bool WvFile::pre_select(SelectInfo &si)
{
    bool ret;
    
    SelectRequest oldwant = si.wants;
    
    if (!readable) si.wants.readable = false;
    if (!writable) si.wants.writable = false;
    ret = WvFDStream::pre_select(si);
    
    si.wants = oldwant;

    // Force select() to always return true by causing it to not wait and
    // setting our pre_select() return value to true.
    if (skip_select)
    {
	si.msec_timeout = 0;
	ret = true;
    }
    return ret;
}
