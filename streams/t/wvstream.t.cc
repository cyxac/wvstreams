#include "wvtest.h"

#define private public
#define protected public
#include "wvstream.h"
#undef private
#undef protected

#include "wvistreamlist.h"
#include "wvcont.h"
#include "wvtimeutils.h"

class ReadableStream : public WvStream
{
public:
    bool yes_readable;
    ReadableStream()
        { yes_readable = false; }
    
    virtual bool pre_select(SelectInfo &si)
    {
	int ret = WvStream::pre_select(si);
	if (yes_readable && si.wants.readable)
	    return true;
	else
	    return ret;
    }
};

class CountStream : public WvStream
{
public:
    size_t rcount, wcount;
    bool yes_writable, block_writes;
    
    CountStream()
    	{ rcount = wcount = 0; yes_writable = block_writes = false; }
    virtual ~CountStream()
    	{ close(); }
    
    virtual size_t uread(void *buf, size_t count)
    {
	size_t ret = WvStream::uread(buf, count);
	rcount += ret;
	return ret;
    }
    
    virtual size_t uwrite(const void *buf, size_t count)
    {
	if (block_writes)
	    return 0;
	size_t ret = WvStream::uwrite(buf, count);
	wcount += ret;
	return ret;
    }
    
    virtual bool post_select(SelectInfo &si)
    {
	int ret = WvStream::post_select(si);
	if (yes_writable 
	  && (si.wants.writable || (outbuf.used() && want_to_flush)))
	    return true;
	else
	    return ret;
    }
};



WVTEST_MAIN("buffered read/write")
{
    WvStream s;
    char buf[1024];
    
    // buffered reads and writes
    WVPASS(!s.isreadable());
    WVPASS(!s.iswritable());
    WVFAIL(s.read(buf, 1024) != 0);
    WVPASS(s.write(buf, 1024) == 1024);
    WVPASS(!s.iswritable());
    WVPASS(!s.isreadable());
    WVPASS(s.isok());
    
    // close() shouldn't have to wait to flush buffers, because plain
    // WvStream has no way to actually flush them.
    WvTime t1 = wvtime();
    s.close();
    WvTime t2 = wvtime();
    WVPASS(msecdiff(t2, t1) >= 0);
    WVPASS(msecdiff(t2, t1) < 1000);
	
    // after close()
    WVPASS(!s.isok());
}


// error tests
WVTEST_MAIN("errors")
{
    WvStream a, b;
    
    WVPASS(a.isok());
    WVPASS(!a.geterr());
    a.seterr(EBUSY);
    WVPASS(!a.isok());
    WVPASS(a.geterr() == EBUSY);
    WVPASS(a.errstr() == strerror(EBUSY));
    
    b.seterr("I'm glue!");
    WVPASS(!b.isok());
    WVPASS(b.geterr() == -1);
    WVPASS(b.errstr() == "I'm glue!");
}


// noread/nowrite behaviour
WVTEST_MAIN("noread/nowrite")
{
    WvStream s;
    char buf[1024];

    s.nowrite();
    WVPASS(s.isok());
    WVFAIL(s.write(buf, 1024) != 0);
    s.noread();
    WVPASS(!s.isok());
}


// getline tests
WVTEST_MAIN("getline")
{
    WvStream s;
    char buf[1024];
    
    WVPASS(!s.isreadable());
    s.inbuf.putstr("a\n b \r\nline");
    WVPASS(s.isreadable());
    s.noread();
    WVPASS(s.isreadable());
    
    WVPASS(s.read(buf, 2) == 2);
    char *line = s.getline();
    WVPASS(line);
    WVPASS(line && !strcmp(line, " b \r"));
    line = s.getline();
    WVPASS(line);
    WVPASS(line && !strcmp(line, "line"));
    WVPASS(!s.getline());
    
    WvTime t1 = wvtime();
    WVPASS(!s.blocking_getline(500));
    WvTime t2 = wvtime();
    WVPASS(msecdiff(t2, t1) >= 0);
    WVPASS(msecdiff(t2, t1) < 400); // noread().  shouldn't actually wait!
   
    WvStream t;
    t.inbuf.putstr("tremfodls\nd\ndopple");
    line = t.getline('\n', 20);
    WVPASS(line && !strcmp(line, "tremfodls"));
    t.close();
    line = t.getline('\n', 20);
    WVPASS(line && !strcmp(line, "d"));
    line = t.getline('\n', 20);
    WVPASS(line && !strcmp(line, "dopple"));

    // FIXME: avoid aborting the entire test here on a freezeup!
    ::alarm(5); // crash after 5 seconds
    WVPASS(!s.blocking_getline(-1));
    ::alarm(0);
}

