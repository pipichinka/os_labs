#define _GNU_SOURCE
#include <ucontext.h>
#include "uthread.h"
#include <malloc.h>
#include <sys/mman.h>
#include <sys/signal.h>
#include <sys/time.h>
#include <errno.h>
#include <memory.h>
#include <stdlib.h>
#include <syscall.h>
#include <ctype.h>
#include <stdint-gcc.h>
#include <linux/futex.h>


#define ERROR 1
#define STACK_SIZE 512*1024

#define TIME_INTERVAL 100000

#define WAIT_ENDING_THREAD_VALUE 1
#define WAIT_JOIN_THREAD_VALUE 2

typedef struct{
    void* args;
    thread_func start_routine;
    ucontext_t current_context;
    void* return_value;
    __uint32_t futex_wait_value;
    char* stack;
    volatile int is_exited;
    volatile int is_detach;
    volatile int is_ready_to_join;
    volatile int is_joined;
    int is_main_thread;
} struct_uthread_t;


struct queue_thread_node_t{
    struct_uthread_t* thread;
    struct queue_thread_node_t* next;
};


typedef struct{
    int size;
    struct queue_thread_node_t* first;
    struct queue_thread_node_t* last;
} queue_thread_t;


queue_thread_t queue;

ucontext_t tmp_context;

struct_uthread_t* current_working_thread = NULL;
int ptid;
int ppid;

int queue_thread_put(struct_uthread_t* thread){
    struct queue_thread_node_t* new = malloc(sizeof(struct queue_thread_node_t));
    if (new == NULL){
        return ERROR;
    }
    new->thread = thread;
    new->next = NULL;
    if (queue.first == NULL){
        queue.first = new;
        queue.last = new;
        queue.size = 1;
    }
    else{
        queue.last->next = new;
        queue.last = new;
        queue.size++;
    }
    return 0;
}


struct_uthread_t* queue_thread_get(){
    if (queue.size == 0){
        return NULL;
    }
    struct queue_thread_node_t* node = queue.first;
    queue.first = queue.first->next;
    if (queue.size == 1){
        queue.last = NULL;
    }
    queue.size--;
    struct_uthread_t* result = node->thread;
    free(node);
    return result;
}


void uthread_start_routine(void){
    uthread_exit(current_working_thread->start_routine(current_working_thread->args));
}


void kill_last_thread(){
    struct itimerval time;
    time.it_interval.tv_sec = 0;
    time.it_interval.tv_usec = 0;
    time.it_value.tv_sec = 0;
    time.it_value.tv_usec = 0;
    if (setitimer(ITIMER_REAL, &time, NULL) == -1){
        abort(); // we can't disable timer
    }

    sigset_t set;
    sigemptyset(&set);
    for (int i = 1; i < 64; i++){
        if (i != SIGKILL && i != SIGSTOP){
            sigaddset(&set, i);
        }
    }
    if (pthread_sigmask(SIG_BLOCK, &set, NULL)){
        perror("uthread_exit");
        abort(); // we can't make environment for exiting thread
    }

    long stack_size = STACK_SIZE;
    asm volatile (
        "movq $11, %%rax\n\t"
        "movq %[a], %%rdi\n\t" 
        "movq %[b], %%rsi\n\t" 
        "syscall\n\t"                        //unmapping a region
        "movq $60, %%rax\n\t"
        "movq $0, %%rdi\n\t"
        "syscall\n\t"                        //calling exit with status 0
        : : [a]"m"(current_working_thread->stack), [b]"m"(stack_size)
        : "%rax", "%rdi", "%rsi", "%rdx");

    abort(); // normaly we nod't get here 
}


void uthread_alarm_hendler(int sig, siginfo_t *si, void *uc){
    ucontext_t* current_context = (ucontext_t*) uc;
    queue_thread_put(current_working_thread);
    struct_uthread_t* next_working_thread = queue_thread_get();
    while(next_working_thread->is_exited){
        if (next_working_thread->is_main_thread){
            free(next_working_thread);
            if (current_working_thread == next_working_thread){ // only main thread left 
                exit(0);
            }
        } 
        else{
            if (current_working_thread != next_working_thread){
                munmap(next_working_thread->stack, STACK_SIZE);
            }
            else{ //only this thread left, so we can exit this kernel thread
                kill_last_thread(); //this function doesn't return
            }

        }
        next_working_thread = queue_thread_get();
        if (next_working_thread == NULL){
            exit(0); //all threads has finished their work
        }
    }
    struct_uthread_t* old_working_thread = current_working_thread;
    current_working_thread = next_working_thread;
    if (swapcontext(&old_working_thread->current_context, &next_working_thread->current_context) == ENOMEM){
        perror("setcontext");
        abort();
    }
}


