#include<stdio.h>
#include <string.h>
#include"methread.h"

void* thread_test(void* data)
{
	int cnt = 0;
	while(1) {
		plsleep(1);
		printf("thread run:%d\n",cnt++);
	}
}

int main(void)
{
	printf("aa");
	return 0;
	/*
	struct methread_d p_data;
	printf("%d\n", sizeof(struct methread_d));
	memset(&p_data,0,sizeof(struct methread_d));
	p_data.handle = thread_test;

	if (mcreate_thread(&p_data,(THREAD_ATTR)0)) {
		printf("thread failed\n");	
		return 0;
	}
	*/
}
