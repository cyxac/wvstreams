/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Test program for base64 functions...
 */

#include "base64.h"
#include <stdio.h>

int main(int argc, char **argv)
{
    char *str;
    bool dec = false;

    if (argc == 3 && !strcmp(argv[1], "-d"))
    {
        str = argv[2];
        dec = true;
    }
    else if (argc == 2)
        str = argv[1];
    else
        str = "<insert secret message here>";

    if (!dec)
    {
	WvDynBuf b;
        WvString enc = base64_encode(str, strlen(str));
	base64_decode(enc, b);
        printf("before:  %s\n"
	       "encoded: %s\n"
	       "decoded: %s\n",
	       str, enc.cstr(), b.getstr().cstr());
    }
    else
    {
	WvDynBuf b;
        base64_decode(str, b);
        printf("encoded: %s\n"
	       "decoded: %s\n", str, base64_decode(str, b).getstr().cstr());
    }
    return( 0 );
}
