/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Defines a UniConfGen that manages a tree of UniConfGen instances.
 */
#ifndef __UNIMOUNTTREEGEN_H
#define __UNIMOUNTTREEGEN_H

#include "uniconfgen.h"
#include "uniconftree.h"
#include "wvstringtable.h"

/**
 * Used by UniMountTreeGen to maintain information about mounted
 * subtrees.
 */
class UniMountTree : public UniConfTree<UniMountTree>
{
public:
    UniConfGenList generators;

    UniMountTree(UniMountTree *parent, const UniConfKey &key);
    ~UniMountTree();

    /** Returns true if the node should not be pruned. */
    bool isessential()
        { return haschildren() || ! generators.isempty(); }

    /**
     * Returns the nearest node in the info tree to the key.
     * "key" is the key
     * "split" is set to the number of leading segments used
     * Returns: the node
     */
    UniMountTree *findnearest(const UniConfKey &key, int &split);

    /** Finds or makes an info node for the specified key. */
    UniMountTree *findormake(const UniConfKey &key);
   
    // an iterator over nodes that have information about a key
    class MountIter;
    // an iterator over generators about a key
    class GenIter;
};


/**
 * An iterator over the UniMountTree nodes that might know something
 * about the provided 'key', starting with the nearest match and then
 * moving up the tree.
 */
class UniMountTree::MountIter
{
    int bestsplit;
    UniMountTree *bestnode;

    int xsplit;
    UniMountTree *xnode;
    UniConfKey xkey;

public:
    MountIter(UniMountTree &root, const UniConfKey &key);
    
    void rewind();
    bool next();
    
    int split() const
        { return xsplit; }
    UniConfKey key() const
        { return xkey; }
    UniConfKey head() const
        { return xkey.first(xsplit); }
    UniConfKey tail() const
        { return xkey.removefirst(xsplit); }
    UniMountTree *node() const
        { return xnode; }
    UniMountTree *ptr() const
        { return node(); }
    WvIterStuff(UniMountTree);
};


/**
 * An iterator over the generators that might provide a key
 * starting with the nearest match.
 * 
 * eg. if you have something mounted on /foo and /foo/bar/baz, and you ask
 * for a GenIter starting at /foo/bar/baz/boo/snoot, GenIter will give you
 * /foo/bar/baz followed by /foo; MountIter will give you /foo/bar/baz,
 * then /foo/bar, then /foo.
 */
class UniMountTree::GenIter : private UniMountTree::MountIter
{
    UniConfGenList::Iter *genit; /*!< active generator iterator */

public:
    GenIter(UniMountTree &root, const UniConfKey &key);
    ~GenIter();

    typedef UniMountTree::MountIter ParentClass;
    using ParentClass::split;
    using ParentClass::key;
    using ParentClass::head;
    using ParentClass::tail;
    using ParentClass::node;

    void rewind();
    bool next();

    IUniConfGen *ptr() const
        { return genit ? genit->ptr() : NULL; }
    WvIterStuff(IUniConfGen);
};


/** The UniMountTree implementation realized as a UniConfGen. */
class UniMountTreeGen : public UniConfGen
{
    class KeyIter;
    friend class KeyIter;
    
    UniMountTree *mounts;

    /** undefined. */
    UniMountTreeGen(const UniMountTreeGen &other);

public:
    /** Creates an empty UniConf tree with no mounted stores. */
    UniMountTreeGen();

    /** Destroys the UniConf tree along with all uncommitted data. */
    ~UniMountTreeGen();
    
    /**
     * Mounts a generator at a key using a moniker.
     * 
     * Returns the generator instance pointer, or NULL on failure.
     */
    virtual IUniConfGen *mount(const UniConfKey &key, WvStringParm moniker,
        bool refresh);
    
    /**
     * Mounts a generator at a key.
     * Takes ownership of the supplied generator instance.
     * 
     * "key" is the key
     * "gen" is the generator instance
     * "refresh" is if true, refreshes the generator after mount
     * Returns: the generator instance pointer, or NULL on failure
     */
    virtual IUniConfGen *mountgen(const UniConfKey &key, IUniConfGen *gen,
        bool refresh);

    /**
     * Unmounts the generator at a key and destroys it.
     *
     * "key" is the key
     * "gen" is the generator instance
     * "commit" is if true, commits the generator before unmount
     */
    virtual void unmount(const UniConfKey &key, IUniConfGen *gen, bool commit);
    
    /**
     * Finds the generator that owns a key.
     * 
     * If the key exists, returns the generator that provides its
     * contents.  Otherwise returns the generator that would be
     * updated if a value were set.
     * 
     * "key" is the key
     * "mountpoint" is if not NULL, replaced with the mountpoint
     *        path on success
     * Returns: the handle, or a null handle if none
     */
    virtual IUniConfGen *whichmount(const UniConfKey &key,
				    UniConfKey *mountpoint);

    /** Determines if a key is a mountpoint. */
    virtual bool ismountpoint(const UniConfKey &key);
    
    /***** Overridden members *****/
    
    virtual bool exists(const UniConfKey &key);
    virtual bool haschildren(const UniConfKey &key);
    virtual WvString get(const UniConfKey &key);
    virtual void set(const UniConfKey &key, WvStringParm value);
    virtual bool refresh();
    virtual void commit();
    virtual Iter *iterator(const UniConfKey &key);

private:
    /**
     * Prunes a branch of the tree beginning at the specified node
     * and moving towards the root.
     * "node" is the node
     */
    void prune(UniMountTree *node);

    /** Called by generators when a key changes. */
    void gencallback(const UniConfKey &key, WvStringParm value, void *userdata);
};


#endif //__UNIMOUNTTREEGEN_H
