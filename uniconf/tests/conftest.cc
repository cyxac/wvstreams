/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Test program for the new, hierarchical WvConf.
 */
#include "uniconfini.h"
#include "wvlog.h"
#include "wvconf.h"
#include "wvdiriter.h"
#include "wvfile.h"
#include "wvtclstring.h"


class HelloGen : public UniConfGen
{
public:
    WvString defstr;
    int count;
    
    HelloGen(WvStringParm _def = "Hello World")
	: defstr(_def) { count = 0; }
    virtual void update(UniConf *h);
};


void HelloGen::update(UniConf *h)
{
    wvcon->print("Hello: updating %s\n", h->full_key());
    *h = WvString("%s #%s", defstr, ++count);
    h->dirty = false;
}


class UniConfFileTree : public UniConfGen
{
public:
    WvString basedir;
    UniConf *top;
    WvLog log;
    
    UniConfFileTree(UniConf *_top, WvStringParm _basedir);
    virtual void update(UniConf *h);
    virtual void load();
};


UniConfFileTree::UniConfFileTree(UniConf *_top, WvStringParm _basedir)
    : basedir(_basedir), log("FileTree", WvLog::Info)
{
    top = _top;
    log(WvLog::Notice,
	"Creating a new FileTree based on '%s' at location '%s'.\n",
	basedir, top->full_key());
}


// use the first nonblank line in the file as the config contents.
void UniConfFileTree::update(UniConf *h)
{
    char *line;
    WvString name("/%s", h->gen_full_key());
    WvFile f(name, O_RDONLY);
    
    if (!f.isok())
	log(WvLog::Warning, "Error reading %s: %s\n", name, f.errstr());
    
    while (f.isok())
    {
	line = f.getline(-1);
	if (!line)
	    continue;
	line = trim_string(line);
	if (!line[0])
	    continue;
	
	*h = line;
	h->dirty = false;
	return;
    }
}


void UniConfFileTree::load()
{
    UniConf *h;
    WvDirIter i(basedir, true);
    
    for (i.rewind(); i.next(); )
    {
	log(WvLog::Debug2, ".");
	h = make_tree(top, i->fullname);
    }
}


int main()
{
    WvLog log("hconftest", WvLog::Info);
    WvLog quiet("*", WvLog::Debug1);
    
    log("An hconf instance is %s bytes long.\n", sizeof(UniConf));
    log("A wvconf instance is %s/%s/%s bytes long.\n",
	sizeof(WvConf), sizeof(WvConfigSection), sizeof(WvConfigEntry));
    log("A stringlist is %s bytes long.\n", sizeof(WvStringList));
    
    {
	wvcon->print("\n\n");
	log("-- Key test begins\n");
	
	UniConfKey key("/a/b/c/d/e/f/ghij////k/l/m");
	UniConfKey key2(key), key3(key, 5), key4(key, 900);
	log("key : %s\nkey2: %s\nkey3: %s\nkey4: %s\n", key, key2, key3, key4);
    }
    
    {
	wvcon->print("\n\n");
	log("-- Basic config test begins\n");
	
	UniConf cfg;
	cfg.set("/foo/blah/weasels", "chickens");
	
	cfg["foo"]["pah"]["meatballs"] = 6;
	
	UniConf &x = cfg["snort/fish/munchkins"];
	x.set("big/bad/weasels", 7);
	x["foo"] = x["blue"] = x["true"] = "sneeze";
	
	log("Config dump:\n");
	cfg.dump(quiet);
    }
    
    {
	wvcon->print("\n\n");
	log("-- Inheritence test begins\n");
	
	UniConf cfg, *h;
	
	cfg.set("/default/users/*/comment", "defuser comment");
	cfg.set("/default/users/bob/comment", "defbob comment");
	
	// should be (nil)/(nil)
	log("Old comment settings are: %s/%s\n",
	    cfg["/users/randomperson/comment"], cfg["/users/bob/comment"]);
	
	cfg["/users"].defaults = &cfg["/default/users"];
       
	// should be defuser comment
	h = cfg["/users/randomperson/comment"].find_default();
	log("Default for randomperson(%s): '%s'\n", h ? h->full_key() : "",
	    h ? *h : WvString("NONE"));
	
	// should be defbob comment
	h = cfg["/users/bob/comment"].find_default();
	log("Default for bob: '%s'\n", h ? *h : WvString("NONE"));
	
	// should be defuser comment/defbob comment
	log("New comment settings are: %s/%s\n",
	    cfg["/users/noperson/comment"], cfg["/users/bob/comment"]);
	
	cfg["/users/bob/someone/comment"] = "fork";
	
	log("Config dump 2:\n");
	cfg.dump(quiet);
    }
    
    {
	wvcon->print("\n\n");
	log("-- Hello Generator test begins\n");
	
	UniConf cfg;
	
	cfg["/hello"].generator = new HelloGen("Hello world!");
	cfg["/bonjour"].generator = new HelloGen("Bonjour tout le monde!");
	
	cfg.get("/bonjour/1");
	cfg.get("/bonjour/2");
	cfg.get("/bonjour/3");
	cfg.get("/hello/3");
	cfg.get("/hello/2");
	cfg.get("/hello/1");
	
	log("Config dump:\n");
	cfg.dump(quiet);
    }
    
    {
	wvcon->print("\n\n");
	log("-- FileTree test begins\n");
	
	UniConf cfg;
	
	cfg.generator = new UniConfFileTree(&cfg, "/etc/modutils");
	cfg.generator->load();
	
	log("Config dump:\n");
	cfg.dump(quiet);
    }
    
    {
	wvcon->print("\n\n");
	log("-- IniFile test begins\n");
	
	UniConf cfg;
	UniConf *cfg2 = &cfg["/weaver ini test"];
	
	cfg.generator = new UniConfIniFile(&cfg, "test.ini");
	cfg.generator->load();
	
	cfg2->generator = new UniConfIniFile(cfg2, "/tmp/weaver.ini");
	cfg2->generator->load();
	
	log("Config dump:\n");
	cfg.dump(quiet);
    }
    
    {
	wvcon->print("\n\n");
	log("-- IniFile test2 begins\n");
	
	UniConf cfg;
	UniConf &h1 = cfg["/1"], &h2 = cfg["/"];
	
	h1.generator = new UniConfIniFile(&h1, "test.ini");
	h1.generator->load();
	
	h2.generator = new UniConfIniFile(&h2, "test2.ini");
	h2.generator->load();
	
	log("Partial config dump (branch 1 only):\n");
	h1.dump(quiet);
	
	log("Trying to save unchanged branches:\n");
	cfg.save();
	
	log("Changing some data:\n");
	if (!h1["big/fat/bob"])
	    h1["big/fat/bob"] = 0;
	h1["big/fat/bob"] = h1["big/fat/bob"].num() + 1;
	h1["chicken/hammer\ndesign"] = "simple test";
	h1["chicken/whammer/designer\\/code\nweasel"] = "this\n\tis a test  ";
	h1.dump(quiet);
	
	log("Saving changed data:\n");
	cfg.save();
    }
    
    return 0;
}