// more noread/nowrite behaviour
WVTEST_MAIN("more noread/nowrite")
{
    WvStream s;
    
    s.inbuf.putstr("hello");
    s.write("yellow");
    WVPASS(s.isok());
    s.nowrite();
    WVPASS(s.isok());
    s.noread();
    WVPASS(s.isok());
    WVPASS(s.getline());
    WVFAIL(s.blocking_getline(-1));
    WVPASS(!s.isok());
}


static void val_cb(WvStream &s, void *userdata)
{
    (*(int *)userdata)++;
}


static void val_icb(int &closeval, WvStream &s)
{
    ++closeval;
}


// callback tests
WVTEST_MAIN("callbacks")
{
    int val = 0, closeval = 0;
    
    WvStream s;
    s.setcallback(val_cb, &val);
    s.setclosecallback(WvBoundCallback<IWvStreamCallback, int&>(&val_icb, closeval));
    
    WVPASS(!val);
    WVPASS(!closeval);
    s.runonce(0);
    WVPASS(!val);
    s.inbuf.putstr("gah");
    s.runonce(0);
    WVPASS(val == 1); // callback works?
    s.runonce(0);
    WVPASS(val == 2); // level triggered?
    s.getline();
    WVPASS(val == 2); // but not by getline
    WVPASS(!closeval);
    s.inbuf.putstr("blah!");
    s.nowrite();
    s.noread();
    s.runonce(0);
    WVPASS(val == 3);
    WVPASS(s.getline());
    s.runonce(0);
    WVPASS(val == 3);
    WVPASS(closeval == 1);
    s.runonce(0);
    WVPASS(closeval == 1);
    s.close();
    WVPASS(closeval == 1);
}


// autoforward and various buffer-related tests
WVTEST_MAIN("autoforward and buffers")
{
    WvStream a;
    CountStream b;
    int val = 0;
    
    a.autoforward(b);
    b.setcallback(val_cb, &val);
    
    a.inbuf.putstr("astr");
    a.runonce(0);
    WVPASS(!a.inbuf.used());
    b.runonce(0);
    WVPASS(val == 0);
    WVPASS(b.wcount == 4);
    a.noautoforward();
    a.inbuf.putstr("astr2");
    a.runonce(0);
    WVPASS(a.inbuf.used() == 5);
    WVPASS(b.wcount == 4);
    
    // delay_output tests
    a.autoforward(b);
    b.delay_output(true);
    a.runonce(0);
    WVPASS(!a.inbuf.used());
    WVPASS(b.wcount == 4);
    WVPASS(b.outbuf.used() == 5);
    b.runonce(0);
    WVFAIL(!b.outbuf.used());
    b.yes_writable = true;
    b.runonce(0);
    WVFAIL(!b.outbuf.used());
    b.flush(0);
    WVPASS(!b.outbuf.used());
    WVPASS(b.wcount == 4+5);
    
    // autoforward() has lower precedence than drain()
    WVPASS(!a.inbuf.used());
    a.inbuf.putstr("googaa");
    a.drain();
    WVPASS(b.wcount == 4+5);
    WVPASS(!a.inbuf.used());
    
    // queuemin() works
    a.inbuf.putstr("phleg");
    a.queuemin(2);
    WVPASS(a.isreadable());
    a.queuemin(6);
    WVFAIL(a.isreadable());
    a.drain();
    WVFAIL(a.getline());
    WVFAIL(a.isreadable());
    WVFAIL(!a.inbuf.used());
    a.inbuf.putstr("x");
    WVPASS(a.isreadable());
    WVPASS(a.inbuf.used() == 6);
    a.drain();
    WVPASS(!a.inbuf.used());
}


