/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * A simple class to access filesystem files using WvStreams.
 */
#ifndef __WVFILE_H
#define __WVFILE_H

#include "wvstream.h"
#include <fcntl.h>


/**
 * WvFile implements a stream connected to a file or Unix device.  We
 * include no support for operations like seek().  Since files are not
 * really streams, you probably do not need WvStream support for seekable
 * files; just use standard C I/O functions in that case.
 *
 * WvFile is primarily useful for Unix device files, which have defined
 * select() behaviour for example.
 */
class WvFile : public WvStream
{
public:
    WvFile()
        { }
    WvFile(int _fd) : WvStream(_fd)
        { }
    WvFile(WvStringParm filename, int mode, int create_mode = 0666)
        { open(filename, mode, create_mode); }
    bool open(WvStringParm filename, int mode, int create_mode = 0666);
    
    bool readable, writable;
    
    virtual bool pre_select(SelectInfo &si);
};


#endif // __WVFILE_H
