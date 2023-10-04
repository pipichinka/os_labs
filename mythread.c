#define _GNU_SOURCE
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <sched.h>
#include <string.h>
#include <stdio.h>
#include <sys/signal.h>
#include <linux/futex.h>
#include <sys/syscall.h>
#include <stdint-gcc.h>
#include <stdlib.h>
#include "mythread.h"
#define STACK_SIZE 256 * 4096

#define WAIT_ENDING_THREAD_VALUE 1
#define WAIT_JOIN_THREAD_VALUE 2

typedef struct {
    start_routine_t start_routine;
    void* ret_val;
    void* arg;
    int tid;
    char* stack;
    uint32_t wait_value_for_futex;
    volatile int is_exited;
    volatile int is_detach;
} mythread_struct_t;


struct thread_list_node_t{
    mythread_struct_t* thread;
    struct thread_list_node_t* next;
    struct thread_list_node_t* prev;
};
typedef struct thread_list_node_t thread_list_node_t;

thread_list_node_t* thread_list;


int futex(uint32_t *uaddr, int futex_op, uint32_t val, const struct timespec *timeout, uint32_t *uaddr2, uint32_t val3){
    return (int) syscall(SYS_futex, uaddr, futex_op, val, timeout, uaddr2, val3);
}



void wait_on_futex_value(int* futex_addr, int val) {
    while (1) {
        int futex_rc = futex(futex_addr, FUTEX_WAIT, val, NULL, NULL, 0);
        if (futex_rc == -1) {
            if (errno != EAGAIN) {
                perror("futex");
                exit(ERROR);
            }
        } else if (futex_rc == 0) {
            if (*futex_addr == val) {
                return;
            }
        } else {
            exit(ERROR);
        }
    }
}


void wake_futex_blocking(int* futex_addr) {
    while (1){
        int futex_rc = futex(futex_addr, FUTEX_WAKE, INT32_MAX, NULL, NULL, 0);
        if (futex_rc == -1) {
            perror("futex wake");
            exit(ERROR);
        } else if (futex_rc > 0) {
            return;
    }
    }
}


int add_thread_to_list(mythread_struct_t* thread){
    if (thread_list == NULL){
        thread_list = malloc(sizeof(thread_list_node_t));
        if (thread_list == NULL){
            return ERROR;
        }
        thread_list->next == NULL;
        thread_list->prev == NULL;
        thread_list->thread = thread;
    }
    else{
        thread_list_node_t* last = thread_list;
        while (last->next != NULL){
            last = last->next;
        }
        last->next = malloc(sizeof(thread_list_node_t));
        if (last->next == NULL){
            return ERROR;
        }
        last->next->prev = last;
        last->next->next = NULL;
        last->next->thread = thread;
    }
    return 0;
}


int erase_thread_from_list(mythread_struct_t* thread){
    if (thread_list == NULL){
        return ERROR;
    }
    thread_list_node_t* thread_node = thread_list;
    while(thread_node->thread != thread){
        thread_node = thread_node->next;
    }
    if (thread_node->prev != NULL){
        thread_node->prev->next = thread_node->next;
    }
    if (thread_node->next != NULL){
        thread_node->next->prev = thread_node->prev;
    }
    if (thread_node == thread_list){
        thread_list = thread_list->next;
    }
    free(thread_node);
    return 0;
}

//this function never returns
void finish_thread(mythread_struct_t* thread){
    long stack_size = STACK_SIZE;
        asm volatile (
            "movq $11, %%rax\n\t"
            "movq %[a], %%rdi\n\t" 
            "movq %[b], %%rsi\n\t" 
            "syscall\n\t"                        //unmapping a region
            "movq $60, %%rax\n\t"
            "movq $0, %%rdi\n\t"
            "syscall\n\t"                        //calling exit with status 0
        : : [a]"m"(thread->stack), [b]"m"(stack_size)
        : "%rax", "%rdi", "%rsi", "%rdx");

    abort(); // we nod't get here 
}


void mythread_exit(void* value){
    int tid = gettid();
    if (getpid() == tid){
        syscall(SYS_exit, 0);
    }
    //blocking all the signals
    sigset_t set;
    sigemptyset(&set);
    for (int i = 1; i < 64; i++){
        if (i != SIGKILL && i != SIGSTOP){
            sigaddset(&set, i);
        }
    }
    if (pthread_sigmask(SIG_BLOCK, &set, NULL)){
        perror("mythread_exit");
        abort(); // we can't make environment for exiting thread
    }

    if (thread_list == NULL){
        abort(); //thread was't created by my mypthread_create so we better abort at this point
    }

    thread_list_node_t* thread = thread_list;
    while (thread->thread->tid != tid  && thread->next != NULL){
        thread = thread->next;
    }
    if (thread->thread->tid != tid){
        abort(); //thread was't created by my mypthread_create so we better abort at this point
    }
    
    mythread_struct_t* self = thread->thread;
    self->is_exited = 1;
    self->ret_val = value;
    long stack_size = STACK_SIZE;

    if (self->is_detach == 1){
        erase_thread_from_list(self);
        finish_thread(self);
    }

    wake_futex_blocking(&self->wait_value_for_futex);
    wait_on_futex_value(&self->wait_value_for_futex, WAIT_JOIN_THREAD_VALUE);
    finish_thread(self);
}


int mythread_start_func(void* arg){
    mythread_struct_t* self = (mythread_struct_t*) arg;
    mythread_exit((*self->start_routine)(self->arg));
}


int mythread_detach(mythread_t id){
    mythread_struct_t* thread = (mythread_struct_t*) id;
    if (thread->is_exited != 0){
        return ERROR;
    }
    thread->is_detach = 1;
    return 0;
}


int mythread_join(mythread_t id, void** result){
    mythread_struct_t* thread = (mythread_struct_t*) id;
    if (thread->is_detach){
        return ERROR;
    }
    wait_on_futex_value(&thread->wait_value_for_futex, WAIT_ENDING_THREAD_VALUE);
    if (result != NULL){
        *result = thread->ret_val;
    } 
    thread->wait_value_for_futex = 2;
    wake_futex_blocking(&thread->wait_value_for_futex);
    erase_thread_from_list(thread);
    return 0;
}


int mythread_create(mythread_t* thread, start_routine_t func, void* arg){
    void* stack = mmap(NULL, STACK_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
    if (stack == NULL){
        return errno;
    }
    mythread_struct_t* thread_struct = stack + STACK_SIZE - sizeof(mythread_struct_t);
    thread_struct->arg = arg;
    thread_struct->wait_value_for_futex = WAIT_ENDING_THREAD_VALUE;
    thread_struct->is_exited = 0;
    thread_struct->is_detach = 0;
    thread_struct->ret_val = 0;
    thread_struct->start_routine = func;
    thread_struct->stack = stack;
    thread_struct->tid = clone(mythread_start_func, stack + STACK_SIZE - sizeof(mythread_struct_t), CLONE_VM|CLONE_FS|CLONE_FILES|CLONE_SIGHAND|CLONE_THREAD, thread_struct);

    if (thread_struct->tid == -1){
        int ret_val = errno;
        munmap(stack, STACK_SIZE);
        return ret_val;
    }
    add_thread_to_list(thread_struct);
    *thread = (unsigned long) thread_struct;
    return 0;
}
