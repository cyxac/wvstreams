/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Various useful string-based utilities.
 *
 */
#include "strutils.h"
#include "wvbuffer.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>


char *terminate_string(char *string, char c)
/**********************************************/
// Add character c to the end of a string after removing crlf's.
// NOTE: You need a buffer that's at least one character bigger than the
// current length of the string, including the terminating NULL.
{
    char *p;

    if (string == NULL)
    	return NULL;

    p = string + strlen(string) - 1;
    while (*p == '\r' || *p == '\n')
    	p--;

    *(++p) = c;
    *(++p) = 0;

    return string;
}


char *trim_string(char *string)
/*********************************/
// Trims spaces off the front and end of strings.  Modifies the string.
// Specifically DOES allow string==NULL; returns NULL in that case.
{
    char *p;
    char *q;

    if (string == NULL)
    	return NULL;

    p = string;
    q = string + strlen(string) - 1;

    while (q >= p && isspace(*q))
    	*(q--) = 0;
    while (isspace(*p))
    	p++;

    return p;
}


char *trim_string(char *string, char c)
// Searches the string for c and removes it plus everything afterwards.
// Modifies the string and returns NULL if string == NULL.
{
    char *p;

    if (string == NULL)
        return NULL;

    p = string;

    while (*p != 0 && *p != c)
        p++;

    while (*p)
        *(p++) = 0;

    return string;
}


// Replaces whitespace characters with nonbreaking spaces.
char *non_breaking(char * string)
{
    if (string == NULL)
        return (NULL);

    WvDynBuf buf;

    while (*string)
    {
        if (isspace(*string))
	    buf.put("&nbsp;");
        else 
	    buf.putch(*string);
        string++;
    }

    WvString s(buf.getstr());
    return s.edit();
}


// Searches _string (up to length bytes), replacing any occurrences of c1
// with c2.
void replace_char(void *_string, char c1, char c2, int length)
{
    char *string = (char *)_string;
    for (int i=0; i < length; i++)
    	if (*(string+i) == c1)
    	    *(string+i) = c2;
}


char *strlwr(char *string)
{
    char *p = string;
    while (*p)
    {
    	*p = tolower(*p);
    	p++;
    }

    return string;
}


char *strupr(char *string)
{
    char *p = string;
    while (*p)
    {
	*p = toupper(*p);
	p++;
    }

    return string;
}


// true if all the characters in "string" are isalnum().
bool is_word(const char *p)
{
    while (*p)
    {
    	if(!isalnum(*p++))
    	    return false;
    }
    
    return true;
}


// produce a hexadecimal dump of the data buffer in 'buf' of length 'len'.
// it is formatted with 16 bytes per line; each line has an address offset,
// hex representation, and printable representation.
WvString hexdump_buffer(const void *_buf, size_t len)
{
    const unsigned char *buf = (const unsigned char *)_buf;
    size_t count, count2, top;
    WvString out;

    out.setsize(len / 16 * 80 + 80);
    char *cptr = out.edit();
    
    for (count = 0; count < len; count+=16)
    {
	top = len-count < 16 ? len-count : 16;
	cptr += sprintf(cptr, "[%03X] ", (unsigned int)count);
	
	// dump hex values
	for (count2 = 0; count2 < top; count2++)
	{
	    if (count2 && !(count2 % 4))
		*cptr++ = ' ';
	    cptr += sprintf(cptr, "%02X", buf[count+count2]);
	}
	
	// print horizontal separation
	for (count2 = top; count2 < 16; count2++)
	{
	    if (count2 && !(count2 % 4))
	    {
		strcat(cptr, "   ");
		cptr += 3;
	    }
	    else
	    {
		strcat(cptr, "  ");
		cptr += 2;
	    }
	}
	
	*cptr++ = ' ';
	
	// dump character representation
	for (count2 = 0; count2 < top; count2++)
	    *cptr++ = (isprint(buf[count+count2])
			     ? buf[count+count2] : '.');
	*cptr++ = '\n';
    }
    *cptr = 0;
    return out;
}


// return true if the character is a newline.
bool isnewline(char c)
{
    return c=='\n' || c=='\r';
}


// eg: hexify(foo, "ABCDEF", 4) will set foo to "41424344".
void hexify(char *obuf, const void *_ibuf, size_t len)
{
    const unsigned char *ibuf = (const unsigned char *)_ibuf;
    
    while (len > 0)
    {
	sprintf(obuf, "%02x", *ibuf++);
	obuf += 2;
	len--;
    }
}


// eg: unhexify(foo, "41424344") sets foo to "ABCD".
void unhexify(void *_obuf, const char *ibuf)
{
    unsigned char *obuf = (unsigned char *)_obuf;
    char lookup[] = "0123456789abcdef", *c, *c2;
    
    if (strlen(ibuf) % 1)  // odd number of bytes in a hex string?  No.
	return;
    
    while (*ibuf != 0)
    {
	c = strchr(lookup, *ibuf);
	c2 = strchr(lookup, *(ibuf+1));
	if (!c || !c2) return;
	
	*obuf++ = ((c - lookup) << 4) | (c2 - lookup);
	ibuf += 2;
    }
}


