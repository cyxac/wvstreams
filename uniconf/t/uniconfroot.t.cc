#include "wvtest.h"
#include "uniconfroot.h"

WVTEST_MAIN("no generator")
{
    UniConfRoot root;
}


WVTEST_MAIN("null generator")
{
    UniConfRoot root("null:");
}


// if this test compiles at all, we win!
WVTEST_MAIN("type conversions")
{
    UniConfRoot root("temp:");
    UniConf cfg(root);
    
    cfg.setme("x");
    cfg.setme(5);
    WVPASSEQ("5", cfg.getme("x"));
    WVPASSEQ("5", cfg.getme(5));
    
    cfg["x"].setme("x");
    cfg[5].setme("x");
    int x = 5;
    WVPASSEQ("x", cfg[x].getme("x"));
    
    cfg[cfg.getme(5)].setme("x");
    cfg.setme(cfg.getme(x));
    cfg.setme(cfg.getmeint(x));
    cfg.setmeint(cfg.getmeint(x));
    cfg.setmeint(cfg.getme(x).num());
    
    cfg[WvString(x)].setme("x");
    cfg[WvFastString(x)].setme("x");
    cfg[WvString(5)].setme("x");
    
    cfg.setme(7);
    cfg[6].setme(*cfg);
    cfg.setme(cfg->num());
    WVPASSEQ("7", *cfg);
    
    cfg.xset("sub", "y");
    WVPASSEQ("y", *cfg["sub"]);
    WVPASSEQ("y", cfg.xget("sub", "foo"));
    WVPASSEQ(55, cfg.xgetint("sub", 55)); // unrecognizable string
    WVPASSEQ(7, cfg.xgetint(6));
    
    cfg.xsetint("sub", 99);
    WVPASSEQ(99, cfg["sub"].getmeint(55));
    
    WVPASSEQ("zz", cfg["blah"]->ifnull("zz"));
}


static int itcount(const UniConf &cfg)
{
    int count = 0;
    
    UniConf::Iter i(cfg);
    for (i.rewind(); i.next(); )
	count++;
    return count;
}


static int ritcount(const UniConf &cfg)
{
    int count = 0;
    
    UniConf::RecursiveIter i(cfg);
    for (i.rewind(); i.next(); )
    {
	fprintf(stderr, "key: '%s'\n", i->fullkey(cfg).cstr());
	count++;
    }
    return count;
}


WVTEST_MAIN("iterators")
{
    UniConfRoot root("temp:");
    root["1"].setme("foo");
    root["2"].setme("blah");
    root["2/2b"].setme("sub");
    root["x/y/z/a/b/1/2/3"].setme("something");
    
    WVPASSEQ(itcount(root), 3);
    WVPASSEQ(itcount(root["2"]), 1);
    WVPASSEQ(itcount(root["1"]), 0);
    WVPASSEQ(itcount(root["2/2b"]), 0);
    WVPASSEQ(itcount(root["3"]), 0);

    WVPASSEQ(ritcount(root), 11);
    WVPASSEQ(ritcount(root["2"]), 1);
    WVPASSEQ(ritcount(root["1"]), 0);
    WVPASSEQ(ritcount(root["2/2b"]), 0);
    WVPASSEQ(ritcount(root["3"]), 0);
    WVPASSEQ(ritcount(root["x/y/z/a/b"]), 3);
    
    UniConf sub(root["x/y/z/a"]);
    UniConf::RecursiveIter i(sub);
    i.rewind(); i.next();
    WVPASSEQ(i->fullkey(sub).printable(), "b");
    i.next();
    WVPASSEQ(i->fullkey(sub).printable(), "b/1");
}
