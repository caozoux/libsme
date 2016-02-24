/*
** mkey_hex and iv are generated at compile time
** 
** muquit@muquit.com May-14-2012 
*/
#include "mutils.h"

#define MKEY_HEX_LENGTH 48
#define IV_HEX_LENGTH   8

/* master key and iv are generated on: Tue Feb  2 10:56:22 CST 2016 */
/* -- GENERATED CODE: do not modify stats -- */
static char *mkey_hex="";
static char *iv_hex="";
/* -- GENERATED CODE: do not modify ends -- */

/*
** Return a 24 byte binary data.
*/
unsigned char *mutils_get_master_key24(void)
{
    unsigned char
        *bin = NULL;

    int
        olen;

    char
        *hex,
        *default_mkey_hex;

    default_mkey_hex="cfed17bd5ca36d94bfb77e029a2d0c876f6d0cc2bc8a53d6";
    if (mkey_hex && strlen(mkey_hex) == MKEY_HEX_LENGTH)
    {
        hex = mkey_hex;
    }
    else
    {
        hex = default_mkey_hex;
    }
    bin = mutils_hex_to_bin(hex, MKEY_HEX_LENGTH, &olen);
    if (olen > 0)
        return(bin);

    return(NULL);
}

/*
** Returns a 8 byte binary data
*/
unsigned char *mutils_get_iv8(void)
{
    unsigned char
        *bin = NULL;

    int
        olen;

    char
        *hex,
        *default_iv_hex;

    default_iv_hex="f76f9c327947ee19";
    if (iv_hex && strlen(iv_hex) == IV_HEX_LENGTH)
    {
        hex = iv_hex;
    }
    else
    {
        hex = default_iv_hex;
    }
    bin = mutils_hex_to_bin(hex, IV_HEX_LENGTH, &olen);
    if (olen > 0)
        return(bin);
    return(NULL);
}