// ex: WvString foo = web_unescape("I+am+text.%0D%0A");
WvString web_unescape(const char *str)
{
    const char *iptr;
    char *optr;
    char *idx1, *idx2;
    static char hex[] = "0123456789ABCDEF";
    WvString in, intmp(str), out;
 
    in = trim_string(intmp.edit());
    out.setsize(strlen(in) + 1);

    optr = out.edit();
    for (iptr = in, optr = out.edit(); *iptr; iptr++)
    {
        if (*iptr == '+')
            *optr++ = ' ';
        else if (*iptr == '%' && iptr[1] && iptr[2])
        {
            idx1 = strchr(hex, toupper((unsigned char) iptr[1]));
            idx2 = strchr(hex, toupper((unsigned char) iptr[2]));

            if (idx1 && idx2)
                *optr++ = ((idx1 - hex) << 4) | (idx2 - hex);

            iptr += 2;
        }
        else
            *optr++ = *iptr;
    }

    *optr = 0;

    return out;
}


WvString rfc822_date(time_t when)
{
    WvString out;
    out.setsize(80);

    if (when < 0)
        when = time(NULL);

    struct tm *tmwhen = localtime(&when);
    strftime(out.edit(), 80, "%a, %d %b %Y %H:%M:%S %z", tmwhen);

    return out;
}


WvString backslash_escape(WvStringParm s1)
{
    // stick a backslash in front of every !isalnum() character in s1
    if (!s1)
        return "";

    WvString s2;
    s2.setsize(s1.len() * 2 + 1);

    const char *p1 = s1;
    char *p2 = s2.edit();
    while (*p1)
    {
        if (!isalnum(*p1))
            *p2++ = '\\';
        *p2++ = *p1++;
    }
    *p2 = 0;

    return s2;
}


int strcount(WvStringParm s, const char c)
{
    int n=0;
    const char *p = s;
    while ((p=strchr(p, c)) != NULL && p++)
        n++;

    return n;
}


WvString encode_hostname_as_DN(WvStringParm hostname)
{
    WvString dn("");
    
    WvStringList fqdnlist;
    WvStringList::Iter i(fqdnlist);
    
    fqdnlist.split(hostname, ".");
    for (i.rewind(); i.next(); )
	dn.append("dc=%s,", *i);
    dn.append("cn=%s", hostname);
    
    return dn;
}


WvString nice_hostname(WvStringParm name)
{
    WvString nice;
    char *optr, *optr_start;
    const char *iptr;
    bool last_was_dash;
    
    nice.setsize(name.len() + 2);

    iptr = name;
    optr = optr_start = nice.edit();
    if (!isascii(*iptr) || !isalnum(*(const unsigned char *)iptr))
	*optr++ = 'x'; // DNS names must start with a letter!
    
    last_was_dash = false;
    for (; *iptr; iptr++)
    {
	if (!isascii(*iptr))
	    continue; // skip it entirely
	
	if (*iptr == '-' || *iptr == '_')
	{
	    if (last_was_dash)
		continue;
	    last_was_dash = true;
	    *optr++ = '-';
	}
	else if (isalnum(*(const unsigned char *)iptr) || *iptr == '.')
	{
	    *optr++ = *iptr;
	    last_was_dash = false;
	}
    }
    
    if (optr > optr_start && !isalnum(*(const unsigned char *)(optr-1)))
	*optr++ = 'x'; // must _end_ in a letter/number too!
    
    *optr++ = 0;
    
    if (!nice.len())
	return "UNKNOWN";
    
    return nice;
}

WvString getfilename(WvStringParm fullname)
{
    WvString tmp(fullname);
    char *cptr = strrchr(tmp.edit(), '/');
    
    if (!cptr) // no slash at all
	return fullname;
    else if (!cptr[1]) // terminating slash
    {
	*cptr = 0;
	return getfilename(tmp);
    }
    else // no terminating slash
	return cptr+1;
}

WvString getdirname(WvStringParm fullname)
{
    WvString tmp(fullname);
    char *cptr = strrchr(tmp.edit(), '/');
    
    if (!cptr) // no slash at all
	return ".";
    else if (!cptr[1]) // terminating slash
    {
	*cptr = 0;
	return getdirname(tmp);
    }
    else // no terminating slash
    {
	*cptr = 0;
	return !tmp ? WvString("/") : tmp;
    }
}


WvString strreplace(WvStringParm s, WvStringParm a, WvStringParm b)
{
    WvDynBuf buf;
    const char *sptr = s, *eptr;
    
    while ((eptr = strstr(sptr, a)) != NULL)
    {
	buf.put(sptr, eptr-sptr);
	buf.put(b);
	sptr = eptr + strlen(a);
    }
    
    buf.put(sptr, strlen(sptr));
    
    return buf.getstr();
}
