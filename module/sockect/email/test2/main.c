#include <stdio.h>

extern int mail_init(void);
int main(int args, char *argv[])
{
	mail_init();
	return 0;
}