// flush_then_close()
WVTEST_MAIN("flush_then_close")
{
    CountStream s;
    
    s.block_writes = true;
    s.write("abcdefg");
    WVPASS(s.outbuf.used() == 7);
    s.flush(0);
    WVPASS(s.outbuf.used() == 7);
    s.runonce(0);
    WVPASS(s.outbuf.used() == 7);
    s.flush_then_close(20000);
    WVPASS(s.outbuf.used() == 7);
    s.runonce(0);
    WVPASS(s.outbuf.used() == 7);
    WVPASS(s.isok());
    s.yes_writable = true;
    s.block_writes = false;
    s.runonce(0);
    s.runonce(0);
    WVPASS(s.outbuf.used() == 0);
    WVFAIL(s.isok());
}


// force_select and the globallist
WVTEST_MAIN("force_select and globallist")
{
    int val = 0;
    CountStream s, x;
    s.yes_writable = x.yes_writable = true;
    
    WVFAIL(s.select(0));
    WVPASS(s.select(0, false, true));
    s.inbuf.putstr("hello");
    WVPASS(s.select(0));
    WVFAIL(s.select(0, false, false));
    s.undo_force_select(true, false, false);
    WVFAIL(s.select(0));
    s.force_select(false, true, false);
    WVPASS(s.select(0));
    s.yes_writable = false;
    WVFAIL(s.select(0));
    
    x.setcallback(val_cb, &val);
    WvIStreamList::globallist.append(&x, false);
    WVFAIL(s.select(0));
    WVPASS(!val);
    x.inbuf.putstr("yikes");
    x.force_select(false, true, false);
    WVPASS(!s.select(0));
    WVPASS(val == 1);
    WVPASS(!s.select(0));
    WVPASS(val == 2);
    WVPASS(!s.select(0, false, true));
    s.yes_writable = true;
    WVPASS(s.select(0, false, true));
    WVPASS(val == 2);
    s.runonce();
    WVPASS(val == 3);
    
    WvIStreamList::globallist.unlink(&x);
}


static void cont_cb(WvStream &s, void *userdata)
{
    int next = 1, sgn = 1;
    
    while (s.isok())
    {
	*(int *)userdata = sgn * next;
	next *= 2;
	sgn = s.continue_select(0) ? 1 : -1;
    }
}


WVTEST_MAIN("continue_select")
{
    WvStream a;
    int aval = 0;
    
    a.uses_continue_select = true;
    a.setcallback(cont_cb, &aval);
    
    a.runonce(0);
    WVPASS(aval == 0);
    a.inbuf.putstr("gak");
    a.runonce(0);
    WVPASS(aval == 1);
    a.runonce(0);
    WVPASS(aval == 2);
    a.drain();
    WVPASS(aval == 2);
    a.alarm(-1);
    a.runonce(0);
    WVPASS(aval == 2);
    a.alarm(0);
    a.runonce(0);
    WVPASS(aval == -4);
    
    a.terminate_continue_select();
}


static void cont_once(WvStream &s, void *userdata)
{
    int *i = (int *)userdata;
    
    (*i)++;
    s.continue_select(10);
    (*i)++;
    *i = -*i;
}


WVTEST_MAIN("continue_select and alarm()")
{
    int i = 1;
    ReadableStream s;
    s.uses_continue_select = true;
    s.setcallback(cont_once, &i);
    
    s.yes_readable = true;
    WVPASSEQ(i, 1);
    s.runonce(100);
    WVPASSEQ(i, 2);
    s.runonce(100);
    WVPASSEQ(i, -3);

    s.yes_readable = false;
    s.runonce(100);
    WVPASSEQ(i, -3);
    
    s.alarm(0);
    s.runonce(100);
    WVPASSEQ(i, -2);
    
    s.alarm(-1); // disabling the alarm should disable continue_select timeout
    s.runonce(100);
    WVPASSEQ(i, -2);
    
    s.terminate_continue_select();
}