int uthread_create(uthread_t* tid, thread_func func, void* arg){
    static int first_entry;
    if (first_entry == 0){
        ptid = gettid();
        ppid = getpid();
        struct_uthread_t* main_thread = malloc (sizeof(struct_uthread_t));
        if (main_thread == NULL){
            return ERROR;
        }
        if (getcontext(&main_thread->current_context) == -1){
            int save_errno = errno;
            free(main_thread);
            errno = save_errno;
            return ERROR;
        }
        main_thread->is_detach = 1;
        main_thread->is_exited = 0;
        main_thread->is_main_thread = 1;
        current_working_thread = main_thread;
        
        stack_t ss;
        ss.ss_flags = 0;
        ss.ss_size = STACK_SIZE;
        ss.ss_sp = mmap(NULL, STACK_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_STACK | MAP_ANONYMOUS, -1, 0);
        sigaltstack(&ss, NULL);
        struct sigaction sa;
        sa.sa_flags = SA_SIGINFO | SA_NODEFER | SA_RESTART; 
        sigemptyset(&sa.sa_mask);
        sa.sa_sigaction = uthread_alarm_hendler;
        if (sigaction(SIGALRM, &sa, NULL) == -1){
            int save_errno = errno;
            free(main_thread);
            errno = save_errno;
            return ERROR;
        }
        struct itimerval time;
        time.it_interval.tv_sec = 0;
        time.it_interval.tv_usec = TIME_INTERVAL;
        time.it_value.tv_sec = 0;
        time.it_value.tv_usec = TIME_INTERVAL;
        if (setitimer(ITIMER_REAL, &time, NULL) == -1){
            int save_errno = errno;
            free(main_thread);
            errno = save_errno;
            return ERROR;
        }
        first_entry = 1;
    }

    char* stack = mmap(NULL, STACK_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_STACK | MAP_ANONYMOUS, -1, 0);
    if (stack == NULL){
        return ERROR;
    }
    struct_uthread_t* thread_struct = (struct_uthread_t*) (stack + STACK_SIZE - sizeof(struct_uthread_t));

    thread_struct->args = arg;
    thread_struct->start_routine = func;
    thread_struct->is_detach = 0;
    thread_struct->is_exited = 0;
    thread_struct->is_ready_to_join = 0;
    thread_struct->is_joined = 0;
    thread_struct->is_main_thread = 0;
    thread_struct->stack = stack;
    thread_struct->return_value = NULL;
    thread_struct->futex_wait_value = WAIT_ENDING_THREAD_VALUE;
    if (getcontext(&thread_struct->current_context) == -1){
        int save_errno = errno;
        munmap(stack, STACK_SIZE);
        errno = save_errno;
        return ERROR;
    }
    thread_struct->current_context.uc_stack.ss_size = STACK_SIZE - sizeof(struct_uthread_t);
    thread_struct->current_context.uc_stack.ss_sp = stack; 
    thread_struct->current_context.uc_link = NULL; 
    makecontext(&thread_struct->current_context, uthread_start_routine, 0);

    queue_thread_put(thread_struct);
    *tid = (uthread_t) thread_struct;
    return 0;
}


int uthread_exit(void* val){
    // if (current_working_thread == NULL || queue.size == 0){
    //     exit(0);
    // }

    if (current_working_thread->is_detach){
        current_working_thread->is_exited = 1;
        raise(SIGALRM); //we won't come back to this function
    }
    current_working_thread->return_value = val;
    current_working_thread->is_ready_to_join = 1;
    while(!current_working_thread->is_joined){
        raise(SIGALRM); // we will come back and check conditional
    }
    current_working_thread->is_exited = 1;
    raise(SIGALRM); //we won't come back to this function
}


int uthread_join(uthread_t tid, void** result){
    struct_uthread_t* thread = (struct_uthread_t*) tid;
    if (thread->is_detach){
        return ERROR;
    }
    while(!thread->is_ready_to_join){
        raise(SIGALRM); // we will come back and check conditional
    }
    if (result != NULL){
        *result = thread->return_value;
    }
    thread->is_joined = 1;
    return 0;
}


int uthread_detach(uthread_t tid){
    struct_uthread_t* thread = (struct_uthread_t*) tid;
    thread->is_detach = 1;
    return 0;
}

uthread_t uthread_self(){
    return (uthread_t) current_working_thread;
}
