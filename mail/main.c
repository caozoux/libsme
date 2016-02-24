/*
**  main for mailsend - a simple mail sender via SMTP protocol
**
**  Limitations and Comments:
**      I needed to send a alert mail from a bare-bone networked NT machine,
**      but could not find a simple mail sender to do this (not surprised!).
**      so I wrote this one!
**
**
**  Development History:
**      who                  when           why
**      muquit@muquit.com    Mar-23-2001    first cut
*/

#define __MAIN__    1

#include "mailsend.h"


/* exits after writing the usage */
static void usage(void)
{
    char
        **p;

    static char
        *options[]=
        {
"  -copyright            - show copyright information",
"  -4                    - Force to use IPv4 address of SMTP server",
"  -6                    - Force to use IPv6 address of SMTP server",
"  -smtp hostname/IP*    - Hostname/IP address of the SMTP server",
"  -port SMTP port       - SMTP port",
"  -domain    domain     - domain name for SMTP HELO/EHLO",
"  -t    to,to..*        - email address/es of the recipient/s",
"  -cc   cc,cc..         - carbon copy address/es",
"  +cc                   - do not ask for Carbon Copy",
"  -ct   seconds         - Connect timeout. Default is 5 seconds",
#if defined(SO_RCVTIMEO)
"  -read-timeout seconds - Read timeout. Default is 5 seconds",
#endif /* SO_RCVTIMEO */
"  -bc   bcc,bcc..       - blind carbon copy address/es",
"  +bc                   - do not ask for Blind carbon copy",
"  +D                    - do not add Date header",
"  -f    address*        - email address of the sender",
"  -sub  subject         - subject",
"  -list_address file    - a file containing a list of email addresses",
"  -log file             - write log messages to this file",
"  -cs   character set   - for text/plain attachments (default is us-ascii)",
"  -separator character  - separator used with -attach. Default is comma (,)",
"                          If used must be specified before -attach",
"  -enc-type type        - encoding type. base64, 8bit, 7bit etc.",
"                          Default is base64. Special type is \"none\"",
"  -aname name           - name of the attachment. Default is filename",
"  -content-id id        - content-id in the attachment",
"  -mime-type type       - MIME type",
"  -dispostion val       - \"attachment\" or \"inline\". Default is \"attachment\"",
"  -attach file,mime_type,[i/a] (i=inline,a=attachment)",
"                        - attach this file as attachment or inline",
"  -show-attach          - show attachment in verbose mode, default is no",
"  -show-mime-types      - show the compiled in MIME types",
"  -M    \"one line msg\"  - attach this one line text message",
"  -content-type type    - Content type. Default: multipart/mixed",
"  -msg-body path        - Path of the file to include as body of mail",
"  -embed-image image    - Path of image to embed in HTML",
"  -H    \"header\"        - Add custom Header",
"  -name \"Full Name\"     - add name in the From header",
"  -v                    - verbose mode",
"  -V                    - show version info",
"  -w                    - wait for a CR after sending the mail",
"  -rt  email_address    - add Reply-To header",
"  -rrr email_address    - request read receipts to this address",
"  -ssl                  - SMTP over SSL",
"  -starttls             - use STARTTLS if the server supports it",
"  -auth                 - try CRAM-MD5,LOGIN,PLAIN in that order",
"  -auth-cram-md5        - use AUTH CRAM-MD5 authentication",
"  -auth-plain           - use AUTH PLAIN authentication",
"  -auth-login           - use AUTH LOGIN authentication",
"  -user username        - username for ESMTP authentication",
"  -pass password        - password for ESMTP authentication",
"  -example              - show examples",
"  -ehlo                 - force EHLO",
"  -info                 - show SMTP server information",
"  -help                 - shows this help",
"  -q                    - quiet",
            (char *) NULL
        };
    (void) printf("\n");
    (void) printf(" Version: %.1024s\n\n",MAILSEND_VERSION);
    (void) printf(" Copyright: %.1024s\n\n",NO_SPAM_STATEMENT);
#ifdef HAVE_OPENSSL
    (void) fprintf(stdout," (Compiled with OpenSSL version: %s)\n",
                   SSLeay_version(SSLEAY_VERSION));
#else
    (void) fprintf(stdout," (Not compiled with OpenSSL)\n");
#endif /* HAVE_OPENSSL */

    (void) printf(" usage: mailsend [options]\n");
    (void) printf(" Where the options are:\n");

    for (p=options; *p != NULL; p++)
        (void) printf("%s\n",*p);

    (void) fprintf(stdout,"\n The options with * must be specified\n");
    (void) fprintf(stdout,
" Environment variables:\n"
"  SMTP_USER_PASS for plain text password (-pass)\n");
}

