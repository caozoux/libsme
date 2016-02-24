#ifndef _MAIL_H_
#define _MAIL_H_

#ifdef __cplusplus  
extern "C"  
{
#endif
int mail_init(void);
/*exmaple:
send_the_mail("cao.zou@windriver.com","cao.zou@windriver.com",NULL,NULL,"eHCR-1000 starting....","147.11.189.41",25,
			"localhost",NULL,NULL,"NULL",0,NULL,NULL,NULL);
*/
int send_the_mail(char *from,char *to,char *cc,char *bcc,char *sub,
					char *smtp_server,int smtp_port,char *helo_domain,
					char *attach_file,char *txt_msg_file,char *the_msg,int is_mime,char *rrr,char *rt,
					int add_dateh);
#ifdef __cplusplus  
}  
#endif
#endif
