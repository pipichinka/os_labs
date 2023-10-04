

typedef void* (*start_routine_t)(void*);

typedef unsigned long mythread_t;

#define ERROR 1


void mythread_exit(void* value);


int mythread_detach(mythread_t id);


int mythread_join(mythread_t id, void** result);


int mythread_create(mythread_t* thread, start_routine_t func, void* arg);