#if 0
static void show_examplesX(void)
{
    (void) fprintf(stdout," Example (Note: type without newline):\n");
(void) fprintf(stdout,
" Show server info\n"
" ================\n"
" mailsend -v -info -port 587 -smtp smtp.gmail.com\n"
" mailsend -v -info -ssl -port 465 -smtp smtp.gmail.com\n"
" mailsend -v -info -smtp smtp.example.com -ct 2\n\n");

(void) fprintf(stdout,
" STARTTLS + AUTHENTICATION\n"
" =========================\n");
(void) fprintf(stdout,
" mailsend -to user@gmail.com -from user@gmail.com\n"
" -starttls -port 587 -auth\n"
" -smtp smtp.gmail.com\n"
" -sub test +cc +bc -v\n"
" -user you -pass \"your_password\"\n");
(void) fprintf(stdout,
" Note: Password can be set by env var SMTP_USER_PASS instead of -pass\n\n");

(void) fprintf(stdout,
" SSL + AUTHENTICATION\n"
" ====================\n");
(void) fprintf(stdout,
" mailsend -to user@gmail.com -from user@gmail.com\n"
" -ssl -port 465 -auth\n"
" -smtp smtp.gmail.com\n"
" -sub test +cc +bc -v\n"
" -user you -pass \"your_password\"\n\n");
(void) fprintf(stdout,
" As -auth is specified, CRAM-MD5, LOGIN, PLAIN will be tried in that order.\n"
" Use -auth-cram-md5, -auth-plan, -auth-login for specific auth mechanism.\n\n"
" Note: Password can be set by env var SMTP_USER_PASS instead of -pass\n\n");

(void) fprintf(stdout,
" Attachments\n"
" ===========\n");
(void) fprintf(stdout,
" mailsend -f user@example.com -smtp 10.100.30.1\n"
"  -t user@example.com -sub test -attach \"file.txt,text/plain\"\n"
"  -attach \"/usr/file.gif,image/gif\" -attach \"file.jpeg,image/jpg\"\n\n");

(void) fprintf(stdout,
" The name of the attachment will be file.gif and file.jpeg.\n"
" If you want the name to be different, add the fourth argument with -attach.\n"
" Note: you MUST have to give the attachment type in this case\n\n");

(void) fprintf(stdout,
" mailsend -f user@example.com -smtp 10.100.30.1\n"
"  -t user@example.com -sub test -attach \"file.txt,text/plain\"\n"
"  -attach \"/usr/file.gif,image/gif,a,bar.gif\" -attach \"file.jpeg,image/jpg\"\n\n");


(void) fprintf(stdout,
" mailsend -f user@example.com -smtp 192.168.0.2\n"
"  -t user@example.com -sub test +cc +bc\n"
"  -attach \"c:\\file.gif,image/gif\" -M \"Sending a GIF file\"\n\n");

(void) fprintf(stdout,
" mailsend -f user@example.com -smtp 192.168.0.2\n"
"  -t user@example.com -sub test +cc +bc -cs \"ISO-8859-1\"\n"
"  -attach \"file2.txt,text/plain\"\n\n");

(void) fprintf(stdout,
" Inline Attachment\n"
" =================\n");
    (void) fprintf(stdout,
" mailsend -f user@example.com -d example.com -smtp 10.100.30.1\n"
"  -t user@example.com -sub test -attach \"nf.jpg,image/jpeg,i\"\n"
"  -M \"body line1: content disposition is inline\"\n"
"  -M \"body line2: this is line2 of the body\"\n\n");
}
#endif /* if 0 */

