#define _GNU_SOURCE
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sched.h>
#include <signal.h>
#include "uthread.h"


void* f(void* arg){
    printf(" arg %s tid %d \n", (char*) arg, gettid());
    sleep(2);
    char* hello = "hello";
    return hello;
}


void* g(void* arg){
    printf(" arg %s tid %d \n", (char*) arg, gettid());
    int i = 0;
    char buff[100];
    while(1){
        snprintf(buff, 99, "%d  thread: %p\n", i, (void*) uthread_self());
        write(1, buff, strlen(buff));
        i++;
    }
}


void test2(){
    uthread_t t1;
    uthread_t t2;
    int ret = uthread_create(&t1, g, "hello world");
    if (ret){
        printf("uthread_create %s\n", strerror(ret));
        return;
    }
    uthread_detach(t1);
    sleep(2);    
    ret = uthread_create(&t2, g, "hello world");
    if (ret){
        printf("uthread_create %s\n", strerror(ret));
        return;
    }
    uthread_detach(t2);
    uthread_exit(NULL);
    printf("hello\n");
}


void test1(){
    uthread_t t1;
    uthread_t t2;
    int ret = uthread_create(&t1, f, "hello world1");
    if (ret){
        printf("uthread_create %s\n", strerror(ret));
        return;
    }
    uthread_detach(t1);
    sleep(2);    
    ret = uthread_create(&t2, f, "hello world2");
    if (ret){
        printf("uthread_create %s\n", strerror(ret));
        return;
    }

    char* result;
    ret = uthread_join(t2, (void**) &result);
    printf("result: %s\n", result);
    sleep(2);

    ret = uthread_create(&t1, f, "hello world3");
    if (ret){
        printf("uthread_create %s\n", strerror(ret));
        return;
    }
    uthread_detach(t1);
    uthread_exit(NULL);
    printf("hello\n");
}


int main(){
    test2();
    return 1;
}