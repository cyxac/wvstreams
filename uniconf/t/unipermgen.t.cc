#include "wvtest.h"
#include "uniconfroot.h"
#include "uniwatch.h"
#include "unitempgen.h"
#include "unipermgen.h"
#include "unisecuregen.h"
#include "uniunwrapgen.h"

// Same as the one in unicachegen.t.cc
class CbCounter
{
public:
    CbCounter() : 
        cbs(0) {}
    void callback(const UniConf keyconf, const UniConfKey key) 
        { 
            cbs++;
        }
    int cbs;
};

WVTEST_MAIN("permgen basic")
{
    UniConfRoot root;
    IUniConfGen *tempgen = new UniTempGen();
    UniPermGen permgen("temp:");
    permgen.setexec(UniConfKey("/"), UniPermGen::WORLD, true);
    permgen.setread(UniConfKey("/"), UniPermGen::WORLD, true);
    permgen.setwrite(UniConfKey("/"), UniPermGen::WORLD, true);

    UniSecureGen *sec = new UniSecureGen(tempgen, &permgen);
    fprintf(stderr, "Mounting securegen\n");
    WVPASS(root.mountgen(sec));
    fprintf(stderr, "Done\n");

    root["/open/foo"].setmeint(1);
    root["/open/bar"].setmeint(1);
    root["/exec_only/read"].setmeint(1);
    root["/exec_only/noread"].setmeint(1);
    root["/exec_only/read_noexec"].setmeint(1); // should be read
    root["/exec_only/read_noexec/read"].setmeint(1); // should be unreadable
    root["/exec_only/read_noexec/read/exec"].setmeint(1); // should be unreadable
    root["/exec_only/read_noexec/exec/read"].setmeint(1); // should be unreadable

    root["/closed/foo"].setmeint(1);
    root["/closed/bar"].setmeint(1);
    root["/closed/exec/foo"].setmeint(1);

    permgen.chmod(UniConfKey("/exec_only"), 7, 7, 1);
    // FIXME: chmodding one key seems to automatically chmod its
    // children. Is this correct?
    permgen.chmod(UniConfKey("/exec_only/read"), 7, 7, 4);
    permgen.chmod(UniConfKey("/exec_only/noread"), 7, 7, 0);
    permgen.chmod(UniConfKey("/exec_only/read_noexec"), 7, 7, 4);
    permgen.chmod(UniConfKey("/exec_only/read_noexec/read"), 7, 7, 4);
    permgen.chmod(UniConfKey("/exec_only/read_noexec/exec"), 7, 7, 1);
    permgen.chmod(UniConfKey("/exec_only/read_noexec/exec/read"), 7, 7, 4);
    permgen.chmod(UniConfKey("/closed"), 7, 7, 0);
    permgen.chmod(UniConfKey("/closed/exec"), 7, 7, 1);
    permgen.chmod(UniConfKey("/closed/exec/foo"), 7, 7, 5);

    // testing "get"
    WVPASS(root["/open"].getme() == "");
    WVPASS(root["/open/foo"].getme() == "1");
    WVPASS(root["/open/bar"].getme() == "1");

    WVPASS(root["/exec_only"].getme() == WvString::null);
    WVPASS(root["/exec_only/read"].getme() == "1");
    WVPASS(root["/exec_only/noread"].getme() == WvString::null);
    WVPASS(root["/exec_only/read_noexec"].getme() == "1");
    WVPASS(root["/exec_only/read_noexec/read"].getme() == WvString::null);
    WVPASS(root["/exec_only/read_noexec/exec"].getme() == WvString::null);
    WVPASS(root["/exec_only/read_noexec/exec/read"].getme() == WvString::null);

    WVPASS(root["/closed"].getme() == WvString::null);
    WVPASS(root["/closed/foo"].getme() == WvString::null);
    WVPASS(root["/closed/bar"].getme() == WvString::null);

    // testing "set" (obviously incomplete)
    root["/exec_only"].setmeint(1);
    WVPASS(root["/exec_only"].getme() == WvString::null);

    root["/closed"].setmeint(1);
    WVPASS(root["/closed"].getme() == WvString::null);

    // testing iteration
    UniConf::Iter i(root);
    i.rewind();
    for (int k=0; k<3; k++)
    {
        WVPASS(i.next());
        if (i.ptr()->key() == "closed")
        {
            WVPASS(i.ptr()->getme() == WvString::null);
            WVPASS(i._value() == WvString::null);
        }
        else if (i.ptr()->key() == "exec_only")
        {
            WVPASS(i.ptr()->getme() == WvString::null);
            WVPASS(i._value() == WvString::null);
        }
        else if (i.ptr()->key() == "open")
        {
            WVPASS(i.ptr()->getme() == "");
            WVPASS(i._value() == "");
        }
    }
    WVFAIL(i.next());

    // testing recursive iteration
    UniConf::RecursiveIter j(root);
    j.rewind();
    for (int k=0; k<3; k++)
    {
        WVPASS(j.next());
        if (j.ptr()->key() == "closed")
        {
            WVPASS(j.ptr()->getme() == WvString::null);
            WVPASS(j._value() == WvString::null);
        }
        else if (j.ptr()->key() == "exec_only")
        {
            WVPASS(j.ptr()->getme() == WvString::null);
            WVPASS(j._value() == WvString::null);

            for (int l=0; l<3; l++)
            {
                WVPASS(j.next());
                if (j.ptr()->key() == "read")
                {
                    WVPASS(j.ptr()->getme() == "1");
                    WVPASS(j._value() == "1");
                }
                else if (j.ptr()->key() == "read_noexec")
                {
                    WVPASS(j.ptr()->getme() == "1");
                    WVPASS(j._value() == "1");
                }
                else if (j.ptr()->key() == "noread")
                {
                    WVPASS(j.ptr()->getme() == WvString::null);
                    WVPASS(j._value() == WvString::null);

                }
            }
        }
        else if (j.ptr()->key() == "open")
        {
            WVPASS(j.ptr()->getme() == "");
            WVPASS(j._value() == "");

            for (int l=0; l<2; l++)
            {
                WVPASS(j.next());
                if (j.ptr()->key() == "bar")
                {
                    WVPASS(j.ptr()->getme() == "1");
                    WVPASS(j._value() == "1");
                }
                else if (j.ptr()->key() == "foo")
                {
                    WVPASS(j.ptr()->getme() == "1");
                    WVPASS(j._value() == "1");

                }
            }
        }
    }
    WVFAIL(j.next());

    // Checking notifications.. (we will assume that we are getting the
    // right keys for now)
    CbCounter notifywatcher;

    UniWatch watcher(root["/"], 
                     UniConfCallback(&notifywatcher,
                                     &CbCounter::callback));
    
    tempgen->set("open/foo", "2");
    WVPASS(notifywatcher.cbs == 1);
    tempgen->set("exec_only/read", "2");
    WVPASS(notifywatcher.cbs == 2);
    tempgen->set("exec_only/noread", "2");
    WVPASS(notifywatcher.cbs == 2);
    tempgen->set("exec_only/read_noexec", "2");
    WVPASS(notifywatcher.cbs == 3);
    tempgen->set("exec_only/read_noexec/read", "2");
    WVPASS(notifywatcher.cbs == 3);
    tempgen->set("exec_only/read_noexec/exec/read", "2");
    WVPASS(notifywatcher.cbs == 3);
    tempgen->set("closed/foo", "2");
    WVPASS(notifywatcher.cbs == 3);
}
