#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <errno.h>
#include "dirrectory.h"
#include <sys/types.h>


typedef struct Dirrectory{
    char* dir_name;
    int name_size;
    DIR* dir;
} Dirrectory;


Dirrectory* new_Dirrectory(char* dir_name, int dir_name_size){
    if (dir_name == NULL || dir_name_size < 0){
        return NULL;
    }
    Dirrectory* result = (Dirrectory*) malloc(sizeof(Dirrectory));
    if (result == NULL){
        printf("new_Dirrectory: %s", strerror(errno)); 
        return NULL;
    }
    result->dir_name = (char*) malloc(sizeof(char) * (dir_name_size + 1));
    if (result->dir_name == NULL){
        printf("new_Dirrectory: %s", strerror(errno)); 
        free(result);
        return NULL;
    }
    strcpy(result->dir_name, dir_name);
    result->name_size = dir_name_size;
    result->dir = opendir(dir_name);
    if (result->dir == NULL){
        printf("new_Dirrectory: openning dir %s: %s", dir_name, strerror(errno)); 
        free(result->dir_name);
        free(result);
        return NULL;
    }
    return result;
}


void free_Dirrectory(Dirrectory* dir){
    if (dir == NULL){
        return;
    }
    closedir(dir->dir);
    free(dir->dir_name);
    free(dir);
}


DIR* get_DIR(Dirrectory* dir){
    if (dir == NULL){
        return NULL;
    }
    return dir->dir;
}


char* make_path(char* dir, int dir_name_size, char* file_name, int file_name_size){
    if (dir == NULL || dir_name_size <= 0 || file_name == NULL || file_name_size <= 0){
        return NULL;
    }
    char* path = (char*) malloc(sizeof(char) * (dir_name_size + file_name_size + 2));
    if (path == NULL){
        return NULL;
    }
    strncpy(path,dir,dir_name_size);
    path[dir_name_size] = '/';
    strncpy(path + dir_name_size + 1, file_name, file_name_size);
    path[dir_name_size + file_name_size +1] = '\0';
    return path;
}


char* get_dir_name(Dirrectory* dir){
    if (dir == NULL){
        return NULL;
    }
    return dir->dir_name;
}


int get_dir_name_size(Dirrectory* dir){
    if (dir == NULL){
        return -1;
    }
    return dir->name_size;
}