/*
 * Grossly incomplete test for WvLoopback.
 */
#include "wvtest.h"
#include "wvloopback.h"
#include <sys/wait.h>

WVTEST_MAIN("loopback")
{
    WvLoopback loop;
    
    WVPASS(loop.isok());
    
    pid_t pid = fork();
    assert(pid >= 0);
    if (pid == 0) // child: WvTest stuff in here won't work right!
    {
	loop.noread();
	loop.write("message from child\n");
	_exit(0);
    }
    
    // otherwise, we're the parent
    WVPASS(loop.isok());
    loop.nowrite();
    WVPASS(loop.isok());
    loop.select(1000, true, false);
    WVPASS(loop.isok());
    const char *line = loop.getline(10000);
    WVPASS(line);
    printf("line is '%s'\n", line);
    WVPASS(waitpid(pid, NULL, 0) == pid);

    loop.nowrite();
}

WVTEST_MAIN("loopback non-blocking")
{
    WvLoopback *loop = new WvLoopback;
    int i;
    
    WVPASS(loop->isok());

    for (i=0; i<(1<<8); ++i)
    {
        char buf[1024];
	memset(buf, 0, sizeof(buf));
        loop->write(buf, 1024);
    }

    WVPASS(loop->isok());

    WVRELEASE(loop);
}
