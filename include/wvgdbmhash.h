/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2003 Net Integration Technologies, Inc.
 *
 * A hash table container backed by a gdbm database.
 */
#ifndef __WVGDBMHASH_H
#define __WVGDBMHASH_H

#include "wvautoconf.h"

#ifndef HAVE_GDBM_H
# error "Sorry, no gdbm support in wvstreams!"
#endif

#include "wvhashtable.h"
#include "wvserialize.h"

#include <gdbm.h>


// Base class for the template to save space
class WvGdbmHashBase
{
public:
    WvGdbmHashBase(WvStringParm dbfile);
    ~WvGdbmHashBase();

    int add(const datum &key, const datum &data, bool replace);
    int remove(const datum &key);
    datum find(const datum &key);
    bool exists(const datum &key);
    void zap();
    
    class IterBase
    {
    public:
        IterBase(WvGdbmHashBase &_gdbmhash);
        ~IterBase();
        void rewind();
        void next();
        
    protected:
        WvGdbmHashBase &gdbmhash;
        datum curkey, nextkey;
        datum curdata;
    };
private:
    friend class IterBase;
    GDBM_FILE dbf;
};


/**
 * This hashtable is different from normal WvStreams hashtables in that it
 * stores the data on disk.
 * 
 * This affects memory management for objects stored in it.
 * 
 * For find and operator[], the returned object is only guaranteed to be
 * around until the next find/or next() for iterators. 
 */
template <class K, class D>
class WvGdbmHash : public WvGdbmHashBase
{
public:
    // this class is interchangeable with datum, but includes a WvDynBuf
    // object that datum.dptr points to.  So when this object goes away,
    // it frees its dptr automatically.
    template <typename T>
    class datumize : public datum
    {
	datumize(datumize &); // not defined
    public:
	WvDynBuf buf;
	
	datumize(const T &t)
	{
	    wv_serialize(buf, t);
	    dsize = buf.used();
	    dptr = (char *)buf.peek(0, buf.used());
	}
    };
    
    template <typename T>
    static T undatumize(datum &data)
    {
	WvConstInPlaceBuf buf(data.dptr, data.dsize);
	return wv_deserialize<T>(buf);
    }

protected:
    D *saveddata;
    
public:
    WvGdbmHash(WvStringParm dbfile) : WvGdbmHashBase(dbfile)
        { saveddata = NULL; }

    void add(const K &key, const D &data, bool replace = false)
    {
        int r = WvGdbmHashBase::add(datumize<K>(key),
				    datumize<D>(data), replace);
        assert(!r && "Set the replace flag to replace existing elements.");
    }

    void remove(const K &key)
        { WvGdbmHashBase::remove(datumize<K>(key)); }

    D &find(const K &key)
    {   
	if (saveddata)
	    delete saveddata;
	datum s = WvGdbmHashBase::find(datumize<K>(key));
	saveddata = undatumize<D *>(s);
        free(s.dptr);
	return *saveddata;
    }

    D &operator[] (const K &key)
        { return find(key); }
        
    bool exists(const K &key)
        { return WvGdbmHashBase::exists(datumize<K>(key)); }
    
    class Iter : public WvGdbmHashBase::IterBase
    {
	K *k;
	D *d;
    public:
        Iter(WvGdbmHash &_gdbmhash) : IterBase(_gdbmhash) 
	    { k = NULL; d = NULL; }
	~Iter()
	{
	    if (k) delete k;
	    if (d) delete d;
	}

        bool next()
        {
            if (!nextkey.dptr)
                return false;
	    if (k) delete k;
	    if (d) delete d;
            IterBase::next();
	    if (curdata.dptr)
	    {
		k = undatumize<K *>(curkey);
		d = undatumize<D *>(curdata);
		return true;
	    }
	    else
	    {
		k = NULL;
		d = NULL;
		return false;
	    }
        }
        
	bool cur()
            { return curdata.dptr; }
	
	K &key() const
	    { return *k; }
	
        D *ptr() const
	    { return d; }
	WvIterStuff(D);
    };
};

#endif // __WVGDBMHASH_H
