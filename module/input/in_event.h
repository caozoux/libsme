#ifndef __IN_EVENT__
#define __IN_EVENT__


typedef void (*fd_func)(int fd, unsigned events, void *userdata);
/* Allocate and initialize a new fdevent object
 * Note: use FD_TIMER as 'fd' to create a fd-less object
 * (used to implement timers).
*/
fdevent *fdevent_create(int fd, fd_func func, void *arg);

/* Uninitialize and deallocate an fdevent object that was
** created by fdevent_create()
*/
void fdevent_destroy(fdevent *fde);

/* Initialize an fdevent object that was externally allocated
*/
void fdevent_install(fdevent *fde, int fd, fd_func func, void *arg);

/* Uninitialize an fdevent object that was initialized by
** fdevent_install()
*/
void fdevent_remove(fdevent *item);

/* Change which events should cause notifications
*/
void fdevent_set(fdevent *fde, unsigned events);
void fdevent_add(fdevent *fde, unsigned events);
void fdevent_del(fdevent *fde, unsigned events);

void fdevent_set_timeout(fdevent *fde, int64_t  timeout_ms);

/* loop forever, handling events.
*/
void fdevent_loop();

struct fdevent 
{
    fdevent *next;
    fdevent *prev;

    int fd; 
    int force_eof;

    unsigned short state;
    unsigned short events;

    fd_func func;
    void *arg;
};
#endif
