typedef unsigned long uthread_t;

typedef void* (*thread_func)(void*); 

int uthread_create(uthread_t* tid, thread_func func, void* arg);

int uthread_exit(void* result);

int uthread_join(uthread_t tid, void** result);

int uthread_detach(uthread_t tid);

uthread_t uthread_self();