#ifndef _MEPLATFORM_H_
#define _MEPLATFORM_H_

typedef void* (*thread_handle)(void* data);
struct methread_d {
	unsigned long int id;
	thread_handle handle;
	void * data;
};

enum THREAD_ATTR {
	THREAD_START = 0,
};
int mcreate_thread(struct methread_d *data, THREAD_ATTR attr);
void wait_thread(struct methread_d *p_id);
void plusleep (int time);
void plsleep (int time);
#endif
