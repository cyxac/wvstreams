/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * X.509 certificate management classes.
 */ 
#ifndef __WVX509_H
#define __WVX509_H

#include "wvrsa.h"
#include "wvlog.h"

// Structures to make the compiler happy so we don't have to include x509v3.h ;)
struct x509_st;
typedef struct x509_st X509;

// workaround for the fact that OpenSSL initialization stuff must be called
// only once.
void wvssl_init();
void wvssl_free();
WvString wvssl_errstr();


/**
 * X509 Class to handle certificates and their related
 * functions
 */
class WvX509Mgr
{
public:
   /** Distinguished Name to be used in the certificate. */
    WvString dname;

   /**
    * Type for the @ref dump() method, which can output the information
    * in this class in a variety of formats
    */
    enum DumpMode { CertPEM = 0, RsaPEM, RsaRaw };

   /**
    * Initialize a blank X509 Object with the certificate *cert
    * (used for client side operations...)
    * 
    * This either initializes a completely empty object, or takes _cert, and extracts
    * the distinguished name into dname, and the the RSA public key into rsa. rsa->prv is empty.
    */
    WvX509Mgr(X509 *_cert = NULL);

    /** Constructor to initialize this object with a pre-existing certificate and key */
    WvX509Mgr(WvStringParm hexcert, WvStringParm hexrsa);

    /**
     * Constructor to create a selfsigned certificate for dn dname
     * NOTE: If you already have a WvRSAKey, then you can shove it
     * in here in the second parameter (i.e.: If you wanted to generate a
     * cert for an existing TunnelVision connection), or if you don't have an 
     * RSA Key yet, you can just give it a number of bits, and it will create 
     * one for you.
     *
     * Also: For SSL Servers:
     * the dname MUST be in the form: cn=FQDN,o=foo,c=CA
     * (actually, any part after the cn=FQDN is up to you... dc= works as well..)
     *
     * But the important bit is to use the Fully Qualified Domain Name in 
     * the cn= part - otherwise Web Clients get confused...(I imagine other
     * server clients will get equally confused, but I haven't checked).
     * I don't check for this, since other kinds of certificates are perfectly
     * valid without this... If you want to generate invalid certs, that's up
     * to you.
     */
    WvX509Mgr(WvStringParm _dname, WvRSAKey *_rsa);
    WvX509Mgr(WvStringParm _dname, int bits);
    
    /** Placeholder: this doesn't exist yet. */
    WvX509Mgr(const WvX509Mgr &mgr);

    /** Destructor */
    virtual ~WvX509Mgr();

    /** X.509v3 Certificate - this is why this class exists */
    X509     *cert;

    /**
     * The Public and Private RSA keypair associated with this certificate
     * Make sure that you save this somewhere!!! If you don't, then you won't
     * really be able to use the certificate for anything...
     */
    WvRSAKey *rsa;
    
    /**
     * Given the Distinguished Name dname and an already generated keypair in 
     * rsa, return a Self Signed Certificate in cert.
     */
    void create_selfsigned();

    /**
     * Create a certificate request (PKCS#10) using this function.. this 
     * request is what you would send off to Verisign, or Entrust.net (or any
     * other CA), to get your real certificate. It leaves the RSA key pair
     * in rsa, where you MUST save it for the certificate to be AT ALL
     * valid when you get it back. Returns a PEM Encoded PKCS#10 certificate
     * request, and leaves the RSA keypair in rsa, and a self-signed temporary
     * certificate in cert.
     */    
    WvString certreq();
    
    /**
     * test to make sure that a certificate and a keypair go together.
     * called internally by unhexify() although you can call it if 
     * you want to test a certificate yourself
     */
    bool test();

    /**
     * Given a hexified certificate, fill the cert member NOTE: ALWAYS load
     * your RSA Keys before calling this! It is best if you have hexify()'d
     * keys to simply use the proper constructor. 
     */
    void unhexify(WvStringParm encodedcert);
    
