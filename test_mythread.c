#define _GNU_SOURCE
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sched.h>
#include "mythread.h"
void* f(void* arg){
    printf(" arg %s tid %d \n", (char*) arg, gettid());
    sleep(2);
    char* hello = "hello";
    return hello;
}


int main(){
    mythread_t t1;
    mythread_t t2;
    int ret = mythread_create(&t1, f, "hello world");
    if (ret){
        printf("mythread_create %s\n", strerror(ret));
        return 0;
    }
    //sleep(2);
    ret = mythread_create(&t2, f, "hello world");
    if (ret){
        printf("mythread_create %s\n", strerror(ret));
        return 0;
    }
    char* result;
    mythread_detach(t1);
    //sleep(5);
    //ret = mythread_join(t1, (void**) &result);
    //printf("result: %s\n", result);
    ret = mythread_join(t2, (void**) &result);
    printf("result: %s\n", result);


    return 0;
}