#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <errno.h>
#include "directory.h"
#include <sys/types.h>

typedef struct Directory{
    char* dir_name;
    int name_size;
    DIR* dir;
} Directory;


Directory* new_Directory(char* dir_name, int dir_name_size){
    if (dir_name == NULL || dir_name_size < 0){
        return NULL;
    }
    Directory* result = (Directory*) malloc(sizeof(Directory));
    if (result == NULL){
        perror("new_Dirrectory"); 
        return NULL;
    }
    result->dir_name = (char*) malloc(sizeof(char) * (dir_name_size + 1));
    if (result->dir_name == NULL){
        perror("new_Dirrectory");
        free(result);
        return NULL;
    }
    strcpy(result->dir_name, dir_name);
    result->name_size = dir_name_size;
    result->dir = opendir(dir_name);
    if (result->dir == NULL){
        fprintf(stderr,"new_Dirrectory: openning dir %s: %s", dir_name, strerror(errno)); 
        free(result->dir_name);
        free(result);
        return NULL;
    }
    return result;
}


void free_Directory(Directory* dir){
    if (dir == NULL){
        return;
    }
    closedir(dir->dir);
    free(dir->dir_name);
    free(dir);
}


DIR* get_DIR(Directory* dir){
    if (dir == NULL){
        return NULL;
    }
    return dir->dir;
}


int make_path(char* dir, int dir_name_size, char* file_name, int file_name_size, char* result){
    if (dir == NULL || dir_name_size <= 0 || file_name == NULL || file_name_size <= 0 || result == NULL){
        return ERROR_DEIRECTORY;
    }
    strncpy(result,dir,dir_name_size);
    result[dir_name_size] = '/';
    strncpy(result + dir_name_size + 1, file_name, file_name_size);
    result[dir_name_size + file_name_size +1] = '\0';
    return 0;
}


char* get_dir_name(Directory* dir){
    if (dir == NULL){
        return NULL;
    }
    return dir->dir_name;
}


int get_dir_name_size(Directory* dir){
    if (dir == NULL){
        return ERROR_DEIRECTORY;
    }
    return dir->name_size;
}
