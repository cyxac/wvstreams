#include "wvtest.h"
#include "wvstringlist.h"

WVTEST_MAIN("basic")
{
    WvString output, desired;
    char * input[] = {"mahoooey", "", "kablooey", "mafooey",0};
    WvStringList l;

    
    // test fill()
    l.fill(input);
    // test popstr()
    for (int i = 0; i < 4; i ++)
    {  
        output = l.popstr();
        if (!WVPASS(output == input[i]))
            printf("   because [%s] != [%s]\n", output.cstr(), desired.cstr());
    }
    // should return empty string for no element
    output = l.popstr();
    WVPASS(output == "");

    
    // populate the list
    for (int i = 0; i < 4; i ++)
        l.append(new WvString(input[i]), true);
    desired = WvString("%s %s %s %s", input[0], input[1], input[2], input[3]);
    output = l.join();
    l.zap();
    if (!WVPASS(output == desired))
        printf("   because [%s] != [%s]\n", output.cstr(), desired.cstr()); 
    
    // split() should ignore all spaces, so just the nonblank ones show up
    l.split(desired);
    for (int i = 0; i < 4; i ++)
    {
	if (i == 1) continue; // this one was blank, so skip it
        desired = WvString("%s", input[i]);
        output = l.popstr();
        if (!WVPASS(output == desired))
	    printf("   because [%s] != [%s]\n", output.cstr(), desired.cstr());
    }
    
    // splitstrict() should detect all spaces and create null entries
    desired = WvString("%s %s %s %s", input[0], input[1], input[2], input[3]);  
    l.splitstrict(desired);
    printf("%s\n", desired.cstr());
    for (int i = 0; i < 4; i ++)
    {
        desired = WvString("%s", input[i]);
        output = l.popstr();
        if (!WVPASS(output == desired))
	    printf("   because [%s] != [%s]\n", output.cstr(), desired.cstr());
    }

    desired = WvString(" %s %s %s %s", input[0], input[1], input[2], input[3]);
    l.splitstrict(desired, " ");
    //printf("%s\n", l.join().cstr());    
    desired = WvString("");
    output = l.popstr();
    // should be an extra space
    if (!WVPASS(output == desired))
        printf("   because [%s] != [%s]\n", output.cstr(), desired.cstr());
    // should be the normal input after the space
    for (int i = 0; i < 4; i ++)
    {
        desired = WvString("%s", input[i]);
        output = l.popstr();
        if (!WVPASS(output == desired))
            printf("   because [%s] != [%s]\n", output.cstr(), desired.cstr());
    }

    desired = WvString(" %s %s %s %s", input[0], input[1], input[2], input[3]);
    l.splitstrict(desired, " ");
    //printf("%s\n", l.join().cstr());    
    desired = WvString("");
    output = l.popstr();
    // should be an extra space
    if (!WVPASS(output == desired))
        printf("   because [%s] != [%s}\n", output.cstr(), desired.cstr());
    for (int i = 0; i < 4; i ++)
    {
        desired = WvString("%s", input[i]);
        output = l.popstr();
        if (!WVPASS(output == desired))
            printf("   because [%s] != [%s}\n", output.cstr(), desired.cstr());
    }
}
