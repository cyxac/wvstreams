/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */
#ifndef __WVSPLITSTREAM_H
#define __WVSPLITSTREAM_H

#include "wvstream.h"

/**
 * A WvSplitStream uses two different file descriptors: one for input
 * and another for output.
 * 
 * This is primarily used for the combined stdin/stdout stream.
 */
class WvSplitStream : public WvStream
{
public:
    WvSplitStream(int _rfd, int _wfd);
    virtual ~WvSplitStream();
    
    virtual void close();
    virtual int getrfd() const;
    virtual int getwfd() const;
    
    /**
     * noread() closes the rfd and makes this stream no longer valid for
     * reading. 
     */
    void noread();

    /**
     * nowrite() closes wfd and makes it no longer valid for
     * writing.
     */
    void nowrite();
    
protected:
    int rfd, wfd;

    WvSplitStream(); // derived classes might not know the fds yet!
};

#endif // __WVSPLITSTREAM_H