//int main(int argc,char **argv)
int mail_init(void)
{
    char
        *x,
        *y,
        buf[BUFSIZ],
        encrypted_pass[128], /* 80 bytes actually */
        *cipher=NULL,
        *option;

    int
        smtp_info=0,
        is_mime=0,
        add_dateh=1,
        port=(-1),
        rc = (-1),
        no_cc=1,
        no_bcc=1,
        i;

    char
        *address_file=NULL,
        *helo_domain=NULL,
        *smtp_server=NULL,
        *attach_file=NULL,
        *msg_body_file=NULL, /* back in 1.17b15 */ 
        *the_msg=NULL,
        *custom_header=NULL,
        *to=NULL,
        *save_to=NULL,
        *save_cc=NULL,
        *save_bcc=NULL,
        *from=NULL,
        *sub=NULL,
        *cc=NULL,
        *bcc=NULL,
        *rt=NULL,
        *rrr=NULL;

    g_verbose=0;
    g_connect_timeout = DEFAULT_CONNECT_TIMEOUT; /* 5 secs */
    g_read_timeout = DEFAULT_READ_TIMEOUT; /* 5 secs */
    g_quiet=0;
    g_wait_for_cr=0;
    g_do_auth=0;
    g_esmtp=0;
    g_auth_plain=0;
    g_auth_cram_md5=0;
    g_auth_login=0;
    g_do_ssl=0;
    g_do_starttls=0;
    g_log_fp = NULL;
    g_show_attachment_in_log = 0;
    g_use_protocol = MSOCK_USE_AUTO; /* detect IPv4 or IPv6 */
    g_force = MUTILS_FALSE;

    memset(g_log_file,0,sizeof(g_log_file));
    memset(g_username,0,sizeof(g_username));
    memset(g_userpass,0,sizeof(g_userpass));
    memset(encrypted_pass, 0, sizeof(encrypted_pass));
    memset(g_from_name,0,sizeof(g_from_name));
	memset(g_content_type,0,sizeof(g_content_type));
    memset(g_attach_sep, 0, sizeof(g_attach_sep));
    memset(g_attach_name, 0, sizeof(g_attach_name));
    memset(g_content_transfer_encoding, 0, sizeof(g_content_transfer_encoding));
    memset(g_mime_type, 0, sizeof(g_mime_type));
    memset(g_content_id, 0, sizeof(g_content_id));

    /* (void) strcpy(g_content_transfer_encoding,"base64"); */ /* no default */
    (void) strcpy(g_content_disposition,"attachment");
    (void) strcpy(g_attach_sep,",");
    (void) strcpy(g_charset,DEFAULT_CHARSET);
    /*
        No default for mime_type, we will detect from file extensinon
        if no mime type is specified with -mime-type
    */
    /*
    (void) strcpy(g_mime_type,"text/plain");
    */


#ifdef UNIX
    signal(SIGPIPE,SIG_IGN);
#endif /* UNIX */


    addAddressToList("cao.zou@windriver.com","To");
    rc = send_the_mail("cao.zou@windriver.com","cao.zou@windriver.com",NULL,NULL,"eHCR-1000 starting....","147.11.189.41",25,
                "localhost",NULL,NULL,"NULL",0,NULL,NULL,NULL);

    if (rc == 0)
    {
        (void) printf("Mail sent successfully\n");
    }
    else
    {
        (void) printf("Could not send mail\n");
    }
    (void) fflush(stdout);

    return (rc);
}
