#include "methread.h"

#ifdef WINHOST
bool mcreate_thread(struct methread_d, THREAD_ATTR attr)
{

}
#else
#include <pthread.h>
#include <sys/wait.h>
#include <unistd.h>
int mcreate_thread(struct methread_d *data, THREAD_ATTR attr)
{
	if (!data->handle)
		return -1;
	return pthread_create(&data->id, NULL, data->handle, data->data);
}
void wait_thread(struct methread_d *p_id)
{
	pthread_join(p_id->id, &p_id->data);
}

void plsleep (int time)
{
	sleep(time);
}

void plmsleep (int time)
{
	usleep(time);
}
#endif