static void seterr_cb(WvStream &s, void *userdata)
{
    s.seterr("my seterr_cb error");
}


WVTEST_MAIN("seterr() inside a continuable callback")
{
    WvStream s;
    s.setcallback(seterr_cb, NULL);
    WVPASS(s.isok());
    s.runonce(0);
    WVPASS(s.isok());
    s.alarm(0);
    s.runonce(5);
    WVFAIL(s.isok());
    WVPASS(s.geterr() == -1);
    s.terminate_continue_select();
}


static void *wvcont_cb(WvStream &s, void *userdata)
{
    int next = 1, sgn = 1;
    int *val = (int *)userdata;
    
    while (s.isok())
    {
	*val = sgn * next;
	next *= 2;
	sgn = WvCont::yield() ? 1 : -1;
	printf("...returned from yield()\n");
    }
    
    *val = 4242;
    return NULL;
}


static void call_wvcont_cb(void *context, WvStream &s, void *userdata)
{
    WvCont *cb = (WvCont *)context;
    (*cb)(userdata);
}


WVTEST_MAIN("continue_select compatibility with WvCont")
{
    WvStream s;
    int sval = 0;
    
    s.uses_continue_select = true;
    
    {
	WvCont cont1(WvBoundCallback<WvCallback<void*,void*>, WvStream&>
		     (&wvcont_cb, s));
	WvCont cont2(cont1), cont3(cont2);
	s.setcallback(WvBoundCallback<WvStreamCallback, void* >
		      (&call_wvcont_cb, &cont3), &sval);
	
	s.inbuf.putstr("gak");
	WVPASS(sval == 0);
	s.runonce(0);
	WVPASS(sval == 1);
	s.runonce(0);
	WVPASS(sval == 2);
	s.close();
	WVPASS(!s.isok());
	s.runonce(0);
	s.setcallback(0, 0);
    }
    
    // the WvCont should have now been destroyed!
    WVPASS(sval == 4242);
}


static void alarmcall(WvStream &s, void *userdata)
{
    WVPASS(s.alarm_was_ticking);
    val_cb(s, userdata);
}


// alarm()
WVTEST_MAIN("alarm")
{
    int val = 0;
    WvStream s;
    s.setcallback(alarmcall, &val);
    
    s.runonce(0);
    WVPASSEQ(val, 0);
    s.alarm(0);
    s.runonce(0);
    WVPASSEQ(val, 1);
    s.runonce(5);
    WVPASSEQ(val, 1);
    s.alarm(-5);
    WVPASS(s.alarm_remaining() == -1);
    s.runonce(0);
    WVPASSEQ(val, 1);
    s.alarm(100);
    time_t remain = s.alarm_remaining();
    WVPASS(remain > 0);
    WVPASS(remain <= 100);
    s.runonce(0);
    WVPASSEQ(val, 1);
    s.runonce(10000);
    WVPASSEQ(val, 2);
    printf("alarm remaining: %d\n", (int)s.alarm_remaining());
    s.runonce(50);
    WVPASSEQ(val, 2);

    WvIStreamList l;
    l.append(&s, false, "alarmer");
    
    s.alarm(1);
    l.runonce(10);
    l.runonce(10);
    l.runonce(10);
    WVPASSEQ(val, 3);
}


static int rn = 0;


static void rcb2(WvStream &s, void *)
{
    rn++;
    WVPASS(rn == 2);
    s.setcallback(0, 0);
}


static void rcb(WvStream &s, void *)
{
    rn++;
    WVPASS(rn == 1);
    s.setcallback(rcb2, NULL);
    WVPASS(rn == 1);
    //s.continue_select(0);
    //WVPASS(rn == 1);
}


WVTEST_MAIN("self-redirection")
{
    WvStream s;
    s.uses_continue_select = true;
    s.setcallback(rcb, NULL);
    s.inbuf.putstr("x");
    s.runonce(0);
    s.runonce(0);
    s.runonce(0);
    s.runonce(0);
    WVPASS(rn == 2);
    s.terminate_continue_select();
}
