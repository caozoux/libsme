#include<stdio.h>
#include "smtpfuncs.h"

int main(int argc, char* argv[])
{
	int ret;
	ret = send_mail("smtp-na.wrs.com", "cao.zou@windriver.com", "cao.zou@windriver.com",
				"test mail", "asf", "hello");
	if (ret) {
		printf("send email failed\n");
	}
	return 0;
}