    /**
     * Given the X509 certificate object cert, return a hexified string
     * useful in a WvConf or UniConf file.
     * 
     * I don't provide a similar function for that for the rsa key, because
     * you can always call rsa->private_str() and rsa->public_str()
     * for that information.
     */
    WvString hexify();

    /**
     * Function to verify the validity of a certificate that has been
     * placed in cert. Currently, this only outputs some information about
     * the certificate but eventually, it should be used to verify that the
     * certificate is valid (has not expired, and was issued by a valid and
     * trusted CA) 
     */
    bool validate();
   
    /**
     * Check the certificate in cert against the CA certificates in
     * certfile - returns true if cert was signed by one of the CA
     * certificates.
     */
    bool signedbyCAindir(WvStringParm certdir);
   
    /**
     * Check the certificate in cert against the CA certificates in certdir
     * - returns true if cert was signed by one of the CA certificates. 
     */
   bool signedbyCAinfile(WvStringParm certfile);

    /**
     * Sign the X509 certificate in cert with CAKeypair
     *
     * NOT IMPLEMENTED
     */
    void sign(WvRSAKey CAKeypair);
   
    /**
     * Check and see if the certificate in cert has been revoked... currently
     * relies on the CRL Distribution Point X509v3 extension...
     * returns true if it has expired
     * 
     * NOT IMPLEMENTED
     */
    bool isinCRL();

    /** Return the information requested by mode as a WvString. */
    WvString encode(const DumpMode mode);

    /**
     * Load the information from the format requested by mode into
     * the class - this overwrites the certificate, and possibly the
     * key - and to enable two stage loading (the certificate first, then the
     * key), it DOES NOT call test() - that will be up to the programmer
     */
    void decode(const DumpMode mode, WvStringParm PemEncoded);

    /**
     * And of course, since PKCS12 files are in the rediculous DER encoding 
     * format, which is binary, we can't use the encode/decode functions, so
     * we deal straight with files... *sigh*
     *  
     * As should be obvious, this writes the certificate and RSA keys in PKCS12
     * format to the file specified by filename.
     */
    void write_p12(WvStringParm filename);
    
    /**
     * And this reads from the file specified in filename, and fills the RSA and
     * cert members with the decoded information.
     */
    void read_p12(WvStringParm filename);

    /** Sets the PKCS12 password */
    void setPkcs12Password(WvStringParm passwd)
    	{ pkcs12pass = passwd; }

    /** 
     * Return the Certificate Issuer (usually the CA who issued 
     * the certificate)
     */
    WvString get_issuer();

    /**
     * Return the Subject field of the certificate
     */
    WvString get_subject();

    /**
     * Return the CRL Distribution points if they exist, WvString::null
     * if they don't.
     */
    WvString get_crl_dp();

    /**
     * Return the Certificate Policy OID if it exists, and WvString::null
     * it if doesn't.
     */
    WvString get_cp_oid();

    /**
     * Return the Subject alt name if it exists, and WvString::null if
     * it doesn't.
     */
    WvString get_altsubject();

    /**
     * Is this certificate Object valid, and in a non-error state
     */
    bool isok() const
        { return cert && rsa && !errstring; }

    /**
     * Accessor for the error string if !isok()
     */
    const WvString &errstr()
        { return errstring; }

private:
    WvLog debug;
    WvString errstring;

   /** 
    * Password for PKCS12 dump - we don't handle this entirely correctly 
    * since we should erase it from memory as soon as we are done with it
    */
    WvString pkcs12pass;

    void seterr(WvStringParm s)
        { if (!errstring) errstring = s; }

    void seterr(WVSTRING_FORMAT_DECL)
        { seterr(WvString(WVSTRING_FORMAT_CALL)); }

    /**
     * Get the Extension information - returns NULL if extension doesn't exist
     */
    WvDynBuf *get_extension(int nid);

    /**
     * Populate dname (the distinguished name);
     */
    void filldname();
};

#endif // __WVX509_H
