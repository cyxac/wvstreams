/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997, 1998 Worldvisions Computer Technology, Inc.
 * 
 * Implementation of a simple and efficient printable-string class.  Most
 * of the class is actually inlined and can be found in wvstring.h.
 */
#include "wvstring.h"
#include <ctype.h>
#include <assert.h>

const WvString WvString::wv_null;


// always a handy function
static inline int _max(int x, int y)
{
    return x>y ? x : y;
}


// parse a 'percent' operator from a format string.  For example:
//        cptr      out:  justify   maxlen  return pointer
//        "%s"               0         0    "s"
//        "%-15s"          -15         0    "s"
//        "%15.5s"          15         5    "s"
// and so on.  On entry, cptr should _always_ point at a percent '%' char.
//
static char *pparse(char *cptr, int &justify, int &maxlen)
{
    assert(*cptr == '%');
    cptr++;

    justify = atoi(cptr);
    
    for (; *cptr && *cptr!='.' && *cptr!='%' && !isalpha(*cptr); cptr++)
	;
    if (!*cptr) return cptr;
    
    if (*cptr == '.')
	maxlen = atoi(cptr+1);
    else
	maxlen = 0;
    
    for (; *cptr && *cptr!='%' && !isalpha(*cptr); cptr++)
	;
    
    return cptr;
}


// Accept a printf-like format specifier (but more limited) and an array
// of WvStrings, and render them into another WvString.  For example:
//          WvString x[] = {"foo", "blue", 1234};
//          WvString ret = WvString::do_format("%s%10.2s%-10s", x);
//
// The 'ret' string will be:  "foo        bl1234      "
// Note that only '%s' is supported, though integers can be rendered
// automatically into WvStrings.  %d, %f, etc are not allowed!
//
// This function is usually called from some other function which allocates
// the array automatically.
//
void WvString::do_format(WvString &output, char *format, const WvString **a)
{
    static WvString blank("(nil)");
    const WvString **aptr = a;
    char *iptr = format, *optr;
    int total = 0, aplen, ladd, justify, maxlen;
    
    while (*iptr)
    {
	if (*iptr != '%')
	{
	    total++;
	    iptr++;
	    continue;
	}
	
	// otherwise, iptr is at a percent expression
	iptr = pparse(iptr, justify, maxlen);
	if (*iptr == '%') // literal percent
	{
	    total++;
	    iptr++;
	    continue;
	}
	
	assert(*iptr == 's');

	if (*iptr++ == 's')
	{
	    if (!*aptr || !(*aptr)->str)
		*aptr = &blank;
	    ladd = _max(abs(justify), strlen((*aptr)->str));
	    if (maxlen && maxlen < ladd)
		ladd = maxlen;
	    total += ladd;
	    aptr++;
	}
    }
    
    output.str = new char[total+1];
    
    iptr = format;
    optr = output.str;
    aptr = a;
    while (*iptr)
    {
	if (*iptr != '%')
	{
	    *optr++ = *iptr++;
	    continue;
	}
	
	// otherwise, iptr is at a percent expression
	iptr = pparse(iptr, justify, maxlen);
	if (*iptr == '%')
	{
	    *optr++ = *iptr++;
	    continue;
	}
	if (*iptr++ == 's')
	{
	    aplen = strlen((*aptr)->str);
	    if (maxlen && maxlen < aplen)
		aplen = maxlen;
	
	    if (justify > aplen)
	    {
		memset(optr, ' ', justify-aplen);
		optr += justify-aplen;
	    }
	
	    strncpy(optr, (*aptr)->str, aplen);
	    optr += aplen;
	
	    if (justify < 0 && -justify > aplen)
	    {
		memset(optr, ' ', -justify - aplen);
		optr += -justify - aplen;
	    }
	    
	    aptr++;
	}
    }
    *optr = 0;
}
