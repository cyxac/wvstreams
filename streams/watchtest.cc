/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997, 1998 Worldvisions Computer Technology, Inc.
 */
#include "wvlog.h"
#include "wvwatcher.h"

int main()
{
    WvLog log("watchtest", WvLog::Info);
    const WvString fname("/tmp/test.txt");
    WvFileWatcher f(fname.str, O_RDONLY | O_CREAT);
    char buf[1024];
    size_t len;
    
    log(WvLog::Notice, "Watching %s:\n", fname);
    
    while (f.isok())
    {
	len = f.read(buf, 1024);
	log.write(buf, len);
    }
    
    if (f.geterr())
	log("%s: %s\n", fname, strerror(f.geterr()));
    return 0;
}
